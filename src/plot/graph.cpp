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
      gplot->plot(conf.x,conf.y,conf.labels,conf.colors);
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
