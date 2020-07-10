#ifndef GUI_WIDGETS_SEQ_GRAPHIC___OBEJCT_BASED_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___OBEJCT_BASED_GLYPH__HPP

/* $Id: object_based_glyph.hpp 19580 2009-07-13 12:28:06Z wuliangs $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
 */

#include <corelib/ncbistr.hpp>
#include <gui/gui_export.h>
#include <gui/opengl/gltypes.hpp>
#include <objects/seqloc/Seq_loc.hpp>


BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT IObjectBasedGlyph
{
public:
    typedef vector<TSeqRange> TIntervals;

    virtual ~IObjectBasedGlyph() {}

    /// access the position of this object.
    virtual const objects::CSeq_loc& GetLocation(void) const = 0;

    /// access our core component - we wrap an object(s) of some sort.
    /// This returns the object at a given sequence position; this is useful if
    /// the layout object wraps more than one object
    virtual CConstRef<CObject> GetObject(TSeqPos pos) const = 0;

    /// retrieve CObjects corresponding to this CSeqGlyph.
    virtual void GetObjects(vector<CConstRef<CObject> >& objs) const = 0;

    /// check if the wrapped object(s) is the one.
    virtual bool HasObject(CConstRef<CObject> obj) const = 0;

    /// return signature for this glyph.
    virtual string GetSignature() const = 0;

    /// access sub-intervals (if any).
    virtual const TIntervals& GetIntervals(void) const = 0;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___OBEJCT_BASED_GLYPH__HPP
