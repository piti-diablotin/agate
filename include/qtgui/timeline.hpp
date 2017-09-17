/**
 * @file include/timeline.hpp
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


#ifndef TIMELINE_HPP
#define TIMELINE_HPP

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
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QFrame>
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
class TimeLine : public QWidget {
  Q_OBJECT

  private :
    bool _mouseUsed;
    int _timeBegin;
    int _timeEnd;
    int _time;
    int _timeTotal;
    QHBoxLayout *_horizontalLayout;
    QFrame *_lineBegin;
    QFrame *_lineEnd;
    QSplitter *_splitter;
    QSlider *_slider;
    QWidget *_leftWidget;
    QWidget *_rightWidget;
    QGridLayout *_leftGridLayout;
    QGridLayout *_rightGridLayout;
    QSpinBox *_timeBeginLabel;
    QSpinBox *_timeEndLabel;

  protected :

  public :

    /**
     * Constructor.
     */
    TimeLine(QWidget *parent=0);

    /**
     * Destructor.
     */
    virtual ~TimeLine();

    virtual void mousePressEvent( QMouseEvent *mouseEvent );
    virtual void mouseReleaseEvent( QMouseEvent *mouseEvent );

    inline int timeEnd() const { return _timeEnd; }
    inline int timeBegin() const { return _timeBegin; }
    inline int time() const { return _time; }
    
  public slots :
    void setTimeBegin(int time);
    void setTimeEnd(int time);
    void setTime(int time);
    void setTimeTotal(int time);
    void setTimes(int begin, int end, int time, int total);

    //void enableSlider() {_mouseUsed = true;}
    //void disableSlider() {_mouseUsed = false;}
    void extractTimes(int pos, int index);
    void readTime(int);

  signals:
    void timeBeginChanged(int);
    void timeEndChanged(int);
    void timeChanged(int);
};

#endif  // TIMELINE_HPP
