#include <cxxtest/TestSuite.h>
#include <sstream>
#include "io/ddb.hpp"
#include "phonons/dispdb.hpp"
#include "base/unitconverter.hpp"

class DdbOUTCAR : public CxxTest::TestSuite
{

  public:

  void testOutcar( void )
  {
#include "OUTCAR.hxx"
    Ddb* ddb = nullptr;
    try{
      ddb = Ddb::getDdb("ref_OUTCAR");
      DispDB db;
      db.computeFromDDB(*ddb);
      db.setQpt({0,0,0});
      UnitConverter eunit(UnitConverter::Ha);
      eunit = UnitConverter::pcm;
      for ( unsigned i = 0 ; i < 3*ddb->natom() ; ++i ) {
        std::cerr << "Mode " << (i+1) << ": "
          << (db.getEnergyMode(i)*eunit) << "cm-1" << std::endl;
      }
    }
    catch( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
    }
    delete ddb;
  }

};

