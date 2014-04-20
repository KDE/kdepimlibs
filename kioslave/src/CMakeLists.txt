#remove it
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
add_subdirectory(ldap)
add_subdirectory(mbox)
add_subdirectory(nntp)
#add_subdirectory(sieve)
#add_subdirectory(smtp)

# FIXME if kio_pop3 isn't needed anymore, just delete it.
# add_subdirectory( pop3 ) #KIO::CMD_COMMAND is not public anymore

