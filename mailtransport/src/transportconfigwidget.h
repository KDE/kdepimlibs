/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

    Based on MailTransport code by:
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

    Based on KMail code by:
    Copyright (c) 2001-2002 Michael Haeckel <haeckel@kde.org>

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

#ifndef MAILTRANSPORT_TRANSPORTCONFIGWIDGET_H
#define MAILTRANSPORT_TRANSPORTCONFIGWIDGET_H

#include <QWidget>

class KConfigDialogManager;

namespace MailTransport
{

class Transport;
class TransportConfigWidgetPrivate;

/**
  @internal

  Abstract configuration widget for a mail transport.  It makes sure that
  the configured transport has a unique name, and takes care of writing its
  settings to the config file.  If it is a new transport, the caller must
  still call TransportManager::addTransport() to register the transport.

  Concrete configuration is done in subclasses SMTPConfigWidget and
  SendmailConfigWidget.  Akonadi-type transports are not configured by
  MailTransport directly, instead the configure() method of their agent
  instance is called.

  To configure a transport from applications, use
  TransportManager::configureTransport().  You still need to call
  TransportManager::addTransport() if this is a new transport, not registered
  with TransportManager.

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class TransportConfigWidget : public QWidget
{
    Q_OBJECT

public:
    /**
      Creates a new mail transport configuration widget for the given
      Transport object.
      @param transport The Transport object to configure.
      @param parent The parent widget.
    */
    explicit TransportConfigWidget(Transport *transport, QWidget *parent = Q_NULLPTR);

    /**
      Destroys the widget.
    */
    virtual ~TransportConfigWidget();

    /**
      @internal
      Get the KConfigDialogManager for this widget.
    */
    KConfigDialogManager *configManager() const;

public Q_SLOTS:
    /**
      Saves the transport's settings.

      The base implementation writes the settings to the config file and makes
      sure the transport has a unique name.  Reimplement in derived classes to
      save your custom settings, and call the base implementation.
    */
    // TODO: do we also want to check for invalid settings?
    virtual void apply();

protected:
    TransportConfigWidgetPrivate *const d_ptr;
    TransportConfigWidget(TransportConfigWidgetPrivate &dd,
                          Transport *transport, QWidget *parent);

private:
    Q_DECLARE_PRIVATE(TransportConfigWidget)

    void init(Transport *transport);

};

} // namespace MailTransport

#endif // MAILTRANSPORT_TRANSPORTCONFIGWIDGET_H
