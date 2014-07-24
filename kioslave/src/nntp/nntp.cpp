/* This file is part of KDE
   Copyright (C) 2000 by Wolfram Diestel <wolfram@steloj.de>
   Copyright (C) 2005 by Tim Way <tim@way.hrcoxmail.com>
   Copyright (C) 2005 by Volker Krause <vkrause@kde.org>

   This is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
*/

#include "nntp.h"

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <QUrl>

#include <QDir>
#include <QHash>
#include <QRegExp>
#include <QUrlQuery>
#include <klocalizedstring.h>
#include <QCoreApplication>

#include <kio/ioslave_defaults.h>

#define DBG_AREA 7114
//#define DBG kDebug(DBG_AREA)
#define DBG qDebug()

#undef ERR
//#define ERR kError(DBG_AREA)
#define ERR qCritical()

using namespace KIO;

extern "C" {
    int Q_DECL_EXPORT kdemain(int argc, char **argv);
}

int kdemain(int argc, char **argv)
{
    QCoreApplication app(argc, argv);   // needed for QSocketNotifier
    app.setApplicationName(QLatin1String("kio_nntp"));

    if (argc != 4) {
        fprintf(stderr, "Usage: kio_nntp protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    NNTPProtocol *slave;

    // Are we going to use SSL?
    if (strcasecmp(argv[1], "nntps") == 0) {
        slave = new NNTPProtocol(argv[2], argv[3], true);
    } else {
        slave = new NNTPProtocol(argv[2], argv[3], false);
    }

    slave->dispatchLoop();
    delete slave;

    return 0;
}

/****************** NNTPProtocol ************************/

NNTPProtocol::NNTPProtocol(const QByteArray &pool, const QByteArray &app, bool isSSL)
    : TCPSlaveBase((isSSL ? "nntps" : "nntp"), pool, app, isSSL),
      isAuthenticated(false)
{
    DBG << "=============> NNTPProtocol::NNTPProtocol";

    readBufferLen = 0;
    m_defaultPort = isSSL ? DEFAULT_NNTPS_PORT : DEFAULT_NNTP_PORT;
    m_port = m_defaultPort;
}

NNTPProtocol::~NNTPProtocol()
{
    DBG << "<============= NNTPProtocol::~NNTPProtocol";

    // close connection
    nntp_close();
}

void NNTPProtocol::setHost(const QString &host, quint16 port, const QString &user,
                           const QString &pass)
{
    DBG << (! user.isEmpty() ? (user + QLatin1Char('@')) : QString::fromLatin1(""))
        << host << ":" << ((port == 0) ? m_defaultPort : port);

    if (isConnected() && (mHost != host || m_port != port ||
                          mUser != user || mPass != pass)) {
        nntp_close();
    }

    mHost = host;
    m_port = ((port == 0) ? m_defaultPort : port);
    mUser = user;
    mPass = pass;
}

void NNTPProtocol::get(const QUrl &url)
{
    DBG << url.toDisplayString();
    QString path = QDir::cleanPath(url.path());

    // path should be like: /group/<msg_id> or /group/<serial number>
    if (path.startsWith(QLatin1Char('/'))) {
        path.remove(0, 1);
    }
    int pos = path.indexOf(QLatin1Char('/'));
    QString group;
    QString msg_id;
    if (pos > 0) {
        group = path.left(pos);
        msg_id = path.mid(pos + 1);
    }

    if (group.isEmpty() || msg_id.isEmpty()) {
        error(ERR_DOES_NOT_EXIST, path);
        return;
    }

    int res_code;
    DBG << "group:" << group << "msg:" << msg_id;

    if (!nntp_open()) {
        return;
    }

    // select group if necessary
    if (mCurrentGroup != group && !group.isEmpty()) {
        infoMessage(i18n("Selecting group %1...", group));
        res_code = sendCommand(QLatin1String("GROUP ") + group);
        if (res_code == 411) {
            error(ERR_DOES_NOT_EXIST, path);
            mCurrentGroup.clear();
            return;
        } else if (res_code != 211) {
            unexpected_response(res_code, QLatin1String("GROUP"));
            mCurrentGroup.clear();
            return;
        }
        mCurrentGroup = group;
    }

    // get article
    infoMessage(i18n("Downloading article..."));
    res_code = sendCommand(QLatin1String("ARTICLE ") + msg_id);
    if (res_code == 423 || res_code == 430) {
        error(ERR_DOES_NOT_EXIST, path);
        return;
    } else if (res_code != 220) {
        unexpected_response(res_code, QLatin1String("ARTICLE"));
        return;
    }

    // read and send data
    char tmp[MAX_PACKET_LEN];
    while (true) {
        if (!waitForResponse(readTimeout())) {
            error(ERR_SERVER_TIMEOUT, mHost);
            nntp_close();
            return;
        }
        int len = readLine(tmp, MAX_PACKET_LEN);
        const char *buffer = tmp;
        if (len <= 0) {
            break;
        }
        if (len == 3 && tmp[0] == '.' && tmp[1] == '\r' && tmp[2] == '\n') {
            break;
        }
        if (len > 1 && tmp[0] == '.' && tmp[1] == '.') {
            ++buffer;
            --len;
        }
        data(QByteArray::fromRawData(buffer, len));
    }

    // end of data
    data(QByteArray());

    // finish
    finished();
}

void NNTPProtocol::put(const QUrl &/*url*/, int /*permissions*/, KIO::JobFlags /*flags*/)
{
    if (!nntp_open()) {
        return;
    }
    if (post_article()) {
        finished();
    }
}

void NNTPProtocol::special(const QByteArray &data)
{
    // 1 = post article
    int cmd;
    QDataStream stream(data);

    if (!nntp_open()) {
        return;
    }

    stream >> cmd;
    if (cmd == 1) {
        if (post_article()) {
            finished();
        }
    } else {
        error(ERR_UNSUPPORTED_ACTION, i18n("Invalid special command %1", cmd));
    }
}

bool NNTPProtocol::post_article()
{
    DBG;

    // send post command
    infoMessage(i18n("Sending article..."));
    int res_code = sendCommand(QLatin1String("POST"));
    if (res_code == 440) { // posting not allowed
        error(ERR_WRITE_ACCESS_DENIED, mHost);
        return false;
    } else if (res_code != 340) { // 340: ok, send article
        unexpected_response(res_code, QLatin1String("POST"));
        return false;
    }

    // send article now
    int result;
    bool last_chunk_had_line_ending = true;
    do {
        QByteArray buffer;
        dataReq();
        result = readData(buffer);
        DBG << "receiving data:" << buffer;
        // treat the buffer data
        if (result > 0) {
            // translate "\r\n." to "\r\n.."
            int pos = 0;
            if (last_chunk_had_line_ending && buffer[0] == '.') {
                buffer.insert(0, '.');
                pos += 2;
            }
            last_chunk_had_line_ending = (buffer.endsWith("\r\n"));     //krazy:exclude=strings
            while ((pos = buffer.indexOf("\r\n.", pos)) > 0) {
                buffer.insert(pos + 2, '.');
                pos += 4;
            }

            // send data to socket, write() doesn't send the terminating 0
            write(buffer, buffer.length());
            DBG << "writing:" << buffer;
        }
    } while (result > 0);

    // error occurred?
    if (result < 0) {
        ERR << "error while getting article data for posting";
        nntp_close();
        return false;
    }

    // send end mark
    write("\r\n.\r\n", 5);

    // get answer
    res_code = evalResponse(readBuffer, readBufferLen);
    if (res_code == 441) { // posting failed
        error(ERR_COULD_NOT_WRITE, mHost);
        return false;
    } else if (res_code != 240) {
        unexpected_response(res_code, QLatin1String("POST"));
        return false;
    }

    return true;
}

void NNTPProtocol::stat(const QUrl &url)
{
    DBG << url.toDisplayString();
    UDSEntry entry;
    QString path = QDir::cleanPath(url.path());
    QRegExp regGroup = QRegExp(QLatin1String("^\\/?[a-z0-9\\.\\-_]+\\/?$"), Qt::CaseInsensitive);
    QRegExp regMsgId = QRegExp(QLatin1String("^\\/?[a-z0-9\\.\\-_]+\\/<\\S+>$"), Qt::CaseInsensitive);
    int pos;
    QString group;
    QString msg_id;

    // / = group list
    if (path.isEmpty() || path == QLatin1String("/")) {
        DBG << "root";
        fillUDSEntry(entry, QString(), 0, false, (S_IWUSR | S_IWGRP | S_IWOTH));

        // /group = message list
    } else if (regGroup.indexIn(path) == 0) {
        if (path.startsWith(QLatin1Char('/'))) {
            path.remove(0, 1);
        }
        if ((pos = path.indexOf(QLatin1Char('/'))) > 0) {
            group = path.left(pos);
        } else {
            group = path;
        }
        DBG << "group:" << group;
        // postingAllowed should be ored here with "group not moderated" flag
        // as size the num of messages (GROUP cmd) could be given
        fillUDSEntry(entry, group, 0, false, (S_IWUSR | S_IWGRP | S_IWOTH));

        // /group/<msg_id> = message
    } else if (regMsgId.indexIn(path) == 0) {
        pos = path.indexOf(QLatin1Char('<'));
        group = path.left(pos);
        msg_id = QUrl::fromPercentEncoding(path.right(path.length() - pos).toLatin1());
        if (group.startsWith(QLatin1Char('/'))) {
            group.remove(0, 1);
        }
        if ((pos = group.indexOf(QLatin1Char('/'))) > 0) {
            group = group.left(pos);
        }
        DBG << "group:" << group << "msg:" << msg_id;
        fillUDSEntry(entry, msg_id, 0, true);

        // invalid url
    } else {
        error(ERR_DOES_NOT_EXIST, path);
        return;
    }

    statEntry(entry);
    finished();
}

void NNTPProtocol::listDir(const QUrl &url)
{
    DBG << url.toDisplayString();
    if (!nntp_open()) {
        return;
    }

    QString path = QDir::cleanPath(url.path());

    if (path.isEmpty()) {
        QUrl newURL(url);
        newURL.setPath(QLatin1String("/"));
        DBG << "redirecting to" << newURL.toDisplayString();
        redirection(newURL);
        finished();
        return;
    } else if (path == QLatin1String("/")) {
        QUrl newUrl(url);
        fetchGroups(QUrlQuery(newUrl).queryItemValue(QLatin1String("since")), QUrlQuery(newUrl).queryItemValue(QLatin1String("desc")) == QLatin1String("true"));
        finished();
    } else {
        // if path = /group
        int pos;
        QString group;
        if (path.startsWith(QLatin1Char('/'))) {
            path.remove(0, 1);
        }
        if ((pos = path.indexOf(QLatin1Char('/'))) > 0) {
            group = path.left(pos);
        } else {
            group = path;
        }
        QUrl newUrl(url);
        QString first = QUrlQuery(newUrl).queryItemValue(QLatin1String("first"));
        QString max = QUrlQuery(newUrl).queryItemValue(QLatin1String("max"));
        if (fetchGroup(group, first.toULong(), max.toULong())) {
            finished();
        }
    }
}

void NNTPProtocol::fetchGroups(const QString &since, bool desc)
{
    int expected;
    int res;
    if (since.isEmpty()) {
        // full listing
        infoMessage(i18n("Downloading group list..."));
        res = sendCommand(QLatin1String("LIST"));
        expected = 215;
    } else {
        // incremental listing
        infoMessage(i18n("Looking for new groups..."));
        res = sendCommand(QLatin1String("NEWGROUPS ") + since);
        expected = 231;
    }
    if (res != expected) {
        unexpected_response(res, QLatin1String("LIST"));
        return;
    }

    // read newsgroups line by line
    QByteArray line;
    QString group;
    int pos, pos2;
    long msg_cnt;
    long access;
    UDSEntry entry;
    QHash<QString, UDSEntry> entryMap;

    // read in data and process each group. one line at a time
    while (true) {
        if (! waitForResponse(readTimeout())) {
            error(ERR_SERVER_TIMEOUT, mHost);
            nntp_close();
            return;
        }
        readBufferLen = readLine(readBuffer, MAX_PACKET_LEN);
        line = QByteArray(readBuffer, readBufferLen);
        if (line == ".\r\n") {
            break;
        }

        // group name
        if ((pos = line.indexOf(' ')) > 0) {

            group = QLatin1String(line.left(pos));

            // number of messages
            line.remove(0, pos + 1);
            long last = 0;
            access = 0;
            if (((pos = line.indexOf(' ')) > 0 || (pos = line.indexOf('\t')) > 0) &&
                    ((pos2 = line.indexOf(' ', pos + 1)) > 0 || (pos2 = line.indexOf('\t', pos + 1)) > 0)) {
                last = line.left(pos).toLongLong();
                long first = line.mid(pos + 1, pos2 - pos - 1).toLongLong();
                msg_cnt = abs(last - first + 1);
                // group access rights
                switch (line[pos2 + 1]) {
                case 'n': access = 0; break;
                case 'm': access = S_IWUSR | S_IWGRP; break;
                case 'y': access = S_IWUSR | S_IWGRP | S_IWOTH; break;
                }
            } else {
                msg_cnt = 0;
            }

            entry.clear();
            fillUDSEntry(entry, group, msg_cnt, false, access);
            if (!desc) {
                listEntry(entry, false);
            } else {
                entryMap.insert(group, entry);
            }
        }
    }

    // handle group descriptions
    QHash<QString, UDSEntry>::Iterator it = entryMap.begin();
    if (desc) {
        infoMessage(i18n("Downloading group descriptions..."));
        totalSize(entryMap.size());
    }
    while (desc) {
        // request all group descriptions
        if (since.isEmpty()) {
            res = sendCommand(QLatin1String("LIST NEWSGROUPS"));
        } else {
            // request only descriptions for new groups
            if (it == entryMap.end()) {
                break;
            }
            res = sendCommand(QLatin1String("LIST NEWSGROUPS ") + it.key());
            ++it;
            if (res == 503) {
                // Information not available (RFC 2980 §2.1.6), try next group
                continue;
            }
        }
        if (res != 215) {
            // No group description available or not implemented
            break;
        }

        // download group descriptions
        while (true) {
            if (! waitForResponse(readTimeout())) {
                error(ERR_SERVER_TIMEOUT, mHost);
                nntp_close();
                return;
            }
            readBufferLen = readLine(readBuffer, MAX_PACKET_LEN);
            line = QByteArray(readBuffer, readBufferLen);
            if (line == ".\r\n") {
                break;
            }

            //DBG << "  fetching group description: " << QString( line ).trimmed();
            int pos = line.indexOf(' ');
            pos = pos < 0 ? line.indexOf('\t') : qMin(pos, line.indexOf('\t'));
            group = QLatin1String(line.left(pos));
            QString groupDesc = QLatin1String(line.right(line.length() - pos).trimmed());

            if (entryMap.contains(group)) {
                entry = entryMap.take(group);
                entry.insert(KIO::UDSEntry::UDS_EXTRA, groupDesc);
                listEntry(entry, false);
            }
        }

        if (since.isEmpty()) {
            break;
        }
    }
    // take care of groups without descriptions
    for (QHash<QString, UDSEntry>::Iterator it = entryMap.begin(); it != entryMap.end(); ++it) {
        listEntry(it.value(), false);
    }

    entry.clear();
    listEntry(entry, true);
}

bool NNTPProtocol::fetchGroup(QString &group, unsigned long first, unsigned long max)
{
    int res_code;
    QString resp_line;

    // select group
    infoMessage(i18n("Selecting group %1...", group));
    res_code = sendCommand(QLatin1String("GROUP ") + group);
    if (res_code == 411) {
        error(ERR_DOES_NOT_EXIST, group);
        mCurrentGroup.clear();
        return false;
    } else if (res_code != 211) {
        unexpected_response(res_code, QLatin1String("GROUP"));
        mCurrentGroup.clear();
        return false;
    }
    mCurrentGroup = group;

    // repsonse to "GROUP <requested-group>" command is 211 then find the message count (cnt)
    // and the first and last message followed by the group name
    unsigned long firstSerNum, lastSerNum;
    resp_line = QString::fromLatin1(readBuffer);
    QRegExp re(QLatin1String("211\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)"));
    if (re.indexIn(resp_line) != -1) {
        firstSerNum = re.cap(2).toLong();
        lastSerNum = re.cap(3).toLong();
    } else {
        error(ERR_INTERNAL, i18n("Could not extract message serial numbers from server response:\n%1",
                                 resp_line));
        return false;
    }

    if (firstSerNum == 0) {
        return true;
    }
    first = qMax(first, firstSerNum);
    if (lastSerNum < first) {   // No need to fetch anything
        // note: this also ensure that "lastSerNum - first" is not negative
        // in the next test (in "unsigned long" computation this leads to an overflow
        return true;
    }
    if (max > 0 && lastSerNum - first > max) {
        first = lastSerNum - max + 1;
    }

    DBG << "Starting from serial number: " << first << " of " << firstSerNum << " - " << lastSerNum;
    setMetaData(QLatin1String("FirstSerialNumber"), QString::number(firstSerNum));
    setMetaData(QLatin1String("LastSerialNumber"), QString::number(lastSerNum));

    infoMessage(i18n("Downloading new headers..."));
    totalSize(lastSerNum - first);
    bool notSupported = true;
    if (fetchGroupXOVER(first, notSupported)) {
        return true;
    } else if (notSupported) {
        return fetchGroupRFC977(first);
    }
    return false;
}

bool NNTPProtocol::fetchGroupRFC977(unsigned long first)
{
    UDSEntry entry;

    // set article pointer to first article and get msg-id of it
    int res_code = sendCommand(QLatin1String("STAT ") + QString::number(first));
    QString resp_line = QLatin1String(readBuffer);
    if (res_code != 223) {
        unexpected_response(res_code, QLatin1String("STAT"));
        return false;
    }

    //STAT res_line: 223 nnn <msg_id> ...
    QString msg_id;
    int pos, pos2;
    if ((pos = resp_line.indexOf(QLatin1Char('<'))) > 0 && (pos2 = resp_line.indexOf(QLatin1Char('>'), pos + 1))) {
        msg_id = resp_line.mid(pos, pos2 - pos + 1);
        fillUDSEntry(entry, msg_id, 0, true);
        listEntry(entry, false);
    } else {
        error(ERR_INTERNAL, i18n("Could not extract first message id from server response:\n%1",
                                 resp_line));
        return false;
    }

    // go through all articles
    while (true) {
        res_code = sendCommand(QLatin1String("NEXT"));
        if (res_code == 421) {
            // last artice reached
            entry.clear();
            listEntry(entry, true);
            return true;
        } else if (res_code != 223) {
            unexpected_response(res_code, QLatin1String("NEXT"));
            return false;
        }

        //res_line: 223 nnn <msg_id> ...
        resp_line = QLatin1String(readBuffer);
        if ((pos = resp_line.indexOf(QLatin1Char('<'))) > 0 && (pos2 = resp_line.indexOf(QLatin1Char('>'), pos + 1))) {
            msg_id = resp_line.mid(pos, pos2 - pos + 1);
            entry.clear();
            fillUDSEntry(entry, msg_id, 0, true);
            listEntry(entry, false);
        } else {
            error(ERR_INTERNAL, i18n("Could not extract message id from server response:\n%1",
                                     resp_line));
            return false;
        }
    }
    return true; // Not reached
}

bool NNTPProtocol::fetchGroupXOVER(unsigned long first, bool &notSupported)
{
    notSupported = false;

    QString line;
    QStringList headers;

    int res = sendCommand(QLatin1String("LIST OVERVIEW.FMT"));
    if (res == 215) {
        while (true) {
            if (! waitForResponse(readTimeout())) {
                error(ERR_SERVER_TIMEOUT, mHost);
                nntp_close();
                return false;
            }
            readBufferLen = readLine(readBuffer, MAX_PACKET_LEN);
            line = QString::fromLatin1(readBuffer, readBufferLen);
            if (line == QLatin1String(".\r\n")) {
                break;
            }
            headers << line.trimmed();
            DBG << "OVERVIEW.FMT:" << line.trimmed();
        }
    } else {
        // fallback to defaults
        headers << QLatin1String("Subject:") << QLatin1String("From:") << QLatin1String("Date:") << QLatin1String("Message-ID:")
                << QLatin1String("References:") << QLatin1String("Bytes:") << QLatin1String("Lines:");
    }

    res = sendCommand(QLatin1String("XOVER ") + QString::number(first) + QLatin1Char('-'));
    if (res == 420) {
        return true;    // no articles selected
    }
    if (res == 500) {
        notSupported = true;    // unknwon command
    }
    if (res != 224) {
        unexpected_response(res, QLatin1String("XOVER"));
        return false;
    }

    long msgSize;
    QString name;
    UDSEntry entry;
    int udsType;

    QStringList fields;
    while (true) {
        if (! waitForResponse(readTimeout())) {
            error(ERR_SERVER_TIMEOUT, mHost);
            nntp_close();
            return false;
        }
        readBufferLen = readLine(readBuffer, MAX_PACKET_LEN);
        line = QString::fromLatin1(readBuffer, readBufferLen);
        if (line == QLatin1String(".\r\n")) {
            entry.clear();
            listEntry(entry, true);
            return true;
        }

        fields = line.split(QLatin1Char('\t'), QString::KeepEmptyParts);
        msgSize = 0;
        entry.clear();
        udsType = KIO::UDSEntry::UDS_EXTRA;
        QStringList::ConstIterator it = headers.constBegin();
        QStringList::ConstIterator it2 = fields.constBegin();
        // first entry is the serial number
        name = (*it2);
        ++it2;
        for (; it != headers.constEnd() && it2 != fields.constEnd(); ++it, ++it2) {
            if ((*it) == QLatin1String("Bytes:")) {
                msgSize = (*it2).toLong();
                continue;
            }
            QString atomStr;
            if ((*it).endsWith(QLatin1String("full")))
                if ((*it2).trimmed().isEmpty()) {
                    atomStr = (*it).left((*it).indexOf(QLatin1Char(':')) + 1);    // strip of the 'full' suffix
                } else {
                    atomStr = (*it2).trimmed();
                }
            else {
                atomStr = (*it) + QLatin1Char(' ') + (*it2).trimmed();
            }
            entry.insert(udsType++, atomStr);
            if (udsType >= KIO::UDSEntry::UDS_EXTRA_END) {
                break;
            }
        }
        fillUDSEntry(entry, name, msgSize, true);
        listEntry(entry, false);
    }
    return true; // not reached
}

void NNTPProtocol::fillUDSEntry(UDSEntry &entry, const QString &name, long size,
                                bool is_article, long access)
{

    long posting = 0;

    // entry name
    entry.insert(KIO::UDSEntry::UDS_NAME, name);

    // entry size
    entry.insert(KIO::UDSEntry::UDS_SIZE, size);

    // file type
    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, is_article ? S_IFREG : S_IFDIR);

    // access permissions
    posting = postingAllowed ? access : 0;
    long long accessVal = (is_article) ? (S_IRUSR | S_IRGRP | S_IROTH) :
                          (S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH | posting);
    entry.insert(KIO::UDSEntry::UDS_ACCESS, accessVal);

    entry.insert(KIO::UDSEntry::UDS_USER, mUser.isEmpty() ? QString::fromLatin1("root") : mUser);

    /*
    entry->insert(UDS_GROUP, QString::fromLatin1("root"));
    */

    // MIME type
    if (is_article) {
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("message/news"));
    }
}

