include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( sieve )
add_subdirectory( ldap )
add_subdirectory( mbox )
add_subdirectory( nntp )
# add_subdirectory( pop3 ) #KIO::CMD_COMMAND is not public anymore
add_subdirectory( smtp )
