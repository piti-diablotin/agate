#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <cxxtest/TestSuite.h>
#include "hist/histdata.hpp"
#include "phonons/supercell.hpp"
#include <algorithm>
#include "phonons/dispdb.hpp"
#include "io/ddbabinit.hpp"

class MappingSupercell : public CxxTest::TestSuite
{

  public:

    void testFindReference() {
#include "WO3_DDB.hxx"
#include "WO3_DIST.hxx"
#include "WO3_DIST_SHIFTED.hxx"
      Dtset ddb;
      Supercell dist;
      Supercell dist_shifted;
      TS_ASSERT_THROWS_NOTHING(ddb.readFromFile("ref_WO3_DDB"));
      TS_ASSERT_THROWS_NOTHING(dist.setCif("ref_WO3_DIST"));
      TS_ASSERT_THROWS_NOTHING(dist.reBuildStructure(0.0001,false));
      TS_ASSERT_THROWS_NOTHING(dist_shifted.readFromFile("ref_WO3_DIST_SHIFTED"));
      TS_ASSERT_THROWS_NOTHING(dist.findReference(ddb));
      TS_ASSERT_THROWS_NOTHING(dist_shifted.findReference(ddb));
    }

    void testFindQpt() {
#include "WO3_DDB.hxx"
#include "WO3_DIST.hxx"
#include "WO3_DIST_SHIFTED.hxx"
      DdbAbinit ddb;
      Supercell dist;
      Supercell dist_shifted;
      TS_ASSERT_THROWS_NOTHING(ddb.readFromFile("ref_WO3_DDB"));
      TS_ASSERT_THROWS_NOTHING(dist.setCif("ref_WO3_DIST"));
      TS_ASSERT_THROWS_NOTHING(dist.reBuildStructure(0.0001,false));
      TS_ASSERT_THROWS_NOTHING(dist_shifted.readFromFile("ref_WO3_DIST_SHIFTED"));
      TS_ASSERT_THROWS_NOTHING(dist.findReference(ddb));
      TS_ASSERT_THROWS_NOTHING(dist_shifted.findReference(ddb));
      auto vec1 = dist.amplitudes(ddb, dist.getDisplacement(ddb));
      auto vec2 = dist_shifted.amplitudes(ddb, dist_shifted.getDisplacement(ddb));
      for ( unsigned int i = 0 ; i < vec1.size() ; ++i ) {
        TS_ASSERT_DELTA(vec1[i][3],vec2[i][3],1e-13);
      }
    }

    void testProjectOnModes() {
#include "WO3_DDB.hxx"
#include "WO3_DIST.hxx"
#include "WO3_DIST_SHIFTED.hxx"
      DdbAbinit ddb;
      Supercell dist;
      Supercell dist_shifted;
      TS_ASSERT_THROWS_NOTHING(ddb.readFromFile("ref_WO3_DDB"));
      TS_ASSERT_THROWS_NOTHING(dist.setCif("ref_WO3_DIST"));
      TS_ASSERT_THROWS_NOTHING(dist.reBuildStructure(0.0001,false));
      TS_ASSERT_THROWS_NOTHING(dist_shifted.readFromFile("ref_WO3_DIST_SHIFTED"));
      TS_ASSERT_THROWS_NOTHING(dist.findReference(ddb));
      TS_ASSERT_THROWS_NOTHING(dist_shifted.findReference(ddb));

      DispDB disp(ddb.natom());
      TS_ASSERT_THROWS_NOTHING(disp.computeFromDDB(ddb));
      disp.setQpt({{0,0,0}});
      DispDB::qptTree condensed;
      auto it = condensed.insert(
          std::pair<geometry::vec3d,std::vector<DispDB::qMode>>(
            {{0,0,0}},
            std::vector<DispDB::qMode>()
            )
          );

      for ( unsigned vib = 0 ; vib < ddb.natom()*3 ; ++vib ) {
        double nrj = disp.getEnergyMode(vib);
        DispDB::qMode vibnrj = {vib,1,nrj};
        it.first->second.push_back(vibnrj);
      }

      auto vec1 = dist.projectOnModes(ddb, disp, condensed, Supercell::NONE);
      auto vec2 = dist_shifted.projectOnModes(ddb, disp, condensed, Supercell::NONE);
      for ( unsigned int i = 0 ; i < vec1.size() ; ++i ) {
        TS_ASSERT_DELTA(vec1[i],vec2[i],1e-13);
      }

      vec1 = dist.projectOnModes(ddb, disp, condensed, Supercell::NORMALL);
      vec2 = dist_shifted.projectOnModes(ddb, disp, condensed, Supercell::NORMALL);
      for ( unsigned int i = 0 ; i < vec1.size() ; ++i ) {
        TS_ASSERT_DELTA(vec1[i],vec2[i],1e-13);
      }
    }
};

