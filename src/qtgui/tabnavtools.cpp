/**
 * @file src/tabnavtools.cpp
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


#include "qtgui/tabnavtools.hpp"
#include "base/utils.hpp"
#include <cmath>
#include <string>

//
TabNavTools::TabNavTools(QWidget *parent) : QWidget(parent),
  _timeLine(nullptr),
  _verticalLayout(nullptr),
  _actionBar(nullptr),
  _prevButton(nullptr),
  _nextButton(nullptr),
  _playButton(nullptr),
  _openButton(nullptr),
  _appendButton(nullptr),
  _fasterButton(nullptr),
  _slowerButton(nullptr),
  _recordButton(nullptr),
  _snapshotButton(nullptr),
  _repeatButton(nullptr),
  _hSpacer(nullptr),
  _nLoop(0),
  _currentPath("./")
{

  _verticalLayout = new QVBoxLayout(this);

  _timeLine = new TimeLine(this);
  _timeLine->setTimes(0,0,0,1);

  _verticalLayout->addWidget(_timeLine);


  _actionBar = new QToolBar(this);

  _prevButton = new QAction(QIcon(":/previous.png"),QString("Previous"),_actionBar);
  _nextButton = new QAction(QIcon(":/next.png"),QString("Next"),_actionBar);
  _playButton = new QAction(QIcon(":/play.png"),QString("Play/Pause"),_actionBar);
  _playButton->setCheckable(true);
  _fasterButton = new QAction(QIcon(":/faster.png"),QString("Faster"),_actionBar);
  _slowerButton = new QAction(QIcon(":/slower.png"),QString("Slower"),_actionBar);

  QFrame *spacer1 = new QFrame(this);
  spacer1->setFrameShape(QFrame::VLine);
  spacer1->setFrameShadow(QFrame::Sunken);


  _recordButton = new QAction(QIcon(":/record.png"),QString("Record"),_actionBar);
  _recordButton->setCheckable(true);
  _snapshotButton = new QAction(QIcon(":/snapshot.png"),QString("Snapshot"),_actionBar);

  QFrame *spacer2 = new QFrame(this);
  spacer2->setFrameShape(QFrame::VLine);
  spacer2->setFrameShadow(QFrame::Sunken);

  _repeatButton = new QAction(QIcon(":/repeat.png"),QString("Toggle Repeat/Palindrome"),_actionBar);
  _repeatButton->setCheckable(true);

  _hSpacer = new QWidget(this);
  _hSpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

  _openButton = new QAction(QIcon(":/open.png"),QString("Open"),_actionBar);
  _appendButton = new QAction(QIcon(":/append.png"),QString("Append"),_actionBar);

  _actionBar->addAction(_prevButton);
  _actionBar->addAction(_playButton);
  _actionBar->addAction(_nextButton);
  _actionBar->addAction(_slowerButton);
  _actionBar->addAction(_fasterButton);
  _actionBar->addWidget(spacer1);
  _actionBar->addAction(_recordButton);
  _actionBar->addAction(_snapshotButton);
  _actionBar->addWidget(spacer2);
  _actionBar->addAction(_repeatButton);
  ///_actionBar->addWidget(_timeLine);
  _actionBar->addWidget(_hSpacer);
  _actionBar->addAction(_openButton);
  _actionBar->addAction(_appendButton);

  _verticalLayout->addWidget(_actionBar);

  this->setLayout(_verticalLayout);

  connect(_timeLine,SIGNAL(timeBeginChanged(int)),this,SLOT(setTimeBegin(int)));
  connect(_timeLine,SIGNAL(timeEndChanged(int)),this,SLOT(setTimeEnd(int)));
  connect(_timeLine,SIGNAL(timeChanged(int)),this,SLOT(setTime(int)));

  connect(_openButton,SIGNAL(triggered()),this,SLOT(openFile()));
  connect(_appendButton,SIGNAL(triggered()),this,SLOT(appendFile()));

  connect(_nextButton,SIGNAL(triggered()),this,SLOT(nextTime()));
  connect(_prevButton,SIGNAL(triggered()),this,SLOT(prevTime()));
  connect(_playButton,SIGNAL(triggered()),this,SLOT(togglePlay()));
  connect(_slowerButton,SIGNAL(triggered()),this,SLOT(slower()));
  connect(_fasterButton,SIGNAL(triggered()),this,SLOT(faster()));
  connect(_recordButton,SIGNAL(triggered()),this,SLOT(record()));
  connect(_snapshotButton,SIGNAL(triggered()),this,SLOT(snapshot()));
  connect(_repeatButton,SIGNAL(triggered()),this,SLOT(toggleRepeat()));

}

//
TabNavTools::~TabNavTools() {
}

void TabNavTools::makeConnexions(GLWidget *glwidget) {
  if ( glwidget != nullptr )  {
    connect(glwidget,SIGNAL(userInput(GLWidget*)),this,SLOT(refreshButtons(GLWidget*)));
    connect(glwidget,SIGNAL(updated(GLWidget*)),this,SLOT(update(GLWidget*)));
    this->disconnect(SIGNAL(sentCommand(std::string)));
    connect(this,SIGNAL(sentCommand(std::string)),glwidget,SLOT(processCommand(std::string)));
    this->refreshButtons(glwidget);
  }
  //emit(sentCommand("t"));
}

void TabNavTools::refreshButtons(GLWidget *glwidget) {
  using std::max;
  Canvas *canvas = glwidget->getCanvas();
  if ( (_recordButton->isChecked() ^ glwidget->getMovie()) ) _recordButton->toggle();
  switch ( canvas->nLoop() ) {
    case -2 :
      if ( !_repeatButton->isChecked() ) {
        _repeatButton->toggle();
        _repeatButton->setIcon(QIcon(":/palindrome.png"));
      }
      break;
    case -1 :
      if ( !_repeatButton->isChecked() ) {
        _repeatButton->toggle();
        _repeatButton->setIcon(QIcon(":/repeat.png"));
      }
      break;
    case 0 :
      if ( _repeatButton->isChecked() ) {
        _repeatButton->toggle();
        _repeatButton->setIcon(QIcon(":/repeat.png"));
      }
    default :
      canvas->nLoop(0);
      if ( _repeatButton->isChecked() ) {
        _repeatButton->toggle();
      }
      _repeatButton->setIcon(QIcon(":/repeat.png"));
  }
  _nLoop = canvas->nLoop();
  int ntime = max(canvas->ntime(),1);
  if ( ntime < 2 ) {
    _timeLine->setEnabled(false);
    _playButton->setEnabled(false);
    _prevButton->setEnabled(false);
    _nextButton->setEnabled(false);
    _slowerButton->setEnabled(false);
    _fasterButton->setEnabled(false);
    _repeatButton->setEnabled(false);
    _recordButton->setEnabled(false);
  }
  else {
    _timeLine->setEnabled(true);
    _playButton->setEnabled(true);
    _prevButton->setEnabled(true);
    _nextButton->setEnabled(true);
    _slowerButton->setEnabled(true);
    _fasterButton->setEnabled(true);
    _repeatButton->setEnabled(true);
    _recordButton->setEnabled(true);
  }
  _appendButton->setEnabled(canvas->histdata() != nullptr);
  _timeLine->setTimes(canvas->tbegin(),max(canvas->tend(),0),canvas->itime(),ntime);

  _appendButton->setEnabled(canvas->histdata() != nullptr);

}

void TabNavTools::update(GLWidget *glwidget) {
  using std::max;
  Canvas *canvas = glwidget->getCanvas();
  _timeLine->setTime(canvas->itime());
  int newEnd;
  if ( _timeLine->timeEnd() != (newEnd=max(0,canvas->tend())) )
    _timeLine->setTimeEnd(newEnd);
  if ( (_playButton->isChecked() ^ !canvas->isPaused()) ) _playButton->toggle();
}

void TabNavTools::setTimeBegin(int time) {
  emit(sentCommand(std::string(":tbegin ")+utils::to_string(time)));
}

void TabNavTools::setTimeEnd(int time) {
  emit(sentCommand(std::string(":tend ")+utils::to_string(time)));
}

void TabNavTools::setTime(int time) {
  emit(sentCommand(std::string(":")+utils::to_string(time)));
}

void TabNavTools::openFile() {
  auto fileNames = QFileDialog::getOpenFileNames(this,"Open File",_currentPath,"Abinit (*.in *.out *_OUT.nc *_HIST *_HIST.nc *_DDB);;VASP (POSCAR);;CIF (*.cif);;XML (*.xml);;XYZ (*.xyz);;All (*)");

  std::string command;
  if ( !fileNames.empty() ) {
    QString file1 = fileNames.first();
    if (file1.isEmpty())
      return;
    command = ":open "+file1.toStdString()+"\n";
    int pos = file1.lastIndexOf(QRegExp("[/\\\\]"));
    _currentPath = file1.left(pos+1);
    emit(changedPath(_currentPath));
    //emit(openedFile(file1.toStdString().substr(pos+1)));

    for ( auto file = fileNames.begin()+1 ; file != fileNames.end() ; ++file ) {
      if ( !file->isEmpty() ) {
        command = command + ":append "+file->toStdString() + "\n";
      }
    }
    emit(sentCommand(command));
  }
}

void TabNavTools::appendFile() {
  auto fileNames = QFileDialog::getOpenFileNames(this,"Append File",_currentPath,"Abinit (*.in *.out *_OUT.nc *_HIST *_HIST.nc *_DDB);;VASP (POSCAR);;CIF (*.cif);;XML (*.xml);;XYZ (*.xyz);;All (*)");

  std::string command;
  if ( !fileNames.empty() ) {
    for ( auto file = fileNames.begin() ; file != fileNames.end() ; ++file ) {
      if ( !file->isEmpty() ) {
        command = command + ":append "+file->toStdString() + "\n";
        int pos = file->lastIndexOf(QRegExp("[/\\\\]"));
        _currentPath = file->left(pos+1);
      }
    }
    emit(changedPath(_currentPath));
    emit(sentCommand(command));
  }
}
//
void TabNavTools::togglePlay() {
  emit(sentCommand(" "));
}

//
void TabNavTools::prevTime() {
  emit(sentCommand("<"));
}

//
void TabNavTools::nextTime() {
  emit(sentCommand(">"));
}

//
void TabNavTools::faster() {
  emit(sentCommand("*"));
}

//
void TabNavTools::slower() {
  emit(sentCommand("/"));
}

//
void TabNavTools::record() {
  emit(sentCommand("m"));
}

//
void TabNavTools::snapshot() {
  emit(sentCommand("s"));
}

void TabNavTools::toggleRepeat(){
  if ( --_nLoop < -2 ) _nLoop = 0;
  emit(sentCommand(":repeat "+utils::to_string(_nLoop)));
  switch ( _nLoop ) {
    case -2 :
      _repeatButton->toggle();
      _repeatButton->setIcon(QIcon(":/palindrome.png"));
      break;
    case -1 :
      _repeatButton->setIcon(QIcon(":/repeat.png"));
      break;
    case 0 :
      _repeatButton->setIcon(QIcon(":/repeat.png"));
      break;
  }
}

//
void TabNavTools::changePath(QString path) {
  _currentPath = path;
}
