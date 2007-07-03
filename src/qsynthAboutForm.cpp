// qsynthAboutForm.cpp
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
#include "qsynthAboutForm.h"

#include <QMessageBox>


//----------------------------------------------------------------------------
// qsynthAboutForm -- UI wrapper form.

// Constructor.
qsynthAboutForm::qsynthAboutForm (
	QWidget *pParent, Qt::WFlags wflags ) : QDialog(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Stuff the about box...
	QString sText = "<p align=\"center\"><br />\n";
	sText += "<b>" QSYNTH_TITLE " - " + tr(QSYNTH_SUBTITLE) + "</b><br />\n";
	sText += "<br />\n";
	sText += tr("Version") + ": <b>" QSYNTH_VERSION "</b><br />\n";
	sText += tr("Build") + ": " __DATE__ " " __TIME__ "<br />\n";
#ifdef CONFIG_DEBUG
	sText += "<small><font color=\"red\">";
	sText += tr("Debugging option enabled.");
	sText += "<br />\n";
	sText += "</font></small>";
#endif
#ifndef CONFIG_FLUID_SERVER
	sText += "<small><font color=\"red\">";
	sText += tr("Server option disabled.");
	sText += "<br />\n";
	sText += "</font></small>";
#endif
#ifndef CONFIG_FLUID_RESET
	sText += "<small><font color=\"red\">";
	sText += tr("System reset option disabled.");
	sText += "<br />\n";
	sText += "</font></small>";
#endif
#ifndef CONFIG_FLUID_BANK_OFFSET
	sText += "<small><font color=\"red\">";
	sText += tr("Bank offset option disabled.");
	sText += "<br />\n";
	sText += "</font></small>";
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
