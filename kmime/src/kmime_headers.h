/*  -*- c++ -*-
    kmime_headers.h

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2001-2002 the KMime authors.
    See file AUTHORS for details
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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
  @file
  This file is part of the API for handling @ref MIME data and
  defines the various header classes:
   - header's base class defining the common interface
   - generic base classes for different types of fields
   - incompatible, Structured-based field classes
   - compatible, Unstructured-based field classes

  @brief
  Defines the various headers classes.

  @authors the KMime authors (see AUTHORS file),
  Volker Krause \<vkrause@kde.org\>
*/

#ifndef __KMIME_HEADERS_H__
#define __KMIME_HEADERS_H__

#include "kmime_export.h"
#include "kmime_header_parsing.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/QDateTime>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QByteArray>

#include <kdatetime.h>

namespace KMime {

class Content;

namespace Headers {

class BasePrivate;

enum contentCategory {
  CCsingle,
  CCcontainer,
  CCmixedPart,
  CCalternativePart
};

/**
  Various possible values for the "Content-Transfer-Encoding" header.
*/
enum contentEncoding {
  CE7Bit,              ///< 7bit
  CE8Bit,              ///< 8bit
  CEquPr,              ///< quoted-printable
  CEbase64,            ///< base64
  CEuuenc,             ///< uuencode
  CEbinary             ///< binary
};

/**
  Various possible values for the "Content-Disposition" header.
*/
enum contentDisposition {
  CDInvalid,           ///< Default, invalid value
  CDinline,            ///< inline
  CDattachment,        ///< attachment
  CDparallel           ///< parallel (invalid, do not use)
};

//often used charset
// TODO: get rid of this!
static const QByteArray Latin1( "ISO-8859-1" );

//@cond PRIVATE
// internal macro to generate default constructors
#define kmime_mk_trivial_ctor( subclass )                               \
  public:                                                               \
    explicit subclass( Content *parent = 0 );                           \
    subclass( Content *parent, const QByteArray &s );                   \
    subclass( Content *parent, const QString &s, const QByteArray &charset ); \
    ~subclass();

#define kmime_mk_dptr_ctor( subclass ) \
  protected: \
    explicit subclass( subclass##Private *d, KMime::Content *parent = 0 );

#define kmime_mk_trivial_ctor_with_name( subclass )     \
  kmime_mk_trivial_ctor( subclass )                     \
    const char *type() const;                           \
    static const char *staticType();
//@endcond

//
//
// HEADER'S BASE CLASS. DEFINES THE COMMON INTERFACE
//
//

/** Baseclass of all header-classes. It represents a
    header-field as described in RFC-822.  */
class KMIME_EXPORT Base
{
  public:
    /**
      A list of headers.
    */
    typedef QList<KMime::Headers::Base*> List;

    /**
      Creates an empty header with a parent-content.
    */
    explicit Base( KMime::Content *parent = 0 );

    /**
      Destructor.
    */
    virtual ~Base();

    /**
      Returns the parent of this header.
    */
    KMime::Content *parent() const;

    /**
      Sets the parent for this header to @p parent.
    */
    void setParent( KMime::Content *parent );

    /**
      Parses the given string. Take care of RFC2047-encoded strings.
      @param s The encoded header data.
    */
    virtual void from7BitString( const QByteArray &s ) = 0;

    /**
      Returns the encoded header.
      @param withHeaderType Specifies whether the header-type should be included.
    */
    virtual QByteArray as7BitString( bool withHeaderType = true ) const = 0;

    /**
      Returns the charset that is used for RFC2047-encoding.
    */
    QByteArray rfc2047Charset() const;

    /**
      Sets the charset for RFC2047-encoding.
      @param cs The new charset used for RFC2047 encoding.
    */
    void setRFC2047Charset( const QByteArray &cs );

    /**
      Returns the default charset.
    */
    QByteArray defaultCharset() const;

    /**
      Returns if the default charset is mandatory.
    */
    bool forceDefaultCharset() const;

    /**
      Parses the given string and set the charset.
      @param s The header data as unicode string.
      @param b The charset preferred for encoding.
    */
    virtual void fromUnicodeString( const QString &s, const QByteArray &b ) = 0;

    /**
      Returns the decoded content of the header without the header-type.

      @note The return value of this method should only be used when showing an address
            to the user. It is not guaranteed that fromUnicodeString( asUnicodeString(), ... )
            will return the original string.
    */
    virtual QString asUnicodeString() const = 0;

    /**
      Deletes.
    */
    virtual void clear() = 0;

    /**
      Checks if this header contains any data.
    */
    virtual bool isEmpty() const = 0;

    /**
      Returns the type of this header (e.g. "From").
    */
    virtual const char *type() const;

    /**
      Checks if this header is of type @p t.
    */
    bool is( const char *t ) const;

    /**
      Checks if this header is a MIME header.
    */
    bool isMimeHeader() const;

    /**
      Checks if this header is a X-Header.
    */
    bool isXHeader() const;

  protected:
    /**
      Helper method, returns the header prefix including ":".
    */
    QByteArray typeIntro() const;

    //@cond PRIVATE
    BasePrivate *d_ptr;
    kmime_mk_dptr_ctor( Base )
    //@endcond

  private:
    Q_DECLARE_PRIVATE( Base )
    Q_DISABLE_COPY( Base )
};

//
//
// GENERIC BASE CLASSES FOR DIFFERENT TYPES OF FIELDS
//
//

namespace Generics {

class UnstructuredPrivate;

/**
  Abstract base class for unstructured header fields
  (e.g. "Subject", "Comment", "Content-description").

  Features: Decodes the header according to RFC2047, incl. RFC2231
  extensions to encoded-words.

  Subclasses need only re-implement @p const @p char* @p type().
*/

// known issues:
// - uses old decodeRFC2047String function, instead of our own...

class KMIME_EXPORT Unstructured : public Base
{
  //@cond PRIVATE
  kmime_mk_dptr_ctor( Unstructured )
  //@endcond
  public:
    explicit Unstructured( Content *p = 0 );
    Unstructured( Content *p, const QByteArray &s );
    Unstructured( Content *p, const QString &s, const QByteArray &cs );
    ~Unstructured();

    virtual void from7BitString( const QByteArray &s );
    virtual QByteArray as7BitString( bool withHeaderType=true ) const;

    virtual void fromUnicodeString( const QString &s,
                                  const QByteArray &b );
    virtual QString asUnicodeString() const;

    virtual void clear();

    virtual bool isEmpty() const;

  private:
    Q_DECLARE_PRIVATE( Unstructured )
};


class StructuredPrivate;

/**
  @brief
  Base class for structured header fields.

  This is the base class for all structured header fields.
  It contains parsing methods for all basic token types found in rfc2822.

  @section Parsing

  At the basic level, there are tokens & tspecials (rfc2045),
  atoms & specials, quoted-strings, domain-literals (all rfc822) and
  encoded-words (rfc2047).

  As a special token, we have the comment. It is one of the basic
  tokens defined in rfc822, but it's parsing relies in part on the
  basic token parsers (e.g. comments may contain encoded-words).
  Also, most upper-level parsers (notably those for phrase and
  dot-atom) choose to ignore any comment when parsing.

  Then there are the real composite tokens, which are made up of one
  or more of the basic tokens (and semantically invisible comments):
  phrases (rfc822 with rfc2047) and dot-atoms (rfc2822).

  This finishes the list of supported token types. Subclasses will
  provide support for more higher-level tokens, where necessary,
  using these parsers.

  @author Marc Mutz <mutz@kde.org>
*/

class KMIME_EXPORT Structured : public Base
{
  public:
    explicit Structured( Content *p = 0 );
    Structured( Content *p, const QByteArray &s );
    Structured( Content *p, const QString &s, const QByteArray &cs );
    ~Structured();

    virtual void from7BitString( const QByteArray &s );
    virtual QString asUnicodeString() const;
    virtual void fromUnicodeString( const QString &s, const QByteArray &b );

  protected:
    /**
      This method parses the raw header and needs to be implemented in
      every sub-class.

      @param scursor Pointer to the start of the data still to parse.
      @param send Pointer to the end of the data.
      @param isCRLF true if input string is terminated with a CRLF.
    */
    virtual bool parse( const char* &scursor, const char *const send,
                        bool isCRLF = false ) = 0;

    //@cond PRIVATE
    kmime_mk_dptr_ctor( Structured )
    //@endcond

  private:
    Q_DECLARE_PRIVATE( Structured )
};

class AddressPrivate;

/**
  Base class for all address related headers.
*/
class KMIME_EXPORT Address : public Structured
{
  public:
    explicit Address( Content *p = 0 );
    Address( Content *p, const QByteArray &s );
    Address( Content *p, const QString &s, const QByteArray &cs );
    ~Address();
  protected:
    //@cond PRIVATE
    kmime_mk_dptr_ctor( Address )
    //@endcond
  private:
    Q_DECLARE_PRIVATE( Address )
};

class MailboxListPrivate;

/**
  Base class for headers that deal with (possibly multiple)
  addresses, but don't allow groups.

  @see RFC 2822, section 3.4
*/
class KMIME_EXPORT MailboxList : public Address
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor( MailboxList )
  kmime_mk_dptr_ctor( MailboxList )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual void fromUnicodeString( const QString &s, const QByteArray &b );
    virtual QString asUnicodeString() const;

    virtual void clear();
    virtual bool isEmpty() const;

    /**
      Adds an address to this header.

      @param mbox A Mailbox object specifying the address.
    */
    void addAddress( const Types::Mailbox &mbox );

    /**
      Adds an address to this header.
      @param address The actual email address, with or without angle brackets.
      @param displayName An optional name associated with the address.
    */
    void addAddress( const QByteArray &address,
                     const QString &displayName = QString() );

    /**
      Returns a list of all addresses in this header, regardless of groups.
    */
    QList<QByteArray> addresses() const;

    /**
      Returns a list of all display names associated with the addresses in
      this header. An empty entry is added for addresses that do not have
      a display name.
    */
    QStringList displayNames() const;

    /**
      Returns a list of assembled display name / address strings of the
      following form: "Display Name &lt;address&gt;". These are unicode
      strings without any transport encoding, ie. they are only suitable
      for displaying.
    */
    QStringList prettyAddresses() const;

    /**
      Returns a list of mailboxes listed in this header.
    */
    Types::Mailbox::List mailboxes() const;

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( MailboxList )
};

class SingleMailboxPrivate;

/**
   Base class for headers that deal with exactly one mailbox
   (e.g. Sender).
*/
class KMIME_EXPORT SingleMailbox : public MailboxList
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor( SingleMailbox )
  //@endcond
  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );
  private:
    Q_DECLARE_PRIVATE( SingleMailbox )
};

class AddressListPrivate;

/**
  Base class for headers that deal with (possibly multiple)
  addresses, allowing groups.

  Note: Groups are parsed but not represented in the API yet. All addresses in
  groups are listed as if they would not be part of a group.

  @todo Add API for groups?

  @see RFC 2822, section 3.4
*/
class KMIME_EXPORT AddressList : public Address
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor( AddressList )
  kmime_mk_dptr_ctor( AddressList )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual void fromUnicodeString( const QString &s, const QByteArray &b );
    virtual QString asUnicodeString() const;

