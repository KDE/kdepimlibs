/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#ifndef KIOSMTP_FAKESESSION_H
#define KIOSMTP_FAKESESSION_H

#include "smtpsessioninterface.h"

#include <QStringList>
#include <KIO/MetaData>

namespace KioSMTP {

class FakeSession : public SMTPSessionInterface {
  public:
    FakeSession() { clear(); }

    //
    // public members to control the API emulation below:
    //
    bool startTLSReturnCode;
    bool usesSSL;
    bool usesTLS;
    int lastErrorCode;
    QString lastErrorMessage;
    int lastMessageBoxCode;
    QString lastMessageBoxText;
    QByteArray nextData;
    int nextDataReturnCode;
    QStringList caps;
    KIO::MetaData metadata;

    void clear() {
      startTLSReturnCode = true;
      usesSSL = usesTLS = false;
      lastErrorCode = lastMessageBoxCode = 0;
      lastErrorMessage.clear();
      lastMessageBoxText.clear();
      nextData.resize( 0 );
      nextDataReturnCode = -1;
      caps.clear();
      metadata.clear();
    }

    //
    // emulated API:
    //
    bool startSsl() {
      return startTLSReturnCode;
    }
    bool isUsingSsl() const { return usesSSL; }
    bool isAutoSsl() const { return usesTLS; }
    bool haveCapability( const char * cap ) const { return caps.contains( cap ); }
    void error( int id, const QString & msg ) {
      lastErrorCode = id;
      lastErrorMessage = msg;
      qWarning() << id << msg;
    }
    void messageBox(KIO::SlaveBase::MessageBoxType id, const QString& msg, const QString& caption) {
      Q_UNUSED( caption );
      lastMessageBoxCode = id;
      lastMessageBoxText = msg;
    }
    bool openPasswordDialog( KIO::AuthInfo & ) { return true; }
    void dataReq() { /* noop */ }
    int readData( QByteArray & ba ) { ba = nextData; return nextDataReturnCode; }
    QString metaData( const QString & key ) const { return metadata[key]; }
};

}

#include "smtpsessioninterface.cpp"
#include "capabilities.cpp"

#endif
