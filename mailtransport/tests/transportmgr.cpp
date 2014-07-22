/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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

#include "transportmgr.h"

#include <mailtransport/transportconfigdialog.h>
#include <mailtransport/transportmanager.h>
#include <mailtransport/transportmanagementwidget.h>
#include <mailtransport/transportjob.h>
#include <mailtransport/transport.h>

#include <KApplication>
#include <KCmdLineArgs>
#include <KLineEdit>
#include <KLocale>
#include <KLocalizedString>
#include <KDebug>
#include <KTextEdit>

#include <QPushButton>

using namespace MailTransport;

TransportMgr::TransportMgr() :
    mCurrentJob( 0 )
{
  new TransportManagementWidget( this );
  mComboBox = new TransportComboBox( this );
  mComboBox->setEditable( true );
  QPushButton *b = new QPushButton( "&Edit", this );
  connect( b, SIGNAL(clicked(bool)), SLOT(editBtnClicked()) );
  b = new QPushButton( "&Remove all transports", this );
  connect( b, SIGNAL(clicked(bool)), SLOT(removeAllBtnClicked()) );
  mSenderEdit = new KLineEdit( this );
  mSenderEdit->setClickMessage( "Sender" );
  mToEdit = new KLineEdit( this );
  mToEdit->setClickMessage( "To" );
  mCcEdit = new KLineEdit( this );
  mCcEdit->setClickMessage( "Cc" );
  mBccEdit = new KLineEdit( this );
  mBccEdit->setClickMessage( "Bcc" );
  mMailEdit = new KTextEdit( this );
  mMailEdit->setAcceptRichText( false );
  mMailEdit->setLineWrapMode( QTextEdit::NoWrap );
  b = new QPushButton( "&Send", this );
  connect( b, SIGNAL(clicked(bool)), SLOT(sendBtnClicked()) );
  b = new QPushButton( "&Cancel", this );
  connect( b, SIGNAL(clicked(bool)), SLOT(cancelBtnClicked()) );
}

void TransportMgr::removeAllBtnClicked()
{
    MailTransport::TransportManager *manager =  MailTransport::TransportManager::self();
    QList<Transport *> transports = manager->transports();
    for ( int i=0; i < transports.count(); i++ ) {
        MailTransport::Transport *transport = transports.at( i );
        kDebug() << transport->host();
        manager->removeTransport( transport->id() );
    }
}

void TransportMgr::editBtnClicked()
{
  // NOTE: Using deprecated TransportConfigDialog here for testing purposes.
  // The TransportManagementWidget uses the non-deprecated method instead.
  const int index = mComboBox->currentTransportId();
  if (index < 0)
      return;
  TransportConfigDialog *t =
    new TransportConfigDialog(
      TransportManager::self()->transportById( index ), this );
  t->exec();
  delete t;
}

void TransportMgr::sendBtnClicked()
{
  TransportJob *job;
  job = TransportManager::self()->createTransportJob( mComboBox->currentTransportId() );
  if ( !job ) {
    kDebug() << "Invalid transport!";
    return;
  }
  job->setSender( mSenderEdit->text() );
  job->setTo( mToEdit->text().isEmpty() ? QStringList() : mToEdit->text().split( ',' ) );
  job->setCc( mCcEdit->text().isEmpty() ? QStringList() : mCcEdit->text().split( ',' ) );
  job->setBcc( mBccEdit->text().isEmpty() ? QStringList() : mBccEdit->text().split( ',' ) );
  job->setData( mMailEdit->document()->toPlainText().toLatin1() );
  connect( job, SIGNAL(result(KJob*)),
           SLOT(jobResult(KJob*)) );
  connect( job, SIGNAL(percent(KJob*,ulong)),
           SLOT(jobPercent(KJob*,ulong)) );
  connect( job, SIGNAL(infoMessage(KJob*,QString,QString)),
           SLOT(jobInfoMessage(KJob*,QString,QString)) );
  mCurrentJob = job;
  TransportManager::self()->schedule( job );
}

void TransportMgr::cancelBtnClicked()
{
  if ( mCurrentJob ) {
    kDebug() << "kill success:" << mCurrentJob->kill();
  }
  mCurrentJob = 0;
}

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "transportmgr", 0,
                      ki18n( "transportmgr" ), "0",
                      ki18n( "Mail Transport Manager Demo" ) );
  KApplication app;
  TransportMgr *t = new TransportMgr();
  t->show();
  app.exec();
  delete t;
}

void TransportMgr::jobResult( KJob *job )
{
  kDebug() << job->error() << job->errorText();
  mCurrentJob = 0;
}

void TransportMgr::jobPercent( KJob *job, unsigned long percent )
{
  Q_UNUSED( job );
  kDebug() << percent << "%";
}

void TransportMgr::jobInfoMessage( KJob *job, const QString &info, const QString &info2 )
{
  Q_UNUSED( job );
  kDebug() << info;
  kDebug() << info2;
}

