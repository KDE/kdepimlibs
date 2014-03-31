/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>
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

#include "queuer.h"

#include <QDateTime>
#include <QTimeEdit>
#include <QPointer>
#include <QPushButton>

#include <KApplication>
#include <KCmdLineArgs>
#include <KDateTimeWidget>
#include <KDialog>
#include <KLineEdit>
#include <KLocale>
#include <KLocalizedString>
#include <KTextEdit>

#include <akonadi/control.h>

#include <mailtransport/dispatchmodeattribute.h>
#include <mailtransport/messagequeuejob.h>
#include <mailtransport/transportmanager.h>
#include <mailtransport/transport.h>

using namespace KMime;
using namespace MailTransport;

MessageQueuer::MessageQueuer()
{
  if ( !Akonadi::Control::start() ) {
    qFatal() << "Could not start Akonadi server.";
  }

  mComboBox = new TransportComboBox( this );
  mComboBox->setEditable( true );
  mSenderEdit = new KLineEdit( this );
  mSenderEdit->setClickMessage( "Sender" );
  mToEdit = new KLineEdit( this );
  mToEdit->setText( "idanoka@gmail.com" );
  mToEdit->setClickMessage( "To" );
  mCcEdit = new KLineEdit( this );
  mCcEdit->setClickMessage( "Cc" );
  mBccEdit = new KLineEdit( this );
  mBccEdit->setClickMessage( "Bcc" );
  mMailEdit = new KTextEdit( this );
  mMailEdit->setText( "test from queuer!" );
  mMailEdit->setAcceptRichText( false );
  mMailEdit->setLineWrapMode( QTextEdit::NoWrap );
  QPushButton *b = new QPushButton( "&Send Now", this );
  connect( b, SIGNAL(clicked(bool)), SLOT(sendNowClicked()) );
  b = new QPushButton( "Send &Queued", this );
  connect( b, SIGNAL(clicked(bool)), SLOT(sendQueuedClicked()) );
  b = new QPushButton( "Send on &Date...", this );
  connect( b, SIGNAL(clicked(bool)), SLOT(sendOnDateClicked()) );
}

void MessageQueuer::sendNowClicked()
{
  MessageQueueJob *qjob = createQueueJob();
  qDebug() << "DispatchMode default (Automatic).";
  qjob->start();
}

void MessageQueuer::sendQueuedClicked()
{
  MessageQueueJob *qjob = createQueueJob();
  qDebug() << "DispatchMode Manual.";
  qjob->dispatchModeAttribute().setDispatchMode( DispatchModeAttribute::Manual );
  qjob->start();
}

void MessageQueuer::sendOnDateClicked()
{
  QPointer<KDialog> dialog = new KDialog( this );
  KDateTimeWidget *dt = new KDateTimeWidget( dialog );
  dt->setDateTime( QDateTime::currentDateTime() );
  // HACK:
  QTimeEdit *te = dt->findChild<QTimeEdit*>();
  Q_ASSERT( te );
  te->setDisplayFormat( "hh:mm:ss" );
  dialog->setMainWidget( dt );
  dialog->enableButtonCancel( false );
  dialog->exec();
  if ( !dialog ) {
    return;
  }
  qDebug() << "DispatchMode AfterDueDate" << dt->dateTime();
  MessageQueueJob *qjob = createQueueJob();
  qjob->dispatchModeAttribute().setDispatchMode( DispatchModeAttribute::Automatic );
  qjob->dispatchModeAttribute().setSendAfter( dt->dateTime() );
  qjob->start();
  delete dialog;
}

MessageQueueJob *MessageQueuer::createQueueJob()
{
  Message::Ptr msg = Message::Ptr( new Message );
  // No headers; need a '\n' to separate headers from body.
  // TODO: use real headers
  msg->setContent( QByteArray( "\n" ) + mMailEdit->document()->toPlainText().toLatin1() );
  qDebug() << "msg:" << msg->encodedContent( true );

  MessageQueueJob *job = new MessageQueueJob();
  job->setMessage( msg );
  job->transportAttribute().setTransportId( mComboBox->currentTransportId() );
  // default dispatch mode
  // default sent-mail collection
  job->addressAttribute().setFrom( mSenderEdit->text() );
  job->addressAttribute().setTo( mToEdit->text().isEmpty() ?
                                 QStringList() : mToEdit->text().split( ',' ) );
  job->addressAttribute().setCc( mCcEdit->text().isEmpty() ?
                                 QStringList() : mCcEdit->text().split( ',' ) );
  job->addressAttribute().setBcc( mBccEdit->text().isEmpty() ?
                                  QStringList() : mBccEdit->text().split( ',' ) );

  connect( job, SIGNAL(result(KJob*)),
           SLOT(jobResult(KJob*)) );
  connect( job, SIGNAL(percent(KJob*,ulong)),
           SLOT(jobPercent(KJob*,ulong)) );
  connect( job, SIGNAL(infoMessage(KJob*,QString,QString)),
           SLOT(jobInfoMessage(KJob*,QString,QString)) );

  return job;
}

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "messagequeuer", 0,
                      ki18n( "messagequeuer" ), "0",
                      ki18n( "MessageQueuerJob Demo" ) );
  KApplication app;
  MessageQueuer *t = new MessageQueuer();
  t->show();
  app.exec();
  delete t;
}

void MessageQueuer::jobResult( KJob *job )
{
  if ( job->error() ) {
    qDebug() << "job error:" << job->errorText();
  } else {
    qDebug() << "job success.";
  }
}

void MessageQueuer::jobPercent( KJob *job, unsigned long percent )
{
  Q_UNUSED( job );
  qDebug() << percent << "%";
}

void MessageQueuer::jobInfoMessage( KJob *job, const QString &info, const QString &info2 )
{
  Q_UNUSED( job );
  qDebug() << info;
  qDebug() << info2;
}

