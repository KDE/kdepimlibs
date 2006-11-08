/*  -*- c++ -*-
    kmime_headers.h

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001-2002 the KMime authors.
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
#ifndef __KMIME_HEADERS_H__
#define __KMIME_HEADERS_H__

// Content:
//
// - header's base class defining the common interface
// - generic base classes for different types of fields
// - incompatible, Structured-based field classes
// - compatible, Unstructured-based field classes

#include "kmime.h"
#include "kmime_header_parsing.h"

#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QDateTime>
#include <QMap>
#include <QList>
#include <QByteArray>

#include <time.h>

namespace KMime {

//forward declaration
class Content;

namespace Headers {

enum contentCategory {
  CCsingle,
  CCcontainer,
  CCmixedPart,
  CCalternativePart
};

enum contentEncoding {
  CE7Bit,
  CE8Bit,
  CEquPr,
  CEbase64,
  CEuuenc,
  CEbinary
};

enum contentDisposition {
  CDinline,
  CDattachment,
  CDparallel
};

//often used charset
static const QByteArray Latin1( "ISO-8859-1" );

#define mk_trivial_subclass_with_name( subclass, subclassName, baseclass ) \
  class KMIME_EXPORT subclass : public Generics::baseclass {                         \
    public:                                                             \
    subclass() : Generics::baseclass() {}                               \
    subclass( Content *p ) : Generics::baseclass( p ) {}                \
    subclass( Content *p, const QByteArray &s )                         \
      : Generics::baseclass( p ) { from7BitString( s ); }               \
    subclass( Content *p, const QString &s, const QByteArray &cs )      \
      : Generics::baseclass( p ) { fromUnicodeString( s, cs ); }        \
    ~subclass() {}                                                      \
                                                                        \
    const char *type() const { return #subclassName; }                  \
  }

#define mk_trivial_subclass( subclass, baseclass )                      \
  mk_trivial_subclass_with_name( subclass, subclass, baseclass )

#define mk_parsing_subclass_with_name( subclass, subclassName, baseclass ) \
  class KMIME_EXPORT subclass : public Generics::baseclass {                         \
    public:                                                             \
    subclass() : Generics::baseclass() {}                               \
    subclass( Content *p ) : Generics::baseclass( p ) {}                \
    subclass( Content *p, const QByteArray &s )                         \
      : Generics::baseclass( p ) { from7BitString( s ); }               \
    subclass( Content *p, const QString &s, const QByteArray &cs )      \
      : Generics::baseclass( p ) { fromUnicodeString( s, cs ); }        \
    ~subclass() {}                                                      \
                                                                        \
    const char *type() const { return #subclassName; }                  \
    protected:                                                          \
    bool parse( const char* &scursor, const char *const send, bool isCRLF=false ); \
  }

#define mk_parsing_subclass( subclass, baseclass )                      \
  mk_parsing_subclass_with_name( subclass, subclass, baseclass )

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
    typedef QList<KMime::Headers::Base*> List;

    /** Create an empty header. */
    Base() : e_ncCS( "" ), p_arent(0) {}

    /** Create an empty header with a parent-content. */
    Base( KMime::Content *parent ) : e_ncCS( "" ), p_arent( parent ) {}

    /** Destructor */
    virtual ~Base() {}

    /** Return the parent of this header. */
    KMime::Content *parent() { return p_arent; }

    /** Set the parent for this header. */
    void setParent( KMime::Content *p ) { p_arent = p; }

    /**
      Parses the given string. Take care of RFC2047-encoded strings.
      A default charset is given. If the last parameter is true the
      default charset is used in any case
    */
    virtual void from7BitString( const QByteArray &s ) { Q_UNUSED( s ); }

    /**
      Returns the encoded header. The parameter specifies whether the
      header-type should be included.
    */
    virtual QByteArray as7BitString( bool=true ) { return QByteArray(); }

    /**
      Returns the charset that is used for RFC2047-encoding.
    */
    QByteArray rfc2047Charset();

    /**
      Sets the charset for RFC2047-encoding.
    */
    void setRFC2047Charset( const QByteArray &cs );

    /**
      Returns the default charset.
    */
    QByteArray defaultCS();

    /**
      Returns if the default charset is mandatory.
    */
    bool forceCS();

    /**
      Parses the given string and set the charset.
    */
    virtual void fromUnicodeString( const QString &s, const QByteArray &b )
      { Q_UNUSED( s ); Q_UNUSED( b ); }

    /**
      Returns the decoded content of the header without the header-type.
    */
    virtual QString asUnicodeString() { return QString(); }

    /**
      Deletes.
    */
    virtual void clear() {}

    /** Do we have data? */
    virtual bool isEmpty() const { return false; }

    /**
      Returns the type of this header (e.g. "From").
    */
    virtual const char *type() const { return ""; }

    /**
      Checks if this header is of type @p t.
    */
    bool is( const char *t ) { return (strcasecmp( t, type() ) == 0 ); }

    /**
      Checks if this header is a MIME header.
    */
    bool isMimeHeader() { return (strncasecmp( type(), "Content-", 8 ) == 0); }

    /**
      Checks if this header is a X-Header.
    */
    bool isXHeader() { return (strncmp( type(), "X-", 2 ) == 0 ); }

  protected:
    QByteArray typeIntro() { return (QByteArray( type() ) + ": " ); }

    QByteArray e_ncCS;
    Content *p_arent;
};

