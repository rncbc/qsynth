// qsynthOptions.cpp
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

#include "qsynthAbout.h"
#include "qsynthOptions.h"

#include "qsynthEngine.h"

#include <QTextStream>
#include <QComboBox>

#include <QApplication>
#include <QDesktopWidget>


//-------------------------------------------------------------------------
// qsynthOptions - Prototype settings structure.
//

// Constructor.
qsynthOptions::qsynthOptions (void)
	: m_settings(QSYNTH_DOMAIN, QSYNTH_TITLE)
{
	// Create default setup descriptor.
	m_pDefaultSetup = new qsynthSetup();
	// Load previous/default fluidsynth settings...
	loadSetup(m_pDefaultSetup, QString());

	loadOptions();
}


// Default Destructor.
qsynthOptions::~qsynthOptions (void)
{
	saveOptions();

	// Delete default setup descriptor.
	delete m_pDefaultSetup;
	m_pDefaultSetup = nullptr;
}




// Explicit load method.
void qsynthOptions::loadOptions (void)
{
	// Load defaults...
	m_settings.beginGroup("/Defaults");
	sSoundFontDir  = m_settings.value("/SoundFontDir").toString();
	bPresetPreview = m_settings.value("/PresetPreview", false).toBool();
	m_settings.endGroup();

	// Load display options...
	m_settings.beginGroup("/Options");
	sMessagesFont   = m_settings.value("/MessagesFont").toString();
	bMessagesLimit  = m_settings.value("/MessagesLimit", true).toBool();
	iMessagesLimitLines = m_settings.value("/MessagesLimitLines", 1000).toInt();
	bMessagesLog    = m_settings.value("/MessagesLog", false).toBool();
	sMessagesLogPath = m_settings.value("/MessagesLogPath", "qsynth.log").toString();
	bQueryClose     = m_settings.value("/QueryClose", true).toBool();
	bKeepOnTop      = m_settings.value("/KeepOnTop", false).toBool();
	bStdoutCapture  = m_settings.value("/StdoutCapture", true).toBool();
	bOutputMeters   = m_settings.value("/OutputMeters", false).toBool();
	bSystemTray     = m_settings.value("/SystemTray", false).toBool();
	bSystemTrayQueryClose = m_settings.value("/SystemTrayQueryClose", true).toBool();
	bStartMinimized = m_settings.value("/StartMinimized", false).toBool();
	iBaseFontSize   = m_settings.value("/BaseFontSize", 0).toInt();
	iKnobStyle      = m_settings.value("/KnobStyle", 0).toInt();
	iKnobMotion     = m_settings.value("/KnobMotion", 1).toInt();
	m_settings.endGroup();

	// Load custom options...
	m_settings.beginGroup("/Custom");
	sCustomColorTheme = m_settings.value("/ColorTheme").toString();
	sCustomStyleTheme = m_settings.value("/StyleTheme").toString();
	m_settings.endGroup();

	// Load custom additional engines.
	m_settings.beginGroup("/Engines");
	const QString sEnginePrefix = "/Engine%1";
	int iEngine = 0;
	for (;;) {
		QString sItem = m_settings.value(sEnginePrefix.arg(++iEngine)).toString();
		if (sItem.isEmpty())
			break;
		engines.append(sItem);
	}
	m_settings.endGroup();
}


// Explicit save method.
void qsynthOptions::saveOptions (void)
{
	// Make program version available in the future.
	m_settings.beginGroup("/Program");
	m_settings.setValue("/Version", CONFIG_BUILD_VERSION);
	m_settings.endGroup();

	// Save defaults...
	m_settings.beginGroup("/Defaults");
	m_settings.setValue("/SoundFontDir", sSoundFontDir);
	m_settings.setValue("/PresetPreview", bPresetPreview);
	m_settings.endGroup();

	// Save display options...
	m_settings.beginGroup("/Options");
	m_settings.setValue("/MessagesFont", sMessagesFont);
	m_settings.setValue("/MessagesLimit", bMessagesLimit);
	m_settings.setValue("/MessagesLimitLines", iMessagesLimitLines);
	m_settings.setValue("/MessagesLog", bMessagesLog);
	m_settings.setValue("/MessagesLogPath", sMessagesLogPath);
	m_settings.setValue("/QueryClose", bQueryClose);
	m_settings.setValue("/KeepOnTop", bKeepOnTop);
	m_settings.setValue("/StdoutCapture", bStdoutCapture);
	m_settings.setValue("/OutputMeters", bOutputMeters);
	m_settings.setValue("/SystemTray", bSystemTray);
	m_settings.setValue("/SystemTrayQueryClose", bSystemTrayQueryClose);
	m_settings.setValue("/StartMinimized", bStartMinimized);
	m_settings.setValue("/BaseFontSize", iBaseFontSize);
	m_settings.setValue("/KnobStyle", iKnobStyle);
	m_settings.setValue("/KnobMotion", iKnobMotion);
	m_settings.endGroup();

	// Save custom options...
	m_settings.beginGroup("/Custom");
	m_settings.setValue("/ColorTheme", sCustomColorTheme);
	m_settings.setValue("/StyleTheme", sCustomStyleTheme);
	m_settings.endGroup();

	// Save engines list...
	m_settings.beginGroup("/Engines");
	// Save last preset list.
	const QString sEnginePrefix = "/Engine%1";
	int iEngine = 0;
	QStringListIterator iter(engines);
	while (iter.hasNext())
		m_settings.setValue(sEnginePrefix.arg(++iEngine), iter.next());
	// Cleanup old entries, if any...
	while (!m_settings.value(sEnginePrefix.arg(++iEngine)).toString().isEmpty())
		m_settings.remove(sEnginePrefix.arg(iEngine));
	m_settings.endGroup();

	// Save/commit to disk.
	m_settings.sync();
}


