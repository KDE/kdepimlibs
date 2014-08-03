/*
    kmime_content.h

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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
  defines the Content class.

  @brief
  Defines the Content class.

  @authors the KMime authors (see AUTHORS file),
  Volker Krause \<vkrause@kde.org\>

TODO: possible glossary terms:
 content
   encoding, transfer type, disposition, description
 header
 body
 attachment
 charset
 article
 string representation
 broken-down object representation
*/

#ifndef __KMIME_CONTENT_H__
#define __KMIME_CONTENT_H__

#include "kmime_export.h"
#include "kmime_contentindex.h"
#include "kmime_util.h"
#include "kmime_headers.h"

#include <QtCore/QTextStream>
#include <QtCore/QByteArray>
#include <QtCore/QList>

#include <boost/shared_ptr.hpp>

namespace KMime
{

class ContentPrivate;
class Message;

/**
  @brief
  A class that encapsulates @ref MIME encoded Content.

  A Content object holds two representations of a content:
  - the string representation: This is the content encoded as a string ready
    for transport.  Accessible through the encodedContent() method.
  - the broken-down representation: This is the tree of objects (headers,
    sub-Contents and (if present) the encapsulated message) that this Content is made of.
    Accessible through methods like header(), contents() and bodyAsMessage().

  The parse() function updates the broken-down representation of the Content
  from its string representation.  Calling it is necessary to access the
  headers, sub-Contents or the encapsulated message of this Content.

  The assemble() function updates the string representation of the Content
  from its broken-down representation.  Calling it is necessary for
  encodedContent() to reflect any changes made to the broken-down representation of the Content.

  There are two basic types of a Content:
  - A leaf Content: This is a content that is neither a multipart content nor an encapsulated
                    message. Because of this, it will not have any children, it has no sub-contents
                    and is therefore a leaf content.
                    Only leaf contents have a body that is not empty, i.e. functions that operate
                    on the body, such as body(), size() and decodedContent(), will work only on
                    leaf contents.
  - A non-leaf Content: This is a content that itself doesn't have any body, but that does have
                        sub-contents.
                        This is the case for contents that are of mimetype multipart/ or of mimetype
                        message/rfc822. In case of a multipart content, contents() will return the
                        multipart child contents. In case of an encapsulated message, the message
                        can be accessed with bodyAsMessage(), and contents() will have one entry
                        that is the message as well.
                        On a non-leaf content, body() will have an empty return value and other
                        functions working on the body will not work.
                        A call to parse() is required before the child multipart contents or the
                        encapsulated message is created.
*/
/*
  KDE5:
  * Do not convert singlepart <-> multipart automatically.
  * A bunch of methods probably don't need to be virtual (since they're not needed
    in either Message or NewsArticle).
*/
class KMIME_EXPORT Content
{
public:

    /**
      Describes a list of Content objects.
    */
    typedef QList<KMime::Content *> List;

    /**
      Creates an empty Content object.
    */
    Content();

    /**
      Creates an empty Content object with a specified parent.
      @param parent the parent Content object
      @since 4.3
    */
    explicit Content(Content *parent);   // KDE5: Merge with the above.

    /**
      Creates a Content object containing the given raw data.

      @param head is a QByteArray containing the header data.
      @param body is a QByteArray containing the body data.
    */
    Content(const QByteArray &head, const QByteArray &body);

    /**
      Creates a Content object containing the given raw data.

      @param head is a QByteArray containing the header data.
      @param body is a QByteArray containing the body data.
      @param parent the parent Content object
      @since 4.3
    */
    // KDE5: Merge with the above.
    Content(const QByteArray &head, const QByteArray &body, Content *parent);

    /**
      Destroys this Content object.
    */
    virtual ~Content();

    /**
      Returns true if this Content object is not empty.
    */
    bool hasContent() const;

    /**
      Sets the Content to the given raw data, containing the Content head and
      body separated by two linefeeds.

      This method operates on the string representation of the Content. Call
      parse() if you want to access individual headers, sub-Contents or the
      encapsulated message.

      @param l is a list of the raw Content data, split by lines.
    */
    void setContent(const QList<QByteArray> &l);

    /**
      Sets the Content to the given raw data, containing the Content head and
      body separated by two linefeeds.

      This method operates on the string representation of the Content. Call
      parse() if you want to access individual headers, sub-Contents or the
      encapsulated message.

      @note The passed data must not contain any CRLF sequences, only LF.
            Use CRLFtoLF for conversion before passing in the data.

      @param s is a QByteArray containing the raw Content data.
    */
    void setContent(const QByteArray &s);

