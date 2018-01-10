qsynth - A fluidsynth Qt GUI Interface
--------------------------------------

Qsynth is a fluidsynth GUI front-end application written in C++
around the Qt framework using Qt Designer. Eventually it may evolve
into a softsynth management application allowing the user to control
and manage a variety of command line softsynths but for the moment
it wraps the excellent FluidSynth (http://www.fluidsynth.org).

FluidSynth is a command line software synthesiser based on the
Soundfont specification.

Homepage: http://qsynth.sourceforge.net

License: GNU General Public License (GPL)


Requirements
------------

The software requirements for build and runtime are listed as follows:

  Mandatory:

  - Qt framework, C++ class library and tools for
        cross-platform application and UI development
        http://qt.io/

  - fluidsynth, real-time software synthesizer
        based on the SoundFont 2 specifications
        http://www.fluidsynth.org/


Installation
------------

The installation procedure follows the standard for source distributions.
Unpack the tarball and in the extracted source directory:

    ./configure [--prefix=/usr/local]
    make

and optionally as root:

    make install

This procedure will end installing the following files:

    ${prefix}/bin/qsynth
    ${prefix}/share/pixmaps/qsynth.png
    ${prefix}/share/applications/qsynth.desktop
    ${prefix}/share/locale/qsynth_*.qm

Just launch ${prefix}/bin/qsynth and you're off (hopefully).

Note that the default installation path ${prefix} is /usr/local.

If you're checking out from Git, you'll have to prepare the
configure script just before you proceed with the above instructions:

   ./autogen.sh


Configuration
-------------

Qsynth holds its settings and configuration state per user, in a file
located as $HOME/.config/rncbc.org/Qsynth.conf . Normally, there's no
need to edit this file, as it is recreated and rewritten everytime
qsynth is run.


Bugs
----

Plenty still although this is beta software ;)


Support
-------

Qsynth is open source free software. For bug reports, feature requests,
discussion forums, mailling lists, or any other matter related to the
development of this piece of software, please use the Sourceforge
project page (http://sourceforge.net/projects/qsynth).


Acknowledgements
----------------

Qsynth has been created by Rui Nuno Capela, Richard Bown and Chris Cannam
to bring a simple but effective software synthesier front end to the Linux
desktop.  From an original idea by Richard Bown and Chris Cannam to create
an open software synthesiser front end to use in conjunction with Rosegarden
(http://www.sourceforge.net/projects/rosegarden) and other ALSA based
software sequencers.

Inspired by Rui's work on QjackCtl (http://qjackctl.sourceforge.net).

Ebrahim Mayat <ebmayat at mac dot com> also contributed with instructions
to build Qsynth on Mac OSX (see README-OSX). This might be outdated now that
Qsynth has migrated to Qt4, but nevertheless...

Pedro Lopez-Cabanillas is currently the most prominent developer, having
contributed with the awesome knob skins/styles option, the alternate cmake
build system and the Windows(tm) installer bundle. Kudos to Pedro!


Enjoy.

rncbc aka Rui Nuno Capela <rncbc at rncbc dot org>
bownie aka Richard Bown <bownie at bownie dot com>
cannam aka Chris Cannam <cannam at all dash day dash breakfast dot com>
