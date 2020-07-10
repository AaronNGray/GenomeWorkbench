#ifndef GUI_PACKAGES_SNP_BINS_TRACK__GMARK_GLYPH__HPP
#define GUI_PACKAGES_SNP_BINS_TRACK__GMARK_GLYPH__HPP

/*  $Id: gmark_glyph.hpp 35413 2016-05-04 17:51:26Z evgeniev $
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
 * Authors:  Dmitry Rudnev
 *
 * File Description:
 *    CGeneMarkerGlyph -- Glyph that represents Gene markers encoded similar to SNP bins
 *
 */


#include <corelib/ncbiobj.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/scope.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/density_map.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <objtools/snputil/snp_bins.hpp>
#include <objtools/snputil/snp_utils.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_SNP_EXPORT CGeneMarkerGlyph : public CSeqGlyph
{
////////////////////////////////////////////
// Public Structs/Classes/Typedefs
////////////////////////////////////////////
public:

    typedef  list<CRef<objects::NSnpBins::SBin> > TGeneMarkers;


//////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////
public:

    /// @name Static Methods
    /// @{
    static string GenerateGeneMarkerSignature(const string& title,
                                       const string& annot_name,
                                       TGi gi,
                                       TSeqRange range,
                                       const string& trackSubType);
    /// @}

    /// @name ctors
    /// @{

    CGeneMarkerGlyph(const TGeneMarkers&    values,
               const objects::CSeq_loc& loc,
               objects::CScope *        scope);
    /// @}

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual bool OnLeftDblClick(const TModelPoint& p);
    virtual bool IsClickable() const;

    virtual TSeqRange GetRange(void) const;
    ///@}

    void SetTitle(const string& title);
    const string& GetTitle() const;

    void SetAnnotName(const string& name);
    const string& GetAnnotName() const;


//////////////////////////////////////////////
// Protected Methods
//////////////////////////////////////////////
protected:
    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();
    ///@}


//////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////
private:
    // this creates the Gene marker signatures in m_GeneMarkers.  It is labeled 'const'
    //   but modifies the mutable m_GeneMarkers and m_bHasSignatures
    void        x_BuildSignatures() const;
    string      x_GetColorTheme() const;

private:
    string                          m_Title;
    string                          m_AnnotName;
    CConstRef<objects::CSeq_loc>    m_Location;
    mutable TGeneMarkers            m_GeneMarkers;

    mutable bool                    m_bHasSignatures;

    mutable CRef<objects::CScope>   m_Scope;
};

///////////////////////////////////////////////////////////////////////////////
/// CGeneMarkerGlyph inline method implementation.

inline
void CGeneMarkerGlyph::SetTitle(const string& title)
{ m_Title = title; }

inline
const string& CGeneMarkerGlyph::GetTitle() const
{ return m_Title; }

inline
void CGeneMarkerGlyph::SetAnnotName(const string& name)
{ m_AnnotName = name; }

inline
const string& CGeneMarkerGlyph::GetAnnotName() const
{ return m_AnnotName; }


END_NCBI_SCOPE

/* @} */

#endif  // GUI_PACKAGES_SNP_BINS_TRACK__GMARK_GLYPH__HPP
