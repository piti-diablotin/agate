/**
 * @file include/winglfw2.hpp
 *
 * @brief Wrappe the glfw stuff inside this class
 * Could be adapted to other window managers.
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


#ifndef WINGLFW2_HPP
#define WINGLFW2_HPP

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
#ifdef HAVE_GLFW2
#  include <GL/glfw.h>
#else
# define GLFW_WINDOW 0     ///< Fake variable
# define GLFW_FULLSCREEN 1 ///< Fake variable
#endif
#ifdef HAVE_GL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

/** 
 * Wrapper for GLFW2
 */
class WinGlfw2 : public Window {

  private :

    static const unsigned _maxKeys = 350;      ///< Maximum number of keys that can be monitored
    bool                  _stateKey[_maxKeys]; ///< Keep track of the status of the keys
    bool                  _stateMouse[8];      ///< Keep track of the status of the keys

#ifdef HAVE_GLFW2
    /**
     * Callback function for input char
     */
    static void GLFWCALL CharCallback(int character, int action);
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
#ifdef HAVE_GLFW2
      glfwSwapBuffers();
#endif
    }

    /**
     * Poll events in the queue
     */
    void pollEvents() {
#ifdef HAVE_GLFW2
      glfwPollEvents();
#endif
    }

    /**
     * Function to exit the main loop
     * @return true if loop should finish/
     */
    bool exitMainLoop() { 
#ifdef HAVE_GLFW2
      return !glfwGetWindowParam( GLFW_OPENED );
#else
      return true;
#endif
    }

  public :

    const static int window     = GLFW_WINDOW; ///< Open a window with borders.
    const static int fullscreen = GLFW_FULLSCREEN; ///< Open a window in fullscreen.

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
     * @param width Width of the window
     * @param height of the desired windo
     * @param mode should it be in fullscreen or window mode
     */
    WinGlfw2(pCanvas &canvas, const int width, const int height, const int mode = WinGlfw2::window);

    /**
     * Destructor.
     */
    virtual ~WinGlfw2();

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

    /**
     * Read some configuration parameters from config file if wanted.
     * @param config The parser containing the parameters
     */
    //void setParameters(const ConfigParser& config);

    /**
     * Close the current window
     */
    void exit() { 
      _exit = true; 
#ifdef HAVE_GLFW2
      glfwCloseWindow();
#endif
    }


};

#endif  // WINGLFW2_HPP
