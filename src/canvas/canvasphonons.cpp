/**
 * @file src/canvasphonons.cpp
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


#include "canvas/canvasphonons.hpp"
#include "hist/histdatadtset.hpp"
#include "base/phys.hpp"
#include "io/ddbabinit.hpp"
#include "plot/gnuplot.hpp"
#include <algorithm>
#include "base/unitconverter.hpp"

//
CanvasPhonons::CanvasPhonons(bool drawing) : CanvasPos(drawing),
  _amplitudeDisplacement(1),
  _displacements(),
  _reference(),
  _supercell(),
  _condensedModes(),
  _qptModes(),
  _ntime(50),
  _originalFile(),
  _ddb(nullptr)
{
  this->nLoop(-2);
  _qptModes = _condensedModes.end();
}

//
CanvasPhonons::CanvasPhonons(CanvasPos &&canvas) : CanvasPos(std::move(canvas)),
  _amplitudeDisplacement(1),
  _displacements(),
  _reference(),
  _supercell(),
  _condensedModes(),
  _qptModes(),
  _ntime(50),
  _originalFile(),
  _ddb(nullptr)
{
  if ( _histdata != nullptr && _histdata->ntimeAvail() > 0 ) {
    _originalFile = _histdata->filename();
    _reference = Dtset(*_histdata.get());
    _histdata.reset(nullptr);
    this->clear();
    _displacements = DispDB(_reference.natom());
    _condensedModes.clear();

  try {
    this->readDdb(_originalFile);
  }
  catch( Exception &e ) {
    std::clog << e.fullWhat() << std::endl;
    _histdata.reset(new HistDataDtset());
    _reference = Dtset();
    this->clear();
    _displacements = DispDB();
  }
  this->buildAnimation();
  }
  this->nLoop(-2);
  _qptModes = _condensedModes.end();
}

//
CanvasPhonons::CanvasPhonons(const CanvasPos &canvas) : CanvasPos(canvas.opengl()),
  _amplitudeDisplacement(1),
  _displacements(),
  _reference(),
  _supercell(),
  _condensedModes(),
  _qptModes(),
  _ntime(50),
  _originalFile(),
  _ddb(nullptr)
{
  if ( canvas.histdata() != nullptr && canvas.histdata()->ntimeAvail() > 0 ) {
    _histdata.reset(nullptr);
    this->clear();
    _originalFile = canvas.histdata()->filename();
    _reference = Dtset(*canvas.histdata());
    _displacements = DispDB(_reference.natom());
    _condensedModes.clear();

    this->readDdb(canvas.histdata()->filename());
    this->buildAnimation();
  }
  this->nLoop(-2);
  _qptModes = _condensedModes.end();
}

//
CanvasPhonons::~CanvasPhonons() {
  ;
}

//
void CanvasPhonons::openFile(const std::string& filename) {
  HistData* hist = nullptr;
  try {
    hist = HistData::getHist(filename,_wait); 
    if ( _status == PAUSE ) _status = UPDATE;
    _histdata.reset(nullptr);
    this->clear();
    _reference = Dtset(*hist);
    _originalFile = hist->filename();
    _displacements = DispDB(_reference.natom());
    _condensedModes.clear();
    bool disploaded = this->readDdb(filename);
    _qptModes = _condensedModes.end();
    this->buildAnimation();
    if ( !disploaded ) 
      throw EXCEPTION("You need to load a DDB file",ERRWAR);
  }
  catch (Exception& e) {
    if ( e.getReturnValue() != ERRWAR ) {
      e.ADD("Could not build reference structure",ERRDIV);
      if ( hist != nullptr ) {
        delete hist;
        hist = nullptr;
      }
    }
    throw e;
  }
}

bool CanvasPhonons::readDdb(const std::string& filename) {
  // Try to read a DDB file
  try { 
    _ddb.reset(Ddb::getDdb(filename));
    /*
     * This is bad because we are not sure the _reference structure
     * is the same as the ddb but if the number of elements is correct 
     * we can assume this is true. The user should know what he does.
     * We cannot do in an other way since qpoints.yaml does not always provide
     * the structure (1.10 at least does not)
     */
    _ddb->buildFrom(_reference); 
    DispDB disp;
    disp.computeFromDDB(*_ddb.get());
    _displacements += disp;
    std::clog << "Displacements loaded" << std::endl;
    if ( _status == PAUSE ) _status = UPDATE;
    return true;
  }
  catch (Exception& e) {
    //e.ADD("Not a DDB file",ERRDIV);
    if ( _ddb != nullptr ) _ddb.reset(nullptr);
    if ( e.getReturnValue() == ERRABT )  
      std::clog << e.fullWhat() << std::endl;
    return false;
  }
}

