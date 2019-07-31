#
# Find the cairo libraries. On exit sets linkable library ocpn::cairo
#

if (CAIRO_INCLUDE_DIRS)
  # Already in cache, be silent
  set(CAIRO_FIND_QUIETLY TRUE)
endif ()

if (CMAKE_HOST_WIN32)
  set(CAIRO_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/buildwin/gtk/include)
else (CMAKE_HOST_WIN32)
  set(CAIRO_INC_LOOK_PATHS /usr/local/include /usr/include)
  if (APPLE)
    if (NOT APPLE_MODERN)
        SET(CAIRO_INC_LOOK_PATHS /usr/local/Cellar/cairo/1.14.6/include)
        SET(LOOK_OPTION NO_DEFAULT_PATH)
    endif ()
  endif (APPLE)
  find_path(CAIRO_INCLUDE_DIRS cairo.h
            PATHS ${CAIRO_INC_LOOK_PATHS}
            PATH_SUFFIXES cairo/ libcairo/ cairo/libcairo/
            ${LOOK_OPTION})
endif (CMAKE_HOST_WIN32)
  
if (CMAKE_HOST_WIN32)
  set(CAIRO_LIBRARIES
    ${CMAKE_SOURCE_DIR}/buildwin/gtk/cairo.lib
    ${CMAKE_SOURCE_DIR}/buildwin/archive.lib
  )
  set(CAIRO_FOUND 1)
else (CMAKE_HOST_WIN32)
  set(CAIRO_LIB_LOOK_PATHS ${LINUX_LIB_PATHS})
  if (APPLE AND NOT APPLE_MODERN)
    set(CAIRO_LIB_LOOK_PATHS /usr/local/Cellar/cairo/1.14.6/lib)
    set(LOOK_OPTION NO_DEFAULT_PATH)
  endif ()
  find_library(CAIRO_LIBRARIES
      NAMES libcairo cairo 
      PATHS ${CAIRO_LIB_LOOK_PATHS}
      ${LOOK_OPTION}
  )
  message(STATUS " Cairo library found: ${CAIRO_LIBRARIES}")
  # handle the QUIETLY and REQUIRED arguments and set CAIRO_FOUND to TRUE if
  # all listed variables are TRUE
  include( "FindPackageHandleStandardArgs" )
  find_package_handle_standard_args("CAIRO"
      DEFAULT_MSG CAIRO_INCLUDE_DIRS CAIRO_LIBRARIES
  )
  mark_as_advanced(CAIRO_INCLUDE_DIRS CAIRO_LIBRARIES )
endif ()

if (NOT CAIRO_FOUND)
  message(FATAL_ERROR "Cairo component required, but not found!")
endif ()

# Some systems (e.g ARMHF RPI2) require some extra libraries
# This is not exactly a general solution, but probably harmless where
# not needed.
if (NOT APPLE AND NOT WIN32)
  find_library(
    PANGOCAIRO_LIBRARY
    NAMES pangocairo-1.0 PATHS ${LINUX_LIB_PATHS}
  )
  find_library(PANGOFT2_LIBRARY NAMES pangoft2-1.0 PATHS ${LINUX_LIB_PATHS})
  find_library(PANGOXFT_LIBRARY NAMES pangoxft-1.0 PATHS ${LINUX_LIB_PATHS})
  find_library(
    GDK_PIXBUF_LIBRARY
    NAMES gdk_pixbuf-2.0 PATHS ${LINUX_LIB_PATHS}
  )
  find_package_handle_standard_args("CAIRO_EXTRAS" DEFAULT_MSG
    PANGOCAIRO_LIBRARY PANGOFT2_LIBRARY PANGOXFT_LIBRARY GDK_PIXBUF_LIBRARY
  )
endif ()

if (CAIRO_EXTRAS_FOUND)
  set(CAIRO_LIBRARIES ${CAIRO_LIBRARIES}
    ${PANGOCAIRO_LIBRARY}
    ${PANGOFT2_LIBRARY} ${PANGOXFT_LIBRARY}
    ${GDK_PIXBUF_LIBRARY}
  )
endif ()
add_library(_CAIRO INTERFACE)
target_link_libraries(_CAIRO INTERFACE ${CAIRO_LIBRARIES})

target_include_directories(_CAIRO INTERFACE ${CAIRO_INCLUDE_DIRS})
if (APPLE)
  target_include_directories(_CAIRO INTERFACE ${CAIRO_INCLUDE_DIRS}/..)
endif ()
if (PANGOCAIRO_INCLUDE_DIRS)
  target_include_directories(_CAIRO INTERFACE ${PANGOCAIRO_INCLUDE_DIRS})
endif ()

add_library(ocpn::cairo ALIAS _CAIRO)
