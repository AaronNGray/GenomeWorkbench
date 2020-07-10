#ifndef GUI_OBJUTILS___ALIGNMENT_SMEAR__HPP
#define GUI_OBJUTILS___ALIGNMENT_SMEAR__HPP

/*  $Id: alignment_smear.hpp 20440 2009-11-25 16:13:30Z wuliangs $
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
 * Authors:  Robert G. Smith, Liangshou Wu
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/objutils/density_map.hpp>

/** @addtogroup GUI_OBJUTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CBioseq_Handle;
    class CAlnMap;
    class CSeq_annot;
    class CAlign_CI;
    struct SAnnotSelector;
END_SCOPE(objects)

class ISeqTaskProgressCallback;
class CSparseAln;

class NCBI_GUIOBJUTILS_EXPORT CAlignmentSmear : public CObject
{
public:
    enum EAlignSmearStrand {
        eSmearStrand_Both,
        eSmearStrand_Pos,
        eSmearStrand_Neg
    };

    typedef int     score_type;
    typedef CDenMapRunIterator<score_type>  runlen_iterator;

    CAlignmentSmear(
        const objects::CBioseq_Handle& handle,
        TSeqPos start,
        TSeqPos stop,
        float window = 1,
        EAlignSmearStrand strand_type = eSmearStrand_Both
        );

/// INPUT:
    /// smear all the alignments in this annotation.
    void AddAnnot(const objects::CSeq_annot& seq_annot,
        ISeqTaskProgressCallback* p_cb = NULL);

    /// Smear all the alignments matched by this selector on my bioseq.
    void AddAlignments(const objects::SAnnotSelector& sel,
        ISeqTaskProgressCallback* p_cb = NULL);

    /// Smear all the alignments in CAlign_CI.
    bool AddAlignments(objects::CAlign_CI& align_iter,
        ISeqTaskProgressCallback* p_cb = NULL);

    /// Add this one alignment to my smear.
    void AddAlignment(const CSparseAln& aln);

    /// Add this one alignment to my smear.
    void AddAlignment(const objects::CAlnMap& align);


    void MaskGaps();

    string      GetLabel() const;
    void        SetLabel(const string& label);

    EAlignSmearStrand   GetStrandType() const { return m_StrandType; }

    static bool SeparateStrands(const objects::CSeq_annot& seq_annot);

/// OUTPUT:
    runlen_iterator  SmearSegBegin() const
        { return m_AccumSeg.RunLenBegin(); }
    runlen_iterator  SmearGapBegin() const
        { return m_AccumGap.RunLenBegin(); }

    score_type         GetMaxValue() const { return m_AccumSeg.GetMax(); }

private:

    static string   x_GetAnnotName(const objects::CSeq_annot& seq_annot);

    const objects::CBioseq_Handle&  m_BioseqHandle;

    typedef CDensityMap<score_type> TSegMap;
    TSegMap     m_AccumSeg;

    typedef CDensityMap<score_type>  TGapMap;
    TGapMap     m_AccumGap;

    EAlignSmearStrand   m_StrandType;
    string      m_Label;
    unsigned long   m_CntAligns;
    unsigned long   m_CntMixes;
};



END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___ALIGNMENT_SMEAR__HPP
