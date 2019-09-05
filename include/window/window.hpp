/**
 * @file include/window.hpp
 *
 * @brief Pure virtual class to be use with by window manager (glfw glx glut ...)
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


#ifndef WINDOW_HPP
#define WINDOW_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <string>
#include <sstream>
#include <queue>
#include <map>
#include <memory>
#include "io/configparser.hpp"
#include "canvas/canvas.hpp"
#include "graphism/imagesaver.hpp"
#include "graphism/textrender.hpp"
#include "graphism/triarrow.hpp"
#ifdef HAVE_GL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

/** 
 * Basic class to handle a window. 
 * Is independant of the window library manager.
 */
class Window {

  public :

    enum ImageSuffix { convert, animate };

#ifdef HAVE_GL
    /*
    static void GLAPIENTRY errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
    */
#endif

  private :

    /**
     * Make the correct transformation to set the camera view
     * @param zoom is the total factor for camera vector
     * @param tx is the target point x coordinate
     * @param ty is the target point y coordinate
     * @param tz is the target point z coordinate
     */
    void lookAt(double zoom, double tx, double ty, double tz);

    /**
     * Process the command with the corresponding options
     * @param token The corresponding token from the user
     * @param cin The stream with all the options if needed
     */
    void my_alter(std::string &token, std::istringstream &cin);

    /**
     * Process mouse event
     * @return tru if an action occured
     */
    bool handleMouse();


  protected :

    enum InputMode { mode_mouse, mode_static, mode_command, mode_add, mode_remove, mode_process};

    bool           _exit;          ///< flag to exit the main loop by signals or other thread.
    int            _posX;          ///< Horizontal position (0 is left)
    int            _posY;          ///< Vertical position (0 is up)
    int            _width;         ///< width of the window
    int            _height;        ///< height of the window
    int            _suffix;        ///< Like a number for numbering the screenshots.
    std::string    _title;         ///< Title of the window
    float          _background[3]; ///< Color for the background

    ImageSaver     _image;       ///< Export a snapshot to a given format.
    char          *_imageBuffer; ///< Keep an allocated pointer to save allocations cost when making a movie.
    bool           _keepImage;   ///< Keep the allocated _imageBuffer or not.
    int            _imageSize;   ///< Keep the size of _imageBuffer
    ImageSuffix    _imageSuffixMode; ///< What suffix do we use
    bool           _movie;       ///< Make a movie of the full animation.

    unsigned int _mouseButtonLeft;        ///< Set the correct value according to the library.
    unsigned int _mouseButtonRight;       ///< Set the correct value according to the library.
    unsigned int _mouseButtonMiddle;      ///< Set the correct value according to the library.
    unsigned int _keyEnter;      ///< Set the correct value according to the library.
    unsigned int _keyKPEnter;    ///< Set the correct value according to the library.
    unsigned int _keyBackspace;      ///< Set the correct value according to the library.
    unsigned int _keyEscape;      ///< Set the correct value according to the library.
    unsigned int _keyArrowUp;     ///< Set the correct value according to the library.
    unsigned int _keyArrowDown;   ///< Set the correct value according to the library.
    unsigned int _keyArrowLeft;   ///< Set the correct value according to the library.
    unsigned int _keyArrowRight;  ///< Set the correct value according to the library.
    unsigned int _keyX;           ///< Set the correct value according to the library.
    unsigned int _keyY;           ///< Set the correct value according to the library.
    unsigned int _keyZ;           ///< Set the correct value according to the library.
    InputMode    _mode;           ///< Current mode for user input
    InputMode    _modeMouse;      ///< Current mode for mouse input
    std::string  _command;        ///< String to know the action to perform
    std::vector<std::string> _commandStack; ///< Store all the entered commands
    unsigned int _commandStackNo; ///Current line in the stack

    TextRender   _render; ///< Save some data for text rendering and quickly render a string
    std::map<std::string,bool> _optionb;
    std::map<std::string,float> _optionf;
    std::map<std::string,int> _optioni;
#ifdef HAVE_CPPTHREAD
    std::unique_ptr<std::thread> _snake;
#endif

