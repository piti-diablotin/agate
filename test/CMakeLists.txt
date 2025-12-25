option(ENABLE_TESTS "Enable unit tests" OFF)

function(configure_test_target tgt)
  if(NOT TARGET ${tgt})
    message(FATAL_ERROR "Target '${tgt}' does not exist")
  endif()

  target_include_directories(${tgt}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/files
  )

  target_link_libraries(${tgt}
    PRIVATE
    CxxTest::CxxTest
    agate::agate
  )
endfunction()

if (ENABLE_TESTS)

  find_package(CxxTest REQUIRED)

  if(CxxTest_FOUND AND NOT TARGET CxxTest::CxxTest)
    add_library(CxxTest::CxxTest INTERFACE IMPORTED)
    set_target_properties(
      CxxTest::CxxTest
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CXXTEST_INCLUDE_DIRS}"
    )
  endif()

  # Add test:
  if(CxxTest_FOUND)
    enable_testing()

    set(Global_tests)


    cxxtest_add_test(
      ddb
      ddb.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ddb_bfo.h
      ${CMAKE_CURRENT_SOURCE_DIR}/ddb_crooh.h 
      ${CMAKE_CURRENT_SOURCE_DIR}/ddb_failures.h 
      ${CMAKE_CURRENT_SOURCE_DIR}/ddb_outcar.h 
      ${CMAKE_CURRENT_SOURCE_DIR}/ddb_pto.h 
      ${CMAKE_CURRENT_SOURCE_DIR}/ddb_wo3p21c.h 
      ${CMAKE_CURRENT_SOURCE_DIR}/ddb_ygo.h 
    )
    list(APPEND Global_tests ddb)

    cxxtest_add_test(
      dtset
      dtset.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/dtset_strain.h
    )
    list(APPEND Global_tests dtset)

    cxxtest_add_test(
      fraction
      fraction.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/fraction_conversion.h
      ${CMAKE_CURRENT_SOURCE_DIR}/fraction_static.h
    )
    list(APPEND Global_tests fraction)

    cxxtest_add_test(
      geometry
      geometry.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/geometry_mat3d.h
    )
    list(APPEND Global_tests geometry)

    cxxtest_add_test(
      h2o
      h2o.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/H2O_plot.h
    )
    list(APPEND Global_tests h2o)

    cxxtest_add_test(
      histcustommodes
      histcustommodes.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/histcustommodes_phonons.h
      ${CMAKE_CURRENT_SOURCE_DIR}/histcustommodes_strain.h
    )
    list(APPEND Global_tests histcustommodes)

    cxxtest_add_test(
      mapping
      mapping.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/mapping_supercell.h
    )
    list(APPEND Global_tests mapping)

    cxxtest_add_test(
      multibinit 
      multibinit.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/multibinit_thermalpop.h
    )
    list(APPEND Global_tests multibinit)

    cxxtest_add_test(
      parser
      parser.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/parser_input.h
    )
    list(APPEND Global_tests parser)

    cxxtest_add_test(
      sro
      sro.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/SRO_phonons.h
      ${CMAKE_CURRENT_SOURCE_DIR}/SRO_plot.h
    )
    list(APPEND Global_tests sro)

    cxxtest_add_test(
      supercell
      supercell.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/supercell_basic.h
    )
    list(APPEND Global_tests supercell)

    cxxtest_add_test(
      utils 
      utils.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/utils_strings.h
    )
    list(APPEND Global_tests utils)

    cxxtest_add_test(
      yno
      yno.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/YNO_phonons.h
    )
    list(APPEND Global_tests yno)

    foreach(tgt IN LISTS Global_tests)
      configure_test_target(${tgt})
    endforeach()
  endif()

endif()
