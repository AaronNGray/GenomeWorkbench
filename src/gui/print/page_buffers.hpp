#ifndef GUI_UTILS__PAGE_BUFFERS_HPP
#define GUI_UTILS__PAGE_BUFFERS_HPP

/*  $Id: page_buffers.hpp 27010 2012-12-07 16:37:16Z falkrb $
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
 *   CPageBuffers - holds buffers and information for a logical page
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/utils/bbox.hpp>
#include <gui/print/print_options.hpp>
#include <gui/print/pdf_object.hpp>


BEGIN_NCBI_SCOPE



class CPBuffer;


class CPageBuffers
{
public:
    typedef vector<const CPBuffer*> TBufferList;
    typedef pair<unsigned int, unsigned int> TOffset;


    CPageBuffers();
    virtual ~CPageBuffers();

    virtual void Add(const CPBuffer* buf);
    virtual void Clear(void);
    virtual bool Empty(void) const;
    CBBox<3> GetBoundingBox() const;

    typedef TBufferList::iterator iterator;
    typedef TBufferList::const_iterator const_iterator;
    typedef TBufferList::reverse_iterator reverse_iterator;
    typedef TBufferList::const_reverse_iterator const_reverse_iterator;

    iterator begin(void);
    const_iterator begin(void) const;
    iterator end(void);
    const_iterator end(void) const;
    reverse_iterator rbegin(void);
    const_reverse_iterator rbegin(void) const;
    reverse_iterator rend(void);
    const_reverse_iterator rend(void) const;

    //void SetPageSettings(const CPageSettings& settings);
    //CPageSettings GetPageSettings(void) const;

    void SetGlobalXFormObj(CRef<CPdfObject> obj);
    CRef<CPdfObject> GetGlobalXFormObj(void) const;

    TOffset GetPageSize(void) const;

private:
    TBufferList m_Buffers;
    CBBox<3> m_BoundingBox;
};



END_NCBI_SCOPE


#endif // GUI_UTILS__PAGE_BUFFERS_HPP