    virtual void clear();
    virtual bool isEmpty() const;

    /**
      Adds an address to this header.

      @param mbox A Mailbox object specifying the address.
    */
    void addAddress( const Types::Mailbox &mbox );

    /**
      Adds an address to this header.
      @param address The actual email address, with or without angle brackets.
      @param displayName An optional name associated with the address.
    */
    void addAddress( const QByteArray &address, const QString &displayName = QString() );

    /**
      Returns a list of all addresses in this header, regardless of groups.
    */
    QList<QByteArray> addresses() const;

    /**
      Returns a list of all display names associated with the addresses in this header.
      An empty entry is added for addresses that don't have a display name.
    */
    QStringList displayNames() const;

    /**
      Returns a list of assembled display name / address strings of the following form:
      "Display Name &lt;address&gt;". These are unicode strings without any transport
      encoding, ie. they are only suitable for displaying.
    */
    QStringList prettyAddresses() const;

    /**
      Returns a list of mailboxes listed in this header.
    */
    Types::Mailbox::List mailboxes() const;

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( AddressList )
};

class IdentPrivate;

/**
  Base class for headers which deal with a list of msg-id's.

  @see RFC 2822, section 3.6.4
*/
class KMIME_EXPORT Ident : public Address
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor( Ident )
  kmime_mk_dptr_ctor( Ident )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual void clear();
    virtual bool isEmpty() const;

    /**
      Returns the list of identifiers contained in this header.
      Note:
      - Identifiers are not enclosed in angle-brackets.
      - Identifiers are listed in the same order as in the header.
    */
    QList<QByteArray> identifiers() const;

    /**
      Appends a new identifier to this header.
      @param id The identifier to append, with or without angle-brackets.
    */
    void appendIdentifier( const QByteArray &id );

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( Ident )
};

