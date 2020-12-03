#include <cxxtest/TestSuite.h>
#include <sstream>
#include "io/ddb.hpp"
#include "io/ddbabinit.hpp"
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
      TS_FAIL("Exception catch");
    }
    delete ddb;
  }
  void testOutcarSTO( void )
  {
#include "OUTCAR_STO.hxx"
    Ddb* ddb = nullptr;
    try{
      using geometry::mat3d;
      ddb = Ddb::getDdb("ref_OUTCAR_STO");
      DispDB db;
      db.computeFromDDB(*ddb);
      db.setQpt({0,0,0});
      UnitConverter eunit(UnitConverter::Ha);
      eunit = UnitConverter::pcm;
      const double ZSr = 2.54894;
      const double ZTi = 7.35194;
      const double ZOpa = -5.82924;
      const double ZOpe = -2.03583;

      mat3d Z0 = {ZSr,0,0,0,ZSr,0,0,0,ZSr};
      mat3d Z1 = {ZTi,0,0,0,ZTi,0,0,0,ZTi};
      mat3d Z2 = {ZOpa,0,0,0,ZOpe,0,0,0,ZOpe};
      mat3d Z3 = {ZOpe,0,0,0,ZOpa,0,0,0,ZOpe};
      mat3d Z4 = {ZOpe,0,0,0,ZOpe,0,0,0,ZOpa};
      std::vector<mat3d> Zeff;
      Zeff.push_back(Z0);
      Zeff.push_back(Z1);
      Zeff.push_back(Z2);
      Zeff.push_back(Z3);
      Zeff.push_back(Z4);
      for ( unsigned i = 0 ; i < 3*ddb->natom() ; ++i ) {
        std::cerr << "Mode " << (i+1) << ": "
          << (db.getEnergyMode(i)*eunit) << "cm-1" << std::endl;
      }
      for ( unsigned iatom = 0 ; iatom < ddb->natom() ; ++iatom ) {
        auto Z = ddb->getZeff(iatom);
        for ( unsigned i = 0 ; i < 9 ; ++i )
          TS_ASSERT_DELTA(Zeff[iatom][i],Z[i],1e-4);
      }
      mat3d epsinf = {6.363933,0,0, 0,6.363933,0, 0,0,6.363933};
      auto E = ddb->getEpsInf();
      for ( unsigned i = 0 ; i < 9 ; ++i )
          TS_ASSERT_DELTA(E[i],epsinf[i],1e-4);

      DdbAbinit::dump(*ddb,"STOfromOUTCAR_DDB");
    }
    catch( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Exception catch");
    }
    delete ddb;
  }

};

