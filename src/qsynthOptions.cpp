// qsynthOptions.cpp
//
/****************************************************************************
   Copyright (C) 2003-2004, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "qsynthOptions.h"
#include "qsynthEngine.h"
#include "qsynthAbout.h"

#include "config.h"


//-------------------------------------------------------------------------
// qsynthOptions - Prototype settings structure.
//

// Constructor.
qsynthOptions::qsynthOptions (void)
{
    // Begin master key group.
    m_settings.beginGroup("/qsynth");

    // Create default setup descriptor.
    m_pDefaultSetup = new qsynthSetup();
    // Load previous/default fluidsynth settings...
    loadSetup(m_pDefaultSetup, QString::null);

    // Load display options...
    m_settings.beginGroup("/Options");
    sMessagesFont  = m_settings.readEntry("/MessagesFont", QString::null);
    bMessagesLimit = m_settings.readBoolEntry("/MessagesLimit", true);
    iMessagesLimitLines = m_settings.readNumEntry("/MessagesLimitLines", 1000);
    bQueryClose    = m_settings.readBoolEntry("/QueryClose",    true);
    bKeepOnTop     = m_settings.readBoolEntry("/KeepOnTop",     true);
    bStdoutCapture = m_settings.readBoolEntry("/StdoutCapture", true);
    m_settings.endGroup();

    // Load defaults...
    m_settings.beginGroup("/Defaults");
    sSoundFontDir  = m_settings.readEntry("/SoundFontDir", QString::null);
    bPresetPreview = m_settings.readBoolEntry("/PresetPreview", false);
    m_settings.endGroup();

    // Load custom additional engines.
    m_settings.beginGroup("/Engines");
    const QString sEnginePrefix = "/Engine";
    int iEngine = 0;
    for (;;) {
        QString sItem = m_settings.readEntry(sEnginePrefix + QString::number(++iEngine), QString::null);
        if (sItem.isEmpty())
            break;
        engines.append(sItem);
    }
    m_settings.endGroup();
}


// Default Destructor.
qsynthOptions::~qsynthOptions (void)
{
    // Make program version available in the future.
    m_settings.beginGroup("/Program");
    m_settings.writeEntry("/Version", QSYNTH_VERSION);
    m_settings.endGroup();

    // Save engines list...
    m_settings.beginGroup("/Engines");
    // Save last preset list.
    const QString sEnginePrefix = "/Engine";
    int iEngine = 0;
    for (QStringList::Iterator iter = engines.begin(); iter != engines.end(); iter++)
        m_settings.writeEntry(sEnginePrefix + QString::number(++iEngine), *iter);
    // Cleanup old entries, if any...
    for (++iEngine; !m_settings.readEntry(sEnginePrefix + QString::number(iEngine)).isEmpty(); iEngine++)
        m_settings.removeEntry(sEnginePrefix + QString::number(iEngine));
    m_settings.endGroup();
    
    // Save defaults...
    m_settings.beginGroup("/Defaults");
    m_settings.writeEntry("/SoundFontDir",  sSoundFontDir);
    m_settings.writeEntry("/PresetPreview", bPresetPreview);
    m_settings.endGroup();

    // Save last display options.
    m_settings.beginGroup("/Options");
    m_settings.writeEntry("/MessagesFont",  sMessagesFont);
    m_settings.writeEntry("/MessagesLimit", bMessagesLimit);
    m_settings.writeEntry("/MessagesLimitLines", iMessagesLimitLines);
    m_settings.writeEntry("/QueryClose",    bQueryClose);
    m_settings.writeEntry("/KeepOnTop",     bKeepOnTop);
    m_settings.writeEntry("/StdoutCapture", bStdoutCapture);
    m_settings.endGroup();

    // Load previous/default fluidsynth settings...
    saveSetup(m_pDefaultSetup, QString::null);
    // Create default setup descriptor.
    delete m_pDefaultSetup;
    m_pDefaultSetup = NULL;
    
    m_settings.endGroup();
}


// Default instance setup accessor.
qsynthSetup *qsynthOptions::defaultSetup (void)
{
    return m_pDefaultSetup;
}


//-------------------------------------------------------------------------
// Command-line argument stuff. Mostly to mimic fluidsynth CLI.
//

// Help about command line options.
void qsynthOptions::print_usage ( const char *arg0 )
{
    const QString sEot = "\n\t";
    const QString sEol = "\n\n";

    fprintf(stderr, QObject::tr("Usage") + ": %s"
        " [" + QObject::tr("options")    + "]"
        " [" + QObject::tr("soundfonts") + "]"
        " [" + QObject::tr("midifiles")  + "]" + sEol, arg0);
    fprintf(stderr, QSYNTH_TITLE " - " + QObject::tr(QSYNTH_SUBTITLE) + sEol);
    fprintf(stderr, QObject::tr("Options") + ":" + sEol);
    fprintf(stderr, "  -n, --no-midi-in" + sEot +
        QObject::tr("Don't create a midi driver to read MIDI input events [default = yes]") + sEol);
    fprintf(stderr, "  -m, --midi-driver=[label]" + sEot +
        QObject::tr("The name of the midi driver to use [oss,alsa,alsa_seq,...]") + sEol);
    fprintf(stderr, "  -K, --midi-channels=[num]" + sEot +
        QObject::tr("The number of midi channels [default = 16]") + sEol);
    fprintf(stderr, "  -a, --audio-driver=[label]" + sEot +
        QObject::tr("The audio driver [alsa,jack,oss,dsound,...]") + sEol);
    fprintf(stderr, "  -j, --connect-jack-outputs" + sEot +
        QObject::tr("Attempt to connect the jack outputs to the physical ports") + sEol);
    fprintf(stderr, "  -L, --audio-channels=[num]" + sEot +
        QObject::tr("The number of stereo audio channels [default = 1]") + sEol);
    fprintf(stderr, "  -G, --audio-groups=[num]" + sEot +
        QObject::tr("The number of audio groups [default = 1]") + sEol);
    fprintf(stderr, "  -z, --audio-bufsize=[size]" + sEot +
        QObject::tr("Size of each audio buffer") + sEol);
    fprintf(stderr, "  -c, --audio-bufcount=[count]" + sEot +
        QObject::tr("Number of audio buffers") + sEol);
    fprintf(stderr, "  -r, --sample-rate=[rate]" + sEot +
        QObject::tr("Set the sample rate") + sEol);
    fprintf(stderr, "  -R, --reverb=[flag]" + sEot +
        QObject::tr("Turn the reverb on or off [1|0|yes|no|on|off, default = on]") + sEol);
    fprintf(stderr, "  -C, --chorus=[flag]" + sEot +
        QObject::tr("Turn the chorus on or off [1|0|yes|no|on|off, default = on]") + sEol);
    fprintf(stderr, "  -g, --gain=[gain]" + sEot +
        QObject::tr("Set the master gain [0 < gain < 10, default = 0.2]") + sEol);
    fprintf(stderr, "  -o, --option [name=value]" + sEot +
        QObject::tr("Define a setting name=value") + sEol);
    fprintf(stderr, "  -s, --server" + sEot +
        QObject::tr("Create and start server [default = no]") + sEol);
    fprintf(stderr, "  -i, --no-shell" + sEot +
        QObject::tr("Don't read commands from the shell [ignored]") + sEol);
    fprintf(stderr, "  -d, --dump" + sEot +
        QObject::tr("Dump midi router events") + sEol);
    fprintf(stderr, "  -v, --verbose" + sEot +
        QObject::tr("Print out verbose messages about midi events") + sEol);
    fprintf(stderr, "  -h, --help" + sEot +
        QObject::tr("Show help about command line options") + sEol);
    fprintf(stderr, "  -V, --version" + sEot +
        QObject::tr("Show version information") + sEol);
}


// Special parsing of '-o' command-line option into fluidsynth settings.
bool qsynthOptions::parse_option ( char *optarg )
{
    char *val;

    for (val = optarg; *val; val++) {
        if (*val == '=') {
            *val++ = (char) 0;
            break;
        }
    }

    fluid_settings_t *pFluidSettings = m_pDefaultSetup->fluid_settings();
    
    switch (::fluid_settings_get_type(pFluidSettings, optarg)) {
    case FLUID_NUM_TYPE:
        if (::fluid_settings_setnum(pFluidSettings, optarg, ::atof(val)))
            break;
    case FLUID_INT_TYPE:
        if (::fluid_settings_setint(pFluidSettings, optarg, ::atoi(val)))
            break;
    case FLUID_STR_TYPE:
        if (::fluid_settings_setstr(pFluidSettings, optarg, val))
            break;
    default:
        return false;
    }

    return true;
}


// Parse command line arguments into fluidsynth settings.
bool qsynthOptions::parse_args ( int argc, char **argv )
{
    const QString sEol = "\n\n";
    int iSoundFontOverride = 0;

    for (int i = 1; i < argc; i++) {

        QString sArg = argv[i];
        QString sVal = QString::null;
        int iEqual = sArg.find("=");
        if (iEqual >= 0) {
            sVal = sArg.right(sArg.length() - iEqual - 1);
            sArg = sArg.left(iEqual);
        }
        else if (i < argc)
            sVal = argv[i + 1];

        if (sArg == "-n" || sArg == "--no-midi-in") {
            m_pDefaultSetup->bMidiIn = false;
        }
        else if (sArg == "-m" || sArg == "--midi-driver") {
        	if (sVal.isNull()) {
                fprintf(stderr, QObject::tr("Option -m requires an argument (midi-driver).") + sEol);
                return false;
            }
            m_pDefaultSetup->sMidiDriver = sVal;
            if (iEqual < 0)
                i++;
        }
        else if (sArg == "-K" || sArg == "--midi-channels") {
        	if (sVal.isNull()) {
                fprintf(stderr, QObject::tr("Option -K requires an argument (midi-channels).") + sEol);
                return false;
            }
            m_pDefaultSetup->iMidiChannels = sVal.toInt();
            if (iEqual < 0)
                i++;
        }
        else if (sArg == "-a" || sArg == "--audio-driver") {
        	if (sVal.isNull()) {
                fprintf(stderr, QObject::tr("Option -a requires an argument (audio-driver).") + sEol);
                return false;
            }
            m_pDefaultSetup->sAudioDriver = sVal;
            if (iEqual < 0)
                i++;
        }
        else if (sArg == "-j" || sArg == "--connect-jack-outputs") {
            m_pDefaultSetup->bJackAutoConnect = true;
        }
        else if (sArg == "-L" || sArg == "--audio-channels") {
        	if (sVal.isNull()) {
                fprintf(stderr, QObject::tr("Option -L requires an argument (audio-channels).") + sEol);
                return false;
            }
            m_pDefaultSetup->iAudioChannels = sVal.toInt();
            if (iEqual < 0)
                i++;
        }
        else if (sArg == "-G" || sArg == "--audio-groups") {
        	if (sVal.isNull()) {
                fprintf(stderr, QObject::tr("Option -G requires an argument (audio-groups).") + sEol);
                return false;
         }
            m_pDefaultSetup->iAudioGroups = sVal.toInt();
            if (iEqual < 0)
                i++;
        }
        else if (sArg == "-z" || sArg == "--audio-bufsize") {
        	if (sVal.isNull()) {
                fprintf(stderr, QObject::tr("Option -z requires an argument (audio-bufsize).") + sEol);
                return false;
            }
            m_pDefaultSetup->iAudioBufSize = sVal.toInt();
            if (iEqual < 0)
                i++;
        }
        else if (sArg == "-c" || sArg == "--audio-bufcount") {
        	if (sVal.isNull()) {
                fprintf(stderr, QObject::tr("Option -c requires an argument (audio-bufcount).") + sEol);
                return false;
            }
            m_pDefaultSetup->iAudioBufCount = sVal.toInt();
            if (iEqual < 0)
                i++;
        }
        else if (sArg == "-r" || sArg == "--sample-rate") {
        	if (sVal.isNull()) {
                fprintf(stderr, QObject::tr("Option -r requires an argument (sample-rate).") + sEol);
                return false;
            }
            m_pDefaultSetup->fSampleRate = sVal.toFloat();
            if (iEqual < 0)
                i++;
        }
        else if (sArg == "-R" || sArg == "--reverb") {
        	if (sVal.isNull()) {
                fprintf(stderr, QObject::tr("Option -R requires an argument (reverb).") + sEol);
                return false;
            }
            m_pDefaultSetup->bReverbActive = !(sVal == "0" || sVal == "no" || sVal == "off");
            if (iEqual < 0)
                i++;
        }
        else if (sArg == "-C" || sArg == "--chorus") {
        	if (sVal.isNull()) {
                fprintf(stderr, QObject::tr("Option -C requires an argument (chorus).") + sEol);
                return false;
            }
            m_pDefaultSetup->bChorusActive = !(sVal == "0" || sVal == "no" || sVal == "off");
            if (iEqual < 0)
                i++;
        }
        else if (sArg == "-g" || sArg == "--gain") {
        	if (sVal.isNull()) {
                fprintf(stderr, QObject::tr("Option -g requires an argument (gain).") + sEol);
                return false;
            }
            m_pDefaultSetup->fGain = sVal.toFloat();
            if (iEqual < 0)
                i++;
        }
        else if (sArg == "-o" || sArg == "--option") {
        	if (++i >= argc) {
                fprintf(stderr, QObject::tr("Option -o requires an argument.") + sEol);
                return false;
            }
            if (!parse_option(argv[i])) {
                fprintf(stderr, QObject::tr("Option -o failed to set") + " \"%s\"." + sEol, argv[i]);
                return false;
            }
        }
        else if (sArg == "-s" || sArg == "--server") {
            m_pDefaultSetup->bServer = true;
        }
        else if (sArg == "-i" || sArg == "--no-shell") {
            // Just ignore this...
        }
        else if (sArg == "-d" || sArg == "--dump") {
            m_pDefaultSetup->bMidiDump = true;
        }
        else if (sArg == "-v" || sArg == "--verbose") {
            m_pDefaultSetup->bVerbose = true;
        }
        else if (sArg == "-h" || sArg == "--help") {
            print_usage(argv[0]);
            return false;
        }
        else if (sArg == "-V" || sArg == "--version") {
            fprintf(stderr, "Qt: %s\n", qVersion());
            fprintf(stderr, "qsynth: %s\n", QSYNTH_VERSION);
            return false;
        }
        else if (::fluid_is_soundfont(argv[i])) {
            if (++iSoundFontOverride == 1)
                m_pDefaultSetup->soundfonts.clear();
            m_pDefaultSetup->soundfonts.append(argv[i]);
        }
        else if (::fluid_is_midifile(argv[i])) {
            m_pDefaultSetup->midifiles.append(argv[i]);
        }
        else {
            fprintf(stderr, QObject::tr("Unknown option") + " %s" + sEol, argv[i]);
            print_usage(argv[0]);
            return false;
        }
    }

    // Alright with argument parsing.
    return true;
}

//---------------------------------------------------------------------------
// A recursive QSettings key entry remover.

void qsynthOptions::deleteKey ( const QString& sKey )
{
    // First, delete all stand-alone entries...
    QStringList entries = m_settings.entryList(sKey);
    for (QStringList::Iterator entry = entries.begin(); entry != entries.end(); ++entry)
        m_settings.removeEntry(sKey + "/" + *entry);
        
    // Then, we'll recurse under sub-keys...
    QStringList subkeys = m_settings.subkeyList(sKey);
    for (QStringList::Iterator subkey = subkeys.begin(); subkey != subkeys.end(); ++subkey)
        deleteKey(sKey + "/" + *subkey);
        
    // Finally we remove our-selves.
    m_settings.removeEntry(sKey);
}


//---------------------------------------------------------------------------
// Engine entry management methods.

void qsynthOptions::newEngine ( qsynthEngine *pEngine )
{
    if (pEngine == NULL)
        return;
    if (pEngine->isDefault())
        return;

    const QString& sName = pEngine->name();
    if (engines.find(sName) == engines.end())
        engines.append(sName);
}


void qsynthOptions::deleteEngine ( qsynthEngine *pEngine )
{
    if (pEngine == NULL)
        return;
    if (pEngine->isDefault())
        return;

    const QString& sName = pEngine->name();
    engines.remove(sName);
    
    deleteKey("/Engine/" + sName);
}


//---------------------------------------------------------------------------
// Setup registry methods.

// Load instance m_settings.
void qsynthOptions::loadSetup ( qsynthSetup *pSetup, const QString& sName )
{
    if (pSetup == NULL)
        return;
        
    // Begin at key group?
    if (!sName.isEmpty())
        m_settings.beginGroup("/Engine/" + sName);

    // Shall we have a default display name.
    QString sDisplayName = sName;
    if (sDisplayName.isEmpty())
        sDisplayName = QObject::tr(QSYNTH_TITLE "1");

    // Load previous/default fluidsynth m_settings...
    m_settings.beginGroup("/Settings");
    pSetup->sDisplayName     = m_settings.readEntry        ("/DisplayName",     sDisplayName);
    pSetup->bMidiIn          = m_settings.readBoolEntry    ("/MidiIn",          true);
    pSetup->sMidiDriver      = m_settings.readEntry        ("/MidiDriver",      "alsa_seq");
    pSetup->iMidiChannels    = m_settings.readNumEntry     ("/MidiChannels",    16);
    pSetup->sAlsaName        = m_settings.readEntry        ("/AlsaName",        "pid");
    pSetup->sAudioDriver     = m_settings.readEntry        ("/AudioDriver",     "jack");
    pSetup->sJackName        = m_settings.readEntry        ("/JackName",        "qsynth");
    pSetup->bJackAutoConnect = m_settings.readBoolEntry    ("/JackAutoConnect", true);
    pSetup->bJackMulti       = m_settings.readBoolEntry    ("/JackMulti",       false);
    pSetup->iAudioChannels   = m_settings.readNumEntry     ("/AudioChannels",   1);
    pSetup->iAudioGroups     = m_settings.readNumEntry     ("/AudioGroups",     1);
    pSetup->iAudioBufSize    = m_settings.readNumEntry     ("/AudioBufSize",    64);
    pSetup->iAudioBufCount   = m_settings.readNumEntry     ("/AudioBufCount",   2);
    pSetup->sSampleFormat    = m_settings.readEntry        ("/SampleFormat",    "16bits");
    pSetup->fSampleRate      = m_settings.readDoubleEntry  ("/SampleRate",      44100.0);
    pSetup->iPolyphony       = m_settings.readNumEntry     ("/Polyphony",       256);
    pSetup->bReverbActive    = m_settings.readBoolEntry    ("/ReverbActive",    true);
    pSetup->fReverbRoom      = m_settings.readDoubleEntry  ("/ReverbRoom",      FLUID_REVERB_DEFAULT_ROOMSIZE);
    pSetup->fReverbDamp      = m_settings.readDoubleEntry  ("/ReverbDamp",      FLUID_REVERB_DEFAULT_DAMP);
    pSetup->fReverbWidth     = m_settings.readDoubleEntry  ("/ReverbWidth",     FLUID_REVERB_DEFAULT_WIDTH);
    pSetup->fReverbLevel     = m_settings.readDoubleEntry  ("/ReverbLevel",     FLUID_REVERB_DEFAULT_LEVEL);
    pSetup->bChorusActive    = m_settings.readBoolEntry    ("/ChorusActive",    true);
    pSetup->iChorusNr        = m_settings.readNumEntry     ("/ChorusNr",        FLUID_CHORUS_DEFAULT_N);
    pSetup->fChorusLevel     = m_settings.readDoubleEntry  ("/ChorusLevel",     FLUID_CHORUS_DEFAULT_LEVEL);
    pSetup->fChorusSpeed     = m_settings.readDoubleEntry  ("/ChorusSpeed",     FLUID_CHORUS_DEFAULT_SPEED);
    pSetup->fChorusDepth     = m_settings.readDoubleEntry  ("/ChorusDepth",     FLUID_CHORUS_DEFAULT_DEPTH);
    pSetup->iChorusType      = m_settings.readNumEntry     ("/ChorusType",      FLUID_CHORUS_DEFAULT_TYPE);
    pSetup->bLadspaActive    = m_settings.readBoolEntry    ("/LadspaActive",    false);
    pSetup->fGain            = m_settings.readDoubleEntry  ("/Gain",            1.0);
    pSetup->bServer          = m_settings.readBoolEntry    ("/Server",          false);
    pSetup->bMidiDump        = m_settings.readBoolEntry    ("/MidiDump",        false);
    pSetup->bVerbose         = m_settings.readBoolEntry    ("/Verbose",         false);
    m_settings.endGroup();

    // Load soundfont list...
    m_settings.beginGroup("/SoundFonts");
    const QString sPrefix = "/SoundFont";
    int i = 0;
    for (;;) {
        QString sSoundFont = m_settings.readEntry(sPrefix + QString::number(++i), QString::null);
        if (sSoundFont.isEmpty())
            break;
        pSetup->soundfonts.append(sSoundFont);
    }
    m_settings.endGroup();

    // Load channel presets list.
    m_settings.beginGroup("/Presets");
    pSetup->sDefPreset = m_settings.readEntry("/DefPreset", "(default)");
    const QString sPresetPrefix = "/Preset";
    int iPreset = 0;
    for (;;) {
        QString sItem = m_settings.readEntry(sPresetPrefix + QString::number(++iPreset), QString::null);
        if (sItem.isEmpty())
            break;
        pSetup->presets.append(sItem);
    }
    m_settings.endGroup();

    // Done with the key group?
    if (!sName.isEmpty())
        m_settings.endGroup();
}


// Save instance m_settings.
void qsynthOptions::saveSetup ( qsynthSetup *pSetup, const QString& sName )
{
    if (pSetup == NULL)
        return;

    // Begin at key group?
    if (!sName.isEmpty())
        m_settings.beginGroup("/Engine/" + sName);

    // Save presets list...
    m_settings.beginGroup("/Presets");
    m_settings.writeEntry("/DefPreset", pSetup->sDefPreset);
    // Save last preset list.
    const QString sPresetPrefix = "/Preset";
    int iPreset = 0;
    for (QStringList::Iterator iter = pSetup->presets.begin(); iter != pSetup->presets.end(); iter++)
        m_settings.writeEntry(sPresetPrefix + QString::number(++iPreset), *iter);
    // Cleanup old entries, if any...
    for (++iPreset; !m_settings.readEntry(sPresetPrefix + QString::number(iPreset)).isEmpty(); iPreset++)
        m_settings.removeEntry(sPresetPrefix + QString::number(iPreset));
    m_settings.endGroup();

    // Save last soundfont list.
    m_settings.beginGroup("/SoundFonts");
    const QString sPrefix = "/SoundFont";
    int i = 0;
    for (QStringList::Iterator iter = pSetup->soundfonts.begin(); iter != pSetup->soundfonts.end(); iter++)
        m_settings.writeEntry(sPrefix + QString::number(++i), *iter);
    // Cleanup old entries, if any...
    for (++i; !m_settings.readEntry(sPrefix + QString::number(i)).isEmpty(); i++)
        m_settings.removeEntry(sPrefix + QString::number(i));
    m_settings.endGroup();

    // Save last fluidsynth m_settings.
    m_settings.beginGroup("/Settings");
    m_settings.writeEntry("/DisplayName",      pSetup->sDisplayName);
    m_settings.writeEntry("/MidiIn",           pSetup->bMidiIn);
    m_settings.writeEntry("/MidiDriver",       pSetup->sMidiDriver);
    m_settings.writeEntry("/MidiChannels",     pSetup->iMidiChannels);
    m_settings.writeEntry("/AlsaName",         pSetup->sAlsaName);
    m_settings.writeEntry("/AudioDriver",      pSetup->sAudioDriver);
    m_settings.writeEntry("/JackName",         pSetup->sJackName);
    m_settings.writeEntry("/JackAutoConnect",  pSetup->bJackAutoConnect);
    m_settings.writeEntry("/JackMulti",        pSetup->bJackMulti);
    m_settings.writeEntry("/AudioChannels",    pSetup->iAudioChannels);
    m_settings.writeEntry("/AudioGroups",      pSetup->iAudioGroups);
    m_settings.writeEntry("/AudioBufSize",     pSetup->iAudioBufSize);
    m_settings.writeEntry("/AudioBufCount",    pSetup->iAudioBufCount);
    m_settings.writeEntry("/SampleFormat",     pSetup->sSampleFormat);
    m_settings.writeEntry("/SampleRate",       pSetup->fSampleRate);
    m_settings.writeEntry("/Polyphony",        pSetup->iPolyphony);
    m_settings.writeEntry("/ReverbActive",     pSetup->bReverbActive);
    m_settings.writeEntry("/ReverbRoom",       pSetup->fReverbRoom);
    m_settings.writeEntry("/ReverbDamp",       pSetup->fReverbDamp);
    m_settings.writeEntry("/ReverbWidth",      pSetup->fReverbWidth);
    m_settings.writeEntry("/ReverbLevel",      pSetup->fReverbLevel);
    m_settings.writeEntry("/ChorusActive",     pSetup->bChorusActive);
    m_settings.writeEntry("/ChorusNr",         pSetup->iChorusNr);
    m_settings.writeEntry("/ChorusLevel",      pSetup->fChorusLevel);
    m_settings.writeEntry("/ChorusSpeed",      pSetup->fChorusSpeed);
    m_settings.writeEntry("/ChorusDepth",      pSetup->fChorusDepth);
    m_settings.writeEntry("/ChorusType",       pSetup->iChorusType);
    m_settings.writeEntry("/LadspaActive",     pSetup->bLadspaActive);
    m_settings.writeEntry("/Gain",             pSetup->fGain);
    m_settings.writeEntry("/Server",           pSetup->bServer);
    m_settings.writeEntry("/MidiDump",         pSetup->bMidiDump);
    m_settings.writeEntry("/Verbose",          pSetup->bVerbose);
    m_settings.endGroup();

    // Done with the key group?
    if (!sName.isEmpty())
        m_settings.endGroup();
}


//---------------------------------------------------------------------------
// Preset management methods.

bool qsynthOptions::loadPreset ( qsynthEngine *pEngine, const QString& sPreset )
{
    if (pEngine == NULL || pEngine->pSynth == NULL)
        return false;

    qsynthSetup *pSetup = pEngine->setup();
    if (pSetup == NULL)
        return false;
    
    QString sSuffix;
    if (sPreset != "(default)" && !sPreset.isEmpty()) {
        sSuffix = "/" + sPreset;
        // Check if on list.
        if (pSetup->presets.find(sPreset) == pSetup->presets.end())
            return false;
    }

    // Begin at key group?
    if (!pEngine->isDefault())
        m_settings.beginGroup("/Engine/" + pEngine->name());

    // Load as current presets.
    const QString sPrefix = "/Chan";
    m_settings.beginGroup("/Preset" + sSuffix);
    int iChannels = ::fluid_synth_count_midi_channels(pEngine->pSynth);
    for (int iChan = 0; iChan < iChannels; iChan++) {
        QString sEntry = m_settings.readEntry(sPrefix + QString::number(iChan + 1), QString::null);
        if (!sEntry.isEmpty() && iChan == sEntry.section(':', 0, 0).toInt()) {
            ::fluid_synth_bank_select(pEngine->pSynth, iChan, sEntry.section(':', 1, 1).toInt());
            ::fluid_synth_program_change(pEngine->pSynth, iChan, sEntry.section(':', 2, 2).toInt());
        }
    }
    m_settings.endGroup();

    // Done with the key group?
    if (!pEngine->isDefault())
        m_settings.endGroup();

    // Recommended to post-stabilize things around.
    ::fluid_synth_program_reset(pEngine->pSynth);

    return true;
}

bool qsynthOptions::savePreset ( qsynthEngine *pEngine, const QString& sPreset )
{
    if (pEngine == NULL || pEngine->pSynth == NULL)
        return false;

    qsynthSetup *pSetup = pEngine->setup();
    if (pSetup == NULL)
        return false;

    QString sSuffix;
    if (sPreset != "(default)" && !sPreset.isEmpty()) {
        sSuffix = "/" + sPreset;
        // Append to list if not already.
        if (pSetup->presets.find(sPreset) == pSetup->presets.end())
            pSetup->presets.prepend(sPreset);
    }

    // Begin at key group?
    if (!pEngine->isDefault())
        m_settings.beginGroup("/Engine/" + pEngine->name());

    // Unload current presets.
    const QString sPrefix = "/Chan";
    m_settings.beginGroup("/Preset" + sSuffix);
    int iChannels = ::fluid_synth_count_midi_channels(pEngine->pSynth);
    int iChan = 0;
    for ( ; iChan < iChannels; iChan++) {
        fluid_preset_t *pPreset = ::fluid_synth_get_channel_preset(pEngine->pSynth, iChan);
        if (pPreset) {
            QString sEntry = QString::number(iChan);
            sEntry += ":";
            sEntry += QString::number(pPreset->get_banknum(pPreset));
            sEntry += ":";
            sEntry += QString::number(pPreset->get_num(pPreset));
            m_settings.writeEntry(sPrefix + QString::number(iChan + 1), sEntry);
        }
    }
    // Cleanup old entries, if any...
    for (++iChan; !m_settings.readEntry(sPrefix + QString::number(iChan)).isEmpty(); iChan++)
        m_settings.removeEntry(sPrefix + QString::number(iChan));
    m_settings.endGroup();

    // Done with the key group?
    if (!pEngine->isDefault())
        m_settings.endGroup();

    return true;
}

bool qsynthOptions::deletePreset ( qsynthEngine *pEngine, const QString& sPreset )
{
    if (pEngine == NULL)
        return false;

    qsynthSetup *pSetup = pEngine->setup();
    if (pSetup == NULL)
        return false;

    QString sPrefix;
    if (!pEngine->isDefault())
        sPrefix = "/Engine/" + pEngine->name();
    QString sSuffix;
    if (sPreset != "(default)" && !sPreset.isEmpty()) {
        sSuffix = "/" + sPreset;
        QStringList::Iterator iter = pSetup->presets.find(sPreset);
        if (iter == pSetup->presets.end())
            return false;
        pSetup->presets.remove(iter);
        m_settings.removeEntry(sPrefix + "/Preset" + sSuffix);
    }
    return true;
}


//---------------------------------------------------------------------------
// Widget geometry persistence helper methods.

void qsynthOptions::loadWidgetGeometry ( QWidget *pWidget )
{
    // Try to restore old form window positioning.
    if (pWidget) {
        QPoint fpos;
        QSize  fsize;
        bool bVisible;
        m_settings.beginGroup("/Geometry/" + QString(pWidget->name()));
        fpos.setX(m_settings.readNumEntry("/x", -1));
        fpos.setY(m_settings.readNumEntry("/y", -1));
        fsize.setWidth(m_settings.readNumEntry("/width", -1));
        fsize.setHeight(m_settings.readNumEntry("/height", -1));
        bVisible = m_settings.readBoolEntry("/visible", false);
        m_settings.endGroup();
        if (fpos.x() > 0 && fpos.y() > 0)
            pWidget->move(fpos);
        if (fsize.width() > 0 && fsize.height() > 0)
            pWidget->resize(fsize);
        else
            pWidget->adjustSize();
        if (bVisible)
            pWidget->show();
    }
}


void qsynthOptions::saveWidgetGeometry ( QWidget *pWidget )
{
    // Try to save form window position...
    // (due to X11 window managers ideossincrasies, we better
    // only save the form geometry while its up and visible)
    if (pWidget) {
        m_settings.beginGroup("/Geometry/" + QString(pWidget->name()));
        bool bVisible = pWidget->isVisible();
        if (bVisible) {
            QPoint fpos  = pWidget->pos();
            QSize  fsize = pWidget->size();
            m_settings.writeEntry("/x", fpos.x());
            m_settings.writeEntry("/y", fpos.y());
            m_settings.writeEntry("/width", fsize.width());
            m_settings.writeEntry("/height", fsize.height());
        }
        m_settings.writeEntry("/visible", bVisible);
        m_settings.endGroup();
    }
}


// end of qsynthOptions.cpp