//
bool CanvasPhonons::selectQpt(geometry::vec3d qpt) {
  using namespace geometry; 
  for ( DispDB::qptTree::iterator myQpt = _condensedModes.begin() ;
      myQpt != _condensedModes.end() ; ++myQpt ) {
    if ( geometry::norm(qpt-myQpt->first) < 1e-6 ) {
      _qptModes = myQpt;
      return true;
    }
  }
  return false;
}


//
void CanvasPhonons::appendFile(const std::string& filename) {
  try {
    DispDB disp;
    disp.readFromFile(filename,_reference.natom());
    _displacements += disp;
    std::clog << "Displacements loaded" << std::endl;
  }
  catch (Exception& e) {
    if ( e.getReturnValue() != ERRDIV )
      std::cerr << e.fullWhat() << std::endl;
    if ( !this->readDdb(filename) ) 
      throw EXCEPTION("You need to load a compatible DDB file",ERRWAR);
  }
  if ( _status == PAUSE ) _status = UPDATE;
}

//
void CanvasPhonons::my_alter(std::string token, std::istringstream &stream) {
  std::ostringstream out;
  bool rebuild = false;

  std::string line;
  size_t pos = stream.tellg();
  std::getline(stream,line);
  stream.clear();
  stream.seekg(pos);
  ConfigParser parser;
  parser.setSensitive(true);
  parser.setContent(line);

  if ( _ddb == nullptr ) 
    throw EXCEPTION("First load a DDB",ERRDIV);
 
  if ( token == "qpt" ) {
    geometry::vec3d qpt;
    std::string x, y, z;
    stream >> x >> y >> z;
    try {
      qpt[0] = utils::parseNumber<double>(x);
      qpt[1] = utils::parseNumber<double>(y);
      qpt[2] = utils::parseNumber<double>(z);
    }
    catch ( Exception &e ) {
      throw EXCEPTION("Unable to parser a number",ERRDIV);
    }
    if ( stream.fail() ) {
      throw EXCEPTION("Give the three component of the qpt vector",ERRDIV);
    }
    if ( !_displacements.hasQpt(qpt) )
      throw EXCEPTION("This qpt has no eigen displacement",ERRDIV);
    if ( !this->selectQpt(qpt) ) {
      _displacements.setQpt(qpt);
      auto it = _condensedModes.insert(
          std::pair<geometry::vec3d,std::vector<DispDB::qMode>>(
            qpt,
            std::vector<DispDB::qMode>()
            )
          );
      _qptModes = it.first;
      out << "Q-point " << qpt[0] << " " << qpt[1] << " " << qpt[2] << " added";
      rebuild = true;
    }
    else 
      out << "Q-point " << qpt[0] << " " << qpt[1] << " " << qpt[2] << " selected";
    throw EXCEPTION(out.str(),ERRCOM);
  }
  else if ( token == "add" ) {
    geometry::vec3d qpt;
    std::string x, y, z;
    stream >> x >> y >> z;
    try {
      qpt[0] = utils::parseNumber<double>(x);
      qpt[1] = utils::parseNumber<double>(y);
      qpt[2] = utils::parseNumber<double>(z);
    }
    catch ( Exception &e ) {
      throw EXCEPTION("Unable to parser a number",ERRDIV);
    }
    if ( stream.fail() ) {
      throw EXCEPTION("Give the three component of the qpt vector",ERRDIV);
    }
    if ( !_displacements.hasQpt(qpt) )
      throw EXCEPTION("This qpt has no eigen displacement",ERRDIV);
    _displacements.setQpt(qpt);
    _condensedModes.insert(
        std::pair<geometry::vec3d,std::vector<DispDB::qMode>>(
          qpt,
          std::vector<DispDB::qMode>()
          )
        );

    std::vector<unsigned> inputModes;
    if ( parser.hasToken("all") )
      for ( int i = 0 ; i < 3*_natom ; ++ i) inputModes.push_back(i);
    else {
      unsigned vib;
      while ( !stream.eof() ) {
        stream >> vib;
        if ( stream.fail() ) break;
        --vib;
        inputModes.push_back(vib);
        if ( vib >= (unsigned) _natom*3 ) {
          throw EXCEPTION("The mode number is wrong",ERRDIV);
        }
      }
    }
    stream.clear();

    for ( unsigned vib : inputModes ) {
      double nrj = _displacements.getEnergyMode(vib);
      DispDB::qMode vibnrj = {vib,_amplitudeDisplacement,nrj};
      auto it = _condensedModes.insert(
          std::pair<geometry::vec3d,std::vector<DispDB::qMode>>(
            qpt,
            std::vector<DispDB::qMode>( 1, vibnrj)
            )
          );
      if ( !it.second ) { // Qpt already exist
        _qptModes = it.first;
        auto qptvib = std::find(_qptModes->second.begin(),_qptModes->second.end(), vibnrj); // Find if the mode is already included.
        if ( qptvib == _qptModes->second.end() ) // If no, add it.
          _qptModes->second.push_back(vibnrj);
      }
    }
    rebuild = true;
    if ( !this->selectQpt(qpt) )
      throw EXCEPTION("Something unexpected happened",ERRDIV);
  }
  else if ( token == "remove" || token == "rm" ) {
    geometry::vec3d qpt;
    std::string x, y, z;
    stream >> x >> y >> z;
    try {
      qpt[0] = utils::parseNumber<double>(x);
      qpt[1] = utils::parseNumber<double>(y);
      qpt[2] = utils::parseNumber<double>(z);
    }
    catch ( Exception &e ) {
      throw EXCEPTION("Unable to parser a number",ERRDIV);
    }
    if ( stream.fail() ) {
      throw EXCEPTION("Give the three component of the qpt vector",ERRDIV);
    }
    if ( _condensedModes.empty() ){
      throw EXCEPTION("Nothing in memory to remove",ERRDIV);
    }
    auto it = _qptModes; // Backup;
    if ( this->selectQpt(qpt) )
      _condensedModes.erase(_qptModes);
    _qptModes = ( it != _qptModes ? it : _condensedModes.begin() );
    rebuild = true;
  }
  else if ( token == "madd" ) {
    if ( _condensedModes.empty() ){
      throw EXCEPTION("First use :add qx qy qz imode to add a qpt",ERRDIV);
    }
    _displacements.setQpt(_qptModes->first);
    auto &myqptmodes = _qptModes->second;

    std::vector<unsigned> inputModes;
    if ( parser.hasToken("all") )
      for ( int i = 0 ; i < 3*_natom ; ++ i) inputModes.push_back(i);
    else {
      unsigned vib;
      while ( !stream.eof() ) {
        stream >> vib;
        if ( stream.fail() ) break;
        --vib;
        inputModes.push_back(vib);
        if ( vib >= (unsigned) _natom*3 ) {
          throw EXCEPTION("The mode number is wrong",ERRDIV);
        }
      }
    }
    stream.clear();

    for ( unsigned vib : inputModes ) {
      double nrj = _displacements.getEnergyMode(vib);
      DispDB::qMode vibnrj {vib,_amplitudeDisplacement,nrj};
      auto it = std::find(myqptmodes.begin(),myqptmodes.end(),vibnrj);
      if ( it == myqptmodes.end() )
        myqptmodes.push_back(vibnrj);
    }
    rebuild = true;
  }
  else if ( token == "mremove" || token == "mrm" ) {
    if ( _condensedModes.empty() ){
      throw EXCEPTION("Nothing in memory to remove",ERRDIV);
    }
    auto &myqptmodes = _qptModes->second;

    while ( !stream.eof() ) {
      unsigned vib;
      stream >> vib;
      if ( stream.fail() ) break;
      --vib;
      if ( vib >= (unsigned)_natom*3 ) {
        throw EXCEPTION("The mode number is wrong",ERRDIV);
      }
      for ( auto it = myqptmodes.begin(); it != myqptmodes.end() ; ++it){
        if ( it->imode == vib ) {
          myqptmodes.erase(it);
          break;
        }
      }
    }
    stream.clear();
    rebuild = true;
  }
  else if ( token == "list" ) {
    UnitConverter eunit(UnitConverter::Ha);
    if ( parser.hasToken("eunit") ) {
      parser.setSensitive(false);
      std::string unit = utils::tolower(parser.getToken<std::string>("eunit"));
      eunit = UnitConverter::getUnit(unit);
    }
      
    for ( auto iqpt = _condensedModes.begin() ; iqpt != _condensedModes.end() ; ++iqpt ) {
      std::sort(iqpt->second.begin(),iqpt->second.end(),
          [](DispDB::qMode v1,DispDB::qMode v2) { return v1.imode < v2.imode; }
          );
      std::cout << "Qpt : " << iqpt->first[0] << "  " << iqpt->first[1] << "  " <<iqpt->first[2] << std::endl;
      std::cout << "  Modes      : ";
      for ( auto imode : iqpt->second ) 
        std::cout << std::setw(12) << imode.imode+1 << "  ";
      std::cout << std::endl;
      std::cout << "  Amplitudes : ";
      for ( auto imode : iqpt->second ) 
        std::cout << std::setw(12) << imode.amplitude << "  ";
      std::cout << std::endl;
      std::cout << "  Energies   : ";
      for ( auto imode : iqpt->second ) 
        std::cout << std::setw(12) << imode.energy*eunit << "  ";
      std::cout << std::endl;
    }
    return;
  }
  else if ( token == "amplitude" ) {
    double amp;
    stream >> amp;
    if ( stream.fail() ) {
      throw EXCEPTION("amplitude is followed by a double precision number",ERRDIV);
    }
    unsigned modif = 0 ;
    auto &myqptmodes = _qptModes->second;
    while ( !stream.eof() ) {
      unsigned vib;
      stream >> vib;
      if ( stream.fail() ) break;
      --vib;
      ++modif;
      if ( vib >= (unsigned)_natom*3 ) {
        throw EXCEPTION("The mode number is wrong",ERRDIV);
      }
      for ( auto it = myqptmodes.begin(); it != myqptmodes.end() ; ++it){
        if ( it->imode == vib ) {
          it->amplitude = amp;
          break;
        }
      }
    }
    if ( modif == 0 )
      _amplitudeDisplacement = amp;
    rebuild = true;
  }
  else if ( token == "ntime" ) {
    unsigned ntime;
    stream >> ntime;
    if ( stream.fail() || ntime < 1 ) {
      throw EXCEPTION("ntime is followed by a strictely positive integer",ERRDIV);
    }
    _ntime = ntime;
    rebuild = true;
  }
  else if ( token == "reset" ) {
    _condensedModes.clear();
    rebuild = true;
  }
  else if ( token == "analyze" || token == "ana" ) {
    std::string filetraj;
    Graph::Config config;

    std::list<std::vector<double>> &y = config.y;
    std::list<std::string> &labels = config.labels;

    stream >> filetraj;
    if ( stream.fail() )
      throw EXCEPTION("You need to provide a filename",ERRDIV);

    std::string normalized;
    stream >> normalized;
    if ( stream.fail() ) stream.clear();
    Supercell::Norming norm = Supercell::Norming::NONE;
    if ( normalized == "normalized") norm = Supercell::Norming::NORMQ;
    if ( normalized == "fullnormalized") norm = Supercell::Norming::NORMALL;

    try {
      config.filename = parser.getToken<std::string>("output");
    }
    catch (Exception &e) {
      config.filename = utils::noSuffix(filetraj)+"_Analysis";
    }

    HistData *trajectory = HistData::getHist(filetraj,true);

    double dt = 1.;
    try {
      std::string tunit = parser.getToken<std::string>("tunit");
      if ( tunit == "fs" ) {
        config.xlabel = "Time [fs]";
        if ( trajectory->ntimeAvail() > 1 ) {
          dt = (trajectory->getTime(1)-trajectory->getTime(0))*phys::atu2fs;
        }
      }
      else if ( tunit == "step" ) {
        config.xlabel = "Time [step]";
      }
      else {
        throw EXCEPTION("Unknow time unit, allowed values fs and step",ERRDIV);
      }
    }
    catch (Exception &e) {
      config.xlabel = "Time [step]";
    }

    config.ylabel = "Mode decomposition ";
    config.ylabel +=  ( norm!=Supercell::Norming::NONE ? "cos(theta)" : "A*cos(theta) [A/sqrt(amu)]" );
    config.title = "Phonon modes analysis";
    config.save = Graph::GraphSave::DATA;

    Supercell superfirst(*trajectory,0);
    try {
      superfirst.findReference(_reference);
    }
    catch (Exception &e) {
      e.ADD("Unable to match reference structure with supercell",ERRDIV);
      throw e;
    }

    unsigned nmodes = 0;
    for ( auto qpt = _condensedModes.begin() ; qpt != _condensedModes.end() ; ++qpt ) {
      std::ostringstream qlabel;
      qlabel << "[" << qpt->first[0] << "," << qpt->first[1] << "," << qpt->first[2] << "] ";
      for ( auto& vib : qpt->second ) {
        labels.push_back(qlabel.str()+utils::to_string(vib.imode+1));
        nmodes++;
      }
    }
    y.resize(nmodes);
    if ( norm!=Supercell::Norming::NONE ) {
      labels.push_back("Norm^2");
      y.resize(nmodes+1);
    }
    for ( auto v = y.begin() ; v != y.end() ; ++v )
      v->resize(trajectory->ntimeAvail());

    config.x.resize(trajectory->ntimeAvail());
    Exception etmp;
#pragma omp parallel for schedule(static), default(shared)
    for ( unsigned itime = 0 ; itime < trajectory->ntimeAvail() ; ++itime ) {
      config.x[itime]=itime*dt;
      Supercell supercell(*trajectory,itime);
      supercell.setReference(superfirst);
      try {
        auto projection = supercell.projectOnModes(_reference,_displacements,_condensedModes, norm);
        //if ( itime == 0) supercell.amplitudes(_reference);
        double norm2 = 0;
        for ( auto p : projection )
          norm2 += p*p;
        projection.push_back(std::sqrt(norm2));
        unsigned imode = 0;
        for ( auto v = y.begin() ; v != y.end() ; ++v )
          v->at(itime) = projection[imode++];
      }
      catch ( Exception &e) {
#pragma omp critical 
        etmp += e;
      }
    }
    if ( etmp.getReturnValue() != 0 ){
      etmp.ADD("Projection may be wrong or incomplete",ERRDIV);
      throw etmp;
    }
    if ( _gplot == nullptr )  {
      _gplot.reset(new Gnuplot);
    }
    Graph::plot(config,_gplot.get());
  }
  else if ( token == "findqpt" ) {
    std::string filetraj;
    Graph::Config config;

    std::list<std::vector<double>> &y = config.y;
    std::list<std::string> &labels = config.labels;

    stream >> filetraj;
    if ( stream.fail() )
      throw EXCEPTION("You need to provide a filename",ERRDIV);

    try {
      config.filename = parser.getToken<std::string>("output");
    }
    catch (Exception &e) {
      config.filename = utils::noSuffix(filetraj)+"_qpt";
    }
    config.ylabel = "Qpt amplitude [A/sqrt(amu)]";
    config.title = "Qpt analysis";
    config.save = Graph::GraphSave::DATA;

    HistData *trajectory = HistData::getHist(filetraj,true);

    double dt = 1.;
    try {
      std::string tunit = parser.getToken<std::string>("tunit");
      if ( tunit == "fs" ) {
        config.xlabel = "Time [fs]";
        if ( trajectory->ntimeAvail() > 1 ) {
          dt = (trajectory->getTime(1)-trajectory->getTime(0))*phys::atu2fs;
        }
      }
      else if ( tunit == "step" ) {
        config.xlabel = "Time [step]";
      }
      else {
        throw EXCEPTION("Unknow time unit, allowed values fs and step",ERRDIV);
      }
    }
    catch (Exception &e) {
      config.xlabel = "Time [step]";
    }

    Supercell superfirst(*trajectory,0);
    try {
      superfirst.findReference(_reference);
    }
    catch (Exception &e) {
      e.ADD("Unable to match reference structure with supercell",ERRDIV);
      throw e;
    }
    auto amp = superfirst.amplitudes(_reference);

    unsigned nqpt = amp.size();
    for ( auto& qpt : amp ) {
      std::ostringstream qlabel;
      qlabel << "[" << qpt[0] << "," << qpt[1] << "," << qpt[2] << "]";
      labels.push_back(qlabel.str());
    }
    //labels.push_back("Norm");
    //y.resize(nmodes+1);
    y.resize(nqpt);
    for ( auto v = y.begin() ; v != y.end() ; ++v )
      v->resize(trajectory->ntimeAvail());

    config.x.resize(trajectory->ntimeAvail());
    Exception etmp;
#pragma omp parallel for schedule(static), default(shared), ordered
    for ( unsigned itime = 0 ; itime < trajectory->ntimeAvail() ; ++itime ) {
      config.x[itime]=itime*dt;
      Supercell supercell(*trajectory,itime);
      supercell.setReference(superfirst);
      try {
        auto myqpt = supercell.amplitudes(_reference);
        unsigned iqpt = 0;
        for ( auto v = y.begin() ; v != y.end() ; ++v )
          v->at(itime) = myqpt[iqpt++][3];
      }
      catch ( Exception &e) {
#pragma omp critical 
        etmp += e;
      }
    }
    if ( etmp.getReturnValue() != 0 ){
      etmp.ADD("Qpt analysis may be wrong or incomplete",ERRDIV);
      throw etmp;
    }
    if ( _gplot == nullptr )  {
      _gplot.reset(new Gnuplot);
    }
    Graph::plot(config,_gplot.get());
  }
  else if ( token == "lin_res_E" ) {
    try {
      double Eamp = parser.getToken<double>("a");
      std::vector<double> Edir = parser.getToken<double>("edir",3);
      _displacements.linearResponseE(Edir,Eamp,*_ddb.get());
      if ( this->selectQpt({{0,0,0}}) ) {
        DispDB::qMode vibnrj {3*(unsigned)_natom,1,0};
        _qptModes->second.push_back(vibnrj);
      }
      else {
        DispDB::qMode vibnrj {3*(unsigned)_natom,1,0};
        _condensedModes.insert(
        std::pair<geometry::vec3d,std::vector<DispDB::qMode>>(
          {{0,0,0}},
          std::vector<DispDB::qMode>(1,vibnrj)
          )
        );
      }
      rebuild = true;
    }
    catch ( Exception &e ) {
      if ( e.getReturnValue() == ConfigParser::ERFOUND ) {
        e.ADD("You need to set both Edir and A",ERRDIV);
        throw e;
      }
      else {
        e.ADD("Unable to parser the line",ERRDIV);
        throw e;
      }
    }
  }
  else if ( token == "dynmat" ) {
    if ( _ddb.get() == nullptr ) 
      throw EXCEPTION("You need to load a DDB first",ERRDIV);
    _ddb->dump(_qptModes->first);
    throw EXCEPTION(std::string("Dynamical matrix written."), ERRCOM);
  }
  else if ( token == "dumpDDB" || token == "dDDB" ) {
    if ( _ddb.get() == nullptr ) 
      throw EXCEPTION("You need to load a DDB first",ERRDIV);
    std::string filename;
    stream >> filename;
    if ( stream.fail() )
      throw EXCEPTION("Please specify a filename",ERRDIV);
    DdbAbinit::dump(*_ddb.get(),filename);
    throw EXCEPTION(std::string("DDB file ")+filename+std::string(" written."), ERRCOM);

  }
  else { 
    CanvasPos::my_alter(token,stream);
    return;
  }
  if ( rebuild ) this->buildAnimation();
}

