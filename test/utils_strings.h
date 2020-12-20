#include <cxxtest/TestSuite.h>
#include "base/utils.hpp"

class UtilsString : public CxxTest::TestSuite
{
  public:

    void testReadString( void ) {
      std::istringstream stream;
      stream.str("string\\ with\\ space");
      auto test = utils::readString(stream);
      TS_ASSERT_EQUALS(test,std::string("string with space"));
      stream.str("\"string with space\"");
      stream.clear();
      test = utils::readString(stream);
      TS_ASSERT_EQUALS(test,std::string("string with space"));
      stream.str("\"string\\ with space\"");
      stream.clear();
      test = utils::readString(stream);
      TS_ASSERT_EQUALS(test,std::string("string with space"));
      stream.str("\"string\\ with\" space\"");
      stream.clear();
      test = utils::readString(stream);
      TS_ASSERT_EQUALS(test,std::string("string with"));
      test = utils::readString(stream);
      TS_ASSERT_EQUALS(test,std::string("space\""));
    }

    void testBasename( void ) {
      std::string filename = "/toto/pouet/tutu/patata";
      auto base = utils::basename(filename);
      TS_ASSERT_EQUALS(base,std::string("patata"));
      filename = "/toto/pouet/tutu/patata/";
      base = utils::basename(filename);
      TS_ASSERT_EQUALS(base,std::string("patata"));
      filename = "patata/";
      base = utils::basename(filename);
      TS_ASSERT_EQUALS(base,std::string("patata"));
      filename = "patata";
      base = utils::basename(filename);
      TS_ASSERT_EQUALS(base,std::string("patata"));
    }

    void testDirname( void ) {
      std::string filename = "/toto/pouet/tutu/patata";
      auto dir = utils::dirname(filename);
      TS_ASSERT_EQUALS(dir,std::string("/toto/pouet/tutu"));
      filename = "/toto/pouet/tutu/patata/";
      dir = utils::dirname(filename);
      TS_ASSERT_EQUALS(dir,std::string("/toto/pouet/tutu"));
      filename = "patata/";
      dir = utils::dirname(filename);
      TS_ASSERT_EQUALS(dir,std::string("."));
      filename = "patata";
      dir = utils::dirname(filename);
      TS_ASSERT_EQUALS(dir,std::string("."));
    }

};

