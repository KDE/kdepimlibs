#remove it
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
add_subdirectory(ldap)
add_subdirectory(mbox)
add_subdirectory(nntp)
add_subdirectory(sieve)
add_subdirectory(smtp)

add_subdirectory( pop3 ) 

