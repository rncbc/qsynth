// qsynthSetup.h
//
/****************************************************************************
   Copyright (C) 2003, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#ifndef __qsynthSetup_h
#define __qsynthSetup_h

#include <qstring.h>
#include <qstringlist.h>
#include <qsettings.h>
#include <qcombobox.h>

#include <stdlib.h>
#include <unistd.h>

#include <fluidsynth.h>


//-------------------------------------------------------------------------
// qsynthSetup - Prototype settings class.
//

class qsynthSetup
{
public:

    // Constructor.
    qsynthSetup();
    // Default destructor.
    ~qsynthSetup();

    // Command line arguments parser.
    bool parse_args(int argc, char **argv);
    // Command line usage helper.
    void print_usage(const char *arg0);

    // Settings cache realization.
    void realize();

    // Fluidsynth settings accessor.
    fluid_settings_t *fluid_settings();

    // Settings variables.
    bool    bMidiIn;
    QString sMidiDriver;
    int     iMidiChannels;
    QString sAudioDriver;
    QString sJackName;
    bool    bJackAutoConnect;
    bool    bJackMulti;
    int     iAudioChannels;
    int     iAudioGroups;
    int     iAudioBufSize;
    int     iAudioBufCount;
    QString sSampleFormat;
    float   fSampleRate;
    int     iPolyphony;
    bool    bReverbActive;
    double  fReverbRoom;
    double  fReverbDamp;
    double  fReverbWidth;
    double  fReverbLevel;
    bool    bChorusActive;
    int     iChorusNr;
    double  fChorusLevel;
    double  fChorusSpeed;
    double  fChorusDepth;
    int     iChorusType;
    bool    bLadspaActive;
    float   fGain;
    bool    bServer;
    bool    bMidiDump;
    bool    bVerbose;

    // Optional file lists.
    QStringList soundfonts;
    QStringList midifiles;

    // Display options...
    QString sMessagesFont;
    bool    bQueryClose;

    // Default options...
    QString sSoundFontDir;

    // Combo box history persistence helper prototypes.
    static void add2ComboBoxHistory(QComboBox *pComboBox, const QString& sNewText, int iIndex = 0, int iLimit = 8);
    void loadComboBoxHistory(QComboBox *pComboBox, int iLimit = 8);
    void saveComboBoxHistory(QComboBox *pComboBox, int iLimit = 8);

    // Widget geometry persistence helper prototypes.
    void saveWidgetGeometry(QWidget *pWidget);
    void loadWidgetGeometry(QWidget *pWidget);

private:

    // Special parsing of '-o' command-line option into fluidsynth settings.
    bool parse_option(char *optarg);

    // Settings member variables.
    QSettings m_settings;
    fluid_settings_t *m_pFluidSettings;
};


#endif  // __qsynthSetup_h


// end of qsynthSetup.h
