/**
 * @file include/tabnavtools.hpp
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


#ifndef TABTOOLS_HPP
#define TABTOOLS_HPP

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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QSlider>
#include <QToolBar>
#  ifdef __GNUC__
#    if __GNUC__ >= 4
#      if __GNUC_MINOR__ >= 6
#        pragma GCC diagnostic pop
#      endif
#    endif
#  endif
#include "qtgui/glwidget.hpp"
#include "qtgui/timeline.hpp"

/** 
 *
 */
class TabNavTools : public QWidget {
  Q_OBJECT

  private :

  protected :
    TimeLine *_timeLine;
    QVBoxLayout *_verticalLayout;
    QToolBar *_actionBar;
    QAction *_prevButton;
    QAction *_nextButton;
    QAction *_playButton;
    QAction *_openButton;
    QAction *_appendButton;
    QAction *_fasterButton;
    QAction *_slowerButton;
    QAction *_recordButton;
    QAction *_snapshotButton;
    QAction *_repeatButton;
    QWidget *_hSpacer;
    int _nLoop;
    QString _currentPath;

  public :

    /**
     * Constructor.
     */
    TabNavTools(QWidget *parent=0);

    /**
     * Destructor.
     */
    virtual ~TabNavTools();

  public slots :
    
    void update(GLWidget *glwidget);
    void refreshButtons(GLWidget *glwidget);

    void togglePlay();
    void prevTime();
    void nextTime();
    void faster();
    void slower();
    void record();
    void snapshot();
    void toggleRepeat();
    void setTimeBegin(int time);
    void setTimeEnd(int time);
    void setTime(int time);
    void openFile();
    void appendFile();
    void makeConnexions(GLWidget *glwidget);
    void changePath(QString path);

  signals :

    void sentCommand(std::string comamnd);
    void openedFile(std::string name);
    void changedPath(QString);
};

#endif  // TABTOOLS_HPP
