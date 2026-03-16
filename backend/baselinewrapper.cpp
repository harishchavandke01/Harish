#include "baselinewrapper.h"
#include <QDebug>
#include <sstream>
#include <fstream>
#include <io.h>

BaselineWrapper::BaselineWrapper(QObject *parent)
    : QObject{parent}
{}

void BaselineWrapper::printhelp(std::ofstream &out, int x)
{
    int i;
    for(i = 0 ; i < (int)(sizeof(x ? help1 : help2)/sizeof(x ? *help1 : *help2)) ; i++)
        out << (x ? help1[i] : help2[i]);
    exit(0);
}

int BaselineWrapper::convbin(int format, rnxopt_t *opt, const std::string &ifile, std::array<std::string, 9> &file, const std::string &dir, std::ofstream &out)
{
    const char* extnav = (opt->rnxver <= 299 || opt->navsys == SYS_GPS) ? "N" : "P";
    const char* extlog = "sbs";

    std::string ifile_mod = ifile;
    std::replace(ifile_mod.begin(), ifile_mod.end(), '*', '0');

    bool def = true;
    for(const auto& f : file) {
        if(!f.empty()) {
            def = false;
            break;
        }
    }

    auto replace_extension = [](std::string& filename, const std::string& new_ext) {
        size_t pos = filename.find_last_of('.');
        if(pos != std::string::npos) {
            filename = filename.substr(0, pos) + new_ext;
        } else {
            filename += new_ext;
        }
    };

    if(file[0].empty()) {
        if(opt->staid[0]) file[0] = "%r%n0.%yO";
        else {
            file[0] = ifile_mod;
            replace_extension(file[0], ".obs");
        }
    }

    if(file[1].empty()) {
        if(opt->staid[0]) file[1] = std::string("%r%n0.%y") + extnav;
        else {
            file[1] = ifile_mod;
            replace_extension(file[1], ".nav");
        }
    }

    if(file[2].empty()) {
        if(opt->rnxver <= 299 && opt->staid[0]) file[2] = "%r%n0.%yG";
        else if(opt->rnxver <= 299 && def) {
            file[2] = ifile_mod;
            replace_extension(file[2], ".gnav");
        }
    }

    if(file[3].empty()) {
        if(opt->rnxver <= 299 && opt->staid[0]) file[3] = "%r%n0.%yH";
        else if(opt->rnxver <= 299 && def) {
            file[3] = ifile_mod;
            replace_extension(file[3], ".hnav");
        }
    }

    if(file[4].empty()) {
        if(opt->rnxver <= 299 && opt->staid[0]) file[4] = "%r%n0.%yQ";
        else if(opt->rnxver <= 299 && def) {
            file[4] = ifile_mod;
            replace_extension(file[4], ".qnav");
        }
    }

    if(file[5].empty()) {
        if(opt->rnxver <= 299 && opt->staid[0]) file[5] = "%r%n0.%yL";
        else if(opt->rnxver <= 299 && def) {
            file[5] = ifile_mod;
            replace_extension(file[5], ".lnav");
        }
    }

    if(file[6].empty()) {
        if(opt->rnxver <= 299 && opt->staid[0]) file[6] = "%r%n0.%yC";
        else if(opt->rnxver <= 299 && def) {
            file[6] = ifile_mod;
            replace_extension(file[6], ".cnav");
        }
    }

    if(file[7].empty()) {
        if(opt->rnxver <= 299 && opt->staid[0]) file[7] = "%r%n0.%yI";
        else if(opt->rnxver <= 299 && def) {
            file[7] = ifile_mod;
            replace_extension(file[7], ".inav");
        }
    }

    if(file[8].empty()) {
        if(opt->staid[0]) file[8] = std::string("%r%n0_%y.") + extlog;
        else {
            file[8] = ifile_mod;
            replace_extension(file[8], std::string(".") + extlog);
        }
    }

    if(!dir.empty()) {
        for(auto& fname : file) {
            if(!fname.empty()) {
                size_t pos = fname.find_last_of("/\\");
                std::string base = (pos != std::string::npos) ? fname.substr(pos + 1) : fname;
                fname = dir;
                if(dir.back() != '/' && dir.back() != '\\') {
                    fname += RTKLIB_FILEPATHSEP;
                }
                fname += base;
            }
        }
    }

    out << "input file  :" << ifile << "(" << formatstrs[format] << ")" << "\n";

    for(int i = 0; i < NOUTFILE; ++i) {
        if(!file[i].empty()) {
            out << "-> output file " << i << " : " << file[i].c_str() << "\n";
        }
    }

    char ofile_buf[NOUTFILE][1024];

    for(int i = 0 ; i < NOUTFILE ; i++) {
        std::snprintf(ofile_buf[i], sizeof(ofile_buf[i]), "%s", file[i].c_str());
    }

    char* ofile_ptrs[NOUTFILE];
    for(int i = 0 ; i < NOUTFILE ; i++) {
        ofile_ptrs[i] = ofile_buf[i];
    }
    if(!convrnx(format, opt, ifile.c_str(), ofile_ptrs)) {
        return 0;
    }
    return 1;
}

