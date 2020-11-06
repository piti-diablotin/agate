#include <cxxtest/TestSuite.h>
#include "hist/histcustommodes.hpp"
#include "io/dtset.hpp"
#include <sstream>

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
  //try{
#include "CTO_Pnma_444.hxx"
    Dtset ref;
    ref.readFromFile("CTO_Pnma_444.in");
    DispDB db;
    HistCustomModes hist(ref,db);
    const geometry::vec3d& qptGrid={1, 1, 1};
    std::map<HistCustomModes::StrainDistBound,double> strainBounds {{HistCustomModes::IsoMin, 0.0001 }, {HistCustomModes::IsoMax, 0.1}};
    unsigned ntime = 1;
    const double temperature = 0;
    hist.buildHist(qptGrid, temperature, strainBounds, HistCustomModes::Ignore, ntime);
    auto strain = hist.getStrain(0,ref);
    TS_ASSERT_DELTA(strain[0], strain[1], 10e-10);
    TS_ASSERT_DELTA(strain[0], strain[2], 10e-10);
    TS_ASSERT_DELTA(strain[1], strain[2], 10e-10 );
    TS_ASSERT(strain[0] <= 0.1);
    for (unsigned i=0; i<6; i++){
    std::cout << strain[i] << '\n';
    }


}


};
