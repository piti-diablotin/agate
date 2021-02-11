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
std::map<GLFWwindow*, std::array<float,2> > WinGlfw3::_offsets; ///< Declare the static variable here so it is done once for all
#endif

//
WinGlfw3::WinGlfw3(pCanvas &canvas, const int width, const int height, const int mode) : Window(canvas, width, height),
#ifdef HAVE_GLFW3
  _win(nullptr),
#endif
  _stateKey(),
  _stateMouse()
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

  //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 16);
#ifdef __APPLE__
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif

  if ( (_win = glfwCreateWindow(_width, _height, PACKAGE_STRING, monitor, nullptr) ) == nullptr )
    throw EXCEPTION("Failed to open GLFW window",ERRDIV);

  glfwMakeContextCurrent(_win);

  glfwSwapInterval(1); // On card that support vertical sync, activate it.
  glfwGetFramebufferSize(_win,&_width,&_height);
  _offsets[_win][0] = 0.0;
  _offsets[_win][1] = 0.0;
  glfwSetScrollCallback(_win,WheelCallback);
  glfwSetCharCallback(_win,CharCallback);
  glfwSetKeyCallback(_win,KeyCallback);
#if defined(HAVE_GLFW3_CONTENTSCALE) && defined(__linux__)
  float xscale, yscale;
  glfwGetWindowContentScale(_win, &xscale, &yscale);
  _width *= xscale;
  _height *= yscale;
  _optioni["fontSize"] *= ( xscale+yscale )/2.;
#endif
  glfwSetWindowSize(_win,_width,_height);
  _render._render.setSize(_optioni["fontSize"]);

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
  _width = width;
  _height = height;
#ifdef HAVE_GLFW3
  glfwSetWindowSize(_win,_width,_height);
#endif
}

//
void WinGlfw3::getSize(int &width, int &height) {
#ifdef HAVE_GLFW3
  glfwGetFramebufferSize(_win,&_width, &_height);
#endif
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
  wheel = floor(_offsets[_win][1]);
  _offsets[_win][0]=0;
  _offsets[_win][1]=0;
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

//
#ifdef HAVE_GLFW3
void WinGlfw3::WheelCallback(GLFWwindow* win, double xoffset, double yoffset) {
  try {
    _offsets[win][0] = (float) xoffset;
    _offsets[win][1] = (float) yoffset;
  }
  catch(...){
    Exception e = EXCEPTION("Failed to recorded scrolling event",ERRDIV);
    std::cerr << e.fullWhat() << std::endl;
  }
}

//

#ifdef HAVE_GLFW3_DROP
void WinGlfw3::setDropCallback(GLFWdropfun cbfun){
  glfwSetDropCallback(_win,cbfun);
}
#endif

//
void WinGlfw3::ErrorCallback(int noeglfw, const char *message) {
  Exception e = EXCEPTION(std::string(message)+std::string("\nError code:")
      +utils::to_string(noeglfw),ERRDIV);
  std::clog << e.fullWhat() << std::endl;
}

//
void WinGlfw3::CharCallback(GLFWwindow* win, unsigned int codepoint) {
  if ( codepoint  < _maxKeys ) 
    _inputChar.push(codepoint);
  (void) win;
}

//
void WinGlfw3::KeyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
  (void) scancode;
  if ( mods == GLFW_MOD_CONTROL && key == GLFW_KEY_V && action == GLFW_PRESS ) {
    const char* text = glfwGetClipboardString(win);
    if ( text ) {
      unsigned c = 0;
      while( text[c] != '\0' )
        _inputChar.push(text[c++]);
    }
  }
}
#endif
