/**
 * @file src/winglfw3.cpp
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


#include "window/winglfw3.hpp"
#include "base/exception.hpp"
#include <cmath>
#ifdef HAVE_GLFW3
#include <GLFW/glfw3.h>
#endif

namespace {
#ifdef HAVE_GLFW3
  std::map<GLFWwindow*,WinGlfw3*> glfwPtr_;

#endif
  void ErrorCallback(int noeglfw, const char *message) {
    Exception e = EXCEPTION(std::string(message)+std::string("\nError code:")
        +utils::to_string(noeglfw),ERRDIV);
    std::clog << e.fullWhat() << std::endl;
  }
}

//
WinGlfw3::WinGlfw3(pCanvas &canvas, const int width, const int height, const int mode) : Window(canvas, width, height),
#ifdef HAVE_GLFW3
  _win(nullptr),
#endif
  _stateKey(),
  _stateMouse(),
  _offsets {0},
  _scalings {1}
{
  for ( unsigned key = 0 ; key < _maxKeys ; ++key )
    _stateKey[key] = false;
  for ( unsigned key = 0 ; key < 8 ; ++key )
    _stateMouse[key] = false;
  if ( mode != WinGlfw3::window && mode != WinGlfw3::fullscreen ) 
    throw EXCEPTION("Bad mode for creating GLFW3 window.\nTry WinGlfw3::window or WinGlfw3::fullscreen",ERRDIV);

#ifdef HAVE_GLFW3
  _mouseButtonLeft = GLFW_MOUSE_BUTTON_LEFT;
  _mouseButtonRight = GLFW_MOUSE_BUTTON_RIGHT;
  _mouseButtonMiddle = GLFW_MOUSE_BUTTON_MIDDLE;
  _keyEnter = GLFW_KEY_ENTER;
  _keyKPEnter = GLFW_KEY_KP_ENTER;
  _keyBackspace = GLFW_KEY_BACKSPACE;
  _keyEscape = GLFW_KEY_ESCAPE;
  _keyArrowUp = GLFW_KEY_UP;
  _keyArrowDown = GLFW_KEY_DOWN;
  _keyArrowLeft = GLFW_KEY_LEFT;
  _keyArrowRight = GLFW_KEY_RIGHT;
  _keyX = GLFW_KEY_X;
  _keyY = GLFW_KEY_Y;
  _keyZ = GLFW_KEY_Z;

  GLFWmonitor *monitor = ( mode == WinGlfw3::window ? nullptr : glfwGetPrimaryMonitor() );

  if ( mode == WinGlfw3::fullscreen ) {
    const GLFWvidmode* vmode = glfwGetVideoMode(monitor);
    _width = vmode->width;
    _height = vmode->height;
  }

  // Prepare windows creation
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 16);
#ifdef __APPLE__
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE); // Use real pixel size of framebuffer
#endif

  if ( (_win = glfwCreateWindow(_width, _height, PACKAGE_STRING, monitor, nullptr) ) == nullptr )
    throw EXCEPTION("Failed to open GLFW window",ERRDIV);

  glfwPtr_[_win]=this;
  glfwMakeContextCurrent(_win);

  glfwSwapInterval(1); // On card that support vertical sync, activate it.
  glfwPollEvents();  // Fix for GLFW 3.3 Scale factor wrong if before polling
                     // (https://github.com/glfw/glfw/issues/1968)

  glfwGetFramebufferSize(_win,&_width,&_height); // OpenGL size
  glfwSetFramebufferSizeCallback(_win,[](GLFWwindow* win, int width, int height)
      {
      glfwPtr_[win]->_width=width;
      glfwPtr_[win]->_height=height;
      });

  glfwSetScrollCallback(_win,[](GLFWwindow* win, double xoffset, double yoffset) {
      try {
        glfwPtr_[win]->_offsets[0] = (float) xoffset;
        glfwPtr_[win]->_offsets[1] = (float) yoffset;
      }
      catch(...){
        Exception e = EXCEPTION("Failed to recorded scrolling event",ERRDIV);
        std::cerr << e.fullWhat() << std::endl;
      }
    });
  glfwSetCharCallback(_win,[](GLFWwindow* win, unsigned int codepoint) {
      if ( codepoint  < _maxKeys ) 
        glfwPtr_[win]->_inputChar.push(codepoint);
      });
  glfwSetKeyCallback(_win,[](GLFWwindow* win, int key, int scancode, int action, int mods) {
      (void) scancode;
      if ( mods == GLFW_MOD_CONTROL && key == GLFW_KEY_V && action == GLFW_PRESS ) {
        const char* text = glfwGetClipboardString(win);
        if ( text ) {
          unsigned c = 0;
          while( text[c] != '\0' )
            glfwPtr_[win]->_inputChar.push(text[c++]);
        }
      }
    });

#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 3
  glfwGetWindowContentScale(_win, &_scalings[0], &_scalings[1]);
  glfwSetWindowContentScaleCallback(_win,[](GLFWwindow* win, float xscale, float yscale)
      {
      auto winptr = glfwPtr_[win];
      auto average = (xscale+yscale)/2.;
      winptr->_optioni["fontSize"] *= average/(( winptr->_scalings[0]+winptr->_scalings[1])/2.);
      winptr->_scalings[0]=xscale;
      winptr->_scalings[1]=yscale;
      winptr->_render._render.setSize(glfwPtr_[win]->_optioni["fontSize"]);
      });

  _optioni["fontSize"] *= ( _scalings[0]+_scalings[1])/2.;
  _render._render.setSize(_optioni["fontSize"]);
#endif

#else
  throw EXCEPTION("GLFW support is not available.\nConsider compiling the code with OpenGL+GLFW",ERRDIV);
#endif
  //glfwDisable(GLFW_MOUSE_CURSOR);

  Window::beginGl();
  this->getWheelOffset(_optionf["wheel"]);
  _optionf["oldWheel"] = _optionf["wheel"];
}

//
WinGlfw3::~WinGlfw3() {
#ifdef HAVE_GLFW3
  glfwDestroyWindow(_win);
#endif
  ;
}

//
void WinGlfw3::init() {
#ifdef HAVE_GLFW3
  glfwSetErrorCallback(ErrorCallback);
  if ( !glfwInit() )
    throw EXCEPTION("Failed to init GLFW",ERRDIV);
#else
  throw EXCEPTION("GLFW support is not available.\nConsider compiling the code with OpenGL+GLFW",ERRDIV);
#endif
}

//
void WinGlfw3::end() {
#ifdef HAVE_GLFW3
  glfwTerminate();
#endif
}

//
void WinGlfw3::version(int &major, int &minor, int &rev) {
#ifdef HAVE_GLFW3
  glfwGetVersion(&major, &minor, &rev);
  std::clog << "Using GLFW version " << major << "." << minor << "." << rev << std::endl;
#else
  major = 0; minor = 0; rev = 0;
#endif
}

//
void WinGlfw3::setTitle(const std::string& title) {
  _title = title;
#ifdef HAVE_GLFW3
  glfwSetWindowTitle(_win,title.c_str());
#endif
}

//
void WinGlfw3::setSize(const int width, const int height) {
#ifdef HAVE_GLFW3
  glfwSetWindowSize(_win,width*_scalings[0],height*_scalings[1]);
#endif
}

//
void WinGlfw3::getSize(int &width, int &height) {
  width = _width;
  height = _height;
}

//
void WinGlfw3::move(const int x, const int y) {
  int xx = x;
  int yy = y;
#ifdef HAVE_GLFW3
  glfwSetWindowPos(_win,xx,yy);
#endif
  _posX = xx;
  _posY = yy;
}

//
/*
void WinGlfw3::setParameters(const ConfigParser& config) {
  Window::setParameters(config);
#ifdef HAVE_GLFW3
  try {
    if ( config.getToken<bool>("fullscreen") ) {
      glfwDestroyWindow(_win);
      GLFWmonitor *monitor = glfwGetPrimaryMonitor() ;

      const GLFWvidmode* mode = glfwGetVideoMode(monitor);
      _width = mode->width;
      _height = mode->height;

      if ( (_win = glfwCreateWindow(_width, _height, PACKAGE_STRING, monitor, nullptr) ) == nullptr )
        throw EXCEPTION("Failed to open GLFW window",ERRDIV);

      glfwMakeContextCurrent(_win);

      glfwSwapInterval(1); // On card that support vertical sync, activate it.
      glfwGetWindowSize(_win,&_width,&_height);
      Window::beginGl();
    }
  }
  catch(Exception& e) {
    if ( e.getReturnValue() != ConfigParser::ERFOUND) {
      e.ADD("Correct you input file", ERRDIV);
      throw e;
    }
  }
#endif
}
*/

