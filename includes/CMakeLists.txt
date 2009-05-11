if (NOT KDEPIM_ONLY_KLEO)

add_subdirectory( tests )

macro(install_headers _dir)
  file(GLOB _includes "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}/[A-Za-z]*")
  install(FILES ${_includes} DESTINATION ${INCLUDE_INSTALL_DIR}/KDEPIMLIBS/${_dir} COMPONENT Devel)
endmacro(install_headers)

if(Akonadi_FOUND)
  install_headers( Akonadi )
endif(Akonadi_FOUND)
install_headers( KHolidays )
install_headers( KABC )
install_headers( KBlog )
install_headers( KCal )
install_headers( KLDAP )
install_headers( KPIMIdentities )
install_headers( KPIMUtils )
install_headers( KResources )
install_headers( Syndication )
install_headers( KPIMTextEdit )

endif (NOT KDEPIM_ONLY_KLEO)