class SingleIdentPrivate;

/**
  Base class for headers which deal with a single msg-id.

  @see RFC 2822, section 3.6.4
*/
class KMIME_EXPORT SingleIdent : public Ident
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor( SingleIdent )
  kmime_mk_dptr_ctor( SingleIdent )
  //@endcond
  public:
    /**
      Returns the identifier contained in this header.
      Note: The identifiers is not enclosed in angle-brackets.
    */
    QByteArray identifier() const;

    /**
      Sets the identifier.
      @param id The new identifier with or without angle-brackets.
    */
    void setIdentifier( const QByteArray &id );

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( SingleIdent )
};

class TokenPrivate;

/**
  Base class for headers which deal with a single atom.
*/
class KMIME_EXPORT Token : public Structured
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor( Token )
  kmime_mk_dptr_ctor( Token )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual void clear();
    virtual bool isEmpty() const;

    /**
      Returns the token.
    */
    QByteArray token() const;

    /**
      Sets the token to @p t,
    */
    void setToken( const QByteArray &t );

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( Token )
};

class PhraseListPrivate;

/**
  Base class for headers containing a list of phrases.
*/
class KMIME_EXPORT PhraseList : public Structured
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor( PhraseList )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual QString asUnicodeString() const;
    virtual void clear();
    virtual bool isEmpty() const;

    /**
      Returns the list of phrases contained in this header.
    */
    QStringList phrases() const;

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( PhraseList )
};

