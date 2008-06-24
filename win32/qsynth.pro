INCPATH += ../src
HEADERS += ../src/qsynthAbout.h \
    ../src/qsynthEngine.h \
    ../src/qsynthChannels.h \
    ../src/qsynthKnob.h \
    ../src/qsynthMeter.h \
    ../src/qsynthSetup.h \
    ../src/qsynthOptions.h \
    ../src/qsynthSystemTray.h \
    ../src/qsynthTabBar.h \
    ../src/qsynthAboutForm.h \
    ../src/qsynthChannelsForm.h \
    ../src/qsynthMainForm.h \
    ../src/qsynthMessagesForm.h \
    ../src/qsynthOptionsForm.h \
    ../src/qsynthPresetForm.h \
    ../src/qsynthSetupForm.h \
    ../src/qsynthDialClassicStyle.h \
    ../src/qsynthDialPeppinoStyle.h \
    ../src/qsynthDialVokiStyle.h
SOURCES += ../src/main.cpp \
    ../src/qsynthEngine.cpp \
    ../src/qsynthChannels.cpp \
    ../src/qsynthKnob.cpp \
    ../src/qsynthMeter.cpp \
    ../src/qsynthSetup.cpp \
    ../src/qsynthOptions.cpp \
    ../src/qsynthSystemTray.cpp \
    ../src/qsynthTabBar.cpp \
    ../src/qsynthAboutForm.cpp \
    ../src/qsynthChannelsForm.cpp \
    ../src/qsynthMainForm.cpp \
    ../src/qsynthMessagesForm.cpp \
    ../src/qsynthOptionsForm.cpp \
    ../src/qsynthPresetForm.cpp \
    ../src/qsynthSetupForm.cpp \
    ../src/qsynthDialClassicStyle.cpp \
    ../src/qsynthDialPeppinoStyle.cpp \
    ../src/qsynthDialVokiStyle.cpp
FORMS = ../src/qsynthAboutForm.ui \
    ../src/qsynthChannelsForm.ui \
    ../src/qsynthMainForm.ui \
    ../src/qsynthMessagesForm.ui \
    ../src/qsynthOptionsForm.ui \
    ../src/qsynthPresetForm.ui \
    ../src/qsynthSetupForm.ui
RESOURCES += ../icons/qsynth.qrc
TEMPLATE = app
CONFIG += qt \
    thread \
    warn_on \
    debug
LANGUAGE = C++
win32 { 
    CONFIG += console
    INCPATH += /usr/local/include
    LIBS += -L/usr/local/lib
    RC_FILE = qsynth.rc
}
LIBS += -lfluidsynth
TRANSLATIONS = ../translations/qsynth_de.ts \
    ../translations/qsynth_es.ts
