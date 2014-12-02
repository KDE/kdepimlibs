/*
  kmime_util.cpp

  KMime, the KDE Internet mail/usenet news message library.
  Copyright (c) 2001 the KMime authors.
  See file AUTHORS for details

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

#include "kmime_util.h"
#include "kmime_util_p.h"

#include "kmime_charfreq.h"
#include "kmime_header_parsing.h"
#include "kmime_message.h"
#include "kmime_warning.h"

#include <config-kmime.h>
// #include <kdefakes.h> // for strcasestr
#include <KLocale>

#include <klocalizedstring.h>
#include <kcharsets.h>
#include <qdebug.h>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QTextCodec>

#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

using namespace KMime;

namespace KMime
{

QList<QByteArray> c_harsetCache;
QList<QByteArray> l_anguageCache;
QString f_allbackCharEnc;
bool u_seOutlookEncoding = false;

QByteArray cachedCharset(const QByteArray &name)
{
    foreach (const QByteArray &charset, c_harsetCache) {
        if (qstricmp(name.data(), charset.data()) == 0) {
            return charset;
        }
    }

    c_harsetCache.append(name.toUpper());
    //qDebug() << "KNMimeBase::cachedCharset() number of cs" << c_harsetCache.count();
    return c_harsetCache.last();
}

QByteArray cachedLanguage(const QByteArray &name)
{
    foreach (const QByteArray &language, l_anguageCache) {
        if (qstricmp(name.data(), language.data()) == 0) {
            return language;
        }
    }

    l_anguageCache.append(name.toUpper());
    //qDebug() << "KNMimeBase::cachedCharset() number of cs" << c_harsetCache.count();
    return l_anguageCache.last();
}

bool isUsAscii(const QString &s)
{
    uint sLength = s.length();
    for (uint i = 0; i < sLength; i++) {
        if (s.at(i).toLatin1() <= 0) {     // c==0: non-latin1, c<0: non-us-ascii
            return false;
        }
    }
    return true;
}

QString nameForEncoding(Headers::contentEncoding enc)
{
    switch (enc) {
    case Headers::CE7Bit: return QString::fromLatin1("7bit");
    case Headers::CE8Bit: return QString::fromLatin1("8bit");
    case Headers::CEquPr: return QString::fromLatin1("quoted-printable");
    case Headers::CEbase64: return QString::fromLatin1("base64");
    case Headers::CEuuenc: return QString::fromLatin1("uuencode");
    case Headers::CEbinary: return QString::fromLatin1("binary");
    default: return QString::fromLatin1("unknown");
    }
}

QList<Headers::contentEncoding> encodingsForData(const QByteArray &data)
{
    QList<Headers::contentEncoding> allowed;
    CharFreq cf(data);

    switch (cf.type()) {
    case CharFreq::SevenBitText:
        allowed << Headers::CE7Bit;
    case CharFreq::EightBitText:
        allowed << Headers::CE8Bit;
    case CharFreq::SevenBitData:
        if (cf.printableRatio() > 5.0 / 6.0) {
            // let n the length of data and p the number of printable chars.
            // Then base64 \approx 4n/3; qp \approx p + 3(n-p)
            // => qp < base64 iff p > 5n/6.
            allowed << Headers::CEquPr;
            allowed << Headers::CEbase64;
        } else {
            allowed << Headers::CEbase64;
            allowed << Headers::CEquPr;
        }
        break;
    case CharFreq::EightBitData:
        allowed << Headers::CEbase64;
        break;
    case CharFreq::None:
    default:
        Q_ASSERT(false);
    }

    return allowed;
}

// "(),.:;<>@[\]
const uchar specialsMap[16] = {
    0x00, 0x00, 0x00, 0x00, // CTLs
    0x20, 0xCA, 0x00, 0x3A, // SPACE ... '?'
    0x80, 0x00, 0x00, 0x1C, // '@' ... '_'
    0x00, 0x00, 0x00, 0x00  // '`' ... DEL
};

// "(),:;<>@[\]/=?
const uchar tSpecialsMap[16] = {
    0x00, 0x00, 0x00, 0x00, // CTLs
    0x20, 0xC9, 0x00, 0x3F, // SPACE ... '?'
    0x80, 0x00, 0x00, 0x1C, // '@' ... '_'
    0x00, 0x00, 0x00, 0x00  // '`' ... DEL
};

// all except specials, CTLs, SPACE.
const uchar aTextMap[16] = {
    0x00, 0x00, 0x00, 0x00,
    0x5F, 0x35, 0xFF, 0xC5,
    0x7F, 0xFF, 0xFF, 0xE3,
    0xFF, 0xFF, 0xFF, 0xFE
};

// all except tspecials, CTLs, SPACE.
const uchar tTextMap[16] = {
    0x00, 0x00, 0x00, 0x00,
    0x5F, 0x36, 0xFF, 0xC0,
    0x7F, 0xFF, 0xFF, 0xE3,
    0xFF, 0xFF, 0xFF, 0xFE
};

// none except a-zA-Z0-9!*+-/
const uchar eTextMap[16] = {
    0x00, 0x00, 0x00, 0x00,
    0x40, 0x35, 0xFF, 0xC0,
    0x7F, 0xFF, 0xFF, 0xE0,
    0x7F, 0xFF, 0xFF, 0xE0
};

void setFallbackCharEncoding(const QString &fallbackCharEnc)
{
    f_allbackCharEnc = fallbackCharEnc;
}

QString fallbackCharEncoding()
{
    return f_allbackCharEnc;
}

void setUseOutlookAttachmentEncoding(bool violateStandard)
{
    u_seOutlookEncoding = violateStandard;
}

bool useOutlookAttachmentEncoding()
{
    return u_seOutlookEncoding;
}

QString decodeRFC2047String(const QByteArray &src, QByteArray &usedCS,
                            const QByteArray &defaultCS, bool forceCS)
{
    QByteArray result;
    QByteArray spaceBuffer;
    const char *scursor = src.constData();
    const char *send = scursor + src.length();
    bool onlySpacesSinceLastWord = false;

    while (scursor != send) {
        // space
        if (isspace(*scursor) && onlySpacesSinceLastWord) {
            spaceBuffer += *scursor++;
            continue;
        }

        // possible start of an encoded word
        if (*scursor == '=') {
            QByteArray language;
            QString decoded;
            ++scursor;
            const char *start = scursor;
            if (HeaderParsing::parseEncodedWord(scursor, send, decoded, language, usedCS, defaultCS, forceCS)) {
                result += decoded.toUtf8();
                onlySpacesSinceLastWord = true;
                spaceBuffer.clear();
            } else {
                if (onlySpacesSinceLastWord) {
                    result += spaceBuffer;
                    onlySpacesSinceLastWord = false;
                }
                result += '=';
                scursor = start; // reset cursor after parsing failure
            }
            continue;
        } else {
            // unencoded data
            if (onlySpacesSinceLastWord) {
                result += spaceBuffer;
                onlySpacesSinceLastWord = false;
            }
            result += *scursor;
            ++scursor;
        }
    }
    // If there are any chars that couldn't be decoded in UTF-8,
    //  use the fallback charset if it exists
    const QString tryUtf8 = QString::fromUtf8(result);
    if (tryUtf8.contains(0xFFFD) && !f_allbackCharEnc.isEmpty()) {
        QTextCodec *codec = KCharsets::charsets()->codecForName(f_allbackCharEnc);
        return codec->toUnicode(result);
    } else {
        return tryUtf8;
    }
}

QString decodeRFC2047String(const QByteArray &src)
{
    QByteArray usedCS;
    return decodeRFC2047String(src, usedCS, "utf-8", false);
}

static const char reservedCharacters[] = "\"()<>@,.;:\\[]=";

QByteArray encodeRFC2047String(const QString &src, const QByteArray &charset,
                               bool addressHeader, bool allow8BitHeaders)
{
    QByteArray result;
    int start = 0, end = 0;
    bool nonAscii = false, ok = true, useQEncoding = false;

    // fromLatin1() is safe here, codecForName() uses toLatin1() internally
    const QTextCodec *codec = KCharsets::charsets()->codecForName(QString::fromLatin1(charset), ok);

    QByteArray usedCS;
    if (!ok) {
        //no codec available => try local8Bit and hope the best ;-)
        usedCS = KLocale::global()->encoding();
        codec = KCharsets::charsets()->codecForName(QString::fromLatin1(usedCS), ok);
    } else {
        Q_ASSERT(codec);
        if (charset.isEmpty()) {
            usedCS = codec->name();
        } else {
            usedCS = charset;
        }
    }

    QTextCodec::ConverterState converterState(QTextCodec::IgnoreHeader);
    QByteArray encoded8Bit = codec->fromUnicode(src.constData(), src.length(), &converterState);
    if (converterState.invalidChars > 0) {
        usedCS = "utf-8";
        codec = QTextCodec::codecForName(usedCS);
        encoded8Bit = codec->fromUnicode(src);
    }

    if (usedCS.contains("8859-")) {     // use "B"-Encoding for non iso-8859-x charsets
        useQEncoding = true;
    }

    if (allow8BitHeaders) {
        return encoded8Bit;
    }

    uint encoded8BitLength = encoded8Bit.length();
    for (unsigned int i = 0; i < encoded8BitLength; i++) {
        if (encoded8Bit[i] == ' ') {   // encoding starts at word boundaries
            start = i + 1;
        }

        // encode escape character, for japanese encodings...
        if (((signed char)encoded8Bit[i] < 0) || (encoded8Bit[i] == '\033') ||
                (addressHeader && (strchr("\"()<>@,.;:\\[]=", encoded8Bit[i]) != 0))) {
            end = start;   // non us-ascii char found, now we determine where to stop encoding
            nonAscii = true;
            break;
        }
    }

    if (nonAscii) {
        while ((end < encoded8Bit.length()) && (encoded8Bit[end] != ' ')) {
            // we encode complete words
            end++;
        }

        for (int x = end; x < encoded8Bit.length(); x++) {
            if (((signed char)encoded8Bit[x] < 0) || (encoded8Bit[x] == '\033') ||
                    (addressHeader && (strchr(reservedCharacters, encoded8Bit[x]) != 0))) {
                end = x;     // we found another non-ascii word

                while ((end < encoded8Bit.length()) && (encoded8Bit[end] != ' ')) {
                    // we encode complete words
                    end++;
                }
            }
        }

        result = encoded8Bit.left(start) + "=?" + usedCS;

        if (useQEncoding) {
            result += "?Q?";

            char c, hexcode;// "Q"-encoding implementation described in RFC 2047
            for (int i = start; i < end; i++) {
                c = encoded8Bit[i];
                if (c == ' ') {   // make the result readable with not MIME-capable readers
                    result += '_';
                } else {
                    if (((c >= 'a') && (c <= 'z')) ||        // paranoid mode, encode *all* special chars to avoid problems
                            ((c >= 'A') && (c <= 'Z')) ||        // with "From" & "To" headers
                            ((c >= '0') && (c <= '9'))) {
                        result += c;
                    } else {
                        result += '=';                 // "stolen" from KMail ;-)
                        hexcode = ((c & 0xF0) >> 4) + 48;
                        if (hexcode >= 58) {
                            hexcode += 7;
                        }
                        result += hexcode;
                        hexcode = (c & 0x0F) + 48;
                        if (hexcode >= 58) {
                            hexcode += 7;
                        }
                        result += hexcode;
                    }
                }
            }
        } else {
            result += "?B?" + encoded8Bit.mid(start, end - start).toBase64();
        }

        result += "?=";
        result += encoded8Bit.right(encoded8Bit.length() - end);
    } else {
        result = encoded8Bit;
    }

    return result;
}

QByteArray encodeRFC2047Sentence(const QString &src, const QByteArray &charset)
{
    QByteArray result;
    QList<QChar> splitChars;
    splitChars << QLatin1Char(',') << QLatin1Char('\"') << QLatin1Char(';') << QLatin1Char('\\');
    const QChar *ch = src.constData();
    const int length = src.length();
    int pos = 0;
    int wordStart = 0;

    //qDebug() << "Input:" << src;
    // Loop over all characters of the string.
    // When encountering a split character, RFC-2047-encode the word before it, and add it to the result.
    while (pos < length) {
        //qDebug() << "Pos:" << pos << "Result:" << result << "Char:" << ch->toLatin1();
        const bool isAscii = ch->unicode() < 127;
        const bool isReserved = (strchr(reservedCharacters, ch->toLatin1()) != 0);
        if (isAscii && isReserved) {
            const int wordSize = pos - wordStart;
            if (wordSize > 0) {
                const QString word = src.mid(wordStart, wordSize);
                result += encodeRFC2047String(word, charset);
            }

            result += ch->toLatin1();
            wordStart = pos + 1;
        }
        ch++;
        pos++;
    }

    // Encode the last word
    const int wordSize = pos - wordStart;
    if (wordSize > 0) {
        const QString word = src.mid(wordStart, pos - wordStart);
        result += encodeRFC2047String(word, charset);
    }

    return result;
}

//-----------------------------------------------------------------------------
QByteArray encodeRFC2231String(const QString &str, const QByteArray &charset)
{
    if (str.isEmpty()) {
        return QByteArray();
    }

    const QTextCodec *codec = KCharsets::charsets()->codecForName(QString::fromLatin1(charset));
    QByteArray latin;
    if (charset == "us-ascii") {
        latin = str.toLatin1();
    } else if (codec) {
        latin = codec->fromUnicode(str);
    } else {
        latin = str.toLocal8Bit();
    }

    char *l;
    for (l = latin.data(); *l; ++l) {
        if (((*l & 0xE0) == 0) || (*l & 0x80)) {
            // *l is control character or 8-bit char
            break;
        }
    }
    if (!*l) {
        return latin;
    }

    QByteArray result = charset + "''";
    for (l = latin.data(); *l; ++l) {
        bool needsQuoting = (*l & 0x80) || (*l == '%');
        if (!needsQuoting) {
            const QByteArray especials = "()<>@,;:\"/[]?.= \033";
            int len = especials.length();
            for (int i = 0; i < len; i++) {
                if (*l == especials[i]) {
                    needsQuoting = true;
                    break;
                }
            }
        }
        if (needsQuoting) {
            result += '%';
            unsigned char hexcode;
            hexcode = ((*l & 0xF0) >> 4) + 48;
            if (hexcode >= 58) {
                hexcode += 7;
            }
            result += hexcode;
            hexcode = (*l & 0x0F) + 48;
            if (hexcode >= 58) {
                hexcode += 7;
            }
            result += hexcode;
        } else {
            result += *l;
        }
    }
    return result;
}

//-----------------------------------------------------------------------------
QString decodeRFC2231String(const QByteArray &str, QByteArray &usedCS, const QByteArray &defaultCS,
                            bool forceCS)
{
    int p = str.indexOf('\'');
    if (p < 0) {
        return KCharsets::charsets()->codecForName(QString::fromLatin1(defaultCS))->toUnicode(str);
    }

    QByteArray charset = str.left(p);

    QByteArray st = str.mid(str.lastIndexOf('\'') + 1);

    char ch, ch2;
    p = 0;
    while (p < (int)st.length()) {
        if (st.at(p) == 37) {
            // Only try to decode the percent-encoded character if the percent sign
            // is really followed by two other characters, see testcase at bug 163024
            if (p + 2 < st.length()) {
                ch = st.at(p + 1) - 48;
                if (ch > 16) {
                    ch -= 7;
                }
                ch2 = st.at(p + 2) - 48;
                if (ch2 > 16) {
                    ch2 -= 7;
                }
                st[p] = ch * 16 + ch2;
                st.remove(p + 1, 2);
            }
        }
        p++;
    }
    qDebug() << "Got pre-decoded:" << st;
    QString result;
    const QTextCodec *charsetcodec = KCharsets::charsets()->codecForName(QString::fromLatin1(charset));
    if (!charsetcodec || forceCS) {
        charsetcodec = KCharsets::charsets()->codecForName(QString::fromLatin1(defaultCS));
    }

    usedCS = charsetcodec->name();
    return charsetcodec->toUnicode(st);
}

QString decodeRFC2231String(const QByteArray &src)
{
    QByteArray usedCS;
    return decodeRFC2231String(src, usedCS, "utf-8", false);
}

QByteArray uniqueString()
{
    static const char chars[] = "0123456789abcdefghijklmnopqrstuvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    time_t now;
    char p[11];
    int pos, ran;
    unsigned int timeval;

    p[10] = '\0';
    now = time(0);
    ran = 1 + (int)(1000.0 * rand() / (RAND_MAX + 1.0));
    timeval = (now / ran) + getpid();

    for (int i = 0; i < 10; i++) {
        pos = (int)(61.0 * rand() / (RAND_MAX + 1.0));
        //qDebug() << pos;
        p[i] = chars[pos];
    }

    QByteArray ret;
    ret.setNum(timeval);
    ret += '.';
    ret += p;

    return ret;
}

QByteArray multiPartBoundary()
{
    return "nextPart" + uniqueString();
}

QByteArray unfoldHeader(const QByteArray &header)
{
    QByteArray result;
    if (header.isEmpty()) {
        return result;
    }

    int pos = 0, foldBegin = 0, foldMid = 0, foldEnd = 0;
    while ((foldMid = header.indexOf('\n', pos)) >= 0) {
        foldBegin = foldEnd = foldMid;
        // find the first space before the line-break
        while (foldBegin > 0) {
            if (!QChar::fromLatin1(header[foldBegin - 1]).isSpace()) {
                break;
            }
            --foldBegin;
        }
        // find the first non-space after the line-break
        while (foldEnd <= header.length() - 1) {
            if (QChar::fromLatin1(header[foldEnd]).isSpace()) {
                ++foldEnd;
            } else if (foldEnd > 0 && header[foldEnd - 1] == '\n' &&
                       header[foldEnd] == '=' && foldEnd + 2 < header.length() &&
                       ((header[foldEnd + 1] == '0' &&
                         header[foldEnd + 2] == '9') ||
                        (header[foldEnd + 1] == '2' &&
                         header[foldEnd + 2] == '0'))) {
                // bug #86302: malformed header continuation starting with =09/=20
                foldEnd += 3;
            } else {
                break;
            }
        }

        result += header.mid(pos, foldBegin - pos);
        if (foldEnd < header.length() - 1) {
            result += ' ';
        }
        pos = foldEnd;
    }
    const int len = header.length();
    if (len > pos) {
        result += header.mid(pos, len - pos);
    }
    return result;
}

int findHeaderLineEnd(const QByteArray &src, int &dataBegin, bool *folded)
{
    int end = dataBegin;
    int len = src.length() - 1;

    if (folded) {
        *folded = false;
    }

    if (dataBegin < 0) {
        // Not found
        return -1;
    }

    if (dataBegin > len) {
        // No data available
        return len + 1;
    }

    // If the first line contains nothing, but the next line starts with a space
    // or a tab, that means a stupid mail client has made the first header field line
    // entirely empty, and has folded the rest to the next line(s).
    if (src.at(end) == '\n' && end + 1 < len &&
            (src[end + 1] == ' ' || src[end + 1] == '\t')) {

        // Skip \n and first whitespace
        dataBegin += 2;
        end += 2;
    }

    if (src.at(end) != '\n') {      // check if the header is not empty
        while (true) {
            end = src.indexOf('\n', end + 1);
            if (end == -1 || end == len) {
                // end of string
                break;
            } else if (src[end + 1] == ' ' || src[end + 1] == '\t' ||
                       (src[end + 1] == '=' && end + 3 <= len &&
                        ((src[end + 2] == '0' && src[end + 3] == '9') ||
                         (src[end + 2] == '2' && src[end + 3] == '0')))) {
                // next line is header continuation or starts with =09/=20 (bug #86302)
                if (folded) {
                    *folded = true;
                }
            } else {
                // end of header (no header continuation)
                break;
            }
        }
    }

    if (end < 0) {
        end = len + 1; //take the rest of the string
    }
    return end;
}

int indexOfHeader(const QByteArray &src, const QByteArray &name, int &end, int &dataBegin, bool *folded)
{
    QByteArray n = name;
    n.append(':');
    int begin = -1;

    if (qstrnicmp(n.constData(), src.constData(), n.length()) == 0) {
        begin = 0;
    } else {
        n.prepend('\n');
        const char *p = strcasestr(src.constData(), n.constData());
        if (!p) {
            begin = -1;
        } else {
            begin = p - src.constData();
            ++begin;
        }
    }

    if (begin > -1) {       //there is a header with the given name
        dataBegin = begin + name.length() + 1; //skip the name
        // skip the usual space after the colon
        if (src.at(dataBegin) == ' ') {
            ++dataBegin;
        }
        end = findHeaderLineEnd(src, dataBegin, folded);
        return begin;

    } else {
        end = -1;
        dataBegin = -1;
        return -1; //header not found
    }
}

QByteArray extractHeader(const QByteArray &src, const QByteArray &name)
{
    int begin, end;
    bool folded;
    QByteArray result;

    if (src.isEmpty() || indexOfHeader(src, name, end, begin, &folded) < 0) {
        return result;
    }

    if (begin >= 0) {
        if (!folded) {
            result = src.mid(begin, end - begin);
        } else {
            if (end > begin) {
                QByteArray hdrValue = src.mid(begin, end - begin);
                result = unfoldHeader(hdrValue);
            }
        }
    }
    return result;
}

QList<QByteArray> extractHeaders(const QByteArray &src, const QByteArray &name)
{
    int begin, end;
    bool folded;
    QList<QByteArray> result;
    QByteArray copySrc(src);

    if (indexOfHeader(copySrc, name, end, begin, &folded) < 0) {
        return result;
    }

    while (begin >= 0) {
        if (!folded) {
            result.append(copySrc.mid(begin, end - begin));
        } else {
            QByteArray hdrValue = copySrc.mid(begin, end - begin);
            result.append(unfoldHeader(hdrValue));
        }

        // get the next one, a tiny bit ugly, but we don't want the previous to be found again...
        copySrc = copySrc.mid(end);
        if (indexOfHeader(copySrc, name, end, begin, &folded) < 0) {
            break;
        }
    }
    return result;
}

void removeHeader(QByteArray &header, const QByteArray &name)
{
    int begin, end, dummy;
    begin = indexOfHeader(header, name, end, dummy);
    if (begin >= 0) {
        header.remove(begin, end - begin + 1);
    }
}

QByteArray CRLFtoLF(const QByteArray &s)
{
    QByteArray ret = s;
    ret.replace("\r\n", "\n");
    return ret;
}

QByteArray CRLFtoLF(const char *s)
{
    QByteArray ret = s;
    return CRLFtoLF(ret);
}

QByteArray LFtoCRLF(const QByteArray &s)
{
    QByteArray ret = s;
    ret.replace('\n', "\r\n");
    return ret;
}

QByteArray LFtoCRLF(const char *s)
{
    QByteArray ret = s;
    return LFtoCRLF(ret);
}

namespace
{
template < typename StringType, typename CharType > void removeQuotesGeneric(StringType &str)
{
    bool inQuote = false;
    for (int i = 0; i < str.length(); ++i) {
        if (str[i] == CharType('"')) {
            str.remove(i, 1);
            i--;
            inQuote = !inQuote;
        } else {
            if (inQuote && (str[i] == CharType('\\'))) {
                str.remove(i, 1);
            }
        }
    }
}
}

void removeQuotes(QByteArray &str)
{
    removeQuotesGeneric<QByteArray, char>(str);
}

void removeQuotes(QString &str)
{
    removeQuotesGeneric<QString, QLatin1Char>(str);
}

template<class StringType, class CharType, class CharConverterType, class StringConverterType, class ToString>
void addQuotes_impl(StringType &str, bool forceQuotes)
{
    bool needsQuotes = false;
    for (int i = 0; i < str.length(); i++) {
        const CharType cur = str.at(i);
        if (QString(ToString(str)).contains(QRegExp(QLatin1String("\"|\\\\|=|\\]|\\[|:|;|,|\\.|,|@|<|>|\\)|\\(")))) {
            needsQuotes = true;
        }
        if (cur == CharConverterType('\\') || cur == CharConverterType('\"')) {
            str.insert(i, CharConverterType('\\'));
            i++;
        }
    }

    if (needsQuotes || forceQuotes) {
        str.insert(0, CharConverterType('\"'));
        str.append(StringConverterType("\""));
    }
}

void addQuotes(QByteArray &str, bool forceQuotes)
{
    addQuotes_impl<QByteArray, char, char, char *, QLatin1String>(str, forceQuotes);
}

void addQuotes(QString &str, bool forceQuotes)
{
    addQuotes_impl<QString, QChar, QLatin1Char, QLatin1String, QString>(str, forceQuotes);
}

KMIME_EXPORT QString balanceBidiState(const QString &input)
{
    const int LRO = 0x202D;
    const int RLO = 0x202E;
    const int LRE = 0x202A;
    const int RLE = 0x202B;
    const int PDF = 0x202C;

    QString result = input;

    int openDirChangers = 0;
    int numPDFsRemoved = 0;
    for (int i = 0; i < input.length(); i++) {
        const ushort &code = input.at(i).unicode();
        if (code == LRO || code == RLO || code == LRE || code == RLE) {
            openDirChangers++;
        } else if (code == PDF) {
            if (openDirChangers > 0) {
                openDirChangers--;
            } else {
                // One PDF too much, remove it
                qWarning() << "Possible Unicode spoofing (unexpected PDF) detected in" << input;
                result.remove(i - numPDFsRemoved, 1);
                numPDFsRemoved++;
            }
        }
    }

    if (openDirChangers > 0) {
        qWarning() << "Possible Unicode spoofing detected in" << input;

        // At PDF chars to the end until the correct state is restored.
        // As a special exception, when encountering quoted strings, place the PDF before
        // the last quote.
        for (int i = openDirChangers; i > 0; i--) {
            if (result.endsWith(QLatin1Char('"'))) {
                result.insert(result.length() - 1, QChar(PDF));
            } else {
                result += QChar(PDF);
            }
        }
    }

    return result;
}

QString removeBidiControlChars(const QString &input)
{
    const int LRO = 0x202D;
    const int RLO = 0x202E;
    const int LRE = 0x202A;
    const int RLE = 0x202B;
    QString result = input;
    result.remove(LRO);
    result.remove(RLO);
    result.remove(LRE);
    result.remove(RLE);
    return result;
}

static bool isCryptoPart(Content *content)
{
    if (!content->contentType(false)) {
        return false;
    }

    if (content->contentType()->subType().toLower() == "octet-stream" &&
            !content->contentDisposition(false)) {
        return false;
    }

    const Headers::ContentType *contentType = content->contentType();
    const QByteArray lowerSubType = contentType->subType().toLower();
    return (contentType->mediaType().toLower() == "application" &&
            (lowerSubType == "pgp-encrypted" ||
             lowerSubType == "pgp-signature" ||
             lowerSubType == "pkcs7-mime" ||
             lowerSubType == "pkcs7-signature" ||
             lowerSubType == "x-pkcs7-signature" ||
             (lowerSubType == "octet-stream" &&
              content->contentDisposition()->filename().toLower() == QLatin1String("msg.asc"))));
}

bool hasAttachment(Content *content)
{
    if (!content) {
        return false;
    }

    bool emptyFilename = true;
    if (content->contentDisposition(false) &&
            !content->contentDisposition()->filename().isEmpty()) {
        emptyFilename = false;
    }

    if (emptyFilename &&
            content->contentType(false) &&
            !content->contentType()->name().isEmpty()) {
        emptyFilename = false;
    }

    // ignore crypto parts
    if (!emptyFilename && !isCryptoPart(content)) {
        return true;
    }

    // Ok, content itself is not an attachment. now we deal with multiparts
    if (content->contentType()->isMultipart()) {
        Q_FOREACH (Content *child, content->contents()) {
            if (hasAttachment(child)) {
                return true;
            }
        }
    }
    return false;
}

bool isSigned(Message *message)
{
    if (!message) {
        return false;
    }

    const KMime::Headers::ContentType *const contentType = message->contentType();
    if (contentType->isSubtype("signed") ||
            contentType->isSubtype("pgp-signature") ||
            contentType->isSubtype("pkcs7-signature") ||
            contentType->isSubtype("x-pkcs7-signature") ||
            message->mainBodyPart("multipart/signed") ||
            message->mainBodyPart("application/pgp-signature") ||
            message->mainBodyPart("application/pkcs7-signature") ||
            message->mainBodyPart("application/x-pkcs7-signature")) {
        return true;
    }
    return false;
}

bool isEncrypted(Message *message)
{
    if (!message) {
        return false;
    }

    const KMime::Headers::ContentType *const contentType = message->contentType();
    if (contentType->isSubtype("encrypted") ||
            contentType->isSubtype("pgp-encrypted") ||
            contentType->isSubtype("pkcs7-mime") ||
            message->mainBodyPart("multipart/encrypted") ||
            message->mainBodyPart("application/pgp-encrypted") ||
            message->mainBodyPart("application/pkcs7-mime")) {
        return true;
    }

    return false;
}

bool isInvitation(Content *content)
{
    if (!content) {
        return false;
    }

    const KMime::Headers::ContentType *const contentType = content->contentType(false);

    if (contentType && contentType->isMediatype("text") && contentType->isSubtype("calendar")) {
        return true;
    }

    return false;
}

} // namespace KMime
