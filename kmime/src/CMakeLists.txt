include(CheckTimezone)

configure_file(config-kmime.h.cmake ${KMime_BINARY_DIR}/src/config-kmime.h)

# Turn exceptions on
kde_enable_exceptions()

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
   kautodeletehash.cpp)

add_library(KF5Mime ${kmime_LIB_SRCS})

generate_export_header(KF5Mime BASE_NAME KMime)

add_library(KF5::Mime ALIAS KF5Mime)

target_include_directories(KF5Mime INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KMime>")
target_include_directories(KF5Mime PUBLIC "$<BUILD_INTERFACE:${KMime_SOURCE_DIR}/src;${KMime_BINARY_DIR}/src>")

target_link_libraries(KF5Mime
    KF5::KDELibs4Support
)

set_target_properties(KF5Mime PROPERTIES
    VERSION ${KMIME_VERSION_STRING}
    SOVERSION ${KMIME_SOVERSION}
    EXPORT_NAME Mime
)

install(TARGETS KF5Mime EXPORT KF5MimeTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

# Don't install them but needed to compile kdepimlibs
ecm_generate_headers(KMime_CamelCase_HEADERS
  HEADER_NAMES
  KMime_Content
  KMime_Message
  KMime_Codecs
  KMime_Util
  KMime_Dateformatter
  KMime_Header_Parsing
  PREFIX KMime

  REQUIRED_HEADERS KMime_HEADERS
)


install(FILES
         ${KMime_BINARY_DIR}/src/kmime_export.h
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
         DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KMime/kmime COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KMime LIB_NAME KF5Mime DEPS "KDELibs4Support" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/KMime)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
