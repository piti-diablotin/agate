/**
 * @file src/window.cpp
 *
 * @brief Implement the main importe function to take a snapshot of the window
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


#include "window/window.hpp"
#include "base/phys.hpp"
#include "graphism/render.hpp"
#include "graphism/triarrow.hpp"
#include <sstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <fstream>

#if defined(HAVE_UNISTD_H) && (!defined(WIN32) && !defined(_WIN32)) /* freetype force the definition of HAVE_UNISTD_H */
#include <unistd.h>
#elif defined(HAVE_CPPTHREAD)
#include <thread>
#endif

#include <chrono>
#include <locale>
#ifdef HAVE_GL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

#ifdef HAVE_GLU
# ifdef __APPLE__
#  include <OpenGL/glu.h>
# else
#  include <GL/glu.h>
# endif
#endif

#include "canvas/canvaspos.hpp"
#include "canvas/canvaslocal.hpp"
#include "canvas/canvasphonons.hpp"

using std::abs;

std::queue<unsigned int> Window::_inputChar; ///< Store all character dropped by the glfw callback function.

Window::Window(pCanvas &canvas, const int width, const int height) :
  _exit(false),
  _posX(0),
  _posY(0),
  _width(width),
  _height(height),
  _suffix(0),
  _title(PACKAGE_STRING) ,
  _background(),
  _image(),
  _imageBuffer(nullptr),
  _keepImage(false),
  _imageSize(0),
  _imageSuffixMode(convert),
  _movie(false),
  _mouseButtonLeft(0),
  _mouseButtonRight(0),
  _mouseButtonMiddle(0),
  _keyEnter(0),
  _keyKPEnter(0),
  _keyBackspace(0),
  _keyEscape(0),
  _keyArrowUp(0),
  _keyArrowDown(0),
  _keyArrowLeft(0),
  _keyArrowRight(0),
  _keyX(0),
  _keyY(0),
  _keyZ(0),
  _mode(mode_static),
  _modeMouse(mode_static),
  _command(),
  _commandStack(),
  _commandStackNo(),
  _render(),
  _optionb(),
  _optionf(),
  _optioni(),
  _canvas(canvas),
  _arrow(nullptr)
{

  _background[0] = 0.f;
  _background[1] = 0.f;
  _background[2] = 0.f;
  _render._color[0] = 255;
  _render._color[1] = 255;
  _render._color[2] = 255;

  _optionf["x0"] = 0.f;
  _optionf["y0"] = 0.f;
  _optionf["campsi"] = 0.f;
  _optionf["camtheta"] = 0.f;
  _optionf["camphi"] = 0.f;
  _optionf["shiftOriginX"] = 0.f;
  _optionf["shiftOriginY"] = 0.f;
  _optionf["speed"] = 1.f;
  _optionf["aspect"] = 1.f;
  //_optionf["distance"] = 1.1f*_canvas->typicalDim();
  _optionf["wheel"] = 0.f;
  _optionf["oldWheel"] = 0.f;
  _optionf["zoom"] = 1.f;

  _optionb["view_angle"] = true;
  _optionb["view_time_input"] = true;
  //_optionb["view_time"] = (_canvas->ntime()>0);
  _optionb["error"] = false;
  _optionb["msaa"] = true;
  _optionb["paral_proj"] = true;
  _optionb["takeSnapshot"] = false;
  _optionb["axis"] = true;

  _optioni["shouldExit"] = 0;
  _optioni["oldWidth"] = 0;
  _optioni["oldHeight"] = 0;
  _optioni["dumpframe"] = 0;

  _optioni["oldWidth"] = _width;
  _optioni["oldHeight"] = _height;

  _optioni["initBuffer"] = 0;
  _optioni["prevNtime"] = 0;
  _optioni["ntime"] = 0;

#if defined(HAVE_GL) && defined(HAVE_GLEXT)
  std::clog << "OpenGL might use VBO" << std::endl;
#endif
}

//
void Window::beginGl() {
#ifdef HAVE_GL
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);
  glDepthRange(0.0f, 1.0f);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_NORMALIZE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;

  /*
     glEnable(GL_LINE_SMOOTH);
     {
     GLfloat linewidth;
     glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE,&linewidth);
     glLineWidth(linewidth);
     }
     */
#endif
}

//
void Window::setParameters(const std::string &filename) {
  try { 
    //this->setTitle(PACKAGE_NAME);
    _keepImage = true;
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

    opt = ":wait 0";
    for ( unsigned i = 0 ; i < opt.size() ; ++i )
      _inputChar.push((unsigned int)opt[i]);
    _inputChar.push((unsigned int)'\n');
  }
  catch ( Exception &e ) {
    throw e;
  }
}

