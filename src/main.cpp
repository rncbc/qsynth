// main.cpp
//
/****************************************************************************
   Copyright (C) 2003-2007, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qsynthAbout.h"
#include "qsynthOptions.h"
#include "qsynthMainForm.h"

#include <QApplication>
#include <QTranslator>
#include <QLocale>

//-------------------------------------------------------------------------
// Single application instance stuff (Qt/X11 only atm.)
//

#if defined(Q_WS_X11)

#include <QX11Info>

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#define QSYNTH_XUNIQUE "qsynthMainForm_xunique"

static bool qsynth_get_xunique (void)
{
	Display *pDisplay = QX11Info::display();
	Atom aSelection = XInternAtom(pDisplay, QSYNTH_XUNIQUE, false);

	XGrabServer(pDisplay);
	Window window = XGetSelectionOwner(pDisplay, aSelection);
	if (window != None)
		XRaiseWindow(pDisplay, window);
	XUngrabServer(pDisplay);

//	XFlush(pDisplay);

	return (window != None);
}

static void qsynth_set_xunique ( QWidget *pWidget )
{
	Display *pDisplay = QX11Info::display();
	Atom aSelection = XInternAtom(pDisplay, QSYNTH_XUNIQUE, false);

	XGrabServer(pDisplay);
	Window window = pWidget->winId();
	XSetSelectionOwner(pDisplay, aSelection, window, CurrentTime);
	XUngrabServer(pDisplay);

//	XFlush(pDisplay);
}

#endif


//-------------------------------------------------------------------------
// main - The main program trunk.
//

int main ( int argc, char **argv )
{
	QApplication app(argc, argv);

	// Load translation support.
	QTranslator translator(0);
	QLocale loc;
	if (loc.language() != QLocale::C) {
		QString sLocName = "qsynth_" + loc.name();
		if (!translator.load(sLocName, ".")) {
			QString sLocPath = CONFIG_PREFIX "/share/locale";
			if (!translator.load(sLocName, sLocPath))
				fprintf(stderr, "Warning: no locale found: %s/%s.qm\n",
					sLocPath.toUtf8().constData(),
					sLocName.toUtf8().constData());
		}
		app.installTranslator(&translator);
	}

	// Construct default settings; override with command line arguments.
	qsynthOptions settings;
	if (!settings.parse_args(app.argc(), app.argv())) {
		app.quit();
		return 1;
	}

#if defined(Q_WS_X11)
	if (qsynth_get_xunique()) {
		app.quit();
		return 2;
	}
#endif

	// What style do we create these forms?
	Qt::WindowFlags wflags = Qt::Window
#if QT_VERSION >= 0x040200
		| Qt::CustomizeWindowHint
#endif
		| Qt::WindowTitleHint
		| Qt::WindowSystemMenuHint
		| Qt::WindowMinMaxButtonsHint;
	if (settings.bKeepOnTop)
		wflags |= Qt::Tool;
	// Construct the main form, and show it to the world.
	qsynthMainForm w(0, wflags);
//	app.setMainWidget(&w);
	w.setup(&settings);
	// If we have a systray icon, we'll skip this.
	if (!settings.bSystemTray) {
		w.show();
		w.adjustSize();
	}

#if defined(Q_WS_X11)
	qsynth_set_xunique(&w);
#endif

	// Register the quit signal/slot.
	// app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
	app.setQuitOnLastWindowClosed(false);

	return app.exec();
}

// end of main.cpp
