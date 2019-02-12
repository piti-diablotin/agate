/**
 * @file include/winglfw3.hpp
 *
 * @brief Wrappe the glfw version 3 stuff inside this class
 * Could be adapted to other window managers.
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


#ifndef WINGLFW3_HPP
#define WINGLFW3_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "window/window.hpp"
#include "io/configparser.hpp"
#include "canvas/canvas.hpp"
#include <string>
#include <map>
#include <array>
#ifdef HAVE_GL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif
#ifdef HAVE_GLFW3
#  include <GLFW/glfw3.h>
#endif

/** 
 * Wrapper for GLFW3
 */
class WinGlfw3 : public Window {

  private :

    static const unsigned _maxKeys = 350;      ///< Maximum number of keys that can be monitored
#ifdef HAVE_GLFW3
    GLFWwindow           *_win;                ///< opaque  handler for window
#endif
    bool                  _stateKey[_maxKeys]; ///< Keep track of the status of the keys
    bool                  _stateMouse[8];      ///< Keep track of the status of the keys

#ifdef HAVE_GLFW3
    static std::map<GLFWwindow*, std::array<float,2> > _offsets;    ///< Offset of all the windows. To enable the callback function to access it.

    /**
     * Callback function for mouse wheel
     */
    static void WheelCallback(GLFWwindow* win, double xoffset, double yoffset);

    /**
     * Callback function for input char
     */
    static void CharCallback(GLFWwindow* win, unsigned int codepoint);

    /**
     * Callback function for input key (ctrl+v ...)
     */
    static void KeyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);

    /**
     * Callback function for error handling
     */
    static void ErrorCallback(int noeglfw, const char *message);
#endif

  protected :

    /**
     * Wrappe the getKey function to only have an event from when a key is pressed until it is released
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    bool getChar(unsigned key);

    /**
     * Wrappe the getKey function to know when a key is pressed currently pressed
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    bool getCharPress(unsigned key);

    /**
     * Wrappe the get mouse wheel of the window library
     * @param wheel Get the position of the wheel inside this argument
     */
    void getWheelOffset(float &wheel);

    /**
     * Wrappe the getMouse function to only have an event from when a key is pressed until it is released
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    bool getMouse(unsigned int key);

    /**
     * Wrappe the getMousePress function to know if the button is pressed or not.
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    virtual bool getMousePress(unsigned int key);

    /**
     * Wrappe the get mouse position of the window library
     * @param x absciss of the cursor
     * @param y ordonnate of the cursor
     */
    void getMousePosition(float &x, float &y);

    /**
     * Swap the buffers if using several buffers for drawing.
     */
    void swapBuffers() {
#ifdef HAVE_GLFW3
      glfwSwapBuffers(_win);
      glfwPollEvents();
#endif
    }

    /**
     * Poll events in the queue
     */
    void pollEvents() {
#ifdef HAVE_GLFW3
      glfwPollEvents();
#endif
    }

    /**
     * Function to exit the main loop
     * @return true if loop should finish/
     */
    bool exitMainLoop() { 
#ifdef HAVE_GLFW3
      return 0<glfwWindowShouldClose( _win );
#else
      return true;
#endif
    }

  public :

    const static int window     = 0; ///< Open a window with borders.
    const static int fullscreen = 1; ///< Open a window in fullscreen.

    /**
     * Initialize GLFW
     */
    static void init();

    /**
     * Terminate GLFW
     */
    static void end();

    /**
     * Get version of GLFW
     * @param major reference to an int that will hold the major number
     * @param minor reference to an int that will hold the minor number
     * @param rev   reference to an int that will hold the revision.
     */
    static void version(int &major, int &minor, int &rev);


    /**
     * Constructor.
     */
    WinGlfw3(pCanvas &canvas, const int width, const int height, const int mode = WinGlfw3::window);

    /**
     * Copy Constructor.
     */
    WinGlfw3(const WinGlfw3& win) = delete;

    /**
     * Move Constructor.
     */
    WinGlfw3(WinGlfw3&& win) = delete;

    /**
     * Copy operator
     */
    WinGlfw3& operator = (const WinGlfw3& win) = delete;

    /**
     * MoveConstructor.
     */
    WinGlfw3& operator = (WinGlfw3&& win) = delete;

    /**
     * Destructor.
     */
    virtual ~WinGlfw3();

    /**
     * Set the title of the window
     * @param title The new title
     */
    void setTitle(const std::string& title);

    /**
     * Set the new size of the window
     * @param width The new width
     * @param height The new height
     */
    void setSize(const int width, const int height);

    /** 
     * Get the size of the window in the references
     * @param width Reference to an int that will hold the widht of the window
     * @param height Reference to an int that will hold the height of the window
     */
    void getSize(int &width, int &height);

    /**
     * Move the position of the window
     * @param x New value for horizontal position.
     * @param y New value for vertical position.
     */
    void move(const int x, const int y);

#ifdef HAVE_GLFW3_DROP
    /**
     * Set the drop callback function for the current window.
     * @param cbfun The callback function to call when something
     * is dropped into the window.
     */
    void setDropCallback(GLFWdropfun cbfun);
#endif

    /**
     * Close the current window
     */
    void exit() { 
      _exit = true; 
    }

};

#endif  // WINGLFW3_HPP
