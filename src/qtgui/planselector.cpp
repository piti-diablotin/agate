/**
 * @file src/planselector.cpp
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


#include "qtgui/planselector.hpp"

//
PlanSelector::PlanSelector(QWidget *parent) :
  QDialog(parent),
  _result("xy"),
  _selectButton(nullptr),
  _cancelButton(nullptr)
{
  QVBoxLayout *vlayout = new QVBoxLayout(this);
  QLabel *question = new QLabel("Select the plan for the projection",this);
  vlayout->addWidget(question);

  QFrame *frame = new QFrame(this);
  QFormLayout *flayout = new QFormLayout(frame);
  QLabel *label = new QLabel("Plan ",frame);
  QComboBox *select = new QComboBox(frame);
  select->addItem("xy");
  select->addItem("xz");
  select->addItem("yx");
  select->addItem("yz");
  select->addItem("zx");
  select->addItem("zy");
  flayout->setWidget(0,QFormLayout::LabelRole,label);
  flayout->setWidget(0,QFormLayout::FieldRole,select);
  frame->setLayout(flayout);
  vlayout->addWidget(frame);

  QWidget *buttons = new QWidget(this);

  QHBoxLayout *hlayout = new QHBoxLayout(buttons);
  _selectButton = new QPushButton("Select",buttons);
  _cancelButton = new QPushButton("Cancel",buttons);
  hlayout->addWidget(_selectButton);
  hlayout->addWidget(_cancelButton);
  buttons->setLayout(hlayout);
  vlayout->addWidget(buttons);

  this->setLayout(vlayout);

  connect(_selectButton,SIGNAL(clicked()),this,SLOT(accept()));
  connect(_cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
  connect(select,SIGNAL(currentIndexChanged(QString)),this,SLOT(updateResult(QString)));
  ;
}

//
PlanSelector::~PlanSelector() {
  ;
}

void PlanSelector::updateResult(QString text) {
  _result = text.toStdString();
}
