######################################################################
# Automatically generated by qmake (2.01a) ?? ??? 14 21:02:20 2012
######################################################################

TEMPLATE = app
TARGET = RelaxConsole
DEPENDPATH += . PDC
INCLUDEPATH += .
# Input
HEADERS += \
           consolewindow.h \
           FFT/Array.h \
           FFT/Array.hpp \
           FFT/def.h \
           FFT/DynArray.h \
           FFT/DynArray.hpp \
           FFT/FFTReal.h \
           FFT/FFTReal.hpp \
           FFT/FFTRealFixLen.h \
           FFT/FFTRealFixLen.hpp \
           FFT/FFTRealFixLenParam.h \
           FFT/FFTRealPassDirect.h \
           FFT/FFTRealPassDirect.hpp \
           FFT/FFTRealPassInverse.h \
           FFT/FFTRealPassInverse.hpp \
           FFT/FFTRealSelect.h \
           FFT/FFTRealSelect.hpp \
           FFT/FFTRealUseTrigo.h \
           FFT/FFTRealUseTrigo.hpp \
           FFT/OscSinCos.h \
           FFT/OscSinCos.hpp \
           FFT/qcomplexnumber.h \
           FFT/qfouriercalculator.h \
           FFT/qfourierfixedcalculator.h \
           FFT/qfouriertransformer.h \
           FFT/qfouriervariablecalculator.h \
           FFT/qwindowfunction.h \
           PDC/showdatawindow.h \
           PDC/startwindow.h \
    converter/convertwindow.h \
    config/configwindow.h \
    opi_helper.h \
    converter/convertoptionswindow.h \
    biofeedback/biofeedback.h \
    biofeedback/startwindowbf.h \
    converter/signalviewer.h \
    license/licensedialog.h \
    profile/profiledialog.h \
    readmm/readmmdialog.h \
    TutorialGame/tutorialgame.h \
    sleepanalysis/sleepanalysiswindow.h \
    converter/twodaccelviewer.h \
    medanalysis/medanalysiswindow.h \
    converter/mixsignaldialog.h \
    converter/spectrogram.h \
    converter/qqwidget.h \
    opi_uce_win.h

FORMS += consolewindow.ui \
         PDC/showdatawindow.ui \
         PDC/startwindow.ui \
    converter/convertwindow.ui \
    config/configwindow.ui \
    converter/convertoptionswindow.ui \
    biofeedback/biofeedback.ui \
    biofeedback/startwindowbf.ui \
    converter/signalviewer.ui \
    license/licensedialog.ui \
    profile/profiledialog.ui \
    readmm/readmmdialog.ui \
    TutorialGame/tutorialgame.ui \
    sleepanalysis/sleepanalysiswindow.ui \
    converter/twodaccelviewer.ui \
    medanalysis/medanalysiswindow.ui \
    converter/mixsignaldialog.ui \
    converter/spectrogram.ui

SOURCES += consolewindow.cpp \
           main.cpp \
           FFT/qcomplexnumber.cpp \
           FFT/qfouriercalculator.cpp \
           FFT/qfourierfixedcalculator.cpp \
           FFT/qfouriertransformer.cpp \
           FFT/qfouriervariablecalculator.cpp \
           FFT/qwindowfunction.cpp \
           PDC/showdatawindow.cpp \
           PDC/startwindow.cpp \
    converter/convertwindow.cpp \
    config/configwindow.cpp \
    opi_helper.cpp \
    converter/convertoptionswindow.cpp \
    biofeedback/biofeedback.cpp \
    biofeedback/startwindowbf.cpp \
    converter/signalviewer.cpp \
    license/licensedialog.cpp \
    profile/profiledialog.cpp \
    readmm/readmmdialog.cpp \
    TutorialGame/tutorialgame.cpp \
    sleepanalysis/sleepanalysiswindow.cpp \
    converter/twodaccelviewer.cpp \
    medanalysis/medanalysiswindow.cpp \
    converter/mixsignaldialog.cpp \
    converter/spectrogram.cpp \
    converter/qqwidget.cpp \
    opi_uce_win.cpp

RESOURCES += relaxconsole.qrc
RC_FILE += images/myIcon.rc



