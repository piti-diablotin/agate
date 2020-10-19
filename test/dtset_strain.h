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
      const double rprim_dot_eta[9] = {56.4476, 16.9334, 0.912645, 
                                       16.8769, 56.254, -0.000947379, 
                                       0.026135, 0.00845817, 55.0218};
                                      
                                      
                                      
                                      
      const mat3d eta={0.98, 0.294, 0,
                       0.294, 0.98 , 0,
                       0, 0 , 0.955259};
      ref.applyStrain(eta);
      auto rprim = ref.rprim();
      for ( unsigned i = 0; i < 9 ; ++i )
        TS_ASSERT_DELTA( rprim[i], rprim_dot_eta[i], 1e-3 );

    }
};

