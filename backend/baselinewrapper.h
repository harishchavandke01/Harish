#ifndef BASELINEWRAPPER_H
#define BASELINEWRAPPER_H

#include <QObject>
#include <fstream>
#include "../backend/src/rtklib.h"
#include <time.h>

#ifdef _WIN32
#define gmtime_r(timep, result) _gmtime64_s(result, timep)
#endif

#define PRGNAME1   "CONVBIN"
#define TRACEFILE1 "convbin.trace"
#define NOUTFILE        9

#define PROGNAME    "rnx2rtkp"
#define MAXFILE         16

static const char *help1[]={
    "",
    " Synopsis",
    "",
    " convbin [option ...] file",
    "",
    " Description",
    "",
    " Convert RTCM, receiver raw data log and RINEX file to RINEX and SBAS/LEX",
    " message file. SBAS message file complies with RTKLIB SBAS/LEX message",
    " format. It supports the following messages or files:",
    "",
    " RTCM 2                 : Type 1, 3, 9, 14, 16, 17, 18, 19, 22",
    " RTCM 3                 : Type 1002, 1004, 1005, 1006, 1010, 1012, 1019, 1020",
    "                          Type 1071-1127 (MSM except for compact msg)",
    " NovAtel OEMV/4,OEMStar : RANGECMPB, RANGEB, RAWEPHEMB, IONUTCB, RAWWASSFRAMEB",
    " u-blox LEA-4T/5T/6T/8/9: RXM-RAW, RXM-RAWX, RXM-SFRB",
    " Swift Piksi Multi      : ",
    " Hemisphere             : BIN76, BIN80, BIN94, BIN95, BIN96",
    " SkyTraq S1315F         : msg0xDD, msg0xE0, msg0xDC",
    " GW10                   : msg0x08, msg0x03, msg0x27, msg0x20",
    " Javad                  : [R*],[r*],[*R],[*r],[P*],[p*],[*P],[*p],[D*],[*d],",
    "                          [E*],[*E],[F*],[TC],[GE],[NE],[EN],[QE],[UO],[IO],",
    "                          [WD]",
    " NVS                    : BINR",
    " BINEX                  : big-endian, regular CRC, forward record (0xE2)",
    "                          0x01-01,0x01-02,0x01-03,0x01-04,0x01-06,0x7f-05",
    " Trimble                : RT17",
    " Septentrio             : SBF",
    " RINEX                  : OBS, NAV, GNAV, HNAV, LNAV, QNAV",
    "",
    " Options [default]",
    "",
    "     file         input receiver binary log file",
    "     -ts y/m/d h:m:s  start time [all]",
    "     -te y/m/d h:m:s  end time [all]",
    "     -tr y/m/d h:m:s  approximated time for RTCM",
    "     -ti tint     observation data interval (s) [all]",
    "     -tt ttol     observation data epoch tolerance (s) [0.005]",
    "     -span span   time span (h) [all]",
    "     -r format    log format type",
    "                  rtcm2= RTCM 2",
    "                  rtcm3= RTCM 3",
    "                  nov  = NovAtel OEM/4/V/6/7,OEMStar",
    "                  ubx  = ublox LEA-4T/5T/6T/7T/M8T/F9",
    "                  sbp  = Swift Navigation SBP",
    "                  hemis= Hemisphere Eclipse/Crescent",
    "                  stq  = SkyTraq S1315F",
    "                  javad= Javad GREIS",
    "                  nvs  = NVS NV08C BINR",
    "                  binex= BINEX",
    "                  rt17 = Trimble RT17",
    "                  sbf  = Septentrio SBF",
    "                  unicore = Unicore binary data output",
    "                  rinex= RINEX",
    "     -ro opt      receiver options",
    "     -f freq      number of frequencies [all]",
    "     -hc comment  rinex header: comment line",
    "     -hm marker   rinex header: marker name",
    "     -hn markno   rinex header: marker number",
    "     -ht marktype rinex header: marker type",
    "     -ho observ   rinex header: observer name and agency separated by /",
    "     -hr rec      rinex header: receiver number, type and version separated by /",
    "     -ha ant      rinex header: antenna number and type separated by /",
    "     -hp pos      rinex header: approx position x/y/z separated by /",
    "     -hd delta    rinex header: antenna delta h/e/n separated by /",
    "     -v ver       rinex version [3.04]",
    "     -od          include doppler frequency in rinex obs [off]",
    "     -os          include snr in rinex obs [off]",
    "     -oi          include iono correction in rinex nav header [off]",
    "     -ot          include time correction in rinex nav header [off]",
    "     -ol          include leap seconds in rinex nav header [off]",
    "     -halfc       half-cycle ambiguity correction [off]",
    "     -sortsats    sort observations by the RTKLib satellite index [off]",
    "     -mask   [sig[,...]] signal mask(s) (sig={G|R|E|J|S|C|I}L{1C|1P|1W|...})",
    "     -nomask [sig[,...]] signal no mask (same as above)",
    "     -x sat       exclude satellite",
    "     -y sys       exclude systems (G:GPS,R:GLO,E:GAL,J:QZS,S:SBS,C:BDS,I:IRN)",
    "     --glofcn [-7 to 6][,...]] GLONASS fcn for R01 to R32",
    "     -d dir       output directory [same as input file]",
    "     -c staid     use RINEX file name convention with staid [off]",
    "     -o ofile     output RINEX OBS file",
    "     -n nfile     output RINEX NAV file",
    "     -g gfile     output RINEX GNAV file",
    "     -h hfile     output RINEX HNAV file",
    "     -q qfile     output RINEX QNAV file",
    "     -l lfile     output RINEX LNAV file",
    "     -b cfile     output RINEX CNAV file",
    "     -i ifile     output RINEX INAV file",
    "     -s sfile     output SBAS message file",
    "     -trace level output trace level [off]",
    "",
    " If any output file specified, default output files (<file>.obs,",
    " <file>.nav, <file>.gnav, <file>.hnav, <file>.qnav, <file>.lnav,",
    " <file>.cnav, <file>.inav and <file>.sbs) are used. To obtain week number info",
    " for RTCM file, use -tr option to specify the approximated log start time.",
    " Without -tr option, the program obtains the week number from the time-tag file",
    " (if it exists) or the last modified time of the log file instead.",
    "",
    " If receiver type is not specified, type is recognized by the input",
    " file extension as follows.",
    "     *.rtcm2       RTCM 2",
    "     *.rtcm3       RTCM 3",
    "     *.gps         NovAtel OEM4/V/6/7,OEMStar",
    "     *.ubx         u-blox LEA-4T/5T/6T/7T/M8T/F9",
    "     *.sbp         Swift Navigation SBP",
    "     *.bin         Hemisphere Eclipse/Crescent",
    "     *.stq         SkyTraq S1315F",
    "     *.jps         Javad GREIS",
    "     *.bnx,*binex  BINEX",
    "     *.rt17        Trimble RT17",
    "     *.sbf         Septentrio SBF",
    "     *.unc         Unicore binary data output",
    "     *.obs,*.*o    RINEX OBS",
    "     *.rnx         RINEX OBS",
    "     *.nav,*.*n    RINEX NAV",
    };