//
void Window::snapshot() {
  std::stringstream sstr;
  switch (_imageSuffixMode) {
    case convert :
      sstr.fill('0');
      sstr << "_" << std::setw(5) << _suffix++;
      break;
    case animate :
      sstr << "_" << _suffix++;
      break;
  }
  std::string str(sstr.str());

  try {
#ifdef HAVE_GL
    try{
      if ( _imageBuffer == nullptr || _imageSize < _width*_height ) {
        if ( _imageBuffer != nullptr ) delete[] _imageBuffer;
        _imageBuffer = new char[3*_width*_height];
      }
    }
    catch(...) {
      throw EXCEPTION("Unable to creat a buffer array big enough to store the image",ERRDIV);
    }

    glPixelStorei(GL_PACK_ALIGNMENT,1);
    glReadBuffer(GL_BACK_LEFT);
    glReadPixels(0,0,_width,_height,GL_RGB,GL_UNSIGNED_BYTE,_imageBuffer);

    _image.save(_width,_height,_imageBuffer,str);

    if ( !_keepImage ) {
      delete[] _imageBuffer;
      _imageBuffer = nullptr;
    }
#else
    throw EXCEPTION("OpenGL support is not available.\nCan not take a snapshot.",ERRDIV);
#endif
  }
  catch(Exception& e) {
    e.ADD("Unable to take a screenshot. Skipping",ERRWAR);
    std::cerr << e.fullWhat() << std::endl;
  }
}

//
void Window::loop() {
  if ( _canvas.get() == nullptr )
    throw EXCEPTION("Canvas is not allocated !", ERRABT);
  _optionb["view_time"] = (_canvas->ntime()>1);
  _optionf["distance"] = 1.1f*_canvas->typicalDim();
  _arrow.reset(new TriArrow(true));

  while( !this->exitMainLoop() && _optioni["shouldExit"]==0 ) {
    this->loopStep();
  }
}

