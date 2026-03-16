#ifndef RINEXCONVERSION_H
#define RINEXCONVERSION_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "../../Utils/customcheckbox.h"
#include "signalsmask.h"

class RinexConversion : public QWidget
{
    Q_OBJECT
public:
    explicit RinexConversion(QWidget *parent = nullptr);
    void setProjectFolder(const QString &_projectFolder);
    void setOp(QString dir);
    void setOption(bool val = true);
    void saveOption();
    std::vector<bool> setMask();

private:
    QString projectFolder;

    QWidget *filesInput;
    QWidget *convertOptions;
    QWidget *mainWidget;

    QLabel * rinexConversionLabel;

    QLabel* inputUbxLabel;
    QLineEdit * ubxInputFileLE;
    QPushButton *ubxPickBtn;

    QLabel *inputDirLabel;
    QLineEdit *outputDirLE;
    QPushButton *outputDirPickBtn;

    QLabel *rinexObsLabel;
    QLineEdit* obsPathLE;

    QLabel *rinexNavLabel;
    QLineEdit *navPathLE;

    QLabel * formatLabel;
    QComboBox *formatCB;

    QPushButton * convertBtn;


    void setUpFilesLayout();
    void setUpOptionsLayout();


private slots:
    void onUbxClicked();
    void onOutputDirClicked();
    void onConvertBtnClicked();

private:

    SignalsMask *signalmask = NULL;

    QLabel *lab1, *lab2, *lab3, *lab4, *lab5, *lab6;
    QLineEdit *outputDir, *mname, *antname, *posx, *posy, *posz, *poleh, *polee, *polen, *exsat;
    QPushButton *selectDir;
    CustomCheckBox *round;

    QLabel *heading;
    QWidget *contentWidget;

    CustomCheckBox *tscheckbox, *techeckbox, *ticheckbox, *gps, *glonass, *galileo, *qzss, *beidou, *sbs, *navic;
    CustomCheckBox *sepnav, *phase, *halfc, *sort, *iono, *timec, *leap;
    CustomCheckBox *L1_G1_E1_B1, *L2_G2_E5b_B2b, *L5_G3_E5a_B2a, *L6_E6_B3, *E5ab_B1C, *B2ab;
    QDateTimeEdit *ts, *te;
    QComboBox *ti, *version, *format, *receiver;
    QPushButton *savebtn, *resetbtn, *mask;

signals:
};

#endif // RINEXCONVERSION_H