int BaselineWrapper::ConvertData(int argc, char **argv, std::string logFile)
{
    std::ofstream out(logFile);
    out << "Welcome to Surveypod RINEX Conversion...\n";
    for(int i = 1 ; i < argc ; i++) {
        out << argv[i] << "\n";
    }

    rnxopt_t opt = {{0}};
    int format, trace = 0, stat;
    std::string ifile;
    std::array<std::string, NOUTFILE> ofile{};
    std::string dir;

    format = cmdopts(argc, argv, &opt, ifile, ofile, dir, trace, out);

    if(ifile.empty()) {
        out << "No input file";
        return EXIT_FAILURE;
    }
    if(format < 0) {
        out << "Input format can not be recognized";
        return EXIT_FAILURE;
    }

    std::snprintf(opt.prog, sizeof(opt.prog), "%s %s %s", PRGNAME1, VER_RTKLIB, PATCH_LEVEL);
    out << "Name : " << PRGNAME1 << "\n";
    out << "RTKLIB Version : " << VER_RTKLIB << "\n";
    out << "Patch Level : " << PATCH_LEVEL << "\n";

    std::snprintf(opt.prog, sizeof(opt.prog),"SURVEYPOD 1.0");

    std::snprintf(opt.runby, sizeof(opt.runby), "Nibrus Technologies Pvt Ltd");

    if(trace > 0) {
        traceopen(TRACEFILE);
        tracelevel(trace);
    }

    stat = convbin(format, &opt, ifile, ofile, dir, out);
    traceclose();
    return stat ? 0 : EXIT_FAILURE;
}

void BaselineWrapper::setmask(const char *argv, rnxopt_t *opt, int mask)
{
    char buff[1024],*p;
    int i;

    std::snprintf(buff, sizeof(buff), "%s", argv);
    char *r;
    for(p = strtok_s(buff,",",&r) ; p ; p = strtok_s(NULL,",",&r)) {
        if (strlen(p) < 4 || p[1] != 'L') continue;
        if     (p[0] == 'G') i = RNX_SYS_GPS;
        else if(p[0] == 'R') i = RNX_SYS_GLO;
        else if(p[0] == 'E') i = RNX_SYS_GAL;
        else if(p[0] == 'J') i = RNX_SYS_QZS;
        else if(p[0] == 'S') i = RNX_SYS_SBS;
        else if(p[0] == 'C') i = RNX_SYS_CMP;
        else if(p[0] == 'I') i = RNX_SYS_IRN;
        else continue;
        int code = obs2code(p+2);
        if(code != CODE_NONE) {
            opt->mask[i][code-1] = mask ? '1' : '0';
        }
    }
}

