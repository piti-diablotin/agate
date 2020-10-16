#include <cxxtest/TestSuite.h>
#include "base/fraction.hpp"
#include <iostream>
#include <sstream>

class FractionConversion : public CxxTest::TestSuite
{
  public:
    void testCompute( void )
    {
      Fraction frac(0.5);
      TS_ASSERT_EQUALS(frac.toString(), std::string("1/2") );
      frac = Fraction(0.25);
      TS_ASSERT_EQUALS(frac.toString(), std::string("1/4") );
      frac = Fraction(0.3333333333);
      TS_ASSERT_EQUALS(frac.toString(), std::string("1/3") );
      frac = Fraction(0.75);
      TS_ASSERT_EQUALS(frac.toString(), std::string("3/4") );
      frac = Fraction(0.625);
      TS_ASSERT_EQUALS(frac.toString(), std::string("5/8") );
      frac = Fraction(0.6666666666666);
      TS_ASSERT_EQUALS(frac.toString(), std::string("2/3") );
      frac = Fraction(1);
      TS_ASSERT_EQUALS(frac.toString(), std::string("1") );
      frac = Fraction(4);
      TS_ASSERT_EQUALS(frac.toString(), std::string("4") );
      frac = Fraction(7);
      TS_ASSERT_EQUALS(frac.toString(), std::string("7") );
      frac = Fraction(0);
      TS_ASSERT_EQUALS(frac.toString(), std::string("0") );
      frac = Fraction(0.08571428571428569);
      TS_ASSERT_EQUALS(frac.toString(), std::string("0.0857143") );
    }
    void testNegativeCompute( void )
    {
      Fraction frac(-0.5);
      TS_ASSERT_EQUALS(frac.toString(), std::string("-1/2") );
      frac = Fraction(-0.3333333333);
      TS_ASSERT_EQUALS(frac.toString(), std::string("-1/3") );
      frac = Fraction(-0.75);
      TS_ASSERT_EQUALS(frac.toString(), std::string("-3/4") );
      frac = Fraction(-0.625);
      TS_ASSERT_EQUALS(frac.toString(), std::string("-5/8") );
      frac = Fraction(-0.6666666666666);
      TS_ASSERT_EQUALS(frac.toString(), std::string("-2/3") );
      frac = Fraction(-1);
      TS_ASSERT_EQUALS(frac.toString(), std::string("-1") );
      frac = Fraction(-4);
      TS_ASSERT_EQUALS(frac.toString(), std::string("-4") );
      frac = Fraction(-7);
      TS_ASSERT_EQUALS(frac.toString(), std::string("-7") );
      frac = Fraction(-0);
      TS_ASSERT_EQUALS(frac.toString(), std::string("0") );
    }

    void testString( void ) {
      std::cout << Fraction(0.5) << std::endl;
      std::ostringstream str;
      str << Fraction(-0.125);
      const char verif[] = "-1/8";
      TS_ASSERT_SAME_DATA(verif,str.str().c_str(),str.str().size());
    }

};

