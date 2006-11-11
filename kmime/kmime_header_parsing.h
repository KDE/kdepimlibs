/*  -*- c++ -*-
    kmime_header_parsing.h

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001-2002 Marc Mutz <mutz@kde.org>

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

#ifndef __KMIME_HEADER_PARSING_H__
#define __KMIME_HEADER_PARSING_H__

#include <time.h>

#include <QString>
#include <QPair>

#include "kmime.h"

template <typename K, typename V> class QMap;
class QStringList;

namespace KMime {

namespace Types {

// for when we can't make up our mind what to use...
struct KMIME_EXPORT QStringOrQPair {
  QStringOrQPair() : qstring(), qpair( 0, 0 ) {}
  QString qstring;
  QPair<const char*,int> qpair;
};

struct KMIME_EXPORT AddrSpec {
  QString asString() const;
  QString localPart;
  QString domain;
};
typedef QList<AddrSpec> AddrSpecList;

struct KMIME_EXPORT Mailbox {
  QString displayName;
  AddrSpec addrSpec;
};
typedef QList<Mailbox> MailboxList;

struct KMIME_EXPORT Address {
  QString displayName;
  MailboxList mailboxList;
};
typedef QList<Address> AddressList;

struct KMIME_EXPORT DateTime {
  time_t time;            // secs since 1.1.1970, 0:00 UTC/GMT
  long int secsEastOfGMT; // timezone
  bool timeZoneKnown;     // do we know the timezone? (e.g. on -0000)
};

} // namespace KMime::Types

namespace HeaderParsing {

/**
 * Parse the encoded word.
 *
 * @param scursor pointer to the first character beyond the initial '=' of
 *                the input string.
 * @param send pointer to end of input buffer.
 * @param result the decoded string the encoded work represented.
 *
 * @return true if the input string was successfully decode; false otherwise.
 */
KMIME_EXPORT bool parseEncodedWord(
  const char* &scursor, const char * const send,
  QString &result, QByteArray &language );

//
// The parsing squad:
//

/** You may or may not have already started parsing into the
    atom. This function will go on where you left off. */
KMIME_EXPORT bool parseAtom( const char* &scursor, const char * const send,
                QString &result, bool allow8Bit=false );

KMIME_EXPORT bool parseAtom( const char* &scursor, const char * const send,
                QPair<const char*,int> &result,
                bool allow8Bit=false );

/** You may or may not have already started parsing into the
    token. This function will go on where you left off. */
KMIME_EXPORT bool parseToken( const char* &scursor, const char * const send,
                 QString &result, bool allow8Bit=false );

KMIME_EXPORT bool parseToken( const char* &scursor, const char * const send,
                 QPair<const char*,int> &result,
                 bool allow8Bit=false );

/** @p scursor must be positioned after the opening openChar. */
KMIME_EXPORT bool parseGenericQuotedString( const char* &scursor, const char* const send,
                               QString &result, bool isCRLF,
                               const char openChar='"',
                               const char closeChar='"' );

/** @p scursor must be positioned right after the opening '(' */
KMIME_EXPORT bool parseComment( const char* &scursor, const char * const send,
                   QString &result, bool isCRLF=false,
                   bool reallySave=true );

/**
 * Parse a phrase.
 *
 * You may or may not have already started parsing into the phrase, but
 * only if it starts with atext. If you setup this function to parse a
 * phrase starting with an encoded-word or quoted-string, @p scursor has
 * to point to the char introducing the encoded-word or quoted-string, resp.
 *
 * @param scursor pointer to the first character beyond the initial '=' of
 *                the input string.
 * @param send pointer to end of input buffer.
 * @param result the parsed string.
 *
 * @return true if the input phrase was successfully parsed; false otherwise.
 */
KMIME_EXPORT bool parsePhrase( const char* &scursor, const char * const send,
                  QString &result, bool isCRLF=false );