void setglofcn(const char *argv, rnxopt_t *opt,  std::ofstream &out) {
    char buff[1024];
    std::snprintf(buff, sizeof(buff), "%s", argv);
    buff[1023] = '\0';
    char *p = buff;
    for(int i = 0 ; i < 32 ; i++) {
        if(p == NULL) break;
        char *fcnstr = p;
        for(;;) {
            int c = *p++;
            if(c == ',') {
                p[-1] = '\0';
                break;
            }
            if(c == '\0') {
                p = NULL;
                break;
            }
        }
        if (strlen(fcnstr) < 1) continue;
        int fcn, r;
        std::istringstream iss(fcnstr);
        if(iss >> fcn) {
            r = 1;
        } else {
            r = 0;
        }
        if(r != 1) {
            out << "GLONASS R%02d fcn invalid '%s'\n" << i + 1 << fcnstr << "\n";
            continue;
        }
        if(fcn < -7 || fcn > 6) {
            out << "GLONASS R%02d fcn %d out of range [-7 to 6]\n" << i + 1 << fcn << "\ns";
            continue;
        }
        opt->glofcn[i] = fcn + 8;
    }
}


int BaselineWrapper::get_filetime(const char *file, gtime_t *time, std::ofstream &out)
{
    FILE *fp;
    uint32_t time_time;
    uint8_t buff[64];
    char path[1024], *paths[1], path_tag[1024];

    paths[0] = path;

    if(!expath(file,paths,1)) return 0;

    out << path_tag << path << ".1019stag\n";

    if((fopen_s(&fp, path_tag, "rb")) == 0 && fp) {
        if(fread(buff,64,1,fp) == 1 && !strncmp((char *)buff,"TIMETAG",7) &&
            fread(&time_time,4,1,fp) == 1) {
            time->time = time_time;
            time->sec = 0.0;
            fclose(fp);
            return 1;
        }
        fclose(fp);
    }

    struct stat st;
    if(!stat(path, &st)) {
        struct tm tm;
        if(gmtime_r(&st.st_mtime, &tm)) {
            double ep[6];
            ep[0] = tm.tm_year + 1900;
            ep[1] = tm.tm_mon + 1;
            ep[2] = tm.tm_mday;
            ep[3] = tm.tm_hour;
            ep[4] = tm.tm_min;
            ep[5] = tm.tm_sec;
            *time = utc2gpst(epoch2time(ep));
            return 1;
        }
    }
    return 0;
}

