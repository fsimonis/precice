## FindPETSc
# 
# Find the Portable, Extensible Toolkit for Scientific Computation (petsc).
#
## The following variables influence the search order
#
# 1) Explicitly set directory of PETSc
# CMake variables
#   PETSc_DIR
#   PETSc_ROOT
# ENV variables
#   PETSc_ROOT
#
# 2) Traditionally set location
# ENV variables
#   PETSC_DIR   - directory in which PETSc resides
#   PETSC_ARCH  - build architecture
# CMake variables
#   CMAKE_LIBRARY_ARCHITECTURE - Use the host architecture if PETSC_ARCH was not defined.
#                                Define PETSc_NO_HOST_ARCH=ON to disable this behaviour.
# 
# 
#
# 
## IMPORTED Targets
# 
# This module defines IMPORTED target PETSc::PETSc, if petsc has been found.
# 
## Result variables
# 
# This module will set the following variables in your project:
# 
# ``PETSc_FOUND``
#   true if PETSc was found
# ``PETSc_INCLUDE_DIR``
#   the directory containing PETSc headers
# ``PETSc_INCLUDE_DIRS``
#   list of the include directories needed to use PETSc
# ``PETSc_LIBRARIES``
#   PETSc libraries to link to
# ``PETSc_VERSION_STRING``
#   the full version of PETSc
# ``PETSc_VERSION_MAJOR``
#   the major version of PETSc
# ``PETSc_VERSION_MINOR``
#   the minor version of PETSc
# ``PETSc_VERSION_PATCH``
#   the patch version of PETSc
# 

cmake_policy(VERSION 3.9)

find_package(PkgConfig QUIET)

function(find_petsc_in_prefix)
  message(STATUS "BOOTING")
  cmake_parse_arguments(PARSE_ARGV 0 FP "" "" "PREFIX")
  if(NOT FP_PREFIX)
    message(FATAL_ERROR "Argument PREFIX missing!")
  endif()
  message(STATUS "Searching in PREFIX ${FP_PREFIX}")

  ## Intro
  # Backup the PKG_CONFIG_PATH
  set(PETSc_PKG_BACKUP "$ENV{PKG_CONFIG_PATH}")

  ## 1) Try to find a cmake config
  set(CMAKE_FIND_ROOT_PATH "${PETSc_DIR};${PETSc_ROOT};$ENV{PETSc_ROOT}")
  find_package(PETSc CONFIG ONLY_CMAKE_FIND_ROOT_PATH)

  ## 2) Try to find a package-config file
  if(NOT PETSc_FOUND)
    # Look for a package config file
    find_path(PETSc_PC_DIR NAMES PETSc.pc DOC "The path to the PETSc.pc file." ONLY_CMAKE_FIND_ROOT_PATH)
    if(PETSc_PC_DIR)
      # Use pkgconfig in that directory
      set(ENV{PKG_CONFIG_PATH} "${PETSc_PC_DIR}")
      PKG_CHECK_MODULES(PC_PETSc QUIET petsc)

      set(PETSc_FOUND "${PC_PETSc_FOUND}")
      find_library(
        PETSc_LIBRARY

        NAMES petsc ${PC_PETSc_LIBRARIES}

        PATH
        "${FP_PREFIX}"
        "${PC_PETSc_LIBRARY_DIRS}"

        PATH_SUFFIXES
        )
      find_path(
        PETSc_INCLUDE_DIR

        NAMES petsc.h petsc.hpp

        PATH
        "${FP_PREFIX}"
        "${PC_PETSc_LIBRARY_DIRS}"
        )
      set(PETSc_VERSION_STRING "${PC_PETSc_VERSION}")
    endif()
  endif()

  ## Outro
  # Restore the PKG_CONFIG_PATH
  set(ENV{PKG_CONFIG_PATH} "${PETSc_PKG_BACKUP}")
endfunction(find_petsc_in_prefix)


# Predefine Vars
set(PETSc_FOUND "")
set(PETSc_LIBRARY "")
set(PETSc_INCLUDE_DIR "")
set(PETSc_VERSION_STRING "")

# First search explicitly set directories
message(STATUS "[1] Search explicit directories")
if(PETSc_DIR OR PETSc_ROOT OR "$ENV{PETSc_ROOT}")
  find_petsc_in_prefix(PREFIX
    ${PETSc_DIR}
    ${PETSc_ROOT}
    "$ENV{PETSc_ROOT}"
    )
endif()

