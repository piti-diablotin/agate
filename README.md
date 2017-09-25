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
  sudo apt-get install debhelper autotools-dev automake autoconf m4 libjpeg8-dev libpng3-dev libnetcdf-dev libnetcdfc++4 libcurl3-dev libfreetype6-dev libglfw-dev libeigen3-dev fontconfig libglu1-mesa-dev wget unzip cmake xorg-dev ttf-ubuntu-font-family libxml2-dev gnuplot-qt libyaml-cpp-dev libboost-dev qtbase5-dev qt5-qmake qt5-default libqt5opengl5-dev libfftw3-dev
  ```
  On Ubuntu >= 16.04 (15.10 not maintained anymore)
  ```
  debhelper (>= 8.0.0), autotools-dev, automake, autoconf, m4, libjpeg8-dev, libpng-dev, libnetcdf-dev, libnetcdf-c++4-dev, libcurl3-dev, libfreetype6-dev, libglfw3-dev, libeigen3-dev, fontconfig, libglu1-mesa-dev, ttf-ubuntu-font-family, libxml2-dev, gnuplot-qt, libyaml-cpp-dev, libboost-dev, qtbase5-dev, qt5-qmake, qt5-default, libqt5opengl5-dev, libfftw3-dev
  ```
  Then the procedure is the same.
  Compile spglib if desired with
  ```
  tar xfz spglib-1.9.9.tar.gz
  cd spglib-1.9.9 && mkdir build && cd build && ../configure &&  make && sudo make install
  ```
  Finally compile abiout with
  ```
  ./autogent.sh
  ./configure --with-qt
  make
  sudo make install
  ```
  That's it.
  
## Windows and MacOS
  You will find in the Win_x86 and OSX directories a ```.exe``` for windows and a ```.dmg```  for MacOS that will install everything needed
  ***WARNING:*** Thoses 2 versions are 2 years old and completely outdated !!! Use at your own risk. (WIP for new versions)
  
  
  
  
  
  
  
