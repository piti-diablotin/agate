#include <cxxtest/TestSuite.h>
#include "io/ddb.hpp"

class DdbYGO : public CxxTest::TestSuite
{

  Ddb *ddb;

  public:

  DdbYGO() : CxxTest::TestSuite(), ddb(nullptr){}

  void setUp()
  {
#include "YGO_DDB.hxx"
    TS_ASSERT_THROWS_NOTHING(ddb=Ddb::getDdb("ref_YGO_DDB"));
  }

  void tearDown() {
    if ( ddb != nullptr ) delete ddb;
  }

  void testData( void )
  {
    TS_ASSERT_DIFFERS(ddb,nullptr);
    std::cerr << ddb->info();
    auto qpts = ddb->getQpts();
    TS_ASSERT_EQUALS(qpts.size(),1);
  }

  void testZeff( void ) 
  {
    TS_ASSERT_DIFFERS(ddb,nullptr);
    unsigned natom = ddb->natom();
    TS_ASSERT_EQUALS(natom,10);
    using geometry::mat3d;
    using geometry::print;

    mat3d Y = {{
      4.043280E+00,  -8.837375E-14,   0.000000E+00,
        0.000000E+00,   4.043290E+00,   0.000000E+00,
        0.000000E+00,   0.000000E+00,   3.482320E+00
    }};
    mat3d Fe = {{
      2.918480E+00,   9.062889E-14,   0.000000E+00,
        0.000000E+00,   2.918470E+00,   0.000000E+00,
        0.000000E+00,   0.000000E+00,   3.479540E+00
    }};
    mat3d O1 = {{
      -2.326450E+00,   4.707346E-14,  -4.529035E-14,
        0.000000E+00,  -2.326455E+00,   5.000000E-06,
        0.000000E+00,   0.000000E+00,  -2.531280E+00
    }};
    mat3d O2 = {{
      -2.317050E+00,   4.796163E-14,   0.000000E+00,
        0.000000E+00,  -2.317055E+00,   0.000000E+00,
        0.000000E+00,   0.000000E+00,  -1.901780E+00
    }};
    std::vector<mat3d> zeff;
    zeff.push_back(Y);
    zeff.push_back(Y);
    zeff.push_back(Fe);
    zeff.push_back(Fe);
    zeff.push_back(O1);
    zeff.push_back(O1);
    zeff.push_back(O1);
    zeff.push_back(O1);
    zeff.push_back(O2);
    zeff.push_back(O2);

    for ( unsigned iatom = 0 ; iatom < natom ; ++iatom ) {
      auto Z = ddb->getZeff(iatom);
      for ( unsigned i = 0 ; i < 9 ; ++i )
        TS_ASSERT_DELTA(Z[i],zeff[iatom][i],1e-3);
    }
  }

  void testEpsInf( void ) 
  {
    TS_ASSERT_DIFFERS(ddb,nullptr);
    using geometry::mat3d;
    using geometry::print;

    mat3d ref = {{
      4.398885,     0.000000,     0.000000,     0.000000,     4.398885,     0.000000,      0.000000,     0.000000,     4.348030
    }};

    auto eps = ddb->getEpsInf();
    for ( unsigned i = 0 ; i < 9 ; ++i ) {
      std::ostringstream str;
      str << "indice " << i;
      TSM_ASSERT_DELTA(str.str().c_str(),eps[i],ref[i],1e-3);
    }
  }

};

