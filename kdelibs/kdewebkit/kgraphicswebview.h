/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2007 Trolltech ASA
 * Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2008 Laurent Montel <montel@kde.org>
 * Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
 * Copyright (C) 2009 Dawit Alemayehu <adawit @ kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef KGRAPHICSWEBVIEW_H
#define KGRAPHICSWEBVIEW_H

#include <kdewebkit_export.h>

#include <QtWebKit/QGraphicsWebView>

class KUrl;
template<class T> class KWebViewPrivate;

/**
 * @short A re-implementation of QGraphicsWebView that provides KDE integration.
 *
 * This is a drop-in replacement for QGraphicsWebView that provides full KDE
 * integration through the use of @ref KWebPage. It also provides signals that
 * capture middle/shift/ctrl mouse clicks on links and url pasting from the
 * selection clipboard.
 *
 * @author Urs Wolfer <uwolfer @ kde.org>
 * @author Dawit Alemayehu <adawit @ kde.org>
 *
 * @since 4.4
 */
class KDEWEBKIT_EXPORT KGraphicsWebView : public QGraphicsWebView
{
    Q_OBJECT

public:
    /**
     * Constructs a KGraphicsWebView object with parent @p parent.
     *
     * The @p createCustomPage flag allows you to prevent the creation of a
     * custom KWebPage object that is used to provide KDE integration. If you
     * are going to use your own implementation of KWebPage, you should set
     * this flag to false to avoid unnecessary creation and deletion of objects.
     *
     * @param parent            the parent object.
     * @param createCustomPage  if true, the default, creates a custom KWebPage object.
     */
    explicit KGraphicsWebView(QGraphicsItem *parent = 0, bool createCustomPage = true);

    /**
     * Destroys the KWebView.
     */
    ~KGraphicsWebView();

    /**
     * Returns true if access to remote content is allowed.
     *
     * By default access to remote content is allowed.
     *
     * @see setAllowExternalContent()
     * @see KWebPage::isExternalContentAllowed()
     */
    bool isExternalContentAllowed() const;

    /**
     * Set @p allow to false if you want to prevent access to remote content.
     *
     * If this function is set to false only resources on the local system
     * can be accessed through this class. By default fetching external content
     * is allowed.
     *
     * @see isExternalContentAllowed()
     * @see KWebPage::setAllowExternalContent(bool)
     */
    void setAllowExternalContent(bool allow);

Q_SIGNALS:
    /**
     * This signal is emitted when a url from the selection clipboard is pasted
     * on this view.
     *
     * @param url   the url of the clicked link.
     */
    void selectionClipboardUrlPasted(const KUrl &url);

    /**
     * This signal is emitted when a link is shift clicked with the left mouse
     * button.
     *
     * @param url   the url of the clicked link.
     */
    void linkShiftClicked(const KUrl &url);

    /**
     * This signal is emitted when a link is either clicked with middle mouse
     * button or ctrl-clicked with the left moust button.
     *
     * @param url   the url of the clicked link.
     */
    void linkMiddleOrCtrlClicked(const KUrl &url);

protected:
    /**
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see QWidget::wheelEvent
     * @internal
     */
    void wheelEvent(QGraphicsSceneWheelEvent *event);

    /**
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see QWidget::mousePressEvent
     * @internal
     */
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

    /**
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see QWidget::mouseReleaseEvent
     * @internal
     */
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    friend class KWebViewPrivate<KGraphicsWebView>;
    KWebViewPrivate<KGraphicsWebView> * const d;
};

#endif // KWEBVIEW_H
