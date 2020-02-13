cmake_minimum_required(VERSION 2.8.7)

include_directories(${SDSL_INCLUDES})
include_directories(${INCLUDES})
include_directories(${SDSL_DEPS_INCLUDES})

add_definitions(${COMPILE_WARNINGS})
add_definitions(-std=c++11 -O3 -ggdb)

add_library(libcdat "Alphabet.cpp"
                    "IndexBitVector.cpp"
                    "Index.cpp"
                    "IndexPerm.cpp"
                    "IndexWaveletTree.cpp"
                    "IndexFM.cpp"
                    "IndexNC.cpp"
                    "IndexBC.cpp"
                    "IndexCSA.cpp"
                    "IndexLookupTable")
add_dependencies(libcdat sdsl -L/home/kasia/doktoratnew/externals/sdsl-lite/external/libdivsufsort-2.0.1/lib/)
set ( LIBSUF libdivsufsort64.a libdivsufsort.a )
link_directories( ~/doktoratnew/ )


add_executable(cdat_build "cdat_build.cpp")
add_dependencies(cdat_build libcdat sdsl)
target_link_libraries(cdat_build libcdat sdsl ${LIBSUF} ${Boost_LIBRARIES})

add_executable(cdat "cdat.cpp")
add_dependencies(cdat libcdat sdsl)
target_link_libraries(cdat libcdat sdsl ${LIBSUF} ${Boost_LIBRARIES})

add_executable(cdat_check "cdat_check.cpp")
add_dependencies(cdat_check libcdat sdsl)
target_link_libraries(cdat_check libcdat sdsl ${LIBSUF} ${Boost_LIBRARIES})
