#include <cxxtest/TestSuite.h>
#include <sstream>
#include "io/ddb.hpp"

class DdbFailures : public CxxTest::TestSuite
{

  public:

  void testYNO( void )
  {
    Ddb* ddb = nullptr;
#include "YNO_DDB.hxx"
    TS_ASSERT_THROWS_NOTHING(ddb=Ddb::getDdb("ref_YNO_DDB"));
    TS_ASSERT_THROWS(ddb->getZeff(0),Exception);
    TS_ASSERT_THROWS(ddb->getEpsInf(),Exception);
    delete ddb;
  }

  void testSRO( void )
  {
    Ddb* ddb = nullptr;
#include "SRO_DDB.hxx"
    TS_ASSERT_THROWS_NOTHING(ddb=Ddb::getDdb("SRO_DDB"));
    TS_ASSERT_THROWS(ddb->getZeff(0),Exception);
    TS_ASSERT_THROWS(ddb->getEpsInf(),Exception);
    delete ddb;
  }


};

