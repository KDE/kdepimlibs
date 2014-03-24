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
# FIXME: Cyclic dependency between kmime and kpimutils.
# kpimutils/email.cpp needs kmime and kmime_newsarticle.h needs kpimutils
#  kmime_newsarticle.cpp
   kmime_dateformatter.cpp
   boolflags.cpp
   kautodeletehash.cpp)


add_library(KF5KMime ${kmime_LIB_SRCS})

generate_export_header(KF5KMime BASE_NAME KMime)

add_library(KF5::KMime ALIAS KF5KMime)

target_include_directories(KF5KMime INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/kmime>")

target_link_libraries(KF5KMime
    KF5::ItemViews
    KF5::Codecs
    KF5::KDE4Support
)

set_target_properties(KF5KMime PROPERTIES
    VERSION ${KMIME_VERSION_STRING}
    SOVERSION ${KMIME_SOVERSION}
    EXPORT_NAME KMime
)

install(TARGETS KF5KMime EXPORT KF5KMimeTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
         boolflags.h
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
         DESTINATION ${INCLUDE_INSTALL_DIR}/kmime COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KMime LIB_NAME KF5KMime DEPS "Codecs ItemViews KDE4Support" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