//
//
// GENERIC BASE CLASSES FOR DIFFERENT TYPES OF FIELDS
//
//

namespace Generics {

/** Abstract base class for unstructured header fields
    (e.g. "Subject", "Comment", "Content-description").

    Features: Decodes the header according to RFC2047, incl. RFC2231
    extensions to encoded-words.

    Subclasses need only re-implement @p const @p char* @p type().

    A macro to automate this is named
    \code
    MK_TRIVIAL_Unstructured_SUBCLASS(classname,headername);
    \endcode

    The ContentDescription class then reads:
    \code
    MK_TRIVIAL_Unstructured_SUBCLASS(ContentDescription,Content-Description);
    \endcode
*/

// known issues:
// - uses old decodeRFC2047String function, instead of our own...

class KMIME_EXPORT Unstructured : public Base
{
  public:
    Unstructured() : Base() {}
    Unstructured( Content *p ) : Base( p ) {}
    Unstructured( Content *p, const QByteArray &s ) : Base( p )
      { from7BitString( s ); }
    Unstructured( Content *p, const QString &s, const QByteArray &cs ) : Base( p )
      { fromUnicodeString( s, cs ); }
    ~Unstructured() {}

    virtual void from7BitString( const QByteArray &str );
    virtual QByteArray as7BitString( bool withHeaderType=true );

    virtual void fromUnicodeString( const QString &str,
                                    const QByteArray &suggestedCharset );
    virtual QString asUnicodeString();

    virtual void clear() { d_ecoded.truncate( 0 ); }
    virtual bool isEmpty() const { return ( d_ecoded.isEmpty() ); }

  private:
    QString d_ecoded;
};

/** This is the base class for all structured header fields. It
    contains parsing methods for all basic token types found in
    rfc2822.

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

    @short Base class for structured header fields.
    @author Marc Mutz <mutz@kde.org>
*/

class KMIME_EXPORT Structured : public Base
{
  public:
    Structured() : Base() {}
    Structured( Content *p ) : Base( p ) {}
    Structured( Content *p, const QByteArray &s ) : Base( p )
      { from7BitString( s ); }
    Structured( Content *p, const QString &s, const QByteArray &cs ) : Base( p )
      { fromUnicodeString( s, cs ); }
    ~Structured() {}

    virtual void from7BitString( const QByteArray &str );
    virtual void fromUnicodeString( const QString &s, const QByteArray &b );

  protected:
    virtual bool parse( const char* &scursor, const char* const send, bool isCRLF = false ) = 0;
};

class KMIME_EXPORT Address : public Structured
{
  public:
    Address() : Structured() {}
    Address( Content *p ) : Structured( p ) {}
    Address( Content *p, const QByteArray &s )
      : Structured( p ) { from7BitString( s ); }
    Address( Content *p, const QString &s, const QByteArray &cs )
      : Structured( p ) { fromUnicodeString( s, cs ); }
    ~Address() {}

  protected:
};

/** Base class for headers that deal with (possibly multiple)
    addresses, but don't allow groups: */
class KMIME_EXPORT MailboxList : public Address
{
  public:
    MailboxList() : Address() {}
    MailboxList( Content *p ) : Address( p ) {}
    MailboxList( Content *p, const QByteArray &s ) : Address( p )
      { from7BitString( s ); }
    MailboxList( Content *p, const QString &s, const QByteArray &cs ) : Address( p )
      { fromUnicodeString( s, cs ); }
    ~MailboxList() {}

  protected:
    bool parse( const char* &scursor, const char * const send, bool isCRLF=false );

