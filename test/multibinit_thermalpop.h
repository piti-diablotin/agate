#include <cxxtest/TestSuite.h>
#include <sstream>
#include "canvas/canvasphonons.hpp"

class ThermalPop : public CxxTest::TestSuite
{

  public:

  ThermalPop() : CxxTest::TestSuite() {}

  void setUp()
  {
  }

  void tearDown() {
  }

  void testPhononsOnly( void )
  {
#include "PTO_DDB.hxx"
    CanvasPhonons canvas(false);
    canvas.openFile("ref_PTO_DDB");
    std::string token = "thermalPop";
    std::istringstream stream("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1");
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
  }

  void testStrainOnly( void )
  {
#include "PTO_DDB.hxx"
    CanvasPhonons canvas(false);
    canvas.openFile("ref_PTO_DDB");
    std::string token = "thermalPop";
    std::istringstream stream("qpt=2 2 2 temperature=0 seedtype=user seed=1 ntime=1 iso=0.01");
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=0 seedtype=user seed=1 ntime=1 tetra=-0.02:0.01,x");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=0 seedtype=user seed=1 ntime=1 shear=-0.02:0.01");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
  }


  void testPhononsStrainIso( void )
  {
#include "PTO_DDB.hxx"
    CanvasPhonons canvas(false);
    canvas.openFile("ref_PTO_DDB");
    std::string token = "thermalPop";
    std::istringstream stream("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 iso=0.01");
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
  }

  void testPhononsStrainTetra( void )
  {
#include "PTO_DDB.hxx"
    CanvasPhonons canvas(false);
    canvas.openFile("ref_PTO_DDB");
    std::string token = "thermalPop";
    std::istringstream stream("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 tetra=0.01");
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 tetra=-0.02:0.01");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 tetra=-0.02:0.01,x");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 tetra=-0.02:0.01,x,y");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 tetra=-0.02:0.01,x,y,z");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
  }

  void testPhononsStrainShear( void )
  {
#include "PTO_DDB.hxx"
    CanvasPhonons canvas(false);
    canvas.openFile("ref_PTO_DDB");
    std::string token = "thermalPop";
    std::istringstream stream("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 shear=0.01");
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 shear=-0.02:0.01");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 shear=-0.02:0.01,xy");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 shear=-0.02:0.01,yx,zx");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 shear=-0.02:0.01,zy,yx,xz");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
  }

  void testPhononsStrain( void )
  {
#include "PTO_DDB.hxx"
    CanvasPhonons canvas(false);
    canvas.openFile("ref_PTO_DDB");
    std::string token = "thermalPop";
    std::istringstream stream("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 shear=0.01");
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 shear=-0.02:0.01");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 iso=0.01 shear=-0.02:0.01");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.str("qpt=2 2 2 temperature=300 seedtype=user seed=1 ntime=1 iso=0.01 tetra=0.004,x,y,z shear=-0.02:0.01");
    stream.clear();
    stream.seekg(0);
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
  }

  void testDistrib( void )
  {
#include "PTO_DDB.hxx"
    CanvasPhonons canvas(false);
    canvas.openFile("ref_PTO_DDB");
    std::string token = "thermalPop";
    std::istringstream stream("qpt=2 2 2 distribution=uniform temperature=300 seedtype=user seed=1 ntime=1 shear=0.01");
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.clear();
    stream.seekg(0);
    stream.str("qpt=2 2 2 distribution=normal temperature=300 seedtype=user seed=1 ntime=1 shear=0.01");
    TS_ASSERT_THROWS_NOTHING(canvas.alter(token,stream);)
    stream.clear();
    stream.seekg(0);
    stream.str("qpt=2 2 2 distribution=toto temperature=300 seedtype=user seed=1 ntime=1 shear=0.01");
    TS_ASSERT_THROWS_ANYTHING(canvas.alter(token,stream);)
  }

};

