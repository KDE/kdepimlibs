/*
    ktnefparser.cpp

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */
/**
 * @file
 * This file is part of the API for handling TNEF data and
 * defines the KTNEFParser class.
 *
 * @author Michael Goffioul
 */

#include "ktnefparser.h"
#include "ktnefattach.h"
#include "ktnefproperty.h"
#include "ktnefmessage.h"
#include "ktnefdefs.h"

#include <qdebug.h>
#include <QMimeType>
#include <QMimeDatabase>
#include <qsavefile.h>

#include <QtCore/QDateTime>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtCore/QVariant>
#include <QtCore/QList>

using namespace KTnef;

//@cond PRIVATE
typedef struct {
    quint16 type;
    quint16 tag;
    QVariant value;
    struct {
        quint32 type;
        QVariant value;
    } name;
} MAPI_value;
//@endcond

//@cond IGNORE
void clearMAPIName(MAPI_value &mapi);
void clearMAPIValue(MAPI_value &mapi, bool clearName = true);
QString readMAPIString(QDataStream &stream, bool isUnicode = false,
                       bool align = true, int len = -1);
quint16 readMAPIValue(QDataStream &stream, MAPI_value &mapi);
QDateTime readTNEFDate(QDataStream &stream);
QString readTNEFAddress(QDataStream &stream);
QByteArray readTNEFData(QDataStream &stream, quint32 len);
QVariant readTNEFAttribute(QDataStream &stream, quint16 type, quint32 len);
QDateTime formatTime(quint32 lowB, quint32 highB);
QString formatRecipient(const QMap<int, KTnef::KTNEFProperty *> &props);
//@endcond

//------------------------------------------------------------------------------

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
//@cond PRIVATE
class KTnef::KTNEFParser::ParserPrivate
{
public:
    ParserPrivate()
    {
        defaultdir_ = QLatin1String("/tmp/");
        current_ = 0;
        deleteDevice_ = false;
        device_ = 0;
        message_ = new KTNEFMessage;
    }
    ~ParserPrivate()
    {
        delete message_;
    }

    bool decodeAttachment();
    bool decodeMessage();
    bool extractAttachmentTo(KTNEFAttach *att, const QString &dirname);
    void checkCurrent(int key);
    bool readMAPIProperties(QMap<int, KTNEFProperty *> &props,
                            KTNEFAttach *attach = 0);
    bool parseDevice();
    void deleteDevice();

    QDataStream  stream_;
    QIODevice    *device_;
    bool         deleteDevice_;
    QString      defaultdir_;
    KTNEFAttach  *current_;
    KTNEFMessage *message_;
};
//@endcond

KTNEFParser::KTNEFParser()
    : d(new ParserPrivate)
{
}

KTNEFParser::~KTNEFParser()
{
    d->deleteDevice();
    delete d;
}

KTNEFMessage *KTNEFParser::message() const
{
    return d->message_;
}

void KTNEFParser::ParserPrivate::deleteDevice()
{
    if (deleteDevice_) {
        delete device_;
    }
    device_ = 0;
    deleteDevice_ = false;
}

