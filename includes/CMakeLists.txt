if (NOT ONLY_KLEO)

install(FILES
  KHolidays/AstroSeasons
  KHolidays/Holidays
  KHolidays/LunarPhase
  KHolidays/Zodiac
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KHolidays COMPONENT Devel)

endif (NOT ONLY_KLEO)
