# - Try to find the gpgme library
#
# Algorithm:
#  - Windows:
#    On Windows, there's three gpgme variants: gpgme{,-glib,-qt}.
#    - The variant used determines the event loop integration possible:
#      - gpgme:      no event loop integration possible, only synchronous operations supported
#      - gpgme-glib: glib event loop integration possible, only asynchronous operations supported
#      - gpgme-qt:   qt event loop integration possible, only asynchronous operations supported
#    - GPGME{,_GLIB,_QT}_{FOUND,LIBRARIES} will be set for each of the above
#    - GPGME_INCLUDES is the same for all of the above
#  - *nix:
#    There's also three variants: gpgme{,-pthread,-pth}.
#    - The variant used determines the mutltithreaded use possible:
#      - gpgme:         no multithreading support available
#      - gpgme-pthread: multithreading available using POSIX threads
#      - gpgme-pth:     multithreading available using GNU PTH (cooperative multithreading)
#    - GPGME_,_PTH,_PTHREAD}_{FOUND,LIBRARIES} will be set for each of the above
#    - GPGME_INCLUDES is the same for all of the above
#

# do away with crappy condition repetition on else/endfoo
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS_gpgme_saved ${CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS} )
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true )

#if this is built-in, please replace, if it isn't, export into a MacroToBool.cmake of it's own
macro( macro_bool_to_bool FOUND_VAR )
  foreach( _current_VAR ${ARGN} )
    if ( ${FOUND_VAR} )
      set( ${_current_VAR} TRUE )
    else()
      set( ${_current_VAR} FALSE )
    endif()
  endforeach()
endmacro()

include (MacroEnsureVersion)



if ( WIN32 )

  # On Windows, we don't have a gpgme-config script, so we need to
  # look for the stuff ourselves:

  # in cmake, AND and OR have the same precedence, there's no
  # subexpressions, and expressions are evaluated short-circuit'ed
  # IOW: CMake if() suxx.
  set( _seem_to_have_cached_gpgme false )
  if ( GPGME_INCLUDES )
    if ( GPGME_LIBRARIES OR GPGME_QT_LIBRARIES OR GPGME_GLIB_LIBRARIES )
      set( _seem_to_have_cached_gpgme true )
    endif()
  endif()

  if ( _seem_to_have_cached_gpgme )

    macro_bool_to_bool( GPGME_LIBRARIES      GPGME_FOUND      )
    macro_bool_to_bool( GPGME_GLIB_LIBRARIES GPGME_GLIB_FOUND )
    macro_bool_to_bool( GPGME_QT_LIBRARIES   GPGME_QT_FOUND   )
    # this would have been preferred:
    #set( GPGME_FOUND macro_bool_to_bool(GPGME_LIBRARIES) )

  else()

    # is this needed, of just unreflected cut'n'paste?
    # this isn't a KDE library, after all!
    if( NOT KDEWIN_FOUND )
      find_package( KDEWIN REQUIRED )
    endif()

    set( GPGME_FOUND      FALSE )
    set( GPGME_GLIB_FOUND FALSE )
    set( GPGME_QT_FOUND   FALSE )

    find_path( GPGME_INCLUDES gpgme.h
      ${CMAKE_INCLUDE_PATH}
      ${CMAKE_INSTALL_PREFIX}/include
    )

    find_library( _GPGME_LIBRARY      NAMES gpgme libgpgme gpgme-11 libgpgme-11
      PATHS 
        ${CMAKE_LIBRARY_PATH}
        ${CMAKE_INSTALL_PREFIX}/lib
    )

    find_library( _GPGME_GLIB_LIBRARY NAMES gpgme-glib libgpgme-glib gpgme-glib-11 libgpgme-glib-11
      PATHS 
        ${CMAKE_LIBRARY_PATH}
        ${CMAKE_INSTALL_PREFIX}/lib
    )

    find_library( _GPGME_QT_LIBRARY   NAMES gpgme-qt libgpgme-qt gpgme-qt-11 libgpgme-qt-11
      PATHS 
        ${CMAKE_LIBRARY_PATH}
        ${CMAKE_INSTALL_PREFIX}/lib
    )

    find_library( _GPG_ERROR_LIBRARY  NAMES gpg-error libgpg-error gpg-error-0 libgpg-error-0
      PATHS 
        ${CMAKE_LIBRARY_PATH}
        ${CMAKE_INSTALL_PREFIX}/lib
    )

    set( GPGME_INCLUDES          ${GPGME_INCLUDES}                          CACHE INTERNAL "The gpgme include paths" )

    if ( _GPGME_LIBRARY AND _GPG_ERROR_LIBRARY )
      set( GPGME_LIBRARIES       ${_GPGME_LIBRARY}      ${_GPG_ERROR_LIBRARY} CACHE INTERNAL "The gpgme libraries" )
      set( GPGME_FOUND           TRUE )
    endif()

    if ( _GPGME_GLIB_LIBRARY AND _GPG_ERROR_LIBRARY )
      set( GPGME_GLIB_LIBRARIES  ${_GPGME_GLIB_LIBRARY} ${_GPG_ERROR_LIBRARY} CACHE INTERNAL "The gpgme-glib libraries" )
      set( GPGME_GLIB_FOUND      TRUE )
    endif()

    if ( _GPGME_QT_LIBRARY AND _GPG_ERROR_LIBRARY )
      set( GPGME_QT_LIBRARIES    ${_GPGME_QT_LIBRARY}   ${_GPG_ERROR_LIBRARY} CACHE INTERNAL "The gpgme-qt libraries" )
      set( GPGME_QT_FOUND        TRUE )
    endif()

  endif()

  # these are Unix-only:
  set( GPGME_PTHREAD_FOUND false )
  set( GPGME_PTH_FOUND     false )
  set( HAVE_GPGME_PTHREAD  0     )
  set( HAVE_GPGME_PTH      0     )

  macro_bool_to_01( GPGME_FOUND      HAVE_GPGME      )
  macro_bool_to_01( GPGME_GLIB_FOUND HAVE_GPGME_GLIB )
  macro_bool_to_01( GPGME_QT_FOUND   HAVE_GPGME_QT   )

