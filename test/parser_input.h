#include <cxxtest/TestSuite.h>
#include "io/configparser.hpp"
#include <array>

class ParserInput : public CxxTest::TestSuite
{
  public:
    void testString( void )
    {
      ConfigParser parser;
      std::string input("str1=toto str2 Tutu str3=titi\\ space");
      parser.setContent(input);
      TS_ASSERT(parser.getToken<std::string>("str1").compare("toto") == 0);
      TS_ASSERT(parser.getToken<std::string>("str2").compare("tutu") == 0);
      TS_ASSERT(parser.getToken<std::string>("str3").compare("titi space") == 0);
    }

    void testStringCase( void )
    {
      ConfigParser parser;
      std::string input("str1=toto str2 TOTO str3=TITI\\ space");
      parser.setContent(input);
      parser.setSensitive(false);
      TS_ASSERT(parser.getToken<std::string>("str1").compare("toto") == 0);
      TS_ASSERT(parser.getToken<std::string>("str2").compare("toto") == 0);
      TS_ASSERT(parser.getToken<std::string>("str3").compare("titi space") == 0);
      parser.setSensitive(true);
      TS_ASSERT(parser.getToken<std::string>("str1").compare("toto") == 0);
      TS_ASSERT(parser.getToken<std::string>("str2").compare("TOTO") == 0);
      TS_ASSERT(parser.getToken<std::string>("str3").compare("TITI space") == 0);
    }

    void testBool( void )
    {
      ConfigParser parser;
      std::string input("val1=0 val2=1 val3=true val4=false val5=qwe val6=123");
      parser.setContent(input);
      TS_ASSERT_EQUALS(parser.getToken<bool>("val1"),false);
      TS_ASSERT_EQUALS(parser.getToken<bool>("val2"),true);
      TS_ASSERT_EQUALS(parser.getToken<bool>("val3"),true);
      TS_ASSERT_EQUALS(parser.getToken<bool>("val4"),false);
      TS_ASSERT_THROWS(parser.getToken<bool>("val5"),Exception);
      TS_ASSERT_THROWS(parser.getToken<bool>("val6"),Exception);
    }

    void testVectorFloat( void )
    {
      ConfigParser parser;
      std::string input("v1 1.0 -3.12 6.234 asdfh");
      parser.setContent(input);
      std::array<double,3> v1({1.0,-3.12,6.234});
      TS_ASSERT_SAME_DATA(parser.getToken<double>("v1",3).data(),v1.data(),3);
      TS_ASSERT_THROWS(parser.getToken<double>("v1",4),Exception);
    }

    void testfloat( void )
    {
      ConfigParser parser;
      std::string input("f1 432.3 f2 asd");
      parser.setContent(input);
      TS_ASSERT_EQUALS(parser.getToken<double>("f1"),432.3);
      TS_ASSERT_THROWS(parser.getToken<double>("f2"),Exception);
    }

};

