/**
 * @file src/glwidget.cpp
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

#include "qtgui/glwidget.hpp"

#ifdef HAVE_QT

#include <sstream>
#include <string>
#include <iostream>
#include <QCursor>

//
GLWidget::GLWidget(pCanvas &canvas, const int width, const int height, int fps, QWidget *parent) : 
  QGLWidget(parent), 
  Window(canvas,width,height),
  _inputKeys({{false}}),
  _updateFromTimer(false),
  _wheelDelta(0.),
  _Timer(nullptr)
{
  if ( fps != 0 ) {
    int seconde = 1000; // 1 second = 1000 ms
    int timerInterval = seconde / fps;
    _Timer = new QTimer(this);
    connect(_Timer, SIGNAL(timeout()), this, SLOT(timeOut()));
    _Timer->start( timerInterval );
  }
  this->glInit();
  _mouseButtonLeft = 0;
  _mouseButtonRight = 1;
  _mouseButtonMiddle = 2;
  _keyEnter = 3;
  _keyKPEnter = 4;
  _keyBackspace = 5;
  _keyEscape = 6;
  _keyArrowUp = 7;
  _keyArrowDown = 8;
  _keyArrowLeft = 9;
  _keyArrowRight = 10;
  _keyX = 11;
  _keyY = 12;
  _keyZ = 13;
  this->setAcceptDrops(true);
  this->setCursor(QCursor(Qt::OpenHandCursor));
  this->makeCurrent();
}

//
GLWidget::~GLWidget() {
  ;
}


void GLWidget::keyPressEvent(QKeyEvent *keyEvent) {
  QString str = keyEvent->text();
  Qt::KeyboardModifiers modifiers = keyEvent->modifiers(); 
  if ( modifiers & Qt::ControlModifier ) {
    if ( keyEvent->text() == "v" )
    {
      QString text = QApplication::clipboard()->text();
      for ( int c = 0 ; c < text.size() ; ++c )
        _inputChar.push((unsigned char)text.data()[c].unicode());
      return;
    }
  }
  switch(keyEvent->key()) {
    case Qt::Key_Escape:
      _inputKeys[_keyEscape] = true;
      break;
    case Qt::Key_Return:
      _inputKeys[_keyEnter] = true;
      break;
    case Qt::Key_Enter:
      _inputKeys[_keyKPEnter] = true;
      break;
    case Qt::Key_Backspace:
      _inputKeys[_keyBackspace] = true;
      break;
    case Qt::Key_Up:
      _inputKeys[_keyArrowUp] = true;
      break;
    case Qt::Key_Down:
      _inputKeys[_keyArrowDown] = true;
      break;
    case Qt::Key_Left:
      _inputKeys[_keyArrowLeft] = true;
      break;
    case Qt::Key_Right:
      _inputKeys[_keyArrowRight] = true;
      break;
    case Qt::Key_X:
      _inputKeys[_keyX] = true;
      _inputChar.push((unsigned char)str.data()[0].unicode());
      break;
    case Qt::Key_Y:
      _inputKeys[_keyY] = true;
      _inputChar.push((unsigned char)str.data()[0].unicode());
      break;
    case Qt::Key_Z:
      _inputKeys[_keyZ] = true;
      _inputChar.push((unsigned char)str.data()[0].unicode());
      break;
    default :
      for ( int c = 0 ; c < str.size() ; ++c )
        _inputChar.push((unsigned char)str.data()[c].unicode());
      break;
  }
}

void GLWidget::keyReleaseEvent(QKeyEvent *keyEvent) {
  QString str = keyEvent->text();
  switch(keyEvent->key()) {
    case Qt::Key_Escape:
      _inputKeys[_keyEscape] = false;
      break;
    case Qt::Key_Return:
      _inputKeys[_keyEnter] = false;
      break;
    case Qt::Key_Enter:
      _inputKeys[_keyKPEnter] = false;
      break;
    case Qt::Key_Backspace:
      _inputKeys[_keyBackspace] = false;
      break;
    case Qt::Key_Up:
      _inputKeys[_keyArrowUp] = false;
      break;
    case Qt::Key_Down:
      _inputKeys[_keyArrowDown] = false;
      break;
    case Qt::Key_Left:
      _inputKeys[_keyArrowLeft] = false;
      break;
    case Qt::Key_Right:
      _inputKeys[_keyArrowRight] = false;
    case Qt::Key_X:
      _inputKeys[_keyX] = false;
      break;
    case Qt::Key_Y:
      _inputKeys[_keyZ] = false;
      break;
    case Qt::Key_Z:
      _inputKeys[_keyZ] = false;
      break;
  }
}

void GLWidget::mousePressEvent( QMouseEvent *mouseEvent ) {
  switch( mouseEvent->button() ) {
    case Qt::LeftButton:
      _inputKeys[_mouseButtonLeft] = true;
      this->setCursor(QCursor(Qt::ClosedHandCursor));
      break;
    case Qt::RightButton:
      _inputKeys[_mouseButtonRight] = true;
      break;
    //case Qt::MiddleButton:
    //  _inputKeys[_mouseButtonMiddle] = true;
    //  break;
    case Qt::NoButton:
    default:
      break;
  }
}

void GLWidget::mouseReleaseEvent( QMouseEvent *mouseEvent ) {
  switch( mouseEvent->button() ) {
    case Qt::LeftButton:
      _inputKeys[_mouseButtonLeft] = false;
      this->setCursor(QCursor(Qt::OpenHandCursor));
      break;
    case Qt::RightButton:
      _inputKeys[_mouseButtonRight] = false;
      break;
    //case Qt::MiddleButton:
    //  _inputKeys[_mouseButtonMiddle] = false;
    //  break;
    //case Qt::NoButton:
    default:
      break;
  }
}

void GLWidget::dragEnterEvent( QDragEnterEvent *dragEnterEvent ) {
  if (dragEnterEvent->mimeData()->hasFormat("text/uri-list"))
    dragEnterEvent->acceptProposedAction();
}

void GLWidget::dropEvent( QDropEvent *dropEvent ) {
  if ( _canvas == nullptr ) return;

  QList<QUrl> urls = dropEvent->mimeData()->urls();
  if (urls.isEmpty())
    return;

  QString fileName = urls.first().toLocalFile();
  if (fileName.isEmpty())
    return;

  try {
    _canvas->openFile(fileName.toStdString());
   size_t pos = fileName.toStdString().find_last_of("/\\");
   this->setTitle(fileName.toStdString().substr(pos+1));
  }
  catch ( Exception &e ) {
    e.ADD("Ignoring file "+fileName.toStdString(), ERRWAR);
    std::clog << e.fullWhat() << std::endl;
  }

  for ( auto url = urls.begin()+1 ; url != urls.end() ; ++url ) {
    QString fileName = url->toLocalFile();
    if ( !fileName.isEmpty() ) {
      try {
        _canvas->appendFile(fileName.toStdString());
      }
      catch ( Exception &e ) {
        e.ADD("Ignoring file "+fileName.toStdString(), ERRWAR);
        std::clog << e.fullWhat() << std::endl;
      }
    }
  }
}

void GLWidget::wheelEvent( QWheelEvent *wheelEvent ) {
  _wheelDelta = (double)wheelEvent->delta()/120.;
  // delta in 1/8 degree, one step is 15deg -> 15*8 = 120
}

void GLWidget::timeOut() {
  if ( _optioni["shouldExit"] == 1 ) {
    this->close();
    emit(closed());
    delete this;
    return;
  }
  else if ( _optioni["shouldExit"] == 2 ) this->window()->close();
  _updateFromTimer = true;
  updateGL();
}

void GLWidget::initializeGL() {
  Window::beginGl();
  _arrow.reset(new TriArrow(true));
}

void GLWidget::resizeGL(int width, int height) {
  _width = width;
  _height = height;
}

void GLWidget::paintGL() {

  if ( !_updateFromTimer ) _optioni["initBuffer"] = 0;
  Window::loopStep();
  _updateFromTimer = false;
  if ( _optionb["updated"] )
    emit(updated(this));
}

void GLWidget::setTitle(const std::string& title) {
  _title = title;
  emit(changedTitle(_title));
}

void GLWidget::setSize(const int width, const int height) {

  QWidget* main = this;
  while( main->parentWidget() != nullptr ) {
    main = main->parentWidget();
  }
  main->resize(width,height);
  int deltax = width - _width;
  int deltay = height - _height;
  if ( deltax > 0 || deltay > 0 )
    main->resize(width+deltax,height+deltay);

}

//
void GLWidget::getSize(int &width, int &height) {
  width = _width;
  height = _height;
}

//
void GLWidget::move(const int x, const int y) {
  int xx = x;
  int yy = y;
  _posX = xx;
  _posY = yy;
}

//
bool GLWidget::getMouse(unsigned int key) {
  if ( key >= _maxKeys ) return false;
  bool pressed = _inputKeys[key];
  _inputKeys[key] = false;
  return pressed;
}

//
bool GLWidget::getMousePress(unsigned int key) {
  if ( key >= _maxKeys ) return false;
  bool pressed = _inputKeys[key];
  return pressed;
}

//
void GLWidget::getWheelOffset(float &wheel) {
  wheel = (float) _wheelDelta;
  _wheelDelta = 0.;
}

//
void GLWidget::getMousePosition(float &x, float &y) {
  QPoint qpt = QCursor::pos();
  x = (float)qpt.rx();
  y = (float)qpt.ry();
}

//
bool GLWidget::getCharPress(unsigned key) {
  if ( key >= _maxKeys ) return false;
  bool pressed = _inputKeys[key];
  //_inputKeys[key] = false;
  return pressed;
}

//
bool GLWidget::getChar(unsigned key) {
  if ( key >= _maxKeys ) return false;
  bool pressed = _inputKeys[key];
  _inputKeys[key] = false;
  return pressed;
}

//
void GLWidget::loop(){
  this->show();
}

//
bool GLWidget::userInput(std::stringstream& info) {
  bool newAction = Window::userInput(info);
  if ( _mode == mode_process ) emit(commandProcessed(this));
  if ( newAction ) emit(userInput(this));
  return newAction;
}

//
QSize GLWidget::sizeHint() const {
  return QSize(_width,_height); 
}

void GLWidget::processCommand(std::string command,bool pop) {
#ifdef DEBUG
  std::clog << command << std::endl;
#endif
  if ( pop )
    while (_inputChar.size() > 0 ) _inputChar.pop();
  for ( unsigned int i = 0 ; i < command.size() ; ++i )
    _inputChar.push((unsigned int)command[i]);
  _inputChar.push((unsigned int)'\n');
}
#endif