else() # not WIN32

  # On *nix, we have the gpgme-config script which can tell us all we
  # need to know:

  # see WIN32 case for an explanation of what this does:
  set( _seem_to_have_cached_gpgme false )
  if ( GPGME_INCLUDES )
    if ( GPGME_LIBRARIES OR GPGME_PTHREAD_LIBRARIES OR GPGME_PTH_LIBRARIES )
      set( _seem_to_have_cached_gpgme true )
    endif()
  endif()

  if ( _seem_to_have_cached_gpgme )

    macro_bool_to_bool( GPGME_LIBRARIES         GPGME_FOUND         )
    macro_bool_to_bool( GPGME_PTHREAD_LIBRARIES GPGME_PTHREAD_FOUND )
    macro_bool_to_bool( GPGME_PTH_LIBRARIES     GPGME_PTH_FOUND     )

  else()

    find_program( _GPGMECONFIG_EXECUTABLE NAMES gpgme-config )

    # if gpgme-config has been found
    if ( _GPGMECONFIG_EXECUTABLE )

      message( STATUS "Found gpgme-config" )

      exec_program( ${_GPGMECONFIG_EXECUTABLE} ARGS --version OUTPUT_VARIABLE GPGME_VERSION )

      set( _GPGME_MIN_VERSION "1.0.0" )
      macro_ensure_version( ${_GPGME_MIN_VERSION} ${GPGME_VERSION} _GPGME_INSTALLED_VERSION_OK )

      if ( NOT _GPGME_INSTALLED_VERSION_OK )

        message( STATUS "The installed version of gpgme is too old: ${GPGME_VERSION} (required: >= ${_GPGME_MIN_VERSION})" )

      else()

        message( STATUS "Found gpgme v${GPGME_VERSION}." )

        exec_program( ${_GPGMECONFIG_EXECUTABLE} ARGS                  --libs OUTPUT_VARIABLE GPGME_LIBRARIES         )
        exec_program( ${_GPGMECONFIG_EXECUTABLE} ARGS --thread=pthread --libs OUTPUT_VARIABLE GPGME_PTHREAD_LIBRARIES )
        exec_program( ${_GPGMECONFIG_EXECUTABLE} ARGS --thread=pth     --libs OUTPUT_VARIABLE GPGME_PTH_LIBRARIES     )

        # append -lgpg-error to the list of libraries, if necessary
        if ( GPGME_LIBRARIES AND NOT GPGME_LIBRARIES MATCHES "lgpg-error" )
          set( GPGME_LIBRARIES "${GPGME_LIBRARIES} -lgpg-error" )
        endif()

        if ( GPGME_PTHREAD_LIBRARIES AND NOT GPGME_PTHREAD_LIBRARIES MATCHES "lgpg-error" )
          set( GPGME_PTHREAD_LIBRARIES "${GPGME_PTHREAD_LIBRARIES} -lgpg-error" )
        endif()

        if ( GPGME_PTH_LIBRARIES AND NOT GPGME_PTH_LIBRARIES MATCHES "lgpg-error" )
          set( GPGME_PTH_LIBRARIES "${GPGME_PTH_LIBRARIES} -lgpg-error" )
        endif()

        macro_bool_to_bool( GPGME_LIBRARIES GPGME_FOUND )
        macro_bool_to_bool( GPGME_PTHREAD_LIBRARIES GPGME_PTHREAD_FOUND )
        macro_bool_to_bool( GPGME_PTH_LIBRARIES  GPGME_PTH_FOUND  )

        if ( GPGME_FOUND OR GPGME_PTHREAD_FOUND OR GPGME_PTH_FOUND )

          exec_program( ${_GPGMECONFIG_EXECUTABLE} ARGS --cflags OUTPUT_VARIABLE _GPGME_CFLAGS )

          if ( _GPGME_CFLAGS )
            string( REGEX REPLACE "(\r?\n)+$" " " _GPGME_CFLAGS  "${_GPGME_CFLAGS}" )
            string( REGEX REPLACE " *-I"      ";" GPGME_INCLUDES "${_GPGME_CFLAGS}" )
          endif()

          # ensure that they are cached
          set( GPGME_INCLUDES          ${GPGME_INCLUDES}          CACHE INTERNAL "The gpgme include paths" )
          set( GPGME_LIBRARIES         ${GPGME_LIBRARIES}         CACHE INTERNAL "The gpgme libraries" )
          set( GPGME_PTHREAD_LIBRARIES ${GPGME_PTHREAD_LIBRARIES} CACHE INTERNAL "The gpgme-pthread libraries" )
          set( GPGME_PTH_LIBRARIES     ${GPGME_PTH_LIBRARIES}     CACHE INTERNAL "The gpgme-pth libraries" )

        endif()

      endif()

    endif()

  endif()

  # these are Windows-only:
  set( GPGME_GLIB_FOUND false )
  set( GPGME_QT_FOUND   false )
  set( HAVE_GPGME_GLIB  0     )
  set( HAVE_GPGME_QT    0     )

  macro_bool_to_01( GPGME_FOUND         HAVE_GPGME         )
  macro_bool_to_01( GPGME_PTHREAD_FOUND HAVE_GPGME_PTHREAD )
  macro_bool_to_01( GPGME_PTH_FOUND     HAVE_GPGME_PTH     )

