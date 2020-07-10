/*  $Id: macro_treeitemdata.cpp 44990 2020-05-04 17:26:18Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrea Asztalos

 */


#include <ncbi_pch.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <util/xregexp/regexp.hpp>
#include <gui/objutils/macro_biodata.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>
#include <gui/objutils/macro_fn_feature.hpp>
#include <gui/objutils/macro_fn_entry.hpp>
#include <gui/objutils/macro_fn_string_constr.hpp>
#include <gui/objutils/macro_fn_where.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/widgets/edit/pub_fieldtype.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/macro_panel_args.hpp>
#include <gui/widgets/edit/macro_apply_itemdata.hpp>
#include <gui/widgets/edit/macro_applytbl_itemdata.hpp>
#include <gui/widgets/edit/macro_edit_itemdata.hpp>
#include <gui/widgets/edit/macro_convert_itemdata.hpp>
#include <gui/widgets/edit/macro_copy_itemdata.hpp>
#include <gui/widgets/edit/macro_parse_itemdata.hpp>
#include <gui/widgets/edit/macro_swap_itemdata.hpp>
#include <gui/widgets/edit/macro_rmv_itemdata.hpp>
#include <gui/widgets/edit/macro_rmvoutside_itemdata.hpp>
#include <gui/widgets/edit/macro_featloc_itemdata.hpp>
#include <gui/widgets/edit/macro_general_itemdata.hpp>
#include <gui/widgets/edit/macro_treeitemdata.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(NMacroArgs);


void IMacroActionItemData::CreateParamPanel(wxStaticBoxSizer& action_sizer, wxPanel* parent_panel)
{
    x_AddParamPanel((wxWindow*)action_sizer.GetStaticBox());

    _ASSERT(m_Panel);
    action_sizer.Add(m_Panel->GetPanel(), 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 2);
    action_sizer.Layout();

    parent_panel->Layout();
    parent_panel->Fit();
    parent_panel->Refresh();
}

void IMacroActionItemData::x_LoadPanel(wxWindow* parent, const string &panel_name, const TArgumentsVector &arguments)
{
    try {
        m_Panel.reset(new CMacroPanelLoader(parent, panel_name, arguments));
    }
    catch (const exception& ex) {
        NcbiMessageBox(ex.what());
        return;
    }
    m_ArgList = m_Panel->GetArgumentList();
    // initialize the arguments member
    m_Args = m_ArgList.GetArguments();
}

void IMacroActionItemData::DeleteParamPanel()
{
    if (m_Panel == nullptr) return;
    wxPanel* panel = m_Panel->GetPanel();
    if (panel) {
        wxSizer* sizer = panel->GetContainingSizer();
        if (sizer) {
            sizer->Detach(panel);
        }
    }

    m_Panel.reset(nullptr);
}

bool IMacroActionItemData::TransferDataFromPanel()
{
    _ASSERT(m_Panel);
    if (!m_Panel->TransferDataFromPanel()) 
        return false;
    m_ArgList = m_Panel->GetArgumentList();
    m_Args = m_ArgList.GetArguments();
    return true;
}

string IMacroActionItemData::x_GetVariables(const vector<CTempString>& args) const
{
    string vars;
    for (auto& it : args) {
        vars += it + " = %" + m_ArgList[it].GetValue() + "%\n";
    }
    return vars;
}

void CMacroActionSimpleItemData::x_AddParamPanel(wxWindow* parent)
{
    TArgumentsVector args;
    x_LoadPanel(parent, "DefaultAction", args);
}

const vector<string>& CMacroActionItemData::GetKeywords() const
{
    return CMacroEditorContext::GetInstance().GetKeywords(m_FieldType);
}

