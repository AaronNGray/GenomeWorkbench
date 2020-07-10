#ifndef GUI_OBJUTILS___VIS_RANGE_GLYPH__HPP
#define GUI_OBJUTILS___VIS_RANGE_GLYPH__HPP

/*  $Id: vis_range_glyph.hpp 35413 2016-05-04 17:51:26Z evgeniev $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *   CVisRangeGlyph -- utility class to layout visible ranges of the
 *                     sibling views
 */


#include <corelib/ncbiobj.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_loc.hpp>

BEGIN_NCBI_SCOPE

class CEvtHandler;
class ITooltipFormatter;

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CVisRangeGlyph : public CSeqGlyph
{
public:
    // ctors
    CVisRangeGlyph(const CEventHandler* target, const list<TSeqRange>& vis_ranges,
            const CRgbaColor& color, const string& label);

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual TSeqRange GetRange(void) const;
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual bool IsClickable() const;
    ///@}

    const CEventHandler*      GetTarget(void) const;
    const objects::CSeq_loc&  GetLocation(void) const;
    const CRgbaColor& GetColor() const;
    string GetLabel() const;

protected:
    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();
    /// @}

private:
    const CEventHandler*    m_Target;
    CRef<objects::CSeq_loc> m_Location;
    CRgbaColor              m_Color;
    string                  m_Label;
};


///////////////////////////////////////////////////////////////////////////////
/// CVisRangeGlyph inline methods
///
inline
const CEventHandler* CVisRangeGlyph::GetTarget(void) const
{
    return m_Target;
}

inline
const objects::CSeq_loc& CVisRangeGlyph::GetLocation(void) const
{
    return *m_Location;
}

inline
const CRgbaColor& CVisRangeGlyph::GetColor() const
{
    return m_Color;
}

inline
string CVisRangeGlyph::GetLabel() const
{
    return m_Label;
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___VIS_RANGE_GLYPH__HPP