endif() # WIN32 | Unix


set( _gpgme_flavours "" )
set( _any_gpgme_found FALSE )

if ( GPGME_FOUND )
  set( _gpgme_flavours "${_gpgme_flavours} vanilla" )
  set( _any_gpgme_found TRUE )
endif()

if ( GPGME_GLIB_FOUND )
  set( _gpgme_flavours "${_gpgme_flavours} Glib" )
  set( _any_gpgme_found TRUE )
endif()

if ( GPGME_QT_FOUND )
  set( _gpgme_flavours "${_gpgme_flavours} Qt" )
  set( _any_gpgme_found TRUE )
endif()

if ( GPGME_PTHREAD_FOUND )
  set( _gpgme_flavours "${_gpgme_flavours} pthread" )
  set( _any_gpgme_found TRUE )
endif()

if ( GPGME_PTH_FOUND )
  set( _gpgme_flavours "${_gpgme_flavours} pth" )
  set( _any_gpgme_found TRUE )
endif()


if ( NOT Gpgme_FIND_QUIETLY )

  if ( _any_gpgme_found )
    message( STATUS "Found gpgme. Flavours:${_gpgme_flavours}." )
  else()
    message( STATUS "gpgme not found." )
  endif()

  macro_bool_to_bool( Gpgme_FIND_REQUIRED _req )

  if ( WIN32 )
    set( _gpgme_homepage "http://www.gpg4win.org" )
  else()
    set( _gpgme_homepage "http://www.gnupg.org/related_software/gpgme" )
  endif()

  macro_log_feature(
    _any_gpgme_found
    "gpgme"
    "GnuPG Made Easy Development Libraries"
    ${_gpgme_homepage}
    ${_req}
    "${_GPGME_MIN_VERSION} or greater"
    "Needed to provide GNU Privacy Guard support in KDE PIM applications. Necessary to compile many PIM application, including KMail."
  )

else()

  if ( Gpgme_FIND_REQUIRED AND NOT _any_gpgme_found )
    message( FATAL_ERROR "" )
  endif()

endif()

set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS_gpgme_saved )
