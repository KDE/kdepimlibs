if (NOT KDEPIM_ONLY_KLEO)

# Qt5: Port
# add_subdirectory( tests )

macro(install_headers _dir)
  install( DIRECTORY ${_dir} DESTINATION ${INCLUDE_INSTALL_DIR}/KDE )
endmacro()

install_headers( Akonadi )
install_headers( KAlarmCal )
install_headers( KABC )
if (NOT KDEPIM_NO_KCAL)
  install_headers( KBlog )
endif()
install_headers( KCalUtils )
install_headers( KIMAP )
install_headers( KLDAP )
install_headers( KontactInterface )
install_headers( KPIMIdentities )
if (NOT KDEPIM_NO_KRESOURCES)
  install_headers( KResources )
endif()
install_headers( KTNEF )
install_headers( Mailtransport )
install_headers( KPIMTextEdit )

endif()