/**
 * Parse into the initial atom.
 * You may or may not have already started parsing into the initial
 * atom, but not up to it's end.
 *
 * @param scursor pointer to the first character beyond the initial '=' of
 *                the input string.
 * @param send pointer to end of input buffer.
 * @param result the parsed string.
 *
 * @return true if the input phrase was successfully parsed; false otherwise.
 */
KMIME_EXPORT bool parseDotAtom( const char* &scursor, const char * const send,
                   QString &result, bool isCRLF=false );

/**
 * Eats comment-folding-white-space, skips whitespace, folding and comments
 * (even nested ones) and stops at the next non-CFWS character.  After
 * calling this function, you should check whether @p scursor == @p send
 * (end of header reached).
 *
 *  If a comment with unbalanced parantheses is encountered, @p scursor
 *  is being positioned on the opening '(' of the outmost comment.
 *
 * @param scursor pointer to the first character beyond the initial '=' of
 *                the input string.
 * @param send pointer to end of input buffer.
 * @param isCRLF true if input string is terminated with a CRLF.
 */
KMIME_EXPORT void eatCFWS( const char* &scursor, const char * const send,
              bool isCRLF );

KMIME_EXPORT bool parseDomain( const char* &scursor, const char * const send,
                  QString &result, bool isCRLF=false );

KMIME_EXPORT bool parseObsRoute( const char* &scursor, const char * const send,
                    QStringList &result, bool isCRLF=false,
                    bool save=false );

KMIME_EXPORT bool parseAddrSpec( const char* &scursor, const char * const send,
                    Types::AddrSpec &result, bool isCRLF=false );

KMIME_EXPORT bool parseAngleAddr( const char* &scursor, const char * const send,
                     Types::AddrSpec &result, bool isCRLF=false );

/**
  Parses a single mailbox.

  RFC 2822, section 3.4 defines a mailbox as follows:
  <pre>mailbox := addr-spec / ([ display-name ] angle-addr)</pre>

  KMime also accepts the legacy format of specifying display names:
  <pre>mailbox := (addr-spec [ "(" display-name ")" ])
                / ([ display-name ] angle-addr)
                / (angle-addr "(" display-name ")")</pre>

  @param scursor pointer to the first character of the input string
  @param send pointer to end of input buffer
  @param result the parsing result
  @param isCRLF true if input string is terminated with a CRLF.
*/
KMIME_EXPORT bool parseMailbox( const char* &scursor, const char * const send,
                   Types::Mailbox &result, bool isCRLF=false );

KMIME_EXPORT bool parseGroup( const char* &scursor, const char * const send,
                 Types::Address &result, bool isCRLF=false );

KMIME_EXPORT bool parseAddress( const char* &scursor, const char * const send,
                   Types::Address &result, bool isCRLF=false );

KMIME_EXPORT bool parseAddressList( const char* &scursor, const char * const send,
                       Types::AddressList &result,
                       bool isCRLF=false );

KMIME_EXPORT bool parseParameter( const char* &scursor, const char * const send,
                     QPair<QString,Types::QStringOrQPair> &result,
                     bool isCRLF=false );

KMIME_EXPORT bool parseParameterList( const char* &scursor, const char * const send,
                         QMap<QString,QString> &result,
                         bool isCRLF=false );

KMIME_EXPORT bool parseRawParameterList( const char* &scursor, const char * const send,
                            QMap<QString,Types::QStringOrQPair> &result,
                            bool isCRLF=false );

KMIME_EXPORT bool parseTime( const char* &scursor, const char * const send, int &hour,
                int &min, int &sec, long int &secsEastOfGMT,
                bool &timeZoneKnown, bool isCRLF=false );

KMIME_EXPORT bool parseDateTime( const char* &scursor, const char * const send,
                    Types::DateTime &result, bool isCRLF=false );

} // namespace HeaderParsing

} // namespace KMime

#endif // __KMIME_HEADER_PARSING_H__