class DotAtomPrivate;

/**
  Base class for headers containing a dot atom.
*/
class KMIME_EXPORT DotAtom : public Structured
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor( DotAtom )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual QString asUnicodeString() const;
    virtual void clear();
    virtual bool isEmpty() const;

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( DotAtom )
};

class ParametrizedPrivate;

/**
  Base class for headers containing a parameter list such as "Content-Type".
*/
class KMIME_EXPORT Parametrized : public Structured
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor( Parametrized )
  kmime_mk_dptr_ctor( Parametrized )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;

    virtual bool isEmpty() const;
    virtual void clear();

    //FIXME: Shouldn't the parameter keys be QByteArray and not QStrings? Only the values can be
    //       non-ascii!

    /**
      Returns the value of the specified parameter.
      @param key The parameter name.
    */
    QString parameter( const QString &key ) const;

    /**
      @param key the key of the parameter to check for
      @return true if a parameter with the given @p key exists.
      @since 4.5
    */
    bool hasParameter( const QString &key ) const;

    /**
      Sets the parameter @p key to @p value.
      @param key The parameter name.
      @param value The new value for @p key.
    */
    void setParameter( const QString &key, const QString &value );

  protected:
    virtual bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( Parametrized )
};

} // namespace Generics

//
//
// INCOMPATIBLE, GSTRUCTURED-BASED FIELDS:
//
//

