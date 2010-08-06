/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.
          (c) 2001 Michael Goffioul <kdeprint@swing.be>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __KPANELMENU_H__
#define __KPANELMENU_H__


#include <kde3support_export.h>
#include <kmenu.h>
#include <kgenericfactory.h>

/**
 * @short Base class to build dynamically loaded menu entries for the K-menu, or the panel.
 *
 * This class allows to build menu entries that will be dynamically added either to
 * the K-menu, or to the panel as a normal button. These dynamic menus are located
 * in shared libraries that will be loaded at runtime by Kicker (the %KDE panel).
 *
 * To build such a menu, you have to inherit this class and implement the pure virtual
 * functions #initialize() and slotExec(). You also have to provide a factory
 * object in your library, see KLibFactory. This factory is only used to construct
 * the menu object.
 *
 * Finally, you also have to provide a desktop file describing your dynamic menu. The
 * relevant entries are: Name, Comment, Icon and X-KDE-Library (which contains the
 * library name without any extension). This desktop file has to be installed in
 * $KDEDIR/share/apps/kicker/menuext/.
 *
 * @author The kicker maintainers, Michael Goffioul <kdeprint@swing.be>
 */
class KDE3SUPPORT_EXPORT K3PanelMenu : public KMenu
{
    Q_OBJECT

public:
    /**
     * Construct a K3PanelMenu object. This is the normal constructor to use when
     * building extrernal menu entries.
     */
    K3PanelMenu(QWidget *parent);
    /**
     * Constructor used internally by Kicker. You don't really want to use it.
     * @param startDir a directory to associate with this menu
     * @param parent parent object
     * @param name name of the object
     * @see path(), setPath()
     */
    K3PanelMenu(const QString &startDir, QWidget *parent);
    /**
     * Destructor.
     */
    virtual ~K3PanelMenu();

    /**
     * Get the directory path associated with this menu, or QString() if
     * there's no such associated path.
     * @return the associated directory path
     * @see setPath()
     */
    const QString& path() const;
    /**
     * Set a directory path to be associated with this menu.
     * @param p the directory path
     * @see path()
     */
    void setPath(const QString &p);
    /**
     * Tell if the menu has been initialized, that is it already contains items.
     * This is useful when you need to know if you have to clear the menu, or to
     * fill it.
     * @return the initial state
     * @see setInitialized(), initialize()
     */
    bool initialized() const;
    /**
     * Set the initial state. Set it to true when you menu is filled with the items
     * you want.
     * @param on the initial state
     * @see initialized(), initialize()
     */
    void setInitialized(bool on);

    /**
     * Disable the automatic clearing of the menu. Kicker uses a cache system for
     * its menus. After a specific configurable delay, the menu will be cleared.
     * Use this function if you want to disable kicker's cache system, and avoid
     * the clearing of your menu.
     */
    void disableAutoClear();

public Q_SLOTS:
    /**
     * Reinitialize the menu: the menu is first cleared, the initial state is set
     * to false, and finally #initialize() is called. Use this if you want to
     * refill your menu immediately.
     */
    void reinitialize();
    /**
     * Deinitialize the menu: the menu is cleared and the initialized state is set to
     * false. #initialize() is NOT called. It will be called before the menu is
     * next shown, however. Use this slot if you want a delayed reinitialization.
     */
    void deinitialize();

protected Q_SLOTS:
    /**
     * This slot is called just before the menu is shown. This allows your menu
     * to update itself if needed. However you should instead re-implement
     * #initialize to provide this feature. This function is responsible for
     * the cache system handling, so if you re-implement it, you should call
     * the base function also. Calls #initialize().
     * @see disableAutoClear()
     */
    virtual void slotAboutToShow();
    /**
     * This is slot is called when an item from the menu has been selected. Your
     * applet is then supposed to perform some action. You must re-implement this
     * function.
     * @param id the ID associated with the selected item
     */
    virtual void slotExec(int id) = 0;
    /**
     * This slots is called to initialize the menu. It is called automatically by
     * slotAboutToShow(). By re-implementing this functions, you can reconstruct
     * the menu before it is being shown. At the end of this function, you should
     * call setInitialize() with true to tell the system that the menu is OK.
     * You applet must re-implement this function.
     * @see slotAboutToShow(), initialized(), setInitialized()
     */
    virtual void initialize() = 0;
    /**
     * Clears the menu, and update the initial state accordingly.
     * @see initialized()
     */
    void slotClear();

protected:
    /**
     * Re-implemented for internal reasons.
     */
    virtual void hideEvent(QHideEvent *ev);
    /**
     * For internal use only. Used by constructors.
     */
    void init(const QString& path = QString());

private:
    void internalInitialize();
    class Private;
    Private *d;
};

#define K_EXPORT_KICKER_MENUEXT( libname, classname )                       \
    K_EXPORT_COMPONENT_FACTORY(                                             \
        kickermenu_##libname,                                               \
        KGenericFactory<classname>("libkickermenu_" #libname) )

#endif
