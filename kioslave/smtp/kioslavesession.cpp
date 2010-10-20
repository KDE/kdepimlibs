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

#include "kioslavesession.h"

KioSMTP::KioSlaveSession::KioSlaveSession(SMTPProtocol* protocol): m_protocol( protocol )
{
}

void KioSMTP::KioSlaveSession::error(int id, const QString& msg)
{
  m_protocol->error( id, msg );
}

bool KioSMTP::KioSlaveSession::haveCapability(const char* cap) const
{
  return m_protocol->haveCapability( cap );
}

bool KioSMTP::KioSlaveSession::isAutoSsl() const
{
  return m_protocol->isAutoSsl();
}

bool KioSMTP::KioSlaveSession::isUsingSsl() const
{
  return m_protocol->isUsingSsl();
}

void KioSMTP::KioSlaveSession::messageBox(KIO::SlaveBase::MessageBoxType id, const QString& msg, const QString& caption )
{
  m_protocol->messageBox( id, msg, caption );
}

bool KioSMTP::KioSlaveSession::openPasswordDialog(KIO::AuthInfo& authInfo)
{
  return m_protocol->openPasswordDialog( authInfo );
}

QString KioSMTP::KioSlaveSession::metaData(const QString& key) const
{
  return m_protocol->metaData( key );
}

void KioSMTP::KioSlaveSession::parseFeatures(const KioSMTP::Response &response )
{
  return m_protocol->parseFeatures( response );
}

void KioSMTP::KioSlaveSession::dataReq()
{
  m_protocol->dataReq();
}

int KioSMTP::KioSlaveSession::readData(QByteArray& ba)
{
  return m_protocol->readData( ba );
}

bool KioSMTP::KioSlaveSession::startSsl()
{
  return m_protocol->startSsl();
}



