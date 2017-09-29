/**
 * @file src/canvas.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
 *
 * This file is part of AbiOut.
 *
 * AbiOut is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AbiOut is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AbiOut.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <iomanip>

#ifdef HAVE_GL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

#include "canvas/canvas.hpp"
#include "base/exception.hpp"
#include "hist/histdataxyz.hpp"
#include "hist/histdatanc.hpp"

//
Canvas::Canvas(bool drawing) :
  _wait(false),
  _light(false),
  _opengl(drawing),
  _ntime(0),
  _tbegin(0),
  _tend(-1),
  _itime(0),
  _nLoop(1),
  _iLoop(0),
  _dir(1),
  _status(PAUSE),
  _translate(),
  _info(),
  _objDraw(TriObj::FILL),
  _histdata(nullptr),
  _gplot(nullptr)
{
  _translate[0] = 1;
  _translate[1] = 1;
  _translate[2] = 1;
#ifdef HAVE_GL
  if ( _opengl )
    this->switchLight();
  //const GLfloat specular[]={0.f,0.f,0.f,0.0f};
  //const GLfloat shininess[]={100};
  //glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
  //glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
#else
  _opengl = false;
  _wait = true;
#endif
}

//
Canvas::Canvas(Canvas&& canvas) :
  _wait(canvas._wait),
  _light(canvas._light),
  _opengl(canvas._opengl),
  _ntime(canvas._ntime),
  _tbegin(canvas._tbegin),
  _tend(canvas._tend),
  _itime(canvas._itime),
  _nLoop(canvas._nLoop),
  _iLoop(canvas._iLoop),
  _dir(canvas._dir),
  _status(canvas._status),
  _translate(),
  _info(canvas._info),
  _objDraw(canvas._objDraw),
  _histdata(canvas._histdata.release()),
  _gplot(canvas._gplot.release())
{
  _translate[0] = canvas._translate[0];
  _translate[1] = canvas._translate[1];
  _translate[2] = canvas._translate[2];
}

//
Canvas::~Canvas() {
  /*
  if ( _histdata != nullptr ) {
    delete _histdata;
    _histdata = nullptr;
  }
  */
}

//
Canvas::Atom::Atom(int id, int typat, float x, float y, float z) :
  _id(id),
  _typat(typat),
  _xcart({{x,y,z}})
{
  ;
}

//
void Canvas::openFile(const std::string& filename) {
  HistData* hist = nullptr;
  this->clear();
  _histdata.reset(nullptr);
  try {
    hist = HistData::getHist(filename,_wait); 
    this->setHist(*hist);
    if ( _status == PAUSE ) _status = UPDATE;
  }
  catch (Exception& e) {
    e.ADD("Could not get a valid HistData object",ERRDIV);
    if ( hist != nullptr ) {
      delete hist;
      hist = nullptr;
      _histdata.reset(hist);
    }
    throw e;
  }
  if ( _histdata->ntime() == 1 ) _status = UPDATE;
}

//
void Canvas::appendFile(const std::string& filename) {
  HistData* hist = nullptr;
  if ( _histdata == nullptr )
    throw EXCEPTION("Before appending a file, open a file with :open", ERRDIV);
  try {
    hist = HistData::getHist(filename,_wait); 
    *_histdata += *hist;
    this->updateHist();
    delete hist;
    if ( _status == PAUSE ) _status = UPDATE;
  }
  catch (Exception& e) {
    e.ADD("Could not get a valid HistData object to append",ERRDIV);
    throw e;
  }
}

//
void Canvas::switchLight(){
  _light = ( _light ? false : true );
#ifdef HAVE_GL
  if ( _opengl ) {
    if ( _light ) {
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
      //glEnable(GL_COLOR_MATERIAL);
      //glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
    }
    else {
      glDisable(GL_LIGHTING);
      glDisable(GL_LIGHT0);
      //glDisable(GL_COLOR_MATERIAL);
    }
  }
#endif
}

//
void Canvas::setNTime(int ntime) {
  if ( ntime == 0 ) return;
  if ( _tbegin >= ntime )
    throw EXCEPTION("time_begin is to large : should be <"+utils::to_string(ntime),ERRDIV);
  else if ( _tbegin < 0 )
    throw EXCEPTION("time_begin negative not allowed",ERRDIV);

  if ( _tend == -1 ) _tend = ntime;
  else if ( _tend <= _tbegin )
    throw EXCEPTION("time_end < time_begin is not allowed",ERRDIV);
  else if ( _tend > ntime ) {
    _tend = ntime;
    Exception e = EXCEPTION("Setting time_end to last time:"+utils::to_string(ntime),ERRWAR);
    std::clog << e.fullWhat() << std::endl;
  }
  _ntime = ntime;
  if ( _itime < _tbegin ) {
    _itime = _tbegin;
    if ( _status == PAUSE ) _status = UPDATE;
  }
}

