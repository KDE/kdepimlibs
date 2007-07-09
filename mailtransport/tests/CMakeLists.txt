
set(tm_srcs transportmgr.cpp)
kde4_add_test_executable(transportmgr ${tm_srcs})
target_link_libraries(transportmgr ${KDE4_KDEUI_LIBS} mailtransport)

