/*
 * Copyright 2008 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef LIBS_PLASMA_EXTENDER_P_H
#define LIBS_PLASMA_EXTENDER_P_H

#include <QString>
#include <QList>
#include <QPointF>
#include "plasma/extenders/extender.h"

class QGraphicsGridLayout;
class QGraphicsLinearLayout;
class QGraphicsWidget;

namespace Plasma
{

class Applet;
class Extender;
class ExtenderGroup;
class ExtenderItem;
class Label;
class Svg;

class Spacer : public QGraphicsWidget
{
public:
    Spacer(QGraphicsItem *parent);
    ~Spacer();

    void setMargins(qreal left, qreal top, qreal right, qreal bottom);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0);


private:
    qreal m_left;
    qreal m_top;
    qreal m_right;
    qreal m_bottom;
};

class ExtenderPrivate
{
    public:
        ExtenderPrivate(Applet *applet, Extender *q);
        ~ExtenderPrivate();

        void addExtenderItem(ExtenderItem *item, const QPointF &pos = QPointF(-1, -1));
        void extenderItemDestroyed(ExtenderItem *item);
        void removeExtenderItem(ExtenderItem *item);
        int insertIndexFromPos(const QPointF &pos) const;
        void loadExtenderItems();
        void updateBorders();
        void updateEmptyExtenderLabel();
        ExtenderGroup *findGroup(const QString &name) const;

        Extender *q;

        QWeakPointer<Applet> applet;
        QGraphicsLinearLayout *layout;
        FrameSvg *background;

        int currentSpacerIndex;
        Spacer *spacerWidget;

        QString emptyExtenderMessage;
        Label *emptyExtenderLabel;

        QList<ExtenderItem*> attachedExtenderItems;

        Extender::Appearance appearance;

        static QGraphicsGridLayout *s_popupLayout;

        bool destroying;
};

} // namespace Plasma

#endif // LIBS_PLASMA_EXTENDER_P_H
