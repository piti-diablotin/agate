#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <cxxtest/TestSuite.h>
#include "hist/histdata.hpp"
#include "base/utils.hpp"
#include <fstream>
#include <sstream>
#include "diff_files.h"

class HistMDPlot : public CxxTest::TestSuite
{

  std::unique_ptr<HistData> hist;
  Graph::Config config;

  public:

  void setUp()
  {
    hist.reset(nullptr);
#if defined(HAVE_NETCDF) && ( defined(_LP64) || defined(__amd64__) || defined(__x86_64) || defined(__LP64__) )
    try {
#include "SRO_HIST.hxx"
      hist.reset(HistData::getHist("SRO_HIST.nc",true));
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      hist.reset(nullptr);
      TS_FAIL("Unable to create HIST");
    }
#endif
    config.save = Graph::DATA;
  }

  void tearDown() {
    config.x.clear();
    config.y.clear();
    config.xy.clear();
    config.labels.clear();
    config.colors.clear();
  }

  void testMSD( void )
  {
    if (!hist) TS_SKIP("NetCDF is needed");
    try {
#include "SRO_HIST_MSD.hxx"
      std::stringstream input;
      input << "msd";
      hist->plot(0,hist->ntime(),input,nullptr,config);
      std::ifstream fref("ref_SRO_HIST_MSD.dat",std::ios::in);
      std::ifstream fnew("SRO_HIST_MSD.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate MSD");
    }
  }

  void testPDF( void )
  {
    if (!hist) TS_SKIP("NetCDF is needed");
    try {
#include "SRO_HIST_PDF.hxx"
      std::stringstream input;
      input << "g(r)";
      hist->plot(0,hist->ntime(),input,nullptr,config);
      std::ifstream fref("ref_SRO_HIST_PDF.dat",std::ios::in);
      std::ifstream fnew("SRO_HIST_PDF.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate PDF");
    }
  }


  void testPressure( void )
  {
    if (!hist) TS_SKIP("NetCDF is needed");
    try {
#include "SRO_HIST_pressure.hxx"
      std::stringstream input;
      input << "P";
      hist->plot(0,hist->ntime(),input,nullptr,config);
      std::ifstream fref("ref_SRO_HIST_pressure.dat",std::ios::in);
      std::ifstream fnew("SRO_HIST_pressure.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate pressure");
    }
  }

  void testTemperature( void )
  {
    if (!hist) TS_SKIP("NetCDF is needed");
    try {
#include "SRO_HIST_temperature.hxx"
      std::stringstream input;
      input << "T";
      hist->plot(0,hist->ntime(),input,nullptr,config);
      std::ifstream fref("ref_SRO_HIST_temperature.dat",std::ios::in);
      std::ifstream fnew("SRO_HIST_temperature.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate temperature");
    }
  }

  void testVolume( void )
  {
    if (!hist) TS_SKIP("NetCDF is needed");
    try {
#include "SRO_HIST_volume.hxx"
#include "SRO_222.hxx"
      std::stringstream input;
      input << "V";
      hist->plot(0,hist->ntime(),input,nullptr,config);
      std::ifstream fref("ref_SRO_HIST_volume.dat",std::ios::in);
      std::ifstream fnew("SRO_HIST_volume.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate volume");
    }
  }

  void testStrain( void )
  {
    if (!hist) TS_SKIP("NetCDF is needed");
    try {
#include "SRO_HIST_strain.hxx"
      std::stringstream input;
      input << "strain reference=ref_SRO_222";
      hist->plot(0,hist->ntime(),input,nullptr,config);
      std::ifstream fref("ref_SRO_HIST_strain.dat",std::ios::in);
      std::ifstream fnew("SRO_HIST_strain.dat",std::ios::in);
      DIFF_FILES(fref,fnew)
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to calculate strain");
    }
  }
};

