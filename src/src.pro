# qsynth.pro
#
TARGET = qsynth

TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

include(src.pri)

#DEFINES += DEBUG

HEADERS += config.h \
	qsynthAbout.h \
	qsynthEngine.h \
	qsynthChannels.h \
	qsynthKnob.h \
	qsynthMeter.h \
	qsynthSetup.h \
	qsynthOptions.h \
	qsynthSystemTray.h \
	qsynthTabBar.h \
	qsynthAboutForm.h \
	qsynthChannelsForm.h \
	qsynthMainForm.h \
	qsynthMessagesForm.h \
	qsynthOptionsForm.h \
	qsynthPresetForm.h \
	qsynthSetupForm.h \
	qsynthDialClassicStyle.h \
	qsynthDialPeppinoStyle.h \
	qsynthDialVokiStyle.h \
	qsynthDialSkulptureStyle.h

SOURCES += \
	qsynth.cpp \
	qsynthEngine.cpp \
	qsynthChannels.cpp \
	qsynthKnob.cpp \
	qsynthMeter.cpp \
	qsynthSetup.cpp \
	qsynthOptions.cpp \
	qsynthSystemTray.cpp \
	qsynthTabBar.cpp \
	qsynthAboutForm.cpp \
	qsynthChannelsForm.cpp \
	qsynthMainForm.cpp \
	qsynthMessagesForm.cpp \
	qsynthOptionsForm.cpp \
	qsynthPresetForm.cpp \
	qsynthSetupForm.cpp \
	qsynthDialClassicStyle.cpp \
	qsynthDialPeppinoStyle.cpp \
	qsynthDialVokiStyle.cpp \
	qsynthDialSkulptureStyle.cpp

FORMS += \
	qsynthAboutForm.ui \
	qsynthChannelsForm.ui \
	qsynthMainForm.ui \
	qsynthMessagesForm.ui \
	qsynthOptionsForm.ui \
	qsynthPresetForm.ui \
	qsynthSetupForm.ui

RESOURCES += \
	qsynth.qrc

TRANSLATIONS += \
   translations/qsynth_cs.ts \
   translations/qsynth_de.ts \
   translations/qsynth_es.ts \
   translations/qsynth_ru.ts

unix {

	# variables
	OBJECTS_DIR = .obj
	MOC_DIR     = .moc
	UI_DIR      = .ui

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	BINDIR = $(bindir)
	DATADIR = $(datadir)
	LOCALEDIR = $(localedir)

	DEFINES += BINDIR=\"$$BINDIR\"
	DEFINES += DATADIR=\"$$DATADIR\"
	DEFINES += LOCALEDIR=\"$$LOCALEDIR\"

	# make install
	INSTALLS += target desktop icon

	target.path = $$BINDIR

	desktop.path = $$DATADIR/applications
	desktop.files += $${TARGET}.desktop

	icon.path = $$DATADIR/icons/hicolor/32x32/apps
	icon.files += images/$${TARGET}.png 
}

win32 {

    contains(CONFIG, debug) { 
        CONFIG += console
    }

    RC_FILE = win32/qsynth.rc
}
