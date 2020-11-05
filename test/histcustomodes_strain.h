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

     mat3d rotMatrixY = {1, 0, 0,
                         0, 0, 1,
                         0, 1, 0};   
    
     mat3d rotMatrixZ = {1, 0, 0,
                         0, 1, 0,
                         0, 0, 1}; 

    
    mat3d strainTest = {1, 2, 3,
                        4, 5, 6,
                        7, 8, 9 };  
    
    mat3d strain = {1, 2, 3,
                    4, 5, 6,
                    7, 8, 9 };
 

     hist.setStrainTetraDir(false,false,true);
     hist.rotateStrain(strain, HistCustomModes::Tetra);
     print(strain);
     mat3d RotateStrainMatrix;
     RotateStrainMatrix = rotMatrixZ * strainTest;
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