// Default instance setup accessor.
qsynthSetup *qsynthOptions::defaultSetup (void)
{
	return m_pDefaultSetup;
}


//-------------------------------------------------------------------------
// Settings accessor.
//

QSettings& qsynthOptions::settings (void)
{
	return m_settings;
}


//-------------------------------------------------------------------------
// Command-line argument stuff. Mostly to mimic fluidsynth CLI.
//

// Help about command line options.
void qsynthOptions::print_usage ( const QString& arg0 )
{
	QTextStream out(stderr);
	const QString sEot = "\n\t";
	const QString sEol = "\n\n";

	out << QObject::tr("Usage: %1"
		" [options] [soundfonts] [midifiles]").arg(arg0) + sEol;
	out << QSYNTH_TITLE " - " + QObject::tr(QSYNTH_SUBTITLE) + sEol;
	out << QObject::tr("Options") + ":" + sEol;
	out << "  -n, --no-midi-in" + sEot +
		QObject::tr("Don't create a midi driver to read MIDI input events [default = yes]") + sEol;
	out << "  -m, --midi-driver=[label]" + sEot +
		QObject::tr("The name of the midi driver to use [oss,alsa,alsa_seq,...]") + sEol;
	out << "  -K, --midi-channels=[num]" + sEot +
		QObject::tr("The number of midi channels [default = 16]") + sEol;
	out << "  -a, --audio-driver=[label]" + sEot +
		QObject::tr("The audio driver [alsa,jack,oss,dsound,...]") + sEol;
	out << "  -j, --connect-jack-outputs" + sEot +
		QObject::tr("Attempt to connect the jack outputs to the physical ports") + sEol;
	out << "  -L, --audio-channels=[num]" + sEot +
		QObject::tr("The number of stereo audio channels [default = 1]") + sEol;
	out << "  -G, --audio-groups=[num]" + sEot +
		QObject::tr("The number of audio groups [default = 1]") + sEol;
	out << "  -z, --audio-bufsize=[size]" + sEot +
		QObject::tr("Size of each audio buffer") + sEol;
	out << "  -c, --audio-bufcount=[count]" + sEot +
		QObject::tr("Number of audio buffers") + sEol;
	out << "  -r, --sample-rate=[rate]" + sEot +
		QObject::tr("Set the sample rate") + sEol;
	out << "  -R, --reverb=[flag]" + sEot +
		QObject::tr("Turn the reverb on or off [1|0|yes|no|on|off, default = on]") + sEol;
	out << "  -C, --chorus=[flag]" + sEot +
		QObject::tr("Turn the chorus on or off [1|0|yes|no|on|off, default = on]") + sEol;
	out << "  -g, --gain=[gain]" + sEot +
		QObject::tr("Set the master gain [0 < gain < 10, default = 0.2]") + sEol;
	out << "  -o, --option [name=value]" + sEot +
		QObject::tr("Define a setting name=value") + sEol;
	out << "  -s, --server" + sEot +
		QObject::tr("Create and start server [default = no]") + sEol;
	out << "  -i, --no-shell" + sEot +
		QObject::tr("Don't read commands from the shell [ignored]") + sEol;
	out << "  -d, --dump" + sEot +
		QObject::tr("Dump midi router events") + sEol;
	out << "  -V, --verbose" + sEot +
		QObject::tr("Print out verbose messages about midi events") + sEol;
	out << "  -h, --help" + sEot +
		QObject::tr("Show help about command line options") + sEol;
	out << "  -v, --version" + sEot +
		QObject::tr("Show version information") + sEol;
}


