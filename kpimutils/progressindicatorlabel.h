/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef PROGRESSINDICATORLABEL_H
#define PROGRESSINDICATORLABEL_H

#include "kpimutils_export.h"

#include <QWidget>
namespace KPIMUtils {
class ProgressIndicatorLabelPrivate;
class KPIMUTILS_EXPORT ProgressIndicatorLabel : public QWidget
{
    Q_OBJECT
public:
    /**
     * @since 4.12
     */
    explicit ProgressIndicatorLabel(const QString &labelStr, QWidget *parent = 0);
    ~ProgressIndicatorLabel();

    void setActiveLabel(const QString &label);

public Q_SLOTS:
    void start();
    void stop();

private:
    friend class ProgressIndicatorLabelPrivate;
    ProgressIndicatorLabelPrivate * const d;
};
}

#endif // PROGRESSINDICATORLABEL_H