//
bool WinGlfw3::getMouse(unsigned int key) {
  bool pressed = false;
  if ( key >= 8 ) {
    Exception e = EXCEPTION("Ignoring mouse key : out of range",ERRWAR);
    std::cerr << e.fullWhat() << std::endl;
    return pressed;
  }
#ifdef HAVE_GLFW3
  bool getkey = glfwGetMouseButton(_win,  key ) == GLFW_PRESS;
  pressed = !_stateMouse[key] && getkey;
  _stateMouse[key] = getkey;
#endif
  return pressed;
}

bool WinGlfw3::getMousePress(unsigned int key) {
  bool pressed = false;
  if ( key >= 8 ) {
    Exception e = EXCEPTION("Ignoring mouse key : out of range",ERRWAR);
    std::cerr << e.fullWhat() << std::endl;
    return pressed;
  }
#ifdef HAVE_GLFW3
  pressed = glfwGetMouseButton(_win,  key ) == GLFW_PRESS;
#endif
  return pressed;
}

//
void WinGlfw3::getWheelOffset(float &wheel) {
  // FIXME !
#ifdef HAVE_GLFW3
  wheel = floor(_offsets[1]);
  _offsets[0]=0;
  _offsets[1]=0;
  //wheel = glfwGetMouseWheel();
#else
  wheel = 0.f;
#endif
}

