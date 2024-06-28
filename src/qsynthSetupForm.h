// qsynthSetupForm.h
//
/****************************************************************************
   Copyright (C) 2003-2024, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __qsynthSetupForm_h
#define __qsynthSetupForm_h

#include "ui_qsynthSetupForm.h"

#include "qsynthSetup.h"

// Forward declarations.
class qsynthOptions;
class qsynthEngine;
class qsynthSetup;

class qsynthSettingsItemEditor;

class QPixmap;


//----------------------------------------------------------------------------
// qsynthSetupForm -- UI wrapper form.

class qsynthSetupForm : public QDialog
{
	Q_OBJECT

public:

	// Constructor.
	qsynthSetupForm(QWidget *pParent = nullptr);
	// Destructor.
	~qsynthSetupForm();

	// Populate (setup) dialog controls from settings descriptors.
	void setup(qsynthOptions *pOptions, qsynthEngine *pEngine, bool bNew);

	// Settings accessors.
	QTreeWidget *settingsListView() const;
	qsynthSetup *engineSetup() const;

	void setSettingsItem(const QString& sKey, const QString& sVal);
	QString settingsItem(const QString& sKey) const;
	bool isSettingsItem(const QString& sKey) const;

	void setSettingsItemEditor(qsynthSettingsItemEditor *pItemEditor);
	qsynthSettingsItemEditor *settingsItemEditor() const;

public slots:

	void nameChanged(const QString&);
	void midiDriverChanged(int index);
	void audioDriverChanged(int index);
	void settingsChanged();

	void soundfontContextMenu(const QPoint&);

	void openSoundFont();
	void editSoundFont();
	void removeSoundFont();
	void moveUpSoundFont();
	void moveDownSoundFont();

	void stabilizeForm();

protected slots:

	void soundfontItemChanged();

	void settingsItemActivated(QTreeWidgetItem *pItem, int iColumn);
	void settingsItemChanged(QTreeWidgetItem *pItem, QTreeWidgetItem *);
	void settingsItemChanged();

	void accept();
	void reject();

protected:

	// A combo-box text item setter helper.
	void setComboBoxCurrentText(
		QComboBox *pComboBox, const QString& sText) const;

	void updateMidiDevices(const QString& sMidiDriver);
	void updateAudioDevices(const QString& sAudioDriver);

	void refreshSoundFonts();

private:

	// The Qt-designer UI struct...
	Ui::qsynthSetupForm m_ui;

	// Instance variables.
	qsynthOptions *m_pOptions;
	qsynthSetup   *m_pSetup;

	int m_iDirtySetup;
	int m_iDirtyCount;

	QString  m_sSoundFontDir;
	QPixmap *m_pXpmSoundFont;

	qsynthSettingsItemEditor *m_pSettingsItemEditor;

	qsynthSetup::Settings m_settings;
};


//-------------------------------------------------------------------------
// qsynthSettingsItemEditor - list-view item editor widget decl.

class qsynthSettingsItemEditor : public QWidget
{
	Q_OBJECT

public:

	// Constructor.
	qsynthSettingsItemEditor(
		qsynthSetupForm *pSetupForm,
		const QModelIndex& index,
		QWidget *pParent = nullptr);

	// Destructor.
	virtual ~qsynthSettingsItemEditor();

	// Target index accessor.
	const QModelIndex& index() const;

	// Value accessors.
	void setValue(const QString& sValue);
	QString value() const;

	// Current/Default value accessors.
	const QString& currentKey()   const;
	const QString& currentValue() const;
	const QString& defaultValue() const;

signals:

	void commitEditor(QWidget *pEditor);

protected slots:

	// Local interaction slots.
	void changed();
	void committed();
	void reset();

private:

	// Instance variables.
	qsynthSetupForm *m_pSetupForm;
	const QModelIndex& m_index;

	enum {
		SpinBox,
		DoubleSpinBox,
		LineEdit,
		ComboBox
	} m_type;

	union {
		QSpinBox       *pSpinBox;
		QDoubleSpinBox *pDoubleSpinBox;
		QLineEdit      *pLineEdit;
		QComboBox      *pComboBox;
	} m_u;

	QToolButton *m_pToolButton;

	QString m_sCurrentKey;
	QString m_sCurrentValue;
	QString m_sDefaultValue;
};


//-------------------------------------------------------------------------
// qsynthSettingsItemDelegate - list-view item delegate decl.

#include <QItemDelegate>

class qsynthSettingsItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:

	// Constructor.
	qsynthSettingsItemDelegate(qsynthSetupForm *pSetupForm);

protected:

	void paint(QPainter *pPainter,
		const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

	QWidget *createEditor(QWidget *pParent,
		const QStyleOptionViewItem& option,
		const QModelIndex & index) const;

	void setEditorData(QWidget *pEditor,
		const QModelIndex &index) const;
	void setModelData(QWidget *pEditor,
		QAbstractItemModel *pModel,
		const QModelIndex& index) const;

	QSize sizeHint(
		const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

protected slots:

	void commitEditor(QWidget *pEditor);

private:

	qsynthSetupForm *m_pSetupForm;
};


#endif	// __qsynthSetupForm_h


// end of qsynthSetupForm.h
