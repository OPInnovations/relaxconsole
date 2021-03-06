#include "sleepanalysiswindow.h"
#include "ui_sleepanalysiswindow.h"
#include "converter/convertwindow.h"
#include "converter/convertoptionswindow.h"

/***
  * Constructor
  */
SleepAnalysisWindow::SleepAnalysisWindow(QWidget *parent) :
   // QMainWindow(parent),
    ui(new Ui::SleepAnalysisWindow)
{
    ui->setupUi(this);
    //set screen size
    this->setMinimumSize(this->width(),this->height());
    this->setMaximumSize(this->width(),this->height());
    //set window icon
    this->setWindowIcon(QIcon("../images/opi_ico.ico"));
    Sigma = Sigma_Default;
    alpha = alpha_Default;
    TDVp = new twoDaccelviewer(false,0,this);
}


/***
  * Destructor
  */
SleepAnalysisWindow::~SleepAnalysisWindow()
{
    delete ui;
    delete TDVp;
}


/***
  * When the input file Browse button is clicked, allow user to choose file
  */
void SleepAnalysisWindow::on_inBrwsPB_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath(),
                       "Supported Files( *.edf  *.opi ) ;; EDF File( *.edf ) ;; OPI File( *.opi ) ;; All Files( *.* )");

    if (!fileName.isEmpty())
    {
        ui->inLE->setText(fileName);
    }
}


/***
  * When the convert button is clicked, disable all inputs and try to convert and then
  * reenable inputs
  */
void SleepAnalysisWindow::on_cnvtPB_clicked()
{
    qint32 inType;

    // Disable inputs
    ui->cnvtPB->setEnabled(false);
    ui->inBrwsPB->setEnabled(false);
    ui->inLE->setEnabled(false);
    ui->outTypCBox->setEnabled(false);
    qApp->processEvents();      // make sure display gets updated

    // first determine type of input file
    inType = ConvertWindow().whatInType(ui->inLE->text());

    if(inType < 0)  // input file error
    {
        ui->msgPTE->appendPlainText(QString("Problem with input file %1").arg(ui->inLE->text()));
    }
    else if(inType != 2)  // unrecognized file
    {
        ui->msgPTE->appendPlainText("Unrecognized input file type");
    }
    else if(inType == 2) // generic edf opi
    {
        ui->msgPTE->appendPlainText("Analyzing EDF+... ");
        qApp->processEvents();      // make sure display gets updated
        sleepanalyzeD();
    }

    // Enable inputs
    ui->cnvtPB->setEnabled(true);
    ui->inBrwsPB->setEnabled(true);
    ui->inLE->setEnabled(true);
    ui->outTypCBox->setEnabled(true);
}


