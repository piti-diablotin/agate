/**
 * @file src/winglfw2.cpp
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


#include "window/winglfw2.hpp"
#include "base/exception.hpp"

//
WinGlfw2::WinGlfw2(pCanvas &canvas, const int width, const int height, const int mode) : Window(canvas, width, height),
  _stateKey(),
  _stateMouse()
{
  for ( unsigned key = 0 ; key < _maxKeys ; ++key )
    _stateKey[key] = false;
  for ( unsigned key = 0 ; key < 8 ; ++key )
    _stateMouse[key] = false;
  if ( mode != WinGlfw2::window && mode != WinGlfw2::fullscreen ) 
    throw EXCEPTION("Bad mode for creating GLFW2 window.\nTry WinGlfw2::window or WinGlfw2::fullscreen",ERRDIV);

#ifdef HAVE_GLFW2
  _mouseButtonLeft = GLFW_MOUSE_BUTTON_LEFT;
  _mouseButtonRight = GLFW_MOUSE_BUTTON_RIGHT;
  _mouseButtonMiddle = GLFW_MOUSE_BUTTON_MIDDLE;
  _keyEnter = GLFW_KEY_ENTER;
  _keyKPEnter = GLFW_KEY_KP_ENTER;
  _keyBackspace = GLFW_KEY_BACKSPACE;
  _keyEscape = GLFW_KEY_ESC;
  _keyArrowUp = GLFW_KEY_UP;
  _keyArrowDown = GLFW_KEY_DOWN;
  _keyArrowLeft = GLFW_KEY_LEFT;
  _keyArrowRight = GLFW_KEY_RIGHT;
  _keyX = 'X';
  _keyY = 'Y';
  _keyZ = 'Z';

  GLFWvidmode desktop;
  glfwGetDesktopMode( &desktop );

  /*
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
  glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  */
  glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 16);

  if ( !glfwOpenWindow(width, height, 
        desktop.RedBits, desktop.GreenBits, desktop.BlueBits,
        8, 24, 0, mode) ) // alpha, depth, stencil, GLFW_FULLSCREEN
    throw EXCEPTION("Failed to open GLFW2 window",ERRDIV);

  glfwSetWindowTitle(PACKAGE_STRING);
  //glfwEnable(GLFW_STICKY_KEYS);
  glfwSwapInterval(1); // On card that support vertical sync, activate it.
  glfwGetWindowSize(&_width,&_height);
  glfwSetCharCallback(CharCallback);
#else
  throw EXCEPTION("GLFW2 support is not available.\nConsider compiling the code with OpenGL+GLFW",ERRDIV);
#endif
  //glfwDisable(GLFW_MOUSE_CURSOR);

  Window::beginGl();
  this->getWheelOffset(_optionf["wheel"]);
  _optionf["oldWheel"] = _optionf["wheel"];
}

//
WinGlfw2::~WinGlfw2() {
  ;
}

//
void WinGlfw2::init() {
#ifdef HAVE_GLFW2
  if ( !glfwInit() )
    throw EXCEPTION("Failed to init GLFW",ERRDIV);
#else
  throw EXCEPTION("GLFW2 support is not available.\nConsider compiling the code with OpenGL+GLFW",ERRDIV);
#endif
}

//
void WinGlfw2::end() {
#ifdef HAVE_GLFW2
  glfwTerminate();
#endif
}

//
void WinGlfw2::version(int &major, int &minor, int &rev) {
#ifdef HAVE_GLFW2
  glfwGetVersion(&major, &minor, &rev);
  std::clog << "Using GLFW version " << major << "." << minor << "." << rev << std::endl;
#else
  major = 0; minor = 0; rev = 0;
#endif
}

//
void WinGlfw2::setTitle(const std::string& title) {
  _title = title;
#ifdef HAVE_GLFW2
  glfwSetWindowTitle(title.c_str());
#endif
}

//
void WinGlfw2::setSize(const int width, const int height) {
  _width = width;
  _height = height;
#ifdef HAVE_GLFW2
  glfwSetWindowSize(_width,_height);
#endif
}