void CMTreeItemDataBuilder::x_Init()
{
    m_ActionMap.emplace(EMActionType::eApplyBsrc, new CApplyBsrcQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eEditBsrcText, new CEditQualTreeItemData("Edit existing biosource text qualifiers", EMacroFieldType::eBiosourceTextAll));

    m_ActionMap.emplace(EMActionType::eEditBsrcTax, new CEditQualTreeItemData("Edit existing biosource qualifiers", EMacroFieldType::eBiosourceTax));
    m_ActionMap.emplace(EMActionType::eConvertBsrc, new CConvertBsrcTreeItemData());
    m_ActionMap.emplace(EMActionType::eCopyBsrc, new CCopyBsrcTreeItemData());
    m_ActionMap.emplace(EMActionType::eParseBsrc, new CParseBsrcTreeItemData());
    m_ActionMap.emplace(EMActionType::eParseToBsrc, new CParseToBsrcTreeItemData());
    m_ActionMap.emplace(EMActionType::eSwapBsrc, new CSwapBsrcTreeItemData());
    m_ActionMap.emplace(EMActionType::eRemoveBsrc, new CRmvBsrcQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eRmvOutsideBsrc, new CRmvOutsideBsrcTreeItemData());
    m_ActionMap.emplace(EMActionType::eFixSubsrcFormat, new CFixSubsrcFormatTreeItemData("Fix format of subsource modifiers: altitude, collection-date, lat-lon"));
    m_ActionMap.emplace(EMActionType::eFixSrcQuals, new CFixSrcQualsTreeItemData("Fix capitalization in biosource qualifiers"));
    m_ActionMap.emplace(EMActionType::eFixMouseStrain, new CFixMouseStrainTreeItemData("Fix capitalization in common Mus Musculus strains"));
    m_ActionMap.emplace(EMActionType::eFixFormatPrimer, new CFixPrimerTreeItemData("Fix i in primer (forward and reverse) sequences", true));
    m_ActionMap.emplace(EMActionType::eTrimJunk, new CFixPrimerTreeItemData("Trim junk from primer (forward and reverse) sequences", false));
    m_ActionMap.emplace(EMActionType::eRmvLineageNotes, new CMacroActionSimpleItemData(macro::CMacroFunction_RemoveLineageSourceNotes::GetFuncName(), "Remove lineage source notes"));
    m_ActionMap.emplace(EMActionType::eRmvOrgName, new CRmvOrgNameTreeItemData());
    m_ActionMap.emplace(EMActionType::eApplySrcQualTable, new CApplySrcTableTreeItemData());
    m_ActionMap.emplace(EMActionType::eApplyStrCommQualTable, new CApplyStrCommTableTreeItemData());

    m_ActionMap.emplace(EMActionType::eApplyMolinfo, new CApplyMolinfoQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eEditMolinfo, new CEditQualTreeItemData("Edit existing molinfo qualifiers", EMacroFieldType::eMolinfo));
    m_ActionMap.emplace(EMActionType::eRemoveMolinfo, new CRmvQualTreeItemData("Remove molinfo qualifiers", EMacroFieldType::eMolinfo));
    m_ActionMap.emplace(EMActionType::eRmvOutsideMolinfo, new CRmvOutsideQualTreeItemData("Remove text outside string in molinfo qualifiers", EMacroFieldType::eMolinfo));
    m_ActionMap.emplace(EMActionType::eEditMolinfoFields, new CEditMolinfoFieldsTreeItemData());
    m_ActionMap.emplace(EMActionType::eApplyMolinfoQualTable, new CApplyMolinfoTableTreeItemData());

    m_ActionMap.emplace(EMActionType::eApplyPub, new CApplyPubQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eApplyAuthors, new CApplyAuthorsTreeItemData());
    m_ActionMap.emplace(EMActionType::eEditPub, new CEditQualTreeItemData("Edit existing publication fields", EMacroFieldType::ePubdesc));
    m_ActionMap.emplace(EMActionType::eRmvPub, new CRmvQualTreeItemData("Remove publication fields", EMacroFieldType::ePubdesc));
    m_ActionMap.emplace(EMActionType::eRmvOutsidePub, new CRmvOutsideQualTreeItemData("Remove text outside string in publication fields", EMacroFieldType::ePubdesc));
    m_ActionMap.emplace(EMActionType::eISOJTALookup, new CMacroActionSimpleItemData(macro::CMacroFunction_ISOJTALookup::GetFuncName(), "Lookup ISOJTA titles in article publications"));
    m_ActionMap.emplace(EMActionType::ePMIDLookup, new CMacroActionSimpleItemData(macro::CMacroFunction_LookupPub::GetFuncName(), "Lookup publications by PMID"));
    m_ActionMap.emplace(EMActionType::eDOILookup, new CApplyDOILookupTreeItemData());
    m_ActionMap.emplace(EMActionType::eFixUSAandStates, new CMacroActionSimpleItemData(macro::CMacroFunction_FixUSAandStatesAbbrev::GetFuncName(), "Fix USA and state abbreviations in publications"));
    m_ActionMap.emplace(EMActionType::eFixPubTitle, new CFixPubCapsTreeItemData("Fix capitalization in publication title", EMacroFieldType::ePubTitle));
    m_ActionMap.emplace(EMActionType::eFixPubAuthors, new CFixPubCapsTreeItemData("Fix capitalization in publication authors", EMacroFieldType::ePubAuthors));
    m_ActionMap.emplace(EMActionType::eFixPubAffil, new CFixPubCapsTreeItemData("Fix capitalization in publication affiliation fields", EMacroFieldType::ePubAffil));
    m_ActionMap.emplace(EMActionType::eFixPubAffilCountry, new CFixPubCapsTreeItemData("Fix capitalization in publication affiliation country", EMacroFieldType::ePubAffilCountry));
    m_ActionMap.emplace(EMActionType::eFixPubAffilExcept, new CFixPubCapsTreeItemData("Fix capitalization in publication affiliation fields except institute and department", EMacroFieldType::ePubAffilExcept));
    m_ActionMap.emplace(EMActionType::eFixPubLastName, new CFixCapsLastNameTreeItemData("Fix capitalization in publication author last name"));
    m_ActionMap.emplace(EMActionType::eMoveMiddleName, new CMacroActionSimpleItemData(macro::CMacroFunction_MoveMiddleName::GetFuncName(), "Move middle name to first name"));
    m_ActionMap.emplace(EMActionType::eStripSuffix, new CMacroActionSimpleItemData(macro::CMacroFunction_StripSuffix::GetFuncName(), "Remove author name suffix"));
    m_ActionMap.emplace(EMActionType::eReverseAuthorNames, new CMacroActionSimpleItemData(macro::CMacroFunction_ReverseAuthNames::GetFuncName(), "Reverse author names"));
    m_ActionMap.emplace(EMActionType::eApplyPubQualTable, new CApplyPubTableTreeItemData());
    
    m_ActionMap.emplace(EMActionType::eApplyDBLink, new CApplyDBLinkQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eEditDBLink, new CEditQualTreeItemData("Edit existing DBLink fields", EMacroFieldType::eDBLink));
    m_ActionMap.emplace(EMActionType::eRmvOutsideDBLink, new CRmvOutsideQualTreeItemData("Remove text outside string in DBLink fields", EMacroFieldType::eDBLink));
    m_ActionMap.emplace(EMActionType::eRmvDBLink, new CRmvQualTreeItemData("Remove DBLink fields", EMacroFieldType::eDBLink));
    m_ActionMap.emplace(EMActionType::eApplyDBlinkQualTable, new CApplyDBlinkTableTreeItemData());

    m_ActionMap.emplace(EMActionType::eApplyMisc, new CApplyQualTreeItemData("Apply new value to selected descriptors", EMacroFieldType::eMiscDescriptors));
    m_ActionMap.emplace(EMActionType::eEditMisc, new CEditQualTreeItemData("Edit selected descriptors", EMacroFieldType::eMiscDescriptors));
    m_ActionMap.emplace(EMActionType::eRemoveMisc, new CRmvMiscQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eRmvOutsideMisc, new CRmvOutsideQualTreeItemData("Remove text outside string in selected descriptors", EMacroFieldType::eMiscDescriptors));
    m_ActionMap.emplace(EMActionType::eApplyMiscQualTable, new CApplyMiscTableTreeItemData());

    m_ActionMap.emplace(EMActionType::eApplyStructComm, new CApplyStrCommQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eEditStructComm, new CEditStrCommQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eRmvStructComm, new CRmvStrCommQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eRmvOutsideStructComm, new CRmvOutsideStrCommQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eAddStructComment, new CApplyStructCommentTreeItemData());
    m_ActionMap.emplace(EMActionType::eParseBsrcToStructComm, new CParseBsrcToStructCommTreeItemData());

    m_ActionMap.emplace(EMActionType::eTrimTerminalNs, new CMacroActionSimpleItemData(macro::CMacroFunction_TrimTerminalNs::GetFuncName(), "Trim terminal Ns from nucleotide sequences"));
    m_ActionMap.emplace(EMActionType::eBoldXref, new CMacroActionSimpleItemData(macro::CMacroFunction_MakeBoldXrefs::GetFuncName(), "Make BARCODE DbXrefs"));
    m_ActionMap.emplace(EMActionType::eReorderStructComm, new CMacroActionSimpleItemData(macro::CMacroFunction_ReorderStructComment::GetFuncName(), "Reorder structured comments"));
    m_ActionMap.emplace(EMActionType::eRmvDuplStructComm, new CMacroActionSimpleItemData(macro::CMacroFunction_RmvDuplStructComments::GetFuncName(), "Remove duplicate structured comments"));
    m_ActionMap.emplace(EMActionType::eRemoveDescr, new CRemoveDescrTreeItemData());
    m_ActionMap.emplace(EMActionType::eConvertFeature, new CConvertFeatureTreeItemData());
    m_ActionMap.emplace(EMActionType::eRemoveFeature, new CRemoveFeatureTreeItemData());
    m_ActionMap.emplace(EMActionType::eRemoveDbXref, new CRemoveDbXrefTreeItemData());
    m_ActionMap.emplace(EMActionType::eRmvDuplFeatures, new CRemoveDuplFeatsTreeItemData());

    m_ActionMap.emplace(EMActionType::eApplyFeatQual, new CApplyFeatQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eEditFeatQual, new CEditFeatQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eConvertFeatQual, new CConvertFeatQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eCopyFeatQual, new CCopyFeatQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eParseFeatQual, new CParseFeatQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eSwapFeatQual, new CSwapFeatQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eRmvFeatQual, new CRmvFeatQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eRmvOutsideFeatQual, new CRmvOutsideFeatQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eApplyFeatQualTable, new CApplyFeatTableTreeItemData());

    m_ActionMap.emplace(EMActionType::eApplyRNA, new CApplyRNAQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eEditRNA, new CEditRNAQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eConvertRNA, new CConvertRNAQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eCopyRNA, new CCopyRNAQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eParseRNA, new CParseRNAQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eSwapRNA, new CSwapRNAQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eRmvRNA, new CRmvRNAQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eRmvOutsideRNA, new CRmvOutsideRNAQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eApplyRNAQualTable, new CApplyRNATableTreeItemData());

    m_ActionMap.emplace(EMActionType::eConvertCdsGeneProt, new CConvertCdsGeneProtTreeItemData());
    m_ActionMap.emplace(EMActionType::eCopyCdsGeneProt, new CCopyCdsGeneProtTreeItemData());
    m_ActionMap.emplace(EMActionType::eParseCdsGeneProt, new CParseCdsGeneProtTreeItemdata());
    m_ActionMap.emplace(EMActionType::eParseToCdsGeneProt, new CParseToCdsGeneProtTreeItemData());
    m_ActionMap.emplace(EMActionType::eSwapCdsGeneProt, new CSwapCdsGeneProtTreeItemData());
    m_ActionMap.emplace(EMActionType::eApplyCDSGeneProtQualTable, new CApplyCDSGeneProtTableTreeItemData());

    m_ActionMap.emplace(EMActionType::eAdjustConsensusSpliceSite, new CMacroActionSimpleItemData(macro::CMacroFunction_AdjustConsensusSpliceSites::GetFuncName(), "Adjust CDS for consensus splice sites"));
    m_ActionMap.emplace(EMActionType::eSynchronizeCDSPartials, new CMacroActionSimpleItemData(macro::CMacroFunction_SynchronizeCDSPartials::GetFuncName(), "Synchronize CDS partials"));
    m_ActionMap.emplace(EMActionType::eReplaceStopsSel, new CMacroActionSimpleItemData(macro::CMacroFunction_ReplaceStopWithSelenocysteine::GetFuncName(), "Replace stops with selenocysteines"));
    m_ActionMap.emplace(EMActionType::eTrimStopsFromCompleteCDS, new CMacroActionSimpleItemData(macro::CMacroFunction_TrimStopFromCompleteCDS::GetFuncName(), "Remove trailing * from complete CDS"));
    m_ActionMap.emplace(EMActionType::eRetranslateCDS, new CRetranslateCDSTreeItemData("Retranslate CDS"));
    m_ActionMap.emplace(EMActionType::eSet5Partial, new CSet5PartialTreeItemData());
    m_ActionMap.emplace(EMActionType::eClear5Partial, new CClear5PartialTreeItemData());
    m_ActionMap.emplace(EMActionType::eSet3Partial, new CSet3PartialTreeItemData());
    m_ActionMap.emplace(EMActionType::eClear3Partial, new CClear3PartialTreeItemData());
    m_ActionMap.emplace(EMActionType::eConvertLocStrand, new CConvertStrandTreeItemData());
    m_ActionMap.emplace(EMActionType::eConvertLocType, new CConvertLocTypeTreeItemData());
    m_ActionMap.emplace(EMActionType::eSetBothPartials, new CSetBothPartialTreeItemData());
    m_ActionMap.emplace(EMActionType::eClearBothPartials, new CClearBothPartialTreeItemData());
    m_ActionMap.emplace(EMActionType::eExtend5ToEnd, new CExtendToSeqEndTreeItemData(EMActionType::eExtend5ToEnd));
    m_ActionMap.emplace(EMActionType::eExtend3ToEnd, new CExtendToSeqEndTreeItemData(EMActionType::eExtend3ToEnd));

    m_ActionMap.emplace(EMActionType::eAddGeneXref, new CAddGeneXrefTreeItemData("Add gene Xref to features"));
    m_ActionMap.emplace(EMActionType::eRemoveGeneXref, new CRemoveGeneXrefTreeItemData("Remove gene Xrefs from features"));
    m_ActionMap.emplace(EMActionType::eSetExceptions, new CSetExceptionsTreeItemData());
    m_ActionMap.emplace(EMActionType::eDiscrAutofix, new CDiscrepancyAutofixTreeItemData("Perform automatic fixes for the selected discrepancy report tests"));
    m_ActionMap.emplace(EMActionType::eFixSpelling, new CMacroActionSimpleItemData(macro::CMacroFunction_FixSpelling::GetFuncName(), "Fix spelling"));
    m_ActionMap.emplace(EMActionType::eAutodefID, new CMacroActionSimpleItemData(macro::CMacroFunction_AutodefId::GetFuncName(), "Create unique deflines"));
    m_ActionMap.emplace(EMActionType::eAutodef, new CAutodefTreeItemData());

    m_ActionMap.emplace(EMActionType::eGeneApply, new CApplyQualTreeItemData("Apply new value to gene qualifiers", EMacroFieldType::eGene));
    m_ActionMap.emplace(EMActionType::eGeneEdit, new CEditQualTreeItemData("Edit existing gene qualifiers", EMacroFieldType::eGene));
    m_ActionMap.emplace(EMActionType::eGeneConvert, new CConvertQualTreeItemData("Convert gene qualifiers", EMacroFieldType::eGene));
    m_ActionMap.emplace(EMActionType::eGeneParse, new CParseQualTreeItemData("Parse gene qualifiers", EMacroFieldType::eGene));
    m_ActionMap.emplace(EMActionType::eGeneCopy, new CCopyQualTreeItemData("Copy gene qualifiers", EMacroFieldType::eGene));
    m_ActionMap.emplace(EMActionType::eGeneSwap, new CSwapQualTreeItemData("Swap gene qualifiers", EMacroFieldType::eGene));
    m_ActionMap.emplace(EMActionType::eRmvGene, new CRmvQualTreeItemData("Remove gene qualifiers", EMacroFieldType::eGene));
    m_ActionMap.emplace(EMActionType::eRmvOutsideGene, new CRmvOutsideQualTreeItemData("Remove text outside string in gene qualifiers", EMacroFieldType::eGene));

    m_ActionMap.emplace(EMActionType::eProteinApply, new CApplyProteinQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eProteinEdit, new CEditProteinQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eProteinConvert, new CConvertProteinQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eProteinParse, new CParseProteinQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eProteinSwap, new CSwapProteinQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eProteinCopy, new CCopyProteinQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eProteinRmv, new CRmvProteinQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eProteinRmvOutside, new CRmvOutsideProteinQualTreeItemData());
    m_ActionMap.emplace(EMActionType::eFixProtFormat, new CMacroActionSimpleItemData(macro::CMacroFunction_FixProteinFormat::GetFuncName(), "Remove organism names from CDS product names"));
    m_ActionMap.emplace(EMActionType::eRmvInvalidEC, new CMacroActionSimpleItemData(macro::CMacroFunction_RemoveInvalidECNumbers::GetFuncName(), "Remove invalid EC numbers from CDS product"));
    m_ActionMap.emplace(EMActionType::eUpdateEC, new CUpdateReplacedECTreeItemData("Update replaced EC numbers in CDS product"));

    m_ActionMap.emplace(EMActionType::eAddCDS, new CAddCDSFeatTreeItemData());
    m_ActionMap.emplace(EMActionType::eAddRNA, new CAddRNAFeatTreeItemData());
    m_ActionMap.emplace(EMActionType::eAddFeature, new CAddFeatureTreeItemData());
    m_ActionMap.emplace(EMActionType::eRmvSegGaps, new CMacroActionSimpleItemData(macro::CMacroFunction_RemoveSegGaps::GetFuncName(), "Remove seg-gaps from alignments"));
    m_ActionMap.emplace(EMActionType::eRmvAlign, new CMacroActionSimpleItemData(macro::CMacroFunction_RemoveAlignment::GetFuncName(), "Remove alignment"));

    m_ActionMap.emplace(EMActionType::eAddProteinTitle, new CMacroActionSimpleItemData(macro::CMacroFunction_AddProteinTitles::GetFuncName(), "Instantiate protein titles"));
    m_ActionMap.emplace(EMActionType::eRmvSequence, new CMacroActionSimpleItemData(macro::CMacroFunction_RemoveSequence::GetFuncName(), "Remove sequence"));
    m_ActionMap.emplace(EMActionType::eJointRNA, new CMacroActionSimpleItemData(macro::CMacroFunction_JoinShorttRNAs::GetFuncName(), "Join short tRNA for nucleotide sequences"));
    m_ActionMap.emplace(EMActionType::eTaxLookup, new CMacroActionSimpleItemData(macro::CMacroFunction_TaxLookup::GetFuncName(), "Perform taxonomy lookup and extended cleanup"));
    m_ActionMap.emplace(EMActionType::eCreateProtFeats, new CMacroActionSimpleItemData(macro::CMacroFunction_CreateProteinFeats::GetFuncName(), "Create full length protein features on protein sequences"));
    m_ActionMap.emplace(EMActionType::eRemoveSingleItem, new CMacroActionSimpleItemData(macro::CMacroFunction_RemoveSingleItemSet::GetFuncName(), "Remove single-sequence set"));
    m_ActionMap.emplace(EMActionType::eRenormNucProt, new CMacroActionSimpleItemData(macro::CMacroFunction_RenormalizeNucProtSet::GetFuncName(), "Renormalize nuc-prot sets"));
    m_ActionMap.emplace(EMActionType::eDeltaToRaw, new CMacroActionSimpleItemData(macro::CMacroFunction_DeltaSeqToRaw::GetFuncName(), "Remove assembly gaps"));
    m_ActionMap.emplace(EMActionType::eConvertSetClass, new CConvertClassTreeItemData());
    m_ActionMap.emplace(EMActionType::eApplyPmidToEntry, new CApplyPmidToEntryTreeItemData());
    m_ActionMap.emplace(EMActionType::eApplyDOIToEntry, new CApplyDOIToEntryTreeItemData());
    m_ActionMap.emplace(EMActionType::eAddAssemblyGapByNs, new CAddAssemblyGapsNTreeItemData());
    m_ActionMap.emplace(EMActionType::eConvertGapsBySize, new CConvertGapsTreeItemData());
}

