#define DIFF_FILES(fref,fnew) { \
  TS_ASSERT(fref.good()); \
  TS_ASSERT(fnew.good()); \
  std::string tmp; \
  std::getline(fref,tmp); \
  std::getline(fnew,tmp); \
  while ( !fref.eof() && !fnew.eof() )  \
  { \
    double ref, value; \
    fref >> ref; \
    fnew >> value; \
    TS_ASSERT_DELTA(ref,value,1e-14); \
  } \
  TS_ASSERT(fref.eof()&&fnew.eof()); \
}
