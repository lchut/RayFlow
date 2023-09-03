## Copyright 2009-2021 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

# additional parameters (beyond the name) are treated as additional dependencies
# if ADDITIONAL_LIBRARIES is set these will be included during linking

MACRO (ADD_TUTORIAL TUTORIAL_NAME)
  ADD_EXECUTABLE(embree_${TUTORIAL_NAME} ../../kernels/embree.rc ${TUTORIAL_NAME}.cpp ${TUTORIAL_NAME}_device.cpp ${ARGN})
  TARGET_LINK_LIBRARIES(embree_${TUTORIAL_NAME} embree image tutorial noise ${ADDITIONAL_LIBRARIES})
  SET_PROPERTY(TARGET embree_${TUTORIAL_NAME} PROPERTY FOLDER tutorials/single)
  SET_PROPERTY(TARGET embree_${TUTORIAL_NAME} APPEND PROPERTY COMPILE_FLAGS " ${FLAGS_LOWEST}")
  INSTALL(TARGETS embree_${TUTORIAL_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT examples)
  SIGN_TARGET(embree_${TUTORIAL_NAME})
ENDMACRO ()

MACRO (ADD_TUTORIAL_SYCL TUTORIAL_NAME)
  IF (EMBREE_SYCL_SUPPORT)
    ADD_EXECUTABLE(embree_${TUTORIAL_NAME}_sycl ${TUTORIAL_NAME}.cpp ${TUTORIAL_NAME}_device.cpp ${ARGN})
    TARGET_LINK_LIBRARIES(embree_${TUTORIAL_NAME}_sycl embree image tutorial_sycl noise ${ADDITIONAL_LIBRARIES})
    TARGET_COMPILE_DEFINITIONS(embree_${TUTORIAL_NAME}_sycl PUBLIC EMBREE_SYCL_TUTORIAL)
    SET_PROPERTY(TARGET embree_${TUTORIAL_NAME}_sycl PROPERTY FOLDER tutorials/sycl)
    SET_PROPERTY(TARGET embree_${TUTORIAL_NAME}_sycl APPEND PROPERTY COMPILE_FLAGS " ${FLAGS_LOWEST} ${CMAKE_CXX_FLAGS_SYCL}")
    SET_PROPERTY(TARGET embree_${TUTORIAL_NAME}_sycl APPEND PROPERTY LINK_FLAGS "${CMAKE_LINK_FLAGS_SYCL}")
    INSTALL(TARGETS embree_${TUTORIAL_NAME}_sycl DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT examples)
    SIGN_TARGET(embree_${TUTORIAL_NAME}_sycl)
  ENDIF()
ENDMACRO ()

MACRO (ADD_TUTORIAL_ISPC TUTORIAL_NAME)
  IF (EMBREE_ISPC_SUPPORT)
    ADD_EMBREE_ISPC_EXECUTABLE(embree_${TUTORIAL_NAME}_ispc ../../kernels/embree.rc ${TUTORIAL_NAME}.cpp ${TUTORIAL_NAME}_device.ispc ${ARGN})
    TARGET_LINK_LIBRARIES(embree_${TUTORIAL_NAME}_ispc embree image tutorial_ispc noise noise_ispc)
    SET_PROPERTY(TARGET embree_${TUTORIAL_NAME}_ispc PROPERTY FOLDER tutorials/ispc)
    SET_PROPERTY(TARGET embree_${TUTORIAL_NAME}_ispc APPEND PROPERTY COMPILE_FLAGS " ${FLAGS_LOWEST}")
    INSTALL(TARGETS embree_${TUTORIAL_NAME}_ispc DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT examples)
    SIGN_TARGET(embree_${TUTORIAL_NAME}_ispc)
  ENDIF()
ENDMACRO ()