int BaselineWrapper::cmdopts(int argc, char **argv, rnxopt_t *opt, std::string &ifile, std::array<std::string, 9> &ofile, std::string &dir, int &trace, std::ofstream &out)
{
    double eps[] = {1980,1,1,0,0,0}, epe[] = {2037,12,31,0,0,0};
    double epr[] = {2010,1,1,0,0,0};
    double span = 0.0;
    int nf = 6, format = -1;
    char buff[256];
    std::string fmt;

    opt->rnxver = 304;
    opt->obstype = OBSTYPE_PR | OBSTYPE_CP;
    opt->navsys = SYS_GPS | SYS_GLO | SYS_GAL | SYS_QZS | SYS_SBS | SYS_CMP | SYS_IRN;
    opt->ttol = 0.005;

    for(int i = 0 ; i < RNX_NUMSYS ; i++) {
        for(int j = 0 ; j < MAXCODE ; j++) opt->mask[i][j] = '1';
        opt->mask[i][MAXCODE] = '\0';
    }

    for(int i = 1 ; i < argc ; i++) {
        std::string arg = argv[i];

        if(arg == "-ts" && i + 2 < argc) {
            std::string dateStr = argv[++i];
            std::string timeStr = argv[++i];
            std::replace(dateStr.begin(), dateStr.end(), '/', ' ');
            std::istringstream dateStream(dateStr);
            dateStream >> eps[0] >> eps[1] >> eps[2];

            std::replace(timeStr.begin(), timeStr.end(), ':', ' ');
            std::istringstream timeStream(timeStr);
            timeStream >> eps[3] >> eps[4] >> eps[5];

            opt->ts = epoch2time(eps);
        }
        else if(arg == "-te" && i + 2 < argc) {
            std::string dateStr = argv[++i];
            std::string timeStr = argv[++i];
            std::replace(dateStr.begin(), dateStr.end(), '/', ' ');
            std::istringstream dateStream(dateStr);
            dateStream >> epe[0] >> epe[1] >> epe[2];

            std::replace(timeStr.begin(), timeStr.end(), ':', ' ');
            std::istringstream timeStream(timeStr);
            timeStream >> epe[3] >> epe[4] >> epe[5];

            opt->te = epoch2time(epe);
        }
        else if(arg == "-tr" && i + 2 < argc) {
            std::string dateStr = argv[++i];
            std::string timeStr = argv[++i];
            std::replace(dateStr.begin(), dateStr.end(), '/', ' ');
            std::istringstream dateStream(dateStr);
            dateStream >> epr[0] >> epr[1] >> epr[2];

            std::replace(timeStr.begin(), timeStr.end(), ':', ' ');
            std::istringstream timeStream(timeStr);
            timeStream >> epr[3] >> epr[4] >> epr[5];

            opt->trtcm = epoch2time(epr);
        }
        else if(arg == "-ti" && i + 1 < argc) {
            opt->tint = std::stod(argv[++i]);
        }
        else if(arg == "-tt" && i + 1 < argc) {
            opt->ttol = std::stod(argv[++i]);
        }
        else if(arg == "-span" && i + 1 < argc) {
            span = std::stod(argv[++i]);
        }
        else if(arg == "-r" && i + 1 < argc) {
            fmt = argv[++i];
        }
        else if(arg == "-ro" && i + 1 < argc) {
            std::snprintf(opt->rcvopt, sizeof(opt->rcvopt), "%s", argv[++i]);
        }
        else if(arg == "-f" && i + 1 < argc) {
            nf = std::stoi(argv[++i]);
        }
        else if(arg == "-hc" && i + 1 < argc) {
            rnxcomment(opt, argv[++i]);
        }
        else if(arg == "-hm" && i + 1 < argc) {
            std::string s = "";
            while(1) {
                s += argv[++i];
                if(s[s.length()-1] == '~') {
                    s = s.substr(0, s.length()-1);
                    break;
                }
                s += ' ';
            }
            std::snprintf(opt->marker, sizeof(opt->marker), "%s", s.c_str());
        }
        else if(arg == "-hn" && i + 1 < argc) {
            std::snprintf(opt->markerno, sizeof(opt->markerno), "%s", argv[++i]);
        }
        else if(arg == "-ht" && i + 1 < argc) {
            std::snprintf(opt->markertype, sizeof(opt->markertype), "%s", argv[++i]);
        }
        else if(arg == "-ho" && i + 1 < argc) {
            std::snprintf(buff, sizeof(buff), "%s", argv[++i]);
            size_t pos = 0;
            for(int j = 0 ; j < 2 ; j++) {
                size_t next = std::string(buff).find('/', pos);
                std::string token = (next == std::string::npos) ? std::string(buff).substr(pos) : std::string(buff).substr(pos, next - pos);
                std::snprintf(opt->name[j], sizeof(opt->name[j]), "%s", token.c_str());
                if(next == std::string::npos) break;
                pos = next + 1;
            }
        }
        else if(arg == "-hr" && i + 1 < argc) {
            std::snprintf(buff, sizeof(buff), "%s", argv[++i]);
            size_t pos = 0;
            for(int j = 0 ; j < 3 ; j++) {
                size_t next = std::string(buff).find('/', pos);
                std::string token = (next == std::string::npos) ? std::string(buff).substr(pos) : std::string(buff).substr(pos, next - pos);
                std::snprintf(opt->rec[j], sizeof(opt->rec[j]), "%s", token.c_str());
                if(next == std::string::npos) break;
                pos = next + 1;
            }
        }
        else if(arg == "-ha" && i + 1 < argc) {
            std::string s = "";
            while(1) {
                s += argv[++i];
                if(s[s.length()-1] == '~') {
                    s = s.substr(0, s.length()-1);
                    break;
                }
                s += ' ';
            }
            std::snprintf(opt->ant[0], sizeof(opt->ant[0]), "%s", s.c_str());
        }
        else if(arg == "-hp" && i + 1 < argc) {
            std::snprintf(buff, sizeof(buff), "%s", argv[++i]);
            size_t pos = 0;
            for(int j = 0 ; j < 3 ; j++) {
                size_t next = std::string(buff).find('/', pos);
                std::string token = (next == std::string::npos) ? std::string(buff).substr(pos) : std::string(buff).substr(pos, next - pos);
                opt->apppos[j] = std::stod(token);
                if(next == std::string::npos) break;
                pos = next + 1;
            }
        }
        else if(arg == "-hd" && i + 1 < argc) {
            std::snprintf(buff, sizeof(buff), "%s", argv[++i]);
            size_t pos = 0;
            for(int j = 0 ; j < 3 ; j++) {
                size_t next = std::string(buff).find('/', pos);
                std::string token = (next == std::string::npos) ? std::string(buff).substr(pos) : std::string(buff).substr(pos, next - pos);
                opt->antdel[j] = std::stod(token);
                if(next == std::string::npos) break;
                pos = next + 1;
            }
        }
        else if(arg == "-v" && i + 1 < argc) {
            opt->rnxver = static_cast<int>(std::stod(argv[++i]) * 100.0);
        }
        else if(arg == "-od") {
            opt->obstype |= OBSTYPE_DOP;
        }
        else if(arg == "-os") {
            opt->obstype |= OBSTYPE_SNR;
        }
        else if(arg == "-oi") {
            opt->outiono = 1;
        }
        else if(arg == "-ot") {
            opt->outtime = 1;
        }
        else if(arg == "-ol") {
            opt->outleaps = 1;
        }
        else if(arg == "-scan") {
            // obsolete
        }
        else if(arg == "-halfc") {
            opt->halfcyc = 1;
        }
        else if(arg == "-sortsats") {
            opt->sortsats = 1;
        }
        else if(arg == "-sp") {
            opt->sep_nav = 1;
        }
        else if(arg == "-ps") {
            opt->phshift = 1;
        }
        else if(arg == "-mask" && i + 1 < argc) {
            for(int j = 0 ; j < RNX_NUMSYS ; j++) {
                for(int k = 0 ; k < MAXCODE ; k++) opt->mask[j][k] = '0';
                opt->mask[j][MAXCODE] = '\0';
            }
            setmask(argv[++i], opt, 1);
        }
        else if(arg == "-nomask" && i + 1 < argc) {
            setmask(argv[++i], opt, 0);
        }
        else if(arg == "-x" && i + 1 < argc) {
            int sat = satid2no(argv[++i]);
            if(sat) opt->exsats[sat - 1] = 1;
        }
        else if(arg == "-y" && i + 1 < argc) {
            std::string sys = argv[++i];
            if(sys == "G") opt->navsys &= ~SYS_GPS;
            else if(sys == "R") opt->navsys &= ~SYS_GLO;
            else if(sys == "E") opt->navsys &= ~SYS_GAL;
            else if(sys == "J") opt->navsys &= ~SYS_QZS;
            else if(sys == "S") opt->navsys &= ~SYS_SBS;
            else if(sys == "C") opt->navsys &= ~SYS_CMP;
            else if(sys == "I") opt->navsys &= ~SYS_IRN;
        }
        else if(arg == "--glofcn" && i + 1 < argc) {
            setglofcn(argv[++i], opt, out);
        }
        else if(arg == "-d" && i + 1 < argc) {
            dir = argv[++i];
        }
        else if(arg == "-c" && i + 1 < argc) {
            snprintf(opt->staid, sizeof(opt->staid), "%s", argv[++i]);
        }
        else if(arg == "-o" && i + 1 < argc) {
            ofile[0] = argv[++i];
        }
        else if(arg == "-n" && i + 1 < argc) {
            ofile[1] = argv[++i];
        }
        else if(arg == "-g" && i + 1 < argc) {
            ofile[2] = argv[++i];
        }
        else if(arg == "-h" && i + 1 < argc) {
            ofile[3] = argv[++i];
        }
        else if(arg == "-q" && i + 1 < argc) {
            ofile[4] = argv[++i];
        }
        else if(arg == "-l" && i + 1 < argc) {
            ofile[5] = argv[++i];
        }
        else if(arg == "-b" && i + 1 < argc) {
            ofile[6] = argv[++i];
        }
        else if(arg == "-i" && i + 1 < argc) {
            ofile[7] = argv[++i];
        }
        else if(arg == "-s" && i + 1 < argc) {
            ofile[8] = argv[++i];
        }
        else if (arg == "-trace" && i + 1 < argc) {
            trace = std::stoi(argv[++i]);
        }
        else if(arg == "--version") {
            out << "convbin RTKLIB " << VER_RTKLIB << " " << PATCH_LEVEL;
            std::exit(0);
        }
        else if (!arg.empty() && arg[0] == '-') {
            printhelp(out, 1);
        }
        else {
            ifile = arg;
        }
    }

    if(span > 0.0 && opt->ts.time) {
        opt->te = timeadd(opt->ts, span * 3600.0 - 1e-3);
    }
    if(nf >= 1) opt->freqtype |= FREQTYPE_L1;
    if(nf >= 2) opt->freqtype |= FREQTYPE_L2;
    if(nf >= 3) opt->freqtype |= FREQTYPE_L3;
    if(nf >= 4) opt->freqtype |= FREQTYPE_L4;
    if(nf >= 5) opt->freqtype |= FREQTYPE_L5;
    if(nf >= 6) opt->freqtype |= FREQTYPE_L6;
    if(nf >= 7) opt->freqtype |= FREQTYPE_ALL;

    if(opt->trtcm.time == 0) {
        if(opt->ts.time != 0) opt->trtcm = opt->ts;
        else if(opt->te.time != 0) opt->trtcm = opt->te;
        else get_filetime(ifile.c_str(), &opt->trtcm, out);
    }

    if(!fmt.empty()) {
        if(fmt == "rtcm2") format = STRFMT_RTCM2;
        else if(fmt == "rtcm3") format = STRFMT_RTCM3;
        else if(fmt == "nov") format = STRFMT_OEM4;
#ifdef RTK_DISABLED
        else if(fmt == "cnav") format = STRFMT_CNAV;
#endif
        else if(fmt == "ubx") format = STRFMT_UBX;
        else if(fmt == "sbp") format = STRFMT_SBP;
        else if(fmt == "hemis") format = STRFMT_CRES;
        else if(fmt == "stq") format = STRFMT_STQ;
        else if(fmt == "javad") format = STRFMT_JAVAD;
        else if(fmt == "nvs") format = STRFMT_NVS;
        else if(fmt == "binex") format = STRFMT_BINEX;
        else if(fmt == "rt17") format = STRFMT_RT17;
        else if(fmt == "sbf") format = STRFMT_SEPT;
        else if(fmt == "unicore") format = STRFMT_UNICORE;
#ifdef RTK_DISABLED
        else if(fmt == "tersus") format = STRFMT_TERSUS;
#endif
        else if(fmt == "rinex") format = STRFMT_RINEX;
    }
    else {
        std::string path = ifile;
        char* p = nullptr;
        char* cpath = &path[0];
        if(!expath(ifile.c_str(), &cpath, 1) || !(p = strrchr(cpath, '.'))) return -1;

        if(strcmp(p, ".rtcm2") == 0) format = STRFMT_RTCM2;
        else if(strcmp(p, ".rtcm3") == 0) format = STRFMT_RTCM3;
        else if(strcmp(p, ".gps") == 0) format = STRFMT_OEM4;
#ifdef RTK_DISABLED
        else if(strcmp(p, "cnav") == 0) format = STRFMT_CNAV;
#endif
        else if(strcmp(p, ".ubx") == 0) format = STRFMT_UBX;
        else if(strcmp(p, ".sbp") == 0) format = STRFMT_SBP;
        else if(strcmp(p, ".bin") == 0) format = STRFMT_CRES;
        else if(strcmp(p, ".stq") == 0) format = STRFMT_STQ;
        else if(strcmp(p, ".jps") == 0) format = STRFMT_JAVAD;
        else if(strcmp(p, ".bnx") == 0) format = STRFMT_BINEX;
        else if(strcmp(p, ".binex") == 0) format = STRFMT_BINEX;
        else if(strcmp(p, ".rt17") == 0) format = STRFMT_RT17;
        else if(strcmp(p, ".sbf") == 0) format = STRFMT_SEPT;
        else if(strcmp(p, ".unc") == 0) format = STRFMT_UNICORE;
#ifdef RTK_DISABLED
        else if(strcmp(p, ".trs") == 0) format = STRFMT_TERSUS;
#endif
        else if(strcmp(p, ".obs") == 0) format = STRFMT_RINEX;
        else if(strcmp(p + 3, "o") == 0 || strcmp(p + 3, "O") == 0) format = STRFMT_RINEX;
        else if(strcmp(p, ".rnx") == 0) format = STRFMT_RINEX;
        else if(strcmp(p, ".nav") == 0) format = STRFMT_RINEX;
        else if(strcmp(p + 3, "n") == 0 || strcmp(p + 3, "N") == 0) format = STRFMT_RINEX;
    }

    return format;
}

