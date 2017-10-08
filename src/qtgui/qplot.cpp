/**
 * @file src/./qplot.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2017 Jordan Bieder
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


#include "qtgui/qplot.hpp"
#include "base/exception.hpp"

const QColor QPlot::qcolor[] = { Qt::black, Qt::red, Qt::green, Qt::blue, Qt::magenta, Qt::cyan, Qt::darkRed, Qt::darkGreen, Qt::darkYellow };

//
QPlot::QPlot(QWidget *parent) : QDialog(parent), Graph(),
  _plot(new QCustomPlot(this)),
  _titleElement(new QCPTextElement(_plot, QString::fromStdString(_title), QFont("Helvetica", 12, QFont::Bold)))
{
  _plot->resize(640,480);
  this->setWindowTitle(QString::fromStdString("Agate Plot"));
  _plot->setAutoAddPlottableToLegend(false);
  _plot->plotLayout()->insertRow(0);
  _plot->plotLayout()->addElement(0, 0, _titleElement);
  connect(_plot->xAxis, SIGNAL(rangeChanged(QCPRange)), _plot->xAxis2, SLOT(setRange(QCPRange)));
  connect(_plot->yAxis, SIGNAL(rangeChanged(QCPRange)), _plot->yAxis2, SLOT(setRange(QCPRange)));
}

//
QPlot::~QPlot() {
  ;
}

void QPlot::plot(std::vector<double> x, std::list<std::vector<double>> y, std::list<std::string> labels, std::vector<short> colors){
  this->clean();
  this->show();
  auto label = labels.begin();
  auto yp = y.begin();
  int autocolor = 0;
  for ( unsigned p = 0 ; p < y.size() ; ++p) {
    _plot->addGraph();
    auto graph = _plot->graph(p);
    if ( p < colors.size() ) {
      graph->setPen(QPen(qcolor[colors[p] < 8 ? colors[p] : autocolor++]));
    }
    else {
      graph->setPen(QPen(qcolor[autocolor++]));
    }
    if ( autocolor >= 8 ) autocolor = 0;


    if ( p < labels.size() && !label->empty() ) {
      graph->setName(QString::fromStdString(*label));
      graph->addToLegend();
      ++label;
    }
    graph->setData(QVector<double>::fromStdVector(x),QVector<double>::fromStdVector(*yp));
    ++yp;
  }
  _plot->rescaleAxes(true);
  _plot->xAxis2->setVisible(true);
  _plot->xAxis2->setTickLabels(false);
  _plot->yAxis2->setVisible(true);
  _plot->yAxis2->setTickLabels(false);
  _plot->xAxis->setLabel(QString::fromStdString(_xlabel));
  _plot->yAxis->setLabel(QString::fromStdString(_ylabel));
  _plot->legend->setVisible(labels.size() > 0);
  _titleElement->setText(QString::fromStdString(_title));
  _plot->replot();
}

void QPlot::plot(std::list< std::pair< std::vector<double>,std::vector<double> > > xy, std::list<std::string> labels, std::vector<short> colors) {
  this->clean();
  this->show();
  auto label = labels.begin();
  auto xyp = xy.begin();
  int autocolor = 0;
  for ( unsigned p = 0 ; p < xy.size() ; ++p) {
    _plot->addGraph();
    auto graph = _plot->graph(p);
    if ( p < colors.size() ) {
      graph->setPen(QPen(qcolor[colors[p] < 8 ? colors[p] : autocolor++]));
    }
    else {
      graph->setPen(QPen(qcolor[autocolor++]));
    }
    if ( autocolor >= 8 ) autocolor = 0;

    if ( p < labels.size() && !label->empty() ) {
      graph->setName(QString::fromStdString(*label));
      graph->addToLegend();
      ++label;
    }
    graph->setData(QVector<double>::fromStdVector(xyp->first),QVector<double>::fromStdVector(xyp->second));
    ++xyp;
  }
  _plot->rescaleAxes(true);
  _plot->xAxis2->setVisible(true);
  _plot->xAxis2->setTickLabels(false);
  _plot->yAxis2->setVisible(true);
  _plot->yAxis2->setTickLabels(false);
  _plot->xAxis->setLabel(QString::fromStdString(_xlabel));
  _plot->yAxis->setLabel(QString::fromStdString(_ylabel));
  _plot->legend->setVisible(labels.size()>0);
  _titleElement->setText(QString::fromStdString(_title));
  _plot->replot();
}

void QPlot::save(std::string filename){
  _plot->savePdf(QString::fromStdString(filename+".pdf"),0,0,QCP::epNoCosmetic);
}

void QPlot::clean(){
  _plot->clearGraphs();
}

void QPlot::custom(const std::string &customlines){
 (void) customlines;
}

void QPlot::dump(std::ostream& out, std::string& plotname) const {
  (void) out;
  (void) plotname;
}

void QPlot::resizeEvent(QResizeEvent * event) {
  QDialog::resizeEvent(event);
  _plot->resize(event->size());
  _plot->replot();
}
