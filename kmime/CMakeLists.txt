add_subdirectory( tests )

include_directories(
	${CMAKE_SOURCE_DIR}/libkdepim/
)

include(CheckTimezone)
configure_file (config-kmime.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmime.h)

########### next target ###############

set(kmime_LIB_SRCS
   kmime_charfreq.cpp
   kmime_util.cpp
   kmime_mdn.cpp
   kmime_codecs.cpp
   kmime_codec_base64.cpp
   kmime_codec_uuencode.cpp
   kmime_codec_qp.cpp
   kmime_codec_identity.cpp
   kmime_parsers.cpp
   kmime_header_parsing.cpp
   kmime_content.cpp
   kmime_contentindex.cpp
   kmime_headers.cpp
   kmime_message.cpp
   kmime_newsarticle.cpp
   kmime_dateformatter.cpp
   boolflags.cpp
   kautodeletehash.cpp )


kde4_add_library(kmime SHARED ${kmime_LIB_SRCS})

target_link_libraries(kmime  ${KDE4_KDECORE_LIBS} )

set_target_properties(kmime PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kmime  
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

install( FILES
        boolflags.h
        kmime_export.h
        kmime_charfreq.h
        kmime_codecs.h
        kmime_content.h
        kmime_contentindex.h
        kmime_header_parsing.h
        kmime_headers.h
        kmime_message.h
        kmime_mdn.h
        kmime_newsarticle.h
        kmime_dateformatter.h
        kmime_util.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kmime)