void Window::loopStep() {
  using std::max;
#ifdef HAVE_GL
  const float pi = (float) phys::pi;
#endif
  bool& update = _optionb["updated"];
  update = false;
  int& initBuffer = _optioni["initBuffer"];
  float& speed = _optionf["speed"];
  if ( _canvas.get() == nullptr ) return;
  if ( _optioni["prevNtime"] != (_optioni["ntime"] = _canvas->ntime()) ) { 
    initBuffer = 0;
    update |= true;
    _optioni["prevNtime"] = _optioni["ntime"];
  }
  if ( initBuffer < 3 ) { ++initBuffer; update |= true; }
  std::stringstream info;

  this->getSize(_width, _height);
  if ( _width != _optioni["oldWidth"] || _height != _optioni["oldHeight"] ) {
    _optioni["oldWidth"] = _width;
    _optioni["oldHeight"] = _height;
    update |= true;
    initBuffer = 0;
  }

  // Process input
  if ( this->userInput(info)) {
    update |= true;
    initBuffer = 0;
  }

  const float aspect = (_optionf["aspect"] = (float) _width / (float) _height);
  const float distance = (_optionf["distance"] = 1.2f*_canvas->typicalDim());

  if ( update || !_canvas->isPaused() ) {
    update = true;
    const bool paral_proj = _optionb["paral_proj"];
    float& zoom = ( _optionf["zoom"]+= _optionf["wheel"]*0.04f);
    if ( zoom < 0.01f ) zoom = 0.01f;

#ifdef HAVE_GL
    glViewport(0,0,_width,_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if ( paral_proj )  {
      (( _height > _width ) ?
       glOrtho(-distance*zoom,distance*zoom,-distance*zoom/aspect,distance*zoom/aspect,-10.f*distance,10.f*distance) :
       glOrtho(-distance*zoom*aspect,distance*aspect*zoom,-distance*zoom,distance*zoom,-10.f*distance,10.f*distance));
    }
    else {
      GLdouble fW, fH;
      fH = tan( 60.0 / 360. * pi ) * 1.0;
      fW = fH * aspect;
      glFrustum( -fW, fW, -fH, fH, 1.0, 200.0 );
    }
#endif

    const float factor_proj = paral_proj ? 1.f : 2.f;


    // Render output
#ifdef HAVE_GL
    glClearColor(_background[0], _background[1], _background[2], 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
#endif


    const float totalfactor = ( (aspect > 1.f || paral_proj) ? 1.f : 1.f/aspect ) 
      * factor_proj * distance * ( paral_proj ? 4.f : zoom); // 4 is for the spot light
    
    const float campsi   = _optionf["campsi"];
    const float camtheta = _optionf["camtheta"];
    const float camphi   = _optionf["camphi"];
    geometry::mat3d euler = geometry::matEuler(campsi,camtheta,camphi);
    glTranslatef(totalfactor*_optionf["shiftOriginX"],totalfactor*_optionf["shiftOriginY"],0.);
    this->lookAt(totalfactor,0,0,0);


    _canvas->refresh({{euler[0],euler[3],euler[6]}},_render);
    if ( _canvas->ntime() >= 1 && _optionb["axis"] ) this->drawAxis();

    if ( _optionb["takeSnapshot"] || (_movie && !_canvas->isPaused()) ) this->snapshot();

#ifdef HAVE_GL
    // Print things on screen
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDisable(GL_DEPTH_TEST);

    glRasterPos2f(0.f,1.f);
    _render.render(_command);

    if ( _movie ) { info << "Recording ";}
    // buffer is modified in _render.render();
    Render::BufferRender &buffer = _render._buffer;
    glRasterPos2f(0.f,1.f-(float)buffer.rows()/(float)_height);
    _render.render(info.str());

    if ( _canvas->ntime() > 1 ) {
      glRasterPos2f(0.f,0.f+(float)buffer.rows()/(float)_height);
      ( _canvas->isPaused() ) ?
        _render.render("Speed:x"+utils::to_string(speed)+", Paused") :
        _render.render("Speed:x"+utils::to_string(speed));

    }

    glEnable(GL_DEPTH_TEST);
#endif

    // Go to next frame
    if ( speed < 1.0f && _optioni["dumpframe"] >= (int)(1.f/speed) ) {
      _canvas->nextFrame(1);
      _optioni["dumpframe"] = 0;
    }
    else if ( speed < 1.0f ) {
      ++_optioni["dumpframe"];
    }
    else {
      _canvas->nextFrame((int) speed);
    }

  }
  this->swapBuffers();
}

//
bool Window::userInput(std::stringstream& info) {
  bool action = false;
  const float pi = (float) phys::pi;
  const float twopi = 2.f*pi;
  float& x0 = _optionf["x0"];
  float& y0 = _optionf["y0"];
  bool& view_angle = _optionb["view_angle"];;
  bool& view_time = _optionb["view_time"];
  bool& view_time_input = _optionb["view_time_input"];
  bool& error = _optionb["error"];
#ifdef HAVE_GL
  bool& msaa = _optionb["msaa"];
#endif
  float& campsi = _optionf["campsi"];
  float& camtheta = _optionf["camtheta"];
  float& camphi = _optionf["camphi"];
  //static std::string string_static;
  float x = 0;
  float y = 0;

  const Canvas::TransDir TransAdd = Canvas::TransDir::PLUS;
  const Canvas::TransDir TransDel = Canvas::TransDir::MINUS;

  _optionb["takeSnapshot"] = false;
  bool process = false;
  if ( _mode == mode_process ) _mode = mode_static;

  try {
    while ( !_inputChar.empty() ) {
      action = true;
      char ic = static_cast<char>(_inputChar.front());

      if ( _mode != mode_command ) {
        if ( ic == ':' ){
          _commandStack.push_back(std::string(":"));
          _commandStackNo = _commandStack.size()-1;
          _inputChar.pop();
          _command = ":|";
          if ( !_render._isOk ) std::clog << ":";
          _mode = mode_command;
          continue;
        }

        if ( _mode == mode_add ) {
          switch ( ic ) {
            case 'x': {_canvas->translateX(TransAdd);break;}
            case 'y' : {_canvas->translateY(TransAdd);break;}
            case 'z' : {_canvas->translateZ(TransAdd);break;}
          }
          _mode = mode_static;
        }
        else if ( _mode == mode_remove ) {
          switch ( ic ) {
            case 'x': {_canvas->translateX(TransDel);break;}
            case 'y' : {_canvas->translateY(TransDel);break;}
            case 'z' : {_canvas->translateZ(TransDel);break;}
          }
          _mode = mode_static;
        }
        else if ( _mode == mode_static && _modeMouse != mode_mouse ) {
          switch ( ic ) {
#ifdef HAVE_GL
            case 'A' : {
                         msaa = !msaa;
                         msaa ? glEnable(GL_MULTISAMPLE) : glDisable(GL_MULTISAMPLE);
                         break;
                       }
#endif
            case 'x' : {campsi = 0      ; camtheta =  0     ; camphi = 0.    ;_mode = mode_static;break;}
            case 'y' : {campsi = 0      ; camtheta =  0     ; camphi = pi*0.5;_mode = mode_static;break;}
            case 'z' : {campsi = 0      ; camtheta = pi*0.5 ; camphi = pi    ;_mode = mode_static;break;}
            case '+' : {_mode = mode_add;break;}
            case '-' : {_mode = mode_remove;break;}
            case '*' : {_optionf["speed"] *= 2.0f;break;}
            case '/' : {_optionf["speed"] /= (_optionf["speed"] <= 0.001 ? 1.f : 2.0f);break;}
            case 'a' : { 
                         if ( !_render._isOk && !view_angle )
                           std::clog 
                             << "psi=" << (int)(campsi/pi*180.f) 
                             << ", theta=" << (int)(camtheta/pi*180.f)
                             << ", phi=" << (int)(camphi/pi*180.f) << std::endl;
                         view_angle = !view_angle;
                         break;
                       }
            case 'p' : {_optionb["paral_proj"]=(!_optionb["paral_proj"]);break;}

            case 'l' : {_canvas->switchLight();break;}
            case ' ' : {_canvas->switchPause();break;}
            case '<' : {_canvas->previousStep();break;}
            case '>' : {_canvas->nextStep();break;}
            case 'o' : {_canvas->switchDrawing();break;}

            case 't' :{
                        if ( !_render._isOk && !view_time_input )
                          std::clog << "Time step: " << _canvas->itime() << "/" << _canvas->ntime()-1 << std::endl;
                        view_time_input = !view_time_input;
                        break;
                      }
            case 's' : {_optionb["takeSnapshot"] = true;break;}
            case 'm' : {if(_canvas->ntime()>1)_movie= ! _movie;break;}
                       //default : string_static+ic;
          }
        }
        _inputChar.pop();
      } // _mode != mode_command
      else {
        size_t cursor = 0; 
        if ( (cursor = _command.find_last_of("|")) != std::string::npos ) 
          _command.erase(cursor,1);
        //while ( !_inputChar.empty() ) {
        //char c = static _cast<char>(_inputChar.front());
        if ( ic != '\n') {
          _inputChar.pop();
          if ( _modeMouse != mode_mouse ) {
            _command += ic;
            if ( !_render._isOk ) {std::clog << ic; std::clog.flush();}
          }
        }
        else 
          process = true;
        //}
        _command += "|";
        _commandStack.back() = _command;
      }
      if ( process ) break;
    }
    // End of while loop

    //
    if ( _mode == mode_command ) {
      if ( this->getChar(_keyEnter) || this->getChar(_keyKPEnter) || process ) {// enter
        action = true;
        _commandStack.back() = _command;
        try {
          if ( !_render._isOk ) std::clog << std::endl;
          std::istringstream cin(_command.substr(1,_command.size()-2));
          std::string token;
          int istep;  
          cin >> istep;
          bool succeeded = !cin.fail();
          cin.clear();
          cin.seekg(0);
          cin >> token;
          if ( succeeded ) _canvas->step(istep);
          else if ( token == "q" ||  token == "quit" ) _optioni["shouldExit"] = 1;
          else if ( token == "qa" ||  token == "quitall" ) _optioni["shouldExit"] = 2;
          else if ( token == "h" ||  token == "help" ) Window::help();
          else if ( token == "axis" ) {
            _optionb["axis"] = !_optionb["axis"];
          }
          else if ( token == "bg" || token == "background" ) {
            unsigned b[3];
            cin >> b[0] >> b[1] >> b[2];
            if ( !cin.fail() && b[0] < 256 && b[1] < 256 && b[2] < 256 ) {
              _background[0] = (float) b[0]/255.f;
              _background[1] = (float) b[1]/255.f;
              _background[2] = (float) b[2]/255.f;
              _render._color[0] = 255-b[0];
              _render._color[1] = 255-b[1];
              _render._color[2] = 255-b[2];
            }
          }
          else if ( token == "fg" || token == "foreground" ) {
            unsigned f[3];
            cin >> f[0] >> f[1] >> f[2];
            if ( !cin.fail() && f[0] < 256 && f[1] < 256 && f[2] < 256 ) {
              _render._color[0] = f[0];
              _render._color[1] = f[1];
              _render._color[2] = f[2];
            }
          }
          else if ( token == "img_fmt" || token == "image_format" ) {
            std::string ext;
            cin >> ext;
            if ( ext == "png" ) _image.setFormat(ImageSaver::ImageType(ImageSaver::png));
            else if ( ext == "jpeg" )  _image.setFormat(ImageSaver::ImageType(ImageSaver::jpeg));
            else if ( ext == "ppm" ) _image.setFormat(ImageSaver::ImageType(ImageSaver::ppm));
            else {
              Exception e = EXCEPTION("Unrecognize format (jpeg|png|ppm)",ERRWAR);
              std::cerr << e.fullWhat() << std::endl;
            }
          }
          else if ( token == "img_qlt" || token == "image_quality" ) {
            int qlt;
            cin >> qlt;
            if ( !cin.fail() ) _image.setQuality(qlt);
          }
          else if ( token == "img_suf" || token == "image_suffix" ) {
            std::string suf;
            cin >> suf;
            if ( suf == "convert" ) _imageSuffixMode = convert;
            else if ( suf == "animate" )  _imageSuffixMode = animate;
            else {
              Exception e = EXCEPTION("Unrecognize suffix format (convert|animate)",ERRWAR);
              std::cerr << e.fullWhat() << std::endl;
            }
          }
          else if ( token == "load" ) {
            std::string filename;
            cin >> filename;
            if ( !cin.fail() ) {
              // The _inputChar will be poped until next "\n" so if don't add it here te first line
              // in filename will be poped too.
              _inputChar.push('\n');
              this->setParameters(filename);
            }
          }
          else if ( token == "psi") {
            float angle;
            cin >> angle;
            if ( !cin.fail() ) campsi = angle*pi/180.f;
          }
          else if ( token == "theta") {
            float angle;
            cin >> angle;
            if ( !cin.fail() ) camtheta = angle*pi/180.f;
          }
          else if ( token == "phi" ) {
            float angle;
            cin >> angle;
            if ( !cin.fail() )  camphi = angle * pi/180.f;
          }
          else if ( token == "s" || token == "speed" ) {
            float speed;
            cin >> speed; 
            if ( !cin.fail() ) {
              _optionf["speed"] = ( speed >= 1.0f ? floor(speed) : speed);
              if ( _optionf["speed"] <= 0.001f ) _optionf["speed"] = 1.f;
              throw EXCEPTION("Speed adjusted to "+utils::to_string(_optionf["speed"]),ERRCOM);
            }
            else
              throw EXCEPTION("You need to set the speed",ERRCOM);
          }
          else if ( token == "t" || token == "title" ) {
            cin >> _title;
            this->setTitle(_title);
            _image.setBasename(_title);
            _suffix=0;
          }
          else if ( token == "m" || token == "mode" ) {
            std::string cmode;
            cin >> cmode;
            if ( cmode == "loc" || cmode == "local" ) {
              _canvas.reset(new CanvasLocal(std::move(*reinterpret_cast<CanvasPos*>(_canvas.get()))));
            }
            else if ( cmode == "pos" || cmode == "positions" ) {
              _canvas.reset(new CanvasPos(std::move(*reinterpret_cast<CanvasPos*>(_canvas.get()))));
            }
            else if ( cmode == "ph" || cmode == "phonons" ) {
              _canvas.reset(new CanvasPhonons(std::move(*reinterpret_cast<CanvasPos*>(_canvas.get()))));
            }
            else throw EXCEPTION("Bad mode "+cmode,ERRDIV);
          }
          else if ( token == "height" ) {
            int height;
            cin >> height; 
            if ( !cin.fail() && height > 1) {
              this->setSize(_width,height);
              throw EXCEPTION("Height adjusted to "+utils::to_string(_height),ERRCOM);
            }
            else 
              throw EXCEPTION("Height should be followed by an positive integer",ERRDIV);
          }
          else if ( token == "width" ) {
            int width;
            cin >> width; 
            if ( !cin.fail() && width> 1) {
              this->setSize(width,_height);
              throw EXCEPTION("Width adjusted to "+utils::to_string(_width),ERRCOM);
            }
            else 
              throw EXCEPTION("Width should be followed by an positive integer",ERRDIV);
          }
          else if ( token == "zoom" ) {
            float zoom;
            cin >> zoom; 
            if ( !cin.fail() && zoom > 0) {
              _optionf["zoom"] = 1.f/zoom;
              throw EXCEPTION("zoom adjusted to "+utils::to_string(1./_optionf["zoom"]),ERRCOM);
            }
            else 
              throw EXCEPTION("zoom should be followed by a positive float",ERRDIV);
          }
          else {
            _canvas->alter(token,cin);
            view_time = view_time_input && (_canvas->ntime()>1);
            std::string info = _canvas->info();
            size_t pos = info.find_last_of("/\\");
            if ( !info.empty() ) this->setTitle(info.substr(pos+1));
          }
          if ( cin.fail() ) throw EXCEPTION("Bad line argument",ERRCOM);
          error = false;
          _command = _canvas->info();
        } // try process
        catch (Exception &e) {
          //if ( e.getReturnValue() != ERRCOM )
          std::clog << e.fullWhat() << std::endl;
          if ( _render._isOk ) _command = e.what("",true);
          error = true;
        }
        _mode = mode_process;
        while ( !_inputChar.empty() && _inputChar.front() != '\n' ) _inputChar.pop();

      } // enter
      else if ( this->getChar(_keyEscape) ) {
        _mode = mode_static;
        _command = _canvas->info();
        _commandStack.pop_back();
        _commandStackNo = _commandStack.size();
        error = false;
        action = true;
        if ( !_render._isOk ) std::clog << std::endl;
        while ( !_inputChar.empty() && _inputChar.front() != '\n' ) _inputChar.pop();
      } // escape
      else if ( this->getChar(_keyBackspace)  && _command.size() > 1 ){
        _command.erase(_command.end()-2);
        _commandStack.back() = _command;
        if ( !_render._isOk ) std::clog << "\b \b";
        action = true;
      } // backspace
      else if ( this->getChar(_keyArrowUp) ) {
        if ( (_commandStackNo - 1) < _commandStack.size()) { // _commandStackNo is unsigend so if <0 it is apriori >> _commandStack.size()
          _command = _commandStack[--_commandStackNo];
          action = true;
        }
      }
      else if ( this->getChar(_keyArrowDown) ) {
        if ( (_commandStackNo+1) < _commandStack.size() ) {
          _command = _commandStack[++_commandStackNo];
          action = true;
        }
      }
    } // _mode == mode_command
    else {
      if ( this->getChar(_keyEscape) ) {// enter
        error = false;
        action = true;
        while ( !_inputChar.empty() && _inputChar.front() != '\n' ) _inputChar.pop();
        _command = "";
      }
      if ( !error ) _command = _canvas->info();
      view_time = (view_time_input && _canvas->ntime()>1);
    }
    // Check Mouse
    if ( this->getMousePress(_mouseButtonLeft) ) {
      action = true;
      if ( _modeMouse != mode_mouse ){
        _modeMouse = mode_mouse;
        this->getMousePosition(x0,y0);
      }
      this->getMousePosition(x,y);
      using namespace geometry;
      //std::cerr << "start" << std::endl;
      mat3d euler = matEuler(campsi,camtheta,camphi);
      vec3d axe1({{euler[2],euler[5],euler[8]}});
      vec3d axe2({{euler[1],euler[4],euler[7]}});
      double angle1 = (x0-x)/_width*twopi;
      double angle2 = (y0-y)/_height*twopi;
      auto mat1 = matRotation(angle1,axe1);
      auto mat2 = matRotation(angle2,axe2);
      auto total = (mat1*mat2)*euler;
      auto newangles = anglesEuler(total);
      if ( !this->getCharPress(_keyX) )
        campsi   = newangles[0];
      if ( !this->getCharPress(_keyY) )
        camtheta = newangles[1];
      if ( !this->getCharPress(_keyZ) )
        camphi   = newangles[2];
      x0 = x; y0 = y;
    }
    else if ( this->getMousePress(_mouseButtonRight) ) {
      action = true;
      if ( _modeMouse != mode_mouse ){
        _modeMouse = mode_mouse;
        this->getMousePosition(x0,y0);
      }
      this->getMousePosition(x,y);
      using namespace geometry;

      const double shiftX = (x-x0)/_width;
      const double shiftY = (y0-y)/_height;
      _optionf["shiftOriginX"] += shiftX;
      _optionf["shiftOriginY"] += shiftY;

      x0 = x; y0 = y;
    }
    else if (_modeMouse == mode_mouse ) {
      _modeMouse = mode_static;
      this->getChar(_keyX);
      this->getChar(_keyY);
      this->getChar(_keyZ);
    }
  }
  catch (Exception &eerror) {
    std::cerr << eerror.fullWhat() << std::endl;
    error = true;
    _command = eerror.what("",true);
    _mode = mode_static;
    action = true;
  }
  catch (...) {
    _command = _canvas->info();
    _mode = mode_static;
    action = true;
  }

  this->getWheelOffset(_optionf["wheel"]);
  if ( _optionf["wheel"] != _optionf["oldWheel"] ) {
    _optionf["oldWheel"] = _optionf["wheel"];
    action |= true;
  }

  if ( view_angle ) 
    info
      << "psi=" << (int)(campsi/pi*180.f) 
      << ", theta=" << (int)(camtheta/pi*180.f)
      << ", phi=" << (int)(camphi/pi*180.f) << " ";
  if ( view_angle && view_time ) info << "| ";
  if ( view_time ) info << "Time step: " << _canvas->itime() << "/" << _canvas->ntime()-1 << " ";
  if ( _exit ) _optioni["shouldExit"] = 1;

  return action;
}

void Window::drawAxis() {
#ifdef HAVE_GL
  const float pi = (float) phys::pi;
  /* Start drawing cartesian axis */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  const bool paral_proj = _optionb["paral_proj"];
  const float aspect = _optionf["aspect"];
  if ( paral_proj )  {
    (( _height > _width ) ?
     glOrtho(-10,10,-10/aspect,10/aspect,-100.f,100.f) :
     glOrtho(-10*aspect,10*aspect,-10,10,-100.f,100.f));
  }
  else {
    GLdouble fW, fH;
    fH = tan( 60.0 / 360. * pi ) * 1.0;
    fW = fH * aspect;
    glFrustum( -fW, fW, -fH, fH, 1.0, 200.0 );
  }
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  (( _height > _width ) ?
   glTranslatef(7.5f,-7.5f/aspect,0.f) :
   glTranslatef(7.5f*aspect,-7.5f,0.f));
  const float factor_proj = paral_proj ? 1.f : 2.f;
  const float totalfactor = ( (paral_proj) ? 1.f : 1.f/aspect ) 
    * factor_proj * ( paral_proj ? 4.f : 10 ); // 4 is for the spot light

  this->lookAt(totalfactor,0,0,0);

  _arrow->push();
  glColor3f(0.f,0.f,1.f);
  _arrow->draw(0.1f,2.f); // Z
  glColor3f(0.f,1.f,0.f);
  glRotatef(-90.0f,1.f,0.f,0.f); // y
  _arrow->draw(0.1f,2.f);
  glColor3f(1.f,0.f,0.f);
  glRotatef(90.f,0.f,1.f,0.0f); // X
  _arrow->draw(0.1f,2.f);
  _arrow->pop();

  glRasterPos3f(0.f,-2.f,0.f);
  _render.render("z");
  //glDrawPixels(buffer.cols(), buffer.rows(), GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.getPtr());
  glRasterPos3f(-2.f,0.f,0.f);
  _render.render("y");
  //glDrawPixels(buffer.cols(), buffer.rows(), GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.getPtr());
  glRasterPos3f(0.f,0.f,2.f);
  _render.render("x");
  //glDrawPixels(buffer.cols(), buffer.rows(), GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.getPtr());
  /* End draw cartesian axis */
#endif
}

void Window::lookAt(double zoom, double centerX, double centerY, double centerZ) {
  using namespace geometry;
    const float& campsi   = _optionf["campsi"];
    const float& camtheta = _optionf["camtheta"];
    const float& camphi   = _optionf["camphi"];

    geometry::mat3d euler = geometry::matEuler(campsi,camtheta,camphi);
    const float eyeX = zoom*euler[0];
    const float eyeY = zoom*euler[3];
    const float eyeZ = zoom*euler[6];

    const float upX = euler[2];
    const float upY = euler[5];
    const float upZ = euler[8];

    vec3d f({{
        centerX-eyeX,
        centerY-eyeY,
        centerZ-eyeZ
        }});
    f = f*(1./norm(f));

    vec3d s({{
         f[1]*upZ-f[2]*upY,
         f[2]*upX-f[0]*upZ,
         f[0]*upY-f[1]*upX
        }});
    s = s*(1./norm(s));

    vec3d u({{
         s[1]*f[2]-s[2]*f[1],
         s[2]*f[0]-s[0]*f[2],
         s[0]*f[1]-s[1]*f[0]
        }});
    u = u*(1./norm(u));
    

    float M[16];
    M[0] =  s[0];
    M[4] =  s[1];
    M[8] =  s[2];
    M[12] = 0;

    M[1] =  u[0];
    M[5] =  u[1];
    M[9] =  u[2];
    M[13] = 0;

    M[2] = -f[0];
    M[6] = -f[1];
    M[10] = -f[2];
    M[14] = 0;

    M[3] = 0;
    M[7] = 0;
    M[11] = 0;
    M[15] = 1;

    glMultMatrixf(M);
    glTranslated(-eyeX, -eyeY, -eyeZ);

}

void Window::help(){
  using std::cout;
  using std::endl;
  using std::setw;
  cout.setf(std::ios::left,std::ios::adjustfield);
  cout << endl << "-- Here are the shortcuts for the interactive mode --" << endl;
  cout <<         "   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   " << endl;
  cout << setw(10) << "space" << setw(59) << "Start/pause animation." << endl;
  cout << setw(10) << "+x or -x" << setw(59) << "Translate in the x direction." << endl;
  cout << setw(10) << "+y or -y" << setw(59) << "Translate in the y direction." << endl;
  cout << setw(10) << "+z or -z" << setw(59) << "Translate in the z direction." << endl;
  cout << setw(10) << "* or /" << setw(59) << "Multiply or divide by 2 the speed." << endl;
  cout << setw(10) << "< or >" << setw(59) << "Go previous or next frame." << endl;
  cout << setw(10) << "A" << setw(59) << "Anti-aliasing switching." << endl;
  cout << setw(10) << "a" << setw(59) << "Display the angles." << endl;
  cout << setw(10) << "l" << setw(59) << "Swith light on/off." << endl;
  cout << setw(10) << "m" << setw(59) << "Take snapshot of all images when time is changing (make a movie)." << endl;
  cout << setw(10) << "o" << setw(59) << "Filled or silhouette drawing." << endl;
  cout << setw(10) << "p" << setw(59) << "Switch between orthogonal and perspective view." << endl;
  cout << setw(10) << "s" << setw(59) << "Take a snapshot of the window." << endl;
  cout << setw(10) << "t" << setw(59) << "Diplay time information." << endl;
  cout << setw(10) << "x" << setw(59) << "Set the view towards the cartesian x axis." << endl;
  cout << setw(10) << "y" << setw(59) << "Set the view towards the cartesian y axis." << endl;
  cout << setw(10) << "z" << setw(59) << "Set the view towards the cartesian z axis." << endl;

  cout << endl << "-- Here are the commands that can be typed inside the window in command mode --" << endl;
  cout <<         "   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   " << endl;
  cout << setw(40) << ":axis" << setw(59) << "Display/Hide the cartesian axis." << endl;
  cout << setw(40) << ":bg or :background X Y Z" << setw(59) << "Set the background color in RGB." << endl;
  cout << setw(40) << ":fg or :forground X Y Z" << setw(59) << "Set the foreground color in RGB." << endl;
  cout << setw(40) << ":h or :help" << setw(59) << "Display this message." << endl;
  cout << setw(40) << ":img_fmt or :image_format (jpeg|png|ppm)" << setw(59) << "Chose the format to take snapshot." << endl;
  cout << setw(40) << ":img_qlt or :image_quality X" << setw(59) << "Set the quality to export image (between 1 and 100)." << endl;
  cout << setw(40) << ":img_suf or :image_suffix (convert|animate)" << setw(59) << "Choose the suffix to be append to the image file name. convert to be used with the \"convert\" tool and \"animate\" to be used with the latex animate package." << endl;
  cout << setw(40) << ":load filename" << setw(59) << "Load a file with commands inside." << endl;
  cout << setw(40) << ":m or :mode ((loc|local)|(pos|positions)|(ph|phonons))" << setw(59) << "Choose what kind of propertie to visualize:" << endl;
  cout << setw(40) << "" << setw(59) << "pos or positions to visualize atomic positions." << endl;
  cout << setw(40) << "" << setw(59) << "rot or rotations to visualize rotations of octahedra. Need to define the z for octahedra (:octa_z)" << endl;
  cout << setw(40) << "" << setw(59) << "ph or phonons to visualize phonons in a reference structure." << endl;
  cout << setw(40) << ":phi angle" << setw(59) << "To set the phi angle." << endl;
  cout << setw(40) << ":q or :quit" << setw(59) << "Quit the current window/tab." << endl;
  cout << setw(40) << ":qa or :quitall" << setw(59) << "Quit all the windows/tabs." << endl;
  cout << setw(40) << ":s or :speed factor" << setw(59) << "Velocity scaling factor to change the animation speed." << endl;
  cout << setw(40) << ":width  WIDTH" << setw(59) << "Set the witdth of the window to WIDTH px." << endl;
  cout << setw(40) << ":height HEIGHT" << setw(59) << "Set the height of the window to HEIGHT px." << endl;
  cout << setw(40) << ":theta angle" << setw(59) << "To set the theta angle." << endl;
  cout << setw(40) << ":t or :title Title" << setw(59) << "Set the title of the window and use it for the snapshot name." << endl;
  cout << setw(40) << ":zoom ZOOM" << setw(59) << "Set the zoom of the window." << endl;

  Canvas::help(cout);
  CanvasPos::help(cout);
  CanvasPhonons::help(cout);
  CanvasLocal::help(cout);
}