    /** The list of mailboxes */
    QList<Types::Mailbox> mMailboxList;
};

/** Base class for headers that deal with exactly one mailbox
    (e.g. Sender) */
mk_parsing_subclass( SingleMailbox, MailboxList );

/** Base class for headers that deal with (possibly multiple)
    addresses, allowing groups. */
class KMIME_EXPORT AddressList : public Address
{
  public:
    AddressList() : Address() {}
    AddressList( Content * p ) : Address( p ) {}
    AddressList( Content * p, const QByteArray & s )
      : Address( p ) { from7BitString( s ); }
    AddressList( Content * p, const QString & s, const QByteArray & cs )
      : Address( p ) { fromUnicodeString( s, cs ); }
    ~AddressList() {}

  protected:
    bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

    /** The list of addresses */
    QList<Types::Address> mAddressList;
};

/**
  Base class for headers which deal with a list of msg-id's

  @see RFC 2822, section 3.6.4
*/
class KMIME_EXPORT Ident : public Address
{
  public:
    Ident() : Address() {}
    Ident( Content * p ) : Address( p ) {}
    Ident( Content * p, const QByteArray & s )
      : Address( p ) { from7BitString( s ); }
    Ident( Content * p, const QString & s, const QByteArray & cs )
      : Address( p ) { fromUnicodeString( s, cs ); }
    ~Ident() {}

    virtual QByteArray as7BitString( bool withHeaderType = true );

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
    bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

    /** The list of msg-id's */
    QList<Types::AddrSpec> mMsgIdList;
};

/**
  Base class for headers which deal with a single msg-id.

  @see RFC 2822, section 3.6.4
*/
class KMIME_EXPORT SingleIdent : public Ident
{
  public:
    SingleIdent() : Ident() {}
    SingleIdent( Content * p ) : Ident( p ) {}
    SingleIdent( Content * p, const QByteArray & s )
      : Ident( p ) { from7BitString( s ); }
    SingleIdent( Content * p, const QString & s, const QByteArray & cs )
      : Ident( p ) { fromUnicodeString( s, cs ); }
    ~SingleIdent() {}

    /**
      Return the identifier contained in this header.
      Note: The identifiers is not enclosed in angle-brackets.
    */
    QByteArray identifier() const;

    /**
      Sets the identifier.
      @param id The new identifier with or without angle-brackets.
    */
    void setIdentifier( const QByteArray &id );

  protected:
    bool parse( const char* & scursor, const char * const send, bool isCRLF=false );
};

/** Base class for headers which deal with a single atom. */
class KMIME_EXPORT GToken : public Structured
{
  public:
    GToken() : Structured() {}
    GToken( Content *p ) : Structured( p ) {}
    GToken( Content *p, const QByteArray &s )
      : Structured( p ) { from7BitString( s ); }
    GToken( Content *p, const QString &s, const QByteArray &cs )
      : Structured( p ) { fromUnicodeString( s, cs ); }
    ~GToken() {}

  protected:
    bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

    QByteArray mToken;
};

class KMIME_EXPORT GPhraseList : public Structured
{
  public:
    GPhraseList() : Structured() {}
    GPhraseList( Content * p ) : Structured( p ) {}
    GPhraseList( Content * p, const QByteArray & s ) : Structured( p )
      { from7BitString( s ); }
    GPhraseList( Content * p, const QString & s, const QByteArray & cs ) : Structured( p )
      { fromUnicodeString( s, cs ); }
    ~GPhraseList() {}

  protected:
    bool parse( const char* &scursor, const char * const send, bool isCRLF=false );

    QStringList mPhraseList;
};

class KMIME_EXPORT GDotAtom : public Structured
{
  public:
    GDotAtom() : Structured() {}
    GDotAtom( Content *p ) : Structured( p ) {}
    GDotAtom( Content *p, const QByteArray &s ) : Structured( p )
      { from7BitString( s ); }
    GDotAtom( Content *p, const QString &s, const QByteArray &cs ) : Structured( p )
      { fromUnicodeString( s, cs ); }
    ~GDotAtom() {}

  protected:
    bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

    QString mDotAtom;
};

class KMIME_EXPORT GParametrized : public Structured
{
  public:
    GParametrized() : Structured() {}
    GParametrized( Content *p ) : Structured( p ) {}
    GParametrized( Content *p, const QByteArray &s ) : Structured( p )
      { from7BitString( s ); }
    GParametrized( Content *p, const QString & s, const QByteArray &cs ) : Structured( p )
      { fromUnicodeString( s, cs ); }
    ~GParametrized() {}