void NNTPProtocol::nntp_close()
{
    if (isConnected()) {
        write("QUIT\r\n", 6);
        disconnectFromHost();
        isAuthenticated = false;
    }
    mCurrentGroup.clear();
}

bool NNTPProtocol::nntp_open()
{
    // if still connected reuse connection
    if (isConnected()) {
        DBG << "reusing old connection";
        return true;
    }

    DBG << "  nntp_open -- creating a new connection to" << mHost << ":" << m_port;
    // create a new connection (connectToHost() includes error handling)
    infoMessage(i18n("Connecting to server..."));
    if (connectToHost((isAutoSsl() ? QLatin1String("nntps") : QLatin1String("nntp")), mHost, m_port)) {
        DBG << "  nntp_open -- connection is open";

        // read greeting
        int res_code = evalResponse(readBuffer, readBufferLen);

        /* expect one of
             200 server ready - posting allowed
             201 server ready - no posting allowed
        */
        if (!(res_code == 200 || res_code == 201)) {
            unexpected_response(res_code, QLatin1String("CONNECT"));
            return false;
        }

        DBG << "  nntp_open -- greating was read res_code :" << res_code;

        res_code = sendCommand(QLatin1String("MODE READER"));

        // TODO: not in RFC 977, so we should not abort here
        if (!(res_code == 200 || res_code == 201)) {
            unexpected_response(res_code, QLatin1String("MODE READER"));
            return false;
        }

        // let local class know whether posting is allowed or not
        postingAllowed = (res_code == 200);

        // activate TLS if requested
        if (metaData(QLatin1String("tls")) == QLatin1String("on")) {
            if (sendCommand(QLatin1String("STARTTLS")) != 382) {
                error(ERR_COULD_NOT_CONNECT, i18n("This server does not support TLS"));
                return false;
            }
            if (!startSsl()) {
                error(ERR_COULD_NOT_CONNECT, i18n("TLS negotiation failed"));
                return false;
            }
        }

        // *try* to authenticate now (see bug#167718)
        authenticate();

        return true;
    }

    return false;
}