// Parse command line arguments into fluidsynth settings.
bool qsynthOptions::parse_args ( const QStringList& args )
{
	QTextStream out(stderr);
	const QString sEol = "\n\n";
	const int argc = args.count();
	int iSoundFontOverride = 0;

	for (int i = 1; i < argc; ++i) {

		QString sVal;
		QString sArg = args.at(i);
		const int iEqual = sArg.indexOf('=');
		if (iEqual >= 0) {
			sVal = sArg.right(sArg.length() - iEqual - 1);
			sArg = sArg.left(iEqual);
		}
		else if (i < argc - 1) {
			sVal = args.at(i + 1);
			if (sVal[0] == '-')
				sVal.clear();
		}

		if (sArg == "-n" || sArg == "--no-midi-in") {
			m_pDefaultSetup->bMidiIn = false;
		}
		else if (sArg == "-m" || sArg == "--midi-driver") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -m requires an argument (midi-driver).") + sEol;
				return false;
			}
			m_pDefaultSetup->sMidiDriver = sVal;
			if (iEqual < 0)
				++i;
		}
		else if (sArg == "-K" || sArg == "--midi-channels") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -K requires an argument (midi-channels).") + sEol;
				return false;
			}
			m_pDefaultSetup->iMidiChannels = sVal.toInt();
			if (iEqual < 0)
				++i;
		}
		else if (sArg == "-a" || sArg == "--audio-driver") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -a requires an argument (audio-driver).") + sEol;
				return false;
			}
			m_pDefaultSetup->sAudioDriver = sVal;
			if (iEqual < 0)
				++i;
		}
		else if (sArg == "-j" || sArg == "--connect-jack-outputs") {
			m_pDefaultSetup->bJackAutoConnect = true;
		}
		else if (sArg == "-L" || sArg == "--audio-channels") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -L requires an argument (audio-channels).") + sEol;
				return false;
			}
			m_pDefaultSetup->iAudioChannels = sVal.toInt();
			if (iEqual < 0)
				++i;
		}
		else if (sArg == "-G" || sArg == "--audio-groups") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -G requires an argument (audio-groups).") + sEol;
				return false;
			}
			m_pDefaultSetup->iAudioGroups = sVal.toInt();
			if (iEqual < 0)
				i++;
		}
		else if (sArg == "-z" || sArg == "--audio-bufsize") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -z requires an argument (audio-bufsize).") + sEol;
				return false;
			}
			m_pDefaultSetup->iAudioBufSize = sVal.toInt();
			if (iEqual < 0)
				++i;
		}
		else if (sArg == "-c" || sArg == "--audio-bufcount") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -c requires an argument (audio-bufcount).") + sEol;
				return false;
			}
			m_pDefaultSetup->iAudioBufCount = sVal.toInt();
			if (iEqual < 0)
				++i;
		}
		else if (sArg == "-r" || sArg == "--sample-rate") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -r requires an argument (sample-rate).") + sEol;
				return false;
			}
			m_pDefaultSetup->fSampleRate = sVal.toFloat();
			if (iEqual < 0)
				++i;
		}
		else if (sArg == "-R" || sArg == "--reverb") {
			if (sVal.isEmpty()) {
				m_pDefaultSetup->bReverbActive = true;
			} else {
				m_pDefaultSetup->bReverbActive = !(sVal == "0" || sVal == "no" || sVal == "off");
				if (iEqual < 0)
					++i;
			}
		}
		else if (sArg == "-C" || sArg == "--chorus") {
			if (sVal.isEmpty()) {
				m_pDefaultSetup->bChorusActive = true;
			} else {
				m_pDefaultSetup->bChorusActive = !(sVal == "0" || sVal == "no" || sVal == "off");
				if (iEqual < 0)
					++i;
			}
		}
		else if (sArg == "-g" || sArg == "--gain") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -g requires an argument (gain).") + sEol;
				return false;
			}
			m_pDefaultSetup->fGain = sVal.toFloat();
			if (iEqual < 0)
				++i;
		}
		else if (sArg == "-o" || sArg == "--option") {
			if (++i >= argc) {
				out << QObject::tr("Option -o requires an argument.") + sEol;
				return false;
			}
			m_pDefaultSetup->options.append(args.at(i));
		}
		else if (sArg == "-s" || sArg == "--server") {
			m_pDefaultSetup->bServer = true;
		}
		else if (sArg == "-i" || sArg == "--no-shell") {
			// Just ignore this one...
		}
		else if (sArg == "-d" || sArg == "--dump") {
			m_pDefaultSetup->bMidiDump = true;
		}
		else if (sArg == "-V" || sArg == "--verbose") {
			m_pDefaultSetup->bVerbose = true;
		}
		else if (sArg == "-h" || sArg == "--help") {
			print_usage(args.at(0));
			return false;
		}
		else if (sArg == "-v" || sArg == "--version") {
			out << QString("Qt: %1\n")
				.arg(qVersion());
			out << QString("FluidSynth: %1\n")
				.arg(::fluid_version_str());
			out << QString("%1: %2\n")
				.arg(QSYNTH_TITLE)
				.arg(CONFIG_BUILD_VERSION);
			return false;
		}
		else {
			const QByteArray tmp = args.at(i).toUtf8();
			const char *name = tmp.constData();
			if (::fluid_is_soundfont(name)) {
				if (++iSoundFontOverride == 1) {
					m_pDefaultSetup->soundfonts.clear();
					m_pDefaultSetup->bankoffsets.clear();
				}
				m_pDefaultSetup->soundfonts.append(name);
				m_pDefaultSetup->bankoffsets.append(QString());
			}
			else if (::fluid_is_midifile(name)) {
				m_pDefaultSetup->midifiles.append(name);
			}
			else {
				out << QObject::tr("Unknown option '%1'.").arg(name) + sEol;
				print_usage(args.at(0));
				return false;
			}
		}
	}

	// Alright with argument parsing.
	return true;
}


