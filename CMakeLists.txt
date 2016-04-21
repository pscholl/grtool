#
# This is a CMake makefile.  You can find the cmake utility and
# information about it at http://www.cmake.org
#


cmake_minimum_required(VERSION 2.8.4)

PROJECT(grtool-dlib)

# make sure cmake does not overwrite existing Makefile
MACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD MSG)
  STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insource)
  GET_FILENAME_COMPONENT(PARENTDIR ${CMAKE_SOURCE_DIR} PATH)
  STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${PARENTDIR}" insourcesubdir)
  IF(insource OR insourcesubdir)
    MESSAGE(FATAL_ERROR "${MSG}")
  ENDIF(insource OR insourcesubdir)
ENDMACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)

MACRO_ENSURE_OUT_OF_SOURCE_BUILD( "
Project '${CMAKE_PROJECT_NAME}' requires an out of source build.
Create a build directory and run 'cmake ..' in there.
You may also want to delete CMakeFiles/ and CMakeCache.txt at this point to clean your source tree.
" )



#
# CONFIGURATION
#

#preempt dlibs c++11 check
SET( COMPILER_CAN_DO_CPP_11 1 )
SET( CMAKE_CXX_STANDARD 14 )

# dlib config
INCLUDE( dlib/cmake )

# Set compiler and linker flags
SET( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -fdiagnostics-color=always" )


#
# EXECUTABLES
#

ADD_EXECUTABLE( train-dlib train-dlib.cpp )
TARGET_LINK_LIBRARIES( train-dlib dlib )

ADD_EXECUTABLE( predict-dlib predict-dlib.cpp )
TARGET_LINK_LIBRARIES( predict-dlib dlib )