class ReturnPathPrivate;

/**
  Represents the Return-Path header field.

  @see RFC 2822, section 3.6.7
*/
class KMIME_EXPORT ReturnPath : public Generics::Address
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( ReturnPath )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual void clear();
    virtual bool isEmpty() const;

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( ReturnPath )
};

// Address et al.:

// rfc(2)822 headers:
/**
   Represent a "From" header.

   @see RFC 2822, section 3.6.2.
*/
class KMIME_EXPORT From : public Generics::MailboxList
{
  kmime_mk_trivial_ctor_with_name( From )
};

/**
  Represents a "Sender" header.

  @see RFC 2822, section 3.6.2.
*/
class KMIME_EXPORT Sender : public Generics::SingleMailbox
{
  kmime_mk_trivial_ctor_with_name( Sender )
};

/**
  Represents a "To" header.

  @see RFC 2822, section 3.6.3.
*/
class KMIME_EXPORT To : public Generics::AddressList
{
  kmime_mk_trivial_ctor_with_name( To )
};

/**
  Represents a "Cc" header.

  @see RFC 2822, section 3.6.3.
*/
class KMIME_EXPORT Cc : public Generics::AddressList
{
  kmime_mk_trivial_ctor_with_name( Cc )
};

/**
  Represents a "Bcc" header.

  @see RFC 2822, section 3.6.3.
*/
class KMIME_EXPORT Bcc : public Generics::AddressList
{
  kmime_mk_trivial_ctor_with_name( Bcc )
};

/**
  Represents a "ReplyTo" header.

  @see RFC 2822, section 3.6.2.
*/
class KMIME_EXPORT ReplyTo : public Generics::AddressList
{
  kmime_mk_trivial_ctor_with_name( ReplyTo )
};


class MailCopiesToPrivate;

/**
  Represents a "Mail-Copies-To" header.

  @see http://www.newsreaders.com/misc/mail-copies-to.html
*/
class KMIME_EXPORT MailCopiesTo : public Generics::AddressList
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( MailCopiesTo )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual QString asUnicodeString() const;

    virtual void clear();
    virtual bool isEmpty() const;

    /**
      Returns true if a mail copy was explicitly requested.
    */
    bool alwaysCopy() const;

    /**
      Sets the header to "poster".
    */
    void setAlwaysCopy();

    /**
      Returns true if a mail copy was explicitly denied.
    */
    bool neverCopy() const;

    /**
      Sets the header to "never".
    */
    void setNeverCopy();

  protected:
    virtual bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( MailCopiesTo )
};

class ContentTransferEncodingPrivate;

/**
  Represents a "Content-Transfer-Encoding" header.

  @see RFC 2045, section 6.
*/
class KMIME_EXPORT ContentTransferEncoding : public Generics::Token
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( ContentTransferEncoding )
  //@endcond
  public:
    virtual void clear();

    /**
      Returns the encoding specified in this header.
    */
    contentEncoding encoding() const;

    /**
      Sets the encoding to @p e.
    */
    void setEncoding( contentEncoding e );

    /**
      Returns whether the Content containing this header is already decoded.
    */
    // KDE5: rename to isDecoded().
    bool decoded() const;

    /**
      Set whether the Content containing this header is already decoded.
      For instance, if you fill your Content with already-encoded base64 data,
      you will want to setDecoded( false ).
      @param decoded if @c true the content is already decoded
    */
    void setDecoded( bool decoded = true );

    /**
      Returns whether the Content containing this header needs to be encoded
      (i.e., if decoded() is true and encoding() is base64 or quoted-printable).
    */
    bool needToEncode() const;

  protected:
    virtual bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( ContentTransferEncoding )
};

