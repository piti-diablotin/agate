/**
 * @file src/viewbar.cpp
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


#include "qtgui/viewbar.hpp"
#include <QHBoxLayout>
#include <QFileDialog>

//
ViewBar::ViewBar(QWidget *parent) : QWidget(parent),
  _viewBar(nullptr),
  _axis(nullptr),
  _antialiasing(nullptr),
  _light(nullptr),
  _fill(nullptr),
  _projection(nullptr),
  _xplus(nullptr),
  _yplus(nullptr),
  _zplus(nullptr),
  _xminus(nullptr),
  _yminus(nullptr),
  _zminus(nullptr),
  _bg(nullptr),
  _fg(nullptr),
  _refresh(nullptr),
  _dump(nullptr),
  _dumpxyz(nullptr),
  _writeDtset(nullptr),
  _writePoscar(nullptr),
  _writeCif(nullptr),
  _write(nullptr),
  _currentPath("./")
{
  _viewBar = new QToolBar(this);
  //this->setFixedHeight(40);
  _viewBar->setFixedHeight(40);

  _axis = new QAction(QIcon(":/axis.png"),QString("Show/Hide axis"),_viewBar);
  _antialiasing= new QAction(QIcon(":/AA.png"),QString("Antialiasing"),_viewBar);
  _light = new QAction(QIcon(":/lighton.png"),QString("Switch light"),_viewBar);
  _fill = new QAction(QIcon(":/fill.png"),QString("Fill drawings"),_viewBar);
  _projection = new QAction(QIcon(":/projection.png"),QString("Projection"),_viewBar);
  _xplus = new QAction(QIcon(":/xplus.png"),QString("Add X translation"),_viewBar);
  _yplus = new QAction(QIcon(":/yplus.png"),QString("Add Y translation"),_viewBar);
  _zplus = new QAction(QIcon(":/zplus.png"),QString("Add Z translation"),_viewBar);
  _xminus = new QAction(QIcon(":/xminus.png"),QString("Remove X translation"),_viewBar);
  _yminus = new QAction(QIcon(":/yminus.png"),QString("Remove Y translation"),_viewBar);
  _zminus = new QAction(QIcon(":/zminus.png"),QString("Remove Z translation"),_viewBar);

  _bg = new QAction(QIcon(":/background.png"),QString("Background color"),_viewBar);
  _fg = new QAction(QIcon(":/foreground.png"),QString("Foreground color"),_viewBar);

  _refresh= new QAction(QIcon(":/refresh.png"),QString("Refresh"),_viewBar);
  QMenu *_dumpMenu = new QMenu(this);
  _dump = new QAction(QIcon(":/dump.png"),QString("Create new history file in the current format(_HIST/xyz)"),_viewBar);
  _dumpxyz = new QAction(QIcon(":/dump.png"),QString("Export to xyz file format"),_viewBar);
  _dumpMenu->addAction(_dumpxyz);
  _dump->setMenu(_dumpMenu);

  QMenu *_saveMenu = new QMenu(this);
  _writeDtset = new QAction(QIcon(":/write.png"),QString("Write current structure as Abinit input"),_viewBar);
  _writePoscar = new QAction(QIcon(":/write.png"),QString("Write current structure as POSCAR"),_viewBar);
  _writeCif = new QAction(QIcon(":/write.png"),QString("Write current structure as a CIF"),_viewBar);
  _saveMenu->addAction(_writeDtset);
  _saveMenu->addAction(_writePoscar);
  _saveMenu->addAction(_writeCif);
  _write = new QAction(QIcon(":/write.png"),QString("Write current structure"),_viewBar);
  _write->setMenu(_saveMenu);

  _viewBar->addAction(_write);
  _viewBar->addAction(_dump);
  QFrame *spacer1 = new QFrame(_viewBar);
  spacer1->setFrameShape(QFrame::VLine);
  spacer1->setFrameShadow(QFrame::Sunken);
  _viewBar->addWidget(spacer1);

  _viewBar->addAction(_refresh);
  QFrame *spacer2 = new QFrame(_viewBar);
  spacer2->setFrameShape(QFrame::VLine);
  spacer2->setFrameShadow(QFrame::Sunken);
  _viewBar->addWidget(spacer2);

  _viewBar->addAction(_xplus);
  _viewBar->addAction(_xminus);
  _viewBar->addAction(_yplus);
  _viewBar->addAction(_yminus);
  _viewBar->addAction(_zplus);
  _viewBar->addAction(_zminus);

  QFrame *spacer3 = new QFrame(_viewBar);
  spacer3->setFrameShape(QFrame::VLine);
  spacer3->setFrameShadow(QFrame::Sunken);
  _viewBar->addWidget(spacer3);

  _viewBar->addAction(_axis);
  _viewBar->addAction(_antialiasing);
  _viewBar->addAction(_light);
  _viewBar->addAction(_fill);
  _viewBar->addAction(_projection);

  _viewBar->addAction(_bg);
  _viewBar->addAction(_fg);

  _axis->setCheckable(true);
  _antialiasing->setCheckable(true);
  _light->setCheckable(true);
  _fill->setCheckable(true);
  _projection->setCheckable(true);

  QWidget *hSpacer = new QWidget(this);
  hSpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  layout->addWidget(_viewBar);
  layout->addWidget(hSpacer);
  this->setLayout(layout);

  connect(_axis,SIGNAL(triggered()),this,SLOT(toggleAxis()));
  connect(_antialiasing,SIGNAL(triggered()),this,SLOT(toggleAA()));
  connect(_light,SIGNAL(triggered()),this,SLOT(toggleLight()));
  connect(_fill,SIGNAL(triggered()),this,SLOT(toggleFill()));
  connect(_projection,SIGNAL(triggered()),this,SLOT(toggleProjection()));
  connect(_xplus,SIGNAL(triggered()),this,SLOT(plusX()));
  connect(_yplus,SIGNAL(triggered()),this,SLOT(plusY()));
  connect(_zplus,SIGNAL(triggered()),this,SLOT(plusZ()));
  connect(_xminus,SIGNAL(triggered()),this,SLOT(minusX()));
  connect(_yminus,SIGNAL(triggered()),this,SLOT(minusY()));
  connect(_zminus,SIGNAL(triggered()),this,SLOT(minusZ()));
  connect(_bg,SIGNAL(triggered()),this,SLOT(bgColor()));
  connect(_fg,SIGNAL(triggered()),this,SLOT(fgColor()));
  connect(_refresh,SIGNAL(triggered()),this,SLOT(refresh()));
  connect(_write,SIGNAL(triggered()),this,SLOT(writeDtset()));
  connect(_writeDtset,SIGNAL(triggered()),this,SLOT(writeDtset()));
  connect(_writePoscar,SIGNAL(triggered()),this,SLOT(writePoscar()));
  connect(_writeCif,SIGNAL(triggered()),this,SLOT(writeCif()));
  connect(_dump,SIGNAL(triggered()),this,SLOT(dump()));
  connect(_dumpxyz,SIGNAL(triggered()),this,SLOT(dumpxyz()));
}

//
ViewBar::~ViewBar() {
  ;
}

void ViewBar::makeConnexions(GLWidget *glwidget) {
  if ( glwidget != nullptr ) {
    connect(glwidget,SIGNAL(userInput(GLWidget*)),this,SLOT(refreshButtons(GLWidget*)));
    this->disconnect(SIGNAL(sentCommand(std::string)));
    connect(this,SIGNAL(sentCommand(std::string)),glwidget,SLOT(processCommand(std::string)));
    this->refreshButtons(glwidget);
  }
}

void ViewBar::refreshButtons(GLWidget *glwidget) {
  auto optB = glwidget->optionBool();
  bool hasHist = glwidget->getCanvas()->histdata() != nullptr;
  if ( optB["axis"] ^ _axis->isChecked() ) {
    _axis->toggle();
  }
    
  if ( optB["msaa"] ^ _antialiasing->isChecked() ) {
    _antialiasing->toggle();
    _antialiasing->setIcon(QIcon((_antialiasing->isChecked() ? ":/AA.png" : ":/A.png")));
  }

  if ( optB["paral_proj"] ^ !_projection->isChecked() )
    _projection->toggle();

  if ( glwidget->getCanvas()->light() ^ _light->isChecked() ) {
    _light->toggle();
    _light->setIcon(QIcon((_light->isChecked() ? ":/lighton.png" : ":/lightoff.png")));
  }

  if( glwidget->getCanvas()->drawing() ^ _fill->isChecked() )
    _fill->toggle();

  _write->setEnabled(hasHist);
  _dump->setEnabled(hasHist);
}

//
void ViewBar::toggleAxis() {
  emit(sentCommand(":axis"));
}

//
void ViewBar::toggleAA() {
  emit(sentCommand("A"));
  _antialiasing->setIcon(QIcon((_antialiasing->isChecked() ? ":/AA.png" : ":/A.png")));
}

//
void ViewBar::toggleLight() {
  emit(sentCommand("l"));
  _light->setIcon(QIcon((_light->isChecked() ? ":/lighton.png" : ":/lightoff.png")));
}

//
void ViewBar::toggleFill() {
  emit(sentCommand("o"));
}

//
void ViewBar::toggleProjection() {
  emit(sentCommand("p"));
}

//
void ViewBar::plusX() {
  emit(sentCommand("+x"));
}

//
void ViewBar::plusY() {
  emit(sentCommand("+y"));
    }

//
void ViewBar::plusZ() {
  emit(sentCommand("+z"));
}

//
void ViewBar::minusX() {
  emit(sentCommand("-x"));
}

//
void ViewBar::minusY() {
  emit(sentCommand("-y"));
}

//
void ViewBar::minusZ() {
  emit(sentCommand("-z"));
}

//
void ViewBar::fgColor() {
  QColor color(255,255,255);
  color = QColorDialog::getColor(color);
  std::ostringstream command;
  command << ":fg " << color.red() << " " << color.green() << " " << color.blue();
  emit(sentCommand(command.str()));
}

//
void ViewBar::bgColor() {
  QColor color(0,0,0);
  color = QColorDialog::getColor(color);
  std::ostringstream command;
  command << ":bg " << color.red() << " " << color.green() << " " << color.blue();
  emit(sentCommand(command.str()));
}

void ViewBar::refresh() {
  emit(sentCommand(":u"));
}

void ViewBar::writeDtset() {
  auto name = QFileDialog::getSaveFileName(this, "Save File", _currentPath+"structure.in", "Abinit (*.in)");
  if ( !name.isEmpty() ) {
    emit(sentCommand(":w dtset "+name.toStdString()));
    int pos = name.lastIndexOf(QRegExp("[/\\\\]"));
    _currentPath = name.left(pos+1);
    emit(changedPath(_currentPath));
  }
}

void ViewBar::writePoscar() {
  auto name = QFileDialog::getSaveFileName(this, "Save File", _currentPath+"POSCAR", "VASP (POSCAR)");
  if ( !name.isEmpty() ) {
    emit(sentCommand(":w poscar "+name.toStdString()));
    int pos = name.lastIndexOf(QRegExp("[/\\\\]"));
    _currentPath = name.left(pos+1);
    emit(changedPath(_currentPath));
  }
}

void ViewBar::writeCif() {
  auto name = QFileDialog::getSaveFileName(this, "Save File", _currentPath+"structure.cif", "CIF (*.cif)");
  if ( !name.isEmpty() ) {
    emit(sentCommand(":w cif "+name.toStdString()));
    int pos = name.lastIndexOf(QRegExp("[/\\\\]"));
    _currentPath = name.left(pos+1);
    emit(changedPath(_currentPath));
  }
}

void ViewBar::dump() {
  auto name = QFileDialog::getSaveFileName(this, "Save File",_currentPath);
  if ( !name.isEmpty() ) {
    emit(sentCommand(":dump "+name.toStdString()));
    int pos = name.lastIndexOf(QRegExp("[/\\\\]"));
    _currentPath = name.left(pos+1);
    emit(changedPath(_currentPath));
  }
}

void ViewBar::dumpxyz() {
  auto name = QFileDialog::getSaveFileName(this, "Save File",_currentPath);
  if ( !name.isEmpty() ) {
    emit(sentCommand(":dumpxyz "+name.toStdString()));
    int pos = name.lastIndexOf(QRegExp("[/\\\\]"));
    _currentPath = name.left(pos+1);
    emit(changedPath(_currentPath));
  }
}

void ViewBar::changePath(QString path) {
  _currentPath = path;
}
