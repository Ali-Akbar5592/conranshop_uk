#site library
add_library(conranshop_uk_lib SHARED
     spiders/conranshop_uk/src/conranshop_ukspider.cpp
     spiders/conranshop_uk/src/conranshop_ukparser.cpp
     spiders/conranshop_uk/src/conranshop_uksearch.cpp
)

target_link_libraries(conranshop_uk_lib
    profiler
    crawlercommon
    ${Boost_LIBRARIES}
    ${LOG4CXX_LIBRARIES}
)

# test case
add_executable(conranshop_ukparser_test
    spiders/conranshop_uk/tests/conranshop_ukparser_test.cpp
    spiders/conranshop_uk/src/conranshop_ukparser.cpp
    
)

target_link_libraries(conranshop_ukparser_test
    crawlercommon
    ${Boost_LIBRARIES}
    profiler
    testutils
    ${LOG4CXX_LIBRARIES}
)

set_target_properties (conranshop_ukparser_test
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)