# Second check the traditional way of pointing to a petsc install using the ENV PETSC_DIR and PETSC_ARCH
message(STATUS "[2] Search PETSC_DIR/PETSC_ARCH directory")
if(NOT PETSc_FOUND)
  # Prepare the search by checking wether PETSC_ARCH was set.
  # If not, use the target architecture unless this is disabled with PETSc_NO_HOST_ARCH.
  set(PETSC_ARCH "$ENV{PETSC_ARCH}")
  if(NOT PETSC_ARCH AND NOT PETSc_NO_HOST_ARCH AND CMAKE_LIBRARY_ARCHITECTURE)
    message(STATUS "PETSC_ARCH empty, using target architecture instead")
    set(PETSC_ARCH "${CMAKE_LIBRARY_ARCHITECTURE}")
  endif()

  message(STATUS "PETSC_DIR  $ENV{PETSC_DIR}")
  message(STATUS "PETSC_ARCH ${PETSC_ARCH}")

  # Perform the actual search
  message("before")
  if($ENV{PETSC_DIR} AND PETSC_ARCH)
    # Assemble the prefix of the specific PETSc install
    set(PETSc_PATH "$ENV{PETSC_DIR}/${PETSC_ARCH}")
    file(TO_CMAKE_PATH "${PETSc_PATH}" PETSc_PATH)

    find_petsc_in_prefix(PREFIX ${PETSc_PATH})
  endif()
  message("after")
endif(NOT PETSc_FOUND)

# Third try a system wide search
message(STATUS "[3] Search in entire system")
if(NOT PETSc_FOUND)
  ## 1) Try to find a cmake config
  find_package(PETSc CONFIG)

  ## 2) Try to find a package-config file
  if(NOT PETSc_FOUND)
    PKG_CHECK_MODULES(PC_PETSc QUIET petsc)

    file(GLOB PETSc_DEBIAN_DIRS "/usr/lib/petscdir/*")
    find_library(
      PETSc_LIBRARY

      NAMES petsc ${PC_PETSc_LIBRARIES}

      HINTS
      ENV PETSC_DIR
      ${PC_PETSc_LIBRARY_DIRS}
      ${PETSc_DEBIAN_DIRS}

      PATH_SUFFIXES
      "$ENV{PETSC_ARCH}"
      "${CMAKE_LIBRARY_ARCHITECTURE}"
      )
    find_path(
      PETSc_INCLUDE_DIR

      NAMES petsc.h petsc.hpp

      HINTS
      ${PC_PETSc_INCLUDE_DIRS}
      ENV PETSC_DIR
      ${PETSc_DEBIAN_DIRS}

      PATH_SUFFIXES
      "$ENV{PETSC_ARCH}"
      "$ENV{PETSC_ARCH}/include"
      "${CMAKE_LIBRARY_ARCHITECTURE}"
      "${CMAKE_LIBRARY_ARCHITECTURE}/include"
      )
      set(PETSc_VERSION_STRING "${PC_PETSc_VERSION}")
  endif(NOT PETSc_FOUND)
endif(NOT PETSc_FOUND)


message("PETSc_FOUND         :${PETSc_FOUND}")
message("PETSc_LIBRARY       :${PETSc_LIBRARY}")
message("PETSc_INCLUDE_DIR   :${PETSc_INCLUDE_DIR}")
message("PETSc_VERSION_STRING:${PETSc_VERSION_STRING}")

# Finally, check if PETSc was found
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PETSc
  REQUIRED_VARS PETSc_LIBRARY PETSc_INCLUDE_DIR
  VERSION_VAR PETSc_VERSION_STRING
  )

# Set alias variables
set(PETSc_LIBRARIES ${PETSc_LIBRARY})
set(PETSc_INCLUDE_DIRS ${PETSc_INCLUDE_DIR})
mark_as_advanced( PETSc_LIBRARIES PETSc_INCLUDE_DIRS)

# Set version variables
if(PETSc_VERSION_STRING)
  set(PETSc_VERSIONS "")
  set(PETSc_VERSION_MAJOR "" PARENT_SCOPE)
  set(PETSc_VERSION_MINOR "" PARENT_SCOPE)
  set(PETSc_VERSION_PATCH "" PARENT_SCOPE)
  string(REGEX MATCHALL "[0-9]+" PETSC_VERSIONS ${PETSC_VERSION_STRING})
  list(GET PETSc_VERSIONS 0 PETSc_VERSION_MAJOR)
  list(GET PETSc_VERSIONS 1 PETSc_VERSION_MINOR)
  list(GET PETSc_VERSIONS 1 PETSc_VERSION_PATCH)
  mark_as_advanced(
    PETSc_VERSION_MAJOR
    PETSc_VERSION_MINOR
    PETSc_VERSION_PATCH
    )
endif(PETSc_VERSION_STRING)


# Create an IMPORTED target for PETSc
if(PETSc_FOUND AND NOT TARGET PETSc::PETSc)
  add_library(PETSc::PETSc UNKNOWN IMPORTED)
  set_target_properties(PETSc::PETSc
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${PETSc_INCLUDE_DIRS}"
    IMPORTED_LOCATION "${PETSc_LIBRARY}"
    VERSION "${PETSc_VERSION_STRING}"
    )
endif()







