
########### next target ###############

set(kio_mbox_PART_SRCS 
   mbox.cpp
   mboxfile.cpp 
   readmbox.cpp
   stat.cpp 
   urlinfo.cpp )


add_library(kio_mbox MODULE ${kio_mbox_PART_SRCS})

target_link_libraries(kio_mbox  KF5::KIOCore KF5::I18n)
set_target_properties(kio_mbox PROPERTIES OUTPUT_NAME "mbox")

install(TARGETS kio_mbox  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kio/)


########### install files ###############

install( FILES mbox.protocol  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})


