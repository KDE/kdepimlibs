/*  -*- c++ -*-
    kmime_util.h

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
#ifndef __KMIME_UTIL_H__
#define __KMIME_UTIL_H__

#include <QtCore/QString>
#include "kmime_export.h"
#include "kmime_headers.h"
#include "kmime_content.h"

namespace KMime {

class Message;

/**
  Consult the charset cache. Only used for reducing mem usage by
  keeping strings in a common repository.
  @param name
*/
KMIME_EXPORT extern QByteArray cachedCharset( const QByteArray &name );

/**
  Consult the language cache. Only used for reducing mem usage by
  keeping strings in a common repository.
  @param name
*/
KMIME_EXPORT extern QByteArray cachedLanguage( const QByteArray &name );

/**
  Checks whether @p s contains any non-us-ascii characters.
  @param s
*/
KMIME_EXPORT extern bool isUsAscii( const QString &s );

/**
  Returns a user-visible string for a contentEncoding, for example
  "quoted-printable" for CEquPr.
  @param enc the contentEncoding to return string for
  @ since 4.4
  TODO should they be i18n'ed?
*/
KMIME_EXPORT extern QString nameForEncoding( KMime::Headers::contentEncoding enc );

/**
  Returns a list of encodings that can correctly encode the @p data.
  @param data the data to check encodings for
  @ since 4.4
*/
KMIME_EXPORT QList<KMime::Headers::contentEncoding> encodingsForData(
                                                       const QByteArray &data );
//@cond PRIVATE
extern const uchar specialsMap[16];
extern const uchar tSpecialsMap[16];
extern const uchar aTextMap[16];
extern const uchar tTextMap[16];
extern const uchar eTextMap[16];

inline bool isOfSet( const uchar map[16], unsigned char ch )
{
  return ( ch < 128 ) && ( map[ ch/8 ] & 0x80 >> ch%8 );
}
inline bool isSpecial( char ch )
{
  return isOfSet( specialsMap, ch );
}
inline bool isTSpecial( char ch )
{
  return isOfSet( tSpecialsMap, ch );
}
inline bool isAText( char ch )
{
  return isOfSet( aTextMap, ch );
}
inline bool isTText( char ch )
{
  return isOfSet( tTextMap, ch );
}
inline bool isEText( char ch )
{
  return isOfSet( eTextMap, ch );
}
//@endcond

/**
  * Set the fallback charset to use when decoding RFC2047-encoded headers.
  *  If decoding according to the RFC fails, then the fallback encoding is
  *  used instead.
  *
  * @param fallbackCharEnc Name of fallback character encoding to use.
  *
  * @since 4.5
  */
KMIME_EXPORT extern void setFallbackCharEncoding( const QString& fallbackCharEnc );

/**
  * Retrieve the set fallback charset if there is one set.
  *
  * @return The name of the fallback encoding, if one was set, otherwise
  *           an empty QString.
  *
  * @since 4.5
  */
KMIME_EXPORT extern QString fallbackCharEncoding();

/**
  * Set whether or not to use outlook compatible attachment filename encoding. Outlook
  *  fails to properly adhere to the RFC2322 standard for parametrized header fields, and
  *  instead is only able to read and write attachment filenames encoded in RFC2047-style.
  *  This will create mails that are not standards-compliant!
  *
  * @param violateStandard      Whether or not to use outlook-compatible attachment
  *                              filename encodings.
  *
  * @since 4.5
  */
KMIME_EXPORT extern void setUseOutlookAttachmentEncoding( bool violateStandard );

/**
 * Retrieve whether or not to use outlook compatible encodings for attachments.
 */
KMIME_EXPORT extern bool useOutlookAttachmentEncoding();
/**
  Decodes string @p src according to RFC2047,i.e., the construct
   =?charset?[qb]?encoded?=

  @param src       source string.
  @param usedCS    the detected charset is returned here
  @param defaultCS the charset to use in case the detected
                   one isn't known to us.
  @param forceCS   force the use of the default charset.

  @return the decoded string.
*/
KMIME_EXPORT extern QString decodeRFC2047String(
  const QByteArray &src, QByteArray &usedCS, const QByteArray &defaultCS = QByteArray(),
  bool forceCS = false );

/** Decode string @p src according to RFC2047 (ie. the
    =?charset?[qb]?encoded?= construct).

    @param src       source string.
    @return the decoded string.
*/
KMIME_EXPORT extern QString decodeRFC2047String( const QByteArray &src );

