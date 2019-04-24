/**
 * @file src/graph.cpp
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


#include "plot/graph.hpp"
#include "base/utils.hpp"
#include "base/exception.hpp"
#include <iomanip>
#include <algorithm>
#include "io/eigparserelectrons.hpp"

//
Graph::Graph() : _xlabel(),
  _ylabel(), 
  _title(), 
  _winTitle(), 
  _xtics(), 
  _ytics(),
  _arrows(),
  _xrange(),
  _yrange()
{
  _xrange.set = false;
  _yrange.set = false;
}

//
Graph::~Graph() {
  ;
}

void Graph::dump(const std::string& filename) const {
  std::ofstream file(filename,std::ios::out);
  size_t pos = filename.find_last_of(".");
  std::string plotname = filename.substr(0,pos)+".ps";
  try { 
    if ( !file ) {
      std::string err_str = "Error opening file " +filename;
      throw EXCEPTION(err_str,ERRDIV);
    }
    this->dump(file,plotname);

    file.close();
  }
  catch (Exception& e) {
    std::string err_str = "Abording writing\n";
    err_str += "File " + filename + "might be wrong, incomplete, or corrupted.";
    e.ADD(err_str,ERRDIV);
    throw e;
  }
  catch (...) {
    std::string err_str = "Something went wrong with the stream.\n";
    err_str += "File " + filename + "might be wrong, incomplete, or corrupted.";
    throw EXCEPTION(err_str, ERRDIV);
  }
}

void Graph::plot(const Config &conf, Graph* gplot) {
  GraphSave localSave = conf.save;

  if ( conf.y.size() > 0 && conf.doSumUp ) utils::sumUp(conf.y,conf.labels,std::cout);

  if ( gplot != nullptr ) {
    gplot->setXLabel(conf.xlabel);
    gplot->setYLabel(conf.ylabel);
    gplot->setTitle(conf.title);
    if ( conf.xy.size() > 0 )
      gplot->plot(conf.xy,conf.labels,conf.colors);
    else
      if ( conf.rgb.size() == 0 ) {
        gplot->plot(conf.x,conf.y,conf.labels,conf.colors);
      }
      else {
        gplot->plot(conf.x,conf.y,conf.rgb,conf.labels);
      }
  }
  else
    localSave = DATA;

  switch ( localSave ) {
    case NONE :
      {
        break;
      }
    case PRINT :
      {
        gplot->save(conf.filename);
        gplot->dump(conf.filename+".plot");
        break;
      }
    case DATA :
      {
        std::ofstream file(conf.filename+".dat",std::ios::out);
        if ( !file )
          throw EXCEPTION("Unable to save file "+conf.filename,ERRABT);
        if ( conf.xy.size() == 0 ) {
          file << "# " << std::setw(20) << conf.xlabel;
          for ( auto& label : conf.labels )
            file << std::setw(22) << label;
          file << std::endl;
          file.precision(14);
          file.setf(std::ios::scientific,std::ios::floatfield);
          file.setf(std::ios::right,std::ios::adjustfield);
          for ( unsigned r = 0 ; r < conf.x.size() ; ++r ) {
            file << std::setw(22) << conf.x[r];
            for ( auto& vec : conf.y ) {
              file << std::setw(22) << vec[r];
            }
            file << std::endl;
          }
        }
        else {
          file << "# ";
          for ( unsigned n = 0 ; n < conf.xy.size() ; ++n ) {
            file << std::setw(n==0 ? 20 : 22) << conf.xlabel;
            auto it = conf.labels.begin();
            std::advance(it,n);
            file << std::setw(22) << *(it);
          }
          file << std::endl;
          file.precision(14);
          file.setf(std::ios::scientific,std::ios::floatfield);
          file.setf(std::ios::right,std::ios::adjustfield);
          std::vector<int> allSize;
          for ( auto it = conf.xy.begin(); it != conf.xy.end() ; ++it )
            allSize.push_back(it->first.size());
          unsigned maxX = *std::max_element(allSize.begin(),allSize.end());

          for ( unsigned r = 0 ; r < maxX ; ++r ) {
            for ( auto curve = conf.xy.begin() ; curve != conf.xy.end(); curve++ )
            {
              if ( r >= curve->first.size() ) {
                file << std::setw(22) << " " << std::setw(22) << " ";
              }
              else {
                file << std::setw(22) << curve->first[r] << std::setw(22) << curve->second[r];
              }

            }
            file << std::endl;
          }
        }
        file.close();
        break;
      }
    default:
      throw EXCEPTION("This is not possible",ERRABT);
  }
}

void Graph::setXRange(double min, double max) {
  _xrange.min = min;
  _xrange.max = max;
  _xrange.set = true;
}

void Graph::setYRange(double min, double max) {
  _yrange.min = min;
  _yrange.max = max;
  _yrange.set = true;
}

void Graph::addXTic(std::string name, double pos){
  tic newTic;
  newTic.position = pos;
  newTic.label = name;
  _xtics.push_back(newTic);
}

void Graph::addYTic(std::string name, double pos){
  tic newTic;
  newTic.position = pos;
  newTic.label = name;
  _ytics.push_back(newTic);
}

void Graph::addArrow(double x1, double y1, double x2, double y2, bool head) {
  arrow arr;
  arr.x1 = x1;
  arr.y1 = y1;
  arr.x2 = x2;
  arr.y2 = y2;
  arr.head = head;
  _arrows.push_back(arr);
}

void Graph::clearCustom() {
  _xrange.set = false;
  _yrange.set = false;
  _xtics.clear();
  _ytics.clear();
  _arrows.clear();
}

void Graph::plotBand(EigParser &eigparser, ConfigParser &parser, Graph* gplot, Graph::GraphSave save) {
  Graph::Config config;
  std::vector<double> &x = config.x;
  std::list<std::vector<double>> &y = config.y;
  std::list<std::string> &labels = config.labels;
  std::vector<short> &colors = config.colors;
  std::string &filename = config.filename;
  std::string &xlabel = config.xlabel;
  std::string &ylabel = config.ylabel;
  std::string &title = config.title;
  bool &doSumUp = config.doSumUp;

  if ( gplot != nullptr )
    gplot->setWinTitle("Band Structure");
  doSumUp = false;
  title = "Band Structure";
  xlabel = "k-path";
  std::clog << std::endl << " -- Band Structure --" << std::endl;

  try {
    filename = parser.getToken<std::string>("output");
  }
  catch (Exception &e) {
    filename = utils::noSuffix(eigparser.getFilename())+"_bandStruct";
  }

  double fermi = 0;
  try {
    fermi = parser.getToken<double>("fermi",ConfigParser::ENERGY);
  }
  catch (Exception &e) {
    if ( e.getReturnValue() != ConfigParser::ERFOUND )
      throw e;
  }
  try {
    std::string strUnit = utils::tolower(parser.getToken<std::string>("eunit"));
    eigparser.setUnit(strUnit);

  }
  catch (Exception &e) {
    if ( e.getReturnValue() != ConfigParser::ERFOUND )
      throw e;
  }

  unsigned ignore = 0;
  try {
    ignore = parser.getToken<double>("ignore");
    if ( ignore >= eigparser.getNband() )
      throw EXCEPTION("ignore should be smaller than the number of bands",ERRDIV);
  }
  catch (Exception &e) {
    if ( e.getReturnValue() != ConfigParser::ERFOUND )
      throw e;
  }
  std::vector<unsigned> ndiv = eigparser.getNdiv();
  std::vector<std::string> kptlabels = eigparser.getLabels();
  try {
    std::string tok1 = parser.getToken<std::string>("ndiv");
    std::vector<std::string> ndivk = utils::explode(tok1,':');
    ndiv.clear();
    unsigned check = 0;
    for ( auto div : ndivk ) {
      int idiv = utils::stoi(div);
      check += idiv;
      ndiv.push_back(idiv);
    }
    //if ( check+1 != eigparser.getPath().size() )
    //  throw EXCEPTION("Sum of all segments is wrong "+utils::to_string(check)+std::string("<>")+utils::to_string(eigparser.getPath().size()-1),ERRDIV);
  }
  catch (Exception &e) {
    if ( e.getReturnValue() != ConfigParser::ERFOUND )
      throw e;
  }
  try {
    std::string tok2 = parser.getToken<std::string>("labels");
    kptlabels = utils::explode(tok2,':');
  }
  catch (Exception &e) {
    if ( e.getReturnValue() != ConfigParser::ERFOUND )
      throw e;
  }
  ylabel = "Energy ["+eigparser.getUnit().str()+"]";

  std::vector<unsigned> projectionUMask;
  bool projection = false;
  if ( parser.hasToken("fatband") ) {
    projection = true;
    try {
      projectionUMask = parser.getToken<unsigned>("fatband",eigparser.getNband());
    }
    catch ( Exception &e ) { 
      if ( e.getReturnValue() & ConfigParser::ERDIM ) {
        auto blabla = e.what("",true);
        auto pos = blabla.find("Could only read ");
        int maxToRead = 0;
        if ( pos != std::string::npos ) {
          std::istringstream sub(blabla.substr(pos+16));
          sub >> maxToRead;
          projectionUMask = parser.getToken<unsigned>("fatband",maxToRead);
        }
      }
    }
  }

  EigParserElectrons* eeig = nullptr;
  if ( (eeig = dynamic_cast<EigParserElectrons*>(&eigparser)) ) {
    std::vector<int> m;
    int lang;
    if ( parser.hasToken("angular") ) {
      lang = parser.getToken<int>("angular");
      if ( parser.hasToken("magnetic") ) {
        try {
          m = parser.getToken<int>("magnetic",2*lang+1);
        }
        catch ( Exception &e ) { 
          if ( e.getReturnValue() & ConfigParser::ERDIM ) {
            auto blabla = e.what("",true);
            auto pos = blabla.find("Could only read ");
            int maxToRead = 0;
            if ( pos != std::string::npos ) {
              std::istringstream sub(blabla.substr(pos+16));
              sub >> maxToRead;
              m = parser.getToken<int>("magnetic",maxToRead);
            }
          }
        }
      }
      else {
        m.resize(2*lang+1);
        int i = -lang;
        for(auto& mm : m )
          mm = (i++);
      }
      eeig->selectLM(lang,m);
    }
    else {
      if ( parser.hasToken("magnetic") ) {
        throw EXCEPTION("You need to specify the angular quantum number first",ERRDIV);
      }
    }
    eeig = nullptr;
  }

  x = eigparser.getPath();
  std::list<std::vector<unsigned>> &projectionsColor = config.rgb;
  for ( unsigned iband = ignore ; iband < eigparser.getNband() ; ++iband ) {
    y.push_back(eigparser.getBand(iband,fermi,1));
    if ( projection )
      projectionsColor.push_back(eigparser.getBandColor(iband,1,projectionUMask));
    colors.push_back(0);
    labels.push_back("");
  }
  if ( eigparser.isPolarized() ) {
    *labels.begin() = "Spin 1";
    labels.push_back("Spin 2");
    for ( unsigned iband = ignore ; iband < eigparser.getNband() ; ++iband ) {
      y.push_back(eigparser.getBand(iband,fermi,2));
      if ( projection )
        projectionsColor.push_back(eigparser.getBandColor(iband,2,projectionUMask));
      colors.push_back(1);
      labels.push_back("");
    }
    labels.pop_back();
  }
  double minval = *std::min_element(y.begin()->begin(),y.begin()->end());
  double maxval = *std::max_element(y.rbegin()->begin(),y.rbegin()->end());
  double min = minval-(maxval-minval)*0.1;
  double max = maxval+(maxval-minval)*0.1;
  std::stringstream tmp;
  if ( gplot != nullptr ) {
    gplot->setXRange(0,eigparser.getLength());
    gplot->setYRange(min,max);
    gplot->addArrow(0,0,eigparser.getLength(),0,false);
    unsigned kptsize = kptlabels.size();
    if ( ndiv.size() > 0 &&  (ndiv.size() == kptsize-1 || kptsize == 0 ) ) {
      if ( kptsize > 0 ) {
        gplot->addXTic(kptlabels[0],0);
      }
      unsigned acc = 0;
      for ( unsigned i = 0 ; i < ndiv.size()-1 ; ++i ) {
        acc += ndiv[i];
        if ( acc >= x.size() )
          throw EXCEPTION("Something is wrong in your ndiv argument",ERRDIV);
        if ( kptsize > 0 ) {
          gplot->addXTic(kptlabels[i+1],x[acc]);
        }
        gplot->addArrow(x[acc],min,x[acc],max,false);
      }
      if ( kptsize > 0 ) {
        gplot->addXTic(*kptlabels.rbegin(),eigparser.getLength());
      }
    }
    else if ( ndiv.size() > 0 &&  ndiv.size() != kptlabels.size()-1 ) {
      throw EXCEPTION("Number of ndiv not compatible with number of labels",ERRDIV);
    }
  }
  if ( save == Graph::GraphSave::DATA ) { 
    eigparser.dump(filename+".dat",EigParser::PRTKPT|EigParser::PRTIKPT|(projection ? EigParser::PRTPROJ : 0));
    save = Graph::GraphSave::NONE;
  }
  config.save = save;
  Graph::plot(config,gplot);
  if ( gplot != nullptr )
    gplot->clearCustom();
}
