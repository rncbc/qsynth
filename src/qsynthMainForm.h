// qsynthMainForm.h
//
/****************************************************************************
   Copyright (C) 2003-2020, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __qsynthMainForm_h
#define __qsynthMainForm_h

#include "ui_qsynthMainForm.h"

#include <QMenu>


// Forward declarations
class qsynthOptions;
class qsynthMessagesForm;
class qsynthChannelsForm;

#ifdef CONFIG_SYSTEM_TRAY
class qsynthSystemTray;
#endif

class QSocketNotifier;
class QSessionManager;
class QMimeSource;


//----------------------------------------------------------------------------
// qsynthMainForm -- UI wrapper form.

class qsynthMainForm : public QWidget
{
	Q_OBJECT

public:

	// Constructor.
	qsynthMainForm(QWidget *pParent = nullptr,
		Qt::WindowFlags wflags = Qt::WindowFlags());
	// Destructor.
	~qsynthMainForm();

	static qsynthMainForm *getInstance();

	void setup(qsynthOptions *pOptions);

	void appendMessages(const QString& sText);
	void appendMessagesColor(const QString& sText, const QColor& rgb);
	void appendMessagesText(const QString& sText);
	void appendMessagesError(const QString& sText);

	bool deleteEngineTab(qsynthEngine *pEngine, int iTab);
	bool setupEngineTab(qsynthEngine *pEngine, int iTab);

	void startAllEngines();

	bool startEngine(qsynthEngine *pEngine);
	void stopEngine(qsynthEngine *pEngine);

	void restartAllEngines();
	void restartEngine(qsynthEngine *pEngine);
	void resetEngine(qsynthEngine *pEngine);

	enum KnobStyle { Classic, Vokimon, Peppino, Skulpture, Legacy };

	void stabilizeForm();
	void stabilizeFormEx();

protected slots:

	void stdoutNotifySlot(int);
	void sigtermNotifySlot(int);

	void programReset();
	void systemReset();
	void promptRestart();

	void newEngine();
	void deleteEngine();

	void toggleMainForm();
	void toggleMessagesForm();
	void toggleChannelsForm();

	void showSetupForm();
	void showOptionsForm();
	void showAboutForm();

	void tabSelect(int);

	void tabContextMenu(int, const QPoint&);

	void timerSlot();

	void reverbActivate(bool);
	void chorusActivate(bool);

	void gainChanged(int);
	void reverbChanged(int);
	void chorusChanged(int);

	void activateEnginesMenu(QAction*);

	void quitMainForm();

	void commitData(QSessionManager& sm);

protected:

	bool queryClose();

	void showEvent(QShowEvent *pShowEvent);
	void hideEvent(QHideEvent *pHideEvent);
	void closeEvent(QCloseEvent *pCloseEvent);

	void appendStdoutBuffer(const QString& s);
	void processStdoutBuffer();
	void flushStdoutBuffer();

	bool stdoutBlock(int fd, bool bBlock) const;

	void playLoadFiles(qsynthEngine *pEngine, const QStringList& files, bool bSetup);

	bool decodeDragFiles(const QMimeSource *pEvent, QStringList& files);
	void dragEnterEvent(QDragEnterEvent *pDragEnterEvent);
	void dropEvent(QDropEvent *pDropEvent);

	void updateMessagesFont();
	void updateMessagesLimit();
	void updateOutputMeters();
#ifdef CONFIG_SYSTEM_TRAY
	void updateSystemTray();
#endif

	void updateContextMenu();

	qsynthEngine *currentEngine() const;

	void setEngineGain(qsynthEngine *pEngine, float fGain);
	void setEngineReverbOn(qsynthEngine *pEngine, bool bActive);
	void setEngineReverb(qsynthEngine *pEngine,
		double fRoom, double fDamp, double fWidth, double fLevel);
	void setEngineChorusOn(qsynthEngine *pEngine, bool bActive);
	void setEngineChorus(qsynthEngine *pEngine,
		int iNr, double fLevel, double fSpeed, double fDepth, int iType);

	void loadPanelSettings(qsynthEngine *pEngine, bool bUpdate);
	void savePanelSettings(qsynthEngine *pEngine);

	void resetChannelsForm(qsynthEngine *pEngine, bool fPreset);

	void resetGain();
	void resetReverb();
	void resetChorus();

	void updateGain();
	void updateReverb();
	void updateChorus();

	void refreshGain();
	void refreshReverb();
	void refreshChorus();

	void contextMenuEvent(QContextMenuEvent *pEvent);

	void updateKnobs();

private:

	// The Qt-designer UI struct...
	Ui::qsynthMainForm m_ui;

	// Instance variables.
	qsynthOptions *m_pOptions;

	int m_iTimerDelay;
	int m_iCurrentTab;

	QSocketNotifier *m_pStdoutNotifier;
	QSocketNotifier *m_pSigtermNotifier;

	qsynthMessagesForm *m_pMessagesForm;
	qsynthChannelsForm *m_pChannelsForm;

	int m_iGainChanged;
	int m_iReverbChanged;
	int m_iChorusChanged;
	int m_iGainUpdated;
	int m_iReverbUpdated;
	int m_iChorusUpdated;

	QString m_sStdoutBuffer;

#ifdef CONFIG_SYSTEM_TRAY
	qsynthSystemTray *m_pSystemTray;
	int m_iSystemTrayState;
	bool m_bQuitClose;
#endif

	bool m_bQuitForce;

	// Common context menu.
	QMenu m_menu;

	// Kind-of singleton reference.
	static qsynthMainForm *g_pMainForm;

	// Style for qsynthKnob (QDial) widgets
	QStyle* m_pKnobStyle;	
};


#endif	// __qsynthMainForm_h


// end of qsynthMainForm.h