int BaselineWrapper::RtkPost(int argc, char **argv, std::string logFile) {
    std::ofstream out(logFile);
    out << "Welcome to Surveypod Baseline Processing...\n";
    for(int i = 1 ; i < argc ; i++) {
        out << argv[i] << "\n";
    }

    prcopt_t prcopt = prcopt_default;
    solopt_t solopt = solopt_default;
    filopt_t filopt = {""};
    gtime_t ts = {0}, te = {0};
    double tint = 0.0, es[] = {2000, 1, 1, 0, 0, 0}, ee[] = {2000, 12, 31, 23, 59, 59}, pos[3];
    std::string outfile = "", base = "", rover = "";
    std::vector<std::string> infile_strs;
    const char* infile[MAXFILE];
    int n = 0, ret;

    prcopt.mode = PMODE_KINEMA;
    prcopt.navsys = 0;
    prcopt.refpos = POSOPT_SINGLE;
    prcopt.glomodear = GLO_ARMODE_ON;
    solopt.timef = 0;

    out << "Name : " << PROGNAME << "\n";
    out << "RTKLIB Version : " << VER_RTKLIB << "\n";
    out << "Patch Level : " << PATCH_LEVEL << "\n";

    for(int i = 1 ; i < argc ; i++) {
        if(std::string(argv[i]) == "-k" && i+1 < argc) {
            resetsysopts();
            if(!loadopts(argv[++i], sysopts)) return EXIT_FAILURE;
            getsysopts(&prcopt, &solopt, &filopt);
        }
    }

    for(int i = 1 ; i < argc ; i++) {
        std::string arg = argv[i];
        if(arg == "-o" && i+1 < argc) outfile = argv[++i];
        else if(arg == "-ts" && i+2 < argc) {
            std::string dateStr = argv[++i];
            std::string timeStr = argv[++i];
            std::replace(dateStr.begin(), dateStr.end(), '/', ' ');
            std::istringstream dateStream(dateStr);
            dateStream >> es[0] >> es[1] >> es[2];

            std::replace(timeStr.begin(), timeStr.end(), ':', ' ');
            std::istringstream timeStream(timeStr);
            timeStream >> es[3] >> es[4] >> es[5];

            ts = epoch2time(es);
        }
        else if(arg == "-te" && i+2 < argc) {
            std::string dateStr = argv[++i];
            std::string timeStr = argv[++i];
            std::replace(dateStr.begin(), dateStr.end(), '/', ' ');
            std::istringstream dateStream(dateStr);
            dateStream >> ee[0] >> ee[1] >> ee[2];

            std::replace(timeStr.begin(), timeStr.end(), ':', ' ');
            std::istringstream timeStream(timeStr);
            timeStream >> ee[3] >> ee[4] >> ee[5];

            te = epoch2time(ee);
        }
        else if(arg == "-ti" && i+1 < argc) tint = atof(argv[++i]);
        else if(arg == "-k" && i+1 < argc) { ++i; continue; }
        else if(arg == "-p" && i+1 < argc) prcopt.mode = atoi(argv[++i]);
        else if(arg == "-f" && i+1 < argc) prcopt.nf = atoi(argv[++i]);
        else if(arg == "-sys" && i+1 < argc) {
            const char *p;
            for(p = argv[++i] ; *p ; p++) {
                switch(*p) {
                case 'G' : prcopt.navsys |= SYS_GPS;
                    break;
                case 'R' : prcopt.navsys |= SYS_GLO;
                    break;
                case 'E' : prcopt.navsys |= SYS_GAL;
                    break;
                case 'J' : prcopt.navsys |= SYS_QZS;
                    break;
                case 'C' : prcopt.navsys |= SYS_CMP;
                    break;
                case 'I' : prcopt.navsys |= SYS_IRN;
                    break;
                }
            }
            if(!(strchr(p,','))) break;
        }
        else if(arg == "-m" && i+1 < argc) prcopt.elmin = atof(argv[++i])*D2R;
        else if(arg == "-v" && i+1 < argc) prcopt.thresar[0] = atof(argv[++i]);
        else if(arg == "-s" && i+1 < argc) snprintf(solopt.sep, sizeof(solopt.sep), "%s", argv[++i]);
        else if(arg == "-d" && i+1 < argc) solopt.timeu = atoi(argv[++i]);
        else if(arg == "-b") prcopt.soltype = 1;
        else if(arg == "-c") prcopt.soltype = 2;
        else if(arg == "-i") prcopt.modear = 2;
        else if(arg == "-h") prcopt.modear = 3;
        else if(arg == "-t") solopt.timef = 1;
        else if(arg == "-u") solopt.times = TIMES_UTC;
        else if(arg == "-e") solopt.posf = SOLF_XYZ;
        else if(arg == "-a") solopt.posf = SOLF_ENU;
        else if(arg == "-n") solopt.posf = SOLF_NMEA;
        else if(arg == "-g") solopt.degf = 1;
        else if(arg == "-bl" && i+2 < argc) {
            for(int j = 0 ; j < 2 ; j++) prcopt.baseline[j] = atof(argv[++i]);
        }
        else if(arg == "-r" && i+3 < argc) {
            prcopt.refpos = prcopt.rovpos = POSOPT_POS_XYZ;
            for(int j = 0 ; j < 3 ; j++) prcopt.rb[j] = atof(argv[++i]);
            matcpy(prcopt.ru,prcopt.rb,3,1);
        }
        else if(arg == "-l" && i+3 < argc) {
            prcopt.refpos = prcopt.rovpos = POSOPT_POS_LLH;
            for(int j = 0 ; j < 3 ; j++) pos[j] = atof(argv[++i]);
            for(int j = 0 ; j < 2 ; j++) pos[j] *= D2R;
            pos2ecef(pos,prcopt.rb);
            matcpy(prcopt.ru,prcopt.rb,3,1);
        }
        else if(arg == "--rover" && i+1 < argc) rover = argv[++i];
        else if(arg == "--base" && i+1 < argc) base = argv[++i];
        else if(arg == "-y" && i+1 < argc) solopt.sstat = atoi(argv[++i]);
        else if(arg == "-x" && i+1 < argc) solopt.trace = atoi(argv[++i]);
        else if(arg == "--version") {
            out << "rnx2rtkp RTKLIB " << VER_RTKLIB << " " << " " << PATCH_LEVEL << "\n";
            out.flush();
            return 0;
        }
        else if(arg[0] == '-') printhelp(out,0);
        else if(n < MAXFILE) {
            infile_strs.push_back(arg);
            infile[n] = infile_strs[n].c_str();
            n++;
        }
    }

    if(!prcopt.navsys) {
        prcopt.navsys = SYS_GPS|SYS_GLO;
    }
    if(n <= 0) {
        out << "error : no input file";
        out.flush();
        return EXIT_FAILURE;
    }

    if(solopt.trace > 0) {
        if(*filopt.trace == '\0')
            snprintf(filopt.trace, sizeof(filopt.trace), "%s.trace", PROGNAME);
        traceopen(filopt.trace);
        tracelevel(solopt.trace);
    }
    std::snprintf(solopt.prog, sizeof(solopt.prog),"SURVEYPOD 1.0");
    ret = postpos(ts,te,tint,0.0,&prcopt,&solopt,&filopt,infile,n,outfile.c_str(),rover.c_str(),base.c_str());

    if(!ret) out << "Failed to post process data...\n";
    return ret ? EXIT_FAILURE : 0;
}
