#ifndef GUI_OPENGL___GL_CONTAINER__HPP
#define GUI_OPENGL___GL_CONTAINER__HPP

/*  $Id: glcontainer.hpp 14562 2007-05-18 11:48:21Z dicuccio $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Peter Meric
 *
 * File Description:
 *     CGlContainer
 */

#include <corelib/ncbiobj.hpp>
#include <gui/opengl/glwidget.hpp>
#include <gui/opengl/glevent.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */


BEGIN_NCBI_SCOPE


class NCBI_GUIOPENGL_EXPORT CGlContainer : public CGlWidget
{
protected:
    typedef CRef<CGlWidget> TWidgetRef;
    typedef vector<TWidgetRef> TWidgetList;

public:
    CGlContainer();
    virtual ~CGlContainer();

    virtual void Draw(void);
    virtual void ProcessEvent(CGlEvent& event);

    virtual void Add(CGlWidget* widget);
    virtual void Remove(CGlWidget* widget);
    virtual void Clear(void);
    virtual size_t Size(void) const;

    typedef TWidgetList::iterator iterator;
    typedef TWidgetList::const_iterator const_iterator;

    iterator begin(void);
    const_iterator begin(void) const;
    iterator end(void);
    const_iterator end(void) const;

private:
    TWidgetList m_Children;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_CONTAINER__HPP
