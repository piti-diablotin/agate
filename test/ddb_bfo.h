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
    using geometry::mat3d, geometry::print;

    mat3d Bi = {{
      4.406307E+00,   1.317481E-02,   3.594771E-02,
        1.318648E-02,   4.397943E+00,  -4.918106E-02,
        3.599125E-02,  -4.914919E-02,   4.119710E+00
    }};
    mat3d Fe = {{                            
      3.938492E+00,   1.372952E-02,   3.757207E-02,
        1.371940E-02,   3.929795E+00,  -5.126883E-02,
        3.753431E-02,  -5.129648E-02,   3.639503E+00,
    }};
    mat3d O1 = {{
      -3.072593E+00,   3.095837E-01,  -3.839974E-02,
        3.136070E-01,  -2.513018E+00,   9.454160E-02,
        1.133147E-01,   2.677272E-02,  -2.561444E+00
    }};
    mat3d O2 = {{
      -2.421952E+00,   6.982654E-02,   9.230470E-02,
        1.001973E-01,  -3.108092E+00,   1.632028E-01,
        6.895069E-02,   3.249380E-01,  -2.617011E+00
    }};
    mat3d O3 = {{
      -2.854041E+00,  -4.064045E-01,  -1.275265E-01,
        -4.408389E-01,  -2.710319E+00,  -1.569081E-01,
        -2.560374E-01,  -2.509847E-01,  -2.582740E+00
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

    TS_SKIP("Not working yet");
    for ( unsigned iatom = 0 ; iatom < natom ; ++iatom ) {
      auto Z = ddb->getZeff(iatom);
      std::ostringstream str;
      str << "Atom " << (iatom+1) << " ";
      for ( unsigned i = 0 ; i < 9 ; ++i ) {
        str << "indice " << i;
        TSM_ASSERT_DELTA(str.str().c_str(),Z[i],zeff[iatom][i],1e-3);
      }
    }
  }

};

