/**
 * @file include/winqt.hpp
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


#ifndef WINQT_HPP
#define WINQT_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#ifdef HAVE_QT

#include "qtgui/glwidget.hpp"
#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Weffc++"
#      endif
#    endif
#    pragma GCC system_header
#  endif
#include <QVBoxLayout>
#include <QTabWidget>
#include <QGridLayout>
#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic pop
#      endif
#    endif
#  endif
#include "qtgui/tabnavtools.hpp"
#include "qtgui/tabwingl.hpp"
#include "qtgui/viewbar.hpp"
#include "qtgui/plotbar.hpp"
#include "qtgui/tabcanvaspos.hpp"


/** 
 *
 */
class WinQt : public QWidget {
  Q_OBJECT

  private :

  protected :
    QVBoxLayout *_verticalLayout;
    TabWinGl *_tabwingl;
    TabNavTools *_navigationBar;
    ViewBar *_viewBar;
    PlotBar *_plotBar;
    TabCanvasPos *_posBar;

  public :

    /**
     * Constructor.
     */
    explicit WinQt(QWidget *parent = 0);

    /**
     * Destructor.
     */
    virtual ~WinQt();

    /**
     * Read some configuration parameters from config file if wanted.
     * @param filename The file containing the parameters
     */
    virtual void setParameters(const std::string &filename);

    /**
     * Set the font to use for displaying information
     * @param font The new font
     */
    inline void setFont(const std::string& font)
    { _tabwingl->currentGlwidget()->Window::setFont(font); }

    GLWidget* current() {return _tabwingl->currentGlwidget();}

    void keyPressEvent( QKeyEvent *keyEvent ) { _tabwingl->currentGlwidget()->keyPressEvent(keyEvent); }

    public slots:
      void changeTitle(std::string name);
      void refresh();

};

#endif
#endif  // WINQT_HPP