int NNTPProtocol::sendCommand(const QString &cmd)
{
    int res_code = 0;

    if (!nntp_open()) {
        ERR << "NOT CONNECTED, cannot send cmd" << cmd;
        return 0;
    }

    DBG << "cmd:" << cmd;

    write(cmd.toLatin1(), cmd.length());
    // check the command for proper termination
    if (!cmd.endsWith(QLatin1String("\r\n"))) {
        write("\r\n", 2);
    }
    res_code =  evalResponse(readBuffer, readBufferLen);

    // if authorization needed send user info
    if (res_code == 480) {
        DBG << "auth needed, sending user info";

        if (mUser.isEmpty() || mPass.isEmpty()) {
            KIO::AuthInfo authInfo;
            authInfo.username = mUser;
            authInfo.password = mPass;
            if (openPasswordDialog(authInfo)) {
                mUser = authInfo.username;
                mPass = authInfo.password;
            }
        }
        if (mUser.isEmpty() || mPass.isEmpty()) {
            return res_code;
        }

        res_code = authenticate();
        if (res_code != 281) {
            // error should be handled by invoking function
            return res_code;
        }

        // ok now, resend command
        write(cmd.toLatin1(), cmd.length());
        if (!cmd.endsWith(QLatin1String("\r\n"))) {
            write("\r\n", 2);
        }
        res_code = evalResponse(readBuffer, readBufferLen);
    }

    return res_code;
}

