/**
 * @file src/tabwingl.cpp
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


#include "qtgui/tabwingl.hpp"
#include "canvas/canvaspos.hpp"
#include "qtgui/qplot.hpp"

//
TabWinGl::TabWinGl(QWidget *parent) : QTabWidget(parent),
  _glwidgets(),
  _currentGlwidget(nullptr),
  _addButton(nullptr)
{

  this->addTab(this->newTab(),"New Tab");
  _currentGlwidget = _glwidgets.back();

  _addButton = new QToolButton(this);
  QAction *action = new QAction(QIcon(":/newtab.png"),"Add new tab",_addButton);
  action->setShortcut(QKeySequence::AddTab);
  _addButton->setDefaultAction(action);
  this->addTab(new QLabel("Add tabs by pressing \"+\""), QString());
  this->setTabEnabled(1, false);

  // Add tab button to current tab. Button will be enabled, but tab -- not
  this->tabBar()->setTabButton(1, QTabBar::RightSide, _addButton);

  // Setting tabs closable and movable
  this->setTabsClosable(true);
  connect(_addButton,SIGNAL(clicked()),this,SLOT(selectTab()));
  connect(this,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int)));

  connect(this,SIGNAL(currentChanged(int)),this,SLOT(selectTab(int)));
  connect(_currentGlwidget,SIGNAL(changedTitle(std::string)),this,SLOT(changeTitle(std::string)));
  connect(_currentGlwidget,SIGNAL(closed()),this,SLOT(closeCurrentTab()));
}

//
TabWinGl::~TabWinGl() {
  ;
  for ( auto widg : _glwidgets )
    delete widg;
}

QWidget* TabWinGl::newTab() {
  QWidget *tab = new QWidget(this);
  QGridLayout *gridLayout = new QGridLayout(tab);
  gridLayout->setContentsMargins(0,0,0,0);

  pCanvas canvas(new CanvasPos(true));
  canvas->setGraph(new QPlot(this));
  _glwidgets.push_back(new GLWidget(canvas,1280,961,60,tab));
  //canvas->reset(new CanvasPos(true)); 
  _glwidgets.back()->setTitle("New Tab");
  gridLayout->addWidget(_glwidgets.back(), 0, 0, 1, 1);
  return tab;
}

void TabWinGl::selectTab(int index) {
  int ntab = this->count();
  if ( _currentGlwidget != nullptr ) {
    _currentGlwidget->stop();
    _currentGlwidget->disconnect();
  }

  if ( index == ntab-1  || index == -1) {
    index = ntab-1;
    QWidget *tab = this->newTab();
    _currentGlwidget = _glwidgets.back();
    this->insertTab(index,tab, QString("New Tab"));
    disconnect(this,SIGNAL(currentChanged(int)),this,SLOT(selectTab(int)));
    this->setCurrentIndex(index);
    connect(this,SIGNAL(currentChanged(int)),this,SLOT(selectTab(int)));
  }
  else {
    // Select new tab;
    _currentGlwidget = _glwidgets[index];
  }
  connect(_currentGlwidget,SIGNAL(changedTitle(std::string)),this,SLOT(changeTitle(std::string)));
  connect(_currentGlwidget,SIGNAL(closed()),this,SLOT(closeCurrentTab()));
  _currentGlwidget->start();
  QString text = this->tabText(index);
  emit(activatedGlwidget(_currentGlwidget));
  emit(changedTitle(text.toStdString()));
}

void TabWinGl::changeTitle(std::string name) {
  int index = this->currentIndex();
  this->setTabText(index,QString::fromStdString(name));
  emit(changedTitle(name));
}

void TabWinGl::closeCurrentTab() {
  int index = this->currentIndex();

  disconnect(this,SIGNAL(currentChanged(int)),this,SLOT(selectTab(int)));
  this->removeTab(index);
  connect(this,SIGNAL(currentChanged(int)),this,SLOT(selectTab(int)));

  _glwidgets.erase(_glwidgets.begin()+index);

  _currentGlwidget->disconnect();
  _currentGlwidget = nullptr;
  if ( _glwidgets.size() == 0 ) {this->window()->close();return;}
  if ( _glwidgets.size() <= (unsigned) index ) {
    index = _glwidgets.size()-1;
  }
  this->setCurrentIndex(index);
  this->selectTab(index);
}

void TabWinGl::closeTab(int index) {
  bool closeMe = (this->currentIndex() == index);
  if ( index < 0 || index >= (int)_glwidgets.size() ) return;

  disconnect(this,SIGNAL(currentChanged(int)),this,SLOT(selectTab(int)));
  this->removeTab(index);
  connect(this,SIGNAL(currentChanged(int)),this,SLOT(selectTab(int)));

  auto toDelete = _glwidgets[index];
  _glwidgets.erase(_glwidgets.begin()+index);

  if ( closeMe ) {
    if ( _glwidgets.size() == 0 ) {this->window()->close();return;}
    if ( _glwidgets.size() <= (unsigned) index ) {
      index = _glwidgets.size()-1;
    }
    this->setCurrentIndex(index);
    this->selectTab(index);
  }
  delete toDelete;
}
