project(kdepimlibs)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

#boost is a hard dependency
find_package(Boost)
macro_log_feature(Boost_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org/" TRUE "" "Needed by libsyndication")

macro_optional_find_package(Sasl2)
macro_log_feature(SASL2_FOUND "cyrus-sasl" "Cyrus SASL API" "http://asg.web.cmu.edu/sasl/sasl-library.html" FALSE "" "Needed to support authentication of logins. IMAP and Sieve kioslaves will not be built.")

# gpgme is a hard dependency
find_package(Gpgme REQUIRED)

include (gpgme++/ConfigureChecks.cmake)
include (ConfigureChecks.cmake)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

add_subdirectory(gpgme++)
add_subdirectory(qgpgme)
# (gpgme++/qgpgme handle their conditions inside their own CMakeLists.txt files)
add_subdirectory(kabc)
add_subdirectory(kblog)
add_subdirectory(kcal)
add_subdirectory(kimap)
add_subdirectory(kldap)
add_subdirectory(kmime)
add_subdirectory(kpimidentities)
add_subdirectory(kpimutils)
add_subdirectory(kresources)
add_subdirectory(ktnef)
add_subdirectory(kxmlrpcclient)
add_subdirectory(mailtransport)
add_subdirectory(syndication)
add_subdirectory(kioslave)

add_subdirectory(cmake)

# ...and append all library dependencies
string(REGEX REPLACE "gpgmepp" "gpgme++" qgpgme_LIB_DEPENDS "${qgpgme_LIB_DEPENDS}")
export_library_dependencies(${CMAKE_CURRENT_BINARY_DIR}/KDEPimLibsDependencies.cmake APPEND)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/KDEPimLibsDependencies.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)

macro_display_feature_log()
