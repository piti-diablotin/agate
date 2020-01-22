#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <cxxtest/TestSuite.h>
#include "phonons/supercell.hpp"
#include "base/utils.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "diff_files.h"

class SupercellBasic : public CxxTest::TestSuite
{

  public:

  void setUp()
  {
#include "files/YNO_DDB.hxx"
#include "files/SRO_DDB.hxx"
  }

  void testConstructor( void )
  {
    try {
      Dtset dtset;
      Dtset ref1;
      Dtset ref2;
      Dtset ref3;
#include "files/Supercell124.hxx"
#include "files/Supercell222.hxx"
#include "files/Supercell444.hxx"
      geometry::vec3d qpt1 = {{0,0.5,0.25}};
      geometry::vec3d qpt2 = {{0.5,0.5,0.5}};
      geometry::vec3d qpt3 = {{0.25,.25,0.25}};
      geometry::vec3d qpt4 = {{-0.25,0.25,0.25}};

      dtset.readFromFile("ref_YNO_DDB");
      ref1.readFromFile("ref_Supercell124.in");
      ref2.readFromFile("ref_Supercell222.in");
      ref3.readFromFile("ref_Supercell444.in");

      Supercell supercell1(dtset,qpt1);
      Supercell supercell2(dtset,qpt2);
      Supercell supercell3(dtset,qpt3);
      Supercell supercell4(dtset,qpt4);
    
      std::clog << (ref1 == supercell1) << std::endl;
      TS_ASSERT_EQUALS(ref1,(Dtset)supercell1);
      TS_ASSERT_EQUALS(ref2,(Dtset)supercell2);
      TS_ASSERT_EQUALS(ref3,(Dtset)supercell3);
      TS_ASSERT_EQUALS(ref3,(Dtset)supercell4);

    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to build Supercell");
    }
  }

  void testMapping( void )
  {
    try {
      Dtset reference;
      reference.readFromFile("SRO_DDB");
#include "files/SRO_222.hxx"
#include "files/SRO_222_mapping.hxx"
      HistData *hist = HistData::getHist("ref_SRO_222",true);
      Supercell sc(*hist,0);
      sc.findReference(reference);
      std::ifstream mapping("ref_SRO_222_mapping",std::ios::in);
      if ( !mapping ) TS_FAIL("no ref file");
      for ( unsigned i = 0 ; i < sc.natom() ; ++i ) {
        int aref, x,y,z;
        int refi, refaref, refx,refy,refz;
        sc.getRefCoord(i,aref,x,y,z);
        mapping >> refi >> refaref >> refx >> refy >> refz;
        std::stringstream str;
        str << "Testing atom " << i << "of supercell";
        TSM_ASSERT_EQUALS(str.str().c_str(),refi,i);
        TSM_ASSERT_EQUALS(str.str().c_str(),refaref,aref);
        TSM_ASSERT_EQUALS(str.str().c_str(),refx,x);
        TSM_ASSERT_EQUALS(str.str().c_str(),refy,y);
        TSM_ASSERT_EQUALS(str.str().c_str(),refz,z);
      }
      mapping.close();
    }
    catch ( Exception &e ) {
      std::cerr << e.fullWhat() << std::endl;
      TS_FAIL("Unable to test mapping");
    }
  }

};