    /**
     * Parses the Content.
     *
     * This means the broken-down object representation of the Content is
     * updated from the string representation of the Content.
     *
     * Call this if you want to access or change headers, sub-Contents or the encapsulated
     * message.
     *
     * @note Calling parse() twice will not work for multipart contents or for contents of which
     *       the body is an encapsulated message. The reason is that the first parse() will delete
     *       the body, so there is no body to work on for the second call of parse().
     *
     * @note Calling this will reset the message returned by bodyAsMessage(), as
     *       the message is re-parsed as well.
     *       Also, all old sub-contents will be deleted, so any old Content pointer will become
     *       invalid.
     */
    virtual void parse();

    /**
      Returns whether this Content is frozen.
      A frozen content is immutable, i.e. calling assemble() will never modify
      its head or body, and encodedContent() will return the same data before
      and after parsing.

      @since 4.4.
      @see setFrozen().
    */
    bool isFrozen() const;

    /**
      Freezes this Content if @p frozen is true; otherwise unfreezes it.
      @param frozen freeze content if @c true, otherwise unfreeze
      @since 4.4
      @see isFrozen().
    */
    void setFrozen(bool frozen = true);

    /**
      Generates the MIME content.
      This means the string representation of this Content is updated from the
      broken-down object representation.
      Call this if you have made changes to the content, and want
      encodedContent() to reflect those changes.

      @note assemble() has no effect if the Content isFrozen().  You may want
      to freeze, for instance, signed sub-Contents, to make sure they are kept
      unmodified.

      @note If this content is an encapsulated message, i.e. bodyIsMessage() returns true,
      then calling assemble() will also assemble the message returned by bodyAsMessage().

      @warning assemble() may change the order of the headers, and other
      details such as where folding occurs.  This may break things like
      signature verification, so you should *ONLY* call assemble() when you
      have actually modified the content.
    */
    virtual void assemble();

    /**
      Clears the content, deleting all headers and sub-Contents.
    */
    // KDE5: make non-virtual.
    virtual void clear();

    /**
      Removes all sub-Contents from this content.  Deletes them if @p del is true.
      This is different from calling removeContent() on each sub-Content, because
      removeContent() will convert this to a single-part Content if only one
      sub-Content is left.  Calling clearContents() does NOT make this Content
      single-part.

      @param del Whether to delete the sub-Contents.
      @see removeContent()
      @since 4.4
    */
    void clearContents(bool del = true);

    /**
      Returns the Content header raw data.

      @see setHead().
    */
    QByteArray head() const;

    /**
      Sets the Content header raw data.

      This method operates on the string representation of the Content. Call
      parse() if you want to access individual headers.

      @param head is a QByteArray containing the header data.

      @see head().
    */
    void setHead(const QByteArray &head);

    /**
      Extracts and removes the next header from @p head.
      The caller is responsible for deleting the returned header.

      @deprecated Use KMime::HeaderParsing::extractFirstHeader().
      @param head is a QByteArray containing the header data.
    */
    KMIME_DEPRECATED Headers::Generic *getNextHeader(QByteArray &head);

    /**
      Extracts and removes the next header from @p head.
      The caller is responsible for deleting the returned header.
      @since 4.2
      @deprecated Use KMime::HeaderParsing::extractFirstHeader().
      @param head is a QByteArray containing the header data.
    */
    // KDE5: Remove this. This method has nothing to do with *this object.
    KMIME_DEPRECATED Headers::Generic *nextHeader(QByteArray &head);

    /**
      Tries to find a @p type header in the Content and returns it.
      @param type the header type to find
      @deprecated Use headerByType( const char * )
    */
    // KDE5: Make non-virtual.
    KMIME_DEPRECATED virtual Headers::Base *getHeaderByType(const char *type);

    /**
      Returns the first header of type @p type, if it exists.  Otherwise returns 0.
      Note that the returned header may be empty.
      @param type the header type to find
      @since 4.2
    */
    // KDE5: Make non-virtual.
    virtual Headers::Base *headerByType(const char *type);

    /**
      Returns the first header of type T, if it exists.
      If the header does not exist and @p create is true, creates an empty header
      and returns it. Otherwise returns 0.
      Note that the returned header may be empty.
      @param create Whether to create the header if it does not exist.
      @since 4.4.

      KDE5: BIC: FIXME: Why is the default argument false here? That is inconsistent with the
                        methods in KMime::Message!
    */
    template <typename T> T *header(bool create = false);

    /**
      Returns all @p type headers in the Content.
      Take care that this result is not cached, so could be slow.
      @param type the header type to find
      @since 4.2
    */
    virtual QList<Headers::Base *> headersByType(const char *type);

