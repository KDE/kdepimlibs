/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#ifndef AKONADI_WAITINGOVERLAY_P_H
#define AKONADI_WAITINGOVERLAY_P_H

#include <QtCore/QPointer>
#include <QWidget>

class KJob;

/**
 * @internal
 * Overlay widget to block widgets while a job is running.
 */
class WaitingOverlay : public QWidget
{
public:
    /**
     * Create an overlay widget on @p baseWidget for @p job.
     * @param baseWidget must not be null.
     * @param parent must not be equal to baseWidget
     */
    explicit WaitingOverlay(KJob *job, QWidget *baseWidget, QWidget *parent = 0);
    ~WaitingOverlay();

protected:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

private:
    void reposition();

private:
    QPointer<QWidget> mBaseWidget;
    bool mPreviousState;
};

#endif
