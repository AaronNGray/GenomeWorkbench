/*  $Id: desktop_typed_items.cpp 39282 2017-09-01 20:08:59Z asztalos $
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
#include <serial/typeinfo.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/misc/sequence_util_macros.hpp>
#include <objtools/validator/utilities.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/align_ci.hpp>

#include <gui/widgets/seq_desktop/desktop_typed_items.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CDesktopBioseqItem::CDesktopBioseqItem(const CBioseq_Handle& bsh)
    : m_Bsh(bsh)
{
    _ASSERT(m_Bsh);
    m_Color = *wxRED;
    if (m_Bsh.IsAa()) {
        m_Color.Set(255, 0, 255);
    }
}

static const CTempString kSpaces("     ");

IDesktopDataItem::TLines CDesktopBioseqItem::GetDescription(bool root) const
{
    string descr;
    if (m_Bsh.IsSetInst()) {
        const CSeq_inst& inst = m_Bsh.GetInst();
        descr = CSeq_inst::ENUM_METHOD_NAME(ERepr)()->FindName(inst.GetRepr(), true);
        descr += ", " + CSeq_inst::ENUM_METHOD_NAME(EMol)()->FindName(inst.GetMol(), true);
        descr += ", length = " + NStr::NumericToString(inst.GetLength());
    }

    CSeq_id_Handle best_idh = sequence::GetId(m_Bsh, sequence::eGetId_Best);
    string name;
    best_idh.GetSeqId()->GetLabel(&name, CSeq_id::eBoth);

    TLines lines;
    if (root) {
        lines.push_back(kSpaces + "Bioseq: " + descr);
        lines.push_back(kSpaces + name);
    }
    else {
        lines.push_back(kSpaces + name);
        lines.push_back(kSpaces + kSpaces + descr);
    }

    return lines;
}


const string& CDesktopBioseqItem::GetType() const
{
    return CBioseq::GetTypeInfo()->GetName();
}

const CObject* CDesktopBioseqItem::GetObject(void) const
{
    CConstRef<CBioseq> bseq = m_Bsh.GetCompleteBioseq();
    return bseq.GetPointerOrNull();
}

CSeq_entry_Handle CDesktopBioseqItem::GetSeqentryHandle() const
{
    return m_Bsh.GetSeq_entry_Handle();
}

// CDesktopBioseqsetItem

CDesktopBioseqsetItem::CDesktopBioseqsetItem(const CBioseq_set_Handle& bssh)
    : m_Bssh(bssh)
{
    _ASSERT(m_Bssh);
    x_SetBrush();
}

IDesktopDataItem::TLines CDesktopBioseqsetItem::GetDescription(bool root) const
{
    CBioseq_set::EClass bioseq_set_class = 
        GET_FIELD_OR_DEFAULT(*m_Bssh.GetCompleteBioseq_set(), Class, CBioseq_set::eClass_not_set);
    const string classString = CBioseq_set::ENUM_METHOD_NAME(EClass)()->FindName(bioseq_set_class, true);

    if (!root) {
        return TLines{ kSpaces + classString };
    }

    // get first bioseq within the set
    CBioseq_CI b_iter(m_Bssh);
    string name;
    if (b_iter) {
        CSeq_id_Handle best_idh = sequence::GetId(*b_iter, sequence::eGetId_Best);
        best_idh.GetSeqId()->GetLabel(&name, CSeq_id::eBoth);
    }
    else {
        name.assign("empty bioseqset");
    }

    TLines lines;
    lines.push_back(kSpaces + "BioseqSet: " + classString);
    lines.push_back(kSpaces + name);
    return lines;
}

const string& CDesktopBioseqsetItem::GetType() const
{
    return CBioseq_set::GetTypeInfo()->GetName();
}

const CObject* CDesktopBioseqsetItem::GetObject(void) const
{
    CConstRef<CBioseq_set> bset = m_Bssh.GetCompleteBioseq_set();
    return bset.GetPointerOrNull();
}

CSeq_entry_Handle CDesktopBioseqsetItem::GetSeqentryHandle() const
{
    return m_Bssh.GetParentEntry();
}

void CDesktopBioseqsetItem::x_SetBrush()
{
    m_BkgdBrush = *wxTRANSPARENT_BRUSH;
    int col = 255;
    switch (m_Bssh.GetClass()) {
    case CBioseq_set::eClass_genbank:
        col = 158;
        break;
    case CBioseq_set::eClass_nuc_prot:
        col = 195;
        break;
    case CBioseq_set::eClass_other:
        col = 210;
        break;
    default:
        col = 162 + (int)(m_Bssh.GetClass());
        break;
    }
    m_BkgdBrush.SetColour(wxColour(col, col, col));
    m_BkgdBrush.SetStyle(wxBRUSHSTYLE_SOLID);
}

// CDesktopSeqSubmitItem

IDesktopDataItem::TLines CDesktopSeqSubmitItem::GetDescription(bool root) const
{
    string label = kSpaces + "SeqSubmit: ";
    if (m_Submit->IsEntrys()) {
        label += "Entries ";
    }
    else if (m_Submit->IsAnnots()) {
        label += "Annotations ";
    }
    else if (m_Submit->IsDelete()) {
        label += "Deletions ";
    }
    else {
        label += "Not Set ";
    }

    TLines lines;
    lines.push_back(label);

    if (m_Submit->IsSetSub()) {
        if (m_Submit->GetSub().IsSetContact()) {
            if (m_Submit->GetSub().GetContact().IsSetContact()) {
                const CAuthor& author = m_Submit->GetSub().GetContact().GetContact();
                if (author.IsSetName()) {
                    string name;
                    author.GetName().GetLabel(&name);
                    lines.push_back(kSpaces + name);
                }
            }
        }
    }
    return lines;
}

const string& CDesktopSeqSubmitItem::GetType() const
{
    return CSeq_submit::GetTypeInfo()->GetName();
}

const CObject* CDesktopSeqSubmitItem::GetObject(void) const
{
    return m_Submit.GetPointerOrNull();
}

CSeq_entry_Handle CDesktopSeqSubmitItem::GetSeqentryHandle() const
{
    if (m_Submit->IsEntrys()) {
        const CSeq_submit::TData::TEntrys& entries = m_Submit->GetData().GetEntrys();
        if (!entries.empty()) {
            return m_Scope->GetSeq_entryHandle(**entries.begin());
        }
    }
    return CSeq_entry_Handle();
}

// CDesktopContactInfoItem

IDesktopDataItem::TLines CDesktopContactInfoItem::GetDescription(bool root) const
{
    string label;
    if (m_Contact->IsSetContact()) {
        const CAuthor& author = m_Contact->GetContact();
        if (author.IsSetName()) {
            author.GetName().GetLabel(&label);
        }
    }
    return TLines{ kSpaces + "Contact Info: " + label };
}

const string& CDesktopContactInfoItem::GetType() const
{
    return CContact_info::GetTypeInfo()->GetName();
}

const CObject* CDesktopContactInfoItem::GetObject(void) const
{
    return m_Contact.GetPointerOrNull();
}

const CSubmit_block& CDesktopContactInfoItem::GetSubmitBlock() const
{
    return m_Submit->GetSub();
}

CSeq_entry_Handle CDesktopContactInfoItem::GetSeqentryHandle() const
{
    if (m_Submit->IsEntrys()) {
        const CSeq_submit::TData::TEntrys& entries = m_Submit->GetData().GetEntrys();
        if (!entries.empty()) {
            return m_Scope->GetSeq_entryHandle(**entries.begin());
        }
    }
    return CSeq_entry_Handle();
}

// CDesktopCitSubItem

IDesktopDataItem::TLines CDesktopCitSubItem::GetDescription(bool root) const
{
    string label;
    m_CitSub->GetLabel(&label);
    return TLines{ kSpaces + "Cit: " + label };
}

const string& CDesktopCitSubItem::GetType() const
{
    return CCit_sub::GetTypeInfo()->GetName();
}

 const CObject* CDesktopCitSubItem::GetObject(void) const
{
    return m_CitSub.GetPointerOrNull();
}

 const CSubmit_block& CDesktopCitSubItem::GetSubmitBlock() const
{
     return m_Submit->GetSub();
}

 CSeq_entry_Handle CDesktopCitSubItem::GetSeqentryHandle() const
 {
     if (m_Submit->IsEntrys()) {
         const CSeq_submit::TData::TEntrys& entries = m_Submit->GetData().GetEntrys();
         if (!entries.empty()) {
             return m_Scope->GetSeq_entryHandle(**entries.begin());
         }
     }
     return CSeq_entry_Handle();
 }

// CDesktopSeqdescItem

IDesktopDataItem::TLines CDesktopSeqdescItem::GetDescription(bool root) const
{
    switch (m_Seqdesc->Which()) {
    case CSeqdesc::e_Title:
        return TLines{ kSpaces + "Title: " + m_Seqdesc->GetTitle() };
    case CSeqdesc::e_Source:
        return TLines{ kSpaces + "BioSrc: " +
            (m_Seqdesc->GetSource().IsSetTaxname() ? m_Seqdesc->GetSource().GetTaxname() : kEmptyStr) };
    case CSeqdesc::e_Molinfo:
    {
        string label;
        m_Seqdesc->GetMolinfo().GetLabel(&label);
        return TLines{ kSpaces + "MolInfo: " + label };
    }
    case CSeqdesc::e_User:
        return s_GetDescription(m_Seqdesc->GetUser());
    case CSeqdesc::e_Pub:
    {
        string label;
        m_Seqdesc->GetPub().GetPub().GetLabel(&label);
        return TLines{ kSpaces + "Pub: " + label };
    }
    case CSeqdesc::e_Update_date:
    {
        string label;
        m_Seqdesc->GetUpdate_date().GetDate(&label);
        return TLines{ kSpaces + "UpdateDate: " + label };
    }
    case CSeqdesc::e_Mol_type:
    {
        string label = (ENUM_METHOD_NAME(EGIBB_mol)()->FindName(m_Seqdesc->GetMol_type(), true));
        return TLines{ kSpaces + "MolType: " + label };
    }
    case CSeqdesc::e_Modif:
        return s_GetDescription(m_Seqdesc->GetModif());
    case CSeqdesc::e_Method:
    {
        string label = (ENUM_METHOD_NAME(EGIBB_method)()->FindName(m_Seqdesc->GetMethod(), true));
        return TLines{ kSpaces + "Method: " + label };
    }
    case CSeqdesc::e_Name:
        return TLines{ kSpaces + "Name: " + m_Seqdesc->GetName() };
    case CSeqdesc::e_Org:
        return s_GetDescription(m_Seqdesc->GetOrg());
    case CSeqdesc::e_Comment:
        return TLines{ kSpaces + "Comment: " + m_Seqdesc->GetComment() };
    case CSeqdesc::e_Num:
        return TLines{ kSpaces + "Num: " + kSpaces };;
    case CSeqdesc::e_Maploc:
    {
        string label;
        m_Seqdesc->GetMaploc().GetLabel(&label);
        return TLines{ kSpaces + "MapLoc: " + label };
    }
    case CSeqdesc::e_Pir:
        return TLines{ kSpaces + "PIR: " + kSpaces };
    case CSeqdesc::e_Genbank:
        return TLines{ kSpaces + "Genbank: " + kSpaces };
    case CSeqdesc::e_Region:
        return TLines{ kSpaces + "Region: " + m_Seqdesc->GetRegion() };
    case CSeqdesc::e_Sp:
        return TLines{ kSpaces + "SP: " + kSpaces };
    case CSeqdesc::e_Dbxref:
    {
        string label;
        m_Seqdesc->GetDbxref().GetLabel(&label);
        return TLines{ kSpaces + "Dbxref: " + label };
    }
    case CSeqdesc::e_Embl:
        return TLines{ kSpaces + "EMBL: " + kSpaces };
    case CSeqdesc::e_Create_date:
    {
        string label;
        m_Seqdesc->GetCreate_date().GetDate(&label);
        return TLines{ kSpaces + "CreateDate: " + label };
    }
    case CSeqdesc::e_Prf:
        return TLines{ kSpaces + "PRF: " + kSpaces };
    case CSeqdesc::e_Pdb:
        return TLines{ kSpaces + "PDB: " + kSpaces };
    case CSeqdesc::e_Het:
        return TLines{ kSpaces + "Heterogen: " + m_Seqdesc->GetHet() };
    case CSeqdesc::e_Modelev:
        return TLines{ kSpaces + "ModelEv: " + kSpaces };
    default:
        break;
    }
    return TLines{ "unrecognized descriptor" };
}

IDesktopDataItem::TLines CDesktopSeqdescItem::s_GetDescription(const CUser_object& user_obj)
{
    string label = (user_obj.CanGetClass() ? user_obj.GetClass()
        : (user_obj.GetType().IsStr() ? user_obj.GetType().GetStr() : kEmptyStr));
    return TLines{ kSpaces + "UserObj: " + label };
}

IDesktopDataItem::TLines CDesktopSeqdescItem::s_GetDescription(const COrg_ref& org)
{
    string label = (org.CanGetTaxname() ? org.GetTaxname() : (org.CanGetCommon() ? org.GetCommon() : kEmptyStr));
    return TLines{ kSpaces + "Org: " + label };
}

IDesktopDataItem::TLines CDesktopSeqdescItem::s_GetDescription(const CSeqdesc::TModif& modif)
{
    string label;
    for (auto& it : modif) {
        label += ENUM_METHOD_NAME(EGIBB_mod)()->FindName(it, true) + ", ";
    }
    label = label.substr(0, label.size() - 2);
    return TLines{ kSpaces + "Modif: " + label };
}

const string& CDesktopSeqdescItem::GetType() const
{
    return CSeqdesc::GetTypeInfo()->GetName();
}

const CObject* CDesktopSeqdescItem::GetObject(void) const
{
    return m_Seqdesc.GetPointerOrNull();
}

// CDesktopAnnotItem

CDesktopAnnotItem::CDesktopAnnotItem(const CSeq_annot_Handle& annoth)
    : m_Annoth(annoth), m_Colour(*wxBLUE)
{
    bool contains_farpointers = false;
    for (CAlign_CI align_it(m_Annoth); align_it && !contains_farpointers; ++align_it) {
        CSeq_align_Handle ah = align_it.GetSeq_align_Handle();
        contains_farpointers = CDesktopAlignItem::s_ContainsFarPointers(ah);
    }

    if (contains_farpointers) {
        m_Colour.Set(0x00458B00);
    }
}

IDesktopDataItem::TLines CDesktopAnnotItem::GetDescription(bool root) const
{
    const CSeq_annot& annot = *m_Annoth.GetCompleteSeq_annot();
    if (!annot.IsSetData()){
        return TLines{ "No annotation data" };
    }

    switch (annot.GetData().Which()) {
    case CSeq_annot::C_Data::e_Ftable:
        return TLines{ "  Feature Table" };
    case CSeq_annot::C_Data::e_Align:
        return TLines{ "  Alignments" };
    case CSeq_annot::C_Data::e_Graph:
        return TLines{ "  Graphs" };
    default:
        break;
    }

    return TLines{ "  Other annotation" };
}

const string& CDesktopAnnotItem::GetType() const
{
    return CSeq_annot::GetTypeInfo()->GetName();
}

const CObject* CDesktopAnnotItem::GetObject(void) const
{
    CConstRef<CSeq_annot> annot = m_Annoth.GetCompleteSeq_annot();
    return annot.GetPointerOrNull();
}

CSeq_entry_Handle CDesktopAnnotItem::GetSeqentryHandle() const
{
    return m_Annoth.GetParentEntry();
}

// CDesktopFeatItem

static const string& s_GetProtRefName(const CProt_ref& prot)
{
    if (prot.IsSetName() && !prot.GetName().empty()) {
        return prot.GetName().front();
    }
    else {
        return kEmptyStr;
    }
}

const string& CDesktopFeatItem::s_GetProteinName(const CSeq_feat_Handle& fh)
{
    if (fh.IsSetProduct()) {
        CBioseq_Handle prot = fh.GetScope().GetBioseqHandle(fh.GetProduct());
        if (prot) {
            CFeat_CI feat_it(prot, CSeqFeatData::eSubtype_prot);
            if (feat_it) {
                return s_GetProtRefName(feat_it->GetData().GetProt());
            }
        }
    }
    if (fh.IsSetXref()) {
        for (auto& it : fh.GetXref()) {
            if (it->IsSetData() && it->GetData().IsProt()) {
                return s_GetProtRefName(it->GetData().GetProt());
            }
        }
    }
    return kEmptyStr;
}

IDesktopDataItem::TLines CDesktopFeatItem::GetDescription(bool root) const
{
    // get the location label
    string loc_label;
    m_Feath.GetLocation().GetLabel(&loc_label);

    TLines lines;
    if (m_Feath.GetData().IsCdregion()) {
        const string& prot_name = s_GetProteinName(m_Feath);
        if (!prot_name.empty()) {
            string prod_label;
            if (m_Feath.IsSetProduct()) {
                m_Feath.GetProduct().GetLabel(&prod_label);
            }
            lines.push_back(kSpaces + "CDS: " + prot_name);
            lines.push_back(kSpaces + kSpaces + loc_label + "\n" + kSpaces + kSpaces + "product: " + prod_label);
            return lines;
        }
        // else, treat CDS as any other feature
    }
   
    string feat_label;
    feature::GetLabel(*m_Feath.GetOriginalSeq_feat(), &feat_label, feature::eBoth);
    lines.push_back(kSpaces + feat_label);
    lines.push_back(kSpaces + kSpaces + loc_label);
    return lines;
}

const string& CDesktopFeatItem::GetType() const
{
    return CSeq_feat::GetTypeInfo()->GetName();
}

const CObject* CDesktopFeatItem::GetObject(void) const
{
    CConstRef<CSeq_feat> feat = m_Feath.GetOriginalSeq_feat();
    return feat.GetPointerOrNull();
}

CSeq_entry_Handle CDesktopFeatItem::GetSeqentryHandle() const
{
    CBioseq_Handle bsh = m_Feath.GetScope().GetBioseqHandle(m_Feath.GetLocationId());
    return (bsh) ? bsh.GetSeq_entry_Handle() : CSeq_entry_Handle();
}


// CDesktopAlignItem

CDesktopAlignItem::CDesktopAlignItem(const CSeq_align_Handle& ah)
    : m_Alignh(ah), m_Colour(*wxBLUE)
{
    if (s_ContainsFarPointers(m_Alignh)) {
        // if it contains far pointers color the text in green
        // as well as the annotation containing this alignment
        m_Colour.SetRGB(0x00458B00);
    }
}

bool CDesktopAlignItem::s_ContainsFarPointers(const CSeq_align_Handle& ah)
{
    bool contains_farpointer = false;

    const CSeq_annot_Handle& annoth = ah.GetAnnot();
    CSeq_entry_Handle seh = annoth.GetTopLevelEntry();
    const CSeq_align& align = *ah.GetSeq_align();
    
    if (align.IsSetDim()) {
        for (CSeq_align::TDim row = 0; !contains_farpointer && row < align.GetDim(); ++row) {
            CRef<CSeq_loc> loc_row = align.CreateRowSeq_loc(row);
            if (validator::IsFarLocation(*loc_row, seh)) {
                contains_farpointer = true;
            }
        }
    }
    return contains_farpointer;
}

IDesktopDataItem::TLines CDesktopAlignItem::GetDescription(bool root) const
{
    string label = kSpaces + "SeqAlign: ";
    const CSeq_align& align = *m_Alignh.GetSeq_align();
    
    if (align.IsSetDim()) {
        for (CSeq_align::TDim row = 0; row < 3 && row < align.GetDim(); ++row) {
            label += align.GetSeq_id(row).GetSeqIdString(true);
            label += ", ";
        }
        label.pop_back();
        label += "...";
    }
    return TLines{ label };
}

const string& CDesktopAlignItem::GetType() const
{
    return CSeq_align::GetTypeInfo()->GetName();
}

const CObject* CDesktopAlignItem::GetObject(void) const
{
    CConstRef<CSeq_align> align = m_Alignh.GetSeq_align();
    return align.GetPointerOrNull();
}

CSeq_entry_Handle CDesktopAlignItem::GetSeqentryHandle() const
{
    const CSeq_annot_Handle& annoth = m_Alignh.GetAnnot();
    return (annoth) ? annoth.GetParentEntry() : CSeq_entry_Handle();
}


// CDesktopGraphItem

IDesktopDataItem::TLines CDesktopGraphItem::GetDescription(bool root) const
{
    string label;
    const CSeq_graph& graph = *m_Graphh.GetSeq_graph();
    graph.GetLoc().GetLabel(&label);
    return TLines{ kSpaces + "SeqGraph: " + label };
}

const string& CDesktopGraphItem::GetType() const
{
    return CSeq_graph::GetTypeInfo()->GetName();
}

const CObject* CDesktopGraphItem::GetObject(void) const
{
    CConstRef<CSeq_graph> graph = m_Graphh.GetSeq_graph();
    return graph.GetPointerOrNull();
}

CSeq_entry_Handle CDesktopGraphItem::GetSeqentryHandle() const
{
    const CSeq_annot_Handle& annoth = m_Graphh.GetAnnot();
    return (annoth) ? annoth.GetParentEntry() : CSeq_entry_Handle();
}

END_NCBI_SCOPE

