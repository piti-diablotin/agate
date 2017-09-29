/**
 * @file src/tabcanvaspos.cpp
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


#include "qtgui/tabcanvaspos.hpp"
#include "canvas/canvaspos.hpp"
#include "canvas/canvasphonons.hpp"
#include "qtgui/atomselector.hpp"
#include <typeinfo>

//
TabCanvasPos::TabCanvasPos(QWidget *parent) :QWidget(parent),
  _natom(0),
  _posBar(nullptr),
  _display(nullptr),
  _buttonBorder(nullptr),
  _buttonPeriodic(nullptr),
  _displayed(),
  _angle(nullptr),
  _distance(nullptr),
  _centroid(nullptr)
{

  _posBar = new QToolBar(this);

  _display = new QComboBox(this);
  _display->addItem("none");
  _display->addItem("id");
  _display->addItem("znucl");
  _display->addItem("name");

  _buttonBorder = new QCheckBox("Boundaries");
  _buttonBorder->setChecked(true);

  _buttonPeriodic = new QCheckBox("Periodic");
  _buttonPeriodic->setChecked(true);

  QLabel *label = new QLabel("Display:",this);

  _posBar->addWidget(label);
  _posBar->addWidget(_display);
  _posBar->addWidget(_buttonBorder);
  _posBar->addWidget(_buttonPeriodic);

  QFrame *spacer1 = new QFrame(_posBar);
  spacer1->setFrameShape(QFrame::VLine);
  spacer1->setFrameShadow(QFrame::Sunken);
  _posBar->addWidget(spacer1);

  _angle = new QAction(QIcon(":/compass.png"),QString("Get angle between 3 atoms"),_posBar);
  _distance= new QAction(QIcon(":/ruler.png"),QString("Get distance between 2 atoms"),_posBar);
  _posBar->addAction(_distance);
  _posBar->addAction(_angle);

  QFrame *spacer2 = new QFrame(_posBar);
  spacer2->setFrameShape(QFrame::VLine);
  spacer2->setFrameShadow(QFrame::Sunken);
  _posBar->addWidget(spacer2);

  _centroid= new QAction(QIcon(":/centroid.png"),QString("Compute the centroid"),_posBar);
  _posBar->addAction(_centroid);

  connect(_display,SIGNAL(currentIndexChanged(int)),this,SLOT(changeDisplay(int)));
  connect(_buttonBorder,SIGNAL(stateChanged(int)),this,SLOT(displayBorder(int)));
  connect(_buttonPeriodic,SIGNAL(stateChanged(int)),this,SLOT(periodic(int)));
  connect(_angle,SIGNAL(triggered()),this,SLOT(angle()));
  connect(_distance,SIGNAL(triggered()),this,SLOT(distance()));
  connect(_centroid,SIGNAL(triggered()),this,SLOT(centroid()));

  QWidget *hSpacer = new QWidget(this);
  hSpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  layout->addWidget(_posBar);
  layout->addWidget(hSpacer);
  this->setLayout(layout);
}

//
TabCanvasPos::~TabCanvasPos() {
  ;
}

//
void TabCanvasPos::makeConnexions(GLWidget *glwidget) {
  if ( glwidget != nullptr ) {
    connect(glwidget,SIGNAL(commandProcessed(GLWidget*)),this,SLOT(refreshButtons(GLWidget*)));
    this->disconnect(SIGNAL(sentCommand(std::string)));
    connect(this,SIGNAL(sentCommand(std::string)),glwidget,SLOT(processCommand(std::string)));
    this->refreshButtons(glwidget);
  }
}

void TabCanvasPos::refreshButtons(GLWidget *glwidget) {
  if ( ( typeid(*(glwidget->getCanvas())) == typeid(CanvasPos)  
        || typeid(*(glwidget->getCanvas())) == typeid(CanvasPhonons) ) 
      && glwidget->getCanvas()->histdata() != nullptr ) {
    this->show();
    CanvasPos *local = reinterpret_cast<CanvasPos*>(glwidget->getCanvas());
    unsigned display = local->getDisplay();
    bool shouldBeChecked = display & CanvasPos::DISP_BORDER;
    if ( shouldBeChecked != _buttonBorder->isChecked() ) {
      _buttonBorder->disconnect(this);
      _buttonBorder->setChecked(shouldBeChecked);
      connect(_buttonBorder,SIGNAL(stateChanged(int)),this,SLOT(displayBorder(int)));
    }
    shouldBeChecked = local->histdata()->isPeriodic();
    if ( shouldBeChecked != _buttonPeriodic->isChecked() ) {
      _buttonPeriodic->disconnect(this);
      _buttonPeriodic->setChecked(shouldBeChecked);
      connect(_buttonPeriodic,SIGNAL(stateChanged(int)),this,SLOT(periodic(int)));
    }
    int opt = 0;
    if ( display & CanvasPos::DISP_ID ) opt = 1;
    else if ( display & CanvasPos::DISP_ZNUCL ) opt = 2;
    else if ( display & CanvasPos::DISP_NAME ) opt = 3;
    if ( _display->currentIndex() != opt ) {
      _display->disconnect(this);
      _display->setCurrentIndex(opt);
      connect(_display,SIGNAL(currentIndexChanged(int)),this,SLOT(changeDisplay(int)));
    }
    _centroid->setEnabled(local->histdata()->nimage()>1);
    _natom = local->histdata()->natom();
  }
  else {
    this->hide();
    _natom = 0;
  }
  _angle->setEnabled((_natom > 2));
  _distance->setEnabled((_natom > 1));
}

//
void TabCanvasPos::changeDisplay(int index) {
  const std::string display = ":hide ";
  switch ( index ) {
    case 0 :
      emit(sentCommand(display+_displayed));
      _displayed = "";
      return;
      break;
    case 1 :
      _displayed = "id";
      break;
    case 2 :
      _displayed = "znucl";
      break;
    case 3 :
      _displayed = "name";
      break;
  }
  emit(sentCommand(std::string(":show "+_displayed)));
}

//
void TabCanvasPos::displayBorder(int state) {
  ( state == Qt::Unchecked ) 
    ? emit(sentCommand(":hide border"))
    : emit(sentCommand(":show border"));
}

//
void TabCanvasPos::periodic(int state) {
  ( state == Qt::Unchecked ) 
    ? emit(sentCommand(":periodic 0"))
    : emit(sentCommand(":periodic 1"));
}

//
void TabCanvasPos::centroid() {
  emit(sentCommand(":centroid"));
}

//
void TabCanvasPos::angle() {
  AtomSelector select(3,_natom,this);
  select.setModal(true);
  std::string command=":angle ";
  if ( select.exec() == QDialog::Accepted ) {
    auto atoms = select.atoms();
    for ( auto atom : atoms )
      command+= (utils::to_string(atom)+" ");
    emit(sentCommand(command));
  }
}

//
void TabCanvasPos::distance() {
  AtomSelector select(2,_natom,this);
  select.setModal(true);
  std::string command=":distance ";
  if ( select.exec() == QDialog::Accepted ) {
    auto atoms = select.atoms();
    for ( auto atom : atoms )
      command+= (utils::to_string(atom)+" ");
    emit(sentCommand(command));
  }
}
