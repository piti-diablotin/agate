/**
 * @file src/atomselector.cpp
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


#include "qtgui/atomselector.hpp"

//
AtomSelector::AtomSelector(int nToSelect, int natom, QWidget *parent) : 
  QDialog(parent),
  _nToSelect(nToSelect),
  _atoms(nToSelect,0),
  _form(),
  _trackButton(nullptr),
  _cancelButton(nullptr)
{
  QVBoxLayout *vlayout = new QVBoxLayout(this);
  QLabel *question = new QLabel("Select the atoms you want to track",this);
  vlayout->addWidget(question);

  QFrame *frame = new QFrame(this);
  QFormLayout *flayout = new QFormLayout(frame);
  for ( int i = 0 ; i < _nToSelect ; ++i ) {
    _form.push_back(
        std::make_pair(
          new QLabel("Atom "+QString::number(i+1),frame),
          new QComboBox(frame)
          )
        );
    auto box = _form.back().second;
    for ( int n = 0 ; n < natom ; ++n ) {
      //if ( n != i )
        box->addItem(QString::number(n+1));
    }
    connect(box,SIGNAL(currentIndexChanged(QString)),this,SLOT(updateBoxes(QString)));
    flayout->setWidget(i,QFormLayout::LabelRole,_form.back().first);
    flayout->setWidget(i,QFormLayout::FieldRole,_form.back().second);
  }
  frame->setLayout(flayout);
  vlayout->addWidget(frame);


  QWidget *buttons = new QWidget(this);

  QHBoxLayout *hlayout = new QHBoxLayout(buttons);
  _trackButton = new QPushButton("Select",buttons);
  _cancelButton = new QPushButton("Cancel",buttons);
  hlayout->addWidget(_trackButton);
  hlayout->addWidget(_cancelButton);
  buttons->setLayout(hlayout);
  vlayout->addWidget(buttons);

  this->setLayout(vlayout);

  connect(_trackButton,SIGNAL(clicked()),this,SLOT(save()));
  connect(_cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
}

//
AtomSelector::~AtomSelector() {
  ;
}

void AtomSelector::save() {
  for ( int i = 0 ; i < _nToSelect ; ++i ) {
    _atoms[i] = _form[i].second->currentText().toInt();
  }
  this->done(QDialog::Accepted);
}

void AtomSelector::updateBoxes(QString text){
  (void) text;
  /*
  std::vector<int> selected(_nToSelect);
  for ( int i = 0 ; i < _nToSelect ; ++i ) {
    selected[i] = _form[i].second->currentText() 
  }

  for ( int i = 0 ; i < _nToSelect ; ++i ) {
    auto box = _form[i].second;
    box->clear();

  }
  */

}
