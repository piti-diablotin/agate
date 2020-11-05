#include <cxxtest/TestSuite.h>
#include "hist/histcustommodes.hpp"
#include "io/dtset.hpp"
#include <sstream>

using namespace geometry;

class RotateMatrix : public CxxTest::TestSuite
{
  public:
  void testRotateMatrix ( void )
  { try{  
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

 
    // hist.setStrainShearDir(true,false,false);
    // hist.rotateStrain(strain, HistCustomModes::Tetra);
    // print(strain);
    // mat3d RotateStrainMatrix;
    // RotateStrainMatrix = rotMatrixX * strainTest;
    // print(RotateStrainMatrix);
    // TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(1,1)], strain[mat3dind(1,1)], 1e-6 );
    // TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,2)], strain[mat3dind(2,2)], 1e-6 );
    // TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,3)], strain[mat3dind(3,3)], 1e-6 );
    // TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,2)], strain[mat3dind(3,2)], 1e-6 );
    // TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,1)], strain[mat3dind(3,1)], 1e-6 );
    // TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,1)], strain[mat3dind(2,1)], 1e-6 );

    hist.setStrainTetraDir(true,false,false);
    strain = {1, 2, 3,
              4, 5, 6,
              7, 8, 9 };
    hist.rotateStrain(strain, HistCustomModes::Tetra);
    print(strain);
    RotateStrainMatrix={};
    RotateStrainMatrix = rotMatrixX * strainTest;
    print(RotateStrainMatrix);
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
    print(strain);
    RotateStrainMatrix={};
    RotateStrainMatrix = rotMatrixY * strainTest;
    print(RotateStrainMatrix);
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
    print(strain);
    RotateStrainMatrix={};
    RotateStrainMatrix = rotMatrixZ * strainTest;
    print(RotateStrainMatrix);
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
    print(strain);
    RotateStrainMatrix={};
    RotateStrainMatrix = rotMatrixXY * strainTest;
    RotateStrainMatrix = RotateStrainMatrix * rotMatrixXY;
    print(RotateStrainMatrix);
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
    print(strain);
    RotateStrainMatrix={};
    RotateStrainMatrix = rotMatrixXZ * strainTest;
    RotateStrainMatrix = RotateStrainMatrix * rotMatrixXZ;
    print(RotateStrainMatrix);
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
    print(strain);
    RotateStrainMatrix={};
    RotateStrainMatrix = rotMatrixYZ * strainTest;
    RotateStrainMatrix = RotateStrainMatrix * rotMatrixYZ;
    print(RotateStrainMatrix);
    TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(1,1)], strain[mat3dind(1,1)], 1e-6 );
    TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,2)], strain[mat3dind(2,2)], 1e-6 );
    TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,3)], strain[mat3dind(3,3)], 1e-6 );
    TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,2)], strain[mat3dind(3,2)], 1e-6 );
    TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(3,1)], strain[mat3dind(3,1)], 1e-6 );
    TS_ASSERT_DELTA( RotateStrainMatrix[mat3dind(2,1)], strain[mat3dind(2,1)], 1e-6 );



    }

catch(Exception e){
std::cout<< e.fullWhat();
   }
  }
};