  protected:
    QMap<QString,QString> mParameterHash;

  private:
};

class KMIME_EXPORT GContentType : public GParametrized
{
  public:
    GContentType() : GParametrized() {}
    GContentType( Content *p ) : GParametrized( p ) {}
    GContentType( Content *p, const QByteArray &s ) : GParametrized( p )
      { from7BitString( s ); }
    GContentType( Content *p, const QString &s, const QByteArray &cs ) : GParametrized( p )
      { fromUnicodeString( s, cs ); }
    ~GContentType() {}

  protected:
    bool parse( const char* & scursor, const char * const send, bool isCRLF=false );

    QByteArray mMimeType;
    QByteArray mMimeSubType;
};

class KMIME_EXPORT GCISTokenWithParameterList : public GParametrized
{
  public:
    GCISTokenWithParameterList() : GParametrized() {}
    GCISTokenWithParameterList( Content *p ) : GParametrized( p ) {}
    GCISTokenWithParameterList( Content *p, const QByteArray &s ) : GParametrized( p )
      { from7BitString( s ); }
    GCISTokenWithParameterList( Content *p, const QString &s, const QByteArray &cs )
      : GParametrized( p )
      { fromUnicodeString( s, cs ); }
    ~GCISTokenWithParameterList() {}

  protected:
    bool parse( const char* &scursor, const char * const send, bool isCRLF=false );

    QByteArray mToken;
};

} // namespace Generics

//
//
// INCOMPATIBLE, GSTRUCTURED-BASED FIELDS:
//
//

/** Represents the Return-Path header field. */
class KMIME_EXPORT ReturnPath : public Generics::Address
{
  public:
    ReturnPath() : Generics::Address() {}
    ReturnPath( Content *p ) : Generics::Address( p ) {}
    ReturnPath( Content *p, const QByteArray &s ) : Generics::Address( p )
      { from7BitString( s ); }
    ReturnPath( Content *p, const QString &s, const QByteArray &cs ) : Generics::Address( p )
      { fromUnicodeString( s, cs ); }
    ~ReturnPath() {}

    const char * type() const { return "Return-Path"; }

  protected:
    bool parse( const char* &scursor, const char * const send, bool isCRLF=false );
};


mk_trivial_subclass_with_name( ContentDescription, Content-Description, Unstructured );
#if defined(KMIME_NEW_STYLE_CLASSTREE)
// classes whose names collide with earlier ones:

// Address et al.:

// rfc(2)822 headers:
mk_trivial_subclass( From, MailboxList );
mk_trivial_subclass( Sender, SingleMailbox );
mk_trivial_subclass_with_name( ReplyTo, Reply-To, AddressList );
mk_trivial_subclass( Cc, AddressList );
mk_trivial_subclass( Bcc, AddressList );
// usefor headers:
mk_trivial_subclass_with_name( MailCopiesTo, Mail-Copies-To, AddressList );

// GToken:

mk_trivial_subclass_with_name( ContentTransferEncoding,
                               Content-Transfer-Encoding, GToken );

// GPhraseList:

mk_trivial_subclass( Keywords, GPhraseList );

// GDotAtom:

mk_trivial_subclass_with_name( MIMEVersion, MIME-Version, GDotAtom );
#endif

// Ident:

/** Represents a "Message-Id" header */
class KMIME_EXPORT MessageID : public Generics::SingleIdent {

  public:
    MessageID() : Generics::SingleIdent()  {}
    MessageID(Content *p) : Generics::SingleIdent(p) {}
    MessageID(Content *p, const QByteArray &s) : Generics::SingleIdent(p) { from7BitString(s); }
    MessageID(Content *p, const QString &s) : Generics::SingleIdent(p)  { fromUnicodeString(s, Latin1); }
    ~MessageID()  {}

    virtual const char* type() const { return "Message-Id"; }

    /**
      Generate a message identifer.
      @param fqdn A fully qualified domain name.
    */
    void generate(const QByteArray &fqdn);
};

/** Represents a "Content-ID" header. */
mk_trivial_subclass_with_name( ContentID, Content-ID, SingleIdent );

/** Represents a "Supersedes" header. */
mk_trivial_subclass( Supersedes, SingleIdent );

/** Represents a "In-Reply-To" header. */
mk_trivial_subclass_with_name( InReplyTo, In-Reply-To, Ident );

/** Represents a "References" header. */
mk_trivial_subclass( References, Ident );

#if defined(KMIME_NEW_STYLE_CLASSTREE)

