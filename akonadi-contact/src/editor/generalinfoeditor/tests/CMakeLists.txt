set(generalinfoeditor_SRCS
    main.cpp
    ../generalinfowidget.cpp
    ../phone/phonewidget.cpp
    ../phone/phonelistwidget.cpp
    ../phone/phonewidgetlister.cpp
    ../../widgets/kwidgetlister.cpp
    ../web/webwidgetlister.cpp
    ../web/weblistwidget.cpp
    ../web/webwidget.cpp
    ../mail/maillistwidget.cpp
    ../mail/mailwidget.cpp
    ../mail/mailwidgetlister.cpp
    ../messaging/messaginglistwidget.cpp
    ../messaging/messagingwidget.cpp
    ../messaging/messagingwidgetlister.cpp
    ../../widgets/imagewidget.cpp
    ../namewidget.cpp
    ../nicknamewidget.cpp
    ../phone/phonecomboboxtype.cpp
    ../../im/imaddress.cpp
    ../../utils/utils.cpp
    ../categorieseditwidget.cpp
    ../nameeditdialog.cpp
    ../../widgets/akonadicontactcombobox.cpp
    ../../widgets/preferredlineeditwidget.cpp
    ../phone/phonetypedialog.cpp
    ../displaynameeditwidget.cpp
    ../akonadicontactselecttypecombobox.cpp
    )

add_executable(generalinfoeditortest ${generalinfoeditor_SRCS})

target_link_libraries(generalinfoeditortest
    Qt5::Widgets
    KF5::AkonadiContact
    KF5::Contacts
    KF5::I18n
    KF5::Completion
    KF5::IconThemes
    KF5::WidgetsAddons
    KF5::KIOCore
    KF5::AkonadiWidgets
    )