/**
  Represents a "Keywords" header.

  @see RFC 2822, section 3.6.5.
*/
class KMIME_EXPORT Keywords : public Generics::PhraseList
{
  kmime_mk_trivial_ctor_with_name( Keywords )
};

// DotAtom:

/**
  Represents a "MIME-Version" header.

  @see RFC 2045, section 4.
*/
class KMIME_EXPORT MIMEVersion : public Generics::DotAtom
{
  kmime_mk_trivial_ctor_with_name( MIMEVersion )
};

// Ident:

/**
  Represents a "Message-ID" header.

  @see RFC 2822, section 3.6.4.
*/
class KMIME_EXPORT MessageID : public Generics::SingleIdent
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( MessageID )
  //@endcond
  public:
    /**
      Generate a message identifer.
      @param fqdn A fully qualified domain name.
    */
    void generate( const QByteArray &fqdn );
};

class ContentIDPrivate;

/**
  Represents a "Content-ID" header.
*/
class KMIME_EXPORT ContentID : public Generics::SingleIdent
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( ContentID )
  kmime_mk_dptr_ctor( ContentID )
  //@endcond

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );
  private:
    Q_DECLARE_PRIVATE( ContentID )
};

/**
  Represents a "Supersedes" header.
*/
class KMIME_EXPORT Supersedes : public Generics::SingleIdent
{
  kmime_mk_trivial_ctor_with_name( Supersedes )
};

/**
  Represents a "In-Reply-To" header.

  @see RFC 2822, section 3.6.4.
*/
class KMIME_EXPORT InReplyTo : public Generics::Ident
{
  kmime_mk_trivial_ctor_with_name( InReplyTo )
};

/**
  Represents a "References" header.

  @see RFC 2822, section 3.6.4.
*/
class KMIME_EXPORT References : public Generics::Ident
{
  kmime_mk_trivial_ctor_with_name( References )
};


class ContentTypePrivate;

/**
  Represents a "Content-Type" header.

  @see RFC 2045, section 5.
*/
class KMIME_EXPORT ContentType : public Generics::Parametrized
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( ContentType )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual void clear();
    virtual bool isEmpty() const;

    /**
      Returns the mimetype.
    */
    QByteArray mimeType() const;

    /**
      Returns the media type (first part of the mimetype).
    */

    QByteArray mediaType() const;

    /**
      Returns the mime sub-type (second part of the mimetype).
    */
    QByteArray subType() const;

    /**
      Sets the mimetype and clears already existing parameters.
      @param mimeType The new mimetype.
    */
    void setMimeType( const QByteArray &mimeType );

    /**
      Tests if the media type equals @p mediatype.
    */
    bool isMediatype( const char *mediatype ) const;

    /**
      Tests if the mime sub-type equals @p subtype.
    */
    bool isSubtype( const char *subtype ) const;

    /**
      Returns true if the associated MIME entity is a text.
    */
    bool isText() const;

    /**
      Returns true if the associated MIME entity is a plain text.
    */
    bool isPlainText() const;

    /**
      Returns true if the associated MIME entity is a HTML file.
    */
    bool isHTMLText() const;

    /**
      Returns true if the associated MIME entity is an image.
    */
    bool isImage() const;

    /**
      Returns true if the associated MIME entity is a mulitpart container.
    */
    bool isMultipart() const;

    /**
      Returns true if the associated MIME entity contains partial data.
      @see partialNumber(), partialCount()
    */
    bool isPartial() const;

    /**
      Returns the charset for the associated MIME entity.
    */
    QByteArray charset() const;

    /**
      Sets the charset.
    */
    void setCharset( const QByteArray &s );

    /**
      Returns the boundary (for mulitpart containers).
    */
    QByteArray boundary() const;

    /**
      Sets the mulitpart container boundary.
    */
    void setBoundary( const QByteArray &s );

    /**
      Returns the name of the associated MIME entity.
    */
    QString name() const;

    /**
      Sets the name to @p s using charset @p cs.
    */
    void setName( const QString &s, const QByteArray &cs );

    /**
      Returns the identifier of the associated MIME entity.
    */
    QByteArray id() const;

    /**
      Sets the identifier.
    */
    void setId( const QByteArray &s );

    /**
      Returns the position of this part in a multi-part set.
      @see isPartial(), partialCount()
    */
    int partialNumber() const;

    /**
      Returns the total number of parts in a multi-part set.
      @see isPartial(), partialNumber()
    */
    int partialCount() const;

    /**
      Sets parameters of a partial MIME entity.
      @param total The total number of entities in the multi-part set.
      @param number The number of this entity in a multi-part set.
    */
    void setPartialParams( int total, int number );

    // TODO: document
    contentCategory category() const;

    void setCategory( contentCategory c );

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( ContentType )
};

