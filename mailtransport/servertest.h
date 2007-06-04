/* 
    Copyright (C) 2007 KovoKs <info@kovoks.nl>

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

#ifndef MAILTRANSPORT_SERVERTEST_H
#define MAILTRANSPORT_SERVERTEST_H

#include <mailtransport/mailtransport_export.h>
#include <mailtransport/transport.h>

#include <QtGui/QWidget>
#include <QtCore/QHash>

class QProgressBar;
  
namespace MailTransport 
{

class ServerTestPrivate;

/**
  * @class ServerTest
  * This class can be used to test certain server to see if they support stuff.
  * @author Tom Albers <tomalbers@kde.nl>
  */
class MAILTRANSPORT_EXPORT ServerTest : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString server READ server WRITE setServer )
    Q_PROPERTY( QString protocol READ protocol WRITE setProtocol )
    Q_PROPERTY( QProgressBar* progressBar READ progressBar WRITE setProgressBar )

  public:
    /**
      * Constructor
      * @param parent Parent Widget
      */
    ServerTest( QWidget* parent = 0 );

    /**
      * Destructor
      */
    ~ServerTest();

    /**
      * Set the server to test.
      */
    void setServer( const QString& server );

    /**
      * Get the server to test.
      */
    QString server();

    /**
      * Makes @p pb the progressbar to use. This class will call show()
      * and hide() and will count down. It does not take ownership of
      * the progressbar.
      */
    void setProgressBar( QProgressBar* pb );

    /**
      * returns the used progressBar
      */
    QProgressBar* progressBar();

    /**
      * Set @p proto the protocol to test, currently supported are
      * "smtp" and "imap". This will be an enum soon.
      */
    void setProtocol( const QString& protocol );

    /**
      * returns the protocol
      */
    QString protocol();

    /**
      * Starts the test. Will emit finished() when done.
      */
    void start();

    /**
      * Get the protocols for the normal connections.. Call this only
      * after the finished() signals has been sent.
      * @return an enum of the type Transport::EnumAuthenticationType
      */
    QList< int > normalProtocols();

    /**
      * Get the protocols for the secure connections.. Call this only
      * after the finished() signals has been sent.
      * @return an enum of the type Transport::EnumAuthenticationType
      */
    QList< int > secureProtocols();

  Q_SIGNALS:
    /**
      * This will be emitted when the test is done. It will contain
      * the values from the enum EnumEncryption which are possible.
      */
    void finished( QList< int > );

  private:
    Q_DECLARE_PRIVATE(ServerTest)
    ServerTestPrivate *const d;

    Q_PRIVATE_SLOT(d, void slotNormalPossible() )
    Q_PRIVATE_SLOT(d, void slotSslPossible())
    Q_PRIVATE_SLOT(d, void slotReadNormal( const QString& text ))
    Q_PRIVATE_SLOT(d, void slotReadSecure( const QString& text ))
    Q_PRIVATE_SLOT(d, void slotNormalNotPossible())
    Q_PRIVATE_SLOT(d, void slotSslNotPossible())
    Q_PRIVATE_SLOT(d, void slotUpdateProgress())
};

}

#endif