// GContentType:

mk_trivial_subclass_with_name( ContentType, ContentType, GContentType );

// GCISTokenWithParameterList:

mk_trivial_subclass_with_name( ContentDisposition, Content-Disposition,
			       GCISTokenWithParameterList );

#endif

//
//
// COMPATIBLE GUNSTRUCTURED-BASED FIELDS:
//
//

/** Represents an arbitrary header, that can contain
    any header-field.
    Adds a type over Unstructured.
    @see Unstructured
*/
class KMIME_EXPORT Generic : public Generics::Unstructured
{
  public:
    Generic() : Generics::Unstructured(), t_ype( 0 ) {}
    Generic( const char *t ) : Generics::Unstructured(), t_ype( 0 )
      { setType( t ); }
    Generic( const char *t, Content *p )
      : Generics::Unstructured( p ), t_ype( 0 )
      { setType( t ); }
    Generic( const char *t, Content *p, const QByteArray &s )
      : Generics::Unstructured( p, s ), t_ype( 0 )
      { setType( t ); }
    Generic( const char *t, Content *p, const QString &s, const QByteArray &cs )
      : Generics::Unstructured( p, s, cs ), t_ype( 0 )
      { setType( t ); }
    ~Generic() { delete[] t_ype; }

    virtual void clear() { delete[] t_ype; Unstructured::clear(); }
    virtual bool isEmpty() const
      { return ( t_ype == 0 || Unstructured::isEmpty() ); }
    virtual const char *type() const
      { return t_ype; }
    void setType( const char *type );

  protected:
    char *t_ype;
};

/** Represents a "Subject" header */
class KMIME_EXPORT Subject : public Generics::Unstructured
{
  public:
    Subject() : Generics::Unstructured() {}
    Subject( Content *p ) : Generics::Unstructured( p ) {}
    Subject( Content *p, const QByteArray &s )
      : Generics::Unstructured( p, s ) {}
    Subject( Content *p, const QString &s, const QByteArray &cs )
      : Generics::Unstructured( p, s, cs ) {}
    ~Subject() {}

    virtual const char *type() const { return "Subject"; }

    bool isReply() {
      return ( asUnicodeString().indexOf( QLatin1String( "Re:" ), 0, Qt::CaseInsensitive ) == 0 );
    }
};

/** Represents a "Organization" header */
class KMIME_EXPORT Organization : public Generics::Unstructured
{
  public:
    Organization() : Generics::Unstructured() {}
    Organization( Content *p ) : Generics::Unstructured( p ) {}
    Organization( Content *p, const QByteArray &s )
      : Generics::Unstructured( p, s ) {}
    Organization( Content *p, const QString &s, const QByteArray &cs )
      : Generics::Unstructured( p, s, cs ) {}
    ~Organization() {}

    virtual const char *type() const { return "Organization"; }
};

//
//
// NOT YET CONVERTED STUFF BELOW:
//
//

/** Represents a "Control" header */
class KMIME_EXPORT Control : public Base
{
  public:
    Control() : Base() {}
    Control( Content *p ) : Base( p ) {}
    Control( Content *p, const QByteArray &s ) : Base( p )
      { from7BitString( s ); }
    Control( Content *p, const QString &s ) : Base( p )
      { fromUnicodeString( s, Latin1 ); }
    ~Control() {}

    virtual void from7BitString( const QByteArray &s );
    virtual QByteArray as7BitString( bool incType=true );
    virtual void fromUnicodeString( const QString &s, const QByteArray &b );
    virtual QString asUnicodeString();
    virtual void clear() { c_trlMsg.truncate( 0 ); }
    virtual bool isEmpty() const { return ( c_trlMsg.isEmpty() ); }
    virtual const char *type() const { return "Control"; }

    bool isCancel()
      { return QString::fromLatin1( c_trlMsg ).contains(
        QLatin1String( "cancel" ), Qt::CaseInsensitive ); }

  protected:
    QByteArray c_trlMsg;
};

/** Represents a "Date" header */
class KMIME_EXPORT Date : public Base
{
  public:
    Date() : Base(), t_ime( 0 ) {}
    Date( Content *p ) : Base( p ), t_ime( 0 ) {}
    Date( Content *p, time_t t ) : Base( p ), t_ime( t ) {}
    Date( Content *p, const QByteArray &s ) : Base( p )
      { from7BitString( s ); }
    Date( Content *p, const QString &s ) : Base( p )
      { fromUnicodeString( s, Latin1 ); }
    ~Date() {}

