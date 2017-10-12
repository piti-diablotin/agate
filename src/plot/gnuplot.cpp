/**
 * @file src/gnuplot.cpp
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


#include "plot/gnuplot.hpp"
#include "base/exception.hpp"
#include <cmath>

//
Gnuplot::Gnuplot() :
  _gp(nullptr),
  _header(),
  _buffer(),
  _custom()
{
#ifdef HAVE_GNUPLOT
  std::string bin(GNUPLOT_BIN);
  bin += " -persist";
  _gp.reset(popen(bin.c_str(),"w"));
  if ( _gp.get() == nullptr )
    throw EXCEPTION("Can not pipe gnuplot",ERRABT);
  _header << "reset" << std::endl;
  _header << "set style data line" << std::endl;
  _header << "set autoscale xy" << std::endl;
#else
  throw EXCEPTION("Gnuplot was not found on this computer",ERRABT);
#endif
}

//
Gnuplot::~Gnuplot() {
  int r_val;
  if ( _gp != nullptr ) {
    fprintf(_gp.get(),"%s\n","quit");
    fflush(_gp.get());
    r_val = pclose(_gp.get());
    _gp.release();
    if (r_val == -1) {
      Exception e(EXCEPTION("Can not close gnuplot", ERRABT));
      std::cerr << e.fullWhat() << std::endl;
    }
  }
}

//
void Gnuplot::plot(std::vector<double> x, std::list<std::vector<double>> y, std::list<std::string> labels, std::vector<short> colors) {
  using namespace std;
  std::stringstream total;
  _buffer.clear();
  _buffer.seekp(0);
  _buffer.str("");
  _buffer << "plot ";
  auto it = labels.begin();
  for( unsigned p = 0 ; p < y.size() ; ++p ){
    if ( p < labels.size() ) {
      _buffer << "\"-\" t '" << *it << "'";
      ++it;
    }
    else {
      _buffer <<  "\"-\" t ''";
    }
    if ( p < colors.size() )
      _buffer << " lc " << colors[p];
    _buffer << ", ";
  }

  _buffer.seekp(-2,ios_base::end);
  _buffer << std::endl;

  _buffer.setf(std::ios::scientific,std::ios::floatfield);
  _buffer.precision(14);
  for( auto p = y.begin() ; p != y.end() ; ++p ){
    for ( unsigned i = 0 ; i < min(x.size(),p->size()) ; ++i )
      _buffer << x[i] << " " << (*p)[i] << std::endl;
    _buffer << "e" << std::endl;
  }

  total << _header.str();
  this->addCustom();
  total << _custom.str();
  total << "set xlabel '" << _xlabel << "'" << std::endl;
  total << "set ylabel '" << _ylabel << "'" << std::endl;
  total << "set title '" << _title << "'" << std::endl;
  total << _buffer.str();

  if ( _gp.get() != nullptr ) {
    fprintf(_gp.get(),"%s\n",total.str().c_str());
    fflush(_gp.get());
  }
}

void Gnuplot::plot(std::list<std::pair<std::vector<double>,std::vector<double>>> xy, std::list<std::string> labels, std::vector<short> colors) {
  using namespace std;
  std::stringstream total;
  _buffer.clear();
  _buffer.seekp(0);
  _buffer.str("");
  _buffer << "plot ";
  auto it = labels.begin();
  for( unsigned p = 0 ; p < xy.size() ; ++p ){
    if ( p < labels.size() ) {
      _buffer << "\"-\" t '" << *it << "'";
      ++it;
    }
    else {
      _buffer <<  "\"-\" t ''";
    }
    if ( p < colors.size() )
      _buffer << " lc " << colors[p];
    _buffer << ", ";
  }

  _buffer.seekp(-2,ios_base::end);
  _buffer << std::endl;

  _buffer.setf(std::ios::scientific,std::ios::floatfield);
  _buffer.precision(14);
  for( auto p = xy.begin() ; p != xy.end() ; ++p ){
    for ( unsigned i = 0 ; i < min(p->first.size(),p->second.size()) ; ++i )
      _buffer << p->first[i] << " " << p->second[i] << std::endl;
    _buffer << "e" << std::endl;
  }

  total << _header.str();
  this->addCustom();
  total << _custom.str();
  total << "set xlabel '" << _xlabel << "'" << std::endl;
  total << "set ylabel '" << _ylabel << "'" << std::endl;
  total << "set title '" << _title << "'" << std::endl;
  total << _buffer.str();

  if ( _gp.get() != nullptr ) {
    fprintf(_gp.get(),"%s\n",total.str().c_str());
    fflush(_gp.get());
  }
}

//
void Gnuplot::save(std::string filename) {
  std::stringstream com;
  std::string name = filename+".eps";
  this->dump(com,name);

  if ( _gp.get() != nullptr ) {
    fprintf(_gp.get(),"%s\n",com.str().c_str());
    fflush(_gp.get());
  }
}

//
void Gnuplot::dump(std::ostream& out, std::string& plotname) const {
  out << _header.str() << std::endl;
  out << "set terminal push" << std::endl;
  out << "set terminal postscript enhanced color solid font 24 eps" << std::endl;
  out << "set output \"" << plotname << "\"" << std::endl;
  out << _custom.str() << std::endl;
  out << "set xlabel '" << _xlabel << "'" << std::endl;
  out << "set ylabel '" << _ylabel << "'" << std::endl;
  out << "set title '" << _title << "'" << std::endl;
  out << _buffer.str() << std::endl;
  out << "set terminal pop" << std::endl;

}

void Gnuplot::addCustom() {
  _custom.str("");
  _custom.clear();
  if ( _xrange.set )
    _custom << "set xrange[" << _xrange.min << ":" << _xrange.max << "]" << std::endl;

  if ( _yrange.set )
    _custom << "set yrange[" << _yrange.min << ":" << _yrange.max << "]" << std::endl;

  if ( _xtics.size() > 0 ) {
    if ( _xrange.set )
      _custom << "set xtics " << _xrange.min-1 << "," << _xrange.max+1 << "," << _xrange.max+1 << std::endl;
    for( auto t : _xtics ) {
      _custom << "set xtics add (\"" << t.label << "\" " << t.position << ")" << std::endl;
    }
  }
  if ( _ytics.size() > 0 ) {
    if ( _yrange.set )
      _custom << "set ytics " << _yrange.min-1 << "," << _yrange.max+1 << "," << _yrange.max+1 << std::endl;
    for( auto t : _ytics ) {
      _custom << "set ytics add (\"" << t.label << "\"" << t.position << ")" << std::endl;
    }
  }

  if ( _arrows.size() > 0 ) {
    for( auto a : _arrows ) {
      _custom << "set arrow from " << a.x1 << "," << a.y1 << " to " << a.x2 << "," << a.y2 << (a.head? " head" : " nohead") << std::endl;
    }
  }
}
