/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2004 Apple Computer, Inc.
 *           (C) 2008 Germain Garand <germain@ebooksfrance.org>
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

//#define CSS_STYLESHEET_DEBUG

#include "css_stylesheetimpl.h"
#include "css_ruleimpl.h"
#include "css_valueimpl.h"
#include "cssparser.h"
#include "css_mediaquery.h"

#include <dom/dom_string.h>
#include <dom/dom_exception.h>
#include <dom/css_stylesheet.h>
#include <dom/css_rule.h>
#include <dom/dom_exception.h>

#include <xml/dom_nodeimpl.h>
#include <html/html_documentimpl.h>
#include <misc/loader.h>

#include <kdebug.h>

using namespace DOM;
using namespace khtml;
// --------------------------------------------------------------------------------

StyleSheetImpl::StyleSheetImpl(StyleSheetImpl *parentSheet, DOMString href)
    : StyleListImpl(parentSheet)
{
    m_disabled = false;
    m_media = 0;
    m_parentNode = 0;
    m_strHref = href;
}


StyleSheetImpl::StyleSheetImpl(DOM::NodeImpl *parentNode, DOMString href)
    : StyleListImpl()
{
    m_parentNode = parentNode;
    m_disabled = false;
    m_media = 0;
    m_strHref = href;
}

StyleSheetImpl::StyleSheetImpl(StyleBaseImpl *owner, DOMString href)
    : StyleListImpl(owner)
{
    m_disabled = false;
    m_media = 0;
    m_parentNode = 0;
    m_strHref = href;
}

StyleSheetImpl::~StyleSheetImpl()
{
    if(m_media) {
	m_media->setParent( 0 );
	m_media->deref();
    }
}

StyleSheetImpl *StyleSheetImpl::parentStyleSheet() const
{
    if( !m_parent ) return 0;
    if( m_parent->isStyleSheet() ) return static_cast<StyleSheetImpl *>(m_parent);
    if( m_parent->isRule() ) return m_parent->stylesheet();
    return 0;
}

void StyleSheetImpl::setMedia( MediaListImpl *media )
{
    if( media )
	media->ref();
    if( m_media ) {
        m_media->setParent( 0 );
	m_media->deref();
    }
    m_media = media;
    if (m_media)
        m_media->setParent( this );
}

void StyleSheetImpl::setDisabled( bool disabled )
{
    bool updateStyle = isCSSStyleSheet() && m_parentNode && disabled != m_disabled;
    m_disabled = disabled;
    if (updateStyle)
        m_parentNode->document()->updateStyleSelector();
}

// -----------------------------------------------------------------------