//---------------------------------------------------------------------------
// Engine entry management methods.

void qsynthOptions::newEngine ( qsynthEngine *pEngine )
{
	if (pEngine == nullptr)
		return;
	if (pEngine->isDefault())
		return;

	const QString& sName = pEngine->name();
	if (!engines.contains(sName))
		engines.append(sName);
}


bool qsynthOptions::renameEngine ( qsynthEngine *pEngine )
{
	if (pEngine == nullptr)
		return false;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == nullptr)
		return false;

	const QString sOldName = pEngine->name();
	const QString sNewName = pSetup->sDisplayName;
	if (sOldName == sNewName)
		return false;

	pEngine->setName(sNewName);

	if (!pEngine->isDefault()) {
		engines = engines.replaceInStrings(sOldName, sNewName);
		m_settings.remove("/Engine/" + sOldName);
	}

	return true;
}


void qsynthOptions::deleteEngine ( qsynthEngine *pEngine )
{
	if (pEngine == nullptr)
		return;
	if (pEngine->isDefault())
		return;

	const QString& sName = pEngine->name();
	int iEngine = engines.indexOf(sName);
	if (iEngine >= 0)
		engines.removeAt(iEngine);

	m_settings.remove("/Engine/" + sName);
}


//---------------------------------------------------------------------------
// Setup registry methods.

