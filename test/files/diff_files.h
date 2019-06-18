#define ABSTOL 1e-6
#define RELTOL 1e-2
#define COMPREL 1e-10
#define DIFF_FILES(fref,fnew) { \
  TS_ASSERT(fref.good()); \
  TS_ASSERT(fnew.good()); \
  std::string tmp; \
  std::getline(fref,tmp); \
  std::getline(fnew,tmp); \
  while ( !fref.eof() && !fnew.eof() )  \
  { \
    using std::isnan; \
    double ref, value; \
    fref >> ref; \
    fnew >> value; \
    std::stringstream message; \
    message << "ref " << ref << " value " << value; \
    if ( std::isnan(ref) ) \
      TSM_ASSERT_IS_NAN(message.str().c_str(),value) \
    if ( !std::isnan((ref-value)/ref) && ref > COMPREL ) \
      TSM_ASSERT_LESS_THAN(message.str().c_str(),(ref-value)/ref,RELTOL); \
    TSM_ASSERT_DELTA(message.str().c_str(),ref,value,ABSTOL); \
  } \
  TS_ASSERT(fref.eof()&&fnew.eof()); \
}
