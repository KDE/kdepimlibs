/*  -*- c++ -*-
    kmime_header_parsing.h

    KMime, the KDE Internet mail/usenet news message library.
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

#include <QtCore/QString>
#include <QtCore/QPair>
#include <QtCore/QList>

#include <kdatetime.h>

#include "kmime_export.h"

template <typename K, typename V> class QMap;
class QStringList;

namespace KMime {

namespace Headers {
  class Base;
}

namespace Types {

// for when we can't make up our mind what to use...
// FIXME: Remove this thing, we should _always_ know whether we are handling a
// byte array or a string.
// In most places where this is used, it should simply be replaced by QByteArray
struct KMIME_EXPORT QStringOrQPair {
  QStringOrQPair() : qstring(), qpair( 0, 0 ) {}
  QString qstring;
  QPair<const char*, int> qpair;
};

struct KMIME_EXPORT AddrSpec {
  QString asString() const;
  /*! This is the same as asString(), except it decodes IDNs for display */
  QString asPrettyString() const;
  bool isEmpty() const;
  QString localPart;
  QString domain;
};
typedef QList<AddrSpec> AddrSpecList;

/**
  Represents an (email address, display name) pair according RFC 2822,
  section 3.4.
*/
class KMIME_EXPORT Mailbox
{
  public:
    typedef QList<Mailbox> List;

    /**
      Returns a string representation of the email address, without
      the angle brackets.
    */
    QByteArray address() const;

    AddrSpec addrSpec() const;

    /**
      Returns the display name.
    */
    QString name() const;

    /**
      Sets the email address.
    */
    void setAddress( const AddrSpec &addr );

    /**
      Sets the email address.
    */
    void setAddress( const QByteArray &addr );

    /**
      Sets the name.
    */
    void setName( const QString &name );

    /**
      Sets the name based on a 7bit encoded string.
    */
    void setNameFrom7Bit( const QByteArray &name,
                          const QByteArray &defaultCharset = QByteArray() );

    /**
      Returns true if this mailbox has an address.
    */
    bool hasAddress() const;

    /**
      Returns true if this mailbox has a display name.
    */
    bool hasName() const;

    /**
      Returns a assembled display name / address string of the following form:
      "Display Name &lt;address&gt;". These are unicode strings without any
      transport encoding, ie. they are only suitable for displaying.
    */
    QString prettyAddress() const;

    /**
     * Describes how display names should be quoted
     * @since 4.5
     */
    //AK_REVIEW: remove this enum
    enum Quoting {
      QuoteNever,         ///< Don't quote display names at all. Such an unquoted display name can not
                          ///  be machine-processed anymore in some cases, for example when it contains
                          ///  commas, like in "Lastname, Firstname".
      QuoteWhenNecessary, ///< Only quote display names when they contain characters that need to be
                          ///  quoted, like commas or quote signs.
      QuoteAlways         ///< Always quote the display name
    };

    /**
     * Overloaded method that gives more control over the quoting of the display name
     * @param quoting describes how the display name should be quoted
     * @since 4.5
     */
    // TODO: KDE5: BIC: remove other prettyAddress() overload, and make it None the default
    //                  parameter here
    //AK_REVIEW: replace by 'QString quotedAddress() const'
    QString prettyAddress( Quoting quoting ) const;

    /**
      Parses the given unicode string.
    */
    void fromUnicodeString( const QString &s );

    /**
      Parses the given 7bit encoded string.
    */
    void from7BitString( const QByteArray &s );

    /**
      Returns a 7bit transport encoded representation of this mailbox.

      @param encCharset The charset used for encoding.
    */
    QByteArray as7BitString( const QByteArray &encCharset ) const;

  private:
    QString mDisplayName;
    AddrSpec mAddrSpec;
};

typedef QList<Mailbox> MailboxList;

struct KMIME_EXPORT Address {
  QString displayName;
  MailboxList mailboxList;
};
typedef QList<Address> AddressList;

} // namespace KMime::Types