//
void WinGlfw3::getMousePosition(float &x, float &y) {
#ifdef HAVE_GLFW3
  double xx, yy;
  glfwGetCursorPos(_win, &xx, &yy);
  x = (float) xx;
  y = (float) yy;
#else
  x = 0;
  y = 0;
#endif
}

//
bool WinGlfw3::getCharPress(unsigned key) {
#ifdef HAVE_GLFW3
  return (glfwGetKey(_win, key ) == GLFW_PRESS);
#else
  return key && false;
#endif
}

//
bool WinGlfw3::getChar(unsigned key) {
  bool pressed = false;
  if ( key >= _maxKeys ) {
    Exception e = EXCEPTION("Ignoring key : out of range",ERRWAR);
    std::cerr << e.fullWhat() << std::endl;
    return pressed;
  }
#ifdef HAVE_GLFW3
  bool getkey = glfwGetKey(_win, key ) == GLFW_PRESS;
  pressed = !_stateKey[key] && getkey;
  _stateKey[key] = getkey;
#endif
  return pressed;
}

void WinGlfw3::swapBuffers() {
#ifdef HAVE_GLFW3
  glfwSwapBuffers(_win);
  glfwPollEvents();
#endif
}

void WinGlfw3::pollEvents() {
#ifdef HAVE_GLFW3
  glfwPollEvents();
#endif
}

bool WinGlfw3::exitMainLoop() { 
#ifdef HAVE_GLFW3
  return 0<glfwWindowShouldClose( _win );
#else
  return true;
#endif
}

void WinGlfw3::setDropCallback(std::function<void(int, const char**)> callback){
  _dropCallback = callback;
#ifdef HAVE_GLFW3_DROP
  glfwSetDropCallback(_win,[](GLFWwindow* win, int count, const char** paths){
      glfwPtr_[win]->_dropCallback(count,paths);
      });
#else
  throw EXCEPTION("Drag&Drop not available",ERRWAR);
#endif
}
