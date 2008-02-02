include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

if(SASL2_FOUND)
  add_subdirectory( sieve )
endif(SASL2_FOUND)

add_subdirectory( imap4 )
add_subdirectory( ldap )
add_subdirectory( mbox ) 
add_subdirectory( nntp )
add_subdirectory( pop3 )
add_subdirectory( smtp )
