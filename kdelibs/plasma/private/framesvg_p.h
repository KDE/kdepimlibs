/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_FRAMESVG_P_H
#define PLASMA_FRAMESVG_P_H

#include <QHash>

namespace Plasma
{
class FrameData
{
public:
    FrameData()
      : enabledBorders(FrameSvg::AllBorders),
        frameSize(-1,-1),
        topHeight(0),
        leftWidth(0),
        rightWidth(0),
        bottomHeight(0),
        topMargin(0),
        leftMargin(0),
        rightMargin(0),
        bottomMargin(0),
        noBorderPadding(false),
        stretchBorders(false),
        tileCenter(false)
    {
    }

    FrameData(const FrameData &other)
      : enabledBorders(other.enabledBorders),
        frameSize(other.frameSize),
        topHeight(0),
        leftWidth(0),
        rightWidth(0),
        bottomHeight(0),
        topMargin(0),
        leftMargin(0),
        rightMargin(0),
        bottomMargin(0),
        noBorderPadding(false),
        stretchBorders(false),
        tileCenter(false),
        composeOverBorder(false)
    {
    }

    ~FrameData()
    {
    }

    FrameSvg::EnabledBorders enabledBorders;
    QPixmap cachedBackground;
    QRegion cachedMask;
    QSize frameSize;

    //measures
    int topHeight;
    int leftWidth;
    int rightWidth;
    int bottomHeight;

    //margins, are equal to the measures by default
    int topMargin;
    int leftMargin;
    int rightMargin;
    int bottomMargin;

    //size of the svg where the size of the "center"
    //element is contentWidth x contentHeight
    bool noBorderPadding : 1;
    bool stretchBorders : 1;
    bool tileCenter : 1;
    bool composeOverBorder : 1;
};

class FrameSvgPrivate
{
public:
    FrameSvgPrivate(FrameSvg *psvg)
      : q(psvg),
        cacheAll(false),
        overlayPos(0,0)
    {
    }

    ~FrameSvgPrivate()
    {
        qDeleteAll(frames);
        frames.clear();
    }

    QPixmap alphaMask();
    void generateBackground(FrameData *frame);
    void generateFrameBackground(FrameData *frame);
    QString cacheId(FrameData *frame, const QString &prefixToUse) const;
    void cacheFrame(const QString &prefixToSave, const QPixmap &background, const QPixmap &overlay);
    void updateSizes() const;
    void updateNeeded();
    void updateAndSignalSizes();
    QSizeF frameSize(FrameData *frame) const;

    Location location;
    QString prefix;

    FrameSvg *q;

    bool cacheAll : 1;
    QPoint overlayPos;

    QHash<QString, FrameData*> frames;
};

}

#endif
