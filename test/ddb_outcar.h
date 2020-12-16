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
      const double freq_ref[] = {
        -0.263687,
        -0.263687,
        -0.263687,
        39.7169,
        39.7169,
        39.7169,
        170.367,
        170.367,
        170.367,
        257.701,
        257.701,
        257.701,
        514.816,
        514.816,
        514.816
      };
      for ( unsigned i = 0 ; i < 3*ddb->natom() ; ++i ) {
        std::cerr << "Mode " << (i+1) << ": "
          << (db.getEnergyMode(i)*eunit) << "cm-1" << std::endl;
        TS_ASSERT_DELTA(freq_ref[i],db.getEnergyMode(i)*eunit,1e-3);
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
      const double freq_ref[] = {
        -91.5483,
        -91.537,
        -91.5306,
        0.330194,
        0.336894,
        0.355442,
        152.313,
        152.316,
        152.317,
        229.812,
        229.817,
        229.827,
        523.727,
        523.73,
        523.734
      };

      for ( unsigned i = 0 ; i < 3*ddb->natom() ; ++i ) {
        std::cerr << "Mode " << (i+1) << ": "
          << (db.getEnergyMode(i)*eunit) << "cm-1" << std::endl;
        TS_ASSERT_DELTA(freq_ref[i],db.getEnergyMode(i)*eunit,1e-3);
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

  void testOutcarBatch( void )
  {
#include "OUTCAR1.hxx"
#include "OUTCAR2.hxx"
#include "OUTCAR3.hxx"
#include "OUTCAR4.hxx"
#include "OUTCAR5.hxx"
    std::vector<std::string> files(5);
    files[0] = "ref_OUTCAR1";
    files[1] = "ref_OUTCAR2";
    files[2] = "ref_OUTCAR3";
    files[3] = "ref_OUTCAR4";
    files[4] = "ref_OUTCAR5";
    for ( auto file : files ) {
      Ddb* ddbV = nullptr;
      Ddb* ddbA = nullptr;
      std::cerr << "Testing file " << file << std::endl;
      try{
        using geometry::mat3d;
        ddbV = Ddb::getDdb(file);
        DdbAbinit::dump(*ddbV,file+"_DDB");
        ddbA = Ddb::getDdb(file+"_DDB");

        DispDB dbV;
        dbV.computeFromDDB(*ddbV);
        dbV.setQpt({0,0,0});

        DispDB dbA;
        dbA.computeFromDDB(*ddbA);
        dbA.setQpt({0,0,0});

        UnitConverter eunit(UnitConverter::Ha);
        eunit = UnitConverter::pcm;

        for ( unsigned i = 0 ; i < 3*ddbV->natom() ; ++i ) {
          std::cerr << "Mode " << (i+1) << ": "
            << (dbV.getEnergyMode(i)*eunit) << "  " << (dbA.getEnergyMode(i)*eunit) << "cm-1" << std::endl;
          std::ostringstream str;
          str << "Mode " << (i+1) << ":";
          TSM_ASSERT_DELTA(str.str().c_str(),dbV.getEnergyMode(i)*eunit,dbA.getEnergyMode(i)*eunit,1e-3);
        }

        try {
          auto epsA = ddbA->getEpsInf();
          auto epsV = ddbV->getEpsInf();
          for ( auto i = 0 ; i < 9 ; ++i ) {
            std::ostringstream str;
            str << "indice " << i;
            TSM_ASSERT_DELTA(str.str().c_str(),epsV[i],epsA[i],1e-3);
          }
        }
        catch ( Exception &e ) {
          TS_WARN("No Eps inf for this file");
        }

        try {
          for ( unsigned iatom = 0 ; iatom < ddbA->natom() ; ++iatom ) {
            auto ZV = ddbV->getZeff(iatom);
            auto ZA = ddbA->getZeff(iatom);
            for ( auto i = 0 ; i < 9 ; ++i ) {
              std::ostringstream str;
              str << "Atom " << (iatom+1) << " ";
              str << "indice " << i;
              TSM_ASSERT_DELTA(str.str().c_str(),ZV[i],ZA[i],1e-3);
            }
          }
        }
        catch ( Exception &e ) {
          TS_WARN("No Zeff inf for this file");
        }

      }
      catch( Exception &e ) {
        std::cerr << e.fullWhat() << std::endl;
        TS_FAIL("Exception catch");
      }
      if ( ddbV != nullptr ) delete ddbV;
      if ( ddbA != nullptr ) delete ddbA;
    }
  }
};

