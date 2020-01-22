#include <cxxtest/TestSuite.h>
#include "base/geometry.hpp"
#include "base/utils.hpp"
#include <sstream>

using namespace geometry;

class GeometryMatrix : public CxxTest::TestSuite
{
  public:
    void testIndice( void )
    {
      const unsigned a = 1;
      const unsigned b = 2;
      const unsigned c = 3;
      const unsigned x = 1;
      const unsigned y = 2;
      const unsigned z = 3;
      TS_ASSERT_EQUALS( mat3dind(a,x), 0 );
      TS_ASSERT_EQUALS( mat3dind(b,x), 1 );
      TS_ASSERT_EQUALS( mat3dind(c,x), 2 );
      TS_ASSERT_EQUALS( mat3dind(a,y), 3 );
      TS_ASSERT_EQUALS( mat3dind(b,y), 4 );
      TS_ASSERT_EQUALS( mat3dind(c,y), 5 );
      TS_ASSERT_EQUALS( mat3dind(a,z), 6 );
      TS_ASSERT_EQUALS( mat3dind(b,z), 7 );
      TS_ASSERT_EQUALS( mat3dind(c,z), 8 );
    }

    void testDeterminant( void )
    {
      const mat3d t = { 6,  1, 1,
                        4, -2, 5,
                        2,  8, 7 };
      const double tt[9] = { 6,  1, 1,
                        4, -2, 5,
                        2,  8, 7 };
      TS_ASSERT_DELTA( det(t), -306, 1e-13 );
      TS_ASSERT_DELTA( det(tt), -306, 1e-13 );
    }

    void testInverse( void )
    {
      const mat3d t = { 6,  1, 1,
                        4, -2, 5,
                        2,  8, 7 };
      const double tt[9] = { 6,  1, 1,
                        4, -2, 5,
                        2,  8, 7 };
      mat3d inv = invert(invert(t));
      for ( unsigned i = 0; i < 9 ; ++i )
        TS_ASSERT_DELTA( inv[i], t[i], 1e-13 );

      inv = invert(invert(tt));
      for ( unsigned i = 0; i < 9 ; ++i )
        TS_ASSERT_DELTA( inv[i], t[i], 1e-13 );
    }
};

