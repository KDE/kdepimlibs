/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "transporttypeinfo.h"

#include "akonadiconfigwidget.h"
#include "sendmailconfigwidget.h"
#include "smtpconfigwidget.h"
#include "akonadijob.h"
#include "sendmailjob.h"
#include "smtpjob.h"
#include "transport.h"

#include <QObject>
#include <QWidget>

#include <KLocalizedString>

using namespace MailTransport;

int TransportTypeInfo::typeCount()
{
  return 3;
}

QString TransportTypeInfo::nameForType( int type )
{
  switch( type ) {
    case Transport::EnumType::SMTP:
      return i18nc( "@option SMTP transport", "SMTP" );
    case Transport::Transport::EnumType::Sendmail:
      return i18nc( "@option sendmail transport", "Sendmail" );
    case Transport::EnumType::Akonadi:
      return i18nc( "@option Akonadi Resource transport", "Akonadi Resource" );
  }
  Q_ASSERT( false );
  return QString();
}

QString TransportTypeInfo::descriptionForType( int type )
{
  // TODO polish these
  switch( type ) {
    case Transport::EnumType::SMTP:
      return i18n( "An SMTP server on the internet" );
    case Transport::EnumType::Sendmail:
      return i18n( "A local sendmail installation" );
    case Transport::EnumType::Akonadi:
      return i18n( "A local Akonadi resource with the ability to send mail" );
  }
  Q_ASSERT( false );
  return QString();
}

TransportJob *TransportTypeInfo::jobForTransport( Transport *transport, QObject *parent )
{
  switch( transport->type() ) {
    case Transport::EnumType::SMTP:
      return new SmtpJob( transport, parent );
    case Transport::EnumType::Sendmail:
      return new SendmailJob( transport, parent );
    case Transport::EnumType::Akonadi:
      return new AkonadiJob( transport, parent );
  }
  Q_ASSERT( false );
  return 0;
}

TransportConfigWidget *TransportTypeInfo::configWidgetForTransport( Transport *transport, QWidget *parent )
{
  switch( transport->type() ) {
    case Transport::EnumType::SMTP:
      return new SMTPConfigWidget( transport, parent );
    case Transport::EnumType::Sendmail:
      return new SendmailConfigWidget( transport, parent );
    case Transport::EnumType::Akonadi:
      return new AkonadiConfigWidget( transport, parent );
  }
  Q_ASSERT( false );
  return 0;
}

