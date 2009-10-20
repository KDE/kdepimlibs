/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef MAILTRANSPORT_TRANSPORTCONFIGDIALOG_H
#define MAILTRANSPORT_TRANSPORTCONFIGDIALOG_H

#include <mailtransport/mailtransport_export.h>

#include <KDE/KDialog>

namespace MailTransport {

class Transport;

/**
 * @short Configuration dialog for a mail transport.
 *
 * @deprecated Use TransportManager::configureTransport() instead.
 */
class MAILTRANSPORT_EXPORT_DEPRECATED TransportConfigDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * Creates a new mail transport configuration dialog for the given
     * Transport object.
     * The config dialog does not delete @p transport, you have to delete it
     * yourself.
     *
     * Note that this class only works for transports that are handled directly
     * by MailTransport, i.e. SMTP and Sendmail.  This class cannot be used to
     * configure an Akonadi transport.
     *
     * @param transport The Transport object to configure. This must be a deep
     * copy of a Transport object or a newly created one, which hasn't been
     * added to the TransportManager yet.
     * @param parent The parent widget.
     */
    explicit TransportConfigDialog( Transport *transport, QWidget *parent = 0 );

    /**
     * Destroys the transport config dialog.
     */
    virtual ~TransportConfigDialog();

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void okClicked() )
    //@endcond
};

} // namespace MailTransport

#endif // MAILTRANSPORT_TRANSPORTCONFIGDIALOG_H