bool KTNEFParser::ParserPrivate::decodeMessage()
{
    quint32  i1, i2, off;
    quint16  u, tag, type;
    QVariant value;

    // read (type+name)
    stream_ >> i1;
    u = 0;
    tag = (i1 & 0x0000FFFF);
    type = ((i1 & 0xFFFF0000) >> 16);
    // read data length
    stream_ >> i2;
    // offset after reading the value
    off = device_->pos() + i2;
    switch (tag) {
    case attAIDOWNER: {
        uint tmp;
        stream_ >> tmp;
        value.setValue(tmp);
        message_->addProperty(0x0062, MAPI_TYPE_ULONG, value);
        qDebug() << "Message Owner Appointment ID" << "(length=" << i2 << ")";
        break;
    }
    case attREQUESTRES:
        stream_ >> u;
        message_->addProperty(0x0063, MAPI_TYPE_UINT16, u);
        value = (bool)u;
        qDebug() << "Message Request Response" << "(length=" << i2 << ")";
        break;
    case attDATERECD:
        value = readTNEFDate(stream_);
        message_->addProperty(0x0E06, MAPI_TYPE_TIME, value);
        qDebug() << "Message Receive Date" << "(length=" << i2 << ")";
        break;
    case attMSGCLASS:
        value = readMAPIString(stream_, false, false, i2);
        message_->addProperty(0x001A, MAPI_TYPE_STRING8, value);
        qDebug() << "Message Class" << "(length=" << i2 << ")";
        break;
    case attMSGPRIORITY:
        stream_ >> u;
        message_->addProperty(0x0026, MAPI_TYPE_ULONG, 2 - u);
        value = u;
        qDebug() << "Message Priority" << "(length=" << i2 << ")";
        break;
    case attMAPIPROPS:
        qDebug() << "Message MAPI Properties" << "(length=" << i2 << ")";
        {
            int nProps = message_->properties().count();
            i2 += device_->pos();
            readMAPIProperties(message_->properties(), 0);
            device_->seek(i2);
            qDebug() << "Properties:" << message_->properties().count();
            value = QString::fromLatin1("< %1 properties >").
                    arg(message_->properties().count() - nProps);
        }
        break;
    case attTNEFVERSION: {
        uint tmp;
        stream_ >> tmp;
        value.setValue(tmp);
        qDebug() << "Message TNEF Version" << "(length=" << i2 << ")";
    }
    break;
    case attFROM:
        message_->addProperty(0x0024, MAPI_TYPE_STRING8, readTNEFAddress(stream_));
        device_->seek(device_->pos() - i2);
        value = readTNEFData(stream_, i2);
        qDebug() << "Message From" << "(length=" << i2 << ")";
        break;
    case attSUBJECT:
        value = readMAPIString(stream_, false, false, i2);
        message_->addProperty(0x0037, MAPI_TYPE_STRING8, value);
        qDebug() << "Message Subject" << "(length=" << i2 << ")";
        break;
    case attDATESENT:
        value = readTNEFDate(stream_);
        message_->addProperty(0x0039, MAPI_TYPE_TIME, value);
        qDebug() << "Message Date Sent" << "(length=" << i2 << ")";
        break;
    case attMSGSTATUS: {
        quint8 c;
        quint32 flag = 0;
        stream_ >> c;
        if (c & fmsRead) {
            flag |= MSGFLAG_READ;
        }
        if (!(c & fmsModified)) {
            flag |= MSGFLAG_UNMODIFIED;
        }
        if (c & fmsSubmitted) {
            flag |= MSGFLAG_SUBMIT;
        }
        if (c & fmsHasAttach) {
            flag |= MSGFLAG_HASATTACH;
        }
        if (c & fmsLocal) {
            flag |= MSGFLAG_UNSENT;
        }
        message_->addProperty(0x0E07, MAPI_TYPE_ULONG, flag);
        value = c;
    }
    qDebug() << "Message Status" << "(length=" << i2 << ")";
    break;
    case attRECIPTABLE: {
        quint32 rows;
        QList<QVariant> recipTable;
        stream_ >> rows;
        for (uint i = 0; i < rows; i++) {
            QMap<int, KTNEFProperty *> props;
            readMAPIProperties(props, 0);
            recipTable << formatRecipient(props);
        }
        message_->addProperty(0x0E12, MAPI_TYPE_STRING8, recipTable);
        device_->seek(device_->pos() - i2);
        value = readTNEFData(stream_, i2);
    }
    qDebug() << "Message Recipient Table" << "(length=" << i2 << ")";
    break;
    case attBODY:
        value = readMAPIString(stream_, false, false, i2);
        message_->addProperty(0x1000, MAPI_TYPE_STRING8, value);
        qDebug() << "Message Body" << "(length=" << i2 << ")";
        break;
    case attDATEMODIFIED:
        value = readTNEFDate(stream_);
        message_->addProperty(0x3008, MAPI_TYPE_TIME, value);
        qDebug() << "Message Date Modified" << "(length=" << i2 << ")";
        break;
    case attMSGID:
        value = readMAPIString(stream_, false, false, i2);
        message_->addProperty(0x300B, MAPI_TYPE_STRING8, value);
        qDebug() << "Message ID" << "(length=" << i2 << ")";
        break;
    case attOEMCODEPAGE:
        value = readTNEFData(stream_, i2);
        qDebug() << "Message OEM Code Page" << "(length=" << i2 << ")";
        break;
    default:
        value = readTNEFAttribute(stream_, type, i2);
        //qDebug().form( "Message: type=%x, length=%d, check=%x\n", i1, i2, u );
        break;
    }
    // skip data
    if (device_->pos() != off && !device_->seek(off)) {
        return false;
    }
    // get checksum
    stream_ >> u;
    // add TNEF attribute
    message_->addAttribute(tag, type, value, true);
    //qDebug() << "stream:" << device_->pos();
    return true;
}

