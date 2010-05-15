if (KDE4_BUILD_TESTS)

# clear the file initially
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/all_includes.h "/* all forwarding includes */\n" )

add_definitions(-DWANT_DEPRECATED_KCAL_API)
add_definitions(-DWANT_DEPRECATED_KABC_API)
add_definitions(-DWANT_DEPRECATED_KRESOURCE_API)

macro(add_includes _dir)
  file(GLOB_RECURSE _includes "${CMAKE_SOURCE_DIR}/includes/${_dir}/[A-Z]*")
  foreach( file ${_includes} )
    if(NOT file MATCHES ".svn")
      file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/all_includes.h "#include <${file}>\n" )
    endif(NOT file MATCHES ".svn")
  endforeach(file)
endmacro(add_includes)

include_directories(
  ${CMAKE_SOURCE_DIR}/kabc
  ${CMAKE_BINARY_DIR}/kabc
  ${CMAKE_BINARY_DIR}/kcal
  ${Boost_INCLUDE_DIR}
)

add_includes( Akonadi )
add_includes( KABC )
add_includes( KBlog )
add_includes( KCal )
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
endif (NOT KDEPIM_NO_KRESOURCES)
add_includes( KTNEF )
add_includes( Mailtransport )
add_includes( Syndication )

add_definitions( -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_NO_KEYWORDS )
kde4_add_executable( headertest header_compile.cpp )
target_link_libraries( headertest ${QT_QTCORE_LIBRARY} )
add_dependencies( headertest kabc ) # ensure addressee.h is generated in parallel builds
add_dependencies( headertest kcal )
add_dependencies( headertest mailtransport ) # ensure transportbase.h is generated in parallel builds

endif (KDE4_BUILD_TESTS)

