# Locate OpenEXR
# This module defines
# OpenEXR_LIBRARY
# OpenEXR_FOUND, if false, do not try to link to OpenEXR 
# OpenEXR_INCLUDE_DIR, where to find the headers
#
# $OPENEXR_DIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENEXR_DIR
#
# Created by Robert Osfield. 


FIND_PATH(OPENEXR_INCLUDE_DIR OpenEXR/ImfIO.h
    $ENV{OPENEXR_DIR}/include
    $ENV{OPENEXR_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include/
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    /usr/freeware/include    
    /usr/local/include/OpenEXR
    /usr/include/OpenEXR
    /sw/include/OpenEXR # Fink
    /opt/local/include/OpenEXR # DarwinPorts
    /opt/csw/include/OpenEXR # Blastwave
    /opt/include/OpenEXR
    /usr/freeware/include/OpenEXR
)

# Macro to find exr libraries (deduplicating search paths)
# example: OPENEXR_FIND_VAR(OPENEXR_IlmIlf_LIBRARY IlmIlf)
MACRO(OPENEXR_FIND_VAR varname libname)
    FIND_LIBRARY( ${varname}
        NAMES ${libname}
        PATHS
        $ENV{OPENEXR_DIR}/lib
        $ENV{OPENEXR_DIR}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        /usr/freeware/lib64
    )
ENDMACRO(OPENEXR_FIND_VAR)

# Macro to find exr libraries (and debug versions)
# example: OPENEXR_FIND(IlmIlf)
SET(_openexr_FIND_COMPONENTS
  Half
  Iex
  IlmImf
  IlmThread
  Imath
)

FIND_PATH(OpenEXR_INCLUDE_DIR
  NAMES
    OpenEXR/ImfXdr.h
  HINTS
    ${_openexr_SEARCH_DIRS}
  PATH_SUFFIXES
    include
)

SET(_openexr_LIBRARIES)
FOREACH(COMPONENT ${_openexr_FIND_COMPONENTS})
  STRING(TOUPPER ${COMPONENT} UPPERCOMPONENT)

  FIND_LIBRARY(OPENEXR_${UPPERCOMPONENT}_LIBRARY
    NAMES
      ${COMPONENT}
    HINTS
      ${_openexr_SEARCH_DIRS}
    PATH_SUFFIXES
      lib64 lib
    )
  LIST(APPEND _openexr_LIBRARIES "${OPENEXR_${UPPERCOMPONENT}_LIBRARY}")
ENDFOREACH()

IF(_openexr_LIBRARIES)
    SET(OpenEXR_LIBRARIES ${_openexr_LIBRARIES} )
    SET(OPENEXR_LIBRARIES_VARS OPENEXR_IlmIlf_LIBRARY OPENEXR_IlmThread_LIBRARY OPENEXR_Half_LIBRARY OPENEXR_Iex_LIBRARY )
    SET(OpenEXR_FOUND 1 CACHE BOOL "Found OpenEXR library")
ELSE(_openexr_LIBRARIES)
    SET(OpenEXR_FOUND 0 CACHE BOOL "OpenEXR library not found!")
ENDIF(_openexr_LIBRARIES)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    OpenEXR
    REQUIRED_VARS OpenEXR_LIBRARIES OpenEXR_INCLUDE_DIR
    FAIL_MESSAGE "Could not find the OpenEXR library. ART will not be able to read and write such images."
    )

MARK_AS_ADVANCED(
    OpenEXR_INCLUDE_DIR
    OpenEXR_LIBRARIES
    OpenEXR_FOUND
    )