bool KTNEFParser::ParserPrivate::decodeAttachment()
{
    quint32  i;
    quint16  tag, type, u;
    QVariant value;
    QString  str;

    stream_ >> i;                // i <- attribute type & name
    tag = (i & 0x0000FFFF);
    type = ((i & 0xFFFF0000) >> 16);
    stream_ >> i;                // i <- data length
    checkCurrent(tag);
    switch (tag) {
    case attATTACHTITLE:
        value = readMAPIString(stream_, false, false, i);
        current_->setName(value.toString());
        qDebug() << "Attachment Title:" << current_->name();
        break;
    case attATTACHDATA:
        current_->setSize(i);
        current_->setOffset(device_->pos());
        device_->seek(device_->pos() + i);
        value = QString::fromLatin1("< size=%1 >").arg(i);
        qDebug() << "Attachment Data: size=" << i;
        break;
    case attATTACHMENT:  // try to get attachment info
        i += device_->pos();
        readMAPIProperties(current_->properties(), current_);
        device_->seek(i);
        current_->setIndex(current_->property(MAPI_TAG_INDEX).toUInt());
        current_->setDisplaySize(current_->property(MAPI_TAG_SIZE).toUInt());
        str = current_->property(MAPI_TAG_DISPLAYNAME).toString();
        if (!str.isEmpty()) {
            current_->setDisplayName(str);
        }
        current_->setFileName(current_->property(MAPI_TAG_FILENAME).
                              toString());
        str = current_->property(MAPI_TAG_MIMETAG).toString();
        if (!str.isEmpty()) {
            current_->setMimeTag(str);
        }
        current_->setExtension(current_->property(MAPI_TAG_EXTENSION).
                               toString());
        value = QString::fromLatin1("< %1 properties >").
                arg(current_->properties().count());
        break;
    case attATTACHMODDATE:
        value = readTNEFDate(stream_);
        qDebug() << "Attachment Modification Date:" << value.toString();
        break;
    case attATTACHCREATEDATE:
        value = readTNEFDate(stream_);
        qDebug() << "Attachment Creation Date:" << value.toString();
        break;
    case attATTACHMETAFILE:
        qDebug() << "Attachment Metafile: size=" << i;
        //value = QString( "< size=%1 >" ).arg( i );
        //device_->seek( device_->pos()+i );
        value = readTNEFData(stream_, i);
        break;
    default:
        value = readTNEFAttribute(stream_, type, i);
        qDebug() << "Attachment unknown field:         tag="
                 << hex << tag << ", length=" << dec << i;
        break;
    }
    stream_ >> u;        // u <- checksum
    // add TNEF attribute
    current_->addAttribute(tag, type, value, true);
    //qDebug() << "stream:" << device_->pos();

    return true;
}