//
void Canvas::nextFrame(const int count) {
  if ( _status == START ) {
    _itime += _dir*count;
    if ( _itime >= _tend ) {
      if ( ++_iLoop < _nLoop || _nLoop == -2 || _nLoop == -1 ) {
        _itime = ( _nLoop == -2 ? _tend-2 : _tbegin );
        if ( _nLoop == -2 ) _dir = -1;
      }
      else {
        _status = PAUSE;
        _itime = _tend-1;
      }
    }
    else if ( _itime <= _tbegin ) {
      _itime = _tbegin;
      _dir = 1;
    }
  }
}

//
void Canvas::translateX(TransDir trans) {
  switch (trans) {
    case PLUS : ++_translate[0]; break;
    case MINUS: _translate[0] > 1 ? --_translate[0] : _translate[0]; break;
  }
}

//
void Canvas::translateY(TransDir trans) {
  switch (trans) {
    case PLUS : ++_translate[1]; break;
    case MINUS: _translate[1] > 1 ? --_translate[1] : _translate[1]; break;
  }
}

//
void Canvas::translateZ(TransDir trans) {
  switch (trans) {
    case PLUS : ++_translate[2]; break;
    case MINUS: _translate[2] > 1 ? --_translate[2] : _translate[2]; break;
  }
}

//
void Canvas::alter(std::string token, std::istringstream &stream) {
  if ( token == "o" || token == "open" || token == "e" || token == "edit" ) {
    std::string ext;
    stream >> ext;
    try{
      this->openFile(ext);
    }
    catch ( Exception &e ) {
      e.ADD("Unable to load file "+ext,ERRDIV);
      throw e;
    }
  }
  else if ( token == "a" || token == "append" ) {
    std::string ext;
    while (!stream.eof()) {
      stream >> ext;
      try{
        this->appendFile(ext);
      }
      catch ( Exception &e ) {
        e.ADD("Unable to append file "+ext,ERRDIV);
        throw e;
      }
    }
  }
  else if ( token == "d" || token == "dump" ) {
    std::string ext;
    std::ostringstream out;
    try {
      stream >> ext;
      _histdata->dump(ext,_tbegin,_tend);
    }
    catch ( Exception &e ) {
      e.ADD("Unable to dump history "+ext,ERRDIV);
      throw e;
    }
    out << "Dumping to file " << ext << " finished.";
    throw EXCEPTION(out.str(), ERRCOM);
  }
  else if ( token == "dxyz" || token == "dumpxyz" ) {
    std::string ext;
    std::ostringstream out;
    try {
      stream >> ext;
      if ( _histdata.get() != nullptr ) {
        HistDataXYZ::dump(*(_histdata.get()),ext,_tbegin,_tend);
      }
      else {
        throw EXCEPTION("Nothing to dump", ERRCOM);
      }
    }
    catch ( Exception &e ) {
      e.ADD("Unable to dump history "+ext,ERRDIV);
      throw e;
    }
    out << "Dumping to file " << ext << " finished.";
    throw EXCEPTION(out.str(), ERRCOM);
  }
  else if ( token == "dhist" || token == "dumphist" ) {
    std::string ext;
    std::ostringstream out;
    try {
      stream >> ext;
      if ( _histdata.get() != nullptr ) {
        HistDataNC::dump(*(_histdata.get()),ext,_tbegin,_tend);
      }
      else {
        throw EXCEPTION("Nothing to dump", ERRCOM);
      }
    }
    catch ( Exception &e ) {
      e.ADD("Unable to dump history "+ext,ERRDIV);
      throw e;
    }
    out << "Dumping to file " << ext << " finished.";
    throw EXCEPTION(out.str(), ERRCOM);
  }
  else if ( token == "tbegin" || token == "time_begin" ) {
    int save = _tbegin;
    try{
      int step;
      stream >> step;
      if ( !stream.fail() ) {
        _tbegin = step;
      }
      else
        throw EXCEPTION("Time should be an integer", ERRDIV);
      this->setNTime(_ntime);
    }
    catch ( Exception &e ) {
      _tbegin = save;
      throw e;
    }
  }
  else if ( token == "tend" || token == "time_end" ) {
    int save = _tend;
    try{
      int step;
      stream >> step;
      if ( !stream.fail() ) {
        _tend = ( step == -1 ? -1 : step+1); // _tend never reached so add one to reach the user value.
      }
      else
        throw EXCEPTION("Time should be a positive integer", ERRDIV);
      this->setNTime(_ntime);
    }
    catch ( Exception &e ) {
      _tend = save;
      throw e;
    }
  }
  else if ( token == "r" || token == "repeat" ) {
    try{
      int loop;
      stream >> loop;
      if ( !stream.fail() ) {
        this->nLoop(loop);
      }
      else
        throw EXCEPTION("repeat should be an integer", ERRDIV);
    }
    catch ( Exception &e ) {
      throw e;
    }
  }
  else if ( token == "wait" ) {
    bool opt;
    stream >> opt; 
    if ( !stream.fail() ) {
      _wait = opt;
      if ( ! (_opengl || _wait) ) {
        _wait = true;
        throw EXCEPTION("In no X mode, always wait for loading file", ERRCOM);
      }
    }
    else 
      throw EXCEPTION("You need to provide 0 or 1 after wait to disable or enable threading",ERRDIV);
  }
  else 
    this->my_alter(token,stream);
}

