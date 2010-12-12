/*
    Copyright (c) 2008 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 David Jarvie <djarvie@kde.org>

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

#ifndef AKONADI_COLLECTIONPROPERTIESDIALOG_H
#define AKONADI_COLLECTIONPROPERTIESDIALOG_H

#include "akonadi_export.h"

#include <akonadi/collectionpropertiespage.h>

#include <kdialog.h>

namespace Akonadi {

class Collection;

/**
 * @short A generic and extensible dialog for collection properties.
 *
 * This dialog allows you to show or modify the properties of a collection.
 *
 * @code
 *
 * Akonadi::Collection collection = ...
 *
 * CollectionPropertiesDialog dlg( collection, this );
 * dlg.exec();
 *
 * @endcode
 *
 * It can be extended by custom pages, which contains gui elements for custom
 * properties.
 *
 * @see Akonadi::CollectionPropertiesPage
 *
 * @author Volker Krause <vkrause@kde.org>
 */
class AKONADI_EXPORT CollectionPropertiesDialog : public KDialog
{
  Q_OBJECT
  public:
    /**
     * Enumerates the default pages which can be displayed.
     *
     * @since 4.6
     */
    enum DefaultPages
    {
      NoPages     = 0,      ///< No default pages
      GeneralPage = 0x01,   ///< General properties page
      CachePage   = 0x02,   ///< Cache properties page
      AllPages    = 0xFF    ///< All default pages
    };
      
    /**
     * Creates a new collection properties dialog.
     *
     * @param collection The collection which properties should be shown.
     * @param parent The parent widget.
     */
    explicit CollectionPropertiesDialog( const Collection &collection, QWidget *parent = 0 );

    /**
     * Creates a new collection properties dialog.
     *
     * This constructor allows to specify the subset of registered pages that will
     * be shown as well as their order. The pages have to set an objectName in their
     * constructor to make it work. If an empty list is passed, all registered pages
     * will be loaded.
     *
     * @param collection The collection which properties should be shown.
     * @param pages The object names of the pages that shall be loaded.
     * @param parent The parent widget.
     *
     * @since 4.6
     */
    CollectionPropertiesDialog( const Collection &collection, const QStringList &pages, QWidget *parent = 0 );

    /**
     * Destroys the collection properties dialog.
     *
     * @note Never call manually, the dialog is deleted automatically once all changes
     *       are written back to the Akonadi storage.
     */
    ~CollectionPropertiesDialog();

    /**
     * Register custom pages for the collection properties dialog.
     *
     * @param factory The properties page factory that provides the custom page.
     *
     * @see Akonadi::CollectionPropertiesPageFactory
     */
    static void registerPage( CollectionPropertiesPageFactory *factory );

    /**
     * Sets whether to @p use all default pages or not.
     *
     * @since 4.4
     * @deprecated Use useDefaultPages() instead.
     */
    static void useDefaultPage( bool use );

    /**
     * Sets which default pages to display. By default, all default pages
     * are displayed.
     *
     * @param defaultPages OR of the pages to display.
     *
     * @since 4.6
     */
    static void useDefaultPages( DefaultPages defaultPages = AllPages );

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void save() )
    Q_PRIVATE_SLOT( d, void saveResult( KJob* ) )
    //@endcond
};

}

#endif
