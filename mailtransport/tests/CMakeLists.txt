set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(tm_srcs transportmgr.cpp)
kde4_add_executable(transportmgr TEST ${tm_srcs})
target_link_libraries(transportmgr ${KDE4_KDEUI_LIBS} mailtransport)

