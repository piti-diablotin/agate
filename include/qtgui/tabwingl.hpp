/**
 * @file include/tabwingl.hpp
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


#ifndef TABWINGL_HPP
#define TABWINGL_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#ifdef HAVE_QT

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
#include <QVBoxLayout>
#include <QTabWidget>
#include <QGridLayout>
#include <QToolButton>
#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic pop
#      endif
#    endif
#  endif

/** 
 *
 */
class TabWinGl : public QTabWidget {

  Q_OBJECT

  private :

    QWidget* newTab();


  protected :

    std::vector<GLWidget*> _glwidgets;
    GLWidget *_currentGlwidget;
    QToolButton *_addButton;


  public :

    /**
     * Constructor.
     */
    TabWinGl(QWidget *parent = 0);

    /**
     * Destructor.
     */
    virtual ~TabWinGl();

    GLWidget* currentGlwidget() { return _currentGlwidget; }

  signals :
    void changedTitle(std::string);
    void activatedGlwidget(GLWidget*);


  protected slots :
    
    void selectTab(int index = -1);

  public slots :
    void changeTitle(std::string name);
    void closeCurrentTab();
    void closeTab(int index);

};

#endif  // TABWINGL_HPP

#endif // HAVE_QT