void KTNEFParser::setDefaultExtractDir(const QString &dirname)
{
    d->defaultdir_ = dirname;
}

bool KTNEFParser::ParserPrivate::parseDevice()
{
    quint16 u;
    quint32 i;
    quint8  c;

    message_->clearAttachments();
    delete current_;
    current_ = 0;

    if (!device_->open(QIODevice::ReadOnly)) {
        qDebug() << "Couldn't open device";
        return false;
    }

    stream_.setDevice(device_);
    stream_.setByteOrder(QDataStream::LittleEndian);
    stream_ >> i;
    if (i == TNEF_SIGNATURE) {
        stream_ >> u;
        qDebug().nospace() << "Attachment cross reference key: 0x"
                           << hex << qSetFieldWidth(4) << qSetPadChar(QLatin1Char('0')) << u;
        //qDebug() << "stream:" << device_->pos();
        while (!stream_.atEnd()) {
            stream_ >> c;
            switch (c) {
            case LVL_MESSAGE:
                if (!decodeMessage()) {
                    goto end;
                }
                break;
            case LVL_ATTACHMENT:
                if (!decodeAttachment()) {
                    goto end;
                }
                break;
            default:
                qDebug() << "Unknown Level:" << c << ", at offset" << device_->pos();
                goto end;
            }
        }
        if (current_) {
            checkCurrent(attATTACHDATA);    // this line has the effect to append the
            // attachment, if it has data. If not it does
            // nothing, and the attachment will be discarded
            delete current_;
            current_ = 0;
        }
        return true;
    } else {
        qDebug() << "This is not a TNEF file";
    end:
        device_->close();
        return false;
    }
}

bool KTNEFParser::extractFile(const QString &filename) const
{
    KTNEFAttach *att = d->message_->attachment(filename);
    if (!att) {
        return false;
    }
    return d->extractAttachmentTo(att, d->defaultdir_);
}

bool KTNEFParser::ParserPrivate::extractAttachmentTo(KTNEFAttach *att,
        const QString &dirname)
{
    QString filename = dirname + QLatin1Char('/');
    if (!att->fileName().isEmpty()) {
        filename += att->fileName();
    } else {
        filename += att->name();
    }
    if (filename.endsWith(QLatin1Char('/'))) {
        return false;
    }

    if (!device_->isOpen()) {
        return false;
    }
    if (!device_->seek(att->offset())) {
        return false;
    }
    QSaveFile outfile(filename);
    if (!outfile.open(QIODevice::WriteOnly)) {
        return false;
    }

    quint32 len = att->size(), sz(16384);
    int     n(0);
    char    *buf = new char[sz];
    bool    ok(true);
    while (ok && len > 0) {
        n = device_->read(buf, qMin(sz, len));
        if (n < 0) {
            ok = false;
        } else {
            len -= n;
            if (outfile.write(buf, n) != n) {
                ok = false;
            }
        }
    }
    outfile.commit();
    delete [] buf;

    return ok;
}

bool KTNEFParser::extractAll()
{
    QList<KTNEFAttach *> l = d->message_->attachmentList();
    QList<KTNEFAttach *>::const_iterator it = l.constBegin();
    for (; it != l.constEnd(); ++it) {
        if (!d->extractAttachmentTo(*it, d->defaultdir_)) {
            return false;
        }
    }
    return true;
}

bool KTNEFParser::extractFileTo(const QString &filename,
                                const QString &dirname) const
{
    qDebug() << "Extracting attachment: filename="
             << filename << ", dir=" << dirname;
    KTNEFAttach *att = d->message_->attachment(filename);
    if (!att) {
        return false;
    }
    return d->extractAttachmentTo(att, dirname);
}