// Load instance m_settings.
void qsynthOptions::loadSetup ( qsynthSetup *pSetup, const QString& sName )
{
	if (pSetup == nullptr)
		return;

	// Begin at key group?
	if (!sName.isEmpty())
		m_settings.beginGroup("/Engine/" + sName);

	// Shall we have a default display name.
	QString sDisplayName = sName;
	if (sDisplayName.isEmpty())
		sDisplayName = QSYNTH_TITLE "1";

	// Load previous/default fluidsynth m_settings...
	m_settings.beginGroup("/Settings");
	pSetup->sDisplayName     = m_settings.value("/DisplayName", sDisplayName).toString();
	pSetup->bMidiIn          = m_settings.value("/MidiIn", true).toBool();
#if defined(__APPLE__)
	pSetup->sMidiDriver      = m_settings.value("/MidiDriver", "coremidi").toString();
	pSetup->sAudioDriver     = m_settings.value("/AudioDriver", "coreaudio").toString();
#elif defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
	pSetup->sMidiDriver      = m_settings.value("/MidiDriver", "winmidi").toString();
	pSetup->sAudioDriver     = m_settings.value("/AudioDriver", "dsound").toString();
#elif defined(__OpenBSD__)
	pSetup->sMidiDriver      = m_settings.value("/MidiDriver", "sndio").toString();
	pSetup->sAudioDriver     = m_settings.value("/AudioDriver", "sndio").toString();
#else
	pSetup->sMidiDriver      = m_settings.value("/MidiDriver", "alsa_seq").toString();
	pSetup->sAudioDriver     = m_settings.value("/AudioDriver", "jack").toString();
#endif
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
	pSetup->iAudioBufSize    = m_settings.value("/AudioBufSize", 512).toInt();
	pSetup->iAudioBufCount   = m_settings.value("/AudioBufCount", 8).toInt();
#else
	pSetup->iAudioBufSize    = m_settings.value("/AudioBufSize", 64).toInt();
	pSetup->iAudioBufCount   = m_settings.value("/AudioBufCount", 2).toInt();
#endif
	pSetup->sMidiName        = m_settings.value("/AlsaName", "pid").toString();
	pSetup->sJackName        = m_settings.value("/JackName", "qsynth").toString();
	pSetup->bJackAutoConnect = m_settings.value("/JackAutoConnect", true).toBool();
	pSetup->bJackMulti       = m_settings.value("/JackMulti", false).toBool();
#if defined(__OpenBSD__)
	pSetup->sMidiDevice      = m_settings.value("/MidiDevice", "midithru/0").toString();
#else
	pSetup->sMidiDevice      = m_settings.value("/MidiDevice").toString();
#endif
	pSetup->iMidiChannels    = m_settings.value("/MidiChannels", 16).toInt();
	pSetup->sMidiBankSelect  = m_settings.value("/MidiBankSelect", "gm").toString();
	pSetup->sAudioDevice     = m_settings.value("/AudioDevice").toString();
	pSetup->iAudioChannels   = m_settings.value("/AudioChannels", 1).toInt();
	pSetup->iAudioGroups     = m_settings.value("/AudioGroups", 1).toInt();
	pSetup->sSampleFormat    = m_settings.value("/SampleFormat", "16bits").toString();
	pSetup->fSampleRate      = m_settings.value("/SampleRate", 44100.0).toDouble();
	pSetup->iPolyphony       = m_settings.value("/Polyphony", 256).toInt();
	pSetup->bReverbActive    = m_settings.value("/ReverbActive", true).toBool();
	pSetup->fReverbRoom      = m_settings.value("/ReverbRoom",  QSYNTH_REVERB_DEFAULT_ROOMSIZE).toDouble();
	pSetup->fReverbDamp      = m_settings.value("/ReverbDamp",  QSYNTH_REVERB_DEFAULT_DAMP).toDouble();
	pSetup->fReverbWidth     = m_settings.value("/ReverbWidth", QSYNTH_REVERB_DEFAULT_WIDTH).toDouble();
	pSetup->fReverbLevel     = m_settings.value("/ReverbLevel", QSYNTH_REVERB_DEFAULT_LEVEL).toDouble();
	pSetup->bChorusActive    = m_settings.value("/ChorusActive", true).toBool();
	pSetup->iChorusNr        = m_settings.value("/ChorusNr",    QSYNTH_CHORUS_DEFAULT_N).toInt();
	pSetup->fChorusLevel     = m_settings.value("/ChorusLevel", QSYNTH_CHORUS_DEFAULT_LEVEL).toDouble();
	pSetup->fChorusSpeed     = m_settings.value("/ChorusSpeed", QSYNTH_CHORUS_DEFAULT_SPEED).toDouble();
	pSetup->fChorusDepth     = m_settings.value("/ChorusDepth", QSYNTH_CHORUS_DEFAULT_DEPTH).toDouble();
	pSetup->iChorusType      = m_settings.value("/ChorusType",  QSYNTH_CHORUS_DEFAULT_TYPE).toInt();
	pSetup->bLadspaActive    = m_settings.value("/LadspaActive", false).toBool();
	pSetup->fGain            = m_settings.value("/Gain", QSYNTH_MASTER_DEFAULT_GAIN).toDouble();
	pSetup->bServer          = m_settings.value("/Server", false).toBool();
	pSetup->bMidiDump        = m_settings.value("/MidiDump", false).toBool();
	pSetup->bVerbose         = m_settings.value("/Verbose", false).toBool();
	m_settings.endGroup();

	// Load soundfont list...
	m_settings.beginGroup("/SoundFonts");
	const QString sSoundFontPrefix  = "/SoundFont%1";
	const QString sBankOffsetPrefix = "/BankOffset%1";
	int i = 0;
	for (;;) {
		++i;
		QString sSoundFont  = m_settings.value(sSoundFontPrefix.arg(i)).toString();
		QString sBankOffset = m_settings.value(sBankOffsetPrefix.arg(i)).toString();
		if (sSoundFont.isEmpty())
				break;
		pSetup->soundfonts.append(sSoundFont);
		pSetup->bankoffsets.append(sBankOffset);
	}
	m_settings.endGroup();

	// Load channel presets list.
	m_settings.beginGroup("/Presets");
	pSetup->sDefPreset = m_settings.value("/DefPreset", pSetup->sDefPresetName).toString();
	const QString sPresetPrefix = "/Preset%1";
	int iPreset = 0;
	for (;;) {
		QString sItem = m_settings.value(sPresetPrefix.arg(++iPreset)).toString();
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
	if (pSetup == nullptr)
		return;

	// Begin at key group?
	if (!sName.isEmpty())
		m_settings.beginGroup("/Engine/" + sName);

	// Save presets list...
	m_settings.beginGroup("/Presets");
	m_settings.setValue("/DefPreset", pSetup->sDefPreset);
	// Save last preset list.
	const QString sPresetPrefix = "/Preset%1";
	int iPreset = 0;
	QStringListIterator iter(pSetup->presets);
	while (iter.hasNext())
		m_settings.setValue(sPresetPrefix.arg(++iPreset), iter.next());
	// Cleanup old entries, if any...
	while (!m_settings.value(sPresetPrefix.arg(++iPreset)).toString().isEmpty())
		m_settings.remove(sPresetPrefix.arg(iPreset));
	m_settings.endGroup();

	// Save last soundfont list.
	m_settings.beginGroup("/SoundFonts");
	const QString sSoundFontPrefix  = "/SoundFont%1";
	const QString sBankOffsetPrefix = "/BankOffset%1";
	int i = 0;
	QStringListIterator sfiter(pSetup->soundfonts);
	while (sfiter.hasNext()) {
		m_settings.setValue(sSoundFontPrefix.arg(++i), sfiter.next());
		m_settings.setValue(sBankOffsetPrefix.arg(i), pSetup->bankoffsets[i - 1]);
	}
	// Cleanup old entries, if any...
	for (;;) {
		if (m_settings.value(sSoundFontPrefix.arg(++i)).toString().isEmpty())
				break;
		m_settings.remove(sSoundFontPrefix.arg(i));
		m_settings.remove(sBankOffsetPrefix.arg(i));
	}
	m_settings.endGroup();

	// Save last fluidsynth m_settings.
	m_settings.beginGroup("/Settings");
	m_settings.setValue("/DisplayName",      pSetup->sDisplayName);
	m_settings.setValue("/MidiIn",           pSetup->bMidiIn);
	m_settings.setValue("/MidiDriver",       pSetup->sMidiDriver);
	m_settings.setValue("/MidiDevice",       pSetup->sMidiDevice);
	m_settings.setValue("/MidiChannels",     pSetup->iMidiChannels);
	m_settings.setValue("/AlsaName",         pSetup->sMidiName);
	m_settings.setValue("/MidiBankSelect",   pSetup->sMidiBankSelect);
	m_settings.setValue("/AudioDriver",      pSetup->sAudioDriver);
	m_settings.setValue("/AudioDevice",      pSetup->sAudioDevice);
	m_settings.setValue("/JackName",         pSetup->sJackName);
	m_settings.setValue("/JackAutoConnect",  pSetup->bJackAutoConnect);
	m_settings.setValue("/JackMulti",        pSetup->bJackMulti);
	m_settings.setValue("/AudioChannels",    pSetup->iAudioChannels);
	m_settings.setValue("/AudioGroups",      pSetup->iAudioGroups);
	m_settings.setValue("/AudioBufSize",     pSetup->iAudioBufSize);
	m_settings.setValue("/AudioBufCount",    pSetup->iAudioBufCount);
	m_settings.setValue("/SampleFormat",     pSetup->sSampleFormat);
	m_settings.setValue("/SampleRate",       pSetup->fSampleRate);
	m_settings.setValue("/Polyphony",        pSetup->iPolyphony);
	m_settings.setValue("/ReverbActive",     pSetup->bReverbActive);
	m_settings.setValue("/ReverbRoom",       pSetup->fReverbRoom);
	m_settings.setValue("/ReverbDamp",       pSetup->fReverbDamp);
	m_settings.setValue("/ReverbWidth",      pSetup->fReverbWidth);
	m_settings.setValue("/ReverbLevel",      pSetup->fReverbLevel);
	m_settings.setValue("/ChorusActive",     pSetup->bChorusActive);
	m_settings.setValue("/ChorusNr",         pSetup->iChorusNr);
	m_settings.setValue("/ChorusLevel",      pSetup->fChorusLevel);
	m_settings.setValue("/ChorusSpeed",      pSetup->fChorusSpeed);
	m_settings.setValue("/ChorusDepth",      pSetup->fChorusDepth);
	m_settings.setValue("/ChorusType",       pSetup->iChorusType);
	m_settings.setValue("/LadspaActive",     pSetup->bLadspaActive);
	m_settings.setValue("/Gain",             pSetup->fGain);
	m_settings.setValue("/Server",           pSetup->bServer);
	m_settings.setValue("/MidiDump",         pSetup->bMidiDump);
	m_settings.setValue("/Verbose",          pSetup->bVerbose);
	m_settings.endGroup();

	// Done with the key group?
	if (!sName.isEmpty())
		m_settings.endGroup();
}


//---------------------------------------------------------------------------
// Preset management methods.

bool qsynthOptions::loadPreset ( qsynthEngine *pEngine, const QString& sPreset )
{
	if (pEngine == nullptr || pEngine->pSynth == nullptr)
		return false;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == nullptr)
		return false;

	QString sSuffix;
	if (sPreset != pSetup->sDefPresetName && !sPreset.isEmpty()) {
		sSuffix = '/' + sPreset;
		// Check if on list.
		if (!pSetup->presets.contains(sPreset))
				return false;
	}

	// Begin at key group?
	if (!pEngine->isDefault())
		m_settings.beginGroup("/Engine/" + pEngine->name());

	// Load as current presets.
#ifdef CONFIG_FLUID_UNSET_PROGRAM
	int iChannelsSet = 0;
#endif
	const QString sPrefix = "/Chan%1";
	m_settings.beginGroup("/Preset" + sSuffix);
	int iChannels = ::fluid_synth_count_midi_channels(pEngine->pSynth);
	for (int iChan = 0; iChan < iChannels; iChan++) {
		QString sEntry = m_settings.value(sPrefix.arg(iChan + 1)).toString();
		if (!sEntry.isEmpty() && iChan == sEntry.section(':', 0, 0).toInt()) {
			::fluid_synth_bank_select(pEngine->pSynth, iChan,
				sEntry.section(':', 1, 1).toInt());
			::fluid_synth_program_change(pEngine->pSynth, iChan,
				sEntry.section(':', 2, 2).toInt());
		#ifdef CONFIG_FLUID_UNSET_PROGRAM
			++iChannelsSet;
		#endif
		}
	#ifdef CONFIG_FLUID_UNSET_PROGRAM
		else ::fluid_synth_unset_program(pEngine->pSynth, iChan);
	#endif
	}
	m_settings.endGroup();

	// Done with the key group?
	if (!pEngine->isDefault())
		m_settings.endGroup();

#ifdef CONFIG_FLUID_UNSET_PROGRAM
	// If there's none channels set (eg. empty/blank preset)
	// then fallback to old default fill up all the channels
	// according to available soundfont stack.
	if (iChannelsSet < 1) {
		int iProg = 0;
		for (int iChan = 0; iChan < iChannels; iChan++) {
			::fluid_synth_bank_select(pEngine->pSynth, iChan, 0);
			::fluid_synth_program_change(pEngine->pSynth, iChan, iProg);
			++iProg;
		}
	}
#endif

	// Recommended to post-stabilize things around.
	::fluid_synth_program_reset(pEngine->pSynth);

	return true;
}

