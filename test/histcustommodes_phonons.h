#include <cxxtest/TestSuite.h>
#include "hist/histcustommodes.hpp"
#include "io/dtset.hpp"
#include "base/utils.hpp"
#include <sstream>
#include <cmath>
using namespace geometry;

class HistCMPhonons : public CxxTest::TestSuite
{
  public:

    void testAmpliGaussian() {
#include "PTO_DDB.hxx"
      Dtset ref;
      ref.readFromFile("ref_PTO_DDB");
      Ddb* ddb = Ddb::getDdb("ref_PTO_DDB");
      DispDB db;
      db.computeFromDDB(*ddb);
      delete ddb;
      HistCustomModes hist(ref,db);
      const geometry::vec3d& qptGrid={1, 1, 1};
      std::map<HistCustomModes::StrainDistBound,double> strainBoundsIso;

      unsigned ntime = 2000;
      const double temperature = 100;
      hist.setRandomType(HistCustomModes::Normal);
      hist.setStatistics(HistCustomModes::Quantum);
      hist.buildHist(qptGrid, temperature, strainBoundsIso, HistCustomModes::Ignore, ntime);
      Supercell first(hist,0);
      first.findReference(ref);
      DispDB::qptTree modes = { { {0,0,0}, { { 7, 1, 1 } } } };  // mode 7 with 1 amplitude 1 energye (we don't care here)
      std::vector<double> proj(ntime);
#pragma omp parallel for
      for ( unsigned itime = 0 ; itime < ntime ; ++itime ) {
        Supercell current(hist,itime);
        current.setReference(first);
        proj[itime] = current.projectOnModes(ref, db,modes,Supercell::NONE,false)[0];
      }
      double mean = utils::mean(proj.begin(),proj.end());
      double dev = utils::deviation(proj.begin(),proj.end(),mean);
      const double expectedMean = 0;
      const double expectedDev = 0.16;
      TS_ASSERT_DELTA(mean,expectedMean,1e-2);
      TS_ASSERT_DELTA(dev,expectedDev,1e-2);
    }

    void testStatistic() {
#include "PTO_DDB.hxx"
      Dtset ref;
      ref.readFromFile("ref_PTO_DDB");
      Ddb* ddb = Ddb::getDdb("ref_PTO_DDB");
      DispDB db; 
      db.computeFromDDB(*ddb);
      delete ddb;
      HistCustomModes histC(ref,db);
      HistCustomModes histQ(ref,db);
      const geometry::vec3d& qptGrid={1, 1, 1}; 
      std::map<HistCustomModes::StrainDistBound,double> strainBoundsIso;

      unsigned ntime = 1;
      const double temperature = 10000;

      histC.setRandomType(HistCustomModes::Normal);
      histC.setStatistics(HistCustomModes::Classical);
      
      histQ.setRandomType(HistCustomModes::Normal);
      histQ.setStatistics(HistCustomModes::Quantum);
     
      histC.buildHist(qptGrid, temperature, strainBoundsIso, HistCustomModes::Ignore, ntime);
      histQ.buildHist(qptGrid, temperature, strainBoundsIso, HistCustomModes::Ignore, ntime);
     
      Supercell first(histC,0);
      first.findReference(ref);
      DispDB::qptTree modes = { { {0,0,0}, { { 7, 1, 1 } } } };  // mode 7 with 1 amplitude 1 energye (we don't care here)
      std::vector<double> projC(ntime);
      std::vector<double> projQ(ntime);
#pragma omp parallel for
      for ( unsigned itime = 0 ; itime < ntime ; ++itime ) {
        Supercell currentC(histC,itime);
        currentC.setReference(first);
        projC[itime] = currentC.projectOnModes(ref, db,modes,Supercell::NONE,false)[0];
      }
      
      double meanC = utils::mean(projC.begin(),projC.end());
      double devC = utils::deviation(projC.begin(),projC.end(),meanC);


     for ( unsigned itime = 0 ; itime < ntime ; ++itime ) {
        Supercell currentQ(histQ,itime);
        currentQ.setReference(first);
        projQ[itime] = currentQ.projectOnModes(ref, db,modes,Supercell::NONE,false)[0];
      }
      
      double meanQ = utils::mean(projQ.begin(),projQ.end());
      double devQ = utils::deviation(projQ.begin(),projQ.end(),meanQ);

      TS_ASSERT_DELTA(devQ,devC,1e-2)

     
     }   

};