    virtual void from7BitString( const QByteArray &s );
    virtual QByteArray as7BitString( bool incType=true );
    virtual void fromUnicodeString( const QString &s, const QByteArray &b );
    virtual QString asUnicodeString();
    virtual void clear() { t_ime=0; }
    virtual bool isEmpty() const { return (t_ime == 0); }
    virtual const char *type() const { return "Date"; }

    time_t unixTime() { return t_ime; }
    void setUnixTime( time_t t ) { t_ime=t; }
    void setUnixTime() { t_ime=time( 0 ); }
    QDateTime qdt();
    int ageInDays();

  protected:
    time_t t_ime;
};

/** Represents a "Newsgroups" header */
class KMIME_EXPORT Newsgroups : public Base
{
  public:
    Newsgroups() : Base() {}
    Newsgroups( Content *p ) : Base( p ) {}
    Newsgroups( Content *p, const QByteArray &s ) : Base( p )
      { from7BitString( s ); }
    Newsgroups( Content *p, const QString &s ) : Base( p )
      { fromUnicodeString( s, Latin1 ); }
    ~Newsgroups() {}

    virtual void from7BitString( const QByteArray &s );
    virtual QByteArray as7BitString( bool incType=true );
    virtual void fromUnicodeString( const QString &s, const QByteArray &b );
    virtual QString asUnicodeString();
    virtual void clear() { g_roups.clear(); }
    virtual bool isEmpty() const { return g_roups.isEmpty(); }
    virtual const char *type() const { return "Newsgroups"; }

    QByteArray firstGroup();
    bool isCrossposted() { return g_roups.contains( ',' ); }
    QStringList getGroups();

  protected:
    QByteArray g_roups;
};

/** Represents a "Followup-To" header */
class KMIME_EXPORT FollowUpTo : public Newsgroups
{
  public:
    FollowUpTo() : Newsgroups() {}
    FollowUpTo( Content *p ) : Newsgroups( p ) {}
    FollowUpTo( Content *p, const QByteArray &s ) : Newsgroups( p, s ) {}
    FollowUpTo( Content *p, const QString &s ) : Newsgroups( p, s ) {}
    ~FollowUpTo() {}

    virtual const char *type() const { return "Followup-To"; }
};

/** Represents a "Lines" header */
class KMIME_EXPORT Lines : public Base
{
  public:
    Lines() : Base(), l_ines( -1 ) {}
    Lines( Content *p ) : Base( p ), l_ines( -1 ) {}
    Lines( Content *p, unsigned int i ) : Base( p ), l_ines( i ) {}
    Lines( Content *p, const QByteArray &s ) : Base( p )
      { from7BitString( s ); }
    Lines( Content *p, const QString &s ) : Base( p )
      { fromUnicodeString( s, Latin1 ); }
    ~Lines() {}

    virtual void from7BitString( const QByteArray &s );
    virtual QByteArray as7BitString( bool incType=true );
    virtual void fromUnicodeString( const QString &s, const QByteArray &b );
    virtual QString asUnicodeString();
    virtual void clear() { l_ines=-1; }
    virtual bool isEmpty() const { return( l_ines == -1 ); }
    virtual const char *type() const { return "Lines"; }

    int numberOfLines() { return l_ines; }
    void setNumberOfLines( int i ) { l_ines = i; }

  protected:
    int l_ines;
};

/** Represents a "User-Agent" header */
class KMIME_EXPORT UserAgent : public Base
{
  public:
    UserAgent() : Base() {}
    UserAgent( Content *p ) : Base( p ) {}
    UserAgent( Content *p, const QByteArray &s ) : Base( p )
      { from7BitString( s ); }
    UserAgent( Content *p, const QString &s ) : Base( p )
      { fromUnicodeString( s, Latin1 ); }
    ~UserAgent() {}

    virtual void from7BitString( const QByteArray &s );
    virtual QByteArray as7BitString( bool incType=true );
    virtual void fromUnicodeString( const QString &s, const QByteArray &b );
    virtual QString asUnicodeString();
    virtual void clear() { u_agent.resize( 0 ); }
    virtual bool isEmpty() const { return (u_agent.isEmpty()); }
    virtual const char *type() const { return "User-Agent"; }

  protected:
    QByteArray u_agent;
};

#if !defined(KMIME_NEW_STYLE_CLASSTREE)
#include "kmime_headers_obs.h"
#endif
}  //namespace Headers

}  //namespace KMime

#endif // __KMIME_HEADERS_H__
