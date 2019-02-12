/**
 * @file include/atomselector.hpp
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


#ifndef ATOMSELECTOR_HPP
#define ATOMSELECTOR_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Weffc++"
#      endif
#    endif
#    pragma GCC system_header
#  endif
#include <QDialog>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QString>
#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic pop
#      endif
#    endif
#  endif

#include <string>
#include <vector>

/** 
 *
 */
class AtomSelector : public QDialog {
  Q_OBJECT

  private :

  protected :
    int _nToSelect;
    std::vector<int> _atoms;
    std::vector<std::pair<QLabel*,QComboBox*>> _form;
    QPushButton *_trackButton; 
    QPushButton *_cancelButton; 


  public :

    /**
     * Constructor.
     */
    AtomSelector(int nToSelect, int natom, QWidget *parent=0);

    /**
     * Destructor.
     */
    virtual ~AtomSelector();

    const std::vector<int>& atoms() const { return _atoms; }

  public slots :

    void save();
    void updateBoxes(QString text);

};

#endif  // ATOMSELECTOR_HPP
