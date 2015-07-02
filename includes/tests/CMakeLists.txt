if (KDE4_BUILD_TESTS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
check_cxx_compiler_flag(-pedantic HAVE_PEDANTIC)
if (HAVE_PEDANTIC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
endif()

# clear the file initially
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/all_includes.h "/* all forwarding includes */\n" )

add_definitions(
  -DKRESOURCES_DEPRECATED=
  -DKABC_DEPRECATED=
  -DKCAL_DEPRECATED=
)
# This one won't be needed when CMake 2.8.13 is depended on.
add_definitions(
  -DKRESOURCES_DEPRECATED_EXPORT=KRESOURCES_EXPORT
  -DKABC_DEPRECATED_EXPORT=KABC_EXPORT
  -DKCAL_DEPRECATED_EXPORT=KCAL_EXPORT
)

macro(add_includes _dir)
  file(GLOB_RECURSE _includes "${CMAKE_SOURCE_DIR}/includes/${_dir}/[A-Z]*")
  foreach( file ${_includes} )
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/all_includes.h "#include <${file}>\n" )
  endforeach(file)
endmacro()

include_directories(
  # HACK: akonadi/calendar has a generated header. We need to go up 3 directories level
  ${CMAKE_BINARY_DIR}/akonadi/calendar/tests
  ${CMAKE_BINARY_DIR}/akonadi
  ${CMAKE_BINARY_DIR}/akonadi/contact
  ${CMAKE_BINARY_DIR}/akonadi/calendar
  ${CMAKE_BINARY_DIR}/akonadi/kabc
  ${CMAKE_BINARY_DIR}/akonadi/kcal
  ${CMAKE_BINARY_DIR}/akonadi/kmime
  ${CMAKE_BINARY_DIR}/akonadi/notes
  ${CMAKE_BINARY_DIR}/akonadi/socialutils
  ${CMAKE_BINARY_DIR}/kabc
  ${CMAKE_BINARY_DIR}/kalarmcal
  ${CMAKE_BINARY_DIR}/kcal
  ${CMAKE_BINARY_DIR}/kcalcore
  ${CMAKE_BINARY_DIR}/kcalutils
  ${CMAKE_BINARY_DIR}/kholidays
  ${CMAKE_BINARY_DIR}/kimap
  ${CMAKE_BINARY_DIR}/kldap
  ${CMAKE_BINARY_DIR}/kontactinterface
  ${CMAKE_BINARY_DIR}/kmbox
  ${CMAKE_BINARY_DIR}/kmime
  ${CMAKE_BINARY_DIR}/kpimidentities
  ${CMAKE_BINARY_DIR}/kpimtextedit
  ${CMAKE_BINARY_DIR}/kpimutils
  ${CMAKE_BINARY_DIR}/kresources
  ${CMAKE_BINARY_DIR}/ktnef
  ${CMAKE_BINARY_DIR}/mailtransport
  ${CMAKE_BINARY_DIR}/microblog
  ${CMAKE_BINARY_DIR}/syndication
  ${Boost_INCLUDE_DIR}
)

add_includes( Akonadi )
add_includes( KABC )
add_includes( KAlarmCal )
add_includes( KCalCore )
add_includes( KCalUtils )
if (NOT KDEPIM_NO_KCAL)
  add_includes( KBlog )
  add_includes( KCal )
endif()
add_includes( KHolidays )
add_includes( KIMAP )
add_includes( KLDAP )
add_includes( KMime )
add_includes( KontactInterface )
add_includes( KPIMIdentities )
add_includes( KPIMTextEdit )
add_includes( KPIMUtils )
if (NOT KDEPIM_NO_KRESOURCES)
  add_includes( KResources )
endif()
add_includes( KTNEF )
add_includes( Mailtransport )
add_includes( Syndication )

add_definitions( -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_NO_KEYWORDS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_STRICT_ITERATORS )
kde4_add_executable( headertest header_compile.cpp )
target_link_libraries( headertest ${QT_QTCORE_LIBRARY} )
add_dependencies( headertest akonadi-calendar ) # ensure calendarsettings.h is generated in parallel builds
add_dependencies( headertest kabc ) # ensure addressee.h is generated in parallel builds
add_dependencies( headertest kcal )
add_dependencies( headertest mailtransport ) # ensure transportbase.h is generated in parallel builds

endif()