    /**
      Sets the specified header to this Content.
      Any previous header of the same type is removed.
      If you need multiple headers of the same type, use appendHeader() or
      prependHeader().

      @param h The header to set.
      @see appendHeader()
      @see removeHeader()
      @since 4.4
    */
    // KDE5: make non-virtual.
    virtual void setHeader(Headers::Base *h);

    /**
      Appends the specified header to the headers of this Content.
      @param h The header to append.
      @since 4.4
    */
    void appendHeader(Headers::Base *h);

    /**
      Prepends the specified header to the headers of this Content.
      @param h The header to prepend.
      @since 4.4
    */
    void prependHeader(Headers::Base *h);

    /**
      Searches for the first header of type @p type, and deletes it, removing
      it from this Content.
      @param type The type of the header to look for.
      @return true if a header was found and removed.
    */
    // TODO probably provide removeHeader<T>() too.
    // KDE5: make non-virtual.
    virtual bool removeHeader(const char *type);

    /**
      @return true if this Content has a header of type @p type.
      @param type The type of the header to look for.
    */
    // TODO probably provide hasHeader<T>() too.
    // TODO: KDE5: make const
    bool hasHeader(const char *type);

    /**
      Returns the Content-Type header.

      @param create If true, create the header if it doesn't exist yet.
    */
    Headers::ContentType *contentType(bool create = true);

    /**
      Returns the Content-Transfer-Encoding header.

      @param create If true, create the header if it doesn't exist yet.
    */
    Headers::ContentTransferEncoding *contentTransferEncoding(bool create = true);

    /**
      Returns the Content-Disposition header.

      @param create If true, create the header if it doesn't exist yet.
    */
    Headers::ContentDisposition *contentDisposition(bool create = true);

    /**
      Returns the Content-Description header.

      @param create If true, create the header if it doesn't exist yet.
    */
    Headers::ContentDescription *contentDescription(bool create = true);

    /**
      Returns the Content-Location header.

      @param create If true, create the header if it doesn't exist yet.
      @since 4.2
    */
    Headers::ContentLocation *contentLocation(bool create = true);

    /**
      Returns the Content-ID header.
      @param create if true, create the header if it does not exist yet.
      @since 4.4
    */
    Headers::ContentID *contentID(bool create = true);

    /**
      Returns the size of the Content body after encoding.
      (If the encoding is quoted-printable, this is only an approximate size.)
      This will return 0 for multipart contents or for encapsulated messages.
    */
    int size();

    /**
      Returns the size of this Content and all sub-Contents.
    */
    int storageSize() const;

    /**
      Line count of this Content and all sub-Contents.
    */
    int lineCount() const;

    /**
      Returns the Content body raw data.

      Note that this will be empty for multipart contents or for encapsulated messages,
      after parse() has been called.

      @see setBody().
    */
    QByteArray body() const;

    /**
      Sets the Content body raw data.

      This method operates on the string representation of the Content. Call
      parse() if you want to access individual sub-Contents or the encapsulated message.

      @param body is a QByteArray containing the body data.

      @see body().
    */
    void setBody(const QByteArray &body);

    /**
      Returns the MIME preamble.

      @return a QByteArray containing the MIME preamble.

      @since 4.9
     */
    QByteArray preamble() const;

    /**
      Sets the MIME preamble.

      @param preamble a QByteArray containing what will be used as the
      MIME preamble.

      @since 4.9
     */

    void setPreamble(const QByteArray &preamble);

    /**
      Returns the MIME preamble.

      @return a QByteArray containing the MIME epilogue.

      @since 4.9
     */
    QByteArray epilogue() const;

    /**
      Sets the MIME preamble.

      @param epilogue a QByteArray containing what will be used as the
      MIME epilogue.

      @since 4.9
     */
    void setEpilogue(const QByteArray &epilogue);

    /**
      Returns a QByteArray containing the encoded Content, including the
      Content header and all sub-Contents.

      If you make changes to the broken-down representation of the message, be
      sure to first call assemble() before calling encodedContent(), otherwise
      the result will not be up-to-date.

      If this content is an encapsulated message, i.e. bodyIsMessage() returns true,
      then encodedContent() will use the message returned by bodyAsMessage() as the
      body of the result, calling encodedContent() on the message.

      @param useCrLf If true, use @ref CRLF instead of @ref LF for linefeeds.
    */
    QByteArray encodedContent(bool useCrLf = false);

    /**
     * Like encodedContent(), with the difference that only the body will be returned, i.e. the
     * headers are excluded.
     *
     * @since 4.6
     */
    QByteArray encodedBody();

