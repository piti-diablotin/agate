/**
 * @file src/timeline.cpp
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


#include "qtgui/timeline.hpp"
#include <iostream>

//
TimeLine::TimeLine(QWidget *parent) : QWidget(parent),
  _mouseUsed(false),
  _timeBegin(0),
  _timeEnd(0),
  _time(0),
  _timeTotal(0),
  _horizontalLayout(nullptr),
  _lineBegin(nullptr),
  _lineEnd(nullptr),
  _splitter(nullptr),
  _slider(nullptr),
  _leftWidget(nullptr),
  _rightWidget(nullptr),
  _leftGridLayout(nullptr),
  _rightGridLayout(nullptr),
  _timeBeginLabel(nullptr),
  _timeEndLabel(nullptr)
{
  this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
  this->setFixedHeight(20);

  _horizontalLayout = new QHBoxLayout(this);
  _horizontalLayout->setSpacing(5);
  _horizontalLayout->setContentsMargins(5, 0, 5, 0);

  _timeBeginLabel = new QSpinBox(this);
  _timeBeginLabel->setValue(_timeBegin);
  _timeBeginLabel->setMinimum(0);
  _timeBeginLabel->setMaximum(_timeTotal);

  _horizontalLayout->addWidget(_timeBeginLabel);

  _splitter = new QSplitter(this);
  _splitter->setOrientation(Qt::Horizontal);

  _leftWidget = new QWidget(_splitter);
  _leftGridLayout = new QGridLayout(_leftWidget);
  _leftGridLayout->setSpacing(0);
  _leftGridLayout->setContentsMargins(0, 0, 0, 0);
  _lineBegin = new QFrame(_leftWidget);
  _lineBegin->setFrameShape(QFrame::HLine);
  _lineBegin->setFrameShadow(QFrame::Sunken);
  _leftGridLayout->addWidget(_lineBegin,0,0,1,1);
  _splitter->addWidget(_leftWidget);

  _slider = new QSlider(_splitter);
  _slider->setOrientation(Qt::Horizontal);
  _splitter->addWidget(_slider);

  _rightWidget = new QWidget(_splitter);
  _rightGridLayout = new QGridLayout(_rightWidget);
  _rightGridLayout->setSpacing(0);
  _rightGridLayout->setContentsMargins(0, 0, 0, 0);
  _lineBegin = new QFrame(_rightWidget);
  _lineBegin->setFrameShape(QFrame::HLine);
  _lineBegin->setFrameShadow(QFrame::Sunken);
  _rightGridLayout->addWidget(_lineBegin,0,0,1,1);
  _splitter->addWidget(_rightWidget);

  auto sizes = _splitter->sizes();
  sizes[1] += sizes[0]+ sizes[2];
  sizes[0] = 0;
  sizes[2] = 0;
  _splitter->setSizes(sizes);

  _horizontalLayout->addWidget(_splitter);

  //_timeEndLabel = new QLabel(QString::number(_timeEnd),this);
  _timeEndLabel = new QSpinBox(this);
  _timeEndLabel->setValue(_timeEnd);
  _timeEndLabel->setMinimum(_time);
  _timeEndLabel->setMaximum(_timeTotal);

  _horizontalLayout->addWidget(_timeEndLabel);

  this->setLayout(_horizontalLayout);

  connect(_slider,SIGNAL(valueChanged(int)),this,SIGNAL(timeChanged(int)));
  connect(_splitter,SIGNAL(splitterMoved(int,int)),this,SLOT(extractTimes(int,int)));
  connect(_timeBeginLabel,SIGNAL(valueChanged(int)),this,SLOT(setTimeBegin(int)));
  connect(_timeEndLabel,SIGNAL(valueChanged(int)),this,SLOT(setTimeEnd(int)));

}

//
TimeLine::~TimeLine() {
  ;
}

void TimeLine::mousePressEvent( QMouseEvent *mouseEvent ) {
  (void) mouseEvent;
  _mouseUsed = true;
}

void TimeLine::mouseReleaseEvent( QMouseEvent *mouseEvent ) {
  (void) mouseEvent;
  _mouseUsed = false;
}

void TimeLine::setTimeBegin(int time) {
  auto sizes = _splitter->sizes();
  int total = sizes[0]+sizes[1]+sizes[2]; 
  if ( time >= _timeEnd ) return;
  sizes[0] = total*time/_timeTotal;
  sizes[1] = total-sizes[0]-sizes[2];
  _timeBegin = time;
  _slider->setMinimum(time);
  _timeBeginLabel->disconnect();
  _timeBeginLabel->setValue(_timeBegin);
  connect(_timeBeginLabel,SIGNAL(valueChanged(int)),this,SLOT(setTimeBegin(int)));
  emit(timeBeginChanged(_timeBegin));
}

void TimeLine::setTimeEnd(int time) {
  auto sizes = _splitter->sizes();
  int total = sizes[0]+sizes[1]+sizes[2]; 
  if ( time < _timeBegin && time >=_timeTotal ) return;
  _timeEnd = time;
  sizes[2] = total*(_timeTotal-1-_timeEnd)/_timeTotal;
  sizes[1] = total-sizes[0]-sizes[2];
  _slider->setMaximum(time);
  _timeEndLabel->disconnect();
  _timeEndLabel->setValue(_timeEnd);
  connect(_timeEndLabel,SIGNAL(valueChanged(int)),this,SLOT(setTimeEnd(int)));
  emit(timeEndChanged(_timeEnd));
}

void TimeLine::setTime(int time) {
  if ( time <= _timeEnd && time >= _timeBegin )
    _time = time;
  disconnect(_slider,SIGNAL(valueChanged(int)),this,SIGNAL(timeChanged(int)));
  _slider->setValue(time);
  connect(_slider,SIGNAL(valueChanged(int)),this,SIGNAL(timeChanged(int)));
}

void TimeLine::setTimeTotal(int time) {
  auto sizes = _splitter->sizes();
  int total = sizes[0]+ sizes[1] + sizes[2];
  if ( time <= _timeEnd ) {
    _timeEnd = time-1;
    _slider->setMaximum(_timeEnd);
    _timeEndLabel->disconnect();
    _timeEndLabel->setValue(_timeEnd);
    connect(_timeEndLabel,SIGNAL(valueChanged(int)),this,SLOT(setTimeEnd(int)));
  }
  if ( time < _timeBegin ) {
    _timeBegin = time-1;
    _timeBeginLabel->disconnect();
    _timeBeginLabel->setValue(_timeBegin);
    connect(_timeBeginLabel,SIGNAL(valueChanged(int)),this,SLOT(setTimeBegin(int)));
  }
  if ( _time > time ) {
    _time = time-1;
    disconnect(_slider,SIGNAL(valueChanged(int)),this,SIGNAL(timeChanged(int)));
    _slider->setValue(time);
    connect(_slider,SIGNAL(valueChanged(int)),this,SIGNAL(timeChanged(int)));
  }
  _timeTotal = time;
  _timeEndLabel->setRange(0,_timeTotal-1);
  _timeBeginLabel->setRange(0,_timeTotal-1);
  _slider->setMinimum(_timeBegin);
  sizes[0] = total*_timeBegin/_timeTotal;
  sizes[2] = total * (_timeTotal-1 - _timeEnd )/_timeTotal;
  sizes[1] = total-sizes[0]-sizes[2];
  _splitter->setSizes(sizes);
}

void TimeLine::setTimes(int begin, int end, int time, int total) {
  if ( !(begin <= time && time <= end && end < total) ) {
    if ( end >= total ) end = total-1;
    if ( time > end ) time = end;
    if ( begin > end ) begin = end;
    if ( time < begin ) time = begin;
    if ( begin < 0 ) begin = 0;
  }
  _timeTotal = total;
  _time = time;
  _timeBegin = begin;
  _timeEnd = end;
  auto sizes = _splitter->sizes();
  int totalpx = sizes[0]+ sizes[1] + sizes[2];
  sizes[0] = (_timeBegin * totalpx )/ _timeTotal;
  sizes[2] = ((_timeTotal-1 - _timeEnd ) * totalpx )/_timeTotal;
  sizes[1] = totalpx-sizes[0]-sizes[2];
  _splitter->setSizes(sizes);
  disconnect(_slider,SIGNAL(valueChanged(int)),this,SIGNAL(timeChanged(int)));
  _slider->setMinimum(_timeBegin);
  _slider->setMaximum(_timeEnd);
  _slider->setValue(_time);
  connect(_slider,SIGNAL(valueChanged(int)),this,SIGNAL(timeChanged(int)));
  _timeBeginLabel->disconnect();
  _timeEndLabel->disconnect();
  _timeEndLabel->setRange(0,_timeTotal-1);
  _timeBeginLabel->setRange(0,_timeTotal-1);
  _timeBeginLabel->setValue(_timeBegin);
  _timeEndLabel->setValue(_timeEnd);
  connect(_timeBeginLabel,SIGNAL(valueChanged(int)),this,SLOT(setTimeBegin(int)));
  connect(_timeEndLabel,SIGNAL(valueChanged(int)),this,SLOT(setTimeEnd(int)));
}

void TimeLine::extractTimes(int pos, int index) {
  (void) pos;
  auto sizes = _splitter->sizes();
  int total = sizes[0]+ sizes[1] + sizes[2];
  int time;
  switch (index) {
    case 1: // tbegin changed
      time = (sizes[0] * _timeTotal)/total;
      _slider->setMinimum(time);
      _timeBeginLabel->disconnect();
      _timeBeginLabel->setValue(time);
      connect(_timeBeginLabel,SIGNAL(valueChanged(int)),this,SLOT(setTimeBegin(int)));
      _timeBegin = time;
      emit(timeBeginChanged(time));
      break;
    case 2: // tend changed
      time = _timeTotal -(sizes[2] * _timeTotal)/total-1;
      _slider->setMaximum(time);
      _timeEndLabel->disconnect();
      _timeEndLabel->setValue(time);
      connect(_timeEndLabel,SIGNAL(valueChanged(int)),this,SLOT(setTimeEnd(int)));
      _timeEnd = time;
      emit(timeEndChanged(time));
      break;
    default :
      break;
  }
}

void TimeLine::readTime(int value) {
  emit(timeChanged(value));
}