wxTreeItemData* CMTreeItemDataBuilder::GetTreeItemData(const CMActionNode& node) const
{
    wxTreeItemData* data = nullptr;
    if (node.GetActionType() == EMActionType::eNotSet) {
        return new CMIteratorItemData(node.GetTarget());
    }

    auto it = m_ActionMap.find(node.GetActionType());
    if (it != m_ActionMap.end()) {
        data = it->second;
        if (IMacroActionItemData* action_data = dynamic_cast<IMacroActionItemData*>(data)) {
            action_data->SetTarget(node.GetTarget());
        }
    }

    return data;
}


namespace NMItemData {

    void WrapInQuotes(string& str)
    {
        str = "\"" + str + "\"";
    }

    void GetFeatureAndField(const string& field_name, string& feature, string& field)
    {
        if (field_name.empty()) return;

        if (macro::NMacroUtil::StringsAreEquivalent(field_name, "codon-start")) {
            feature = "CDS";
            field = field_name;
        }

        SIZE_TYPE pos = field_name.find(" ");
        if (pos == NPOS) {
            field = field_name;
            return;
        }
        feature = field_name.substr(0, pos);
        field = field_name.substr(pos + 1);
    }

    void GetRNASpecificFields(const string& field_name, string& rna_type, string& ncRNA_class, string& rna_qual)
    {
        rna_type = ncRNA_class = rna_qual = kEmptyStr;
        vector<string> rna_fields = CMacroEditorContext::GetInstance().GetFieldNames(EMacroFieldType::eRNA);

        for (auto&& it : rna_fields) {
            if (NStr::EndsWith(field_name, it)) {
                rna_qual = it;
                if (it.length() < field_name.length()) {
                    rna_type = field_name.substr(0, field_name.length() - it.length());
                    NStr::TruncateSpacesInPlace(rna_type);
                }
                break;
            }
        }

        vector<string> rna_types = CMacroEditorContext::GetInstance().GetRNATypes();
        if (!rna_qual.empty()) {
            if (rna_type.empty()) {
                rna_type = "any";
            }
            else {
                string remainder = rna_type;
                if (NStr::EqualNocase(rna_type, "misc_RNA")) {
                    rna_type = "miscRNA";
                }
                else if (NStr::EqualNocase(rna_type, "pre_RNA")) {
                    rna_type = "preRNA";
                }
                else {
                    for (auto&& it : rna_types) {
                        string major_type = it;
                        if (NStr::StartsWith(rna_type, major_type)) {
                            string remainder = rna_type.substr(major_type.length());
                            NStr::TruncateSpacesInPlace(remainder);
                            if (NStr::EqualNocase(major_type, "ncRNA")) {
                                ncRNA_class = remainder;
                                remainder = "";
                                if (ncRNA_class.empty()) {
                                    ncRNA_class = "any";
                                }
                            }
                            rna_type = major_type;
                            break;
                        }
                    }
                }
            }
        }
    }


    void UpdateFeatureType(string& feature, const string& field)
    {
        if (feature == "CDS") {
            if (field == "product" ||
                field == "name" ||
                field == "activity" ||
                macro::NMacroUtil::StringsAreEquivalent(field, "EC-number")) {
                feature = "Protein";
            }
        } 

        if (field == "locus" ||
            macro::NMacroUtil::StringsAreEquivalent(field, "gene-comment") ||
            macro::NMacroUtil::StringsAreEquivalent(field, "gene-desciption")) {
            feature = "Gene";
        }
    }

    string GetResolveFuncForQual(const string& container, const string& field, const string& rt_var, TConstraints& constraints, bool remove_constraint)
    {
        if (field.empty() || rt_var.empty() || container.empty()) {
            return kEmptyStr;
        }

        string field_type;
        string member;
        string main_qualifier = field;
        if (NStr::EqualNocase(container, "org.orgname.mod")) {
            field_type = "subtype";
            member = "subname";
        }
        else if (NStr::EqualNocase(container, "subtype")) {
            field_type = "subtype";
            member = "name";
        }
        else if (CSeqFeatData::GetQualifierType(field) != CSeqFeatData::eQual_bad ||
            field == "codons_recognized") {
            field_type = "qual";
            member = "val";
        } else if (macro::NMacroUtil::IsSatelliteSubfield(field)) {
            field_type = "qual";
            member = "val";
            main_qualifier = "satellite";
        }
        else if (macro::NMacroUtil::IsMobileElementTSubfield(field)) {
            field_type = "qual";
            member = "val";
            main_qualifier = macro::kMobileElementTQual;
        }

        string func = rt_var + " = " + macro::CMacroFunction_Resolve::GetFuncName();
        func += "(\"" + container + "\")";
        func += " WHERE " + rt_var + "." + field_type + " = \"" + main_qualifier + "\"";

        if (constraints.empty()) {
            // ends with a semicolon
            func += ";\n";
            return func;
        }

        TConstraints::iterator it = constraints.begin();
        while (it != constraints.end()) {
            auto& field_ref = it->first;
            if (field_ref.find(macro::CMacroFunction_StructVoucherPart::GetFuncName()) != NPOS) {
                if (it->second.find(field) != NPOS) {
                    NStr::ReplaceInPlace(it->second, field, rt_var + "." + member);
                    func += " AND " + it->second;
                    if (remove_constraint) {
                        it = constraints.erase(it);
                        continue;
                    }
                }
            }
            else if ((field == macro::kSatelliteName && field_ref.find(macro::CMacroFunction_SatelliteName::GetFuncName()) != NPOS) ||
                (field == macro::kSatelliteType && field_ref.find(macro::CMacroFunction_SatelliteType::GetFuncName()) != NPOS)) {
                if (field_ref.find(macro::CMacroFunction_RelatedFeatures::GetFuncName()) == NPOS) {
                    func += " AND " + it->second;
                    if (remove_constraint) {
                        it = constraints.erase(it);
                        continue;
                    }
                }
            }
            else if ((field == macro::kMobileElementTName && field_ref.find(macro::CMacroFunction_MobileElementName::GetFuncName()) != NPOS) ||
                (field == macro::kMobileElementTType && field_ref.find(macro::CMacroFunction_MobileElementType::GetFuncName()) != NPOS)) {
                if (field_ref.find(macro::CMacroFunction_RelatedFeatures::GetFuncName()) == NPOS) {
                    func += " AND " + it->second;
                    if (remove_constraint) {
                        it = constraints.erase(it);
                        continue;
                    }
                }
            }
            else if (field_ref.find(',') != NPOS) {
                vector<string> tokens;
                NStr::Split(field_ref, ",", tokens);
                for (auto& word : tokens) {
                    NStr::ReplaceInPlace(word, "\"", "");
                    NStr::ReplaceInPlace(word, " ", "");
                }
                if (NStr::EqualNocase(tokens[0], container) &&
                    NStr::EqualNocase(tokens[1], field)) {
                    // found relevant constraint
                    size_t pos = it->second.find(field_ref);
                    if (pos != NPOS) {
                        string new_field = "\"" + rt_var + "." + member + "\"";
                        string new_constraint = it->second; // in case we need to keep the original constraint
                        NStr::ReplaceInPlace(new_constraint, field_ref, new_field);
                        func += " AND " + new_constraint;
                        if (remove_constraint) {
                            it = constraints.erase(it);
                            continue;
                        }
                    }
                }
            }
            ++it;
        }
        // ends with a semicolon
        func += ";\n";
        return func;
    }