bool KTNEFParser::openFile(const QString &filename) const
{
    d->deleteDevice();
    delete d->message_;
    d->message_ = new KTNEFMessage();
    d->device_ = new QFile(filename);
    d->deleteDevice_ = true;
    return d->parseDevice();
}

bool KTNEFParser::openDevice(QIODevice *device)
{
    d->deleteDevice();
    d->device_ = device;
    return d->parseDevice();
}

void KTNEFParser::ParserPrivate::checkCurrent(int key)
{
    if (!current_) {
        current_ = new KTNEFAttach();
    } else {
        if (current_->attributes().contains(key)) {
            if (current_->offset() >= 0) {
                if (current_->name().isEmpty()) {
                    current_->setName(QLatin1String("Unnamed"));
                }
                if (current_->mimeTag().isEmpty()) {
                    // No mime type defined in the TNEF structure,
                    // try to find it from the attachment filename
                    // and/or content (using at most 32 bytes)
                    QMimeType mimetype;
                    QMimeDatabase db;
                    if (!current_->fileName().isEmpty()) {
                        mimetype = db.mimeTypeForFile(current_->fileName(), QMimeDatabase::MatchExtension);
                    }
                    if (!mimetype.isValid()) {
                        return; // FIXME
                    }
                    if (mimetype.name() == QLatin1String("application/octet-stream") &&
                            current_->size() > 0) {
                        int oldOffset = device_->pos();
                        QByteArray buffer(qMin(32, current_->size()), '\0');
                        device_->seek(current_->offset());
                        device_->read(buffer.data(), buffer.size());
                        mimetype = db.mimeTypeForData(buffer);
                        device_->seek(oldOffset);
                    }
                    current_->setMimeTag(mimetype.name());
                }
                message_->addAttachment(current_);
                current_ = 0;
            } else {
                // invalid attachment, skip it
                delete current_;
                current_ = 0;
            }
            current_ = new KTNEFAttach();
        }
    }
}

//------------------------------------------------------------------------------

//@cond IGNORE
#define ALIGN( n, b ) if ( n & ( b-1 ) ) { n = ( n + b ) & ~( b-1 ); }
#define ISVECTOR( m ) ( ( ( m ).type & 0xF000 ) == MAPI_TYPE_VECTOR )

void clearMAPIName(MAPI_value &mapi)
{
    mapi.name.value.clear();
}

void clearMAPIValue(MAPI_value &mapi, bool clearName)
{
    mapi.value.clear();
    if (clearName) {
        clearMAPIName(mapi);
    }
}

QDateTime formatTime(quint32 lowB, quint32 highB)
{
    QDateTime dt;
    quint64 u64;
    u64 = highB;
    u64 <<= 32;
    u64 |= lowB;
    u64 -= 116444736000000000LL;
    u64 /= 10000000;
    if (u64 <= 0xffffffffU) {
        dt.setTime_t((unsigned int)u64);
    } else {
        qWarning().nospace() << "Invalid date: low byte="
                             << showbase << qSetFieldWidth(8) << qSetPadChar(QLatin1Char('0'))
                             << lowB << ", high byte=" << highB;
        dt.setTime_t(0xffffffffU);
    }
    return dt;
}

QString formatRecipient(const QMap<int, KTnef::KTNEFProperty *> &props)
{
    QString s, dn, addr, t;
    QMap<int, KTnef::KTNEFProperty *>::ConstIterator it;
    if ((it = props.find(0x3001)) != props.end()) {
        dn = (*it)->valueString();
    }
    if ((it = props.find(0x3003)) != props.end()) {
        addr = (*it)->valueString();
    }
    if ((it = props.find(0x0C15)) != props.end()) {
        switch ((*it)->value().toInt()) {
        case 0:
            t = QLatin1String("From:");
            break;
        case 1:
            t = QLatin1String("To:");
            break;
        case 2:
            t = QLatin1String("Cc:");
            break;
        case 3:
            t = QLatin1String("Bcc:");
            break;
        }
    }
    if (!t.isEmpty()) {
        s.append(t);
    }
    if (!dn.isEmpty()) {
        s.append(QLatin1Char(' ') + dn);
    }
    if (!addr.isEmpty() && addr != dn) {
        s.append(QLatin1String(" <") + addr + QLatin1Char('>'));
    }

    return s.trimmed();
}

