add_subdirectory( tests )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5320 -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII )
include(CheckTimezone)
configure_file (config-kmime.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmime.h)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

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
   kmime_headerfactory.cpp
   kmime_content.cpp
   kmime_contentindex.cpp
   kmime_headers.cpp
   kmime_message.cpp
   kmime_newsarticle.cpp
   kmime_dateformatter.cpp
   boolflags.cpp
   kautodeletehash.cpp )


add_library(kmime ${LIBRARY_TYPE} ${kmime_LIB_SRCS})
generate_export_header(kmime)

target_link_libraries(kmime LINK_PRIVATE ${KDE4_KDECORE_LIBS} )

set_target_properties(kmime PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kmime EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
         boolflags.h
         ${CMAKE_CURRENT_BINARY_DIR}/kmime_export.h
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
         DESTINATION ${INCLUDE_INSTALL_DIR}/kmime COMPONENT Devel)
