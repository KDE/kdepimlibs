add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

set(pimtextedit_lib_srcs
  emailquotehighlighter.cpp
  textedit.cpp
)

kde4_add_library(pimtextedit SHARED ${pimtextedit_lib_srcs})

target_link_libraries(pimtextedit
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  kpimidentities
  kmime
)

set_target_properties(pimtextedit PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS pimtextedit EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  pimtextedit_export.h
  textedit.h
  emailquotehighlighter.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/pimtextedit COMPONENT Devel
)


