
if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/..   )

########### next target ###############

set(testemail_SRCS ${emailfunctions_SRCS} testemail.cpp )

kde4_automoc(testemail ${testemail_SRCS})

kde4_add_executable(testemail ${testemail_SRCS})

target_link_libraries(testemail  ${KDE4_KDECORE_LIBS} emailfunctions )

########### install files ###############

endif(KDE4_BUILD_TESTS)