namespace HeaderParsing {

/**
  Parses the encoded word.

  @param scursor pointer to the first character beyond the initial '=' of
  the input string.
  @param send pointer to end of input buffer.
  @param result the decoded string the encoded work represented.
  @param language The language parameter according to RFC 2231, section 5.
  @param usedCS    the used charset is returned here
  @param defaultCS the charset to use in case the detected
                   one isn't known to us.
  @param forceCS   force the use of the default charset.

  @return true if the input string was successfully decode; false otherwise.
*/
KMIME_EXPORT bool parseEncodedWord( const char* &scursor,
                                    const char * const send,
                                    QString &result, QByteArray &language,
                                    QByteArray &usedCS, const QByteArray &defaultCS = QByteArray(),
                                    bool forceCS = false );

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
KMIME_EXPORT bool parseGenericQuotedString( const char* &scursor,
                                            const char* const send,
                                            QString &result, bool isCRLF,
                                            const char openChar='"',
                                            const char closeChar='"' );

/** @p scursor must be positioned right after the opening '(' */
KMIME_EXPORT bool parseComment( const char* &scursor, const char * const send,
                                QString &result, bool isCRLF=false,
                                bool reallySave=true );

/**
  Parses a phrase.

  You may or may not have already started parsing into the phrase, but
  only if it starts with atext. If you setup this function to parse a
  phrase starting with an encoded-word or quoted-string, @p scursor has
  to point to the char introducing the encoded-word or quoted-string, resp.

  @param scursor pointer to the first character beyond the initial '=' of
  the input string.
  @param send pointer to end of input buffer.
  @param result the parsed string.

  @return true if the input phrase was successfully parsed; false otherwise.
*/
KMIME_EXPORT bool parsePhrase( const char* &scursor, const char * const send,
                               QString &result, bool isCRLF=false );

/**
  Parses into the initial atom.
  You may or may not have already started parsing into the initial
  atom, but not up to it's end.

  @param scursor pointer to the first character beyond the initial '=' of
  the input string.
  @param send pointer to end of input buffer.
  @param result the parsed string.

  @return true if the input phrase was successfully parsed; false otherwise.
*/
KMIME_EXPORT bool parseDotAtom( const char* &scursor, const char * const send,
                                QString &result, bool isCRLF=false );

/**
  Eats comment-folding-white-space, skips whitespace, folding and comments
  (even nested ones) and stops at the next non-CFWS character.  After
  calling this function, you should check whether @p scursor == @p send
  (end of header reached).

  If a comment with unbalanced parantheses is encountered, @p scursor
  is being positioned on the opening '(' of the outmost comment.

  @param scursor pointer to the first character beyond the initial '=' of
  the input string.
  @param send pointer to end of input buffer.
  @param isCRLF true if input string is terminated with a CRLF.
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

KMIME_EXPORT bool parseAddressList( const char* &scursor,
                                    const char * const send,
                                    Types::AddressList &result,
                                    bool isCRLF=false );

KMIME_EXPORT bool parseParameter( const char* &scursor, const char * const send,
                                  QPair<QString,Types::QStringOrQPair> &result,
                                  bool isCRLF=false );

KMIME_EXPORT bool parseParameterList( const char* &scursor,
                                      const char * const send,
                                      QMap<QString,QString> &result,
                                      bool isCRLF=false );

KMIME_EXPORT bool parseRawParameterList( const char* &scursor,
                                         const char * const send,
                                         QMap<QString,Types::QStringOrQPair> &result,
                                         bool isCRLF=false );

/**
 * Extract the charset embedded in the parameter list if there is one.
 *
 * @since 4.5
 */
KMIME_EXPORT bool parseParameterListWithCharset( const char* &scursor,
                                                const char * const send,
                                                QMap<QString,QString> &result,
                                                QByteArray& charset, bool isCRLF=false );

/**
  Parses an integer number.
  @param scursor pointer to the first character of the input string
  @param send pointer to end of input buffer
  @param result the parsing result
  @returns The number of parsed digits (don't confuse with @p result!)
*/
KMIME_EXPORT int parseDigits( const char* &scursor, const char* const send, int &result );

KMIME_EXPORT bool parseTime( const char* &scursor, const char * const send,
                             int &hour, int &min, int &sec,
                             long int &secsEastOfGMT,
                             bool &timeZoneKnown, bool isCRLF=false );

KMIME_EXPORT bool parseDateTime( const char* &scursor, const char * const send,
                                 QDateTime &result, bool isCRLF=false );

/**
 * Extracts and returns the first header that is contained in the given byte array.
 * The header will also be removed from the passed-in byte array head.
 *
 * @since 4.4
 */
KMIME_EXPORT KMime::Headers::Base *extractFirstHeader( QByteArray &head );

/**
 * Extract the header header and the body from a complete content.
 * Internally, it will simply look for the first newline and use that as a
 * separator between the header and the body.
 *
 * @param content the complete mail
 * @param header return value for the extracted header
 * @param body return value for the extracted body
 * @since 4.6
 */
KMIME_EXPORT void extractHeaderAndBody( const QByteArray &content,
                                        QByteArray &header, QByteArray &body );


} // namespace HeaderParsing

} // namespace KMime

#endif // __KMIME_HEADER_PARSING_H__

