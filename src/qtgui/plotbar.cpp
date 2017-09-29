/**
 * @file src/plotbar.cpp
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


#include "qtgui/plotbar.hpp"
#include "qtgui/atomselector.hpp"
#include "qtgui/planselector.hpp"

//
PlotBar::PlotBar(QWidget *parent) : QWidget(parent),
  _natom(0),
  _plotBar(nullptr),
  _choice(nullptr),
  _action(":plot"),
  _temperature(nullptr),
  _pression(nullptr),
  _pdf(nullptr),
  _volume(nullptr),
  _acell(nullptr),
  _angle(nullptr),
  _angleAtoms(nullptr),
  _distance(nullptr),
  _stress(nullptr),
  _ekin(nullptr),
  _etotal(nullptr),
  _entropy(nullptr),
  _positions(nullptr),
  _vacf(nullptr),
  _pdos(nullptr),
  _msd(nullptr),
  _pacf(nullptr),
  _tdep(nullptr),
  _gyration(nullptr)
{
  _plotBar = new QToolBar(this);
  //_plotBar->setFixedHeight(40);
  //
  _choice = new QComboBox(this);
  _choice->addItem("Plot");
  _choice->addItem("Print");
  _choice->addItem("Data");

  _plotBar->addWidget(_choice);

  _temperature = new QAction("T",_plotBar); _temperature->setToolTip("Temperature");
  _pression = new QAction("P",_plotBar); _pression->setToolTip("Pressure");
  _pdf = new QAction("PDF",_plotBar); _pdf->setToolTip("Pair Distribution Function");
  _volume = new QAction("V",_plotBar); _volume->setToolTip("Volume");
  _acell = new QAction("a b c",_plotBar); _acell->setToolTip("Lattice parameters");
  QString acell(3);
  acell[0]=QChar(0xb1,0x03);
  acell[1]=QChar(0xb2,0x03);
  acell[2]=QChar(0xb3,0x03);
  _angle = new QAction(acell,_plotBar); _angle->setToolTip("Primitive vector angles");
  QString angle="t atoms";
  angle[0]=QChar(0xb8,0x03);
  _angleAtoms = new QAction(angle,_plotBar); _angleAtoms->setToolTip("Angle between 3 atoms");
  _distance = new QAction("Distance",_plotBar); _distance->setToolTip("Distance between 2 atoms");
  _stress = new QAction(QChar(0xc3,0x03),_plotBar); _stress->setToolTip("Stress tensor");
  _ekin = new QAction("K",_plotBar); _ekin->setToolTip("Kinetic energy");
  _etotal = new QAction("E",_plotBar); _etotal->setToolTip("Total electronic energy");
  _entropy = new QAction("S",_plotBar); _entropy->setToolTip("Electronic entropy");
  _positions = new QAction("Positions",_plotBar); _positions->setToolTip("Atomic positions");
  _vacf = new QAction("VACF",_plotBar); _vacf->setToolTip("Velocity autocorrelation function");
  _pdos = new QAction("PDOS",_plotBar); _vacf->setToolTip("Phonon Density of States");
  _msd = new QAction("MSD",_plotBar); _vacf->setToolTip("Mean Square Displacements");
  _pacf = new QAction("PACF",_plotBar); _vacf->setToolTip("Position autocorrelation function");
  _tdep = new QAction("TDEP",_plotBar); _vacf->setToolTip("Phonons");
  _gyration = new QAction("Gyration",_plotBar); _vacf->setToolTip("Gyration tensor");

  _plotBar->addAction(_acell);
  _plotBar->addAction(_angle);
  _plotBar->addAction(_volume);
  _plotBar->addAction(_angleAtoms);
  _plotBar->addAction(_distance);
  _plotBar->addAction(_positions);
  _plotBar->addAction(_gyration);
  _plotBar->addAction(_pdf);
  _plotBar->addAction(_msd);
  _plotBar->addAction(_pacf);

  _plotBar->addAction(_temperature);
  _plotBar->addAction(_pression);
  _plotBar->addAction(_stress);
  _plotBar->addAction(_ekin);
  _plotBar->addAction(_etotal);
  _plotBar->addAction(_entropy);
  _plotBar->addAction(_vacf);
  _plotBar->addAction(_pdos);
  _plotBar->addAction(_tdep);

  connect(_choice,SIGNAL(currentIndexChanged(int)),this,SLOT(changeAction(int)));
  connect(_temperature,SIGNAL(triggered()),this,SLOT(plotTemperature()));
  connect(_pression,SIGNAL(triggered()),this,SLOT(plotPression()));
  connect(_pdf,SIGNAL(triggered()),this,SLOT(plotPdf()));
  connect(_volume,SIGNAL(triggered()),this,SLOT(plotVolume()));
  connect(_acell,SIGNAL(triggered()),this,SLOT(plotAcell()));
  connect(_angle,SIGNAL(triggered()),this,SLOT(plotAngle()));
  connect(_angleAtoms,SIGNAL(triggered()),this,SLOT(plotAngleAtoms()));
  connect(_distance,SIGNAL(triggered()),this,SLOT(plotDistance()));
  connect(_stress,SIGNAL(triggered()),this,SLOT(plotStress()));
  connect(_ekin,SIGNAL(triggered()),this,SLOT(plotEkin()));
  connect(_etotal,SIGNAL(triggered()),this,SLOT(plotEtotal()));
  connect(_entropy,SIGNAL(triggered()),this,SLOT(plotEntropy()));
  connect(_positions,SIGNAL(triggered()),this,SLOT(plotPositions()));
  connect(_gyration,SIGNAL(triggered()),this,SLOT(plotGyration()));
  connect(_vacf,SIGNAL(triggered()),this,SLOT(plotVacf()));
  connect(_pdos,SIGNAL(triggered()),this,SLOT(plotPdos()));
  connect(_msd,SIGNAL(triggered()),this,SLOT(plotMsd()));
  connect(_pacf,SIGNAL(triggered()),this,SLOT(plotPacf()));
  connect(_tdep,SIGNAL(triggered()),this,SLOT(plotTdep()));

  QWidget *hSpacer = new QWidget(this);
  hSpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  layout->addWidget(_plotBar);
  layout->addWidget(hSpacer);
  this->setLayout(layout);
}

//
PlotBar::~PlotBar() {
  ;
}

void PlotBar::makeConnexions(GLWidget *glwidget) {
  if ( glwidget != nullptr ) {
    connect(glwidget,SIGNAL(commandProcessed(GLWidget*)),this,SLOT(refreshButtons(GLWidget*)));
    this->disconnect(SIGNAL(sentCommand(std::string)));
    connect(this,SIGNAL(sentCommand(std::string)),glwidget,SLOT(processCommand(std::string)));
    this->refreshButtons(glwidget);
  }
}

void PlotBar::refreshButtons(GLWidget *glwidget) {
  auto hist = glwidget->getCanvas()->histdata();
  //this->setEnabled(hist!=nullptr);
  int natomImg = 0;
  if ( hist != nullptr ) {
    this->show();
    _natom = hist->natom();
    natomImg = _natom / hist->nimage();
    bool thermo = hist->hasThermo();
    _temperature->setEnabled(thermo);
    _pression->setEnabled(thermo);
    _stress->setEnabled(thermo);
    _ekin->setEnabled(thermo);
    _etotal->setEnabled(hist->hasEtotal());
    _entropy->setEnabled(thermo);
    _vacf->setEnabled(thermo);
    _pdos->setEnabled(thermo);
    _tdep->setEnabled(thermo);
    _gyration->setEnabled( hist->nimage() > 1 );
  }
  else {
    this->hide();
    _natom = 0;
    natomImg = 0;
  }
  _angleAtoms->setEnabled((natomImg > 2));
  _distance->setEnabled((natomImg > 1));
}

void PlotBar::plotTemperature() {
  emit(sentCommand(_action+" T"));
}

void PlotBar::plotPression() {
  emit(sentCommand(_action+" P"));
}

void PlotBar::plotPdf() {
  emit(sentCommand(_action+" g(r)"));
}

void PlotBar::plotVolume() {
  emit(sentCommand(_action+" V"));
}

void PlotBar::plotAcell() {
  emit(sentCommand(_action+" acell"));
}

void PlotBar::plotAngle() {
  emit(sentCommand(_action+" angle"));
}

void PlotBar::plotAngleAtoms() {
  AtomSelector select(3,_natom,this);
  select.setModal(true);
  std::string command=_action+" angle ";
  if ( select.exec() == QDialog::Accepted ) {
    auto atoms = select.atoms();
    for ( auto atom : atoms )
      command+= (utils::to_string(atom)+" ");
    emit(sentCommand(command));
  }
}

void PlotBar::plotDistance() {
  AtomSelector select(2,_natom,this);
  select.setModal(true);
  std::string command=_action+" distance ";
  if ( select.exec() == QDialog::Accepted ) {
    auto atoms = select.atoms();
    for ( auto atom : atoms )
      command+= (utils::to_string(atom)+" ");
    emit(sentCommand(command));
  }
}

void PlotBar::plotStress() {
  emit(sentCommand(_action+" stress"));
}

void PlotBar::plotEkin() {
  emit(sentCommand(_action+" ekin"));
}

void PlotBar::plotEtotal() {
  emit(sentCommand(_action+" etotal"));
}

void PlotBar::plotEntropy() {
  emit(sentCommand(_action+" entropy"));
}

void PlotBar::plotPositions() {
  PlanSelector select(this);
  select.setModal(true);
  std::string command=_action+" positions ";
  if ( select.exec() == QDialog::Accepted ) {
    command+= select.plan();
    emit(sentCommand(command));
  }
}

void PlotBar::plotVacf() {
  emit(sentCommand(_action+" vacf"));
}

void PlotBar::plotPdos() {
  emit(sentCommand(_action+" pdos"));
}

void PlotBar::plotMsd() {
  emit(sentCommand(_action+" msd"));
}

void PlotBar::plotPacf() {
  emit(sentCommand(_action+" pacf"));
}

void PlotBar::plotTdep() {
  emit(sentCommand(_action+" tdep"));
}

void PlotBar::plotGyration() {
  emit(sentCommand(_action+" gyration"));
}

void PlotBar::changeAction(int index) {
  switch ( index ) {
    case 0 :
      _action = ":plot";
      break;
    case 1 :
      _action = ":print";
      break;
    case 2 :
      _action = ":data";
      break;
  }
}
