/**
 * @file include/tabcanvaspos.hpp
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


#ifndef TABCANVASPOS_HPP
#define TABCANVASPOS_HPP

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
#include <QWidget>
#include <QToolBar>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic pop
#      endif
#    endif
#  endif

#include "qtgui/glwidget.hpp"

/** 
 *
 */
class TabCanvasPos : public QWidget {
  Q_OBJECT

  private :

  protected :
    int        _natom;
    QToolBar  *_posBar;
    QComboBox *_display;
    QCheckBox *_buttonBorder;
    QCheckBox *_buttonBond;
    QCheckBox *_buttonPeriodic;
    std::string _displayed;
    QAction   *_angle;
    QAction   *_distance;
    QAction   *_centroid;

  public :

    /**
     * Constructor.
     */
    TabCanvasPos(QWidget *parent=0);

    /**
     * Destructor.
     */
    virtual ~TabCanvasPos();

  public slots :

    void makeConnexions(GLWidget *glwidget);
    void refreshButtons(GLWidget *glwidget);

    void displayBorder(int);
    void displayBond(int);
    void periodic(int);

    void changeDisplay(int);
    void centroid();

    void angle(void);
    void distance(void);

  signals :
    sentCommand(std::string);
};

#endif  // TABCANVASPOS_HPP
