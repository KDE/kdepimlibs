/* This file is part of the KDE libraries
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001 Anders Lund <anders@alweb.dk>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

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

#ifndef __KATE_VIEW_HELPERS_H__
#define __KATE_VIEW_HELPERS_H__

#include <kselectaction.h>
#include <kencodingprober.h>
#include <klineedit.h>

#include <QtGui/QPixmap>
#include <QtGui/QColor>
#include <QtGui/QScrollBar>
#include <QtCore/QHash>
#include <QtGui/QStackedWidget>
#include <QtCore/QMap>
#include <QtCore/QTimer>

#include <ktexteditor/containerinterface.h>

class KateDocument;
class KateView;
class KateViewInternal;

#define MAXFOLDINGCOLORS 16

class KateLineInfo;

namespace KTextEditor {
  class Command;
  class SmartRange;
  class AnnotationModel;
  class MovingRange;
}

class QTimer;

/**
 * This class is required because QScrollBar's sliderMoved() signal is
 * really supposed to be a sliderDragged() signal... so this way we can capture
 * MMB slider moves as well
 *
 * Also, it adds some useful indicators on the scrollbar.
 */
class KateScrollBar : public QScrollBar
{
  Q_OBJECT

  public:
    KateScrollBar(Qt::Orientation orientation, class KateViewInternal *parent);

    inline bool showMarks() { return m_showMarks; }
    inline void setShowMarks(bool b) { m_showMarks = b; update(); }

  Q_SIGNALS:
    void sliderMMBMoved(int value);

  protected:
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseMoveEvent (QMouseEvent* e);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void styleChange(QStyle &oldStyle);
    virtual void sliderChange ( SliderChange change );
    virtual void wheelEvent(QWheelEvent *e);

  protected Q_SLOTS:
    void sliderMaybeMoved(int value);
    void marksChanged();

  private:
    void redrawMarks();
    void recomputeMarksPositions();

    bool m_middleMouseDown;

    KateView *m_view;
    KateDocument *m_doc;
    class KateViewInternal *m_viewInternal;

    QHash<int, QColor> m_lines;

    bool m_showMarks;
};

class KateIconBorder : public QWidget
{
  Q_OBJECT

  public:
    KateIconBorder( KateViewInternal* internalView, QWidget *parent );
    virtual ~KateIconBorder();
    // VERY IMPORTANT ;)
    virtual QSize sizeHint() const;

    void updateFont();
    int lineNumberWidth() const;

    void setIconBorderOn(       bool enable );
    void setLineNumbersOn(      bool enable );
    void setAnnotationBorderOn( bool enable );
    void setDynWrapIndicators(int state );
    int dynWrapIndicators()  const { return m_dynWrapIndicators; }
    bool dynWrapIndicatorsOn() const { return m_dynWrapIndicatorsOn; }
    void setFoldingMarkersOn( bool enable );
    void toggleIconBorder()     { setIconBorderOn(     !iconBorderOn() );     }
    void toggleLineNumbers()    { setLineNumbersOn(    !lineNumbersOn() );    }
    void toggleFoldingMarkers() { setFoldingMarkersOn( !foldingMarkersOn() ); }
    inline bool iconBorderOn()       const { return m_iconBorderOn;       }
    inline bool lineNumbersOn()      const { return m_lineNumbersOn;      }
    inline bool foldingMarkersOn()   const { return m_foldingMarkersOn;   }
    inline bool annotationBorderOn() const { return m_annotationBorderOn; }

    enum BorderArea { None, LineNumbers, IconBorder, FoldingMarkers, AnnotationBorder };
    BorderArea positionToArea( const QPoint& ) const;

  Q_SIGNALS:
    void toggleRegionVisibility( unsigned int );
  public Q_SLOTS:
    void updateAnnotationBorderWidth();
    void updateAnnotationLine( int line );
    void annotationModelChanged( KTextEditor::AnnotationModel* oldmodel, KTextEditor::AnnotationModel* newmodel );

  private:
    void paintEvent( QPaintEvent* );
    void paintBorder (int x, int y, int width, int height);

    void mousePressEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void mouseDoubleClickEvent( QMouseEvent* );
    void leaveEvent(QEvent *event);

    void showMarkMenu( uint line, const QPoint& pos );

    void showAnnotationTooltip( int line, const QPoint& pos );
    void hideAnnotationTooltip();
    void removeAnnotationHovering();
    void showAnnotationMenu( int line, const QPoint& pos);
    int annotationLineWidth( int line );

    KateView *m_view;
    KateDocument *m_doc;
    KateViewInternal *m_viewInternal;

    bool m_iconBorderOn:1;
    bool m_lineNumbersOn:1;
    bool m_foldingMarkersOn:1;
    bool m_dynWrapIndicatorsOn:1;
    bool m_annotationBorderOn:1;
    int m_dynWrapIndicators;

    int m_lastClickedLine;

    int m_cachedLNWidth;

    int m_maxCharWidth;
    int iconPaneWidth;
    int m_annotationBorderWidth;

    mutable QPixmap m_arrow;
    mutable QColor m_oldBackgroundColor;


    KTextEditor::MovingRange *m_foldingRange;
    int m_nextHighlightBlock;
    int m_currentBlockLine;
    QTimer m_delayFoldingHlTimer;
    void showDelayedBlock(int line);
    void hideBlock();

  private Q_SLOTS:
    void showBlock();

  private:
    QColor m_foldingColors[MAXFOLDINGCOLORS];
    QBrush foldingColor(KateLineInfo *, int,bool solid);
    QString m_hoveredAnnotationText;

