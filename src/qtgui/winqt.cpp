/**
 * @file src/winqt.cpp
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



#include "qtgui/winqt.hpp"
#include "canvas/canvaspos.hpp"

#ifdef HAVE_QT

//
WinQt::WinQt(QWidget *parent) :
  QWidget(parent),
  _verticalLayout(nullptr),
  _tabwingl(nullptr),
  _navigationBar(nullptr),
  _viewBar(nullptr),
  _plotBar(nullptr),
  _posBar(nullptr)
{
  _verticalLayout = new QVBoxLayout(this);
  _verticalLayout->setContentsMargins(0,0,0,0);

  _viewBar = new ViewBar(this);
  _verticalLayout->addWidget(_viewBar);

  _plotBar = new PlotBar(this);
  _verticalLayout->addWidget(_plotBar);

  _posBar = new TabCanvasPos(this);
  _verticalLayout->addWidget(_posBar);

  _tabwingl = new TabWinGl(this);
  _tabwingl->setFocusPolicy(Qt::StrongFocus);
  _verticalLayout->addWidget(_tabwingl);

  _navigationBar = new TabNavTools(this);
  _verticalLayout->addWidget(_navigationBar);

  this->setLayout(_verticalLayout);
  this->setWindowTitle("QAgate");

  _navigationBar->makeConnexions(_tabwingl->currentGlwidget());
  _viewBar->makeConnexions(_tabwingl->currentGlwidget());
  _plotBar->makeConnexions(_tabwingl->currentGlwidget());
  _posBar->makeConnexions(_tabwingl->currentGlwidget());

  connect(_tabwingl,SIGNAL(changedTitle(std::string)),this,SLOT(changeTitle(std::string)));
  connect(_tabwingl,SIGNAL(activatedGlwidget(GLWidget*)),_navigationBar,SLOT(makeConnexions(GLWidget*)));
  connect(_tabwingl,SIGNAL(activatedGlwidget(GLWidget*)),_viewBar,SLOT(makeConnexions(GLWidget*)));
  connect(_tabwingl,SIGNAL(activatedGlwidget(GLWidget*)),_plotBar,SLOT(makeConnexions(GLWidget*)));
  connect(_tabwingl,SIGNAL(activatedGlwidget(GLWidget*)),_posBar,SLOT(makeConnexions(GLWidget*)));

  connect(_navigationBar,SIGNAL(openedFile(std::string)),this,SLOT(changeTitle(std::string)));
  connect(_navigationBar,SIGNAL(openedFile(std::string)),_tabwingl,SLOT(changeTitle(std::string)));

  connect(_viewBar,SIGNAL(changedPath(QString)),_navigationBar,SLOT(changePath(QString)));
  connect(_navigationBar,SIGNAL(changedPath(QString)),_viewBar,SLOT(changePath(QString)));
}

//
WinQt::~WinQt() {
  ;
}

//
void WinQt::setParameters(const std::string &filename) {
  _tabwingl->currentGlwidget()->setParameters(filename);
}

void WinQt::changeTitle(std::string name) {
  this->setWindowTitle(QString::fromStdString(name)+" - QAgate");
}

void WinQt::refresh() {
  _tabwingl->currentGlwidget()->emitCommandProcessed();
}

#endif 