class ContentDispositionPrivate;

/**
  Represents a "Content-Disposition" header.

  @see RFC 2183
*/
class KMIME_EXPORT ContentDisposition : public Generics::Parametrized
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( ContentDisposition )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual bool isEmpty() const;
    virtual void clear();

    /**
      Returns the content disposition.
    */
    contentDisposition disposition() const;

    /**
      Sets the content disposition.
      @param disp The new content disposition.
    */
    void setDisposition( contentDisposition disp );

    /**
      Returns the suggested filename for the associated MIME part.
      This is just a convenience function, it is equivalent to calling
      parameter( "filename" );
    */
    QString filename() const;

    /**
      Sets the suggested filename for the associated MIME part.
      This is just a convenience function, it is equivalent to calling
      setParameter( "filename", filename );
      @param filename The filename.
    */
    void setFilename( const QString &filename );

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false );

  private:
    Q_DECLARE_PRIVATE( ContentDisposition )
};

//
//
// COMPATIBLE GUNSTRUCTURED-BASED FIELDS:
//
//


class GenericPrivate;

/**
  Represents an arbitrary header, that can contain any header-field.
  Adds a type over Unstructured.
  @see Unstructured
*/
class KMIME_EXPORT Generic : public Generics::Unstructured
{
  public:
    Generic();
    Generic( const char *t );
    Generic( const char *t, Content *p );
    Generic( const char *t, Content *p, const QByteArray &s );
    Generic( const char *t, Content *p, const QString &s, const QByteArray &cs );
    ~Generic();

    virtual void clear();

    virtual bool isEmpty() const;

    virtual const char *type() const;

    void setType( const char *type );

  private:
    Q_DECLARE_PRIVATE( Generic )
};

/**
  Represents a "Subject" header.

  @see RFC 2822, section 3.6.5.
*/
class KMIME_EXPORT Subject : public Generics::Unstructured
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( Subject )
  //@endcond
  public:
    bool isReply() const;
};

/**
  Represents a "Organization" header.
*/
class KMIME_EXPORT Organization : public Generics::Unstructured
{
  kmime_mk_trivial_ctor_with_name( Organization )
};

/**
  Represents a "Content-Description" header.
*/
class KMIME_EXPORT ContentDescription : public Generics::Unstructured
{
  kmime_mk_trivial_ctor_with_name( ContentDescription )
};

/**
  Represents a "Content-Location" header.
  @since 4.2
*/
class KMIME_EXPORT ContentLocation : public Generics::Unstructured
{
  kmime_mk_trivial_ctor_with_name( ContentLocation )
};

class ControlPrivate;

