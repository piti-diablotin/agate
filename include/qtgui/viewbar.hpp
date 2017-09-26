/**
 * @file include/viewbar.hpp
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


#ifndef VIEWBAR_HPP
#define VIEWBAR_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "qtgui/glwidget.hpp"
#include <vector>

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
#include <QToolButton>
#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic pop
#      endif
#    endif
#  endif
#include <string>

/** 
 *
 */
class ViewBar : public QWidget {
  Q_OBJECT

  private :

  protected :
    QToolBar *_viewBar;
    QAction *_axis;
    QAction *_antialiasing;
    QAction *_light;
    QAction *_fill;
    QAction *_projection;
    QAction *_xplus;
    QAction *_yplus;
    QAction *_zplus;
    QAction *_xminus;
    QAction *_yminus;
    QAction *_zminus;
    QAction *_bg;
    QAction *_fg;
    QAction *_refresh;
    QAction *_dump;
    QAction *_dumphist;
    QAction *_dumpxyz;
    QAction *_writeDtset;
    QAction *_writePoscar;
    QAction *_writeCif;
    QAction *_write;
    QString _currentPath;

  public :

    /**
     * Constructor.
     */
    ViewBar(QWidget *parent=0);

    /**
     * Destructor.
     */
    virtual ~ViewBar();

  public slots :

    void makeConnexions(GLWidget *glwidget);
    void refreshButtons(GLWidget *glwidget);

    void toggleAxis();
    void toggleAA();
    void toggleLight();
    void toggleFill();
    void toggleProjection();
    void plusX();
    void plusY();
    void plusZ();
    void minusX();
    void minusY();
    void minusZ();
    void fgColor();
    void bgColor();
    void refresh();
    void writeDtset();
    void writePoscar();
    void writeCif();
    void dump();
    void dumpxyz();
    void dumphist();
    void changePath(QString path);

  signals :
    sentCommand(std::string);
    changedPath(QString);
};

#endif  // VIEWBAR_HPP