bool qsynthOptions::savePreset ( qsynthEngine *pEngine, const QString& sPreset )
{
	if (pEngine == nullptr || pEngine->pSynth == nullptr)
		return false;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == nullptr)
		return false;

	QString sSuffix;
	if (sPreset != pSetup->sDefPresetName && !sPreset.isEmpty()) {
		sSuffix = '/' + sPreset;
		// Append to list if not already.
		if (!pSetup->presets.contains(sPreset))
				pSetup->presets.prepend(sPreset);
	}

	// Begin at key group?
	if (!pEngine->isDefault())
		m_settings.beginGroup("/Engine/" + pEngine->name());

	// Unload current presets.
	const QString sPrefix = "/Chan%1";
	m_settings.beginGroup("/Preset" + sSuffix);
	int iChannels = ::fluid_synth_count_midi_channels(pEngine->pSynth);
	int iChan = 0;
	for ( ; iChan < iChannels; iChan++) {
	#ifdef CONFIG_FLUID_CHANNEL_INFO
		fluid_synth_channel_info_t info;
		::memset(&info, 0, sizeof(info));
		::fluid_synth_get_channel_info(pEngine->pSynth, iChan, &info);
		if (info.assigned) {
		#ifdef CONFIG_FLUID_BANK_OFFSET
			info.bank += ::fluid_synth_get_bank_offset(pEngine->pSynth, info.sfont_id);
		#endif
			QString sEntry = QString::number(iChan);
			sEntry += ':';
			sEntry += QString::number(info.bank);
			sEntry += ':';
			sEntry += QString::number(info.program);
			m_settings.setValue(sPrefix.arg(iChan + 1), sEntry);
		}
	#else
		fluid_preset_t *pPreset = ::fluid_synth_get_channel_preset(pEngine->pSynth, iChan);
		if (pPreset) {
		#ifdef CONFIG_FLUID_PRESET_GET_BANKNUM
			int iBank = ::fluid_preset_get_banknum(pPreset);
		#else
			int iBank = pPreset->get_banknum(pPreset);
		#endif
		#ifdef CONFIG_FLUID_BANK_OFFSET
			int iSFID = 0;
		#ifdef CONFIG_FLUID_PRESET_GET_SFONT
			fluid_sfont_t *pSoundFont = ::fluid_preset_get_sfont(pPreset);
		#else
			fluid_sfont_t *pSoundFont = pPreset->sfont;
		#endif
			if (pSoundFont) {
			#ifdef CONFIG_FLUID_SFONT_GET_ID
				iSFID = ::fluid_sfont_get_id(pSoundFont);
			#else
				iSFID = pSoundFont->id;
			#endif
			}
			iBank += ::fluid_synth_get_bank_offset(pEngine->pSynth, iSFID);
		#endif
		#ifdef CONFIG_FLUID_PRESET_GET_NUM
			const int iProg = ::fluid_preset_get_num(pPreset);
		#else
			const int iProg = pPreset->get_num(pPreset);
		#endif
			QString sEntry = QString::number(iChan);
			sEntry += ':';
			sEntry += QString::number(iBank);
			sEntry += ':';
			sEntry += QString::number(iProg);
			m_settings.setValue(sPrefix.arg(iChan + 1), sEntry);
		}
	#endif
		else m_settings.remove(sPrefix.arg(iChan + 1));
	}
	// Cleanup old entries, if any...
	while (!m_settings.value(sPrefix.arg(++iChan)).toString().isEmpty())
		m_settings.remove(sPrefix.arg(iChan));
	m_settings.endGroup();

	// Done with the key group?
	if (!pEngine->isDefault())
		m_settings.endGroup();

	return true;
}