void SleepAnalysisWindow::sleepanalyzeD()
{
    QString outFileName, outDirName;
    QFile *infilep, *outfile1p, *outfileTp;
    QDataStream *instrp;
    QDataStream *out1strp;
    QTextStream *outTstrp;
    QString datetimestr;
    QDateTime firstFrmDT, stDT, endDT;
    quint8 pdnListp[PDNLISTLEN];
    QVector<qint64> *tsQVpp[PDNLISTLEN];
    QVector<quint8> *skpQVpp[PDNLISTLEN];
    QVector<quint8> *batQVpp[PDNLISTLEN];
    QVector<qint16> *adcQVpp[PDNLISTLEN];
    QVector<qint16> *axQVpp[PDNLISTLEN];
    QVector<qint16> *ayQVpp[PDNLISTLEN];
    QVector<qint16> *azQVpp[PDNLISTLEN];
    QVector<qint16> *tmpQVpp[PDNLISTLEN];
    QVector<qint16> *sqQVpp[PDNLISTLEN]; //Activity
    QVector<quint8> *edQVpp[PDNLISTLEN];
    QVector<qint64> annOnsetTSQV;
    QVector<QString> annTextQV;
    QVector<qint16> sleephypQV;
    QVector<qint16> eegM2QV;
    QVector<qint16> eegM1QV;
    QVector<qint16> eegG2QV;
    QVector<qint16> eegG1QV;
    QVector<qint16> eegUPQV;
    QVector<qint16> eegBetaQV;
    QVector<qint16> eegSigmaQV;
    QVector<qint16> eegalphaQV;
    QVector<qint16> eegthetaQV;
    QVector<qint16> eegdeltaQV;
    ConvertOptionsWindow *coWinp;
    QString localPatientID, lpid, lrid;
    QDateTime startDT;
    qint32 numDataRecs, dataRecDur;
    qint32 numSignals;
    QVector<QString> labelSignalsQV, transTypeQV, physDimQV, prefiltQV;
    QVector<qint32> physMinQV, physMaxQV, digMinQV, digMaxQV;
    QVector<qint32> sampsPerDRQV;
    QString metaAnalysisText;
    qint32 pdnSlot;
    qint32 pdnDes;
    qint8 outDataType;
    qint32 dataRecordCt;
    bool useProfile;
    QString readName, readDOB, readSex;
    QMessageBox msgBox;
    qint32 i, j, k, sizeAct, sizeEEG, sizeAx, sizeAz, DGavecount;
    double DeltaTH2, DeltaTH1, SigmaTH, ThetaTH, BetaTH, G1TH, M2BTH, M2THA, M2THB, ActTH, PosTH; //preset threshold levels
    double DeltaV, ThetaV, SigmaV, BetaV, G1V, M2V, ActV, AzV, PosV; //for values
    double DGratio, TGratio, SGratio, GMratio, MBratio; //for subtraction in db
    double  DGave,DGmean,TGmean,SGmean,GMmean,MBmean; //for mean value in db within epoch, DGave is slow average of previous epochs
    qint8  HypW, HypN1, HypN2, HypN3, HypR, HypP, HypM2; //preset values for HYP output
    qint8  Hyplast, Hypnow, Hypnext; //HYP score
    qint32 epochsec,meanfactor, Wcount,N1count,N2count,N3count,N4count,Rcount,Dcount,M2count,Pcount,Actcount,Axcount; //length of epoch in sec
    qint32 epochW,epochR,epochN1,epochN2,epochN3,epochM2,epochP,epochTotal,epochSleep, epochStart; //calc statistics
    bool HyplastF; //flag for incorrect posture or motion
    QStringList tempQSL;
    QString stDateQS;
    qint32 currMonth;
    bool doTxtFile; // flag for writing out text files: summary and epoch data
    QVector<qint8> epochScoreQVs;  // for storing the epoch scores for writing to text
    QString readSleep_DeltaTH2, readSleep_DeltaTH1, readSleep_ThetaTH, readSleep_SigmaTH, readSleep_BetaTH, readSleep_G1TH;

    epochsec = 30;   // 30sec
    meanfactor=15;   // <epochsec / adjustment factor
/*  default values set in .cfg file in main.cpp
    DeltaTH2 = 10.5; //10.5db for N3 detect 2 spikes
    DeltaTH1 =  8.0; // 8.0db for(backup) N2 detect 2 spikes
    ThetaTH  = 10;   // 10db for N1 detect multi spikes
    SigmaTH  =  7;   //  7db for N2 detect 2 spikes
    BetaTH   = 10;   // reserved ***
    G1TH     =  4.5;   // 4.5db for W  detect multi spikes
*/
    M2BTH    =  5;   //  4db M2>B for motion detect
    M2THA    = 10;   // 10db  motion single spike
    M2THB    = 15;   // 15db big motion single spike
    ActTH    = 10*655.34;   //  8db for Activity threshold low(5)-mid(10)walk
    PosTH    = 0.6*16383.5; // >0.6G threshold for horizontal posture
    HypW     = 6;
    HypR     = 5;
    HypN1    = 4;
    HypN2    = 3;
    HypN3    = 2;
    HypM2    = 1; //blocked by physical and muscle activity
    HypP     = 0; //blocked if not horizontal posture

    // Opening of inputs/outputs and Error Checking
    infilep = new QFile(ui->inLE->text());
    if (!infilep->open(QIODevice::ReadOnly))
    {
        ui->msgPTE->appendPlainText(QString("Problem with input file \"%1\"").arg(ui->inLE->text()));
        delete infilep;
        return;
    }

    // open input file stream
    instrp = new QDataStream(infilep);
    instrp->setByteOrder(QDataStream::LittleEndian);

    // get header information
    ConvertWindow().edfhdrread(instrp, &lpid, &lrid, &startDT, &numDataRecs, &dataRecDur,
               &numSignals, &labelSignalsQV, &transTypeQV, &physDimQV, &physMinQV,
               &physMaxQV, &digMinQV, &digMaxQV, &prefiltQV, &sampsPerDRQV);
    dataRecordCt = numDataRecs;

    // init qvector pointers to null
    for(pdnSlot = 0; pdnSlot < PDNLISTLEN; pdnSlot++)
    {
        tsQVpp[pdnSlot] = 0;
        skpQVpp[pdnSlot] = 0;
        batQVpp[pdnSlot] = 0;
        adcQVpp[pdnSlot] = 0;
        tmpQVpp[pdnSlot] = 0;
        axQVpp[pdnSlot] = 0;
        ayQVpp[pdnSlot] = 0;
        azQVpp[pdnSlot] = 0;
        sqQVpp[pdnSlot] = 0;
        edQVpp[pdnSlot] = 0;
    }

    // get PDN from Device field
    pdnDes = getPDNlrid(lrid);

    pdnSlot = 0;

    // copy pdn's
    for(i = 0; i < PDNLISTLEN; i++)
        pdnListp[i] = 0xFF;
    pdnListp[pdnSlot] = pdnDes;

    // initialize qvectors
    tsQVpp[pdnSlot] = new QVector<qint64>(0);
    skpQVpp[pdnSlot] = new QVector<quint8>(0);
    batQVpp[pdnSlot] = new QVector<quint8>(0);
    adcQVpp[pdnSlot] = new QVector<qint16>(0);
    tmpQVpp[pdnSlot] = new QVector<qint16>(0);
    axQVpp[pdnSlot] = new QVector<qint16>(0);
    ayQVpp[pdnSlot] = new QVector<qint16>(0);
    azQVpp[pdnSlot] = new QVector<qint16>(0);
    sqQVpp[pdnSlot] = new QVector<qint16>(0);
    edQVpp[pdnSlot] = new QVector<quint8>(0);

    dataRecordCt = ConvertWindow().edfDread(instrp, startDT, numDataRecs, dataRecDur, numSignals,
                                            sampsPerDRQV, tsQVpp[pdnSlot], skpQVpp[pdnSlot], batQVpp[pdnSlot],
                                            adcQVpp[pdnSlot], tmpQVpp[pdnSlot], axQVpp[pdnSlot], ayQVpp[pdnSlot],
                                            azQVpp[pdnSlot], sqQVpp[pdnSlot], edQVpp[pdnSlot],
                                            &annOnsetTSQV, &annTextQV);
    if(dataRecordCt < 0)
    {
        ConvertWindow().delspecQVs(pdnSlot, tsQVpp, skpQVpp, batQVpp, adcQVpp, tmpQVpp, axQVpp, ayQVpp, azQVpp, sqQVpp, edQVpp);
    }

    if(dataRecordCt != numDataRecs)
    {
        msgBox.setText("your file has crashed");
        msgBox.exec();
        return;
    }
    // At this point, input file is not needed anymore
    infilep->close();
    delete infilep;
    delete instrp;

    if(dataRecordCt<1)
    {   ui->msgPTE->appendPlainText("error:the duration < 1 ");
        return;   // user terminates so don't do anything else
    }

    // check that there is actually usable data, if not exit
    if((!tsQVpp[0]) && (!tsQVpp[1]) && (!tsQVpp[2]) && (!tsQVpp[3]) &&
            (!tsQVpp[4]) && (!tsQVpp[5]) && (!tsQVpp[6]) && (!tsQVpp[7]))
    {
        ui->msgPTE->appendPlainText("No usable data in files... ");
        return;   // user terminates so don't do anything else
    }

    // go to converter options, once it returns, data has been processed and only needs
    // to be written to file
    coWinp = new ConvertOptionsWindow(&outDirName, pdnListp, &outDataType, &alpha, &Sigma, tsQVpp, skpQVpp,
                                      batQVpp, adcQVpp, tmpQVpp, axQVpp, ayQVpp, azQVpp,
                                      sqQVpp, edQVpp, &annOnsetTSQV, &annTextQV ,EDFDTOEDF, &useProfile, this, &doTxtFile);
    coWinp->sleepanalysis_uiconfig();   // configure ui for sleepanalysis

    if(coWinp->exec() == QDialog::Rejected)
    {
        ui->msgPTE->appendPlainText("User aborted... ");
        ConvertWindow().delQVs(tsQVpp, skpQVpp, batQVpp, adcQVpp, tmpQVpp, axQVpp, ayQVpp, azQVpp, sqQVpp, edQVpp);
        delete coWinp;
        return;   // user terminates so don't do anything else
    }
    delete coWinp;

    // get variables from config file, use if not empty
    readSleep_DeltaTH2 = getConfigValue("Sleep_DeltaTH2");
    readSleep_DeltaTH1 = getConfigValue("Sleep_DeltaTH1");
    readSleep_ThetaTH = getConfigValue("Sleep_ThetaTH");
    readSleep_SigmaTH = getConfigValue("Sleep_SigmaTH");
    readSleep_BetaTH = getConfigValue("Sleep_BetaTH");
    readSleep_G1TH = getConfigValue("Sleep_G1TH");
    if(!readSleep_DeltaTH2.isEmpty())
    {
        DeltaTH2 = readSleep_DeltaTH2.toDouble();
        DeltaTH1 = readSleep_DeltaTH1.toDouble();
        ThetaTH = readSleep_ThetaTH.toDouble();
        SigmaTH = readSleep_SigmaTH.toDouble();
        BetaTH = readSleep_BetaTH.toDouble();
        G1TH = readSleep_G1TH.toDouble();
    }
    // convert to EEG, 1 value per second, data is already in log according to
    // fftDoubleQInt16Conversion
    ConvertWindow().DtoMeegQVs(adcQVpp[pdnSlot],
                               &eegM2QV,&eegM1QV,&eegG2QV,&eegG1QV,
                               &eegUPQV,&eegBetaQV,
                               &eegSigmaQV,&eegalphaQV,
                               &eegthetaQV,&eegdeltaQV);

    stDT = QDateTime::currentDateTime();    // time how long this takes
    // data processing, algorithm
    // *** start ***
    sizeEEG = eegM2QV.size();
    sizeAct = sqQVpp[pdnSlot]->size();
    sizeAx  = axQVpp[pdnSlot]->size();
    sizeAz  = azQVpp[pdnSlot]->size();
    sleephypQV.resize(sizeEEG);
    Hyplast = HypW;
    Hypnow  = HypW;
    Hypnext = HypW;
    epochW=0;
    epochR=0;
    epochN1=0;
    epochN2=0;
    epochN3=0;
    epochM2=0;
    epochP=0;
    epochTotal=0;
    epochSleep=0;
    Wcount = 0;
    N1count= 0;
    N2count= 0;
    N3count= 0;
    N4count= 0;
    Rcount = 0;
    Dcount = 0;
    M2count= 0;
    Pcount = 0;
    Actcount=0;
    ActV  = 0;
    j=0;
    k=0;
    DGavecount=0; //init, count to 240 = 4min limit
    DGave=DeltaTH1; //initialized to DeltaTH1 in db
    DGmean=0;
    TGmean=0;
    SGmean=0;
    GMmean=0;
    MBmean=0;
    for(i = 0; i < sizeEEG; i++)  //main scoring loop
    {
        AzV = 0;
        PosV = 0;
        for(k=0; k<4; k++) //get Activity
        {
            Axcount = (i*4+k); //
            if(Axcount<sizeAct)
                if(sqQVpp[pdnSlot]->at(Axcount)>ActTH) Actcount++; //4Hz ax ay vs 1Hz EEG
            if((Axcount*4+3)<sizeAz && Axcount<sizeAx)
            {
                AzV+=(azQVpp[pdnSlot]->at(Axcount*4));
                AzV+=(azQVpp[pdnSlot]->at(Axcount*4+1));
                AzV+=(azQVpp[pdnSlot]->at(Axcount*4+2));
                AzV+=(azQVpp[pdnSlot]->at(Axcount*4+3)); //16Hz az vs 1Hz EEG
                PosV+=(axQVpp[pdnSlot]->at(Axcount));
                PosV+=(ayQVpp[pdnSlot]->at(Axcount));
            }
        }
        if(PosV<0) PosV=-1*PosV; //absolute value
        PosV = PosV/4; //average of 4
        PosV = PosV*PosV/2; //half of square
        AzV  = AzV/16; //average of 16
        AzV  = AzV*AzV; //square
        PosV += AzV; //sum of squares
        PosV = sqrt(PosV); //sqrt = radius
        DeltaV = (double) eegdeltaQV.at(i)/327.67; //to db
        ThetaV = (double) eegthetaQV.at(i)/327.67; //to db
        SigmaV = (double) eegSigmaQV.at(i)/327.67; //to db
        BetaV =  (double) eegBetaQV.at(i)/327.67; //to db
        G1V   =  (double) eegG1QV.at(i)/327.67; //to db
        M2V   =  (double) eegM2QV.at(i)/327.67; //to db
        DGratio = DeltaV-G1V-DGave; //db: self reference to moving average
        TGratio = ThetaV-G1V; //db
        SGratio = SigmaV-G1V; //db
        GMratio = G1V-M2V; //db
        MBratio = M2V-BetaV; //db
        DGmean += pow(10,DGratio/10)/meanfactor; //power
        TGmean += pow(10,TGratio/10)/meanfactor; //power
        SGmean += pow(10,SGratio/10)/meanfactor; //power
        GMmean += pow(10,GMratio/10)/meanfactor; //power
        MBmean += pow(10,MBratio/10)/meanfactor; //power

        if(PosV<PosTH) Pcount++; //check posture
        else if((MBratio>(M2BTH) && M2V>M2THA) || M2V>M2THB) M2count++; //check absolute and relative values
        else
        {
            DGavecount++;
            if(DGavecount<=240) DGave = DGave*0.995+(DeltaV-G1V)*0.005; //db: 0.005 very slow varying self reference
            if(DGratio<DeltaTH1 && SGratio<SigmaTH && GMratio>G1TH) Wcount++; //
            if(DGratio>DeltaTH2) N3count++;
            if(DGratio>DeltaTH1) N2count++; //duplicate count of N3
            else if(SGratio>SigmaTH) N2count++; //no duplicate
            else if(TGratio>ThetaTH) N1count++; //no duplicate
            else Dcount++; //dummy
        }
        j++;
        if(j>=epochsec)  //vote within epochsec
        {
            DGmean=log10(DGmean)*10; //db
            TGmean=log10(TGmean)*10; //db
            SGmean=log10(SGmean)*10; //db
            GMmean=log10(GMmean)*10; //db
            MBmean=log10(MBmean)*10; //db

            if(Pcount>=epochsec/6) Hypnow=HypP; //invalid epoch: posture incorrect
            else if(M2count>=2 || Actcount>=3) Hypnow=HypM2; //invalid epoch: motion
            else if(Wcount>=epochsec/4 || (DGmean<DeltaTH1 && SGmean<SigmaTH && GMmean>G1TH))
                Hypnow=HypW;
            else if(N3count>=2 || DGmean>DeltaTH2) Hypnow=HypN3;
            else if(N2count>=2 || DGmean>DeltaTH1 || SGmean>SigmaTH) Hypnow=HypN2;
            else if(N1count>=epochsec/6 || TGmean>ThetaTH)
            {
                if(Hyplast==HypW || Hyplast==HypN1) Hypnow=HypN1;
                else Hypnow=Hyplast; //usually noise
            }
            else if(Wcount>=epochsec/6)
            {
                if(Hyplast==HypW || HyplastF==true) Hypnow=HypW; //continued wake
                else Hypnow=Hyplast;
            }
            else if(Hyplast!=HypW && Hyplast!=HypN1 && HyplastF==false) Hypnow=HypR;
            else Hypnow=Hyplast; //no feature => Hyplast

            if((epochSleep-epochM2)<20 && Hypnow==HypR) Hypnow=HypW; //initial 20 valid sleep epochs: W (not R)

            //start statistics
            epochTotal++;
            if(Hypnow==HypP) epochP++;
            else //exclude HypP
            {
                epochSleep++; //count horizontal time
                if(epochSleep==2) epochStart=epochTotal-2; //Lie Down start offest
                if(Hypnow==HypM2) epochM2++;
                if(Hypnow==HypW) epochW++;
                if(Hypnow==HypN1) epochN1++;
                if(Hypnow==HypN2) epochN2++;
                if(Hypnow==HypN3) epochN3++;
                if(Hypnow==HypR) epochR++;
            }
            epochScoreQVs.append(Hypnow);   // save for later
            j=0; //reset epoch
            Wcount = 0;
            N1count= 0;
            N2count= 0;
            N3count= 0;
            N4count= 0;
            Rcount = 0;
            Dcount = 0;
            M2count= 0;
            Pcount = 0;
            Actcount=0; //reset count
            DGmean=0;
            TGmean=0;
            SGmean=0;
            GMmean=0;
            MBmean=0;
            if(Hypnow!=HypM2 && Hypnow!=HypP)
            {
                Hyplast=Hypnow; //update Hyplast
                HyplastF=false;
            }
            else HyplastF=true; //set flag for possible wake
        } //end epoch processing

        if((i+1-epochsec)>=0 &&(i+1-epochsec)<sizeEEG) sleephypQV[i-epochsec+1] = Hypnow;  // write score
    }
//    qDebug() << "Sleep" << epochSleep <<"R" << epochR << "N1" << epochN1 <<"N2"<<epochN2<< "N3" << epochN3;
//    qDebug() << "Total" << epochTotal <<"W"<< epochW << "P" << epochP << "M2"<< epochM2;

    // *** end ***

    // posture viewer
    TDVp->setData(axQVpp[pdnSlot], ayQVpp[pdnSlot], azQVpp[pdnSlot], tsQVpp[pdnSlot],
                   tsQVpp[pdnSlot]->at(0), tsQVpp[pdnSlot]->at(tsQVpp[pdnSlot]->size()-1));
    TDVp->setPDN(pdnDes);
    TDVp->setVisible(false);    // have to do this to make drawing faster
    TDVp->setVisible(true);

    // Conversion to QDateTime, ref date & time for all sensors is 2012/sep/28 08:00:00.000
    firstFrmDT = QDateTime::fromString("20120928080000000","yyyyMMddhhmmsszzz").addMSecs((tsQVpp[pdnSlot]->at(0) % ((qint64) 1 << 47))*1000/UCERTCFREQ);
    datetimestr = firstFrmDT.toString("yyyyMMdd_hhmmss");

    if(epochSleep!=0)
    {
        metaAnalysisText.append(QString("<Sleep> (not for medical use)\n"));
        metaAnalysisText.append(QString("Total time: %1min\n").arg(epochTotal*epochsec/60));
        metaAnalysisText.append(QString("Lie Down: %1min\n").arg(epochSleep*epochsec/60));
        metaAnalysisText.append(QString("Lie Down Start: %1\n").arg(firstFrmDT.addSecs(epochStart*epochsec).toString("yyyyMMdd_hhmmss")));
        metaAnalysisText.append(QString("Sleep time: %1min %2\%\n").arg((epochR+epochN1+epochN2+epochN3)*epochsec/60).arg((epochR+epochN1+epochN2+epochN3)*100/epochSleep));
        metaAnalysisText.append(QString("Wake: %1min %2\%\n").arg((epochW)*epochsec/60).arg((epochW)*100/epochSleep));
        metaAnalysisText.append(QString("REM: %1min %2\%\n").arg(epochR*epochsec/60).arg(epochR*100/epochSleep));
        metaAnalysisText.append(QString("S1: %1min %2\%\n").arg(epochN1*epochsec/60).arg(epochN1*100/epochSleep));
        metaAnalysisText.append(QString("S2: %1min %2\%\n").arg(epochN2*epochsec/60).arg(epochN2*100/epochSleep));
        metaAnalysisText.append(QString("S3: %1min %2\%\n").arg(epochN3*epochsec/60).arg(epochN3*100/epochSleep));
        metaAnalysisText.append(QString("Motion: %1min %2\%\n").arg((epochM2)*epochsec/60).arg((epochM2)*100/epochSleep));
        TDVp->showMetaAnalysisText(&metaAnalysisText);
    }

    // Output file stuff

    // Text files if needed
    if(doTxtFile)
    {
        // summary
        outFileName = outDirName;
        outFileName.append(QString("E%1_%2_hyp_sum.txt").arg(datetimestr).arg(pdnDes));
        outfileTp = new QFile(outFileName);
        if(outfileTp->exists())
        {
            msgBox.setWindowTitle("Output Text File exists");
            msgBox.setText(QString("Out file %1 exists in selected directory.").arg(QString("E%1_%2_hyp_summary.txt").arg(datetimestr).arg(pdnDes)));
            msgBox.setInformativeText("Do you want to overwrite?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            if(msgBox.exec() == QMessageBox::Cancel)
            {
                delete outfileTp;
                return;
            }
        }
        if(!outfileTp->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            ui->msgPTE->appendPlainText(QString("Problem with output files"));
            delete outfileTp;
            return;
        }

        outTstrp = new QTextStream(outfileTp);
        *outTstrp << QString("<Sleep> (not for medical use)\n");

        if(epochSleep!=0)
        {
            *outTstrp << QString("Total time: %1min\n").arg(epochTotal*epochsec/60);
            *outTstrp << QString("Lie Down: %1min\n").arg(epochSleep*epochsec/60);
            *outTstrp << QString("Lie Down Start:  %1\n").arg(firstFrmDT.addSecs(epochStart*epochsec).toString("yyyyMMdd_hhmmss"));
            *outTstrp << QString("Sleep time: %1min %2\%\n").arg((epochR+epochN1+epochN2+epochN3)*epochsec/60).arg((epochR+epochN1+epochN2+epochN3)*100/epochSleep);
            *outTstrp << QString("Wake: %1min %2\%\n").arg((epochW)*epochsec/60).arg((epochW)*100/epochSleep);
            *outTstrp << QString("REM: %1min %2\%\n").arg(epochR*epochsec/60).arg(epochR*100/epochSleep);
            *outTstrp << QString("S1: %1min %2\%\n").arg(epochN1*epochsec/60).arg(epochN1*100/epochSleep);
            *outTstrp << QString("S2: %1min %2\%\n").arg(epochN2*epochsec/60).arg(epochN2*100/epochSleep);
            *outTstrp << QString("S3: %1min %2\%\n").arg(epochN3*epochsec/60).arg(epochN3*100/epochSleep);
            *outTstrp << QString("Motion: %1min %2\%\n").arg((epochM2)*epochsec/60).arg((epochM2)*100/epochSleep);
        }
        else
        {
            *outTstrp << QString("No Sleep detected");
        }
        outfileTp->close();
        delete outTstrp;
        delete outfileTp;

        // epoch scores
        outFileName = outDirName;
        outFileName.append(QString("E%1_%2_hyp.txt").arg(datetimestr).arg(pdnDes));
        outfileTp = new QFile(outFileName);
        if(outfileTp->exists())
        {
            msgBox.setWindowTitle("Output File exists");
            msgBox.setText(QString("Out file %1 exists in selected directory.").arg(QString("E%1_%2_hyp_epochscores.txt").arg(datetimestr).arg(pdnDes)));
            msgBox.setInformativeText("Do you want to overwrite?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            if(msgBox.exec() == QMessageBox::Cancel)
            {
                delete outfileTp;
                return;
            }
        }
        if(!outfileTp->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            ui->msgPTE->appendPlainText(QString("Problem with output files"));
            delete outfileTp;
            return;
        }

        outTstrp = new QTextStream(outfileTp);
        for(i = 0; i < epochScoreQVs.size(); i++)
            *outTstrp << i*epochsec << "\t" << epochScoreQVs.at(i) << endl;

        outfileTp->close();
        delete outTstrp;
        delete outfileTp;
    }

    // EDF file
    outFileName = outDirName;
    outFileName.append(QString("E%1_%2_hyp.edf").arg(datetimestr).arg(pdnDes));
    outfile1p = new QFile(outFileName);

    if(outfile1p->exists())
    {
        msgBox.setWindowTitle("Out File exists");
        msgBox.setText("Out file exists in selected directory.");
        msgBox.setInformativeText("Do you want to overwrite?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        if(msgBox.exec() == QMessageBox::Cancel)
        {
            delete outfile1p;
            return;
        }
    }
    if(!outfile1p->open(QIODevice::WriteOnly))
    {
        ui->msgPTE->appendPlainText(QString("Problem with output files"));
        delete outfile1p;
        return;
    }

    out1strp = new QDataStream(outfile1p);
    out1strp->setByteOrder(QDataStream::LittleEndian);

    // EDF header stuff
    if(useProfile)
    {
        readName = getConfigValue("Name");
        readDOB = getConfigValue("DOB");
        readSex = getConfigValue("Sex");
        localPatientID = QString("X "); // for hospital idcode
        if(readSex.isEmpty())
            localPatientID.append("X ");
        else
            localPatientID.append(readSex).append(" ");
        if(readDOB.isEmpty())
            localPatientID.append("X ");
        else
            localPatientID.append(readDOB).append(" ");
        if(readName.isEmpty())
            localPatientID.append("X");
        else
            localPatientID.append(readName);
    }
    else
        localPatientID = lpid;

    // make sure StartDate is correct in local recording id
    tempQSL = lrid.split(QRegExp("\\s+"));
    stDateQS = firstFrmDT.toString("-yyyy");
    currMonth = firstFrmDT.date().month();      // have to do this because of QDateTime letters being 2 chars
    if(currMonth == 1)  stDateQS.prepend("JAN");
    else if(currMonth == 2) stDateQS.prepend("FEB");
    else if(currMonth == 3) stDateQS.prepend("MAR");
    else if(currMonth == 4) stDateQS.prepend("APR");
    else if(currMonth == 5) stDateQS.prepend("MAY");
    else if(currMonth == 6) stDateQS.prepend("JUN");
    else if(currMonth == 7) stDateQS.prepend("JUL");
    else if(currMonth == 8) stDateQS.prepend("AUG");
    else if(currMonth == 9) stDateQS.prepend("SEP");
    else if(currMonth == 10) stDateQS.prepend("OCT");
    else if(currMonth == 11) stDateQS.prepend("NOV");
    else stDateQS.prepend("DEC");
    stDateQS.prepend(firstFrmDT.toString("dd-"));
    tempQSL.replace(1, stDateQS);
    lrid = tempQSL.join(" ");

    // EDF header, will need to come back later to adjust number of data records
    edfMslphdropiwrite(out1strp, &localPatientID, &lrid, &firstFrmDT, sleephypQV.size()/EDFDRDURSEC);

    // Stuff in all the data according to specified format and save for meta data processing
    edfMslpwrite(out1strp, &sleephypQV, sleephypQV.size()/EDFDRDURSEC, tsQVpp[pdnSlot]->at(0), &annOnsetTSQV, &annTextQV);

    // can close file now
    outfile1p->close();
    delete out1strp;
    delete outfile1p;

    ui->msgPTE->appendPlainText(QString("PDN#%1: Sleep Analysis Finished, Saved in %2").arg(pdnDes).arg(outFileName));
    endDT = QDateTime::currentDateTime();

    // Show done
    ui->msgPTE->appendPlainText(QString(">> Sleep Analysis Finished in %1 seconds").arg((endDT.toMSecsSinceEpoch()-stDT.toMSecsSinceEpoch())/1000));
    return;
}


bool SleepAnalysisWindow::edfMslphdropiwrite(QDataStream *out, QString *lpidp, QString *lridp,
                                QDateTime *startDTp, qint32 numDataRecs)
{
    QString tempstr, showstr;

    out->writeRawData("0       ", 8);     // edf version of data format
    out->writeRawData(lpidp->toUtf8().leftJustified(80,' ').data(),80);   // local patient identification
    out->writeRawData(lridp->toUtf8().leftJustified(80,' ').data(),80);   // local recording identification
    out->writeRawData(startDTp->toString("dd.MM.yyhh.mm.ss").toUtf8().data(),16); // startdate and starttime
    out->writeRawData("768     ", 8);     // number of header bytes (256+2signals*256)
    out->writeRawData(QByteArray("EDF+C").leftJustified(44,' ').data(),44); // format type (reserved)
    out->writeRawData(QString("%1").arg(numDataRecs).toUtf8().leftJustified(8,' ').data(),8);  // number of data records
    out->writeRawData("8       ", 8);     // duration of a data record in seconds
    out->writeRawData("2   ", 4);       // number of signals: hyp signal(1),annotatino

    out->writeRawData("Hypnogram       ", 16);
    out->writeRawData("EDF Annotations ", 16);

    // transducer type
    out->writeRawData(QByteArray(" ").leftJustified(80,' ').data(),80);
    out->writeRawData(QByteArray(" ").leftJustified(80,' ').data(),80);

    // physical dimensions
    out->writeRawData("state   ", 8);
    out->writeRawData("        ", 8);

    // physical mins and maxs
    out->writeRawData("-1      ", 8);
    out->writeRawData("-100    ", 8);

    out->writeRawData("32767     ", 8);
    out->writeRawData("100     ", 8);

    // digital mins and maxs
    out->writeRawData("-1      ", 8);
    out->writeRawData("-32768  ", 8);

    out->writeRawData("32767   ", 8);
    out->writeRawData("32767   ", 8);

    // prefiltering
    out->writeRawData(QByteArray(" ").leftJustified(80,' ').data(),80);
    out->writeRawData(QByteArray(" ").leftJustified(80,' ').data(),80);

    // number of samples in each data record (8s)
    out->writeRawData("8       ", 8);
    out->writeRawData("30      ", 8);

    // reserved fields
    out->writeRawData(QByteArray(" ").leftJustified(32,' ').data(),32);
    out->writeRawData(QByteArray(" ").leftJustified(32,' ').data(),32);

    return true;
}


void SleepAnalysisWindow::edfMslpwrite(QDataStream *outstrp,
                                       QVector<qint16> *sleephypQVp, qint32 numDataRecs,
                                       qint64 firstFrmTS, QVector<qint64> *tagTSQVp,
                                       QVector<QString> *tagTextQVp)
{
    qint32 i, j;
    QByteArray tempQBA;
    QString tempQS;

    for(i = 0; i < numDataRecs; i++)  //count seconds
    {
        //prepare data, 8 each record
        for(j = 0; j < EDFDRDURSEC; j++)
        {
            *outstrp << sleephypQVp->at(i*EDFDRDURSEC+j);
        }

        // EDF Annotations, max of 2 annotations per data record, each with 33 total chars
        tempQBA.clear();
        tempQBA.append(QString("+%1").arg(i*EDFDRDURSEC).toUtf8().append(QChar(20)).append(QChar(20)).append(QChar(0)));
        for(j = 0; j < 2; j++)
        {
            if(tagTSQVp->size() > 0)
            {
                tempQS = tagTextQVp->at(0);
                tempQS.truncate(14);    // limit text to 14 charcters
                tempQBA.append(QString("+%1").arg((float)(tagTSQVp->at(0)-firstFrmTS)/UCERTCFREQ).toUtf8().append(QChar(20)).append(tempQS.toUtf8()));
                tempQBA.append(QChar(20)).append(QChar(0));
                tagTSQVp->remove(0,1);
                tagTextQVp->remove(0,1);
            }
        }
        outstrp->writeRawData(tempQBA.leftJustified(60,0x00), 60);
    }
}

