INCLUDEPATH += ./src

SOURCES += src/main.cpp \
           src/qsynthSetup.cpp

HEADERS += src/qsynthAbout.h \
           src/qsynthSetup.h

FORMS    = src/qsynthMainForm.ui \
           src/qsynthMessagesForm.ui \
           src/qsynthSetupForm.ui \
           src/qsynthAboutForm.ui

IMAGES   = icons/qsynth.xpm \
           icons/messages1.png \
           icons/setup1.png \
           icons/about1.png \
           icons/accept1.png \
           icons/quit1.png \
           icons/qtlogo.png

TEMPLATE = app
CONFIG  += qt warn_on release
LANGUAGE = C++

LIBS    += -lfluidsynth

