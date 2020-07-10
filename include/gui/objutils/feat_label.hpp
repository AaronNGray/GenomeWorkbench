#error "Don't include me"
#ifndef GUI_OBJUTILS___FEAT_LABEL__HPP
#define GUI_OBJUTILS___FEAT_LABEL__HPP

/*  $Id: feat_label.hpp 20325 2009-11-06 19:46:40Z wuliangs $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *   CLayoutFeatLabel -- utility class to layout feature labels
 *
 */


#include <corelib/ncbiobj.hpp>
#include <gui/objutils/feature.hpp>
#include <objects/seqloc/Seq_interval.hpp>

/** @addtogroup GUI_OBJUTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE


class NCBI_GUIOBJUTILS_EXPORT CLayoutFeatLabel : public CLayoutFeat
{
public:
    // ctor
    CLayoutFeatLabel(const objects::CMappedFeat& feat);

    int  GetColumn(void) const { return column; }
    void SetColumn(int col)    { column = col;  }

    void SetRange(TSeqPos from, TSeqPos to);

    virtual CConstRef<CObject> GetObject(TSeqPos pos) const;
    virtual void  GetObjects(vector<CConstRef<CObject> >& objs) const;

    // access the position of this object.
    const objects::CSeq_loc& GetLocation(void) const;

    // access the position of this object.
    TSeqRange GetRange(void) const;

    // retrieve the type of this object
    EType GetType() const;

private:

    // column in grid layout on the screen
    int column;

    // there is no real location for feature labels
    // layout is done into columns and not location.
    CRef<objects::CSeq_loc> m_Location;

};



END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___FEAT_LABEL__HPP
