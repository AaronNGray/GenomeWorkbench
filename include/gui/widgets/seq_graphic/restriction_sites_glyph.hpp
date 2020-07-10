#ifndef GUI_WIDGETS_SEQ_GRAPHIC___RESTRICTION_SITES_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___RESTRICTION_SITES_GLYPH__HPP

/*  $Id: restriction_sites_glyph.hpp 35413 2016-05-04 17:51:26Z evgeniev $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 */

#include <gui/gui.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <corelib/ncbiobj.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// A glyph representing a list of restriction sites stored in one seq-feat.
/// It presumes the seq-feat is indeed a rsite feature (no validation), and
/// the feature contain a packed-int seq-location.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CRsitesGlyph :
    public CSeqGlyph
{
public:
    /// @name ctors.
    /// @{
    CRsitesGlyph(const objects::CSeq_feat& feat);
    /// @}

    /// @name CSeqGlyph virtual method implementation.
    /// @{
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual TSeqRange GetRange(void) const;
    /// @}

    void SetSiteWidth(int w);
    void SetSiteHeight(int h);
    void SetSiteColor(const CRgbaColor& color);

    const objects::CSeq_feat& GetFeature(void) const;
    string GetLabel() const;
    bool HasVisibleRsite() const;

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

private:
    CConstRef<objects::CSeq_feat> m_Feature;
    int        m_SiteW;
    int        m_SiteH;
    CRgbaColor m_Color;
};



///////////////////////////////////////////////////////////////////////////////
/// CRsitesGlyph inline implementations
inline
void CRsitesGlyph::SetSiteWidth(int w)
{ m_SiteW = w; }

inline
void CRsitesGlyph::SetSiteHeight(int h)
{ m_SiteH = h; }

inline
void CRsitesGlyph::SetSiteColor(const CRgbaColor& color)
{ m_Color = color; }

inline
const objects::CSeq_feat& CRsitesGlyph::GetFeature() const
{ return *m_Feature; }


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___RESTRICTION_SITES_GLYPH__HPP
