#include <cxxtest/TestSuite.h>
#include "io/ddb.hpp"

class DdbCrOOH : public CxxTest::TestSuite
{

  Ddb *ddb;

  public:

  DdbCrOOH() : CxxTest::TestSuite(), ddb(nullptr){}

  void setUp()
  {
#include "CrOOH_DDB.hxx"
    TS_ASSERT_THROWS_NOTHING(ddb=Ddb::getDdb("ref_CrOOH_DDB"));
  }

  void tearDown() {
    if ( ddb != nullptr ) delete ddb;
  }

  void testData( void )
  {
    TS_ASSERT_DIFFERS(ddb,nullptr);
    std::cerr << ddb->info();
    auto qpts = ddb->getQpts();
    TS_ASSERT_EQUALS(qpts.size(),4);
  }

  void testZeff( void ) 
  {
    TS_ASSERT_DIFFERS(ddb,nullptr);
    unsigned natom = ddb->natom();
    TS_ASSERT_EQUALS(natom,4);
    using geometry::mat3d, geometry::print;

    mat3d Cr = {{
      3.404827E+00, -1.195162E-01, -5.769288E-01,
        -1.195162E-01, 3.523352E+00, -3.578877E-01,
        -5.769288E-01, -3.578877E-01, 1.869897E+00
    }};
    mat3d O = {{
      -1.897774E+00, -3.877365E-02, -1.871682E-01,
        -3.877365E-02, -1.859322E+00, -1.161065E-01,
        -1.871682E-01, -1.161065E-01, -2.395739E+00
    }};
    mat3d H = {{
      3.907208E-01, 1.970635E-01, 9.512652E-01,
        1.970635E-01, 1.952916E-01, 5.901007E-01,
        9.512652E-01, 5.901007E-01, 2.921581E+00
    }};
    std::vector<mat3d> zeff;
    zeff.push_back(Cr);
    zeff.push_back(O);
    zeff.push_back(O);
    zeff.push_back(H);

    for ( unsigned iatom = 0 ; iatom < natom; ++iatom ) {
      auto Z = ddb->getZeff(iatom);
      for ( unsigned i = 0 ; i < 9 ; ++i )
        TS_ASSERT_DELTA(Z[i],zeff[iatom][i],1e-3);
    }
  }

};