QDateTime readTNEFDate(QDataStream &stream)
{
    // 14-bytes long
    quint16 y, m, d, hh, mm, ss, dm;
    stream >> y >> m >> d >> hh >> mm >> ss >> dm;
    return QDateTime(QDate(y, m, d), QTime(hh, mm, ss));
}

QString readTNEFAddress(QDataStream &stream)
{
    quint16 totalLen, strLen, addrLen;
    QString s;
    stream >> totalLen >> totalLen >> strLen >> addrLen;
    s.append(readMAPIString(stream, false, false, strLen));
    s.append(QLatin1String(" <"));
    s.append(readMAPIString(stream, false, false, addrLen));
    s.append(QLatin1String(">"));
    quint8 c;
    for (int i = 8 + strLen + addrLen; i < totalLen; i++) {
        stream >> c;
    }
    return s;
}

QByteArray readTNEFData(QDataStream &stream, quint32 len)
{
    QByteArray array(len, '\0');
    if (len > 0) {
        stream.readRawData(array.data(), len);
    }
    return array;
}

QVariant readTNEFAttribute(QDataStream &stream, quint16 type, quint32 len)
{
    switch (type) {
    case atpTEXT:
    case atpSTRING:
        return readMAPIString(stream, false, false, len);
    case atpDATE:
        return readTNEFDate(stream);
    default:
        return readTNEFData(stream, len);
    }
}

QString readMAPIString(QDataStream &stream, bool isUnicode, bool align,
                       int len_)
{
    quint32 len;
    char *buf = 0;
    if (len_ == -1) {
        stream >> len;
    } else {
        len = len_;
    }
    quint32 fullLen = len;
    if (align) {
        ALIGN(fullLen, 4);
    }
    buf = new char[ len ];
    stream.readRawData(buf, len);
    quint8 c;
    for (uint i = len; i < fullLen; i++) {
        stream >> c;
    }
    QString res;
    if (isUnicode) {
        res = QString::fromUtf16((const unsigned short *)buf);
    } else {
        res = QString::fromLocal8Bit(buf);
    }
    delete [] buf;
    return res;
}

