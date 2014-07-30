#include "../request.h"

#include <iostream>

//using std::cout;
//using std::endl;

int main(int , char **)
{
    static QByteArray expected =
        "From: mutz@kde.org\r\n"
        "Subject: missing subject\r\n"
        "To: joe@user.org,\r\n"
        "\tvalentine@14th.february.org\r\n"
        "Cc: boss@example.com\r\n"
        "\n"
        "From: Marc Mutz <mutz@kde.org>\r\n"
        "Subject: missing subject\r\n"
        "To: joe@user.org,\r\n"
        "\tvalentine@14th.february.org\r\n"
        "Cc: boss@example.com\r\n"
        "\n"
        "From: \"Mutz, Marc\" <mutz@kde.org>\r\n"
        "Subject: missing subject\r\n"
        "To: joe@user.org,\r\n"
        "\tvalentine@14th.february.org\r\n"
        "Cc: boss@example.com\r\n"
        "\n"
        "From: =?utf-8?b?TWFyYyBNw7Z0eg==?= <mutz@kde.org>\r\n"
        "Subject: missing subject\r\n"
        "To: joe@user.org,\r\n"
        "\tvalentine@14th.february.org\r\n"
        "Cc: boss@example.com\r\n"
        "\n"
        "From: mutz@kde.org\r\n"
        "Subject: =?utf-8?b?QmzDtmRlcyBTdWJqZWN0?=\r\n"
        "To: joe@user.org,\r\n"
        "\tvalentine@14th.february.org\r\n"
        "Cc: boss@example.com\r\n"
        "\n"
        "From: Marc Mutz <mutz@kde.org>\r\n"
        "Subject: =?utf-8?b?QmzDtmRlcyBTdWJqZWN0?=\r\n"
        "To: joe@user.org,\r\n"
        "\tvalentine@14th.february.org\r\n"
        "Cc: boss@example.com\r\n"
        "\n"
        "From: \"Mutz, Marc\" <mutz@kde.org>\r\n"
        "Subject: =?utf-8?b?QmzDtmRlcyBTdWJqZWN0?=\r\n"
        "To: joe@user.org,\r\n"
        "\tvalentine@14th.february.org\r\n"
        "Cc: boss@example.com\r\n"
        "\n"
        "From: =?utf-8?b?TWFyYyBNw7Z0eg==?= <mutz@kde.org>\r\n"
        "Subject: =?utf-8?b?QmzDtmRlcyBTdWJqZWN0?=\r\n"
        "To: joe@user.org,\r\n"
        "\tvalentine@14th.february.org\r\n"
        "Cc: boss@example.com\r\n"
        "\n";

    KioSMTP::Request request;
    QByteArray result;

    request.setEmitHeaders(true);
    request.setFromAddress(QLatin1String("mutz@kde.org"));
    request.addTo(QLatin1String("joe@user.org"));
    request.addTo(QLatin1String("valentine@14th.february.org"));
    request.addCc(QLatin1String("boss@example.com"));

    result += request.headerFields() + '\n';
    result += request.headerFields(QLatin1String("Marc Mutz")) + '\n';
    result += request.headerFields(QLatin1String("Mutz, Marc")) + '\n';
    result += request.headerFields(QString::fromUtf8("Marc Mötz")) + '\n';

    request.setSubject(QString::fromUtf8("Blödes Subject"));

    result += request.headerFields() + '\n';
    result += request.headerFields(QLatin1String("Marc Mutz")) + '\n';
    result += request.headerFields(QLatin1String("Mutz, Marc")) + '\n';
    result += request.headerFields(QString::fromUtf8("Marc Mötz")) + '\n';

    if (result != expected) {
        std::cout << "Result:\n" << result.data() << std::endl;
        std::cout << "Expected:\n" << expected.data() << std::endl;
    }

    return result == expected ? 0 : 1 ;
}

#include "../request.cpp"

