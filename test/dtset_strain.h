#include <cxxtest/TestSuite.h>
#include "io/dtset.hpp"
#include <sstream>

using namespace geometry;

class StrainMatrix : public CxxTest::TestSuite
{
  public:


    void testStrainProduct( void )
    {   
#include "CTO_Pnma_444.hxx"
      Dtset ref;
      ref.readFromFile("CTO_Pnma_444.in");
      const double rprim_dot_eta[9] = {58.7522, 17.2195, 0.9742,
        17.2808, 58.5496, 0.285605 , 
        0.0248712, 0.00065099, 54.1388};




      const mat3d eta={0.020000, 0.30000, 0.00000,
        0.300000, 0.02 , 0.000000,
        0.0000, 0.000000 , -0.0600699};
      ref.applyStrain(eta);
      auto rprim = ref.rprim();
      for ( unsigned i = 0; i < 9 ; ++i )
        TS_ASSERT_DELTA( rprim[i], rprim_dot_eta[i], 1e-3 );

    }   

    void testGetStrain ( void )
    {   
      Dtset ref, test;
      ref.readFromFile("CTO_Pnma_444.in");
      test = ref;

      const mat3d eta={0.0, 0.0, -0.136721,
        0.00000, 0.762778, 0.000000,
        0.762778, 0.00000,0.0000000};

      test.applyStrain(eta);
      auto strain = test.getStrain(ref);
      TS_ASSERT_DELTA( eta[mat3dind(1,1)], strain[0], 1e-6 );
      TS_ASSERT_DELTA( eta[mat3dind(2,2)], strain[1], 1e-6 );
      TS_ASSERT_DELTA( eta[mat3dind(3,3)], strain[2], 1e-6 );
      TS_ASSERT_DELTA( eta[mat3dind(3,2)], strain[3], 1e-6 );
      TS_ASSERT_DELTA( eta[mat3dind(3,1)], strain[4], 1e-6 );
      TS_ASSERT_DELTA( eta[mat3dind(2,1)], strain[5], 1e-6 );
    }   
};