quint16 readMAPIValue(QDataStream &stream, MAPI_value &mapi)
{
    quint32 d;

    clearMAPIValue(mapi);
    stream >> d;
    mapi.type = (d & 0x0000FFFF);
    mapi.tag = ((d & 0xFFFF0000) >> 16);
    if (mapi.tag >= 0x8000 && mapi.tag <= 0xFFFE) {
        // skip GUID
        stream >> d >> d >> d >> d;
        // name type
        stream >> mapi.name.type;
        // name
        if (mapi.name.type == 0) {
            uint tmp;
            stream >> tmp;
            mapi.name.value.setValue(tmp);
        } else if (mapi.name.type == 1) {
            mapi.name.value.setValue(readMAPIString(stream, true));
        }
    }

    int n = 1;
    QVariant value;
    if (ISVECTOR(mapi)) {
        stream >> n;
        mapi.value = QList<QVariant>();
    }
    for (int i = 0; i < n; i++) {
        value.clear();
        switch (mapi.type & 0x0FFF) {
        case MAPI_TYPE_UINT16:
            stream >> d;
            value.setValue(d & 0x0000FFFF);
            break;
        case MAPI_TYPE_BOOLEAN:
        case MAPI_TYPE_ULONG: {
            uint tmp;
            stream >> tmp;
            value.setValue(tmp);
        }
        break;
        case MAPI_TYPE_FLOAT:
            // FIXME: Don't we have to set the value here
            stream >> d;
            break;
        case MAPI_TYPE_DOUBLE: {
            double tmp;
            stream >> tmp;
            value.setValue(tmp);
        }
        break;
        case MAPI_TYPE_TIME: {
            quint32 lowB, highB;
            stream >> lowB >> highB;
            value = formatTime(lowB, highB);
        }
        break;
        case MAPI_TYPE_USTRING:
        case MAPI_TYPE_STRING8:
            // in case of a vector'ed value, the number of elements
            // has already been read in the upper for-loop
            if (ISVECTOR(mapi)) {
                d = 1;
            } else {
                stream >> d;
            }
            for (uint i = 0; i < d; i++) {
                value.clear();
                value.setValue(readMAPIString(stream, (mapi.type & 0x0FFF) == MAPI_TYPE_USTRING));
            }
            break;
        case MAPI_TYPE_OBJECT:
        case MAPI_TYPE_BINARY:
            if (ISVECTOR(mapi)) {
                d = 1;
            } else {
                stream >> d;
            }
            for (uint i = 0; i < d; i++) {
                value.clear();
                quint32 len;
                stream >> len;
                value = QByteArray(len, '\0');
                if (len > 0) {
                    int fullLen = len;
                    ALIGN(fullLen, 4);
                    stream.readRawData(value.toByteArray().data(), len);
                    quint8 c;
                    for (int i = len; i < fullLen; i++) {
                        stream >> c;
                    }
                    // FIXME: Shouldn't we do something with the value???
                }
            }
            break;
        default:
            mapi.type = MAPI_TYPE_NONE;
            break;
        }
        if (ISVECTOR(mapi)) {
            QList <QVariant> lst = mapi.value.toList();
            lst << value;
            mapi.value.setValue(lst);
        } else {
            mapi.value = value;
        }
    }
    return mapi.tag;
}
//@endcond