static const char *help2[]={
    "",
    " usage: rnx2rtkp [option]... file file [...]",
    "",
    " Read RINEX OBS/NAV/GNAV/HNAV/CLK, SP3, SBAS message log files and compute ",
    " receiver (rover) positions and output position solutions.",
    " The first RINEX OBS file shall contain receiver (rover) observations. For the",
    " relative mode, the second RINEX OBS file shall contain reference",
    " (base station) receiver observations. At least one RINEX NAV/GNAV/HNAV",
    " file shall be included in input files. To use SP3 precise ephemeris, specify",
    " the path in the files. The extension of the SP3 file shall be .sp3 or .eph.",
    " All of the input file paths can include wild-cards (*). To avoid command",
    " line deployment of wild-cards, use \"...\" for paths with wild-cards.",
    " Command line options are as follows ([]:default). A maximum number of",
    " input files is currently set to 16. With -k option, the",
    " processing options are input from the configuration file. In this case,",
    " command line options precede options in the configuration file.",
    "",
    " -?        print help",
    " -k file   input options from configuration file [off]",
    " -o file   set output file [stdout]",
    " -ts ds ts start day/time (ds=y/m/d ts=h:m:s) [obs start time]",
    " -te de te end day/time   (de=y/m/d te=h:m:s) [obs end time]",
    " -ti tint  time interval (sec) [all]",
    " -p mode   mode (0:single,1:dgps,2:kinematic,3:static,4:static-start,",
    "                 5:moving-base,6:fixed,7:ppp-kinematic,8:ppp-static,9:ppp-fixed) [2]",
    " -m mask   elevation mask angle (deg) [15]",
    " -sys s[,s...] nav system(s) (s=G:GPS,R:GLO,E:GAL,J:QZS,C:BDS,I:IRN) [G|R]",
    " -f freq   number of frequencies for relative mode (1:L1,2:L1+L2,3:L1+L2+L5) [2]",
    " -v thres  validation threshold for integer ambiguity (0.0:no AR) [3.0]",
    " -b        backward solutions [off]",
    " -c        forward/backward combined solutions [off]",
    " -i        instantaneous integer ambiguity resolution [off]",
    " -h        fix and hold for integer ambiguity resolution [off]",
    " -bl bl,std     baseline distance and stdev",
    " -e        output x/y/z-ecef position [latitude/longitude/height]",
    " -a        output e/n/u-baseline [latitude/longitude/height]",
    " -n        output NMEA-0183 GGA sentence [off]",
    " -g        output latitude/longitude in the form of ddd mm ss.ss' [ddd.ddd]",
    " -t        output time in the form of yyyy/mm/dd hh:mm:ss.ss [sssss.ss]",
    " -u        output time in utc [gpst]",
    " -d col    number of decimals in time [3]",
    " -s sep    field separator [' ']",
    " -r x y z  reference (base) receiver ecef pos (m) [average of single pos]",
    "           rover receiver ecef pos (m) for fixed or ppp-fixed mode",
    " -l lat lon hgt reference (base) receiver latitude/longitude/height (deg/m)",
    "           rover latitude/longitude/height for fixed or ppp-fixed mode",
    " -y level  output solution status (0:off,1:states,2:residuals) [0]",
    " -x level  debug trace level (0:off) [0]",
    " --rover list rover names for processing, separated by a space",
    " --base list  base names for processing, separated by a space",
    " --version display release version",
};



class BaselineWrapper : public QObject
{
    Q_OBJECT
public:
    explicit BaselineWrapper(QObject *parent = nullptr);
    void printhelp(std::ofstream &out, int x);
    int convbin(int format, rnxopt_t *opt, const std::string& ifile,
                std::array<std::string, NOUTFILE>& file, const std::string& dir, std::ofstream &out);
    int ConvertData(int argc, char **argv, std::string logFile);
    void setmask(const char *argv, rnxopt_t *opt, int mask);
    int get_filetime(const char *file, gtime_t *time, std::ofstream &out);
    int cmdopts(int argc, char** argv, rnxopt_t* opt, std::string &ifile, std::array<std::string,
                                                                                     NOUTFILE> &ofile, std::string &dir, int &trace, std::ofstream &out);

    int RtkPost(int argc, char **argv, std::string logFile);
signals:
};

#endif // BASELINEWRAPPER_H
