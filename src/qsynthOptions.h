// qsynthOptions.h
//
/****************************************************************************
   Copyright (C) 2003-2019, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __qsynthOptions_h
#define __qsynthOptions_h

#include "qsynthSetup.h"

// Forward declarations.
class QComboBox;


// Former fluidsynth < 2.0 defaults...
//
#define QSYNTH_MASTER_DEFAULT_GAIN     1.0

#define QSYNTH_REVERB_DEFAULT_ROOMSIZE 0.2
#define QSYNTH_REVERB_DEFAULT_DAMP     0.0
#define QSYNTH_REVERB_DEFAULT_WIDTH    0.5
#define QSYNTH_REVERB_DEFAULT_LEVEL    0.9

#define QSYNTH_CHORUS_DEFAULT_N        3
#define QSYNTH_CHORUS_DEFAULT_LEVEL    2.0
#define QSYNTH_CHORUS_DEFAULT_SPEED    0.3
#define QSYNTH_CHORUS_DEFAULT_DEPTH    8.0
#define QSYNTH_CHORUS_DEFAULT_TYPE     0


//-------------------------------------------------------------------------
// qsynthOptions - Prototype settings class.
//

// Forward declaration.
class qsynthEngine;

class qsynthOptions
{
public:

	// Constructor.
	qsynthOptions();
	// Default destructor.
	~qsynthOptions();

	// The settings object accessor.
	QSettings& settings();

	// Explicit I/O methods.
	void loadOptions();
	void saveOptions();

	// Command line arguments parser.
	bool parse_args(const QStringList& args);
	// Command line usage helper.
	void print_usage(const QString& arg0);

	// Default instance setup accessor.
	qsynthSetup *defaultSetup();

	// Display options...
	QString sMessagesFont;
	bool    bMessagesLimit;
	int     iMessagesLimitLines;
	bool    bMessagesLog;
	QString sMessagesLogPath;
	bool    bQueryClose;
	bool    bKeepOnTop;
	bool    bStdoutCapture;
	bool    bOutputMeters;
	bool    bSystemTray;
	bool    bSystemTrayQueryClose;
	bool    bStartMinimized;
	int     iBaseFontSize;
	int     iKnobStyle;
	int     iKnobMotion;

	QString sCustomColorTheme;
	QString sCustomStyleTheme;

	// Default options...
	QString sSoundFontDir;
	bool    bPresetPreview;

	// Available custom engines list.
	QStringList engines;

	// Engine management methods.
	void newEngine(qsynthEngine *pEngine);
	bool renameEngine(qsynthEngine *pEngine);
	void deleteEngine(qsynthEngine *pEngine);

	// Setup registry methods.
	void loadSetup(qsynthSetup *pSetup, const QString& sName);
	void saveSetup(qsynthSetup *pSetup, const QString& sName);

	// Preset management methods.
	bool loadPreset(qsynthEngine *pEngine, const QString& sPreset);
	bool savePreset(qsynthEngine *pEngine, const QString& sPreset);
	bool deletePreset(qsynthEngine *pEngine, const QString& sPreset);

	// Combo box history persistence helper prototypes.
	void loadComboBoxHistory(QComboBox *pComboBox, int iLimit = 8);
	void saveComboBoxHistory(QComboBox *pComboBox, int iLimit = 8);

	// Widget geometry persistence helper prototypes.
	void saveWidgetGeometry(QWidget *pWidget, bool bVisible = false);
	void loadWidgetGeometry(QWidget *pWidget, bool bVisible = false);

private:

	// Settings member variables.
	QSettings m_settings;
	
	// The default setup descriptor.
	qsynthSetup *m_pDefaultSetup;
};


#endif  // __qsynthOptions_h


// end of qsynthOptions.h

