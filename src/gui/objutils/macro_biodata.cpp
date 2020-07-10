/*  $Id: macro_biodata.cpp 45066 2020-05-20 15:52:19Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <gui/objutils/macro_biodata.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

const char* CMacroBioData::sm_Seq = "Seq";
const char* CMacroBioData::sm_SeqNa = "SeqNA";
const char* CMacroBioData::sm_SeqAa = "SeqAA";
const char* CMacroBioData::sm_SeqSet = "SeqSet";

const char* CMacroBioData::sm_Seqdesc = "Seqdesc";
const char* CMacroBioData::sm_BioSource = "BioSource";
const char* CMacroBioData::sm_MolInfo = "MolInfo";
const char* CMacroBioData::sm_Pubdesc = "Pubdesc";
const char* CMacroBioData::sm_UserObject = "UserObject";
const char* CMacroBioData::sm_DBLink = "DBLink";
const char* CMacroBioData::sm_StrComm = "StructComment";

const char* CMacroBioData::sm_SeqFeat = "SeqFeat";
const char* CMacroBioData::sm_Gene = "Gene";
const char* CMacroBioData::sm_CdRegion = "Cdregion";
const char* CMacroBioData::sm_Protein = "Protein";
const char* CMacroBioData::sm_Miscfeat = "MiscFeat";
const char* CMacroBioData::sm_ImpFeat = "ImpFeat";
const char* CMacroBioData::sm_RNA = "RNA";
const char* CMacroBioData::sm_rRNA = "rRNA";
const char* CMacroBioData::sm_mRNA = "mRNA";
const char* CMacroBioData::sm_miscRNA = "MiscRNA";
const char* CMacroBioData::sm_SNP = "SNP";
const char* CMacroBioData::sm_SeqAlign = "SeqAlign";
const char* CMacroBioData::sm_TSEntry = "TSEntry";

CMacroBioData::CMacroBioData(const CSeq_entry_Handle& entry, CConstRef<objects::CSeq_submit> submit)
    : m_TopSeqEntry(entry), m_SeqSubmit(submit)
{
}

bool CMacroBioData::s_IsFeatSelector(const string& selector)
{
    static const vector<const char*> s_FeatSelectors { 
       sm_Gene,
       sm_CdRegion,
       sm_Protein,
       sm_SeqFeat,
       sm_SNP,
       sm_mRNA,
       sm_miscRNA,
       sm_rRNA,
       sm_RNA,
       sm_Miscfeat,
       sm_ImpFeat
    };

    auto it = find_if( s_FeatSelectors.begin(), s_FeatSelectors.end(), 
        [&selector](const char* elem) { return NStr::EqualNocase(selector, elem); } );
    return (it != s_FeatSelectors.end());
}

IMacroBioDataIter* CMacroBioData::CreateIterator(const string& selector,
    const string& named_annot, const TSeqRange& range, CNcbiOstream* ostr) const
{
    IMacroBioDataIter* it = nullptr;

    if (s_IsFeatSelector(selector)) {
        it = x_CreateFeatIterator(selector, named_annot, range);
    }
    else {
        it = x_CreateNonFeatIterator(selector);
    }

    if (it) {
        if (IsSetSeqSubmit()) {
            it->SetSeqSubmit(m_SeqSubmit.GetObject());
        }
        if (ostr) {
            it->SetOutputStream(ostr);
        }
    }
    return it;
}

IMacroBioDataIter* CMacroBioData::CreateIterator(const string& selector,
    const string& named_annot, const SFeatInterval& feat_interval, CNcbiOstream* ostr) const
{
    IMacroBioDataIter* it = nullptr;

    if (s_IsFeatSelector(selector)) {
        it = x_CreateFeatIterator(selector, named_annot, feat_interval);
    }
    else {
        it = x_CreateNonFeatIterator(selector);
    }
    
    if (it) {
        if (IsSetSeqSubmit()) {
            it->SetSeqSubmit(m_SeqSubmit.GetObject());
        }
        if (ostr) {
            it->SetOutputStream(ostr);
        }
    }
    return it;
}


IMacroBioDataIter* CMacroBioData::x_CreateNonFeatIterator(const string& selector) const
{
    IMacroBioDataIter* it = nullptr;

    if (NStr::EqualNocase(selector, sm_BioSource))
        it = new CMacroBioData_BioSourceIter(m_TopSeqEntry);
    else if (NStr::EqualNocase(selector, sm_MolInfo))
        it = new CMacroBioData_MolInfoIter(m_TopSeqEntry);
    else if (NStr::EqualNocase(selector, sm_Pubdesc))
        it = new CMacroBioData_PubdescIter(m_TopSeqEntry);
    else if (NStr::EqualNocase(selector, sm_UserObject))
        it = new CMacroBioData_UserObjectIter(m_TopSeqEntry);
    else if (NStr::EqualNocase(selector, sm_StrComm))
        it = new CMacroBioData_StructCommentIter(m_TopSeqEntry);
    else if (NStr::EqualNocase(selector, sm_DBLink))
        it = new CMacroBioData_DBLinkIter(m_TopSeqEntry);
    else if (NStr::EqualNocase(selector, sm_Seqdesc))
        it = new CMacroBioData_SeqdescIter(m_TopSeqEntry);
    else if (NStr::EqualNocase(selector, sm_SeqAlign))
        it = new CMacroBioData_SeqAlignIter(m_TopSeqEntry);
    else if (NStr::EqualNocase(selector, sm_SeqNa))
        it = CMacroBioData_SeqIter::s_MakeSeqIterator(m_TopSeqEntry, CSeq_inst::eMol_na);
    else if (NStr::EqualNocase(selector, sm_SeqAa))
        it = CMacroBioData_SeqIter::s_MakeSeqIterator(m_TopSeqEntry, CSeq_inst::eMol_aa);
    else if (NStr::EqualNocase(selector, sm_Seq))
        it = CMacroBioData_SeqIter::s_MakeSeqIterator(m_TopSeqEntry, CSeq_inst::eMol_not_set);
    else if (NStr::EqualNocase(selector, sm_SeqSet))
        it = new CMacroBioData_SeqSetIter(m_TopSeqEntry);
    else if (NStr::EqualNocase(selector, sm_TSEntry))
        it = new CMacroBioData_TSEntryIter(m_TopSeqEntry);

    return it;
}

IMacroBioDataIter* CMacroBioData::x_CreateFeatIterator(const string& selector,
    const string& named_annot, const TSeqRange& range) const
{
    IMacroBioDataIter* it = nullptr;

    SAnnotSelector sel = x_GetAnnotSelector(selector, named_annot);
    _ASSERT(m_TopSeqEntry);
    if (m_TopSeqEntry.IsSeq()) {
        CBioseq_Handle bsh = m_TopSeqEntry.GetSeq();

        try {
            CRef<CSeq_loc> loc = bsh.GetRangeSeq_loc(range.GetFrom(), range.GetTo());
            if (loc) {
                it = new CMacroBioData_FeatIterBase(bsh, sel, *loc);
            }
        }
        catch (const CMacroExecException&) {
            it = nullptr;
        }
    }
    else {
        it = new CMacroBioData_FeatIterBase(m_TopSeqEntry, sel);
    }

    return it;
}

IMacroBioDataIter* CMacroBioData::x_CreateFeatIterator(const string& selector, 
    const string& named_annot, const SFeatInterval& feat_interval) const
{
    IMacroBioDataIter* it = nullptr;

    SAnnotSelector sel = x_GetAnnotSelector(selector, named_annot);
    _ASSERT(m_TopSeqEntry);
    if (m_TopSeqEntry.IsSeq()) {
        CBioseq_Handle bsh = m_TopSeqEntry.GetSeq();

        try {
            if ((feat_interval.left_closed == feat_interval.right_closed) &&
                (feat_interval.left_closed == true)) {
                CRef<CSeq_loc> loc = bsh.GetRangeSeq_loc(feat_interval.m_Range.GetFrom(), feat_interval.m_Range.GetTo());
                if (loc) {
                    it = new CMacroBioData_FeatIterBase(bsh, sel, *loc);
                }
            }
            else {
                it = new CMacroBioData_FeatIntervalIter(bsh, sel, feat_interval);
            }
        }
        catch (const CMacroExecException&) {
            it = nullptr;
        }
    }
    else {
        it = new CMacroBioData_FeatIterBase(m_TopSeqEntry, sel);
    }

    return it;
}

SAnnotSelector CMacroBioData::x_GetAnnotSelector(const string& selector, const string& named_annot) const
{
    SAnnotSelector sel;
    if (!named_annot.empty()) {
        // consider overlaps by total range
        sel.SetOverlapTotalRange();
        // resolve all segments
        sel.SetResolveAll();
        sel.SetAdaptiveDepth(true);
        
        sel.IncludeNamedAnnotAccession(named_annot);
        sel.AddNamedAnnots(named_annot);
    }

    if (NStr::EqualNocase(selector, sm_Gene))
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_gene);
    else if (NStr::EqualNocase(selector, sm_CdRegion))
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_cdregion);
    else if (NStr::EqualNocase(selector, sm_Protein))
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_prot);
    else if (NStr::EqualNocase(selector, sm_Miscfeat))
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_misc_feature);
    else if (NStr::EqualNocase(selector, sm_ImpFeat))
        sel.SetFeatType(CSeqFeatData::e_Imp);
    else if (NStr::EqualNocase(selector, sm_RNA))
        sel.SetFeatType(CSeqFeatData::e_Rna);
    else if (NStr::EqualNocase(selector, sm_rRNA))
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_rRNA);
    else if (NStr::EqualNocase(selector, sm_mRNA))
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_mRNA);
    else if (NStr::EqualNocase(selector, sm_miscRNA))
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_otherRNA);
    else if (NStr::EqualNocase(selector, sm_SNP))
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_variation);
    else if (NStr::EqualNocase(selector, sm_SeqFeat))
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_any);

    return sel;
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

