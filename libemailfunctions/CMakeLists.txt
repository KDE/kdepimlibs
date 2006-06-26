
add_subdirectory( tests )

########### next target ###############

set(emailfunctions_LIB_SRCS
   email.cpp)

kde4_automoc(${emailfunctions_LIB_SRCS})

kde4_add_library(emailfunctions SHARED ${emailfunctions_LIB_SRCS})

target_link_libraries(emailfunctions ${KDE4_KDECORE_LIBS})

set_target_properties(emailfunctions PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} emailfunctions )

########### install files ###############




