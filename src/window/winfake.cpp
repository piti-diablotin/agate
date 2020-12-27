/**
 * @file src/window/winfake.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2017 Jordan Bieder
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


#include "window/winfake.hpp"
#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

Winfake::Winfake(pCanvas &canvas, const int width, const int height) : Window(canvas,width,height) 
{
}

//
//Winfake::~Winfake() {
//  ;
//}

void Winfake::init() {
}

void Winfake::end() {
}

void Winfake::loop() {
  _render._isOk = true;
  if ( _canvas.get() == nullptr )
    throw EXCEPTION("Canvas is not allocated !", ERRABT);
  _optionb["view_time"] = (_canvas->ntime()>1);
  _optionf["distance"] = 1.1f*_canvas->typicalDim();

  while( _optioni["shouldExit"]==0 ) {
    this->loopStep();
  }
}

//
void Winfake::setParameters(const std::string &filename) {
  try { 
    if ( filename == "" ) return;
    std::ifstream file(filename,std::ios::in);

    if ( !file ) throw EXCEPTION(std::string("Unable to open file ")+filename,ERRDIV);

    std::string opt(":wait 1");
    for ( unsigned i = 0 ; i < opt.size() ; ++i )
      _inputChar.push((unsigned int)opt[i]);
    _inputChar.push((unsigned int)'\n');
    for ( std::string line ; std::getline(file,line) ; ) {
      size_t pos_com = line.find_first_of("#");
      if ( pos_com != std::string::npos ) {
        line.resize(pos_com);
      }
      for ( unsigned i = 0 ; i < line.size() ; ++i )
        _inputChar.push((unsigned int)line[i]);
      _inputChar.push((unsigned int)'\n');
    }
  }
  catch ( Exception &e ) {
    throw e;
  }
}

//
void Winfake::loopStep() {
  if ( _canvas.get() == nullptr ) return;
  std::stringstream info;

  // Process input
  this->userInput(info);

  const float camx = 0.;
  const float camy = 0.;
  const float camz = 0.;
  _canvas->refresh({{camx,camy,camz}},_render);
  if ( !_canvas->isPaused() )
    _canvas->switchPause();

  if ( _optioni["shouldExit"]==0 && _inputChar.empty() ) {
    std::string inputString;
#ifdef HAVE_READLINE
    char *input = readline(" >> ");
    //if (strlen(input)>0) add_history(input);
    inputString = input;
    delete[] input;
#else
    std::cout << " >> ";
    std::getline(std::cin,inputString);
#endif

    for ( unsigned i = 0 ; i < inputString.size() ; ++i )
      _inputChar.push((unsigned int)inputString[i]);
    _inputChar.push((unsigned int)'\n');
  }
}

//
bool Winfake::getChar(unsigned key) {
  (void) key;
  return false;
}

//
bool Winfake::getCharPress(unsigned key) {
  (void) key;
  return false;
}
