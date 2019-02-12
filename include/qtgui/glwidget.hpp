/**
 * @file include/glwidget.hpp
 *
 * @brief 
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


#ifndef GLWIDGET_HPP
#define GLWIDGET_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "window/window.hpp"
#ifdef HAVE_QT
#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Weffc++"
#      endif
#    endif
#    pragma GCC system_header
#  endif
#include <QtOpenGL>
#include <QGLWidget>
#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic pop
#      endif
#    endif
#  endif

/** 
 *
 */
class GLWidget : public QGLWidget, public Window {
  Q_OBJECT

  private :
    static const size_t _maxKeys = 14;
    std::array<bool,_maxKeys> _inputKeys;
    bool _updateFromTimer;
    double _wheelDelta;

  protected :
    QTimer *_Timer;

    /**
     * Manage user input to do things (zoom, rotation, ... )
     */
    virtual bool userInput(std::stringstream& info);

  public :

    /**
     * Constructor.
     */
    explicit GLWidget(pCanvas &canvas, const int width, const int height, int fps = 0, QWidget *parent = 0);

    /**
     * Copy Constructor.
     */
    GLWidget(const GLWidget& win) = delete;

    /**
     * Move Constructor.
     */
    GLWidget(GLWidget&& win) = delete;

    /**
     * Copy operator
     */
    GLWidget& operator = (const GLWidget& win) = delete;

    /**
     * MoveConstructor.
     */
    GLWidget& operator = (GLWidget&& win) = delete;


    /**
     * Destructor.
     */
    virtual ~GLWidget();

    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();
    virtual void keyPressEvent( QKeyEvent *keyEvent );
    virtual void keyReleaseEvent( QKeyEvent *keyEvent );
    virtual void mousePressEvent( QMouseEvent *mouseEvent );
    virtual void mouseReleaseEvent( QMouseEvent *mouseEvent );
    virtual void wheelEvent( QWheelEvent *mouseEvent );
    virtual void dragEnterEvent( QDragEnterEvent *dragEnterEvent );
    virtual void dropEvent( QDropEvent *dropEvent );

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
    virtual bool getMouse(unsigned int key);

    /**
     * Wrappe the getMousePress function to know if the button is pressed or not.
     * @param key the key of interest.
     * @return true if pressed _| and false otherwise
     */
    virtual bool getMousePress(unsigned int key);

    /**
     * Wrappe the get mouse wheel of the window library
     * @param wheel Get the position of the wheel inside this argument
     */
    virtual void getWheelOffset(float &wheel);

    /**
     * Wrappe the get mouse position of the window library
     * @param x absciss of the cursor
     * @param y ordonnate of the cursor
     */
    virtual void getMousePosition(float &x, float &y);

    bool getMovie() { return _movie; }

    /**
     * Swap the buffers if using several buffers for drawing.
     * No need for this with Qt...
     */
    virtual void swapBuffers() {};

    /**
     * Poll events in the queue
     */
    virtual void pollEvents() {};

    /**
     * Function to exit the main loop
     * @return always false since Qt handle the closing button itself
     */
    virtual bool exitMainLoop() { return false; }

    /**
     * Loop of the main application
     */
    virtual void loop();


    virtual QSize sizeHint() const;

    Canvas* getCanvas() {
      return _canvas.get();
    }

    void setCanvas(Canvas* canvas) {
      return _canvas.reset(canvas);
    }

    const std::map<std::string,bool>& optionBool() const {return _optionb;}
    const std::map<std::string,float>& optionFloat() const  {return _optionf;}
    const std::map<std::string,int>& optionInt() const {return _optioni;}

    //const float* background() const { return _background; }


  public slots:
    
    void timeOut();
    void processCommand(std::string,bool pop=true);
    void stop() { _Timer->stop(); while ( !_inputChar.empty() ) _inputChar.pop();}
    void start() { _Timer->start();}
    void emitCommandProcessed() { emit(commandProcessed(this)); }

  signals :

    void updated(GLWidget *glwidget);
    void commandProcessed(GLWidget *glwidget);
    void userInput(GLWidget *glwidget);
    void changedTitle(std::string);
    void closed();


};
#endif

#endif  // GLWIDGET_HPP