    /**
     * Returns the decoded Content body.
     *
     * Note that this will be empty for multipart contents or for encapsulated messages,
     * after parse() has been called.
     */
    // TODO: KDE5: BIC: Rename this to decodedBody(), since only the body is returned.
    // In contrast, setContent() sets the head and the body!
    // Also, try to make this const.
    QByteArray decodedContent();

    /**
      Returns the decoded text. Additional to decodedContent(), this also
      applies charset decoding. If this is not a text Content, decodedText()
      returns an empty QString.

      @param trimText If true, then the decoded text will have all trailing
      whitespace removed.
      @param removeTrailingNewlines If true, then the decoded text will have
      all consecutive trailing newlines removed.

      The last trailing new line of the decoded text is always removed.

    */
    // TODO: KDE5: BIC: Convert to enums. Also, what if trimText = true but removeTrailingNewlines
    //                  is false?
    QString decodedText(bool trimText = false,
                        bool removeTrailingNewlines = false);

    /**
      Sets the Content body to the given string using charset of the content type.

      If the charset can not be found, the system charset is taken and the content type header is
      changed to that charset.
      The charset of the content type header should be set to a charset that can encode the given
      string before calling this method.

      This method does not set the content transfer encoding automatically, it needs to be set
      to a suitable value that can encode the given string before calling this method.

      This method only makes sense for single-part contents, do not try to pass a multipart body
      or an encapsulated message here, that wouldn't work.

      @param s Unicode-encoded string.
    */
    void fromUnicodeString(const QString &s);

    /**
      Returns the first Content with mimetype text/.
    */
    Content *textContent();

    /**
      Returns a list of attachments.

      @param incAlternatives If true, include multipart/alternative parts.
    */
    List attachments(bool incAlternatives = false);

    /**
     * For multipart contents, this will return a list of all multipart child contents.
     * For contents that are of mimetype message/rfc822, this will return a list with one entry,
     * and that entry is the encapsulated message, as it would be returned by bodyAsMessage().
     */
    List contents() const;

    /**
      Adds a new sub-Content. If the sub-Content is already part of another
      Content object, it is removed from there and its parent is updated.
      If the current Content object is single-part, it is converted to
      multipart/mixed first.

      @warning If the single-part to multipart conversion happens, all
      pointers you may have into this object (such as headers) will become
      invalid!

      @param content The new sub-Content.
      @param prepend If true, prepend to the Content list; otherwise append.
      to the Content list.

      @see removeContent().
    */
    // KDE5: Do not convert single-part->multipart automatically.
    void addContent(Content *content, bool prepend = false);

    /**
      Removes the given sub-Content. If only one sub-Content is left, the
      current Content object is converted into a single-part Content.

      @warning If the multipart to single-part conversion happens, the head
      and body of the single remaining sub-Content are copied over, and the
      sub-Content is deleted.  All pointers to it or into it (such as headers)
      will become invalid!

      @param content The Content to remove.
      @param del If true, delete the removed Content object. Otherwise set its
      parent to 0.

      @see addContent().
      @see clearContents().
    */
    // KDE5: Do not convert multipart->single-part automatically.
    void removeContent(Content *content, bool del = false);

    /**
      Changes the encoding of this Content to @p e.  If the Content is binary,
      this actually re-encodes the data to use the new encoding.

      @param e The new encoding to use.
    */
    void changeEncoding(Headers::contentEncoding e);

    /**
      Saves the encoded Content to the given textstream

      @param ts is the stream where the Content should be written to.
      @param scrambleFromLines: If true, replace "\nFrom " with "\n>From "
      in the stream. This is needed to avoid problem with mbox-files
    */
    void toStream(QTextStream &ts, bool scrambleFromLines = false);

    // NOTE: The charset methods below are accessed by the headers which
    // have this Content as a parent.

    /**
      Returns the charset that is used to decode RFC2047 strings in all headers and to decode
      the body if the charset is not declared explictly.
      It is also used as the charset when encoding RFC2047 strings in headers.

      @see setDefaultCharset()
    */
    // TODO: Split this up into a charset for encoding and one for decoding, and make the one for
    //       encoding UTF-8 by default.
    QByteArray defaultCharset() const;

    /**
      Sets the default charset.

      @param cs is a QByteArray containing the new default charset.

      @see defaultCharset().
    */
    void setDefaultCharset(const QByteArray &cs);

    /**
      Use the default charset even if a different charset is
      declared in the article.

      @see setForceDefaultCharset().
    */
    bool forceDefaultCharset() const;

    /**
      Enables/disables the force mode, housekeeping.
      works correctly only when the article is completely empty or
      completely loaded.

      @param b If true, force the default charset to be used.

      @see forceDefaultCharset().
    */
    virtual void setForceDefaultCharset(bool b);