    pCanvas _canvas; ///< Canvas to use
    std::unique_ptr<TriArrow> _arrow;

    static std::queue<unsigned int> _inputChar; ///< Store all character dropped by the glfw callback function.

    /**
     * Start OpenGl with the correct option
     */
    static void beginGl();

    /**
     * Wrappe the getKey function to only have an event from when a key is pressed until it is released
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    virtual bool getChar(unsigned key) = 0;

    /**
     * Wrappe the getKey function to know when a key is pressed currently pressed
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    virtual bool getCharPress(unsigned key) = 0;

    /**
     * Wrappe the getMouse function to only have an event from when a key is pressed until it is released
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    virtual bool getMouse(unsigned int key) = 0 ;

    /**
     * Wrappe the getMousePress function to know if the button is pressed or not.
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    virtual bool getMousePress(unsigned int key) = 0;

    /**
     * Wrappe the get mouse wheel of the window library
     * @param wheel Get the position of the wheel inside this argument
     */
    virtual void getWheelOffset(float &wheel) = 0;

    /**
     * Wrappe the get mouse position of the window library
     * @param x absciss of the cursor
     * @param y ordonnate of the cursor
     */
    virtual void getMousePosition(float &x, float &y) = 0;

    /**
     * Swap the buffers if using several buffers for drawing.
     */
    virtual void swapBuffers() = 0;

    /**
     * Poll events in the queue
     */
    virtual void pollEvents() = 0;

    /**
     * Function to exit the main loop
     * @return true if loop should finish/
     */
    virtual bool exitMainLoop() = 0;

    /**
     * Draw the cartesian axis on the left right corner
     */
    virtual void drawAxis();

    /**
     * Manage user input to do things (zoom, rotation, ... )
     */
    virtual bool userInput(std::stringstream& info);

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
     * Help function displaying all command available
     */
    static void help();

    /**
     * Get version information
     * @param major Major number
     * @param minor Minor number
     * @param rev revision number of this distrib (window manager)
     */
    static void version(int &major, int &minor, int &rev);

    /**
     * Constructor.
     */
    Window(pCanvas &canvas, const int width, const int height);

    Window();

    /**
     * Copy constructor.
     */
    Window(const Window& win) = delete;

    /**
     * Move constructor.
     */
    Window(Window&& win) = delete;

    /**
     * Copy operator
     */
    Window& operator = (const Window& win) = delete;

    /**
     * Move operator
     */
    Window& operator = (Window&& win) = delete;

    /**
     * Destructor.
     */
    virtual ~Window(){
      if ( _imageBuffer != nullptr ) {
        delete[] _imageBuffer;
        _imageBuffer = nullptr;
      }
      utils::fftw3Free();
    }

    /**
     * Set the title of the window
     * @param title The new title
     */
    virtual void setTitle(const std::string& title)
    { _title = title; }

    /**
     * Set the font to use for displaying information
     * @param font The new font
     */
    inline void setFont(const std::string& font)
    { _render._render.setFont(font); }

    /**
     * Set the new size of the window
     * @param width The new width
     * @param height The new height
     */
    virtual void setSize(const int width, const int height)
    { _width = width; _height = height; }

    /** 
     * Get the size of the window in the references
     * @param width Reference to an int that will hold the widht of the window
     * @param height Reference to an int that will hold the height of the window
     */
    virtual void getSize(int &width, int &height)
    { width = _width; height = _height; }

    /**
     * Move the position of the window
     * @param x New value for horizontal position.
     * @param y New value for vertical position.
     */
    virtual void move(const int x, const int y)
    { _posX = x; _posY = y; }

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
    virtual void snapshot();

    /**
     * Set flag to exit the main loop
     */
    virtual void exit() { _exit = true; }
    
    virtual Canvas* canvas() const {return _canvas.get();}

    virtual void canvas(pCanvas &canvas) {_canvas.reset(canvas.release());}
    virtual void canvas(Canvas* canvas) {_canvas.reset(canvas);canvas=nullptr;}

};

#endif  // WINDOW_HPP
