# - Try to find the QGpgME library
# Once done this will define
#
# QGPGME_FOUND
# QGPGME_LIBRARIES

# What we do here is a bit simplictic, but it's no worse than what
# people were using in kdepim up to now...

set( QGPGME_FOUND false )

find_package(Gpgme)

if ( WIN32 AND GPGME_QT_FOUND )
   set( QGPGME_FOUND true )
   set( QGPGME_LIBRARIES "qgpgme;gpgme++-qt;${GPGME_QT_LIBRARIES}" )
endif( WIN32 AND GPGME_QT_FOUND )

if ( NOT WIN32 AND GPGME_PTHREAD_FOUND )
   set( QGPGME_FOUND true )
   set( QGPGME_LIBRARIES "qgpgme;gpgme++-pthread;${GPGME_PTHREAD_LIBRARIES}" )
endif( NOT WIN32 AND GPGME_PTHREAD_FOUND )

if ( QGPGME_FOUND )
   if( NOT QGpgme_FIND_QUIETLY) 
      message( STATUS "Found qgpgme: libraries: ${QGPGME_LIBRARIES}" )
   endif( NOT QGpgme_FIND_QUIETLY )
else( QGPGME_FOUND )
   if( QGpgme_FIND_REQUIRED )
      message( FATAL_ERROR "Did NOT find qgpgme" )
   else( QGpgme_FIND_REQUIRED )
      message( STATUS "Did NOT find qgpgme" )
   endif( QGpgme_FIND_REQUIRED )
endif( QGPGME_FOUND )

