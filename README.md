[![Build Status](https://travis-ci.org/piti-diablotin/agate.svg?branch=master)](https://travis-ci.org/piti-diablotin/agate)
# Presentation
Agate is a tool engine to visualize and post-process data from *ab-initio* codes.
It is mainly designed to work with the [Abinit](www.abinit.org "Abinit website") DFT code. However, it can perfectly read some [VASP](www.vasp.at "VASP website") files and other commonly used format. 

Agate should help you to visualize in a glance any input file to make sure the structure you're simulating is the one you want.
You can in some ways modify the structure and save it into a new file with the format you want.

A new Qt GUI is available and is called [qAgate](https://github.com/piti-diablotin/qAgate).

At the moment the most remarkable functionnalities are for molecular dynamics (MD) simulations and phonon analysis. Agate allows you to extract information of a trajectory to interpret the physics on the system. Several tools are included to make life easier.

## Some features
Here is a small list of quantities that you can extract with **agate** from a MD simulation:
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

  And some phonon related capabilities:
- Visualize indivual or combined mode(s)
  - Extract dominant **q** vectors
  - Extract indivual mode amplitude
  - Construct your own structure with condensed phonons  

# How to install
  Altough the package is available on Windows, MacOS and Linux, only the Linux and MacOS under Hombrew procedures will be presented.

## Ubuntu 

### Personal Repository
  The PPA contains package for all maintained Ubuntu distributions.
  Simply run 
  ```
  sudo add-apt-repository ppa:piti-diablotin/abiout
  sudo apt-get update
  sudo apt-get install agate
  ```
  and you are done !
  Simply execute `agate` in a terminal.

### From sources
  First you need to install some dependancies.
  On Ubuntu <= 15.04
  ```
  sudo apt-get install g++ autotools-dev automake autoconf m4 libtool libncurses5-dev libjpeg8-dev libpng3-dev libnetcdf-dev libcurl3-dev libfreetype6-dev libglfw-dev libeigen3-dev fontconfig ttf-ubuntu-font-family libxml2-dev gnuplot-qt libyaml-cpp-dev libfftw3-dev libssh-dev cxxtest git
  ```
On Ubuntu >= 16.04 (15.10 not maintained anymore)
  ```
  sudo apt-get install g++ autotools-dev automake autoconf m4 libtool libncurses5-dev libjpeg8-dev libpng-dev libnetcdf-dev libcurl3-dev libfreetype6-dev libglfw3-dev libeigen3-dev fontconfig ttf-ubuntu-font-family libxml2-dev gnuplot-qt libyaml-cpp-dev libfftw3-dev libssh-dev cxxtest git
  ```
  Then the procedure is the same.
  Compile agate with
  ```
  git clone https://github.com/piti-diablotin/agate.git
  cd agate 
  ./autogen.sh
  mkdir build
  cd build
  ../configure 
  make
  sudo make install
  ```
  That's it.

## Fedora
  It is exactly the same as for Ubuntu with the following packages (Fedora 22)
  ```
  sudo su
  yum install autoconf automake m4 libjpeg-turbo-devel libpng-devel netcdf-devel libcurl-devel freetype-devel glfw-devel eigen3-devel fontconfig wget unzip libxml2-devel yaml-cpp-devel fftw-devel libssh-devel cxxtest git gcc-c++ gnuplot
  git clone https://github.com/piti-diablotin/agate.git
  cd agate 
  ./autogen.sh
  mkdir build
  cd build
  ../configure
  make
  make install
  ```
## MacOS X

### Homebrew
  A tap formula is available. You need first to had the tap.
  You can add the tap from `piti-diablotin` which is the most up-to-date, or the on from `abinit`.
  ```
  brew tap piti-diablotin/tap
  # or
  brew tap abinit/tap
  ```
  Then just run
  ``` 
  brew install agate
  ```
  You can now run `agate` from your terminal.

### From sources
  The procedure has been tested on MacOS only with *Homebrew*. *Macport* should work too.
  ```
  brew install automake autoconf curl cxxtest eigen fftw freetype git glfw gnuplot jpeg libpng libssh libtool libxml2 netcdf yaml-cpp
  git clone https://github.com/piti-diablotin/agate.git
  cd agate 
  ./autogen.sh
  mkdir build
  cd build
  ../configure
  make
  sudo make install
  ```

