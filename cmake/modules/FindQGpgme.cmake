# - Try to find the QGpgME library
# Once done this will define
#
# QGPGME_FOUND
# QGPGME_LIBRARIES
# QGPGME_INCLUDE_DIR

# What we do here is a bit simplictic, but it's no worse than what
# people were using in kdepim up to now...

find_package(Gpgme)

if(GPGME_FOUND)

   if ( WIN32 )
      find_library(_QGPGME_EXTRA_LIBRARY gpgme++
                   HINTS ${GPGME_LIBRARY_DIR})
   else ( WIN32 )
      find_library(_QGPGME_EXTRA_LIBRARY gpgme++-pthread
                   HINTS ${GPGME_LIBRARY_DIR})
   endif ( WIN32 )

   find_library(QGPGME_LIBRARY qgpgme
                HINTS ${GPGME_LIBRARY_DIR})

   if (QGPGME_LIBRARY)
      # get the libdirectory and then go one up
      get_filename_component(_QGPGME_PREFIX "${QGPGME_LIBRARY}" PATH)
      get_filename_component(_QGPGME_PREFIX "${_QGPGME_PREFIX}" PATH)
      find_path(QGPGME_INCLUDE_DIR qgpgme/qgpgme_export.h 
                HINTS "${_QGPGME_PREFIX}/include" )
   endif (QGPGME_LIBRARY)

   if ( WIN32 )
      set(QGPGME_LIBRARIES ${QGPGME_LIBRARY} ${_QGPGME_EXTRA_LIBRARY} ${GPGME_VANILLA_LIBRARIES} ${GPGME_QT_LIBRARIES})
   else ( WIN32 )
      set(QGPGME_LIBRARIES ${QGPGME_LIBRARY} ${_QGPGME_EXTRA_LIBRARY} ${GPGME_PTHREAD_LIBRARIES})
   endif ( WIN32 )

endif(GPGME_FOUND)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QGpgme  DEFAULT_MSG  QGPGME_LIBRARY QGPGME_INCLUDE_DIR _QGPGME_EXTRA_LIBRARY)

mark_as_advanced(QGPGME_LIBRARY _QGPGME_EXTRA_LIBRARY QGPGME_INCLUDE_DIR)
