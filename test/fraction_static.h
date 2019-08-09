#include <cxxtest/TestSuite.h>
#include "base/mendeleev.hpp"
#include "base/fraction.hpp"

Agate::mendeleev Agate::Mendeleev;

class FractionStatic : public CxxTest::TestSuite, public Fraction
{
  public:
    void testInverse( void )
    {
      TS_ASSERT_EQUALS(Fraction::inverse(1.), 1 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.5), 2 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.333333333), 3 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.25), 4 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.20), 5 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.166666666), 6 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.142857143), 7 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.125), 8 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.111111111), 9 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.111211111), 0 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.111211111), 0 );
      TS_ASSERT_EQUALS(Fraction::inverse(NAN), 0 );
      TS_ASSERT_EQUALS(Fraction::inverse(INFINITY), 0 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.), 0 );
    }

    void testDigits( void )
    {
      TS_ASSERT_EQUALS(Fraction::digits(0.5), 1 );
      TS_ASSERT_EQUALS(Fraction::digits(0.25), 2 );
      TS_ASSERT_EQUALS(Fraction::digits(0.20), 1 );
      TS_ASSERT_EQUALS(Fraction::digits(0.125), 3 );
      TS_ASSERT_EQUALS(Fraction::digits(0.3333333333), 10 );
      TS_ASSERT_EQUALS(Fraction::inverse(NAN), 0 );
      TS_ASSERT_EQUALS(Fraction::inverse(INFINITY), 0 );
      TS_ASSERT_EQUALS(Fraction::inverse(0.), 0 );
    }

    void testPgcd( void )
    {
      TS_ASSERT_EQUALS(Fraction::pgcd(1,3), 1 );
      TS_ASSERT_EQUALS(Fraction::pgcd(2,6), 2 );
      TS_ASSERT_EQUALS(Fraction::pgcd(15,10), 5 );
      TS_ASSERT_EQUALS(Fraction::pgcd(7,5), 1 );
      TS_ASSERT_EQUALS(Fraction::pgcd(42,30), 6 );
      TS_ASSERT_EQUALS(Fraction::pgcd(42,0), 0 );
    }

    void testNegativeInverse( void )
    {
      TS_ASSERT_EQUALS(Fraction::inverse(-1.), -1 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.5), -2 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.333333333), -3 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.25), -4 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.20), -5 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.166666666), -6 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.142857143), -7 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.125), -8 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.111111111), -9 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.111211111), 0 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.111211111), 0 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.), 0 );
    }

    void testNegativeDigits( void )
    {
      TS_ASSERT_EQUALS(Fraction::digits(-0.5), 1 );
      TS_ASSERT_EQUALS(Fraction::digits(-0.25), 2 );
      TS_ASSERT_EQUALS(Fraction::digits(-0.20), 1 );
      TS_ASSERT_EQUALS(Fraction::digits(-0.125), 3 );
      TS_ASSERT_EQUALS(Fraction::digits(-0.3333333333), 10 );
      TS_ASSERT_EQUALS(Fraction::inverse(-0.), 0 );
    }

    void testNegativePgcd( void )
    {
      TS_ASSERT_EQUALS(Fraction::pgcd(-1,3), 0 );
      TS_ASSERT_EQUALS(Fraction::pgcd(2,-6), 0 );
      TS_ASSERT_EQUALS(Fraction::pgcd(-15,-10), 0 );
    }

};

