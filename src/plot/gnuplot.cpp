/**
 * @file src/gnuplot.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
 *
 * This file is part of Agate.
 *
 * Agate is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Agate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Agate.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "plot/gnuplot.hpp"
#include "base/exception.hpp"
#include <cmath>
#include <regex>
#include <map>

//
Gnuplot::Gnuplot() : Graph(),
  _gp(nullptr),
  _header(),
  _buffer(),
  _custom()
{
  _gp.reset(popen("gnuplot","w"));
  if ( _gp.get() == nullptr )
    throw EXCEPTION("Unable to open pipe for gnuplot",ERRABT);
  _header << "reset" << std::endl;
  _header << "set style data line" << std::endl;
  _header << "set autoscale xy" << std::endl;
  _header << "set grid" << std::endl;
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
void Gnuplot::plot(const std::vector<double> &x, const std::list<std::vector<double>> &y, const std::list<std::string> &labels, const std::vector<unsigned> &colors) {
  using namespace std;
  std::stringstream total;
  _buffer.clear();
  _buffer.seekp(0);
  _buffer.str("");
  _buffer << "plot ";
  auto it = labels.begin();
  for( unsigned p = 0 ; p < y.size() ; ++p ){
    if ( p < labels.size() ) {
      _buffer << "\"-\" t '" << *it << "' lw 3";
      ++it;
    }
    else {
      _buffer <<  "\"-\" t '' lw 3";
    }
    if ( p < colors.size() && colors[p] != (unsigned) -1 )
      _buffer << " lc rgbcolor " << colors[p];
    else
      _buffer << " lc rgb \"" << HTMLcolor[p%15] << "\"";
    _buffer << ", ";
  }

  _buffer.seekp(-2,ios_base::end);
  _buffer << std::endl;
  _buffer.str(translateToSymbol(_buffer.str()));
  _buffer.seekp(0,ios_base::end);

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

void Gnuplot::plot(const std::vector<double> &x, const std::list<std::vector<double>> &y, const std::list<std::vector<unsigned>> &c, const std::list<std::string> &labels) {
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
    if ( p < c.size() )
      _buffer << " lc rgb variable lw 3";
    else
      _buffer << " lc rgb \"" << HTMLcolor[p%15] << "\"";
    _buffer << ", ";
  }

  _buffer.seekp(-2,ios_base::end);
  _buffer << std::endl;
  _buffer.str(translateToSymbol(_buffer.str()));
  _buffer.seekp(0,ios_base::end);

  _buffer.setf(std::ios::scientific,std::ios::floatfield);
  _buffer.precision(14);
  auto color = c.begin();
  for( auto p = y.begin() ; p != y.end() ; ++p, ++color ){
    for ( unsigned i = 0 ; i < min(x.size(),p->size()) ; ++i )
      _buffer << x[i] << " " << (*p)[i] << " " << (*color)[i] << std::endl;
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

void Gnuplot::plot(const std::list<std::pair<std::vector<double>,std::vector<double>>> &xy, const std::list<std::string> &labels, const std::vector<unsigned> &colors) {
  using namespace std;
  std::stringstream total;
  _buffer.clear();
  _buffer.seekp(0);
  _buffer.str("");
  _buffer << "plot ";
  auto it = labels.begin();
  for( unsigned p = 0 ; p < xy.size() ; ++p ){
    if ( p < labels.size() ) {
      _buffer << "\"-\" t '" << *it << "' lw 3";
      ++it;
    }
    else {
      _buffer <<  "\"-\" t '' lw 3";
    }
    if ( p < colors.size() && colors[p] != (unsigned) -1)
      _buffer << " lc rgbcolor " << colors[p];
    else
      _buffer << " lc rgb \"" << HTMLcolor[p%15] << "\"";
    _buffer << ", ";
  }

  _buffer.seekp(-2,ios_base::end);
  _buffer << std::endl;
  _buffer.str(translateToSymbol(_buffer.str()));
  _buffer.seekp(0,ios_base::end);

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
void Gnuplot::save(const std::string &filename) {
  std::stringstream com;
  this->dump(com,filename);

  if ( _gp.get() != nullptr ) {
    fprintf(_gp.get(),"%s\n",com.str().c_str());
    fflush(_gp.get());
  }
}

//
void Gnuplot::dump(std::ostream& out, const std::string& plotname) const {
  out << translateToSymbol(_header.str()) << std::endl;
  out << "set terminal push" << std::endl;
  out << "set terminal postscript enhanced color solid font 24 eps" << std::endl;
  out << "set output \"" << (plotname+".eps") << "\"" << std::endl;
  out << translateToSymbol(_custom.str()) << std::endl;
  out << "set xlabel '" << translateToSymbol(_xlabel) << "'" << std::endl;
  out << "set ylabel '" << translateToSymbol(_ylabel) << "'" << std::endl;
  out << "set title '" << translateToSymbol(_title) << "'" << std::endl;
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

std::string Gnuplot::translateToSymbol(const std::string &input) {
  const std::map<std::string,std::string> translator = {
    std::pair<std::string,std::string>("Alpha"  , "{/Symbol A}"),
    std::pair<std::string,std::string>("Beta"   , "{/Symbol B}"),
    std::pair<std::string,std::string>("Gamma"  , "{/Symbol G}"),
    std::pair<std::string,std::string>("Delta"  , "{/Symbol D}"),
    std::pair<std::string,std::string>("Epsilon", "{/Symbol E}"),
    std::pair<std::string,std::string>("Zeta"   , "{/Symbol Z}"),
    std::pair<std::string,std::string>("Theta"  , "{/Symbol Q}"),
    std::pair<std::string,std::string>("Eta"    , "{/Symbol H}"),
    std::pair<std::string,std::string>("Iota"   , "{/Symbol I}"),
    std::pair<std::string,std::string>("Kappa"  , "{/Symbol K}"),
    std::pair<std::string,std::string>("Lambda" , "{/Symbol L}"),
    std::pair<std::string,std::string>("Mu"     , "{/Symbol M}"),
    std::pair<std::string,std::string>("Nu"     , "{/Symbol N}"),
    std::pair<std::string,std::string>("Xi"     , "{/Symbol X}"),
    std::pair<std::string,std::string>("Omicron", "{/Symbol O}"),
    std::pair<std::string,std::string>("Pi"     , "{/Symbol P}"),
    std::pair<std::string,std::string>("Rho"    , "{/Symbol R}"),
    std::pair<std::string,std::string>("Sigma"  , "{/Symbol S}"),
    std::pair<std::string,std::string>("Tau"    , "{/Symbol T}"),
    std::pair<std::string,std::string>("Upsilon", "{/Symbol U}"),
    std::pair<std::string,std::string>("Phi"    , "{/Symbol F}"),
    std::pair<std::string,std::string>("Chi"    , "{/Symbol C}"),
    std::pair<std::string,std::string>("Khi"    , "{/Symbol C}"),
    std::pair<std::string,std::string>("Psi"    , "{/Symbol Y}"),
    std::pair<std::string,std::string>("Omega"  , "{/Symbol W}"),
    std::pair<std::string,std::string>("alpha"  , "{/Symbol a}"),
    std::pair<std::string,std::string>("beta"   , "{/Symbol b}"),
    std::pair<std::string,std::string>("gamma"  , "{/Symbol g}"),
    std::pair<std::string,std::string>("delta"  , "{/Symbol d}"),
    std::pair<std::string,std::string>("epsilon", "{/Symbol e}"),
    std::pair<std::string,std::string>("zeta"   , "{/Symbol z}"),
    std::pair<std::string,std::string>("theta"  , "{/Symbol q}"),
    std::pair<std::string,std::string>("eta"    , "{/Symbol h}"),
    std::pair<std::string,std::string>("iota"   , "{/Symbol i}"),
    std::pair<std::string,std::string>("kappa"  , "{/Symbol k}"),
    std::pair<std::string,std::string>("lambda" , "{/Symbol l}"),
    std::pair<std::string,std::string>("mu"     , "{/Symbol m}"),
    std::pair<std::string,std::string>("nu"     , "{/Symbol n}"),
    std::pair<std::string,std::string>("xi"     , "{/Symbol x}"),
    std::pair<std::string,std::string>("omicron", "{/Symbol o}"),
    std::pair<std::string,std::string>("pi"     , "{/Symbol p}"),
    std::pair<std::string,std::string>("rho"    , "{/Symbol r}"),
    std::pair<std::string,std::string>("sigma"  , "{/Symbol s}"),
    std::pair<std::string,std::string>("tau"    , "{/Symbol t}"),
    std::pair<std::string,std::string>("upsilon", "{/Symbol u}"),
    std::pair<std::string,std::string>("phi"    , "{/Symbol f}"),
    std::pair<std::string,std::string>("chi"    , "{/Symbol c}"),
    std::pair<std::string,std::string>("khi"    , "{/Symbol c}"),
    std::pair<std::string,std::string>("psi"    , "{/Symbol y}"),
    std::pair<std::string,std::string>("omega"  , "{/Symbol w}"),
  };

  std::string output(input);
  for ( auto& word : translator ) {
    output = std::regex_replace(output,std::regex("\\b"+word.first+"\\b"),word.second);
  }
  return output;
}
