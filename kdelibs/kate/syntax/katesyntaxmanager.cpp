/* This file is part of the KDE libraries
   Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
   Copyright (C) 2003, 2004 Anders Lund <anders@alweb.dk>
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2001,2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

//BEGIN INCLUDES
#include "katesyntaxmanager.h"
#include "katesyntaxmanager.moc"

#include "katetextline.h"
#include "katedocument.h"
#include "katesyntaxdocument.h"
#include "katerenderer.h"
#include "kateglobal.h"
#include "kateschema.h"
#include "kateconfig.h"
#include "kateextendedattribute.h"
#include "katehighlight.h"

#include <kconfig.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kmenu.h>
#include <kcolorscheme.h>
#include <kcolorutils.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kapplication.h>

#include <QtCore/QSet>
#include <QtGui/QAction>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
//END

using namespace KTextEditor;

//BEGIN KateHlManager
KateHlManager::KateHlManager()
  : QObject()
  , m_config ("katesyntaxhighlightingrc", KConfig::NoGlobals)
  , commonSuffixes (QString(".orig;.new;~;.bak;.BAK").split(';'))
  , syntax (new KateSyntaxDocument(&m_config))
  , dynamicCtxsCount(0)
  , forceNoDCReset(false)
{
  KateSyntaxModeList modeList = syntax->modeList();
  for (int i=0; i < modeList.count(); i++)
  {
    KateHighlighting *hl = new KateHighlighting(modeList[i]);

    int insert = 0;
    for (; insert <= hlList.count(); insert++)
    {
      if (insert == hlList.count())
        break;

      if ( QString(hlList.at(insert)->section() + hlList.at(insert)->nameTranslated()).toLower()
            > QString(hl->section() + hl->nameTranslated()).toLower() )
        break;
    }

    hlList.insert (insert, hl);
    hlDict.insert (hl->name(), hl);
  }

  // Normal HL
  KateHighlighting *hl = new KateHighlighting(0);
  hlList.prepend (hl);
  hlDict.insert (hl->name(), hl);

  lastCtxsReset.start();
}

KateHlManager::~KateHlManager()
{
  delete syntax;
  qDeleteAll(hlList);
}

KateHlManager *KateHlManager::self()
{
  return KateGlobal::self ()->hlManager ();
}

KateHighlighting *KateHlManager::getHl(int n)
{
  if (n < 0 || n >= hlList.count())
    n = 0;

  return hlList.at(n);
}

int KateHlManager::nameFind(const QString &name)
{
  const QString lower_name = name.toLower();
  int z (hlList.count() - 1);
  for (; z > 0; z--)
    if (hlList.at(z)->name().toLower() == lower_name)
      return z;

  return z;
}

uint KateHlManager::defaultStyles()
{
  return 14;
}

QString KateHlManager::defaultStyleName(int n, bool translateNames)
{
  static QStringList names;
  static QStringList translatedNames;

  if (names.isEmpty())
  {
    names << "Normal";
    names << "Keyword";
    names << "Data Type";
    names << "Decimal/Value";
    names << "Base-N Integer";
    names << "Floating Point";
    names << "Character";
    names << "String";
    names << "Comment";
    names << "Others";
    names << "Alert";
    names << "Function";
    // this next one is for denoting the beginning/end of a user defined folding region
    names << "Region Marker";
    // this one is for marking invalid input
    names << "Error";

    translatedNames << i18nc("@item:intable Text context", "Normal");
    translatedNames << i18nc("@item:intable Text context", "Keyword");
    translatedNames << i18nc("@item:intable Text context", "Data Type");
    translatedNames << i18nc("@item:intable Text context", "Decimal/Value");
    translatedNames << i18nc("@item:intable Text context", "Base-N Integer");
    translatedNames << i18nc("@item:intable Text context", "Floating Point");
    translatedNames << i18nc("@item:intable Text context", "Character");
    translatedNames << i18nc("@item:intable Text context", "String");
    translatedNames << i18nc("@item:intable Text context", "Comment");
    translatedNames << i18nc("@item:intable Text context", "Others");
    translatedNames << i18nc("@item:intable Text context", "Alert");
    translatedNames << i18nc("@item:intable Text context", "Function");
    // this next one is for denoting the beginning/end of a user defined folding region
    translatedNames << i18nc("@item:intable Text context", "Region Marker");
    // this one is for marking invalid input
    translatedNames << i18nc("@item:intable Text context", "Error");
  }

  return translateNames ? translatedNames[n] : names[n];
}

void KateHlManager::getDefaults(const QString &schema, KateAttributeList &list)
{
  KColorScheme scheme(QPalette::Active, KColorScheme::View);
  KColorScheme schemeSelected(QPalette::Active, KColorScheme::Selection);

  ///NOTE: it's important to append in the order of the HighlightInterface::DefaultStyle
  ///      enum, to make KateDocument::defaultStyle() work properly.

  { // dsNormal
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground().color() );
    attrib->setSelectedForeground( schemeSelected.foreground().color() );
    list.append(attrib);
  }
  { // dsKeyword
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground().color() );
    attrib->setSelectedForeground( schemeSelected.foreground().color() );
    attrib->setFontBold(true);
    list.append(attrib);
  }
  { // dsDataType
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::LinkText).color() );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::LinkText).color() );
    list.append(attrib);
  }
  { // dsDecVal
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::NeutralText).color() );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::NeutralText).color() );
    list.append(attrib);
  }
  { // dsBaseN
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::NeutralText).color() );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::NeutralText).color() );
    list.append(attrib);
  }
  { // dsFloat
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::NeutralText).color() );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::NeutralText).color() );
    list.append(attrib);
  }
  { // dsChar
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::ActiveText).color() );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::ActiveText).color() );
    list.append(attrib);
  }
  { // dsString
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::NegativeText).color() );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::NegativeText).color() );
    list.append(attrib);
  }
  { // dsComment
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::InactiveText).color() );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::InactiveText).color() );
    attrib->setFontItalic(true);
    list.append(attrib);
  }
  { // dsOthers
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::PositiveText).color() );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::PositiveText).color() );
    list.append(attrib);
  }
  { // dsAlert
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::NegativeText).color() );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::NegativeText).color() );
    attrib->setFontBold(true);
    attrib->setBackground( scheme.background(KColorScheme::NegativeBackground).color() );
    list.append(attrib);
  }
  { // dsFunction
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::VisitedText).color() );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::VisitedText).color() );
    list.append(attrib);
  }
  { // dsRegionMarker
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::LinkText).color() );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::LinkText).color() );
    attrib->setBackground( scheme.background(KColorScheme::LinkBackground).color() );
    list.append(attrib);
  }
  { // dsError
    Attribute::Ptr attrib(new KTextEditor::Attribute());
    attrib->setForeground( scheme.foreground(KColorScheme::NegativeText) );
    attrib->setSelectedForeground( schemeSelected.foreground(KColorScheme::NegativeText).color() );
    attrib->setFontUnderline(true);
    list.append(attrib);
  }

  KConfigGroup config(KateHlManager::self()->self()->getKConfig(),
                      "Default Item Styles - Schema " + schema);

  for (uint z = 0; z < defaultStyles(); z++)
  {
    KTextEditor::Attribute::Ptr i = list.at(z);
    QStringList s = config.readEntry(defaultStyleName(z), QStringList());
    if (!s.isEmpty())
    {
      while( s.count()<9)
        s << "";

      QString tmp;
      QRgb col;

      tmp=s[0]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); i->setForeground(QColor(col)); }

      tmp=s[1]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); i->setSelectedForeground(QColor(col)); }

      tmp=s[2]; if (!tmp.isEmpty()) i->setFontBold(tmp!="0");

      tmp=s[3]; if (!tmp.isEmpty()) i->setFontItalic(tmp!="0");

      tmp=s[4]; if (!tmp.isEmpty()) i->setFontStrikeOut(tmp!="0");

      tmp=s[5]; if (!tmp.isEmpty()) i->setFontUnderline(tmp!="0");

      tmp=s[6]; if (!tmp.isEmpty()) {
        if ( tmp != "-" )
        {
          col=tmp.toUInt(0,16);
          i->setBackground(QColor(col));
        }
        else
          i->clearBackground();
      }
      tmp=s[7]; if (!tmp.isEmpty()) {
        if ( tmp != "-" )
        {
          col=tmp.toUInt(0,16);
          i->setSelectedBackground(QColor(col));
        }
        else
          i->clearProperty(KTextEditor::Attribute::SelectedBackground);
      }
      tmp=s[8]; if (!tmp.isEmpty() && tmp!=QLatin1String("---")) i->setFontFamily(tmp);
    }
  }
}

void KateHlManager::setDefaults(const QString &schema, KateAttributeList &list)
{
  KConfigGroup config(KateHlManager::self()->self()->getKConfig(),
                      "Default Item Styles - Schema " + schema);

  for (uint z = 0; z < defaultStyles(); z++)
  {
    QStringList settings;
    KTextEditor::Attribute::Ptr p = list.at(z);

    settings<<(p->hasProperty(QTextFormat::ForegroundBrush)?QString::number(p->foreground().color().rgb(),16):"");
    settings<<(p->hasProperty(KTextEditor::Attribute::SelectedForeground)?QString::number(p->selectedForeground().color().rgb(),16):"");
    settings<<(p->hasProperty(QTextFormat::FontWeight)?(p->fontBold()?"1":"0"):"");
    settings<<(p->hasProperty(QTextFormat::FontItalic)?(p->fontItalic()?"1":"0"):"");
    settings<<(p->hasProperty(QTextFormat::FontStrikeOut)?(p->fontStrikeOut()?"1":"0"):"");
    settings<<(p->hasProperty(QTextFormat::FontUnderline)?(p->fontUnderline()?"1":"0"):"");
    settings<<(p->hasProperty(QTextFormat::BackgroundBrush)?QString::number(p->background().color().rgb(),16):"");
    settings<<(p->hasProperty(KTextEditor::Attribute::SelectedBackground)?QString::number(p->selectedBackground().color().rgb(),16):"");
    settings<<(p->hasProperty(QTextFormat::FontFamily)?(p->fontFamily()):QString());
    settings<<"---";

    config.writeEntry(defaultStyleName(z),settings);
  }

  emit changed();
}

int KateHlManager::highlights()
{
  return (int) hlList.count();
}

QString KateHlManager::hlName(int n)
{
  return hlList.at(n)->name();
}

QString KateHlManager::hlNameTranslated(int n)
{
  return hlList.at(n)->nameTranslated();
}

QString KateHlManager::hlSection(int n)
{
  return hlList.at(n)->section();
}

bool KateHlManager::hlHidden(int n)
{
  return hlList.at(n)->hidden();
}

QString KateHlManager::identifierForName(const QString& name)
{
  KateHighlighting *hl = 0;

  if ((hl = hlDict[name]))
    return hl->getIdentifier ();

  return QString();
}

QString KateHlManager::nameForIdentifier(const QString& identifier)
{
  for ( QHash<QString, KateHighlighting*>::iterator it = hlDict.begin();
        it != hlDict.end(); ++it )
  {
    if ( (*it)->getIdentifier() == identifier ) {
      return it.key();
    }
  }

  return QString();
}

bool KateHlManager::resetDynamicCtxs()
{
  if (forceNoDCReset)
    return false;

  if (lastCtxsReset.elapsed() < KATE_DYNAMIC_CONTEXTS_RESET_DELAY)
    return false;

  foreach (KateHighlighting *hl, hlList)
    hl->dropDynamicContexts();

  dynamicCtxsCount = 0;
  lastCtxsReset.start();

  return true;
}
//END

// kate: space-indent on; indent-width 2; replace-tabs on;