    string GetResolveFuncForMultiValuedQual(const string& field, const string& rt_var, TConstraints& constraints, bool remove_constraint)
    {
        if (field.empty() || rt_var.empty() || constraints.empty()) {
            return kEmptyStr;
        }

        string func;
        // if there are no relevant constraints, there is no need to define run-time variable
        TConstraints::iterator it = constraints.begin();
        while (it != constraints.end()) {
            if (it->first.find(field) != NPOS) {
                if (func.empty()) {
                    func = rt_var + " = " + macro::CMacroFunction_Resolve::GetFuncName();
                    func += "(\"" + field + "\") ";
                }

                if (func.find("WHERE") == NPOS) {
                    func += "WHERE ";
                }
                else {
                    func += "AND ";
                }

                string new_constraint = it->second;
                NStr::ReplaceInPlace(new_constraint, it->first, rt_var);
                func += new_constraint;
                if (remove_constraint) {
                    it = constraints.erase(it);
                    continue;
                }
            }
            ++it;
        }
        
        if (!func.empty()) {
            func += ";\n";
        }
        return func;
    }

    pair<string, string> GetResolveFuncForMultiValuedQuals(EMacroFieldType type, const string& fieldname, const string& rt_var, TConstraints& constraints, bool remove_constraint)
    {
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(fieldname, type);
        string resolve_fnc, var_name = path;
        WrapInQuotes(var_name);

        if (MultipleValuesAllowed(path)) {
            resolve_fnc = GetResolveFuncForMultiValuedQual(path, rt_var, constraints, remove_constraint);

            if (!resolve_fnc.empty()) {
                var_name = rt_var;
            }
        }
        return make_pair(resolve_fnc, var_name);
    }

