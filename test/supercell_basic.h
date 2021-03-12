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

  void testMappingSRO( void )
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

  void testMappingCTO( void )
  {
    try {
      Dtset reference;
#include "files/CTO_Pnma.hxx"
#include "files/CTO_Pnma_444.hxx"
#include "files/CTO_444_mapping.hxx"
      reference.readFromFile("CTO_Pnma.in");
      HistData *hist = HistData::getHist("CTO_Pnma_444.in",true);
      Supercell sc(*hist,0);
      sc.findReference(reference);
      std::ifstream mapping("ref_CTO_444_mapping",std::ios::in);
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

  void testDisplacementNoStrain( void )
  {
    std::stringstream refstream;
    refstream << " acell  3*8\n";
    refstream << " natom  5\n";
    refstream << " ntypat 3\n";
    refstream << " typat  1 2 3 3 3\n";
    refstream << " znucl 38 44 8\n";
    refstream << " xcart 0 0 0\n";
    refstream << "       4 4 4\n";
    refstream << "       0 4 4\n";
    refstream << "       4 0 4\n";
    refstream << "       4 4 0\n";
    Dtset reference;
    ConfigParser parser;
    std::cout << refstream.str() << std::endl;
    parser.setContent(refstream.str());
    reference.readConfig(parser);
    Supercell test1;
    refstream.str("");
    refstream.clear();
    refstream << " acell  3*8\n";
    refstream << " natom  5\n";
    refstream << " ntypat 3\n";
    refstream << " typat  1 2 3 3 3\n";
    refstream << " znucl 38 44 8\n";
    refstream << " xcart 0 0 0\n";
    refstream << "       4 4 5\n";
    refstream << "       0 3 4\n";
    refstream << "       4 0 4\n";
    refstream << "       -4 -4 0\n";
    std::cout << refstream.str() << std::endl;
    parser.setContent(refstream.str());
    test1.readConfig(parser);
    test1.findReference(reference);
    auto disp = test1.getDisplacement(reference,false);
    std::vector<double> result(15,0);
    /*result[0] = 0; result[1] = 0; result[2] = 0;*/
    /*result[3] = 0; result[4] = 0;*/ result[5] = 1;
    /*result[6] = 0;*/ result[7] = -1; /*result[8] = 0;*/
    /*result[9] = 0; result[10] = 0; result[11] = 0;*/
    /*result[12] = 0; result[13] = 0; result[14] = 0;*/
    for (int i=0; i<disp.size();++i ) 
        TS_ASSERT_DELTA(disp[i],result[i],1e-13);
  }

  void testDisplacementStrainNiDisp( void )
  {
    std::stringstream refstream;
    refstream << " acell  3*8\n";
    refstream << " natom  5\n";
    refstream << " ntypat 3\n";
    refstream << " typat  1 2 3 3 3\n";
    refstream << " znucl 38 44 8\n";
    refstream << " xcart 0 0 0\n";
    refstream << "       4 4 4\n";
    refstream << "       0 4 4\n";
    refstream << "       4 0 4\n";
    refstream << "       4 4 0\n";
    Dtset reference;
    ConfigParser parser;
    std::cout << refstream.str() << std::endl;
    parser.setContent(refstream.str());
    reference.readConfig(parser);
    Supercell test1;
    refstream.str("");
    refstream.clear();
    refstream << " acell  2*8 8.5\n";
    refstream << " natom  5\n";
    refstream << " ntypat 3\n";
    refstream << " typat  1 2 3 3 3\n";
    refstream << " znucl 38 44 8\n";
    refstream << " xred 0 0 0\n";
    refstream << "       0.5 0.5 0.5\n";
    refstream << "       0 0.5 0.5\n";
    refstream << "       0.5 0 0.5\n";
    refstream << "       0.5 0.5 0\n";
    std::cout << refstream.str() << std::endl;
    parser.setContent(refstream.str());
    test1.readConfig(parser);
    test1.findReference(reference);
    auto disp = test1.getDisplacement(reference,false);
    std::vector<double> result(15,0);
    for (int i=0; i<disp.size();++i ) 
        TS_ASSERT_DELTA(disp[i],result[i],1e-13);
  }

  void testDisplacementStrainDisp( void )
  {
    std::stringstream refstream;
    refstream << " acell  3*8\n";
    refstream << " natom  5\n";
    refstream << " ntypat 3\n";
    refstream << " typat  1 2 3 3 3\n";
    refstream << " znucl 38 44 8\n";
    refstream << " xcart 0 0 0\n";
    refstream << "       4 4 4\n";
    refstream << "       0 4 4\n";
    refstream << "       4 0 4\n";
    refstream << "       4 4 0\n";
    Dtset reference;
    ConfigParser parser;
    std::cout << refstream.str() << std::endl;
    parser.setContent(refstream.str());
    reference.readConfig(parser);
    Supercell test1;
    refstream.str("");
    refstream.clear();
    refstream << " acell  2*8 8.5\n";
    refstream << " natom  5\n";
    refstream << " ntypat 3\n";
    refstream << " typat  1 2 3 3 3\n";
    refstream << " znucl 38 44 8\n";
    refstream << " xcart 0 0 0\n";
    refstream << "       4 4 5.25\n";
    refstream << "       0 3 4.25\n";
    refstream << "       4 0 4.25\n";
    refstream << "       4 4 0\n";
    std::cout << refstream.str() << std::endl;
    parser.setContent(refstream.str());
    test1.readConfig(parser);
    test1.findReference(reference);
    auto disp = test1.getDisplacement(reference,false);
    std::vector<double> result(15,0);
    /*result[0] = 0; result[1] = 0; result[2] = 0;*/
    /*result[3] = 0; result[4] = 0;*/ result[5] = 1;
    /*result[6] = 0;*/ result[7] = -1; /*result[8] = 0;*/
    /*result[9] = 0; result[10] = 0; result[11] = 0;*/
    /*result[12] = 0; result[13] = 0; result[14] = 0;*/
    for (int i=0; i<disp.size();++i ) 
        TS_ASSERT_DELTA(disp[i],result[i],1e-13);
  }

};