//
void WinGlfw2::getSize(int &width, int &height) {
#ifdef HAVE_GLFW2
  glfwGetWindowSize(&_width, &_height);
#endif
  width = _width;
  height = _height;
}

//
void WinGlfw2::move(const int x, const int y) {
  int xx = x;
  int yy = y;
#ifdef HAVE_GLFW2
  glfwSetWindowPos(xx,yy);
#endif
  _posX = xx;
  _posY = yy;
}

//
/*
void WinGlfw2::setParameters(const ConfigParser& config) {
  Window::setParameters(config);
#ifdef HAVE_GLFW2
  try {
    if ( config.getToken<bool>("fullscreen") ) {
      glfwCloseWindow();
      GLFWvidmode desktop;
      glfwGetDesktopMode( &desktop );

      if ( !glfwOpenWindow(_width, _height, 
            desktop.RedBits, desktop.GreenBits, desktop.BlueBits,
            8, 24, 0, WinGlfw2::fullscreen) ) // alpha, depth, stencil, GLFW_FULLSCREEN
        throw EXCEPTION("Failed to open GLFW2 window",ERRDIV);

      glfwSetWindowTitle(_title.c_str());
      glfwEnable(GLFW_STICKY_KEYS);
      glfwSwapInterval(1); // On card that support vertical sync, activate it.
      glfwGetWindowSize(&_width,&_height);
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
bool WinGlfw2::getMouse(unsigned int key) {
  bool pressed = false;
  if ( key >= 8 ) {
    Exception e = EXCEPTION("Ignoring mouse key : out of range",ERRWAR);
    std::cerr << e.fullWhat() << std::endl;
    return pressed;
  }
#ifdef HAVE_GLFW2
  bool getkey = glfwGetMouseButton( key ) == GLFW_PRESS;
  pressed = !_stateMouse[key] && getkey;
  _stateMouse[key] = getkey;
#endif
  return pressed;
}

//
bool WinGlfw2::getMousePress(unsigned int key) {
  bool pressed = false;
  if ( key >= 8 ) {
    Exception e = EXCEPTION("Ignoring mouse key : out of range",ERRWAR);
    std::cerr << e.fullWhat() << std::endl;
    return pressed;
  }
#ifdef HAVE_GLFW2
  pressed = glfwGetMouseButton( key ) == GLFW_PRESS;
#endif
  return pressed;
}

//
void WinGlfw2::getWheelOffset(float &wheel) {
#ifdef HAVE_GLFW2
  static float old_wheel = 0.f;
  float new_wheel = 0.f;
  new_wheel = (float) glfwGetMouseWheel();
  wheel =  new_wheel - old_wheel;
  old_wheel = new_wheel;
#else
  wheel = 0.f;
#endif
}

//
void WinGlfw2::getMousePosition(float &x, float &y) {
#ifdef HAVE_GLFW2
  int xx, yy;
  glfwGetMousePos(&xx, &yy);
  x = (float) xx;
  y = (float) yy;
#else
  x = 0;
  y = 0;
#endif
}

//
bool WinGlfw2::getChar(unsigned key) {
  bool pressed = false;
  if ( key >= _maxKeys ) {
    Exception e = EXCEPTION("Ignoring key : out of range",ERRWAR);
    std::cerr << e.fullWhat() << std::endl;
    return pressed;
  }
#ifdef HAVE_GLFW2
  bool getkey = glfwGetKey( key ) == GLFW_PRESS;
  pressed = !_stateKey[key] && getkey;
  _stateKey[key] = getkey;
#endif
  return pressed;
}

//
bool WinGlfw2::getCharPress(unsigned key) {
#ifdef HAVE_GLFW2
  return (glfwGetKey( key ) == GLFW_PRESS);
#else
  return key && false;
#endif
}

#ifdef HAVE_GLFW2
void GLFWCALL WinGlfw2::CharCallback(int character, int action) {
  if ( action == GLFW_PRESS )
    _inputChar.push(static_cast<unsigned int>(character));
}
#endif
