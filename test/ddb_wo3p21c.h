#include <cxxtest/TestSuite.h>
#include "io/ddb.hpp"

class DdbWO3 : public CxxTest::TestSuite
{

  Ddb *ddb;

  public:

  DdbWO3() : CxxTest::TestSuite(), ddb(nullptr){}

  void setUp()
  {
#include "WO3_p21c_DDB.hxx"
    TS_ASSERT_THROWS_NOTHING(ddb=Ddb::getDdb("ref_WO3_p21c_DDB"));
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
    TS_ASSERT_EQUALS(natom,16);
    using geometry::mat3d, geometry::print;

    std::vector<mat3d> zeff;
    zeff.push_back({{
        1.049826E+01,  -8.647195E-01,  -6.416455E-02,
        8.162467E-01,   9.503555E+00,   6.862659E-01, // 52->35
        -9.251144E-02,  -7.323376E-01,   7.723649E+00 // 48->36
        }});
    zeff.push_back({{
        1.049826E+01,   8.647195E-01,  -6.416455E-02,
        -8.162467E-01,   9.503555E+00,  -6.862659E-01, // 52->35
        -9.251144E-02,   7.323376E-01,   7.726049E+00  // 48->60
        }});
    zeff.push_back({{
        1.049826E+01,  -8.647195E-01,  -6.416455E-02,
        8.162467E-01,   9.507055E+00,   6.862659E-01, // 52->70
        -9.251144E-02,  -7.323376E-01,   7.726049E+00 // 48->60
        }});
    zeff.push_back({{
        1.049826E+01,   8.647195E-01,  -6.416455E-02,
        -8.162467E-01,   9.507055E+00,  -6.862659E-01, // 52->70
        -9.251144E-02,   7.323376E-01,   7.723649E+00 // 48->36
        }});
    zeff.push_back({{
        -4.642712E+00,   3.177868E+00,  -6.721881E-01,
        2.983692E+00,  -4.172355E+00,   5.609011E-01,
        -2.366123E-01,   2.354364E-01,  -1.048538E+00
        }});
    zeff.push_back({{
        -4.642712E+00,  -3.177868E+00,  -6.721881E-01,
        -2.983692E+00,  -4.172355E+00,  -5.609011E-01,
        -2.366123E-01,  -2.354364E-01,  -1.048538E+00
        }});
    zeff.push_back({{
        -4.642712E+00,   3.177868E+00,  -6.721881E-01,
        2.983692E+00,  -4.172355E+00,   5.609011E-01,
        -2.366123E-01,   2.354364E-01,  -1.048538E+00
        }});
    zeff.push_back({{
        -4.642712E+00,  -3.177868E+00,  -6.721881E-01,
        -2.983692E+00,  -4.172355E+00,  -5.609011E-01,
        -2.366123E-01,  -2.354364E-01,  -1.048538E+00
        }});
    zeff.push_back({{
        -4.689443E+00,   3.043796E+00,   6.299436E-01,
        2.973238E+00,  -4.110545E+00,  -5.521265E-01,
        2.526205E-01,  -2.436102E-01,  -1.075372E+00
        }});
    zeff.push_back({{
        -4.689443E+00,  -3.043796E+00,   6.299436E-01,
        -2.973238E+00,  -4.110545E+00,   5.521265E-01,
        2.526205E-01,   2.436102E-01,  -1.075372E+00
        }});
    zeff.push_back({{
        -4.689443E+00,   3.043796E+00,   6.299436E-01,
        2.973238E+00,  -4.110545E+00,  -5.521265E-01,
        2.526205E-01,  -2.436102E-01,  -1.075372E+00
        }});
    zeff.push_back({{
        -4.689443E+00,  -3.043796E+00,   6.299436E-01,
        -2.973238E+00,  -4.110545E+00,   5.521265E-01,
        2.526205E-01,   2.436102E-01,  -1.075372E+00
        }});
    zeff.push_back({{
        -1.169220E+00,   1.113389E-02,   1.066678E-01,
        -2.736095E-02,  -1.229345E+00,   2.350488E-01,
        7.632611E-02,   4.360583E-01,  -5.600130E+00
        }});
    zeff.push_back({{
        -1.169220E+00,  -1.113389E-02,   1.066678E-01,
        2.736095E-02,  -1.229345E+00,  -2.350488E-01,
        7.632611E-02,  -4.360583E-01,  -5.600130E+00
        }});
    zeff.push_back({{
        -1.169220E+00,   1.113389E-02,   1.066678E-01,
        -2.736095E-02,  -1.229345E+00,   2.350488E-01,
        7.632611E-02,   4.360583E-01,  -5.600130E+00
        }});
    zeff.push_back({{
        -1.169220E+00,  -1.113389E-02,   1.066678E-01,
        2.736095E-02,  -1.229345E+00,  -2.350488E-01,
        7.632611E-02,  -4.360583E-01,  -5.600130E+00
        }});

    for ( unsigned iatom = 0 ; iatom < natom ; ++iatom ) {
      auto Z = ddb->getZeff(iatom);
      for ( unsigned i = 0 ; i < 9 ; ++i ){
        std::ostringstream str;
        str << "Atom " << (iatom+1) << " ";
        str << "indice " << i;
        TSM_ASSERT_DELTA(str.str().c_str(),Z[i],zeff[iatom][i],1e-3);
      }
    }
  }

  void testEpsInf( void ) 
  {
    TS_ASSERT_DIFFERS(ddb,nullptr);
    using geometry::mat3d, geometry::print;

    mat3d ref = {{
      7.03228400,   0.00000000,   -0.01121100, 0.00000000,   6.56787400,   0.00000000, -0.01121100,   0.00000000,   5.44549800
    }};

    auto eps = ddb->getEpsInf();
    for ( unsigned i = 0 ; i < 9 ; ++i ) {
      std::ostringstream str;
      str << "indice " << i;
      TSM_ASSERT_DELTA(str.str().c_str(),eps[i],ref[i],1e-3);
    }
  }

};

