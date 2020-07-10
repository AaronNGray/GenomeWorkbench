#ifndef GUI_WIDGETS_SEQ_GRAPHIC___MATE_PAIR_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___MATE_PAIR_GLYPH__HPP

/*  $Id: mate_pair_glyph.hpp 39020 2017-07-20 15:44:33Z shkeda $
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
 *   CMatePairGlyph -- utility class to layout mate pairs (a special type of
 *      pairwise alignments and hold a set of CAlignGlyph
 */


#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/widgets/seq_graphic/alignment_glyph.hpp>
#include <gui/widgets/seq_graphic/object_based_glyph.hpp>
#include <gui/widgets/seq_graphic/alignment_conf.hpp>
#include <objects/seqalign/Seq_align.hpp>


BEGIN_NCBI_SCOPE


class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CMatePairGlyph 
    : public CSeqGlyph
    , public IObjectBasedGlyph
{
public:
    typedef vector< CRef<CAlignGlyph> > TAlignList;

    // ctors
    CMatePairGlyph(const TAlignList& aligns);

    /// @name CSeqGlyph methods
    /// {
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual TSeqRange   GetRange(void) const;
    virtual bool IsClickable() const;
    /// }

    /// @name IObjectBasedGlyph implementation.
    /// @{
    virtual const objects::CSeq_loc& GetLocation(void) const;
    virtual CConstRef<CObject> GetObject(TSeqPos pos) const;
    virtual void GetObjects(vector<CConstRef<CObject> >& objs) const;
    virtual bool HasObject(CConstRef<CObject> obj) const;
    virtual string GetSignature() const;
    virtual const TIntervals& GetIntervals(void) const;
    /// @}

    bool IsPairSelected(const CAlignGlyph* pw_aln) const;


    //TSeqRange                GetRange(void) const;
    const TAlignList&        GetSeqAligns() const;
    TAlignList&              SetSeqAligns();

    // library information
    enum ELibraryId {
        eLibrary_NotSet = -1,
        eLibrary_NotFound = 0
    };
    size_t GetLibraryId() const;

    // matepair errors
    enum EErrorType {

        // no error found (initial state)
        eError_NotSet = -1,

        // no error
        eError_NoError = 0,

        // the mate is not aligned with the correct otientation
        eError_Orientation = 1,

        // the mate aligns to multiple locations
        eError_NonUnique = 2,

        // the mate is not in the expected distance range
        eError_Distance = 3,

        // co-alignment
        eError_CoAlign = 4,

        // contra-alignment
        eError_ContraAlign = 5

    };

    EErrorType GetError() const;

    void SetConfig(CConstRef<CMatePairConfig> conf);

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

private:
    void x_GetTitle(string* title, CLabel::ELabelType type) const;

protected:
    mutable TAlignList              m_SeqAligns;
    CRef<objects::CSeq_loc> m_Location;

    // parsed matepair library (-1 = not set, 0 = not found)
    int m_LibraryId;

    // parsed mate pair error code
    EErrorType m_ErrorType;

    TIntervals  m_Intervals;

    // store the selected object (if any)
    mutable vector<CConstRef<CObject> > m_ObjSel;

    CConstRef<CMatePairConfig>          m_Config;
};


///////////////////////////////////////////////////////////////////////////////
/// inline methods
///
inline
const CMatePairGlyph::TAlignList& CMatePairGlyph::GetSeqAligns(void) const
{
    return m_SeqAligns;
}

inline
CMatePairGlyph::TAlignList& CMatePairGlyph::SetSeqAligns()
{
    return m_SeqAligns;
}

inline
size_t CMatePairGlyph::GetLibraryId() const
{
    return m_LibraryId;
}

inline
CMatePairGlyph::EErrorType CMatePairGlyph::GetError() const
{
    return m_ErrorType;
}

inline
void CMatePairGlyph::SetConfig(CConstRef<CMatePairConfig> conf)
{
    m_Config = conf;
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___MATE_PAIR_GLYPH__HPP
