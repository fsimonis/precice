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

# Find pkg-config and backup the PKG_CONFIG_PATH
find_package(PkgConfig QUIET)
set(PETSc_PKG_BACKUP "$ENV{PKG_CONFIG_PATH}")

# First search explicitly set directories
if(PETSc_DIR OR PETSc_ROOT OR "$ENV{PETSc_ROOT}")
  # 1) Try to find a cmake config
  set(CMAKE_FIND_ROOT_PATH "${PETSc_DIR};${PETSc_ROOT};$ENV{PETSc_ROOT}")
  find_package(PETSc CONFIG ONLY_CMAKE_FIND_ROOT_PATH)

  # 2) Try to find a package-config file
  if(NOT PETSc_FOUND)
    # Look for a package config file
    find_path(PETSc_PC_DIR NAMES PETSc.pc DOC "The path to the PETSc.pc file." ONLY_CMAKE_FIND_ROOT_PATH)
    if(PETSc_PC_DIR)
      # Use pkgconfig in that directory
      set(ENV{PKG_CONFIG_PATH} "${PETSc_PC_DIR}")
      PKG_CHECK_MODULES(PC_PETSc QUIET petsc)
    endif()
  endif()
endif()

# Second check the traditional way of pointing to a petsc install using the ENV PETSC_DIR and PETSC_ARCH
if(NOT PETSc_FOUND)
  # Prepare the search by checking wether PETSC_ARCH was set.
  # If not, use the target architecture unless this is disabled with PETSc_NO_HOST_ARCH.
  set(PETSC_ARCH "$ENV{PETSC_ARCH}")
  if(PETSC_ARCH AND NOT PETSc_NO_HOST_ARCH)
    set(PETSC_ARCH "${CMAKE_LIBRARY_ARCHITECTURE}")
  endif()

  # Perform the actual search
  if($ENV{PETSC_DIR} AND PETSC_ARCH)
    # Assemble the prefix of the specific PETSc install
    set(PETSc_PATH "$ENV{PETSC_DIR}/${PETSC_ARCH}")
    file(TO_CMAKE_PATH "${PETSc_PATH}" PETSc_PATH)

    # 1) Try to find a cmake config
    set(CMAKE_FIND_ROOT_PATH "${PETSc_PATH}")
    find_package(PETSc CONFIG ONLY_CMAKE_FIND_ROOT_PATH)

    # 2) Try to find a package-config file
    if(NOTE PETSc_FOUND)
      find_path(PETSc_PC_DIR NAMES PETSc.pc DOC "The path to the PETSc.pc file." ONLY_CMAKE_FIND_ROOT_PATH)
      if(PETSc_PC_DIR)
        # Use pkgconfig in that directory
        set(ENV{PKG_CONFIG_PATH} "${PETSc_PC_DIR}")
        PKG_CHECK_MODULES(PC_PETSc QUIET petsc)

        # do
      endif()
    endif()
  endif()
endif(NOT PETSc_FOUND)










# Restore the PKG_CONFIG_PATH
set(ENV{PKG_CONFIG_PATH} "${PETSc_PKG_BACKUP}")