//
void CanvasPhonons::buildAnimation() {
  // First, find smallest qpt to build the supercell
  geometry::vec3d qpt = {{ 1.0, 1.0, 1.0 }};
  const double tol = 1e-6;

  for ( auto it = _condensedModes.begin() ; it != _condensedModes.end() ; ++it ) {
    if ( it->first[0] > tol && it->first[0] < qpt[0] ) qpt[0] = it->first[0];
    if ( it->first[1] > tol && it->first[1] < qpt[1] ) qpt[1] = it->first[1];
    if ( it->first[2] > tol && it->first[2] < qpt[2] ) qpt[2] = it->first[2];
  }

  HistData *hist;
  if ( _condensedModes.empty() ) {
    hist = new HistDataDtset(_reference);
  }

  else {
    if ( _ntime > 1 ) { // Build an animation
      // Now go through all mode of all qpt and make the displacement
      _drawSpins[3] = true;
      const double dtheta = phys::pi/(double)_ntime;
      _supercell = Supercell(_reference,qpt);
      for ( unsigned itime = 0 ; itime < _ntime ; ++itime ) {
        const double theta = (double) itime*dtheta;
        Supercell supercell(_supercell);
        for ( auto qpt = _condensedModes.begin() ; qpt != _condensedModes.end() ; ++qpt ) {
          for ( auto vib : qpt->second ) {
            supercell.makeDisplacement(qpt->first,_displacements,vib.imode,vib.amplitude,theta);
          }
        }
        if ( itime == 0 ) {
          hist = new HistDataDtset(supercell);
        }
        else {
          HistDataDtset histi(supercell);
          *hist += histi;
        }
      }
    }
    else { //Display vectors
      _drawSpins[3] = false;
      _supercell = Supercell(_reference,qpt);
      for ( auto qpt = _condensedModes.begin() ; qpt != _condensedModes.end() ; ++qpt ) {
        for ( auto vib : qpt->second ) {
          _supercell.arrowDisplacement(qpt->first,_displacements,vib.imode,vib.amplitude);
        }
      }
      hist = new HistDataDtset(_supercell);

    }
  }
  auto save = _octahedra_z;
  this->setHist(*hist);
  if ( _status == PAUSE && _histdata->ntime() > 1 ) _status = START;
  for ( auto z : save )
    this->updateOctahedra(z);
}