/**
  Encodes string @p src according to RFC2047 using charset @p charset.

  This function also makes commas, quotes and other characters part of the encoded name, for example
  the string "Jöhn Döe" <john@example.com"> would be encoded as <encoded word for "Jöhn Döe"> <john@example.com>,
  i.e. the opening and closing quote mark would be part of the encoded word.
  Therefore don't use this function for input strings that contain semantically meaningful characters,
  like the quoting marks in this example.

  @param src           source string.
  @param charset       charset to use. If it can't encode the string, UTF-8 will be used instead.
  @param addressHeader if this flag is true, all special chars
                       like <,>,[,],... will be encoded, too.
  @param allow8bitHeaders if this flag is true, 8Bit headers are allowed.

  @return the encoded string.
*/
KMIME_EXPORT extern QByteArray encodeRFC2047String(
  const QString &src, const QByteArray &charset, bool addressHeader=false,
  bool allow8bitHeaders=false );


/**
  Decodes string @p src according to RFC2231

  @param src       source string.
  @param usedCs    the detected charset is returned here
  @param defaultCS the charset to use in case the detected
                   one isn't known to us.
  @param forceCS   force the use of the default charset.

  @return the decoded string.
*/
KMIME_EXPORT extern QString decodeRFC2231String(
  const QByteArray &src, QByteArray &usedCS, const QByteArray &defaultCS = QByteArray(),
  bool forceCS = false );

/** Decode string @p src according to RFC2231 (ie. the
    charset'lang'encoded construct).

    @param src       source string.
    @return the decoded string.
*/
KMIME_EXPORT extern QString decodeRFC2231String( const QByteArray &src );


/**
  Encodes string @p src according to RFC2231 using charset @p charset.

  @param src           source string.
  @param charset       charset to use.
  @return the encoded string.
*/
KMIME_EXPORT extern QByteArray encodeRFC2231String( const QString &src, const QByteArray &charset );

/**
  Uses current time, pid and random numbers to construct a string
  that aims to be unique on a per-host basis (ie. for the local
  part of a message-id or for multipart boundaries.

  @return the unique string.
  @see multiPartBoundary
*/
KMIME_EXPORT extern QByteArray uniqueString();

/**
  Constructs a random string (sans leading/trailing "--") that can
  be used as a multipart delimiter (ie. as @p boundary parameter
  to a multipart/... content-type).

  @return the randomized string.
  @see uniqueString
*/
KMIME_EXPORT extern QByteArray multiPartBoundary();

/**
  Unfolds the given header if necessary.
  @param header The header to unfold.
*/
KMIME_EXPORT extern QByteArray unfoldHeader( const QByteArray &header );

/**
  Tries to extract the header with name @p name from the string
  @p src, unfolding it if necessary.

  @param src  the source string.
  @param name the name of the header to search for.

  @return the first instance of the header @p name in @p src
          or a null QCString if no such header was found.
*/
KMIME_EXPORT extern QByteArray extractHeader( const QByteArray &src,
                                 const QByteArray &name );

/**
  Tries to extract the headers with name @p name from the string
  @p src, unfolding it if necessary.

  @param src  the source string.
  @param name the name of the header to search for.

  @return all instances of the header @p name in @p src

  @since 4.2
*/
KMIME_EXPORT extern QList<QByteArray> extractHeaders( const QByteArray &src,
                                 const QByteArray &name );

/**
  Converts all occurrences of "\r\n" (CRLF) in @p s to "\n" (LF).

  This function is expensive and should be used only if the mail
  will be stored locally. All decode functions can cope with both
  line endings.

  @param s source string containing CRLF's

  @return the string with CRLF's substitued for LF's
  @see CRLFtoLF(const char*) LFtoCRLF
*/
KMIME_EXPORT extern QByteArray CRLFtoLF( const QByteArray &s );

/**
  Converts all occurrences of "\r\n" (CRLF) in @p s to "\n" (LF).

  This function is expensive and should be used only if the mail
  will be stored locally. All decode functions can cope with both
  line endings.

  @param s source string containing CRLF's

  @return the string with CRLF's substitued for LF's
  @see CRLFtoLF(const QCString&) LFtoCRLF
*/
KMIME_EXPORT extern QByteArray CRLFtoLF( const char *s );

/**
  Converts all occurrences of "\n" (LF) in @p s to "\r\n" (CRLF).

  This function is expensive and should be used only if the mail
  will be transmitted as an RFC822 message later. All decode
  functions can cope with and all encode functions can optionally
  produce both line endings, which is much faster.

  @param s source string containing CRLF's

  @return the string with CRLF's substitued for LF's
  @see CRLFtoLF(const QCString&) LFtoCRLF
*/
KMIME_EXPORT extern QByteArray LFtoCRLF( const QByteArray &s );

/**
  Removes quote (DQUOTE) characters and decodes "quoted-pairs"
  (ie. backslash-escaped characters)

  @param str the string to work on.
  @see addQuotes
*/
//AK_REVIEW: add correctly spelled methods and deprecated the wrongly spelled
// TODO: KDE5: BIC: rename to "removeQuotes"
KMIME_EXPORT extern void removeQuots( QByteArray &str );

