include(CheckIncludeFiles)
include(MacroBoolTo01)

macro_bool_to_01(SASL2_FOUND HAVE_LIBSASL2)

check_include_files(sys/select.h  HAVE_SYS_SELECT_H)
check_include_files(sys/socket.h  HAVE_SYS_SOCKET_H)
check_include_files(sys/types.h   HAVE_SYS_TYPES_H)
