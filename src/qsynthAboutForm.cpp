// qsynthAboutForm.cpp
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

#include "qsynthAbout.h"
#include "qsynthAboutForm.h"

#include <QMessageBox>

#ifdef CONFIG_FLUID_VERSION_STR
#include <fluidsynth.h>
#endif

//----------------------------------------------------------------------------
// qsynthAboutForm -- UI wrapper form.

// Constructor.
qsynthAboutForm::qsynthAboutForm ( QWidget *pParent )
	: QDialog(pParent)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	QStringList list;
#ifdef CONFIG_DEBUG
	list << tr("Debugging option enabled.");
#endif
#ifndef CONFIG_SYSTEM_TRAY
	list << tr("System tray disabled.");
#endif
#ifndef CONFIG_FLUID_SERVER
	list << tr("Server option disabled.");
#endif
#ifndef CONFIG_FLUID_SYSTEM_RESET
	list << tr("System reset option disabled.");
#endif
#ifndef CONFIG_FLUID_BANK_OFFSET
	list << tr("Bank offset option disabled.");
#endif

	// Stuff the about box...
	QString sText = "<p align=\"center\"><br />\n";
	sText += "<b>" QSYNTH_TITLE " - " + tr(QSYNTH_SUBTITLE) + "</b><br />\n";
	sText += "<br />\n";
	sText += tr("Version") + ": <b>" CONFIG_BUILD_VERSION "</b><br />\n";
//	sText += "<small>" + tr("Build") + ": " CONFIG_BUILD_DATE "<small><br />\n";
	if (!list.isEmpty()) {
		sText += "<small><font color=\"red\">";
		sText += list.join("<br />\n");
		sText += "</font></small>";
	}
#ifdef CONFIG_FLUID_VERSION_STR
	sText += "<br />\n";
	sText += tr("Using: FluidSynth %1").arg(::fluid_version_str());
	sText += "<br />\n";
#endif
	sText += "<br />\n";
	sText += tr("Website") + ": <a href=\"" QSYNTH_WEBSITE "\">" QSYNTH_WEBSITE "</a><br />\n";
	sText += "<br />\n";
	sText += "<small>";
	sText += QSYNTH_COPYRIGHT "<br />\n";
	sText += "<br />\n";
	sText += tr("This program is free software; you can redistribute it and/or modify it") + "<br />\n";
	sText += tr("under the terms of the GNU General Public License version 2 or later.");
	sText += "</small>";
	sText += "</p>\n";
	m_ui.AboutTextView->setText(sText);

	// UI connections...
	QObject::connect(m_ui.AboutQtButton,
		SIGNAL(clicked()),
		SLOT(aboutQt()));
	QObject::connect(m_ui.ClosePushButton,
		SIGNAL(clicked()),
		SLOT(close()));
}


// Destructor.
qsynthAboutForm::~qsynthAboutForm (void)
{
}


// About Qt request.
void qsynthAboutForm::aboutQt()
{
	QMessageBox::aboutQt(this);
}


// end of qsynthAboutForm.cpp
