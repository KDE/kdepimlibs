
include(ECMMarkAsTest)

find_package(Qt5Test CONFIG REQUIRED)

set(tm_srcs transportmgr.cpp)
add_executable(transportmgr ${tm_srcs})
ecm_mark_as_test(transportmgr)
target_link_libraries(transportmgr KF5MailTransport)

set(queuer_srcs queuer.cpp)
add_executable(queuer ${queuer_srcs})
ecm_mark_as_test(queuer)
target_link_libraries(queuer KF5MailTransport)

set( sendqueued_srcs sendqueued.cpp )
add_executable( sendqueued ${sendqueued_srcs} )
add_test(sendqueued sendqueued)
ecm_mark_as_test(sendqueued)
target_link_libraries( sendqueued KF5MailTransport  KF5::AkonadiMime)

set( clearerror_srcs clearerror.cpp )
add_executable( clearerror ${clearerror_srcs} )
add_test(clearerror clearerror)
ecm_mark_as_test(clearerror)
target_link_libraries( clearerror KF5MailTransport  KF5::AkonadiMime)

set( abort_srcs abort.cpp )
add_executable( abort ${abort_srcs} )
add_test(abort abort)
ecm_mark_as_test(abort)
target_link_libraries( abort KF5MailTransport KF5::AkonadiCore)