bool KTNEFParser::ParserPrivate::readMAPIProperties(QMap<int, KTNEFProperty *> &props,
        KTNEFAttach *attach)
{
    quint32       n;
    MAPI_value    mapi;
    KTNEFProperty *p;
    QMap<int, KTNEFProperty *>::ConstIterator it;
    bool foundAttachment = false;

    // some initializations
    mapi.type = MAPI_TYPE_NONE;
    mapi.value.clear();

    // get number of properties
    stream_ >> n;
    qDebug() << "MAPI Properties:" << n;
    for (uint i = 0; i < n; i++) {
        if (stream_.atEnd()) {
            clearMAPIValue(mapi);
            return false;
        }
        readMAPIValue(stream_, mapi);
        if (mapi.type == MAPI_TYPE_NONE) {
            qDebug().nospace() << "MAPI unsupported:         tag="
                               << hex << mapi.tag << ", type=" << mapi.type;
            clearMAPIValue(mapi);
            return false;
        }
        int key = mapi.tag;
        switch (mapi.tag) {
        case MAPI_TAG_DATA: {
            if (mapi.type == MAPI_TYPE_OBJECT && attach) {
                QByteArray data = mapi.value.toByteArray();
                int len = data.size();
                ALIGN(len, 4);
                device_->seek(device_->pos() - len);
                quint32 interface_ID;
                stream_ >> interface_ID;
                if (interface_ID == MAPI_IID_IMessage) {
                    // embedded TNEF file
                    attach->unsetDataParser();
                    attach->setOffset(device_->pos() + 12);
                    attach->setSize(data.size() - 16);
                    attach->setMimeTag(QLatin1String("application/vnd.ms-tnef"));
                    attach->setDisplayName(QLatin1String("Embedded Message"));
                    qDebug() << "MAPI Embedded Message: size=" << data.size();
                }
                device_->seek(device_->pos() + (len - 4));
                break;
            } else if (mapi.type == MAPI_TYPE_BINARY && attach && attach->offset() < 0) {
                foundAttachment = true;
                int len = mapi.value.toByteArray().size();
                ALIGN(len, 4);
                attach->setSize(len);
                attach->setOffset(device_->pos() - len);
                attach->addAttribute(attATTACHDATA, atpBYTE, QString::fromLatin1("< size=%1 >").arg(len), false);
            }
        }
        qDebug() << "MAPI data: size=" << mapi.value.toByteArray().size();
        break;
        default: {
            QString mapiname = QLatin1String("");
            if (mapi.tag >= 0x8000 && mapi.tag <= 0xFFFE) {
                if (mapi.name.type == 0) {
                    mapiname = QString().sprintf(" [name = 0x%04x]", mapi.name.value.toUInt());
                } else {
                    mapiname = QString::fromLatin1(" [name = %1]").arg(mapi.name.value.toString());
                }
            }
            switch (mapi.type & 0x0FFF) {
            case MAPI_TYPE_UINT16:
                qDebug().nospace() << "(tag="
                                   << hex << mapi.tag
                                   << ") MAPI short" <<  mapiname.toLatin1().data()
                                   << ":" << hex << mapi.value.toUInt();
                break;
            case MAPI_TYPE_ULONG:
                qDebug().nospace() << "(tag="
                                   << hex << mapi.tag
                                   << ") MAPI long" <<  mapiname.toLatin1().data()
                                   << ":" << hex << mapi.value.toUInt();
                break;
            case MAPI_TYPE_BOOLEAN:
                qDebug().nospace() << "(tag="
                                   << hex << mapi.tag
                                   << ") MAPI boolean" <<  mapiname.toLatin1().data()
                                   << ":" << mapi.value.toBool();
                break;
            case MAPI_TYPE_TIME:
                qDebug().nospace() << "(tag="
                                   << hex << mapi.tag
                                   << ") MAPI time" <<  mapiname.toLatin1().data()
                                   << ":" << mapi.value.toString().toLatin1().data();
                break;
            case MAPI_TYPE_USTRING:
            case MAPI_TYPE_STRING8:
                qDebug().nospace() << "(tag="
                                   << hex << mapi.tag
                                   << ") MAPI string" <<  mapiname.toLatin1().data()
                                   << ":size=" << mapi.value.toByteArray().size()
                                   << mapi.value.toString();
                break;
            case MAPI_TYPE_BINARY:
                qDebug().nospace() << "(tag="
                                   << hex << mapi.tag
                                   << ") MAPI binary" <<  mapiname.toLatin1().data()
                                   << ":size=" << mapi.value.toByteArray().size();
                break;
            }
        }
        break;
        }
        // do not remove potential existing similar entry
        if ((it = props.constFind(key)) == props.constEnd()) {
            p = new KTNEFProperty(key, (mapi.type & 0x0FFF),
                                  mapi.value, mapi.name.value);
            props[ p->key() ] = p;
        }
        //qDebug() << "stream:" << device_->pos();
    }

    if (foundAttachment && attach) {
        attach->setIndex(attach->property(MAPI_TAG_INDEX).toUInt());
        attach->setDisplaySize(attach->property(MAPI_TAG_SIZE).toUInt());
        QString str = attach->property(MAPI_TAG_DISPLAYNAME).toString();
        if (!str.isEmpty()) {
            attach->setDisplayName(str);
        }
        attach->setFileName(attach->property(MAPI_TAG_FILENAME).toString());
        str = attach->property(MAPI_TAG_MIMETAG).toString();
        if (!str.isEmpty()) {
            attach->setMimeTag(str);
        }
        attach->setExtension(attach->property(MAPI_TAG_EXTENSION).toString());
        if (attach->name().isEmpty()) {
            attach->setName(attach->fileName());
        }
    }

    return true;
}
