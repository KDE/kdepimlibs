if (KDE4_BUILD_TESTS)

# clear the file initially
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/all_includes.h "/* all forwarding includes */\n" )

macro(add_includes _dir)
  file(GLOB _includes "${CMAKE_CURRENT_SOURCE_DIR}/../${_dir}/[A-Z]*")
  foreach( file ${_includes} )
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/all_includes.h "#include <${file}>\n" )
  endforeach(file)
endmacro(add_includes)

include_directories(
  ${CMAKE_SOURCE_DIR}/kabc
  ${CMAKE_BINARY_DIR}/kabc
  ${CMAKE_BINARY_DIR}/kcal
)

if ( Akonadi_FOUND )
  add_includes( Akonadi )
endif( Akonadi_FOUND )
add_includes( KHolidays )
add_includes( KABC )
add_includes( KBlog )
add_includes( KCal )
add_includes( KLDAP )
add_includes( KResources )
add_includes( KPIMIdentities )
add_includes( Syndication )
add_includes( KPIMUtils )
add_includes( PimTextEdit )

add_definitions( -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_NO_KEYWORDS )
kde4_add_executable( headertest header_compile.cpp )
add_dependencies( headertest kabc ) # ensure addressee.h is generated in parallel builds
add_dependencies( headertest kcal )

endif (KDE4_BUILD_TESTS)

