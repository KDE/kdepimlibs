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

#ifndef AKONADI_SUBSCRIPTIONDIALOG_P_H
#define AKONADI_SUBSCRIPTIONDIALOG_P_H

#include "akonadi_export.h"

#include <kdialog.h>

namespace Akonadi {

/**
 * @internal
 *
 * Local subsription dialog.
 */
class AKONADI_EXPORT SubscriptionDialog : public KDialog
{
  Q_OBJECT
  public:
    /**
     * Creates a new subscription dialog.
     *
     * @param parent The parent widget.
     */
    explicit SubscriptionDialog( QWidget *parent = 0 );

    /**
     * Creates a new subscription dialog.
     *
     * @param parent The parent widget.
     * @param mimetypes The specific mimetypes
     * @since 4.6
     */
    explicit SubscriptionDialog( const QStringList &mimetypes, QWidget *parent = 0 );

    /**
     * Destroys the subscription dialog.
     *
     * @note Don't call the destructor manually, the dialog will
     *       be destructed automatically as soon as all changes
     *       are written back to the server.
     */
    ~SubscriptionDialog();

    /**
     * @since 4.9
     */
    void showHiddenCollection(bool showHidden);

  private:
    class Private;
    Private* const d;

    void init( const QStringList &mimetypes );

    Q_PRIVATE_SLOT( d, void done() )
    Q_PRIVATE_SLOT( d, void subscriptionResult( KJob* job ) )
    Q_PRIVATE_SLOT( d, void modelLoaded() )
    Q_PRIVATE_SLOT( d, void slotSetPattern(const QString &) )
    Q_PRIVATE_SLOT( d, void slotSetIncludeCheckedOnly(bool) )
    Q_PRIVATE_SLOT( d, void slotUnSubscribe())
    Q_PRIVATE_SLOT( d, void slotSubscribe())
};

}

#endif
