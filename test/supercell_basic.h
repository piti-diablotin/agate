#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <cxxtest/TestSuite.h>
#include "base/mendeleev.hpp"
#include "phonons/supercell.hpp"
#include "base/utils.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "diff_files.h"

Agate::mendeleev Agate::Mendeleev;

class SupercellBasic : public CxxTest::TestSuite
{

  public:

  void testConstructor( void )
  {
    try {
      Dtset dtset;
      Dtset ref1;
      Dtset ref2;
      Dtset ref3;
#include "files/YNO_DDB.hxx"
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

};