    void initializeFoldingColors();
};

class KateViewEncodingAction: public KSelectAction
{
  Q_OBJECT

  Q_PROPERTY(QString codecName READ currentCodecName WRITE setCurrentCodec)
  Q_PROPERTY(int codecMib READ currentCodecMib)

  public:
    KateViewEncodingAction(KateDocument *_doc, KateView *_view, const QString& text, QObject *parent);

    ~KateViewEncodingAction();

    int mibForName(const QString &codecName, bool *ok = 0) const;
    QTextCodec *codecForMib(int mib) const;

    QTextCodec *currentCodec() const;
    bool setCurrentCodec(QTextCodec *codec);

    QString currentCodecName() const;
    bool setCurrentCodec(const QString &codecName);

    int currentCodecMib() const;
    bool setCurrentCodec(int mib);

  Q_SIGNALS:
    /**
    * Specific (proper) codec was selected
    */
    void triggered(QTextCodec *codec);

  private:
    KateDocument* doc;
    KateView *view;
    class Private;
    Private* const d;
    Q_PRIVATE_SLOT( d, void _k_subActionTriggered(QAction*) )

  private Q_SLOTS:
    void setEncoding (const QString &e);
    void slotAboutToShow();
};

class KateViewBar;

class KateViewBarWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit KateViewBarWidget (bool addCloseButton, QWidget* parent = 0);

    virtual void closed(){};
  protected:
    /**
     * @return widget that should be used to add controls to bar widget
     */
    QWidget *centralWidget() { return m_centralWidget; }

  signals:
    void hideMe();

  private:
    QWidget *m_centralWidget;
};

// Helper layout class to always provide minimum size
class KateStackedWidget : public QStackedWidget
{
  Q_OBJECT
public:
  KateStackedWidget(QWidget* parent);
  virtual QSize sizeHint() const;
  virtual QSize minimumSize() const;
};

class QVBoxLayout;

class KateViewBar : public QWidget
{
  Q_OBJECT

  public:
    KateViewBar (bool external, KTextEditor::ViewBarContainer::Position pos,QWidget *parent,KateView *view);


    /**
     * Adds a widget to this viewbar.
     * Widget is initially invisible, you should call showBarWidget, to show it.
     * Several widgets can be added to the bar, but only one can be visible
     */
    void addBarWidget (KateViewBarWidget *newBarWidget);
    /**
     * Shows barWidget that was previously added with addBarWidget.
     * @see hideCurrentBarWidget
     */
    void showBarWidget (KateViewBarWidget *barWidget);

    /**
     * Adds widget that will be always shown in the viewbar.
     * After adding permanent widget viewbar is immediately shown.
     * ViewBar with permanent widget won't hide itself
     * until permanent widget is removed.
     * OTOH showing/hiding regular barWidgets will work as usual
     * (they will be shown above permanent widget)
     *
     * If permanent widget already exists, new one replaces old one
     * Old widget is not deleted, caller can do it if it wishes
     */
    void addPermanentBarWidget (KateViewBarWidget *barWidget);
    /**
     * Removes permanent bar widget from viewbar.
     * If no other viewbar widgets are shown, viewbar gets hidden.
     *
     * barWidget is not deleted, caller must do it if it wishes
     */
    void removePermanentBarWidget (KateViewBarWidget *barWidget);
    /**
     * @return if viewbar has permanent widget @p barWidget
     */
    bool hasPermanentWidget (KateViewBarWidget *barWidget) const;

  public Q_SLOTS:
    /**
     * Hides currently shown bar widget
     */
    void hideCurrentBarWidget();

  protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void hideEvent(QHideEvent* event);

  private:
    bool hasWidget(KateViewBarWidget*) const;
    /**
     * Shows or hides whole viewbar
     */
    void setViewBarVisible(bool visible);

    bool m_external;
    KTextEditor::ViewBarContainer::Position m_pos;

  private:
    KateView *m_view;
    KateStackedWidget *m_stack;
    KateViewBarWidget *m_permanentBarWidget;
    QVBoxLayout *m_layout;
};

class KateCommandLineBar : public KateViewBarWidget
{
  public:
    explicit KateCommandLineBar(KateView *view, QWidget *parent = 0);
    ~KateCommandLineBar();

  void setText(const QString &text, bool selected = true);

  private:
    class KateCmdLineEdit *m_lineEdit;
};

class KateCmdLineEdit : public KLineEdit
{
  Q_OBJECT

  public:
    KateCmdLineEdit (KateCommandLineBar *bar, KateView *view);
    virtual bool event(QEvent *e);

    void hideEvent (QHideEvent *e);

  signals:
    void hideRequested();

  private Q_SLOTS:
    void hideLineEdit();
    void slotReturnPressed ( const QString& cmd );

  protected:
    void focusInEvent ( QFocusEvent *ev );
    void keyPressEvent( QKeyEvent *ev );

  private:
    void fromHistory( bool up );
    QString helptext( const QPoint & ) const;

    KateView *m_view;
    KateCommandLineBar *m_bar;
    bool m_msgMode;
    QString m_oldText;
    uint m_histpos; ///< position in the history
    uint m_cmdend; ///< the point where a command ends in the text, if we have a valid one.
    KTextEditor::Command *m_command; ///< For completing flags/args and interactiveness
    class KateCmdLnWhatsThis *m_help;
    QRegExp m_cmdRange;
    QRegExp m_cmdExpr;
    QRegExp m_gotoLine;
    QTimer *m_hideTimer;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
