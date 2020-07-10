#ifndef GUI_WIDGETS_SEQ_GRAPHIC___CDS_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___CDS_GLYPH__HPP

/*  $Id: cds_glyph.hpp 40489 2018-02-26 20:36:05Z evgeniev $
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

#include <gui/widgets/seq_graphic/feature_glyph.hpp>
#include <gui/opengl/gltexturefont.hpp>

BEGIN_NCBI_SCOPE

class CCdsConfig;

///////////////////////////////////////////////////////////////////////////////
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CCdsGlyph : public CFeatGlyph
{
public:
    /// @name ctors.
    /// @{
    CCdsGlyph(const objects::CMappedFeat& feat);
    CCdsGlyph(const objects::CMappedFeat& feat, const objects::CSeq_loc& loc);
    /// @}

    void SetCdsConfig(CCdsConfig* config);

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

private:
    void x_DrawProtSeqWithMapping(TModelUnit trans_y,
        TModelUnit gen_y, TModelUnit prot_y) const;
    void x_DrawProjectedProtSeq(TModelUnit trans_y,
                                TModelUnit gen_y, TModelUnit prot_y) const;

    void x_PutCodon(const string& codon_str, TSeqPos pos,
        TModelUnit y, bool neg, bool append) const;
   
    bool x_WillFit() const;

protected:
    CRef<CCdsConfig> m_CdsConfig;
};

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CCdsConfig : public CObject
{
public:
    CCdsConfig() : m_Dirty(false) {}

    /// @name colors.
    /// @{
    CRgbaColor  m_bgProtProd;   ///< protein product background color.
    CRgbaColor  m_fgProtProd;   ///< protein product frorground color.
    CRgbaColor  m_LabelProtProd;///< protein product label color.
    CRgbaColor  m_SeqProt;      ///< sequence color.
    CRgbaColor  m_SeqProtMismatch;  ///< mismatched sequence color.
    CRgbaColor  m_SeqProtTrans;     ///< translated sequence color.
    /// @}

    /// @name fonts.
    /// @{
    CGlTextureFont m_ProdFont;
    CGlTextureFont m_TransFont;
    /// @}

    mutable bool   m_Dirty;     ///< dirty flag indicating any setting changed
};

///////////////////////////////////////////////////////////////////////////////
/// CCdsGlyph inline methods

inline
void CCdsGlyph::SetCdsConfig(CCdsConfig* config)
{
    m_CdsConfig.Reset(config);
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___CDS_GLYPH__HPP
