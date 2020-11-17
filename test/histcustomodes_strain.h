#include <cxxtest/TestSuite.h>
#include "hist/histcustommodes.hpp"
#include "io/dtset.hpp"
#include "base/utils.hpp"
#include <sstream>
#include <cmath>
using namespace geometry;

class RotateMatrix : public CxxTest::TestSuite
{
  public:
    void testRotateMatrix ( void )
    {  
      Dtset dtset;
      DispDB db;
      HistCustomModes hist(dtset, db);


      mat3d rotMatrixX = {0, 0, 1,
        0, 1, 0,
        1, 0, 0}; 


      mat3d rotMatrixYZ = {0, 0, 1,
        0, 1, 0,
        1, 0, 0}; 

      mat3d rotMatrixY = {1, 0, 0,
        0, 0, 1,
        0, 1, 0};   
      mat3d rotMatrixXZ = {1, 0, 0,
        0, 0, 1,
        0, 1, 0}; 

      mat3d rotMatrixZ = {1, 0, 0,
        0, 1, 0,
        0, 0, 1}; 

      mat3d rotMatrixXY = {1, 0, 0,
        0, 1, 0,
        0, 0, 1};

      mat3d strainTest = {1, 2, 3,
        4, 5, 6,
        7, 8, 9 };  

      mat3d strain = {1, 2, 3,
        4, 5, 6,
        7, 8, 9 };



      mat3d RotateStrainMatrix;
      hist.setStrainTetraDir(true,false,false);
      strain = {1, 2, 3,
        4, 5, 6,
        7, 8, 9 };
      hist.rotateStrain(strain, HistCustomModes::Tetra);
      RotateStrainMatrix={};
      RotateStrainMatrix = rotMatrixX * strainTest;
      RotateStrainMatrix = RotateStrainMatrix * rotMatrixX;
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(1,1)], strain[mat3dind(1,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,2)], strain[mat3dind(2,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,3)], strain[mat3dind(3,3)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,2)], strain[mat3dind(3,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,1)], strain[mat3dind(3,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,1)], strain[mat3dind(2,1)], 1e-6 );


      hist.setStrainTetraDir(false,true,false);
      strain = {1, 2, 3,
        4, 5, 6,
        7, 8, 9 };
      hist.rotateStrain(strain, HistCustomModes::Tetra);
      RotateStrainMatrix={};
      RotateStrainMatrix = rotMatrixY * strainTest;
      RotateStrainMatrix = RotateStrainMatrix * rotMatrixY;
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(1,1)], strain[mat3dind(1,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,2)], strain[mat3dind(2,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,3)], strain[mat3dind(3,3)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,2)], strain[mat3dind(3,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,1)], strain[mat3dind(3,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,1)], strain[mat3dind(2,1)], 1e-6 );


      hist.setStrainTetraDir(false,false,true);
      strain = {1, 2, 3,
        4, 5, 6,
        7, 8, 9 };
      hist.rotateStrain(strain, HistCustomModes::Tetra);
      RotateStrainMatrix={};
      RotateStrainMatrix = rotMatrixZ * strainTest;
      RotateStrainMatrix = RotateStrainMatrix * rotMatrixZ;
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(1,1)], strain[mat3dind(1,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,2)], strain[mat3dind(2,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,3)], strain[mat3dind(3,3)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,2)], strain[mat3dind(3,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,1)], strain[mat3dind(3,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,1)], strain[mat3dind(2,1)], 1e-6 );


      hist.setStrainShearDir(true,false,false);
      strain = {1, 2, 3,
        4, 5, 6,
        7, 8, 9 };
      hist.rotateStrain(strain, HistCustomModes::Shear);
      RotateStrainMatrix={};
      RotateStrainMatrix = rotMatrixXY * strainTest;
      RotateStrainMatrix = RotateStrainMatrix * rotMatrixXY;
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(1,1)], strain[mat3dind(1,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,2)], strain[mat3dind(2,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,3)], strain[mat3dind(3,3)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,2)], strain[mat3dind(3,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,1)], strain[mat3dind(3,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,1)], strain[mat3dind(2,1)], 1e-6 );


      hist.setStrainShearDir(false,true,false);
      strain = {1, 2, 3,
        4, 5, 6,
        7, 8, 9 };
      hist.rotateStrain(strain, HistCustomModes::Shear);
      RotateStrainMatrix={};
      RotateStrainMatrix = rotMatrixXZ * strainTest;
      RotateStrainMatrix = RotateStrainMatrix * rotMatrixXZ;
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(1,1)], strain[mat3dind(1,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,2)], strain[mat3dind(2,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,3)], strain[mat3dind(3,3)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,2)], strain[mat3dind(3,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,1)], strain[mat3dind(3,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,1)], strain[mat3dind(2,1)], 1e-6 );


      hist.setStrainShearDir(false,false,true);
      strain = {1, 2, 3,
        4, 5, 6,
        7, 8, 9 };
      hist.rotateStrain(strain, HistCustomModes::Shear);
      RotateStrainMatrix={};
      RotateStrainMatrix = rotMatrixYZ * strainTest;
      RotateStrainMatrix = RotateStrainMatrix * rotMatrixYZ;
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(1,1)], strain[mat3dind(1,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,2)], strain[mat3dind(2,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,3)], strain[mat3dind(3,3)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,2)], strain[mat3dind(3,2)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,1)], strain[mat3dind(3,1)], 1e-6 );
      TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,1)], strain[mat3dind(2,1)], 1e-6 );
    }

    void testStrainAmplitude ( void ) {
#include "CTO_Pnma_444.hxx"
      Dtset refIso;
      Dtset refTetra;
      Dtset refShear;
      refIso.readFromFile("CTO_Pnma_444.in");
      refTetra.readFromFile("CTO_Pnma_444.in");
      refShear.readFromFile("CTO_Pnma_444.in");
      DispDB db;
      HistCustomModes histIso(refIso,db);
      const geometry::vec3d& qptGrid={1, 1, 1};
      std::map<HistCustomModes::StrainDistBound,double> strainBoundsIso {{HistCustomModes::IsoMin, 0.0001 }, {HistCustomModes::IsoMax, 0.1}};
      unsigned ntime = 1;
      const double temperature = 0;

      histIso.buildHist(qptGrid, temperature, strainBoundsIso, HistCustomModes::Ignore, ntime);
      auto strainIso = histIso.getStrain(0,refIso);
      TS_ASSERT_DELTA(strainIso[0], strainIso[1], 10e-10);
      TS_ASSERT_DELTA(strainIso[0], strainIso[2], 10e-10);
      TS_ASSERT_DELTA(strainIso[1], strainIso[2], 10e-10 );
      TS_ASSERT(strainIso[0] <= 0.1);
      TS_ASSERT(strainIso[0] >= 0.0001);
      TS_ASSERT(strainIso[1] <= 0.1);
      TS_ASSERT(strainIso[1] >= 0.0001);   
      TS_ASSERT(strainIso[2] <= 0.1);
      TS_ASSERT(strainIso[2] >= 0.0001);    

      HistCustomModes histTetra(refTetra,db);
      std::map<HistCustomModes::StrainDistBound,double> strainBoundsTetra {{HistCustomModes::TetraMin, 0.0001 }, {HistCustomModes::TetraMax, 0.1}};
      histTetra.setStrainTetraDir(true,true,true);
      histTetra.buildHist(qptGrid, temperature, strainBoundsTetra, HistCustomModes::Ignore, ntime);
      auto strainTetra = histTetra.getStrain(0,refTetra);
      TS_ASSERT((strainTetra[0] <= 0.1 && strainTetra[1]<= 0.1) || (strainTetra[0] <= 0.1 && strainTetra[2]<= 0.1) || (strainTetra[1] <= 0.1 && strainTetra[2]<= 0.1));
      TS_ASSERT((strainTetra[0]>= 0.0001 && strainTetra[1]>= 0.0001) || (strainTetra[0]>= 0.0001 && strainTetra[2]>= 0.0001) || (strainTetra[1] >= 0.0001 && strainTetra[2]>= 0.0001));
      TS_ASSERT((abs(strainTetra[0] - strainTetra[1]) <= 10e-10) || (abs(strainTetra[0] - strainTetra[2]) <= 10e-10)|| (abs(strainTetra[1] - strainTetra[2]) <= 10e-10)) ;
      TS_ASSERT((abs((-2*strainTetra[0]-strainTetra[0]*strainTetra[0])/((1+strainTetra[0])*(1+strainTetra[0])) - strainTetra[2]) <= 10e-6) || (abs((-2*strainTetra[0]-strainTetra[0]*strainTetra[0])/((1+strainTetra[0])*(1+strainTetra[0])) - strainTetra[1]) <= 10e-6) || (abs((-2*strainTetra[2]-strainTetra[2]*strainTetra[2])/((1+strainTetra[2])*(1+strainTetra[2])) - strainTetra[0]) <= 10e-6));    

      HistCustomModes histShear(refShear,db);
      std::map<HistCustomModes::StrainDistBound,double> strainBoundsShear {{HistCustomModes::ShearMin, 0.0001 }, {HistCustomModes::ShearMax, 0.1}};
      histShear.setStrainShearDir(true,true,true);
      histShear.buildHist(qptGrid, temperature, strainBoundsShear, HistCustomModes::Ignore, ntime);
      auto strainShear = histShear.getStrain(0,refShear);
      TS_ASSERT((strainShear[3] <= 0.1) || (strainShear[4] <= 0.1) || (strainShear[5] <= 0.1));
      TS_ASSERT((strainShear[3] >= 0.0001) || (strainShear[4] >= 0.0001) || (strainShear[5] >= 0.0001));
      TS_ASSERT((abs((strainShear[5]*strainShear[5])/(1-strainShear[5]*strainShear[5]) - strainShear[2]) <= 10e-6)|| (abs((strainShear[4]*strainShear[4])/(1-strainShear[4]*strainShear[4]) - strainShear[1]) <= 10e-6) || (abs((strainShear[3]*strainShear[3])/(1-strainShear[3]*strainShear[3]) - strainShear[0]) <= 10e-6));    
    }

    void testStrainGaussian() {
#include "CTO_Pnma_444.hxx"
      Dtset refIso;
      refIso.readFromFile("CTO_Pnma_444.in");
      DispDB db;
      HistCustomModes histIso(refIso,db);
      const geometry::vec3d& qptGrid={1, 1, 1};
      std::map<HistCustomModes::StrainDistBound,double> strainBoundsIso {{HistCustomModes::IsoMin, 0.0001 }, {HistCustomModes::IsoMax, 0.1}};
      unsigned ntime = 500;
      const double temperature = 0;
      histIso.setRandomType(HistCustomModes::Normal);

      histIso.buildHist(qptGrid, temperature, strainBoundsIso, HistCustomModes::Ignore, ntime);
      std::vector<double> strainIso;
      for ( unsigned itime = 0 ; itime < ntime ; ++itime ) {
        strainIso.push_back(histIso.getStrain(itime,refIso)[0]);
      }
      double mean = utils::mean(strainIso.begin(),strainIso.end());
      double dev = utils::deviation(strainIso.begin(),strainIso.end(),mean);
      const double expectedMean = (strainBoundsIso[HistCustomModes::IsoMin]+strainBoundsIso[HistCustomModes::IsoMax])*0.5;
      const double expectedDev = (strainBoundsIso[HistCustomModes::IsoMax]-strainBoundsIso[HistCustomModes::IsoMin])/6;
      TS_ASSERT_DELTA(mean,expectedMean,1e-3);
      TS_ASSERT_DELTA(dev,expectedDev,1e-3);
    }

};
