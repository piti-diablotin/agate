#include <cxxtest/TestSuite.h>
#include <sstream>
#include "io/ddb.hpp"

class DdbBFO : public CxxTest::TestSuite
{

  Ddb *ddb;

  public:

  DdbBFO() : CxxTest::TestSuite(), ddb(nullptr){}

  void setUp()
  {
#include "BFO_DDB.hxx"
    TS_ASSERT_THROWS_NOTHING(ddb=Ddb::getDdb("ref_BFO_DDB"));
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

    mat3d Bi = {{
      4.41592000,   0.00000000,   -0.00003000, 0.00004000,   4.41596000,   -0.00004000, -0.00000000,   0.00000000,   4.09208000
    }};
    mat3d Fe = {{                            
      3.94859000,   0.00000000,   0.00003000, -0.00006000,   3.94853000,   0.00002000, -0.00000000,   -0.00000000,   3.61067000
    }};
    mat3d O1 = {{
      -3.40953000,   0.00000000,   0.11571000, -0.00006000,   -2.16934000,   0.00004000, 0.29774000,   0.00000000,   -2.56819000
    }};
    mat3d O2 = {{
      -2.47936000,   0.53703000,   -0.05780000, 0.53705000,   -3.09950000,   0.10015000, -0.14887000,   0.25785000,   -2.56819000
    }};
    mat3d O3 = {{
      -2.47936000,   -0.53703000,   -0.05780000, -0.53710000,   -3.09955000,   -0.10005000, -0.14887000,   -0.25785000,   -2.56819000
    }};
    std::vector<mat3d> zeff;
    zeff.push_back(Bi);
    zeff.push_back(Bi);
    zeff.push_back(Fe);
    zeff.push_back(Fe);
    zeff.push_back(O1);
    zeff.push_back(O2);
    zeff.push_back(O3);
    zeff.push_back(O2);
    zeff.push_back(O3);
    zeff.push_back(O1);

    for ( unsigned iatom = 0 ; iatom < natom ; ++iatom ) {
      auto Z = ddb->getZeff(iatom);
      for ( unsigned i = 0 ; i < 9 ; ++i ) {
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
    using geometry::mat3d;
    using geometry::print;

    mat3d ref = {{
      7.24183300,   0.00000000,   0.00000000, 0.00000000,   7.24183300,   0.00000000, 0.00000000,   0.00000000,   6.57924200
    }};

    auto eps = ddb->getEpsInf();
    for ( unsigned i = 0 ; i < 9 ; ++i ) {
      std::ostringstream str;
      str << "indice " << i;
      TSM_ASSERT_DELTA(str.str().c_str(),eps[i],ref[i],1e-3);
    }
  }

};

