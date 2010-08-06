/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
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
 */

#ifndef _KNEWSTUFF3_KRATINGWIDGET_H_
#define _KNEWSTUFF3_KRATINGWIDGET_H_

#include <QtGui/QFrame>
#include <kdemacros.h>

namespace KNS3 {
    // This is a copy of the class from Nepomuk as long as Nepomuk is not a hard dependency in kdelibs
    // In addition it has the editable property

/**
 * \brief Displays a rating value as a row of pixmaps.
 *
 * The KRatingWidget displays a range of stars or other arbitrary
 * pixmaps and allows the user to select a certain number by mouse.
 *
 * \sa KRatingPainter
 *
 * \author Sebastian Trueg <trueg@kde.org>
 */
class RatingWidget : public QFrame
{
    Q_OBJECT

 public:
    /**
     * Creates a new rating widget.
     */
    RatingWidget( QWidget* parent = 0 );

    /**
     * Destructor
     */
    ~RatingWidget();

    /**
     * \return The current rating.
     */
    unsigned int rating() const;

    /**
     * \return the maximum possible rating.
     */
    int maxRating() const;

    /**
     * The alignment of the stars.
     *
     * \sa setAlignment
     */
    Qt::Alignment alignment() const;

    /**
     * The layout direction. If RTL the stars
     * representing the rating value will be drawn from the
     * right.
     *
     * \sa setLayoutDirection
     */
    Qt::LayoutDirection layoutDirection() const;

    /**
     * The spacing between the rating stars.
     *
     * \sa setSpacing
     */
    int spacing() const;

    QSize sizeHint() const;

    /**
     * If half steps are enabled one star equals to 2 rating
     * points and uneven rating values result in half-stars being
     * drawn.
     *
     * \sa setHalfStepsEnabled
     */
    bool halfStepsEnabled() const;

    /**
     * Set if the widget allows the user to set a rating, true by default.
     *
     * \sa isEditable
     */
    void setEditable(bool editable);

    /**
    * If the widget allows the user to set a rating.
    *
    * \sa setEditable
    */
    bool isEditable();

    /**
     * The icon used to draw a star. In case a custom pixmap has been set
     * this value is ignored.
     *
     * \sa setIcon, setCustomPixmap
     */
    QIcon icon() const;

 Q_SIGNALS:
    /**
     * Emitted if the rating is changed by user interaction (ie. mouse click).
     * A call to setRating does not trigger this signal.
     */
    void ratingChanged( unsigned int rating );
    void ratingChanged( int rating );

 public Q_SLOTS:
    /**
     * Set the current rating. Calling this method will NOT trigger the
     * ratingChanged signal.
     */
    void setRating( int rating );

    /**
     * \deprecated use setRating( int max )
     */
    KDE_DEPRECATED void setRating( unsigned int rating );

    /**
     * Set the maximum allowed rating value. The default is 10 which means
     * that a rating from 1 to 10 is selectable. If \a max is uneven steps
     * are automatically only allowed full.
     */
    void setMaxRating( int max );

    /**
     * \deprecated use setMaxRating( int max )
     */
    KDE_DEPRECATED void setMaxRating( unsigned int max );

    /**
     * If half steps are enabled (the default) then
     * one rating step corresponds to half a star.
     */
    void setHalfStepsEnabled( bool enabled );

    /**
     * \deprecated Use setHalfStepsEnabled
     */
    KDE_DEPRECATED void setOnlyPaintFullSteps( bool );

    /**
     * Set the spacing between the pixmaps. The default is 0.
     */
    void setSpacing( int );

    /**
     * The alignment of the stars in the drawing rect.
     * All alignment flags are supported.
     */
    void setAlignment( Qt::Alignment align );

    /**
     * LTR or RTL
     */
    void setLayoutDirection( Qt::LayoutDirection direction );

    /**
     * Set a custom icon. Defaults to "rating".
     */
    void setIcon( const QIcon& icon );

    /**
     * Set a custom pixmap.
     */
    void setCustomPixmap( const QPixmap& pixmap );

    /**
     * Set the pixap to be used to display a rating step.
     * By default the "rating" pixmap is loaded.
     *
     * \deprecated use setCustomPixmap
     */
    KDE_DEPRECATED void setPixmap( const QPixmap& );

    /**
     * Set the recommended size of the pixmaps. This is
     * only used for the sizeHint. The actual size is always
     * dependent on the size of the widget itself.
     */
    void setPixmapSize( int size );

 protected:
    void mousePressEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent* e );
    void leaveEvent( QEvent* e );
    void paintEvent( QPaintEvent* e );
    void resizeEvent( QResizeEvent* e );

 private:
    class Private;
    Private* const d;
};
}
#endif
