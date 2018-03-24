# Presentation
This package contains 2 mains utilities. 
The first one is **agate** which a `glfw` based UI and **qagate** which offers the same functionnalities with a `Qt` UI. 

## What you can do with **(q)agate**
Agate is mainly designed to work with the [Abinit](www.abinit.org "Abinit website") DFT code. However, it can perfectly read some [VASP](www.vasp.at "VASP website") files and other commonly used format. 

Agate should help you to visualize in a glance any input file to make sure the structure your simulating is the one you want.
You can in some ways modify the structure and save it into a new file with the format you want.

At the moment the most remarkable functionnalities are for molecular dynamics (MD) simulations and phonon analysis. Agate allows you to extract information of a trajectory to interpret the physics on the system. Several tools are included to make life easier.

## Some features
Here is a small list of quantities that you can extract with **agate** from an MD simulation:
- OpenGL capabilities
- Extract images to make a movie
- Concatenate several simulations in one 
- PIMD aware : centroid, gyration
- NEB/String methode
- Plot functions :
  * Temperature
  * Pression
  * Volume
  * Lattice parameters
  * Stress tensor
  * Electronic energy (DFT energy)
  * Electronic entropy
  * Kinetic energy
  * Positions
  * Pair Distribution Function
  * Mean Square displacements
  * Positions autocorrelation
  * Velocities autocorrelation
  * Phonon density of states
  * Phonons band structures (if tdep from abinit is available)

  Ans some phonon related capabilities:
- Visualize indivual or combined mode(s)
  - Extract dominant **q** vectors
  - Extract indivual mode amplitude
  - Construct your own structure with condensed phonons  

# How to install
  Altough the **abiout** package is available on Windows, MacOS and Linux, only the linux installation procedure will be presented here. For MacOS, you can almost follow the **From the source** procedure.

## Ubuntu 

### Personal Repository
  The PPA contains package for all maintained Ubuntu distributions.
  Simply run 
  ```
  sudo add-apt-repository ppa:piti-diablotin/abiout
  sudo apt-get update
  sudo apt-get install abiout
  ```
  and you are done !
  Simply execute `agate` or `qagate` in a terminal.

### From the source
  First you need to install some dependancies.
  On Ubuntu <= 15.04
  ```
  sudo apt-get install autotools-dev automake autoconf m4 g++ libjpeg8-dev libpng3-dev libnetcdf-dev libnetcdfc++4 libcurl3-dev libfreetype6-dev libglfw-dev libeigen3-dev fontconfig libglu1-mesa-dev wget unzip cmake xorg-dev ttf-ubuntu-font-family libxml2-dev gnuplot-qt libyaml-cpp-dev libboost-dev qtbase5-dev qt5-qmake qt5-default libqt5opengl5-dev libfftw3-dev git
  ```
On Ubuntu >= 16.04 (15.10 not maintained anymore)
  ```
  sudo autotools-dev automake autoconf m4 g++ libjpeg8-dev libpng-dev libnetcdf-dev libnetcdf-c++4-dev libcurl3-dev libfreetype6-dev libglfw3-dev libeigen3-dev fontconfig libglu1-mesa-dev ttf-ubuntu-font-family libxml2-dev gnuplot-qt libyaml-cpp-dev libboost-dev qtbase5-dev qt5-qmake qt5-default libqt5opengl5-dev libfftw3-dev git
  ```
  Then the procedure is the same.
  Compile abiout with
  ```
  git clone https://github.com/piti-diablotin/abiout.git
  cd abiout
  ./autogent.sh
  mkdir build
  cd build
  ../configure --with-qt
  make
  sudo make install
  ```
  That's it.

## Fedora
  It is exactly the same as for Ubuntu with the following packages (Fedora 22)
  ```
  sudo su
  yum install autoconf automake m4 libjpeg-turbo-devel libpng-devel netcdf-devel libcurl-devel freetype-devel glfw-devel eigen3-devel fontconfig wget unzip libxml2-devel yaml-cpp-devel fftw-devel qt5-qtbase-devel gambas3-gb-qt5-opengl qt-devel git gcc-c++ gnuplot
  git clone https://github.com/piti-diablotin/abiout.git
  cd abiout
  ./autogen.sh
  mkdir build
  cd build
  ../configure --with-qt
  make
  make install
  ```

## Windows and MacOS
  You will find in the Win_x86 and OSX directories a ```.exe``` for windows and a ```.dmg```  for MacOS that will install everything needed
  ***WARNING:*** MacOSX version has been updated on March 8th, 2018.
  ***WARNING:*** The windows version is 2 years old and completely outdated !!! Use at your own risk. (WIP for new versions)