int NNTPProtocol::authenticate()
{
    int res_code = 0;

    if (isAuthenticated) {
        // already authenticated
        return 281;
    }

    if (mUser.isEmpty() || mPass.isEmpty()) {
        return 281; // failsafe : maybe add a "relax" mode to optionally ask user/pwd.
    }

    // send username to server and confirm response
    write("AUTHINFO USER ", 14);
    write(mUser.toLatin1(), mUser.length());
    write("\r\n", 2);
    res_code = evalResponse(readBuffer, readBufferLen);

    if (res_code == 281) {
        // no password needed (RFC 2980 3.1.1 does not required one)
        return res_code;
    }
    if (res_code != 381) {
        // error should be handled by invoking function
        return res_code;
    }

    // send password
    write("AUTHINFO PASS ", 14);
    write(mPass.toLatin1(), mPass.length());
    write("\r\n", 2);
    res_code = evalResponse(readBuffer, readBufferLen);

    if (res_code == 281) {
        isAuthenticated = true;
    }

    return res_code;
}

void NNTPProtocol::unexpected_response(int res_code, const QString &command)
{
    ERR << "Unexpected response to" << command << "command: (" << res_code << ")"
        << readBuffer;

    // See RFC 3977 appendix C "Summary of Response Codes"
    switch (res_code) {
    case 205: // connection closed by the server: this can happens, e.g. if the session timeout on the server side
    // Not the same thing, but use the same message as code 400 anyway.
    case 400: // temporary issue on the server
        error(ERR_INTERNAL_SERVER,
              i18n("The server %1 could not handle your request.\n"
                   "Please try again now, or later if the problem persists.", mHost));
        break;
    case 480: // credential request
        error(ERR_COULD_NOT_LOGIN,
              i18n("You need to authenticate to access the requested resource."));
        break;
    case 481: // wrong credential (TODO: place a specific message for this case)
        error(ERR_COULD_NOT_LOGIN,
              i18n("The supplied login and/or password are incorrect."));
        break;
    case 502:
        error(ERR_ACCESS_DENIED, mHost);
        break;
    default:
        error(ERR_INTERNAL, i18n("Unexpected server response to %1 command:\n%2", command, QLatin1String(readBuffer)));
    }

    nntp_close();
}