/**
  Represents a "Control" header.

  @see RFC 1036, section 3.
*/
class KMIME_EXPORT Control : public Generics::Structured
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( Control )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual void clear();
    virtual bool isEmpty() const;

    /**
      Returns the control message type.
    */
    QByteArray controlType() const;

    /**
      Returns the control message parameter.
    */
    QByteArray parameter() const;

    /**
      Returns true if this is a cancel control message.
      @see RFC 1036, section 3.1.
    */
    bool isCancel() const;

    /**
      Changes this header into a cancel control message for the given message-id.
      @param msgid The message-id of the article that should be canceled.
    */
    void setCancel( const QByteArray &msgid );

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF = false );

  private:
    Q_DECLARE_PRIVATE( Control )
};

class DatePrivate;

/**
  Represents a "Date" header.

  @see RFC 2822, section 3.3.
*/
class KMIME_EXPORT Date : public Generics::Structured
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( Date )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual void clear();
    virtual bool isEmpty() const;

    /**
      Returns the date contained in this header.
    */
    QDateTime dateTime() const;

    /**
      Sets the date.
    */
    void setDateTime( const QDateTime &dt );

    /**
      Returns the age of the message.
    */
    int ageInDays() const;

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF = false );

  private:
    Q_DECLARE_PRIVATE( Date )
};


class NewsgroupsPrivate;

/**
  Represents a "Newsgroups" header.

  @see RFC 1036, section 2.1.3.
*/
class KMIME_EXPORT Newsgroups : public Generics::Structured
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( Newsgroups )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual void fromUnicodeString( const QString &s, const QByteArray &b );
    virtual QString asUnicodeString() const;
    virtual void clear();
    virtual bool isEmpty() const;

    /**
      Returns the list of newsgroups.
    */
    QList<QByteArray> groups() const;

    /**
      Sets the newsgroup list.
    */
    void setGroups( const QList<QByteArray> &groups );

    /**
      Returns true if this message has been cross-posted, i.e. if it has been
      posted to multiple groups.
    */
    bool isCrossposted() const;

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF = false );

  private:
    Q_DECLARE_PRIVATE( Newsgroups )
};

/**
  Represents a "Followup-To" header.

  @see RFC 1036, section 2.2.3.
*/
class KMIME_EXPORT FollowUpTo : public Newsgroups
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( FollowUpTo )
  //@endcond
};


class LinesPrivate;

/**
  Represents a "Lines" header.

  @see RFC 1036, section 2.2.12.
*/
class KMIME_EXPORT Lines : public Generics::Structured
{
  //@cond PRIVATE
  kmime_mk_trivial_ctor_with_name( Lines )
  //@endcond
  public:
    virtual QByteArray as7BitString( bool withHeaderType = true ) const;
    virtual QString asUnicodeString() const;
    virtual void clear();
    virtual bool isEmpty() const;

    /**
      Returns the number of lines, undefined if isEmpty() returns true.
    */
    int numberOfLines() const;

    /**
      Sets the number of lines to @p lines.
    */
    void setNumberOfLines( int lines );

  protected:
    bool parse( const char* &scursor, const char *const send, bool isCRLF = false );

  private:
    Q_DECLARE_PRIVATE( Lines )
};

/**
  Represents a "User-Agent" header.
*/
class KMIME_EXPORT UserAgent : public Generics::Unstructured
{
  kmime_mk_trivial_ctor_with_name( UserAgent )
};

/** Creates a header based on @param type. If @param type is a known header type,
 * the right object type will be created, otherwise a null pointer is returned. */
KMIME_EXPORT Base *createHeader( const QByteArray& type );

}  //namespace Headers

}  //namespace KMime

// undefine code generation macros again
#undef kmime_mk_trivial_ctor
#undef kmime_mk_dptr_ctor
#undef kmime_mk_trivial_ctor_with_name

#endif // __KMIME_HEADERS_H__