//
void CanvasPhonons::help(std::ostream &out) {
  using std::endl;
  using std::setw;
  out << endl << "-- Here are the commands related to phonons mode --" << endl;
  out <<         "   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   " << endl;
  out << setw(40) << ":analyze or :ana filename [[full]normalized]" << setw(59) << "Project the trajectory of filename onto the selected condensed qpt/modes. normalized is for each indivual qpt, fullnormalized is for the global displacement." << endl;
  out << setw(40) << ":amplitude A [imode [imode ...] ]" << setw(59) << "Set the amplitude of the listed modes. If none is present then set the default amplitude" << endl;
  out << setw(40) << ":add qx qy qz imode" << setw(59) << "Freeze the mode imode at the q-pt [qx qy qz]." << endl;
  out << setw(40) << ":a or :append filename" << setw(59) << "Use file filename to get the eigen displacements." << endl;
  out << setw(40) << ":dynmat" << setw(59) << "Dump the dynamial matrix at the current q-point in reduced coordinates into dynmat-qx-qy-qz.out" << endl;
  out << setw(40) << ":dDDB or :dumpDDB filename" << setw(59) << "Dump all the dynamial matrix into Abinit DDB format. WARNING : header is missing !!" << endl;
  out << setw(40) << ":findqpt filename" << setw(59) << "List the square amplitudes of each Qpt in filename" << endl;
  out << setw(40) << ":list" << setw(59) << "List all the q-pt and the related frozen mode." << endl;
  out << setw(40) << ":madd" << setw(59) << "Freeze the mode imode at the selected qpt (or the last added)." << endl;
  out << setw(40) << ":mremove or :mrm" << setw(59) << "Remove the mode imode at the selected qpt (or the last added)." << endl;
  out << setw(40) << ":ntime N" << setw(59) << "Generate the animation for N times." << endl;
  out << setw(40) << ":qpt qx qy qz" << setw(59) << "Select or add the q-pt [qx qy qz]." << endl;
  out << setw(40) << ":remove or :rm qx qy qz" << setw(59) << "Remove the q-pt from the frozen modes." << endl;
  out << setw(40) << ":reset" << setw(59) << "Reset to the initial reference structure." << endl;
  out << setw(40) << ":lin_res_E Edir x y z A amp" << setw(59) << "Calculate linear response to an applied Electric field in dir [x y z] with Amplitude [amp]." <<endl;
  out << "Commands from positions mode are also available." << endl;
}
