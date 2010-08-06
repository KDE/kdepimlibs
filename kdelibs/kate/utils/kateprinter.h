/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2002-2010 Anders Lund <anders@alweb.dk>
 *
 *  Rewritten based on code of Copyright (c) 2002 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef __KATE_PRINTER_H__
#define __KATE_PRINTER_H__

#include <QtGui/QWidget>

class KateDocument;

class KColorButton;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class KLineEdit;
class KIntSpinBox;

class KatePrinter
{
  public:
    static bool print (KateDocument *doc);
};

//BEGIN Text settings
/*
  Text settings page:
  - [ ] Print Selection (enabled if there is a selection in the view)
  - Print Line Numbers
    () Smart () Yes () No
*/
class KatePrintTextSettings : public QWidget
{
  Q_OBJECT
  public:
    explicit KatePrintTextSettings( QWidget *parent=0 );
    ~KatePrintTextSettings(){}

//     bool printSelection();
    bool printLineNumbers();
    bool printGuide();

    /* call if view has a selection, enables the seelction checkbox according to the arg */
//     void enableSelection( bool );

  private:
    QCheckBox /* *cbSelection,*/ *cbLineNumbers, *cbGuide;
};
//END Text Settings

//BEGIN Header/Footer
/*
  Header & Footer page:
  - enable header/footer
  - header/footer props
    o formats
    o colors
*/

class KatePrintHeaderFooter : public QWidget
{
  Q_OBJECT
  public:
    explicit KatePrintHeaderFooter( QWidget *parent=0 );
    ~KatePrintHeaderFooter(){}

    QFont font();

    bool useHeader();
    QStringList headerFormat();
    QColor headerForeground();
    QColor headerBackground();
    bool useHeaderBackground();

    bool useFooter();
    QStringList footerFormat();
    QColor footerForeground();
    QColor footerBackground();
    bool useFooterBackground();

  public Q_SLOTS:
    void setHFFont();

  private:
    QCheckBox *cbEnableHeader, *cbEnableFooter;
    QLabel *lFontPreview;
    QGroupBox *gbHeader, *gbFooter;
    KLineEdit *leHeaderLeft, *leHeaderCenter, *leHeaderRight;
    KColorButton *kcbtnHeaderFg, *kcbtnHeaderBg;
    QCheckBox *cbHeaderEnableBgColor;
    KLineEdit *leFooterLeft, *leFooterCenter, *leFooterRight;
    KColorButton *kcbtnFooterFg, *kcbtnFooterBg;
    QCheckBox *cbFooterEnableBgColor;
};

//END Header/Footer

//BEGIN Layout
/*
  Layout page:
  - Color scheme
  - Use Box
  - Box properties
    o Width
    o Margin
    o Color
*/
class KatePrintLayout : public QWidget
{
  Q_OBJECT
  public:
    explicit KatePrintLayout( QWidget *parent=0 );
    ~KatePrintLayout(){}

    QString colorScheme();
    bool useBackground();
    bool useBox();
    int boxWidth();
    int boxMargin();
    QColor boxColor();

  private:
    QComboBox *cmbSchema;
    QCheckBox *cbEnableBox, *cbDrawBackground;
    QGroupBox *gbBoxProps;
    KIntSpinBox *sbBoxWidth, *sbBoxMargin;
    KColorButton* kcbtnBoxColor;
};
//END Layout

#endif
