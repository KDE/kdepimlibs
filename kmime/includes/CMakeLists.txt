# The KMime headers names are prepended with kmime_ to avoid name clashes.
# We can't use ecm_generate_headers if the CamelCase header doesn't match the filename
set(KMime_HEADERS
  BoolFlags
  CharFreq
  Codecs
  Content
  ContentIndex
  DateFormatter
  HeaderParsing
  Headers
  KMimeMessage
  MDN
  Message
  NewsArticle
  Util
)

install(FILES
        ${KMime_HEADERS}
        DESTINATION ${INCLUDE_INSTALL_DIR}/KMime COMPONENT Devel
)