    pair<string, string> GetResolveFuncForFeatQuals(const string& feature, const string& field, const string& rt_var, TConstraints& constraints, bool remove_constraint)
    {
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(feature, field);
        string resolve_fnc, var_name = path;
        WrapInQuotes(var_name);

        if (path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(path, ",", tokens);

            if (!NStr::EqualNocase(feature, "gene") && IsGeneQualifier(tokens[1])) {
                resolve_fnc = GetResolveFuncForGeneQuals(tokens[0], tokens[1], rt_var, constraints);
                var_name = rt_var;
            }
            else {
                resolve_fnc = GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints, true);
                if (!resolve_fnc.empty()) {
                    var_name = GetEditableFieldForPair(tokens[1], rt_var);
                    WrapInQuotes(var_name);
                }
            }
        }
        else if (MultipleValuesAllowed(path)) {
            resolve_fnc = GetResolveFuncForMultiValuedQual(path, rt_var, constraints, remove_constraint);

            if (!resolve_fnc.empty()) {
                var_name = rt_var;
            }
        } else if (path == "dbxref") {
            resolve_fnc = GetResolveFuncForDbXref(path, rt_var, constraints);
            if (!resolve_fnc.empty()) {
                var_name = rt_var;
            }
        }
        return make_pair(resolve_fnc, var_name);
    }

    pair<string, string> GetResolveFuncForSwapFeatQuals(const string& feature, const string& field, const string& rt_var,
        TConstraints& constraints, bool remove_constraint, bool& is_gbqual)
    {
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(feature, field);
        string resolve_fnc, var_name = path;
        NMItemData::WrapInQuotes(var_name);

        if (path.find(',') != NPOS) {
            is_gbqual = true;
            vector<string> tokens;
            NStr::Split(path, ",", tokens);

            if (!NStr::EqualNocase(feature, "gene") && NMItemData::IsGeneQualifier(tokens[1])) {
                resolve_fnc = NMItemData::GetResolveFuncForGeneQuals(tokens[0], tokens[1], rt_var, constraints);
            }
            else {
                resolve_fnc = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints, true);
            }
            var_name = tokens[1];
            NMItemData::WrapInQuotes(var_name);
            
        }
        else if (NMItemData::MultipleValuesAllowed(path)) {
            resolve_fnc = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, remove_constraint);

            if (!resolve_fnc.empty()) {
                var_name = rt_var;
            }
        }
        return make_pair(resolve_fnc, var_name);
    }

    pair<string, string> GetResolveFuncForCdsGeneProtQuals(const string& fieldname, const string& rt_var, TConstraints& constraints, bool remove_constraint)
    {
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(fieldname, EMacroFieldType::eCdsGeneProt);
        string resolve_fnc, var_name = path;
        NMItemData::WrapInQuotes(var_name);

        if (path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(path, ",", tokens);

            resolve_fnc = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints, true);
            if (!resolve_fnc.empty()) {
                var_name = NMItemData::GetEditableFieldForPair(tokens[1], rt_var);
                NMItemData::WrapInQuotes(var_name);
            }
        }
        else if (NMItemData::MultipleValuesAllowed(path)) {
            resolve_fnc = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, remove_constraint);

            if (!resolve_fnc.empty()) {
                var_name = rt_var;
            }
        }
        return make_pair(resolve_fnc, var_name);
    }

    pair<string, string> GetResolveFuncForRNAQuals(const string& fieldname, const string& rt_var, TConstraints& constraints, bool remove_constraint)
    {
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(fieldname, EMacroFieldType::eRNA);
        string resolve_fnc, var_name = path;
        NMItemData::WrapInQuotes(var_name);

        if (path.find(',') != NPOS) {
            vector<string> tokens;
            NStr::Split(path, ",", tokens);

            resolve_fnc = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints);
            var_name = NMItemData::GetEditableFieldForPair(tokens[1], rt_var);
            NMItemData::WrapInQuotes(var_name);
        }
        else if (NMItemData::MultipleValuesAllowed(path)) {
            resolve_fnc = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, remove_constraint);

            if (!resolve_fnc.empty()) {
                var_name = rt_var;
            }
        }
        return make_pair(resolve_fnc, var_name);
    }

    pair<string, string> GetResolveFuncForSwapQuals(EMacroFieldType fieldtype, const string& fieldname,
            const string& rt_var, TConstraints& constraints, bool remove_constraint, bool& is_gbqual)
    {
        string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(fieldname, fieldtype);
        string resolve_fnc, var_name = path;
        NMItemData::WrapInQuotes(var_name);

        if (path.find(',') != NPOS) {
            is_gbqual = true;
            vector<string> tokens;
            NStr::Split(path, ",", tokens);
            resolve_fnc = NMItemData::GetResolveFuncForQual(tokens[0], tokens[1], rt_var, constraints, true);

            var_name = tokens[1];
            NMItemData::WrapInQuotes(var_name);
        }
        else if (NMItemData::MultipleValuesAllowed(path)) {
            resolve_fnc = NMItemData::GetResolveFuncForMultiValuedQual(path, rt_var, constraints, remove_constraint);

            if (!resolve_fnc.empty()) {
                var_name = rt_var;
            }
        }
        return make_pair(resolve_fnc, var_name);
    }

    string GetEditableFieldForPair(const string& field, const string& rt_var)
    {
        if (field.empty() || rt_var.empty()) {
            return kEmptyStr;
        }

        string member;
        if (macro::NMacroUtil::StringsAreEquivalent(field, kOrgModNote)) {
            member = "subname";
        }
        else if (macro::NMacroUtil::StringsAreEquivalent(field, kSubSourceNote)) {
            member = "name";
        }
        else if (COrgMod::IsValidSubtypeName(field, COrgMod::eVocabulary_insdc)) {
            member = "subname";
        }
        else if (CSubSource::IsValidSubtypeName(field, CSubSource::eVocabulary_insdc)) {
            member = "name";
        }
        else if (CSeqFeatData::GetQualifierType(field) != CSeqFeatData::eQual_bad ||
            field == "codons_recognized") {
            member = "val";
        }

        if (member.empty()) {
            return kEmptyStr;
        }

        return rt_var + '.' + member;
    }

    string GetResolveFuncForPubQual(const string& field, string& rt_var, TConstraints& constraints)
    {
        if (field.empty()) return kEmptyStr;

        rt_var.resize(0);
        string pub_func = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, EMacroFieldType::ePubdesc);
        CRegexp regexp("\\((.*)\\)");
        regexp.GetMatch(pub_func, 0, 0, CRegexp::eCompile_ignore_case, true);
        rt_var = regexp.GetSub(pub_func);
        if (rt_var == "()") {
            rt_var = pub_func.substr(4, NPOS);
            NStr::ToLower(rt_var);
        }
        else {
            NStr::ReplaceInPlace(rt_var, "\"", "");
        }
        NStr::ReplaceInPlace(rt_var, "(", "");
        NStr::ReplaceInPlace(rt_var, ")", "");
        NStr::ReplaceInPlace(rt_var, "-", "");

        string func;
        if (!rt_var.empty()) {
            func = rt_var + " = " + pub_func;
        }
        else {
            return kEmptyStr;
        }

        TConstraints::iterator it = constraints.begin();
        while (it != constraints.end()) {
            if (NStr::EqualNocase(it->first, pub_func)) {
                if (func.find("WHERE") == NPOS) {
                    func += " WHERE ";
                }
                else {
                    func += " AND ";
                }

                NStr::ReplaceInPlace(it->second, it->first, rt_var);
                func += it->second;
                it = constraints.erase(it);
            }
            else {
                ++it;
            }
        }
        // ends with semicolon
        func += ";\n";
        return func;
    }

    string GetResolveFuncForPubQualConst(const string& field, const string& rt_var, TConstraints& constraints, EMacroFieldType field_type)
    {
        if (field.empty() || rt_var.empty()) return kEmptyStr;
        string pub_func = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(field, field_type);
        string func;
        if (!rt_var.empty()) {
            func = rt_var + " = " + pub_func;
        }
        else {
            return kEmptyStr;
        }

        TConstraints::iterator it = constraints.begin();
        while (it != constraints.end()) {
            if (NStr::EqualNocase(it->first, pub_func)) {
                if (func.find("WHERE") == NPOS) {
                    func += " WHERE ";
                }
                else {
                    func += " AND ";
                }

                NStr::ReplaceInPlace(it->second, it->first, rt_var);
                func += it->second;
                it = constraints.erase(it);
            }
            else {
                ++it;
            }
        }
        // ends with semicolon
        func += ";\n";
        return func;
    }

    string GetResolveFuncForApplyAuthors(CPubFieldType::EPubFieldType type, const string& rt_var, TConstraints& constraints)
    {
        string func;
        func = rt_var + " = " + macro::CMacroFunction_PubFields::sm_PubAuthors + "()";

        TConstraints::iterator it = constraints.begin();
        while (it != constraints.end()) {
            if (type == CPubFieldType::ePubFieldType_AuthorLastName
                || type == CPubFieldType::ePubFieldType_AuthorFirstName
                || type == CPubFieldType::ePubFieldType_AuthorMiddleInitial
                || type == CPubFieldType::ePubFieldType_AuthorSuffix
                || type == CPubFieldType::ePubFieldType_AuthorConsortium) {

                size_t pos = NStr::Find(it->second, macro::CMacroFunction_PubFields::sm_PubAuthors);
                if (pos != NPOS) {
                    string constr_field = CMacroEditorContext::GetInstance().GetAsnPathToAuthorFieldName(it->first);
                    string search = macro::CMacroFunction_PubFields::sm_PubAuthors + string("(\"" + constr_field + "\")");
                    string replace = "\"" + rt_var + "." + constr_field + "\"";
                    NStr::ReplaceInPlace(it->second, search, replace);
                    
                    if (func.find("WHERE") == NPOS) {
                        func += " WHERE ";
                    }
                    else {
                        func += " AND ";
                    }
                    func += it->second;
                    it = constraints.erase(it);
                    continue;
                }
            }
            ++it;
        }
        // ends with a semicolon
        func += ";\n";
        return func;
    }

    string GetResolveFuncForDBLink(const string& field, string& rt_var, TConstraints& constraints, bool& found_constraint)
    {
        rt_var = "dblink_obj";
        string func = rt_var + " = " + macro::CMacroFunction_Resolve::GetFuncName();
        func += "(\"data\") WHERE EQUALS(\"" + rt_var + ".label.str\", \"" + field + "\", true);\n";


        string temp_rtvar = "data_obj";
        string temp_func = temp_rtvar + " = " + macro::CMacroFunction_Resolve::GetFuncName();
        temp_func += "(\"" + rt_var + ".data.strs\")";
        found_constraint = false;

        TConstraints::iterator it = constraints.begin();
        while (it != constraints.end()) {
            if (it->second.find(field) != NPOS) {
                auto pos = it->second.find("AND");
                if (pos != NPOS) {
                    it->second = it->second.substr(pos + 4, NPOS);
                    if (temp_func.find("WHERE") == NPOS) {
                        temp_func += " WHERE ";
                    }
                    else {
                        temp_func += " AND ";
                    }
                    temp_func += it->second;
                    found_constraint = true;
                    it = constraints.erase(it);
                    continue;
                }
                else {
                    string dblinktype_func = macro::CMacroFunction_GetDBLink::GetFuncName();
                    dblinktype_func += "(\"" + field + "\")";
                    auto pos = it->second.find(dblinktype_func);
                    if (pos != NPOS) {
                        NStr::ReplaceInPlace(it->second, dblinktype_func, temp_rtvar);
                        if (temp_func.find("WHERE") == NPOS) {
                            temp_func += " WHERE ";
                        }
                        else {
                            temp_func += " AND ";
                        }
                        temp_func += it->second;
                        found_constraint = true;
                        it = constraints.erase(it);
                        continue;
                    }
                }
            }
            ++it;
        }
        // ends with semicolon
        temp_func += ";\n";

        if (found_constraint) {
            func += temp_func;
            rt_var = temp_rtvar;
        }
        return func;
    }

    string GetResolveFuncForDbXref(const string& path, const string& rt_var, TConstraints& constraints)
    {
        if (constraints.empty() || rt_var.empty()) {
            return kEmptyStr;
        }
        string func = rt_var + " = " + macro::CMacroFunction_Resolve::GetFuncName();
        func += "(\"" + path + "\")";

        bool found = false;
        TConstraints::iterator it = constraints.begin();
        while (it != constraints.end()) {
            if (it->second.find(path) != NPOS) {
                NStr::ReplaceInPlace(it->second, path, rt_var);
                if (func.find("WHERE") == NPOS) {
                    func += " WHERE ";
                }
                else {
                    func += " AND ";
                }
                func += it->second;
                it = constraints.erase(it);
                found = true;
            }
            else {
                ++it;
            }
        }

        return (found) ? func + ";\n" : kEmptyStr;
        // ends with semicolon
    }

    string GetResolveFuncForSVPart(const string& field, const string& rt_var, TConstraints& constraints, bool& found_constraint)
    {
        if (rt_var.empty()) {
            return kEmptyStr;
        }

        string func = rt_var + " = " + macro::CMacroFunction_Resolve::GetFuncName();
        func += "(\"org.orgname.mod\")";

        auto pos = field.find_last_of('-');
        auto subtype = field.substr(0, pos);
        func += " WHERE " + rt_var + ".subtype = \"" + subtype + "\"";

        found_constraint = false;
        TConstraints::iterator it = constraints.begin();
        while (it != constraints.end()) {
            if (it->second.find(subtype) != NPOS) {
                NStr::ReplaceInPlace(it->second, subtype, rt_var + ".subname");
                if (func.find("WHERE") == NPOS) {
                    func += " WHERE ";
                }
                else {
                    func += " AND ";
                }
                func += it->second;
                it = constraints.erase(it);
                found_constraint = true;
            }
            else {
                ++it;
            }
        }
        return  func + ";\n";
    }

    string GetFuncFromTable_Depr(const string& rt_var, const string& filename, const string& col, TConstraints& constraints, string& warning)
    {
        string func;
        func = rt_var + " = " + macro::CMacroFunction_ValueFromTable::GetFuncName();
        func += "(" + kTableName + ", " + col + ");\n";

        TConstraints::iterator it = constraints.begin();
        while (it != constraints.end()) {
            if (it->second.find(macro::CMacroFunction_InTable::GetFuncName()) != NPOS) {
                if (it->second.find(filename) != NPOS) {
                    string quoted = "\"" + filename + "\"";
                    NStr::ReplaceInPlace(it->second, quoted, kTableName);
                }
                else {
                    warning.assign("The given file names differ from each other");
                }

                if (NStr::EndsWith(it->second, CTempString(", " + col + ")")) ||
                    NStr::EndsWith(it->second, CTempString("," + col + ")"))) {
                    if (!warning.empty())
                        warning += " and ";
                    warning += "the same column number is specified twice";
                }
            }
            ++it;
        }

        return func;
    }

    string GetFuncFromTable(const string& rt_var, const string& filename, const string& col, TConstraints& constraints, string& warning)
    {
        string func;
        func = rt_var + " = " + macro::CMacroFunction_ValueFromTable::GetFuncName();
        func += "(" + kTableName + ", " + col + ");\n";

        TConstraints::iterator it = constraints.begin();
        while (it != constraints.end()) {
            if (it->second.find(macro::CMacroFunction_InTable::GetFuncName()) != NPOS) {
                if (it->second.find(filename) != NPOS) {
                    string quoted = "\"" + filename + "\"";
                    NStr::ReplaceInPlace(it->second, quoted, kTableName);
                }
                else if (it->second.find(kTableName) == NPOS) {
                    warning.assign("The given file names differ from each other");
                }

                if (NStr::EndsWith(it->second, CTempString(", " + col + ")")) ||
                    NStr::EndsWith(it->second, CTempString("," + col + ")"))) {
                    if (!warning.empty())
                        warning += " and ";
                    warning += "the same column number is specified twice";
                }
            }
            ++it;
        }

        return func;
    }

    bool IsStructVoucherPart(const string& field)
    {
        return (NStr::EndsWith(field, macro::kColl_suffix) ||
            NStr::EndsWith(field, macro::kInst_suffix) ||
            NStr::EndsWith(field, macro::kSpecid_suffix));
    }

    bool MultipleValuesAllowed(const string& field)
    {
        return (field == "data.prot.name" ||
            field == "data.prot.activity" ||
            field == "data.prot.ec" ||
            field == "data.gene.syn" ||
            field.find("genbank.keywords") != NPOS);
    }

    bool IsGeneQualifier(const string& field)
    {
        return (macro::NMacroUtil::StringsAreEquivalent(field, "locus_tag") ||
            field == "locus" ||
            field == "allele" ||
            field == "maploc" ||
            field == "desc");  // maybe syn?
    }

    bool IsGeneField(const string& field_name)
    {
        static vector<string> gene_field_names{ "locus", "allele", "gene-comment", "gene-description" };
        return find(gene_field_names.begin(), gene_field_names.end(), field_name) != gene_field_names.end();
    }

    string GetResolveFuncForGeneQuals(const string& container, const string& field, const string& rt_var, TConstraints& constraints, bool remove_constraint)
    {
        if (container.empty() || field.empty() || rt_var.empty()) {
            return kEmptyStr;
        }

        string func = rt_var + " = " + macro::CMacroFunction_GeneQual::GetFuncName();
        func += "(\"" + field + "\")";

        if (constraints.empty()) {
            // ends with a semicolon
            func += ";\n";
            return func;
        }

        TConstraints::iterator it = constraints.begin();
        while (it != constraints.end()) {
            if (it->first.find(',') != NPOS) {
                vector<string> tokens;
                NStr::Split(it->first, ",", tokens);
                for (auto& word : tokens) {
                    NStr::ReplaceInPlace(word, "\"", "");
                    NStr::ReplaceInPlace(word, " ", "");
                }
                if (NStr::EqualNocase(tokens[0], container) &&
                    NStr::EqualNocase(tokens[1], field)) {
                    // found relevant constraint
                    size_t pos = it->second.find(it->first);
                    if (pos != NPOS) {
                        string new_constraint = it->second; // in case we need to keep the original constraint
                        NStr::ReplaceInPlace(new_constraint, it->first, rt_var);
                        if (func.find("WHERE") == NPOS) {
                            func += " WHERE ";
                        }
                        else {
                            func += " AND ";
                        }
                        
                        func += new_constraint;
                        if (remove_constraint) {
                            it = constraints.erase(it);
                            continue;
                        }
                    }
                }
            }
            ++it;
        }
        // ends with a semicolon
        func += ";\n";
        return func;
    }

    string GetResolveFuncForStrCommQual(const string& field, const string& field_name, const string& rt_var, TConstraints& constraints)
    {
        if (field.empty() || rt_var.empty()) return kEmptyStr;
        string func;
        if (NStr::EqualNocase(field, "Field")) {
            if (field_name.empty()) return kEmptyStr;
            func = rt_var + " = " + macro::CMacroFunction_Resolve::GetFuncName();
            func += "(\"data\")";
            func += " WHERE EQUALS(\"" + rt_var + ".label.str\", \"" + field_name + "\", true)";

            string constraint_field = macro::CMacroFunction_StructCommField::GetFuncName();
            constraint_field += "(\"" + field_name + "\")";

            TConstraints::iterator it = constraints.begin();
            while (it != constraints.end()) {
                if (NStr::EqualNocase(it->first, constraint_field)) {
                    NStr::ReplaceInPlace(it->second, constraint_field, "\"" + rt_var + ".data.str\"");
                    if (func.find("WHERE") == NPOS) {
                        func += " WHERE ";
                    }
                    else {
                        func += " AND ";
                    }
                    func += it->second;
                    it = constraints.erase(it);
                    continue;
                }
                ++it;
            }
        }
        else if (NStr::EqualNocase(field, "Database name")) {
            func = rt_var + " = " + macro::CMacroFunction_Resolve::GetFuncName();
            func += "(\"data\")";
            func += " WHERE EQUALS(\"" + rt_var + ".label.str\", \"StructuredCommentPrefix\", true)";
            func += " OR EQUALS(\"" + rt_var + ".label.str\", \"StructuredCommentSuffix\", true)";
        }
        else if (NStr::EqualNocase(field, "Field name")) {
            func = rt_var + " = " + macro::CMacroFunction_Resolve::GetFuncName();
            func += "(\"data\")";
            func += " WHERE NOT EQUALS(\"" + rt_var + ".label.str\", \"StructuredCommentPrefix\", true)";
            func += " AND NOT EQUALS(\"" + rt_var + ".label.str\", \"StructuredCommentSuffix\", true)";

            string constraint_field = macro::CMacroFunction_StructCommFieldname::GetFuncName() + "()";

            TConstraints::iterator it = constraints.begin();
            while (it != constraints.end()) {
                if (NStr::EqualNocase(it->first, constraint_field)) {
                    NStr::ReplaceInPlace(it->second, constraint_field, "\"" + rt_var + ".label.str\"");
                    if (func.find("WHERE") == NPOS) {
                        func += " WHERE ";
                    }
                    else {
                        func += " AND ";
                    }
                    func += it->second;
                    it = constraints.erase(it);
                    continue;
                }
                ++it;
            }
        }
        func += ";";
        return func;
    }

    string GetDelimiterDescr(const string& del)
    {
        string descr;
        if (NStr::EqualNocase(del, "no separation")) {
            descr = "(no separator)";
        }
        else {
            descr = "separated by " + del;
        }

        return descr;
    }

    string GetExistingTextDescr(const CArgumentList& arg_list)
    {
        if (!arg_list.count(kExistingText) || !arg_list.count(kDelimiter))
            return kEmptyStr;

        const string& existing_text = arg_list[kExistingText].GetValue();
        const string& del = arg_list[kDelimiter].GetValue();

        string descr = " (";
        if (NStr::EqualNocase(existing_text, "overwrite")) {
            descr += "overwrite existing text";
        }
        else if (NStr::StartsWith(existing_text, "ignore", NStr::eNocase)) {
            descr += "ignore new text when existing text is present";
        }
        else if (NStr::StartsWith(existing_text, "add", NStr::eNocase)) {
            descr += "add new qualifier";
        } else {
            descr += existing_text;
            NStr::ToLower(descr);
            descr += " " + GetDelimiterDescr(del);
        }
        descr += ")";
        return descr;
    }

    string GetHandleOldValue(const string& type)
    {
        if (NStr::EqualNocase(type, "overwrite")) {
            return "eReplace";
        }
        else if (NStr::EqualNocase(type, "append")) {
            return "eAppend";
        }
        else if (NStr::EqualNocase(type, "prefix")) {
            return "ePrepend";
        }
        else if (NStr::StartsWith(type, "ignore", NStr::eNocase)) {
            return "eLeaveOld";
        }
        else if (NStr::StartsWith(type, "add", NStr::eNocase)) {
            return "eAddQual";
        }
        return kEmptyStr;
    }

    string GetDelimiterChar(const string& del)
    {
        if (NStr::EqualNocase(del, "semicolon")) {
            return ";";
        }
        else if (NStr::EqualNocase(del, "space")) {
            return " ";
        }
        else if (NStr::EqualNocase(del, "colon")) {
            return ":";
        }
        else if (NStr::EqualNocase(del, "comma")) {
            return ",";
        }

        return kEmptyStr;
    }

    string GetHandleBlank(const string& str)
    {
        return (NStr::StartsWith(str, "ignore")) ? "false" : "true";
    }

    string GetStructCommQualDescr(const CArgumentList& arg_list)
    {
        if (!arg_list.count(kField) || !arg_list.count(kStrCommField))
            return kEmptyStr;

        string descr = "structured comment ";
        const string& field = arg_list[kField].GetValue();
        if (NStr::EqualNocase(field, "Field")) {
            if (arg_list[kStrCommField].GetShown()) {
                descr += "field '" + arg_list[kStrCommField].GetValue() + "'";
            }
        }
        else if (NStr::EqualNocase(field, "Database name")) {
            descr += "database name";
        }
        else if (NStr::EqualNocase(field, "Field name")) {
            descr += "field names";
        }
        return descr;
    }

    void UpdateConstraintsForMisc(TConstraints& constraints, const string& fieldname, const string& target)
    {
        if (NStr::EqualNocase(fieldname, "Genome Project id")) {
            string new_constraint = macro::CMacroFunction_ChoiceType::GetFuncName();
            new_constraint += "(\"type\") = \"str\" AND type.str = \"GenomeProjectsDB\"";
            constraints.insert(constraints.begin(), make_pair("", new_constraint));
        }
        else {
            string path = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(fieldname, EMacroFieldType::eMiscDescriptors, target);
            size_t pos = path.find(".keyword");
            if (pos != NPOS) {
                path = path.substr(0, pos);
            }

            string new_constraint = macro::CMacroFunction_ChoiceType::GetFuncName();
            new_constraint += "() = \"" + path + "\"";
            constraints.insert(constraints.begin(), make_pair("", new_constraint));
        } 
    }

    CTempString UpdateTargetForMisc(const string& fieldname)
    {
        CTempString target = macro::CMacroBioData::sm_Seqdesc;
        if (NStr::EqualNocase(fieldname, "genome project id")) {
            target = macro::CMacroBioData::sm_UserObject;
        }
        return target;
    }

    CTempString UpdateTargetForMolinfo(const string& molinfo_type)
    {
        CTempString target = macro::CMacroBioData::sm_MolInfo;
        if (NStr::EqualNocase(molinfo_type, "class") ||
            NStr::EqualNocase(molinfo_type, "topology") ||
            NStr::EqualNocase(molinfo_type, "strand")) {
            target = macro::CMacroBioData::sm_Seq;
        }
        return target;
    }

    CTempString UpdateTargetForRnas(const string& rna_type)
    {
        CTempString target = macro::CMacroBioData::sm_RNA;
        if (NStr::EqualNocase(rna_type, "mRNA")) {
            target = macro::CMacroBioData::sm_mRNA;
        }
        else if (NStr::EqualNocase(rna_type, "rRNA")) {
            target = macro::CMacroBioData::sm_rRNA;
        }
        else if (NStr::EqualNocase(rna_type, "miscRNA")) {
            target = macro::CMacroBioData::sm_miscRNA;
        }
        return target;
    }

    CTempString UpdateTargetForDualRnas(const string& rna_type, const string& from_field)
    {
        CTempString target = macro::CMacroBioData::sm_RNA;
        if (NStr::StartsWith(from_field, "gene")) {
            // it's a gene field
            target = macro::CMacroBioData::sm_Gene;
        }
        else {
            target = UpdateTargetForRnas(rna_type);
        }
        return target;
    }

    CTempString UpdateTargetForFeatures(const string& feature, const string& qualifier, bool for_removal)
    {
        if (feature == "All") {
            if (for_removal) {
                return macro::CMacroBioData::sm_TSEntry;
            }
            else {
                return macro::CMacroBioData::sm_SeqFeat;
            }
        }

        // select FOR EACH target based on descriptor type:
        pair<int, int> feat_type = CMacroEditorContext::GetInstance().GetFeatureType(feature);
        CSeqFeatData::E_Choice type = CSeqFeatData::E_Choice(feat_type.first);
        CSeqFeatData::ESubtype subtype = CSeqFeatData::ESubtype(feat_type.second);

        // default target:
        CTempString target = macro::CMacroBioData::sm_SeqFeat;

        if (type == CSeqFeatData::e_Rna) {
            target = macro::CMacroBioData::sm_RNA;
        }
        else if (type == CSeqFeatData::e_Imp) {
            target = macro::CMacroBioData::sm_ImpFeat;
        }

        switch (subtype) {
        case CSeqFeatData::eSubtype_gene:
            target = macro::CMacroBioData::sm_Gene;
            break;
        case CSeqFeatData::eSubtype_cdregion:
            if (qualifier == "product" || qualifier == "EC-number" || qualifier == "activity") {
                target = macro::CMacroBioData::sm_Protein;
            }
            else {
                target = macro::CMacroBioData::sm_CdRegion;
            }
            break;
        case CSeqFeatData::eSubtype_prot:
            target = macro::CMacroBioData::sm_Protein;
            break;
        case CSeqFeatData::eSubtype_imp:
            target = macro::CMacroBioData::sm_ImpFeat;
            break;
        case CSeqFeatData::eSubtype_mRNA:
            target = macro::CMacroBioData::sm_mRNA;
            break;
        case CSeqFeatData::eSubtype_rRNA:
            target = macro::CMacroBioData::sm_rRNA;
            break;
        case CSeqFeatData::eSubtype_otherRNA:
            target = macro::CMacroBioData::sm_miscRNA;
            break;
        case CSeqFeatData::eSubtype_misc_feature:
            target = macro::CMacroBioData::sm_Miscfeat;
            break;
        default:
            break;
        }

        return target;
    }

    CTempString UpdateTargetForDualFeatures(const string& feature, const string& from_field)
    {
        // select FOR EACH target based on descriptor type:
        pair<int, int> feat_type = CMacroEditorContext::GetInstance().GetFeatureType(feature);
        CSeqFeatData::E_Choice type = CSeqFeatData::E_Choice(feat_type.first);
        CSeqFeatData::ESubtype subtype = CSeqFeatData::ESubtype(feat_type.second);

        // default target:
        CTempString target = macro::CMacroBioData::sm_SeqFeat;

        if (type == CSeqFeatData::e_Rna) {
            target = macro::CMacroBioData::sm_RNA;
        }
        else if (type == CSeqFeatData::e_Imp) {
            target = macro::CMacroBioData::sm_ImpFeat;
        }

        switch (subtype) {
        case CSeqFeatData::eSubtype_gene:
            target = macro::CMacroBioData::sm_Gene;
            break;
        case CSeqFeatData::eSubtype_cdregion:
            if (from_field == "product" ||
                from_field == "name" ||
                from_field == "activity" ||
                macro::NMacroUtil::StringsAreEquivalent(from_field, "EC-number")) {
                target = macro::CMacroBioData::sm_Protein;
            }
            else if (from_field == "locus" ||
                macro::NMacroUtil::StringsAreEquivalent(from_field, "gene-comment") ||
                macro::NMacroUtil::StringsAreEquivalent(from_field, "gene-description")) {
                target = macro::CMacroBioData::sm_Gene;
            } else {
                target = macro::CMacroBioData::sm_CdRegion;
            }
            break;
        case CSeqFeatData::eSubtype_prot:
            target = macro::CMacroBioData::sm_Protein;
            break;
        case CSeqFeatData::eSubtype_imp:
            target = macro::CMacroBioData::sm_ImpFeat;
            break;
        case CSeqFeatData::eSubtype_mRNA:
            target = macro::CMacroBioData::sm_mRNA;
            break;
        case CSeqFeatData::eSubtype_rRNA:
            target = macro::CMacroBioData::sm_rRNA;
            break;
        case CSeqFeatData::eSubtype_otherRNA:
            target = macro::CMacroBioData::sm_miscRNA;
            break;
        case CSeqFeatData::eSubtype_misc_feature:
            target = macro::CMacroBioData::sm_Miscfeat;
            break;
        default:
            break;
        }

        return target;
    }

    CTempString UpdateTargetForCdsGeneProt(const string& fieldname)
    {
        CTempString target = macro::CMacroBioData::sm_Protein;

        if (NStr::StartsWith(fieldname, "CDS") || fieldname == "codon-start") {
            target = macro::CMacroBioData::sm_CdRegion;
        }
        else if (NStr::StartsWith(fieldname, "gene")) {
            target = macro::CMacroBioData::sm_Gene;
        }
        else if (NStr::StartsWith(fieldname, "mRNA")) {
            target = macro::CMacroBioData::sm_mRNA;
        }
        else if (NStr::StartsWith(fieldname, "mat_peptide") || NStr::StartsWith(fieldname, "mat-peptide")) {
            target = macro::CMacroBioData::sm_SeqFeat;
        }
        return target;
    }

    void UpdateConstraintsForRnas(TConstraints& constraints, const CArgumentList& arg_list)
    {
        // add constraint if necessary
        const string& rna_type = arg_list[kRNAType].GetValue();
        string ncRNA_class;
        if (rna_type == "ncRNA" && arg_list[kncRNAClass].GetEnabled()) {
            ncRNA_class = arg_list[kncRNAClass].GetValue();
        }
        UpdateConstraintsForRnas(constraints, rna_type, ncRNA_class);
     }

    void UpdateConstraintsForRnas(TConstraints& constraints, const string& rna_type, const string& ncRNA_class)
    {
        vector<string> new_constraints;

        if (rna_type == "ncRNA") {
            new_constraints.push_back("data.rna.type = \"ncRNA\"");
            if (ncRNA_class != "any" && !ncRNA_class.empty()) {
                new_constraints.push_back("data.rna.ext.gen.class = \"" + ncRNA_class + "\"");
            }
        }
        else if (rna_type == "preRNA") {
            new_constraints.push_back("data.rna.type = \"premsg\"");
        }
        else if (rna_type == "tRNA") {
            new_constraints.push_back("data.rna.type = \"tRNA\"");
        }
        else if (rna_type == "tmRNA") {
            new_constraints.push_back("data.rna.type = \"tmRNA\"");
        }

        for (auto it = new_constraints.rbegin(); it != new_constraints.rend(); ++it) {
            constraints.insert(constraints.begin(), make_pair(kEmptyStr, *it));
        }
    }

    void UpdateConstraintsForFeatures(TConstraints& constraints, const string& feature)
    {
        // add constraint if necessary
        pair<int, int> feat_type = CMacroEditorContext::GetInstance().GetFeatureType(feature);
        CSeqFeatData::E_Choice type = CSeqFeatData::E_Choice(feat_type.first);
        
        vector<string> new_constraints;
        string sel_feature = feature;
        if (type == CSeqFeatData::e_Imp && feature != "misc_feature") {
            new_constraints.push_back("data.imp.key = \"" + feature + "\"");
        }
        
        if (sel_feature == "preRNA" || sel_feature == "precursor_RNA") {
            new_constraints.push_back("data.rna.type = \"premsg\"");
        } else if (sel_feature == "ncRNA" ||
            sel_feature == "tRNA" ||
            sel_feature == "tmRNA" ||
            sel_feature == "scRNA" ||
            sel_feature == "snRNA" ||
            sel_feature == "snoRNA" ||
            sel_feature == "sno_RNA") {
            NStr::ReplaceInPlace(sel_feature, "_", "");
            new_constraints.push_back("data.rna.type = \"" + sel_feature + "\"");
        }

        if (type == CSeqFeatData::e_Prot && sel_feature != "Protein") {
            new_constraints.push_back(macro::CMacroFunction_ChoiceType::GetFuncName() + "(\"data\") = \"prot\"");
            if (sel_feature == "ProProtein") {
                new_constraints.push_back("data.prot.processed = \"preprotein\"");
            }
            else if (NStr::EqualNocase(sel_feature, "mature peptide AA")) {
                new_constraints.push_back("data.prot.processed = \"mature\"");
            }
            else if (NStr::EqualNocase(sel_feature, "signal peptide AA")) {
                new_constraints.push_back("data.prot.processed = \"signal-peptide\"");
            }
            else if (NStr::EqualNocase(sel_feature, "transit peptide AA")) {
                new_constraints.push_back("data.prot.processed = \"transit-peptide\"");
            }
            else if (NStr::EqualNocase(sel_feature, "propeptide AA")) {
                new_constraints.push_back("data.prot.processed = \"propeptide\"");
            }
        }

        if (type == CSeqFeatData::e_Org ||
            type == CSeqFeatData::e_Comment ||
            type == CSeqFeatData::e_Bond ||
            type == CSeqFeatData::e_Site ||
            type == CSeqFeatData::e_Region ||
            type == CSeqFeatData::e_Seq ||
            type == CSeqFeatData::e_Rsite ||
            type == CSeqFeatData::e_User ||
            type == CSeqFeatData::e_Txinit ||
            type == CSeqFeatData::e_Num ||
            type == CSeqFeatData::e_Psec_str ||
            type == CSeqFeatData::e_Non_std_residue ||
            type == CSeqFeatData::e_Het ||
            type == CSeqFeatData::e_Clone ||
            type == CSeqFeatData::e_Variation ||
            type == CSeqFeatData::e_Biosrc ||
            type == CSeqFeatData::e_Pub) {
            new_constraints.push_back(macro::CMacroFunction_ChoiceType::GetFuncName()
                + "(\"data\") = \"" + CSeqFeatData::SelectionName(type) + "\"");
        }
        
        for (auto it = new_constraints.rbegin(); it != new_constraints.rend(); ++it) {
            constraints.insert(constraints.begin(), make_pair(kEmptyStr, *it));
        }
    }

    void UpdateConstraintsForDualCdsGeneProt(TConstraints& constraints, const string& src_feat)
    {
        if (macro::NMacroUtil::StringsAreEquivalent(src_feat, "mat_peptide")) {
            string new_constraint = "data.prot.processed = \"mature\"";
            constraints.insert(constraints.begin(), make_pair(kEmptyStr, new_constraint));
            new_constraint = macro::CMacroFunction_ChoiceType::GetFuncName() + "(\"data\") = \"prot\"";
            constraints.insert(constraints.begin(), make_pair(kEmptyStr, new_constraint));
        }
    }

    void AddConstraintIgnoreNewText(const string& field, TConstraints& constraints, const CArgumentList& arg_list, const string& target)
    {
        try {
            if (NStr::StartsWith(arg_list[kExistingText].GetValue().get(), "ignore", NStr::eNocase)) {
                string corrected_field = field;
                vector<string> tokens;
                if (field.find(',') != NPOS) {
                    NStr::Split(field, ",", tokens);
                    NMItemData::WrapInQuotes(tokens[0]);
                    NMItemData::WrapInQuotes(tokens[1]);
                    corrected_field = tokens[0] + ", " + tokens[1];
                }
                else if (!NStr::StartsWith(field, "\"")) {
                    NMItemData::WrapInQuotes(corrected_field);
                }

                string target_constr;
                if (!target.empty()) {
                    target_constr = macro::CMacroFunction_RelatedFeatures::GetFuncName();
                    target_constr += "(\"" + target + "\", " + corrected_field + ")";
                }
                
                string new_constraint = string("NOT ") + macro::CMacroFunction_IsPresent::GetFuncName();
                if (!target_constr.empty()) {
                    new_constraint += "(" + target_constr + ")";
                }
                else {
                    new_constraint += "(" + corrected_field + ")";
                }
                
                constraints.insert(constraints.begin(), make_pair(kEmptyStr, new_constraint));
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    string GetUpdatemRNADescription(const CArgument& arg)
    {
        if (arg.GetEnabled() && NStr::EqualNocase(arg.GetValue().get(), "true")) {
            return ", update mRNA product to match protein name";
        }
        return kEmptyStr;
    }

    string GetUpdatemRNAVar(const CArgumentList& arg_list)
    {
        bool update_mrna = arg_list[kUpdatemRNA].GetEnabled() && arg_list[kUpdatemRNA].IsTrue();
        return "\n" + kUpdatemRNA + " = %" + NStr::BoolToString(update_mrna) + "%";
    }

    void AppendUpdatemRNAFnc(bool update_mrna, string& function)
    {
        if (update_mrna) {
            function += "\n" + macro::CMacroFunction_UpdatemRNAProduct::GetFuncName() + "();";
        }
    }

    void OnExistingTextChanged(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kExistingText) {
                const string& existing_text = arg.GetValue();
                if (NStr::EqualNocase(existing_text, "append") ||
                    NStr::EqualNocase(existing_text, "prefix")) {
                    list[kDelimiter].SetEnabled() = true;
                }
                else {
                    list[kDelimiter].SetEnabled() = false;
                }
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnProteinNameSingle(CArgumentList& list, CArgument& arg)
    {
        try {
            bool enable = false;
            if (arg.GetName() == kFeatType) {
                const string& feature = arg.GetValue();
                if (NStr::EqualNocase(feature, "cds") || NStr::EqualNocase(feature, "protein")) {
                    if (list[kFeatQual].GetValue() == "product" || list[kFeatQual].GetValue() == "name") {
                        enable = true;
                    }
                }
                list[kUpdatemRNA].SetEnabled() = enable;
            }
            else if (arg.GetName() == kField) {
                enable = NStr::EqualNocase(arg.GetValue().get(), "protein name");
                if (list.count(kUpdatemRNA) > 0) {
                    list[kUpdatemRNA].SetEnabled() = enable;
                }
                if (list.count(kMoveToNote) > 0) {
                    list[kMoveToNote].SetEnabled() = enable;
                }
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnProteinNameDouble(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kFromField) {
                if (NStr::EqualNocase(arg.GetValue().get(), "protein name")) {
                    list[kUpdatemRNA].SetEnabled() = true;
                }
                else if (NStr::EqualNocase(list[kToField].GetValue().get(), "protein name")) {
                    list[kUpdatemRNA].SetEnabled() = true;
                }
                else {
                    list[kUpdatemRNA].SetEnabled() = false;
                }
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnProductDouble(CArgumentList& list, CArgument& arg)
    {
        try {
            bool enable = false;
            if (arg.GetName() == kFeatType) {
                if (arg.GetValue() == "CDS" || arg.GetValue() == "Protein") {
                    const string& to_field = list[kToField].GetValue();
                    const string& from_field = list[kFromField].GetValue();
                    if (to_field == "product" || to_field == "name" ||
                        from_field == "product" || from_field == "name") {
                        enable = true;
                    }
                    
                }
                list[kUpdatemRNA].SetEnabled() = enable;
            }
        }
        catch (const CException& ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnRnaTypeChanged(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kRNAType) {
                list[kncRNAClass].SetEnabled() = NStr::EqualNocase(arg.GetValue().get(), "ncRNA");
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnControlsAddRNAPanelSelected(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kRNAType) {
                const string& rna_name = arg.GetValue();
                if (NStr::EqualNocase(rna_name, "miscRNA")) {
                    list["miscRNA_comment1"].SetShown() = true;
                    list["miscRNA_comment2"].SetShown() = true;
                }
                else {
                    list["miscRNA_comment1"].SetShown() = false;
                    list["miscRNA_comment2"].SetShown() = false;
                }

                if (NStr::EqualNocase(rna_name, "tRNA")
                    || NStr::EqualNocase(rna_name, "mRNA")
                    || NStr::EqualNocase(rna_name, "ncRNA")
                    || NStr::EqualNocase(rna_name, "miscRNA")) {
                    list["gene_symbol"].SetShown() = true;
                    list["gene_descr"].SetShown() = true;
                    list["gene_symbol_static"].SetShown() = true;
                    list["gene_descr_static"].SetShown() = true;
                }
                else {
                    list["gene_symbol"].SetShown() = false;
                    list["gene_descr"].SetShown() = false;
                    list["gene_symbol_static"].SetShown() = false;
                    list["gene_descr_static"].SetShown() = false;
                }
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnRemoveFromParsedField(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kRmvParse) {
                if (arg.GetValue() == "true") {
                    bool value = (list[kTextLeft].IsTrue() && !list[kDelLeft].Empty())
                        || list[kDigitsLeft].IsTrue()
                        || list[kLetterLeft].IsTrue();
                    list[kRmvLeft].SetEnabled() = value;

                    value = (list[kTextRight].IsTrue() && !list[kDelRight].Empty())
                        || list[kDigitsRight].IsTrue()
                        || list[kLetterRight].IsTrue();
                    list[kRmvRight].SetEnabled() = value;
                }
                else {
                    list[kRmvLeft].SetEnabled() = false;
                    list[kRmvRight].SetEnabled() = false;
                }
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnLeftDigitsLettersInParsePanels(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kTextLeft) {
                list[kDelLeft].SetEnabled() = (arg.GetValue() == "true");
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnRightDigitsLettersInParsePanels(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kTextRight) {
                list[kDelRight].SetEnabled() = (arg.GetValue() == "true");
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnBeforeCheck(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kBefore) {
                bool value = (arg.GetValue() == "true");
                list[kTextLeft].SetEnabled() = value;
                list[kDigitsLeft].SetEnabled() = value;
                list[kLetterLeft].SetEnabled() = value;
                list[kDelLeft].SetEnabled() = value && list[kTextLeft].IsTrue();
                list[kRmvLeft].SetEnabled() = value;
            }
        }
        catch (const CException& ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnAfterCheck(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kAfter) {
                bool value = (arg.GetValue() == "true");
                list[kTextRight].SetEnabled() = value;
                list[kDigitsRight].SetEnabled() = value;
                list[kLetterRight].SetEnabled() = value;
                list[kDelRight].SetEnabled() = value && list[kTextRight].IsTrue();
                list[kRmvRight].SetEnabled() = value;
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnStructCommFieldChanged(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kField) {
                list[kStrCommField].SetShown() = (arg.GetValue() == "Field");
                if (list.count(kTableEnable) > 0) {
                    bool enable = NStr::EqualNocase(arg.GetValue().get(), "Field name") ? false : true;
                    list[kTableEnable].SetEnabled() = enable;
                }
            } 
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnStructCommFieldDblChanged(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kToField) {
                list[kStrCommField].SetShown() = (arg.GetValue() == "Field");
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnConvertFeatFromCDS(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kFromFeat) {
                bool value = (arg.GetValue() == "CDS");
                list[kRemmRNA].SetEnabled() = value;
                list[kRemGene].SetEnabled() = value;
                list[kRemTranscriptid].SetEnabled() = value;
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnConvertFeatToChanged(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kToFeat) {
                bool show_choices = (arg.GetValue() == "ncRNA" || arg.GetValue() == "bond" || arg.GetValue() == "site");
                list[kSelText].SetShown() = show_choices;
                list[kChoices].SetShown() = show_choices;
                list[kCreateOnProtein].SetShown() = (arg.GetValue() == "region");
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnSelectTable(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kTableEnable) {
                bool checked = (arg.GetEnabled()) ? arg.IsTrue() : false;
                list[kTableName].SetShown() = checked;
                list[kTableLabel].SetShown() = checked;
                list[kTableCol].SetShown() = checked;
                list[kColLabel].SetShown() = checked;
                list[kOpenFile].SetShown() = checked;
                list[kNewValue].SetEnabled() = !checked;
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnSelectFeature(CArgumentList& list, CArgument& arg)
    {
        try {
            if (list.count(kRmvOverlapGene) > 0 && arg.GetName() == kFeatType) {
                bool enable = true;
                const string val = arg.GetValue();
                if (NStr::EqualNocase(val, "all") || NStr::EndsWith(val, " AA")) {
                    enable = false;
                }
                list[kRmvOverlapGene].SetEnabled() = enable;
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnMiscFeatRuleSelected(CArgumentList& list, CArgument& arg)
    {
        try {
            const CTempString arg_name = macro::CMacroFunction_Autodef::sm_Arguments[2].m_Name;
            if (arg.GetName() == arg_name) {
                list["misc_feat_rule"].SetEnabled() = (arg.GetValue() == "true");
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    void OnIllegalDbXrefSelected(CArgumentList& list, CArgument& arg)
    {
        try {
            if (arg.GetName() == kIllegalDbxref) {
                bool enable = arg.IsTrue() ? true : false;
                list[kMoveToNote].SetEnabled() = enable;
            }
        }
        catch (const CException& ex) {
            NcbiMessageBox(ex.what());
            return;
        }
    }

    EMacroFieldType GetSourceTypeInSinglePanel(const CArgumentList& arg_list)
    {
        EMacroFieldType type = EMacroFieldType::eNotSet;
        try {
            if (arg_list[kBsrcText].IsTrue()) {
                type = EMacroFieldType::eBiosourceText;
            }
            else if (arg_list[kBsrcTax].IsTrue()) {
                type = EMacroFieldType::eBiosourceTax;
            }
            else if (arg_list[kBsrcLoc].IsTrue()) {
                type = EMacroFieldType::eBiosourceLocation;
            }
            else if (arg_list[kBsrcOrigin].IsTrue()) {
                type = EMacroFieldType::eBiosourceOrigin;
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return type;
        }
        
        return type;
    }

    EMacroFieldType GetSourceTypeInDoublePanel(const CArgumentList& arg_list, EDirType dir)
    {
        EMacroFieldType type = EMacroFieldType::eNotSet;
        try {
            if (dir == eFrom) {
                if (arg_list[kBsrcTextFrom].IsTrue()) {
                    type = EMacroFieldType::eBiosourceText;
                }
                else if (arg_list[kBsrcTaxFrom].IsTrue()) {
                    type = EMacroFieldType::eBiosourceTax;
                }
            }
            else if (dir == eTo) {
                if (arg_list[kBsrcTextTo].IsTrue()) {
                    type = EMacroFieldType::eBiosourceText;
                }
                else if (arg_list[kBsrcTaxTo].IsTrue()) {
                    type = EMacroFieldType::eBiosourceTax;
                }
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
            return type;
        }
        return type;
    }

    string GetSelectedCapType(const CArgumentList& arg_list)
    {
        string cap_change;
        try {
            if (arg_list["no_change"].IsTrue()) {
                cap_change = "none";
            }
            else if (arg_list["to_upper"].IsTrue()) {
                cap_change = "toupper";
            }
            else if (arg_list["to_lower"].IsTrue()) {
                cap_change = "tolower";
            }
            else if (arg_list["fcap_nochange"].IsTrue()) {
                cap_change = "firstcap-restnochange";
            }
            else if (arg_list["cap_atspaces"].IsTrue()) {
                cap_change = "cap-word-space";
            }
            else if (arg_list["fcap_lower"].IsTrue()) {
                cap_change = "firstcap";
            }
            else if (arg_list["flower_nochange"].IsTrue()) {
                cap_change = "firstlower-restnochange";
            }
            else if (arg_list["cap_atspaces_punct"].IsTrue()) {
                cap_change = "cap-word-space-punct";
            }
        }
        catch (const CException &ex) {
            NcbiMessageBox(ex.what());
        }

        _ASSERT(!cap_change.empty());
        if (cap_change.empty()) {
            LOG_POST(Error << "No capitalization was selected");
        }
        return cap_change;
    }

} // end of namespace

END_NCBI_SCOPE