bool qsynthOptions::deletePreset ( qsynthEngine *pEngine, const QString& sPreset )
{
	if (pEngine == nullptr)
		return false;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == nullptr)
		return false;

	QString sPrefix;
	if (!pEngine->isDefault())
		sPrefix = "/Engine/" + pEngine->name();
	QString sSuffix;
	if (sPreset != pSetup->sDefPresetName && !sPreset.isEmpty()) {
		sSuffix = "/" + sPreset;
		int iPreset = pSetup->presets.indexOf(sPreset);
		if (iPreset < 0)
				return false;
		pSetup->presets.removeAt(iPreset);
		m_settings.remove(sPrefix + "/Preset" + sSuffix);
	}

	return true;
}


//---------------------------------------------------------------------------
// Combo box history persistence helper implementation.

void qsynthOptions::loadComboBoxHistory ( QComboBox *pComboBox, int iLimit )
{
	const bool bBlockSignals = pComboBox->blockSignals(true);

	// Load combobox list from configuration settings file...
	m_settings.beginGroup("/History/" + pComboBox->objectName());

	if (m_settings.childKeys().count() > 0) {
		pComboBox->setUpdatesEnabled(false);
		pComboBox->setDuplicatesEnabled(false);
		pComboBox->clear();
		for (int i = 0; i < iLimit; ++i) {
			const QString& sText = m_settings.value(
				"/Item" + QString::number(i + 1)).toString();
			if (sText.isEmpty())
				break;
			pComboBox->addItem(sText);
		}
		pComboBox->setUpdatesEnabled(true);
	}

	m_settings.endGroup();

	pComboBox->blockSignals(bBlockSignals);
}


