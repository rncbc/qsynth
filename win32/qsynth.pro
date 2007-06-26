INCLUDEPATH += ../src

SOURCES += ../src/main.cpp \
           ../src/qsynthSetup.cpp \
           ../src/qsynthEngine.cpp \
           ../src/qsynthOptions.cpp \
           ../src/qsynthChannels.cpp \
           ../src/qsynthTabBar.cpp \
           ../src/qsynthMeter.cpp \
           ../src/qsynthSystemTray.cpp \
           ../src/qsynthKnob.cpp

HEADERS += ../src/qsynthAbout.h \
           ../src/qsynthSetup.h \
           ../src/qsynthEngine.h \
           ../src/qsynthOptions.h \
           ../src/qsynthChannels.h \
           ../src/qsynthTabBar.h \
           ../src/qsynthMeter.h \
           ../src/qsynthSystemTray.h \
           ../src/qsynthKnob.h

FORMS    = ../src/qsynthMainForm.ui \
           ../src/qsynthMessagesForm.ui \
           ../src/qsynthChannelsForm.ui \
           ../src/qsynthOptionsForm.ui \
	       ../src/qsynthPresetForm.ui \
           ../src/qsynthSetupForm.ui \
           ../src/qsynthAboutForm.ui

IMAGES   = ../icons/qsynth.png \
           ../icons/messages1.png \
           ../icons/options1.png \
           ../icons/about1.png \
           ../icons/accept1.png \
           ../icons/quit1.png \
           ../icons/setup1.png \
           ../icons/reset1.png \
           ../icons/panic1.png \
           ../icons/restart1.png \
           ../icons/channels1.png \
           ../icons/ledoff1.png \
           ../icons/ledon1.png \
           ../icons/sfont1.png \
           ../icons/open1.png \
           ../icons/save1.png \
           ../icons/add1.png \
           ../icons/remove1.png \
           ../icons/up1.png \
           ../icons/down1.png \
           ../icons/qtlogo.png

TEMPLATE = app
CONFIG  += qt warn_on debug
LANGUAGE = C++

win32 {
	CONFIG  += console
	INCLUDEPATH	+= C:\usr\local\include
	LIBS    += C:\usr\local\lib\libfluidsynth.lib
}

unix {
	LIBS    += -lfluidsynth
}