CSSStyleSheetImpl::CSSStyleSheetImpl(CSSStyleSheetImpl *parentSheet, DOMString href)
    : StyleSheetImpl(parentSheet, href)
{
    m_lstChildren = new QList<StyleBaseImpl*>;
    m_doc = parentSheet ? parentSheet->doc() : 0;
    m_implicit = false;
    m_namespaces = 0;
    m_defaultNamespace = NamespaceName::fromId(anyNamespace);
    m_loadedHint = false;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(DOM::NodeImpl *parentNode, DOMString href, bool _implicit)
    : StyleSheetImpl(parentNode, href)
{
    m_lstChildren = new QList<StyleBaseImpl*>;
    m_doc = parentNode->document();
    m_implicit = _implicit;
    m_namespaces = 0;
    m_defaultNamespace = NamespaceName::fromId(anyNamespace);
    m_loadedHint = false;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(CSSRuleImpl *ownerRule, DOMString href)
    : StyleSheetImpl(ownerRule, href)
{
    m_lstChildren = new QList<StyleBaseImpl*>;
    m_doc = static_cast<CSSStyleSheetImpl*>(ownerRule->stylesheet())->doc();
    m_implicit = false;
    m_namespaces = 0;
    m_defaultNamespace = NamespaceName::fromId(anyNamespace);
    m_loadedHint = false;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(DOM::NodeImpl *parentNode, CSSStyleSheetImpl *orig)
    : StyleSheetImpl(parentNode, orig->m_strHref)
{
    m_lstChildren = new QList<StyleBaseImpl*>;
    StyleBaseImpl *rule;
    QListIterator<StyleBaseImpl*> it( *orig->m_lstChildren );
    while ( it.hasNext() )
    {
        rule = it.next();
        m_lstChildren->append(rule);
        rule->setParent(this);
    }
    m_doc = parentNode->document();
    m_implicit = false;
    m_namespaces = 0;
    m_defaultNamespace = NamespaceName::fromId(anyNamespace);
    m_loadedHint = false;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(CSSRuleImpl *ownerRule, CSSStyleSheetImpl *orig)
    : StyleSheetImpl(ownerRule, orig->m_strHref)
{
    // m_lstChildren is deleted in StyleListImpl
    m_lstChildren = new QList<StyleBaseImpl*>;
    StyleBaseImpl *rule;
    QListIterator<StyleBaseImpl*> it( *orig->m_lstChildren );
    while ( it.hasNext() )
    {
        rule = it.next();
        m_lstChildren->append(rule);
        rule->setParent(this);
    }
    m_doc = static_cast<CSSStyleSheetImpl*>(ownerRule->stylesheet())->doc();
    m_implicit = false;
    m_namespaces = 0;
    m_defaultNamespace = NamespaceName::fromId(anyNamespace);
    m_loadedHint = false;
}

CSSRuleImpl *CSSStyleSheetImpl::ownerRule() const
{
    if( !m_parent ) return 0;
    if( m_parent->isRule() ) return static_cast<CSSRuleImpl *>(m_parent);
    return 0;
}

unsigned long CSSStyleSheetImpl::insertRule( const DOMString &rule, unsigned long index, int &exceptioncode )
{
    exceptioncode = 0;
    if (index > (unsigned) m_lstChildren->count()) {
        exceptioncode = DOMException::INDEX_SIZE_ERR;
        return 0;
    }
    CSSParser p( strictParsing );
    CSSRuleImpl *r = p.parseRule( this, rule );

    if(!r) {
        exceptioncode = CSSException::SYNTAX_ERR + CSSException::_EXCEPTION_OFFSET;
        return 0;
    }
    // ###
    // HIERARCHY_REQUEST_ERR: Raised if the rule cannot be inserted at the specified index e.g. if an
    //@import rule is inserted after a standard rule set or other at-rule.
    m_lstChildren->insert(index, r);
    if (m_doc)
        m_doc->updateStyleSelector(true /*shallow*/);
    return index;
}

CSSRuleListImpl *CSSStyleSheetImpl::cssRules(bool omitCharsetRules)
{
    return new CSSRuleListImpl(this, omitCharsetRules);
}

void CSSStyleSheetImpl::deleteRule( unsigned long index, int &exceptioncode )
{
    exceptioncode = 0;
    if (index+1 > (unsigned) m_lstChildren->count()) {
        exceptioncode = DOMException::INDEX_SIZE_ERR;
        return;
    }
    StyleBaseImpl *b = m_lstChildren->takeAt(index);

    // TreeShared requires delete not deref when removed from tree
    b->setParent(0);
    if( !b->refCount() ) delete b;
    if (m_doc)
        m_doc->updateStyleSelector(true /*shallow*/);
}

void CSSStyleSheetImpl::addNamespace(CSSParser* /*p*/, const DOM::DOMString& prefix, const DOM::DOMString& uri)
{
    if (uri.isNull())
        return;

    m_namespaces = new CSSNamespace(prefix, uri, m_namespaces);

    if (prefix.isEmpty()) {
        Q_ASSERT(m_doc != 0);

        m_defaultNamespace = NamespaceName::fromString(uri);
    }
}

void CSSStyleSheetImpl::determineNamespace(NamespaceName& namespacename, const DOM::DOMString& prefix)
{
    // If the stylesheet has no namespaces we can just return.  There won't be any need to ever check
    // namespace values in selectors.
    //if (!m_namespaces)
    //    return;

    if (prefix.isEmpty())
        namespacename = NamespaceName::fromId(emptyNamespace); // No namespace. If an element/attribute has a namespace, we won't match it.
    else if (prefix == "*")
        namespacename = NamespaceName::fromId(anyNamespace); // We'll match any namespace.
    else {
        if (!m_namespaces)
            return;
        CSSNamespace* ns = m_namespaces->namespaceForPrefix(prefix);
        if (ns) {
            Q_ASSERT(m_doc != 0);

            // Look up the id for this namespace URI.
            namespacename = NamespaceName::fromString(ns->uri());
        }
    }
}

bool CSSStyleSheetImpl::parseString(const DOMString &string, bool strict)
{
#ifdef CSS_STYLESHEET_DEBUG
    kDebug( 6080 ) << "parsing sheet, len=" << string.length() << ", sheet is " << string.string();
#endif

    strictParsing = strict;
    CSSParser p( strict );
    p.parseSheet( this, string );
    return true;
}

bool CSSStyleSheetImpl::isLoading() const
{
    StyleBaseImpl *rule;
    QListIterator<StyleBaseImpl*> it( *m_lstChildren );
    while ( it.hasNext() )
    {
        rule = it.next();
        if(rule->isImportRule())
        {
            CSSImportRuleImpl *import = static_cast<CSSImportRuleImpl *>(rule);
#ifdef CSS_STYLESHEET_DEBUG
            kDebug( 6080 ) << "found import";
#endif
            if(import->isLoading())
            {
#ifdef CSS_STYLESHEET_DEBUG
                kDebug( 6080 ) << "--> not loaded";
#endif
                m_loadedHint = false;
                return true;
            }
        }
    }
    m_loadedHint = true;
    return false;
}

void CSSStyleSheetImpl::checkLoaded() const
{
    if (isLoading()) 
        return;
    if (m_parent) 
        m_parent->checkLoaded();
    if (m_parentNode)
        m_loadedHint = m_parentNode->checkRemovePendingSheet();
    else if (parentStyleSheet() && parentStyleSheet()->isCSSStyleSheet())
        m_loadedHint = static_cast<CSSStyleSheetImpl*>(parentStyleSheet())->loadedHint();
    else
        m_loadedHint = true;
}

void CSSStyleSheetImpl::checkPending() const
{
    if (!m_loadedHint)
        return;
    if (m_parent)
        m_parent->checkPending();
    else if (m_parentNode)
        m_parentNode->checkAddPendingSheet();
}

// ---------------------------------------------------------------------------


StyleSheetListImpl::~StyleSheetListImpl()
{
    foreach (StyleSheetImpl* sh, styleSheets)
        sh->deref();
}

void StyleSheetListImpl::add( StyleSheetImpl* s )
{
    if (managerDocument)
        managerDocument->ensureStyleSheetListUpToDate();

    // ### in cases this is document.styleSheets, maybe 
    // we should route to DocumentImpl::addStyleSheets?

    if ( !styleSheets.contains( s ) ) {
        s->ref();
        styleSheets.append( s );
    }
}

void StyleSheetListImpl::remove( StyleSheetImpl* s )
{
    if (managerDocument)
        managerDocument->ensureStyleSheetListUpToDate();

    if ( styleSheets.removeAll( s ) )
        s->deref();
}

unsigned long StyleSheetListImpl::length() const
{
    if (managerDocument)
        managerDocument->ensureStyleSheetListUpToDate();

    // hack so implicit BODY stylesheets don't get counted here
    unsigned long l = 0;
    foreach (StyleSheetImpl* sh, styleSheets) {
        if (!sh->isCSSStyleSheet() || !static_cast<CSSStyleSheetImpl*>(sh)->implicit())
            ++l;
    }
    return l;
}

StyleSheetImpl *StyleSheetListImpl::item ( unsigned long index )
{
    if (managerDocument)
        managerDocument->ensureStyleSheetListUpToDate();

    unsigned long l = 0;
    foreach (StyleSheetImpl* sh, styleSheets) {
        if (!sh->isCSSStyleSheet() || !static_cast<CSSStyleSheetImpl*>(sh)->implicit()) {
            if (l == index)
                return sh;
            ++l;
        }
    }
    return 0;
}

// --------------------------------------------------------------------------------------------

/* MediaList is used to store 3 types of media related entities which mean the same:
 * Media Queries, Media Types and Media Descriptors.
 * Currently MediaList always tries to parse media queries and if parsing fails,
 * tries to fallback to Media Descriptors if m_fallback flag is set.
 * Slight problem with syntax error handling:
 * CSS 2.1 Spec (http://www.w3.org/TR/CSS21/media.html)
 * specifies that failing media type parsing is a syntax error
 * CSS 3 Media Queries Spec (http://www.w3.org/TR/css3-mediaqueries/)
 * specifies that failing media query is a syntax error
 * HTML 4.01 spec (http://www.w3.org/TR/REC-html40/present/styles.html#adef-media)
 * specifies that Media Descriptors should be parsed with forward-compatible syntax
 * DOM Level 2 Style Sheet spec (http://www.w3.org/TR/DOM-Level-2-Style/)
 * talks about MediaList.mediaText and refers
 *   -  to Media Descriptors of HTML 4.0 in context of StyleSheet
 *   -  to Media Types of CSS 2.0 in context of CSSMediaRule and CSSImportRule
 *
 * These facts create situation where same (illegal) media specification may result in
 * different parses depending on whether it is media attr of style element or part of
 * css @media rule.
 * <style media="screen and resolution > 40dpi"> ..</style> will be enabled on screen devices where as
 * @media screen and resolution > 40dpi {..} will not.
 * This gets more counter-intuitive in JavaScript:
 * document.styleSheets[0].media.mediaText = "screen and resolution > 40dpi" will be ok and
 * enabled, while
 * document.styleSheets[0].cssRules[0].media.mediaText = "screen and resolution > 40dpi" will
 * throw SYNTAX_ERR exception.
 */

MediaListImpl::MediaListImpl( CSSStyleSheetImpl *parentSheet,
                              const DOMString &media, bool fallbackToDescriptor)
    : StyleBaseImpl( parentSheet )
    , m_fallback(fallbackToDescriptor)
{
    int ec = 0;
    setMediaText(media, ec);
    // FIXME: parsing can fail. The problem with failing constructor is that
    // we would need additional flag saying MediaList is not valid
    // Parse can fail only when fallbackToDescriptor == false, i.e when HTML4 media descriptor
    // forward-compatible syntax is not in use. 
    // DOMImplementationCSS seems to mandate that media descriptors are used
    // for both html and svg, even though svg:style doesn't use media descriptors
    // Currently the only places where parsing can fail are
    // creating <svg:style>, creating css media / import rules from js
    if (ec)
        setMediaText("invalid", ec);
}

MediaListImpl::MediaListImpl( CSSRuleImpl *parentRule, const DOMString &media, bool fallbackToDescriptor)
    : StyleBaseImpl(parentRule)
    , m_fallback(fallbackToDescriptor)
{
    int ec = 0;
    setMediaText(media, ec);
    // FIXME: parsing can fail. The problem with failing constructor is that
    // we would need additional flag saying MediaList is not valid
    // Parse can fail only when fallbackToDescriptor == false, i.e when HTML4 media descriptor
    // forward-compatible syntax is not in use. 
    // DOMImplementationCSS seems to mandate that media descriptors are used
    // for both html and svg, even though svg:style doesn't use media descriptors
    // Currently the only places where parsing can fail are
    // creating <svg:style>, creating css media / import rules from js
    if (ec)
        setMediaText("invalid", ec);
}

MediaListImpl::~MediaListImpl()
{
    qDeleteAll(m_queries);
}

CSSStyleSheetImpl *MediaListImpl::parentStyleSheet() const
{
    if( m_parent->isCSSStyleSheet() ) return static_cast<CSSStyleSheetImpl *>(m_parent);
    return 0;
}

CSSRuleImpl *MediaListImpl::parentRule() const
{
    if( m_parent->isRule() ) return static_cast<CSSRuleImpl *>(m_parent);
    return 0;
}

static DOMString parseMediaDescriptor(const DOMString& s)
{
    int len = s.length();

    // http://www.w3.org/TR/REC-html40/types.html#type-media-descriptors
    // "Each entry is truncated just before the first character that isn't a
    // US ASCII letter [a-zA-Z] (ISO 10646 hex 41-5a, 61-7a), digit [0-9] (hex 30-39),
    // or hyphen (hex 2d)."
    int i;
    unsigned short c;
    for (i = 0; i < len; ++i) {
        c = s[i].unicode();
        if (! ((c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || (c >= '1' && c <= '9')
            || (c == '-')))
            break;
    }
    return s.implementation()->substring(0, i);
}

void MediaListImpl::deleteMedium(const DOMString& oldMedium, int& ec)
{
    MediaListImpl tempMediaList;
    CSSParser p(true);

    MediaQuery* oldQuery = 0;
    bool deleteOldQuery = false;

    if (p.parseMediaQuery(&tempMediaList, oldMedium)) {
        if (tempMediaList.m_queries.size() > 0)
            oldQuery = tempMediaList.m_queries[0];
    } else if (m_fallback) {
        DOMString medium = parseMediaDescriptor(oldMedium);
        if (!medium.isNull()) {
            oldQuery = new MediaQuery(MediaQuery::None, medium, 0);
            deleteOldQuery = true;
        }
    }

    // DOM Style Sheets spec doesn't allow SYNTAX_ERR to be thrown in deleteMedium
    ec = DOMException::NOT_FOUND_ERR;

    if (oldQuery) {
        for(int i = 0; i < m_queries.size(); ++i) {
            MediaQuery* a = m_queries[i];
            if (*a == *oldQuery) {
                m_queries.removeAt(i);
                delete a;
                ec = 0;
                break;
            }
        }
        if (deleteOldQuery)
            delete oldQuery;
    }
}

DOM::DOMString MediaListImpl::mediaText() const
{
    DOMString text;
    bool first = true;
    const QList<MediaQuery*>::ConstIterator itEnd = m_queries.end();

    for ( QList<MediaQuery*>::ConstIterator it = m_queries.begin(); it != itEnd; ++it ) {
        if (!first)
            text += ", ";
        text += (*it)->cssText();
        first = false;
    }
    return text;
}

void MediaListImpl::setMediaText(const DOM::DOMString &value, int& ec)
{
    MediaListImpl tempMediaList;
    CSSParser p(true);

    const QString val = value.string();
    const QStringList list = val.split( ',' );

    const QStringList::ConstIterator itEnd = list.end();

    for ( QStringList::ConstIterator it = list.begin(); it != itEnd; ++it )
    {
        const DOMString medium = (*it).trimmed();
        if( !medium.isEmpty() ) {
            if (!p.parseMediaQuery(&tempMediaList, medium)) {
                if (m_fallback) {
                    DOMString mediaDescriptor = parseMediaDescriptor(medium);
                    if (!mediaDescriptor.isNull())
                        tempMediaList.m_queries.append(new MediaQuery(MediaQuery::None, mediaDescriptor, 0));
                } else {
                    ec = CSSException::SYNTAX_ERR;
                    return;
                }
            }          
        } else if (!m_fallback) {
            ec = CSSException::SYNTAX_ERR;
            return;
        }            
    }
    // ",,,," falls straight through, but is not valid unless fallback
    if (!m_fallback && list.begin() == list.end()) {
        DOMString s = value.string().trimmed();
        if (!s.isEmpty()) {
            ec = CSSException::SYNTAX_ERR;
            return;
            }
    }
    
    ec = 0;
    qDeleteAll(m_queries);
    m_queries = tempMediaList.m_queries;
    tempMediaList.m_queries.clear();
}

 
DOMString MediaListImpl::item(unsigned long index) const
{
    if (index < (unsigned)m_queries.size()) {
        MediaQuery* query = m_queries[index];
        return query->cssText();
    }

    return DOMString();
}

void MediaListImpl::appendMedium(const DOMString& newMedium, int& ec)
{
    ec = DOMException::INVALID_CHARACTER_ERR;
    CSSParser p(true);
    if (p.parseMediaQuery(this, newMedium)) {
        ec = 0;
    } else if (m_fallback) {
        DOMString medium = parseMediaDescriptor(newMedium);
        if (!medium.isNull()) {
            m_queries.append(new MediaQuery(MediaQuery::None, medium, 0));
            ec = 0;
        }
    }
}

void MediaListImpl::appendMediaQuery(MediaQuery* mediaQuery)
{
    m_queries.append(mediaQuery);
}
