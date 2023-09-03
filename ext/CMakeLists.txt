cmake_minimum_required (VERSION 3.20)

###########################################################################
# googletest
set (GTest_BUILD_STATIC_LIBS ON CACHE BOOL " " FORCE)
set (GTest_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
add_subdirectory (googletest)
set_property(TARGET gtest PROPERTY FOLDER "ext/gtest")
set (GTest_LIBRARIES gtest gtest_main PARENT_SCOPE)
set (GTest_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include/ PARENT_SCOPE)

###########################################################################
# glog
set (GLOG_BUILD_STATIC_LIBS ON CACHE BOOL " " FORCE)
set (GLOG_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
add_subdirectory (glog)
set_property(TARGET glog PROPERTY FOLDER "ext/glog")
set (GLOG_LIBRARIES glog PARENT_SCOPE)
set (GLOG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/glog/ ${CMAKE_CURRENT_SOURCE_DIR}/glog/src  PARENT_SCOPE)


###########################################################################
# stb_image
set (STB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/stb_image PARENT_SCOPE)

###########################################################################
# tinyxml2
set (BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
add_subdirectory(tinyxml2)
set_property(TARGET tinyxml2 PROPERTY FOLDER "ext/tinyxml2")
set (TINYXML2_LIBRARIES tinyxml2 PARENT_SCOPE)
set (TINYXML2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2/ PARENT_SCOPE)

###########################################################################
# assimp
set (BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
set (ASSIMP_BUILD_TESTS OFF CACHE BOOL " " FORCE)
add_subdirectory(assimp)
set_property(TARGET assimp PROPERTY FOLDER "ext/assimp")
set (ASSIMP_LIBRARIES assimp PARENT_SCOPE)
set (ASSIMP_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/ext/assimp/include/ ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include/ PARENT_SCOPE)

###########################################################################
# tbb
set (BUILD_SHARED_LIBS ON)
set (TBB_TEST OFF CACHE BOOL " " FORCE)
add_subdirectory(oneTBB)
set_property(TARGET tbb PROPERTY FOLDER "ext/tbb")
set(TBB_LIBRARIES tbb PARENT_SCOPE)
set(TBB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/oneTBB/include PARENT_SCOPE)


###########################################################################
# tinyobj
set (BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
add_subdirectory(tinyobjloader)
set_property(TARGET tinyobjloader PROPERTY FOLDER "ext/tinyobjloader")
set (TINYOBJLOADER_LIBRARIES tinyobjloader PARENT_SCOPE)
set (TINYOBJLOADER_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader/ PARENT_SCOPE)
