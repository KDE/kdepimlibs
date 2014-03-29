
macro(install_headers _dir)
  install( DIRECTORY ${_dir} DESTINATION ${INCLUDE_INSTALL_DIR}/KDE )
endmacro()

install_headers( Akonadi )
install_headers( KAlarmCal )
if (NOT KDEPIM_NO_KCAL)
  install_headers( KBlog )
endif()
install_headers( KLDAP )
install_headers( KontactInterface )
install_headers( KTNEF )
install_headers( Mailtransport )
install_headers( KPIMTextEdit )

endif()
