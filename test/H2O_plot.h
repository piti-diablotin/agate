#include <cxxtest/TestSuite.h>
#include "base/mendeleev.hpp"
#include "hist/histdata.hpp"
#include <fstream>
#include <sstream>
#include "diff_files.h"

Agate::mendeleev Agate::Mendeleev;

class HistMDPlot : public CxxTest::TestSuite
{

  HistData *hist;

  public:

  void setUp()
  {
    try {
#include "H2O_HIST.hxx"
      hist = HistData::getHist("H2O_HIST.nc",true);
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      hist = nullptr;
      TS_FAIL("Unable to create HIST");
    }
  }

  void tearDown() {
    if ( hist != nullptr ) delete hist;
  }

  void testMSD( void )
  {
    try {
#include "H2O_HIST_MSD.hxx"
      std::stringstream input;
      input << "msd";
      hist->plot(0,hist->ntime(),input,nullptr,Graph::DATA);
      std::ifstream fref("ref_H2O_HIST_MSD.dat",std::ios::in);
      std::ifstream fnew("H2O_HIST_MSD.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate MSD");
    }
  }

  void testPDF( void )
  {
    try {
#include "H2O_HIST_PDF.hxx"
      std::stringstream input;
      input << "g(r)";
      hist->plot(0,hist->ntime(),input,nullptr,Graph::DATA);
      std::ifstream fref("ref_H2O_HIST_PDF.dat",std::ios::in);
      std::ifstream fnew("H2O_HIST_PDF.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate PDF");
    }
  }

  void testAngle( void )
  {
    try {
#include "H2O_HIST_angle.hxx"
      std::stringstream input;
      input << "angle";
      hist->plot(0,hist->ntime(),input,nullptr,Graph::DATA);
      std::ifstream fref("ref_H2O_HIST_angle.dat",std::ios::in);
      std::ifstream fnew("H2O_HIST_angle.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate Angle");
    }
  }

  void testAngleAtoms( void )
  {
    try {
#include "H2O_HIST_angle_269_77_333.hxx"
      std::stringstream input;
      input << "angle 269 77 333";
      hist->plot(0,hist->ntime(),input,nullptr,Graph::DATA);
      std::ifstream fref("ref_H2O_HIST_angle_269_77_333.dat",std::ios::in);
      std::ifstream fnew("H2O_HIST_angle_269_77_333.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate Angles");
    }
  }

  void testDistance( void )
  {
    try {
#include "H2O_HIST_distance_9_265.hxx"
      std::stringstream input;
      input << "distance 9 265";
      hist->plot(0,hist->ntime(),input,nullptr,Graph::DATA);
      std::ifstream fref("ref_H2O_HIST_distance_9_265.dat",std::ios::in);
      std::ifstream fnew("H2O_HIST_distance_9_265.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate distances");
    }
  }

  void testLattice( void )
  {
    try {
#include "H2O_HIST_latticeLengths.hxx"
      std::stringstream input;
      input << "acell";
      hist->plot(0,hist->ntime(),input,nullptr,Graph::DATA);
      std::ifstream fref("ref_H2O_HIST_latticeLengths.dat",std::ios::in);
      std::ifstream fnew("H2O_HIST_latticeLengths.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate lattice");
    }
  }

  void testPressure( void )
  {
    try {
#include "H2O_HIST_pressure.hxx"
      std::stringstream input;
      input << "P";
      hist->plot(0,hist->ntime(),input,nullptr,Graph::DATA);
      std::ifstream fref("ref_H2O_HIST_pressure.dat",std::ios::in);
      std::ifstream fnew("H2O_HIST_pressure.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate pressure");
    }
  }

  void testTemperature( void )
  {
    try {
#include "H2O_HIST_temperature.hxx"
      std::stringstream input;
      input << "T";
      hist->plot(0,hist->ntime(),input,nullptr,Graph::DATA);
      std::ifstream fref("ref_H2O_HIST_temperature.dat",std::ios::in);
      std::ifstream fnew("H2O_HIST_temperature.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate temperature");
    }
  }

  void testVolume( void )
  {
    try {
#include "H2O_HIST_volume.hxx"
      std::stringstream input;
      input << "V";
      hist->plot(0,hist->ntime(),input,nullptr,Graph::DATA);
      std::ifstream fref("ref_H2O_HIST_volume.dat",std::ios::in);
      std::ifstream fnew("H2O_HIST_volume.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate volume");
    }
  }
};

