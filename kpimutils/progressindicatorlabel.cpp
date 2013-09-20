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

#include "progressindicatorlabel.h"
#include "progressindicatorwidget.h"

#include <QHBoxLayout>

namespace KPIMUtils {
class ProgressIndicatorLabelPrivate
{
public:
    ProgressIndicatorLabelPrivate(ProgressIndicatorLabel *qq)
        : q(qq)
    {
        QHBoxLayout *lay = new QHBoxLayout;
        lay->setMargin(0);
        q->setLayout(lay);
        indicator = new ProgressIndicatorWidget;
        lay->addWidget(indicator);
        label = new QLabel;
        lay->addWidget(label);
    }

    ~ProgressIndicatorLabelPrivate()
    {
    }

    void start()
    {
        indicator->start();
        label->setText(labelStr);
    }

    void stop()
    {
        indicator->stop();
        label->clear();
    }

    QLabel *label;
    QString labelStr;
    ProgressIndicatorWidget *indicator;
    ProgressIndicatorLabel *q;
};

ProgressIndicatorLabel::ProgressIndicatorLabel(QWidget *parent)
    : QWidget(parent),
      d(new ProgressIndicatorLabelPrivate(this))
{
}

ProgressIndicatorLabel::~ProgressIndicatorLabel()
{
    delete d;
}

void ProgressIndicatorLabel::start()
{
    d->start();
}

void ProgressIndicatorLabel::stop()
{
    d->stop();
}

void ProgressIndicatorLabel::setLabel(const QString &label)
{
    d->labelStr = label;
}

}

#include "progressindicatorlabel.moc"
