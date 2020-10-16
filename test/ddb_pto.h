#include <cxxtest/TestSuite.h>
#include "io/ddb.hpp"

class DdbPTO : public CxxTest::TestSuite
{

  Ddb *ddb;

  public:

  DdbPTO() : CxxTest::TestSuite(), ddb(nullptr){}

  void setUp()
  {
#include "PTO_DDB.hxx"
    TS_ASSERT_THROWS_NOTHING(ddb=Ddb::getDdb("ref_PTO_DDB"));
  }

  void tearDown() {
    if ( ddb != nullptr ) delete ddb;
  }

  void testData( void )
  {
    TS_ASSERT_DIFFERS(ddb,nullptr);
    std::cerr << ddb->info();
    auto qpts = ddb->getQpts();
    TS_ASSERT_EQUALS(qpts.size(),10);
  }

  void testZeff( void ) 
  {
    TS_ASSERT_DIFFERS(ddb,nullptr);
    unsigned natom = ddb->natom();
    TS_ASSERT_EQUALS(natom,5);
    using geometry::mat3d;

    const double ZPb = 3.8964;
    const double ZTi = 7.1908;
    const double ZOpa = -5.9090;
    const double ZOpe = -2.5907;

    mat3d Z0 = {ZPb,0,0,0,ZPb,0,0,0,ZPb};
    mat3d Z1 = {ZTi,0,0,0,ZTi,0,0,0,ZTi};
    mat3d Z2 = {ZOpe,0,0,0,ZOpa,0,0,0,ZOpe};
    mat3d Z3 = {ZOpa,0,0,0,ZOpe,0,0,0,ZOpe};
    mat3d Z4 = {ZOpe,0,0,0,ZOpe,0,0,0,ZOpa};
    std::vector<mat3d> Zeff;
    Zeff.push_back(Z0);
    Zeff.push_back(Z1);
    Zeff.push_back(Z2);
    Zeff.push_back(Z3);
    Zeff.push_back(Z4);

    for ( unsigned iatom = 0 ; iatom < natom ; ++iatom ) {
      auto Z = ddb->getZeff(iatom);
      for ( unsigned i = 0 ; i < 9 ; ++i )
        TS_ASSERT_DELTA(Zeff[iatom][i],Z[i],1e-4);
    }
  }

  void testEpsInf( void ) 
  {
    TS_ASSERT_DIFFERS(ddb,nullptr);
    using geometry::mat3d;
    using geometry::print;

    mat3d ref = {{
      8.5256,     0.000000,     0.000000,     0.000000,     8.5256,     0.000000,      0.000000,     0.000000,   8.5256  
    }};

    auto eps = ddb->getEpsInf();
    for ( unsigned i = 0 ; i < 9 ; ++i ) {
      std::ostringstream str;
      str << "indice " << i;
      TSM_ASSERT_DELTA(str.str().c_str(),eps[i],ref[i],1e-3);
    }
  }

};

