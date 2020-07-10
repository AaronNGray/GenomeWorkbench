#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_SMEAR_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_SMEAR_GLYPH__HPP

/*  $Id: alignment_smear_glyph.hpp 35413 2016-05-04 17:51:26Z evgeniev $
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
 *   CAlignSmearGlyph -- utility class to layout mate pairs (a special type of
 *      pairwise alignments and hold a set of CAlignGlyph
 */


#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/objutils/alignment_smear.hpp>
#include <gui/widgets/seq_graphic/alignment_conf.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <objects/seqalign/Seq_align.hpp>


BEGIN_NCBI_SCOPE


class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAlignSmearGlyph 
    : public CSeqGlyph
{
public:
    // ctors
    CAlignSmearGlyph(
        const objects::CBioseq_Handle& handle,
        TSeqPos start, TSeqPos stop, float window,
        CAlignmentSmear::EAlignSmearStrand strand_type);

    CAlignSmearGlyph(const objects::CBioseq_Handle& handle,
        TSeqPos start, TSeqPos stop, float window,
        CAlignmentSmear::EAlignSmearStrand strand_type,
        const objects::CSeq_annot& seq_annot);


    /// @name CSeqGlyph methods
    /// {
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual bool IsClickable() const;
    /// }

    // Get our native data.
    const CAlignmentSmear& GetAlignSmear() const;
    CAlignmentSmear& GetAlignSmear();

    void SetConfig(CConstRef<CAlignmentConfig> conf);

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

protected:
    CRef<CAlignmentSmear>       m_AlignSmear;
    CConstRef<CAlignmentConfig> m_Config;
};


///////////////////////////////////////////////////////////////////////////////
/// inline methods
inline
void CAlignSmearGlyph::SetConfig(CConstRef<CAlignmentConfig> conf)
{
    m_Config = conf;
}

inline
const CAlignmentSmear& CAlignSmearGlyph::GetAlignSmear() const
{
    return *m_AlignSmear;
}

inline
CAlignmentSmear& CAlignSmearGlyph::GetAlignSmear()
{
    return *m_AlignSmear;
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_SMEAR_GLYPH__HPP
