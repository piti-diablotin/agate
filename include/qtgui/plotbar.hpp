/**
 * @file include/plotbar.hpp
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


#ifndef PLOTBAR_HPP
#define PLOTBAR_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
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
#include <QAction>
#include <QToolBar>
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
class PlotBar : public QWidget {
  Q_OBJECT

  private :

  protected :
    int _natom;
    QToolBar *_plotBar;
    QComboBox *_choice;
    std::string _action;
    QAction *_temperature;
    QAction *_pression;
    QAction *_pdf;
    QAction *_volume;
    QAction *_acell;
    QAction *_angle;
    QAction *_angleAtoms;
    QAction *_distance;
    QAction *_stress;
    QAction *_ekin;
    QAction *_etotal;
    QAction *_entropy;
    QAction *_positions;
    QAction *_vacf;
    QAction *_pdos;
    QAction *_msd;
    QAction *_pacf;

  public :

    /**
     * Constructor.
     */
    PlotBar(QWidget *parent=0);

    /**
     * Destructor.
     */
    virtual ~PlotBar();

  public slots :

    void makeConnexions(GLWidget *glwidget);
    void refreshButtons(GLWidget *glwidget);

    void plotTemperature();
    void plotPression();
    void plotPdf();
    void plotVolume();
    void plotAcell();
    void plotAngle();
    void plotAngleAtoms();
    void plotDistance();
    void plotStress();
    void plotEkin();
    void plotEtotal();
    void plotEntropy();
    void plotPositions();
    void plotVacf();
    void plotPdos();
    void plotMsd();
    void plotPacf();

    void changeAction(int);

  signals :
    sentCommand(std::string);
};

#endif  // PLOTBAR_HPP