/**
  Removes quote (DQUOTE) characters and decodes "quoted-pairs"
  (ie. backslash-escaped characters)

  @param str the string to work on.
  @see addQuotes
*/
//AK_REVIEW: add correctly spelled methods and deprecated the wrongly spelled
// TODO: KDE5: BIC: rename to "removeQuotes"
KMIME_EXPORT extern void removeQuots( QString &str );

/**
  Converts the given string into a quoted-string if the string contains
  any special characters (ie. one of ()<>@,.;:[]=\").

  @param str us-ascii string to work on.
  @param forceQuotes if @c true, always add quote characters.
*/
KMIME_EXPORT extern void addQuotes( QByteArray &str, bool forceQuotes );

/**
 * Overloaded method, behaves same as the above.
 * @param str us-ascii string to work on.
 * @param forceQuotes if @c true, always add quote characters.
 * @since 4.5
 */
KMIME_EXPORT extern void addQuotes( QString &str, bool forceQuotes );

/**
 * Makes sure that the bidirectional state at the end of the string is the
 * same as at the beginning of the string.
 *
 * This is useful so that Unicode control characters that can change the text
 * direction can not spill over to following strings.
 *
 * As an example, consider a mailbox in the form "display name" <local@domain.com>.
 * If the display name here contains unbalanced control characters that change the
 * text direction, it would also have an effect on the addrspec, which could lead to
 * spoofing.
 *
 * By passing the display name to this function, one can make sure that no change of
 * the bidi state can spill over to the next strings, in this case the addrspec.
 *
 * Example: The string "Hello <RLO>World" is unbalanced, as it contains a right-to-left
 *          override character, which is never followed by a <PDF>, the "pop directional
 *          formatting" character. This function adds the missing <PDF> at the end, and
 *          the output of this function would be "Hello <RLO>World<PDF>".
 *
 * Example of spoofing:
 *   Consider "Firstname Lastname<RLO>" <moc.mitciv@attacker.com>. Because of the RLO,
 *   it is displayed as "Firstname Lastname <moc.rekcatta@victim.com>", which spoofs the
 *   domain name.
 *   By passing "Firstname Lastname<RLO>" to this function, one can balance the <RLO>,
 *   leading to "Firstname Lastname<RLO><PDF>", so the whole mailbox is displayed
 *   correctly as "Firstname Lastname" <moc.mitciv@attacker.com> again.
 *
 * See http://unicode.org/reports/tr9 for more information on bidi control chars.
 *
 * @param input the display name of a mailbox, which is checked for unbalanced Unicode
 *              direction control characters
 * @return the display name which now contains a balanced state of direction control
 *         characters
 *
 * Note that this function does not do any parsing related to mailboxes, it only works
 * on plain strings. Therefore, passing the complete mailbox will not lead to any results,
 * only the display name should be passed.
 *
 * @since 4.5
 */
KMIME_EXPORT QString balanceBidiState( const QString &input );

/**
 * Similar to the above function. Instead of trying to balance the Bidi chars, it outright
 * removes them from the string.
 *
 * @param input the display name of a mailbox, which is checked for unbalanced Unicode
 * direction control characters
 * Reason: KHTML seems to ignore the PDF character, so adding them doesn't fix things :(
 */
KMIME_EXPORT QString removeBidiControlChars( const QString &input );

/**
 * Returns whether or not the given MIME node contains an attachment part. This function will
 *  recursively parse the MIME tree looking for a suitable attachment and return true if one is found.
 * @param content the MIME node to parse
 */
KMIME_EXPORT bool hasAttachment( Content* content );

/**
 * Returns whether or not the given MIME node contains an invitation part. This function will
 *  recursively parse the MIME tree looking for a suitable invitation and return true if one is found.
 * @param content the MIME node to parse
 * @since 4.14.6
 */
KMIME_EXPORT bool hasInvitation( Content* content );

/**
 * Returns whether or not the given @p message is partly or fully signed.
 *
 * @param message the message to check for being signed
 * @since 4.6
 */
KMIME_EXPORT bool isSigned( Message* message );

/**
 * Returns whether or not the given @p message is partly or fully encrypted.
 *
 * @param message the message to check for being encrypted
 * @since 4.6
 */
KMIME_EXPORT bool isEncrypted( Message* message );

/**
 * Returns whether or not the given MIME @p content is an invitation
 * message of the iTIP protocol.
 *
 * @since 4.6
 */
KMIME_EXPORT bool isInvitation( Content* content );

} // namespace KMime

#endif /* __KMIME_UTIL_H__ */
