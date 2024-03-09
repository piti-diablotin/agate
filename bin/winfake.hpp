/**
 * @file include/window/winfake.hpp
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


#ifndef WINFAKE_HPP
#define WINFAKE_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "window/window.hpp"

/** 
 * Fake window class for command line use
 * No GUI
 */
class Winfake : public Window{

  private :

  protected :

    /**
     * Wrappe the getKey function to only have an event from when a key is pressed until it is released
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    virtual bool getChar(unsigned key);

    /**
     * Wrappe the getKey function to know when a key is pressed currently pressed
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    virtual bool getCharPress(unsigned key);

    /**
     * Wrappe the getMouse function to only have an event from when a key is pressed until it is released
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    virtual bool getMouse(unsigned int key) {(void) key; return false;}

    /**
     * Wrappe the getMousePress function to know if the button is pressed or not.
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    virtual bool getMousePress(unsigned int key) {(void) key; return false;}

    /**
     * Wrappe the get mouse wheel of the window library
     * @param wheel Get the position of the wheel inside this argument
     */
    virtual void getWheelOffset(float &wheel) {(void) wheel;}

    /**
     * Wrappe the get mouse position of the window library
     * @param x absciss of the cursor
     * @param y ordonnate of the cursor
     */
    virtual void getMousePosition(float &x, float &y) {(void) x;(void) y;}

    /**
     * Swap the buffers if using several buffers for drawing.
     */
    virtual void swapBuffers() {;}

    /**
     * Poll events in the queue
     */
    virtual void pollEvents() {;}

    /**
     * Function to exit the main loop
     * @return true if loop should finish/
     */
    virtual bool exitMainLoop() {return false;}

    /**
     * Draw the cartesian axis on the left right corner
     */
    virtual void drawAxis() {;}


  public :
    /**
     * Basic stuff to do before anything
     */
    static void init();

    /**
     * Last thing to do before exiting the program
     */
    static void end();

    /**
     * Constructor.
     */
    Winfake(pCanvas &canvas, const int width, const int height);

    /**
     * Copy constructor.
     */
    Winfake(const Winfake& win) = delete;

    /**
     * Move constructor.
     */
    Winfake(Winfake&& win) = delete;

    /**
     * Copy operator
     */
    Winfake& operator = (const Winfake& win) = delete;

    /**
     * Move operator
     */
    Winfake& operator = (Winfake&& win) = delete;

    /**
     * Destructor.
     */
    virtual ~Winfake() {;}

    /**
     * Read some configuration parameters from config file if wanted.
     * @param filename The file containing the parameters
     */
    virtual void setParameters(const std::string &filename);

    /**
     * Loop of the main application
     */
    virtual void loop();

    /**
     * Loop of the main application
     * @param canvas Address of the canvas to use to plot the animation.
     */
    virtual void loopStep();

    /**
     * Take a snapshot of the current window.
     * And store the result in the correct format.
     */
    virtual void snapshot() {;}
};

#endif  // WINFAKE_HPP