void qsynthOptions::saveComboBoxHistory ( QComboBox *pComboBox, int iLimit )
{
	const bool bBlockSignals = pComboBox->blockSignals(true);

	// Add current text as latest item...
	const QString sCurrentText = pComboBox->currentText();
	int iCount = pComboBox->count();
	for (int i = 0; i < iCount; i++) {
		const QString& sText = pComboBox->itemText(i);
		if (sText == sCurrentText) {
			pComboBox->removeItem(i);
			--iCount;
			break;
		}
	}
	while (iCount >= iLimit)
		pComboBox->removeItem(--iCount);
	pComboBox->insertItem(0, sCurrentText);
	pComboBox->setCurrentIndex(0);
	++iCount;

	// Save combobox list to configuration settings file...
	m_settings.beginGroup("/History/" + pComboBox->objectName());
	for (int i = 0; i < iCount; ++i) {
		const QString& sText = pComboBox->itemText(i);
		if (sText.isEmpty())
			break;
		m_settings.setValue("/Item" + QString::number(i + 1), sText);
	}
	m_settings.endGroup();

	pComboBox->blockSignals(bBlockSignals);
}


//---------------------------------------------------------------------------
// Widget geometry persistence helper methods.

void qsynthOptions::loadWidgetGeometry ( QWidget *pWidget, bool bVisible )
{
	// Try to restore old form window positioning.
	if (pWidget) {
	//	if (bVisible) pWidget->show(); -- force initial exposure?
		m_settings.beginGroup("/Geometry/" + pWidget->objectName());
	#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
		const QByteArray& geometry
			= m_settings.value("/geometry").toByteArray();
		if (geometry.isEmpty()) {
			QWidget *pParent = pWidget->parentWidget();
			if (pParent)
				pParent = pParent->window();
			if (pParent == nullptr)
				pParent = QApplication::desktop();
			if (pParent) {
				QRect wrect(pWidget->geometry());
				wrect.moveCenter(pParent->geometry().center());
				pWidget->move(wrect.topLeft());
			}
		} else {
			pWidget->restoreGeometry(geometry);
		}
	#else//--LOAD_OLD_GEOMETRY
		QPoint wpos;
		QSize  wsize;
		wpos.setX(m_settings.value("/x", -1).toInt());
		wpos.setY(m_settings.value("/y", -1).toInt());
		wsize.setWidth(m_settings.value("/width", -1).toInt());
		wsize.setHeight(m_settings.value("/height", -1).toInt());
		if (wpos.x() > 0 && wpos.y() > 0)
			pWidget->move(wpos);
		if (wsize.width() > 0 && wsize.height() > 0)
			pWidget->resize(wsize);
	#endif
	//	else
	//	pWidget->adjustSize();
		if (!bVisible)
			bVisible = m_settings.value("/visible", false).toBool();
		if (bVisible && !bStartMinimized)
			pWidget->show();
		else
			pWidget->hide();
		m_settings.endGroup();
	}
}


void qsynthOptions::saveWidgetGeometry ( QWidget *pWidget, bool bVisible )
{
	// Try to save form window position...
	// (due to X11 window managers ideossincrasies, we better
	// only save the form geometry while its up and visible)
	if (pWidget) {
		m_settings.beginGroup("/Geometry/" + pWidget->objectName());
	#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
		m_settings.setValue("/geometry", pWidget->saveGeometry());
	#else//--SAVE_OLD_GEOMETRY
		const QPoint& wpos  = pWidget->pos();
		const QSize&  wsize = pWidget->size();
		m_settings.setValue("/x", wpos.x());
		m_settings.setValue("/y", wpos.y());
		m_settings.setValue("/width", wsize.width());
		m_settings.setValue("/height", wsize.height());
	#endif
		if (!bVisible) bVisible = pWidget->isVisible();
		m_settings.setValue("/visible", bVisible && !bStartMinimized);
		m_settings.endGroup();
	}
}


// end of qsynthOptions.cpp

