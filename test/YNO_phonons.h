#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <cxxtest/TestSuite.h>
#include "canvas/canvasphonons.hpp"
#include "base/utils.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "diff_files.h"

class CanvasPhononsProj : public CxxTest::TestSuite
{

  CanvasPhonons *canvas;

  public:

  void setUp()
  {
    canvas = nullptr;
    canvas = new CanvasPhonons(false);
#ifdef HAVE_NETCDF
    try {
#include "YNO_HIST.hxx"
#include "YNO_eigenvec.hxx"
      canvas->openFile("YNO_eigenvec.nc");
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to create Canvas");
    }
#endif
  }

  void tearDown() {
    if ( canvas != nullptr ) delete canvas;
  }


  void testAnalyze( void )
  {
#ifndef HAVE_NETCDF
    TS_SKIP("NetCDF is needed");
#endif
    try {
#include "YNO_HIST_Analysis_absolute1.hxx"
#include "YNO_HIST_Analysis_absolute0.hxx"
      std::istringstream input;
      input.str("0 0 0 all");
      canvas->alter(std::string("add"),input);
      input.clear();
      input.str("YNO_HIST.nc absolute=1");
      canvas->alter(std::string("analyze"),input);
      std::ifstream fref2("ref_YNO_HIST_Analysis_absolute1.dat",std::ios::in);
      std::ifstream fnew2("YNO_HIST_Analysis.dat",std::ios::in);
      DIFF_FILES(fref2,fnew2);
      fref2.close();
      fnew2.close();
      input.clear();
      input.str("YNO_HIST.nc absolute=0");
      canvas->alter(std::string("analyze"),input);
      std::ifstream fref3("ref_YNO_HIST_Analysis_absolute0.dat",std::ios::in);
      std::ifstream fnew3("YNO_HIST_Analysis.dat",std::ios::in);
      DIFF_FILES(fref3,fnew3);
      fref3.close();
      fnew3.close();
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate mode projection");
    }
  }


};