    /**
      Returns the Content specified by the given index.
      If the index does not point to a Content, 0 is returned. If the index
      is invalid (empty), this Content is returned.

      @param index The Content index.
    */
    Content *content(const ContentIndex &index) const;

    /**
      Returns the ContentIndex for the given Content, or an invalid index
      if the Content is not found within the hierarchy.
      @param content the Content object to search.
    */
    ContentIndex indexForContent(Content *content) const;

    /**
      Returns true if this is the top-level node in the MIME tree. The top-level node is always
      a Message or NewsArticle. However, a node can be a Message without being a top-level node when
      it is an encapsulated message.
    */
    virtual bool isTopLevel() const;

    /**
     * Sets a new parent to the Content and add to its contents list. If it already had a parent, it is removed from the
     * old parents contents list.
     * @param parent the new parent
     * @since 4.3
     */
    void setParent(Content *parent);

    /**
     * Returns the parent content object, or 0 if the content doesn't have a parent.
     * @since 4.3
     */
    Content *parent() const;

    /**
     * Returns the toplevel content object, 0 if there is no such object.
     * @since 4.3
     */
    Content *topLevel() const;

    /**
     * Returns the index of this Content based on the topLevel() object.
     * @since 4.3
     */
    ContentIndex index() const;

    /**
     * @return true if this content is an encapsulated message, i.e. if it has the mimetype
     *         message/rfc822.
     *
     * @since 4.5
     */
    //AK_REVIEW: move to MessageViewer/ObjectTreeParser
    bool bodyIsMessage() const;

    /**
     * If this content is an encapsulated message, in which case bodyIsMessage() will return
     * true, the message represented by the body of this content will be returned.
     * The returned message is already fully parsed.
     * Calling this method is the aquivalent of calling contents().first() and casting the result
     * to a KMime::Message*. bodyAsMessage() has the advantage that it will return a shared pointer
     * that will not be destroyed when the container message is destroyed or re-parsed.
     *
     * The message that is returned here is created when calling parse(), so make sure to call
     * parse() first. Since each parse() creates a new message object, a different message object
     * will be returned each time you call parse().
     *
     * If you make changes to the returned message, you need to call assemble() on this content
     * or on the message if you want that encodedContent() reflects these changes. This also means
     * that calling assemble() on this content will assemble the returned message.
     *
     * @since 4.5
     */
    //AK_REVIEW: move to MessageViewer/ObjectTreeParser
    boost::shared_ptr<Message> bodyAsMessage() const;

protected:
    /**
      Reimplement this method if you need to assemble additional headers in a
      derived class. Don't forget to call the implementation of the base class.
      @return The raw, assembled headers.
    */
    virtual QByteArray assembleHeaders();

    /**
      Returns the raw string representing the header of type @p name.
      @param name the header type to find
      @deprecated Use KMime::extractHeader() directly instead.
    */
    KMIME_DEPRECATED QByteArray rawHeader(const char *name) const;

    /**
      Returns a list of raw strings representing all header of type @p name.
      @deprecated Use KMime::extractHeaders() directly instead.
    */
    KMIME_DEPRECATED QList<QByteArray> rawHeaders(const char *name) const;

    /**
      Returns whether this object holds text content.
    */
    // KDE5: Not needed outside. Move to Private class.
    bool decodeText();

    /**
      Returns the first header of type T, if it exists.
      @deprecated Use header() instead.
    */
    template <class T> KMIME_DEPRECATED T *headerInstance(T *ptr, bool create);

    /**
      The list of headers in this Content.
      Do not use this directly.
    */
    // KDE5: Not needed outside. Move to Private class.
    Headers::Base::List h_eaders;

    //@cond PRIVATE
    ContentPrivate *d_ptr;
    explicit Content(ContentPrivate *d);
    //@endcond

private:
    Q_DECLARE_PRIVATE(Content)
    Q_DISABLE_COPY(Content)
};

// some compilers (for instance Compaq C++) need template inline functions
// here rather than in the *.cpp file

template <class T> T *Content::headerInstance(T */*ptr*/, bool create)
{
    return header<T>(create);
}

template <typename T> T *Content::header(bool create)
{
    Headers::Base *h = headerByType(T::staticType());
    if (h) {
        // Make sure the header is actually of the right type.
        Q_ASSERT(dynamic_cast<T *>(h));
    } else if (create) {
        h = new T(this);
        setHeader(h);
    }
    return static_cast<T *>(h);
}

} // namespace KMime

#endif // __KMIME_CONTENT_H__