void Canvas::help(std::ostream &out) {
  using std::endl;
  using std::setw;
  out << endl << "-- Here are the commands related to all modes --" << endl;
  out <<         "   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   " << endl;
  out << setw(40) << ":a or :append filename" << setw(59) << "Append filename to the current trajectory." << endl;
  out << setw(40) << ":o or :open filename" << setw(59) << "Open the file filename and dispay it." << endl;
  out << setw(40) << ":dump filename" << setw(59) << "Dump the full history in the original format (_HIST,XYZ,...) if available." << endl;
  out << setw(40) << ":dumpxyz filename" << setw(59) << "Dump the full history in the xyz format." << endl;
  out << setw(40) << ":dumphist filename" << setw(59) << "Dump the full history in the _HIST format." << endl;
  out << setw(40) << ":(plot|print|data) FUNCTION" << setw(59) << "Extract information from the MD simulation." << endl;
  out << setw(40) << "" << setw(59) << "\"plot\" display a gnuplot (if available) graph." << endl;
  out << setw(40) << "" << setw(59) << "\"print\" display a gnuplot (if available) graph and save it into a ps file." << endl;
  out << setw(40) << "" << setw(59) << "\"data\" display a gnuplot (if available) graph and save the plotted data into an ascii file." << endl;
  out << setw(40) << "" << setw(59) << "FUNCTION can be :" << endl;
  out << setw(45) << "" << setw(59) << "T: Temperature" << endl;
  out << setw(45) << "" << setw(59) << "P: Pressure" << endl;
  out << setw(45) << "" << setw(59) << "g(r) Rmax dR: pair distribution function with Rmax and dR in Bohr." << endl;
  out << setw(45) << "" << setw(59) << "V: Volume" << endl;
  out << setw(45) << "" << setw(59) << "acell: Lattice parameters" << endl;
  out << setw(45) << "" << setw(59) << "angle: Lattice angles" << endl;
  out << setw(45) << "" << setw(59) << "angle id1 id2 id3: angle between atoms id1 id2 and id3." << endl;
  out << setw(45) << "" << setw(59) << "gyration: The gyration tensor for each type of atom (if PIMD)." << endl;
  out << setw(45) << "" << setw(59) << "distance id1 id2: distance between atoms id1 and id2" << endl;
  out << setw(45) << "" << setw(59) << "stress: Stress tensor" << endl;
  out << setw(45) << "" << setw(59) << "ekin: Kinetic energy" << endl;
  out << setw(45) << "" << setw(59) << "etotal: Total electronic energy" << endl;
  out << setw(45) << "" << setw(59) << "entropy: Electronic entropy" << endl;
  out << setw(45) << "" << setw(59) << "positions (x|y|z)(x|y|z) [cartesian|reduced]: The atomic position in cartedisan or reduced coordinates (default is cartesian) in the chosen plan" << endl;
  out << setw(45) << "" << setw(59) << "msd: Mean Square displacement Function" << endl;
  out << setw(45) << "" << setw(59) << "pacf: Position AutoCorrelation Function" << endl;
  out << setw(45) << "" << setw(59) << "vacf: Velocity AutoCorrelation Function" << endl;
  out << setw(45) << "" << setw(59) << "pdos: Phonon density of States" << endl;
  out << setw(45) << "" << setw(59) << "band filename [fermi FERMI] [ignore I] [ndiv X:X:X...] [labels G:M:R...] [eunit Ha|eV|THz|cm-1]: A band structure FERMI is the fermi energy, I is the number of bands to ignore, X number of point on the first segment labels are the special points and eunit the desired output unit." << endl;
  out << setw(40) << ":r or : repeat number" << setw(59) << "Number of times the animation is display (-1 or 0 for infinite, -2 palindrome)." << endl;
  out << setw(40) << ":tbegin or :time_begin" << setw(59) << "Set the initial time step to use." << endl;
  out << setw(40) << ":tend or :time_end" << setw(59) << "Set the last time step to use (-1 means until the last available time)." << endl;
}