int NNTPProtocol::evalResponse(char *data, ssize_t &len)
{
    if (!waitForResponse(responseTimeout())) {
        error(ERR_SERVER_TIMEOUT , mHost);
        nntp_close();
        return -1;
    }
    len = readLine(data, MAX_PACKET_LEN);

    if (len < 3) {
        return -1;
    }

    // get the first three characters. should be the response code
    int respCode = ((data[0] - 48) * 100) + ((data[1] - 48) * 10) + ((data[2] - 48));

    DBG << "got:" << respCode;

    return respCode;
}

/* not really necessary, because the slave has to
   use the KIO::Error's instead, but let this here for
   documentation of the NNTP response codes and may
   by later use.
QString  NNTPProtocol::errorStr(int resp_code) {
  QString ret;

  switch (resp_code) {
  case 100: ret = "help text follows"; break;
  case 199: ret = "debug output"; break;

  case 200: ret = "server ready - posting allowed"; break;
  case 201: ret = "server ready - no posting allowed"; break;
  case 202: ret = "slave status noted"; break;
  case 205: ret = "closing connection - goodbye!"; break;
  case 211: ret = "group selected"; break;
  case 215: ret = "list of newsgroups follows"; break;
  case 220: ret = "article retrieved - head and body follow"; break;
  case 221: ret = "article retrieved - head follows"; break;
  case 222: ret = "article retrieved - body follows"; break;
  case 223: ret = "article retrieved - request text separately"; break;
  case 230: ret = "list of new articles by message-id follows"; break;
  case 231: ret = "list of new newsgroups follows"; break;
  case 235: ret = "article transferred ok"; break;
  case 240: ret = "article posted ok"; break;

  case 335: ret = "send article to be transferred"; break;
  case 340: ret = "send article to be posted"; break;

  case 400: ret = "service discontinued"; break;
  case 411: ret = "no such news group"; break;
  case 412: ret = "no newsgroup has been selected"; break;
  case 420: ret = "no current article has been selected"; break;
  case 421: ret = "no next article in this group"; break;
  case 422: ret = "no previous article in this group"; break;
  case 423: ret = "no such article number in this group"; break;
  case 430: ret = "no such article found"; break;
  case 435: ret = "article not wanted - do not send it"; break;
  case 436: ret = "transfer failed - try again later"; break;
  case 437: ret = "article rejected - do not try again"; break;
  case 440: ret = "posting not allowed"; break;
  case 441: ret = "posting failed"; break;

  case 500: ret = "command not recognized"; break;
  case 501: ret = "command syntax error"; break;
  case 502: ret = "access restriction or permission denied"; break;
  case 503: ret = "program fault - command not performed"; break;
  default:  ret = QString("unknown NNTP response code %1").arg(resp_code);
  }

  return ret;
}
*/
