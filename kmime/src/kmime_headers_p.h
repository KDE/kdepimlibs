/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KMIME_HEADERS_P_H
#define KMIME_HEADERS_P_H

//@cond PRIVATE

#define kmime_mk_empty_private( subclass, base ) \
class subclass##Private : public base##Private {};

namespace KMime {

namespace Headers {

class BasePrivate
{
  public:
    BasePrivate() : parent( 0 ) {}

    virtual ~BasePrivate() {}

    KMime::Content *parent;
    QByteArray encCS;
};

namespace Generics {

class UnstructuredPrivate : public BasePrivate
{
  public:
    QString decoded;
};

kmime_mk_empty_private( Structured, Base )
kmime_mk_empty_private( Address, Structured )

class MailboxListPrivate : public AddressPrivate
{
  public:
    QList<Types::Mailbox> mailboxList;
};

kmime_mk_empty_private( SingleMailbox, MailboxList )

class AddressListPrivate : public AddressPrivate
{
  public:
    QList<Types::Address> addressList;
};

class IdentPrivate : public AddressPrivate
{
  public:
    QList<Types::AddrSpec> msgIdList;
    mutable QByteArray cachedIdentifier;
};

kmime_mk_empty_private( SingleIdent, Ident )

class TokenPrivate : public StructuredPrivate
{
  public:
    QByteArray token;
};

class PhraseListPrivate : public StructuredPrivate
{
  public:
    QStringList phraseList;
};

class DotAtomPrivate : public StructuredPrivate
{
  public:
    QString dotAtom;
};

class ParametrizedPrivate : public StructuredPrivate
{
  public:
    QMap<QString, QString> parameterHash;
};

} // namespace Generics

class ReturnPathPrivate : public Generics::AddressPrivate
{
  public:
    Types::Mailbox mailbox;
};

class MailCopiesToPrivate : public Generics::AddressListPrivate
{
  public:
    bool alwaysCopy;
    bool neverCopy;
};

class ContentTransferEncodingPrivate : public Generics::TokenPrivate
{
  public:
    contentEncoding cte;
    bool decoded;
};

class ContentTypePrivate : public Generics::ParametrizedPrivate
{
  public:
    QByteArray mimeType;
    contentCategory category;
};

class ContentDispositionPrivate : public Generics::ParametrizedPrivate
{
  public:
    contentDisposition disposition;
};

class GenericPrivate : public Generics::UnstructuredPrivate
{
  public:
    GenericPrivate() : type( 0 ) {}
    ~GenericPrivate()
    {
      delete[] type;
    }

    char *type;
};

class ControlPrivate : public Generics::StructuredPrivate
{
  public:
    QByteArray name;
    QByteArray parameter;
};

class DatePrivate : public Generics::StructuredPrivate
{
  public:
    QDateTime dateTime;
};

class NewsgroupsPrivate : public Generics::StructuredPrivate
{
  public:
    QList<QByteArray> groups;
};

class LinesPrivate : public Generics::StructuredPrivate
{
  public:
    int lines;
};

kmime_mk_empty_private( ContentID, Generics::SingleIdent )
}

}

#undef kmime_mk_empty_private

//@endcond

#endif
