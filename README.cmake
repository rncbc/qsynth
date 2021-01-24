What is CMake?
==============

CMake is a cross platform build system, that can be used to replace the old 
auto-tools, providing a nice building environment and advanced features.

Some of these features are:
* Out of sources build: CMake allows you to build your software into a directory 
  different to the source tree. You can safely delete the build directory and 
  all its contents once you are done.
* Multiple generators: classic makefiles can be generated for Unix and MinGW, 
  but also Visual Studio, XCode and Eclipse CDT projects among other types.
* Graphic front-ends for configuration and build options.

More information and documentation is available at the CMake project site:
    http://www.cmake.org

CMake is free software. You can get the sources and pre-compiled packages for
Linux and other systems at:
     http://www.cmake.org/cmake/resources/software.html
     
How to use it?
==============

1. You need CMake 3.10 or newer to build Qsynth

2. Unpack the Qsynth sources somewhere, or checkout the repository, 
   and create a build directory. For instance, using a command line shell:
   
$ tar -xvzf Downloads/qsynth-x.y.z.tar.gz
$ cd qsynth-x.y.z
$ mkdir build

2. Execute CMake from the build directory, providing the source directory 
   location and optionally, the build options. There are several ways.

* From a command line shell:

$ pwd
qsynth-x.y.z
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=debug ..

3. Execute the build command. If you used the Makefiles generator (the default
   in Linux and other Unix systems) then execute make, gmake, or mingw32-make.
   If you generated a project file, use your IDE to build it.
   
Configuration options   
=====================

CMAKE_BUILD_TYPE=Release

    Specifies the build type: size optimized or including debug symbols.

CMAKE_INSTALL_PREFIX=/usr/local

    Install directory prefix that will be used when you call 'make install'.

CMAKE_PREFIX_PATH="$HOME/Qt/5.12.10/gcc_64;$HOME/fluidsynth-2"

    List of directories specifying installation prefixes to be searched for 
    dependencies. This is relevant when you want to use compiled libraries 
    installed on non-standard prefixes, for instance several versions of 
    the Qt libraries or Fluidsynth.

For more information, please see:
    https://cmake.org/cmake/help/v3.10/manual/cmake.1.html
    
Compiling with make
===================

There are many targets available. To see a complete list of them, type:

$ make help

The build process usually hides the compiler command lines, to show them:

$ make VERBOSE=1

There is a "clean" target, but not a "distclean" one. You should use a build
directory different to the source tree. In this case, the "distclean" target 
would be equivalent to simply removing the build directory. 

Compiling on Windows and macOS
==============================

You can use CMake to build Qsynth on operating systems different from Linux.
You just need to provide the locations of the dependencies using 
CMAKE_PREFIX_PATH.

You can download precompiled Qt libraries for Windows and macOS from: 
    https://www.qt.io/download 

There are precompiled Fluidsynth packages for Windows here:
    https://github.com/FluidSynth/fluidsynth/releases

The precompiled Fluidsynth library have been created using MinGW, so it is
recommended to use the same compiler to build Qsynth.

You can compile Fluidsynth for macOS yourself, or get it from Homebrew:
    https://formulae.brew.sh/formula/fluid-synth
    
After a successful build, you have a qsynth.exe program in Windows, and a
qsynth.app bundle on macOS. If you want to create a deployment package...

On macOS, run the bash script 'creadmg.sh', to build a disk image (dmg) 
from the build directory where 'qsynth.app' is located.

On windows, compile the NSIS script 'setup.nsi' that is produced in the 
build directory. You need the NSIS program from:
    https://nsis.sourceforge.io/Download

If something fails
==================

If there is an error message while executing CMake, this probably means that a
required package is missing in your system. You should install the missing 
component and run CMake again.

If there is an error executing the build process, after running a flawless CMake
configuration process, this means that there may be an error in the source code, 
or in the build system, or something incompatible in 3rd party libraries.

The CMake build system for Qsynth is experimental. It will take a while
until it becomes stable and fully tested. You can help providing feedback, 
please send a report containing your problems to the Qsynth development 
mailing list, https://lists.sourceforge.net/mailman/listinfo/qsynth-devel
