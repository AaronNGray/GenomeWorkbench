/*  $Id: field_name_panel.cpp 44895 2020-04-09 18:42:21Z asztalos $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/objutils/macro_biodata.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

vector<string> CFieldNamePanel::GetChoices(bool& allow_other)
{
    vector<string> choices;
    return choices;
}

vector<string> CFieldNamePanel::GetMacroValues()
{
    vector<string> values;
    return values;
}

void CFieldNamePanel::x_UpdateParent()
{
    wxWindow* w = this->GetParent();
    while (w != NULL) {
        CFieldNamePanelParent* parent = dynamic_cast<CFieldNamePanelParent*>(w);
        if (parent) {
            parent->UpdateEditor();
        } 
        w = w->GetParent();
    }
}


bool CFieldNamePanel::x_IsParentAECRDlg()
{
    return false;
}

void CFieldNamePanel::x_UpdateAECRDlgLayout()
{
}

bool CFieldNamePanel::x_ShouldDecreaseStrConstrPanel()
{
//    wxWindow* w = this->GetParent();
//    while (w) {
//        CEditPubDlg* epubdlg = dynamic_cast<CEditPubDlg*>(w);
//        if (epubdlg) {
//            return true;
//        }
//        w = w->GetParent();
//    }
    return false;
}

string CFieldNamePanel::GetFieldTypeName(EFieldType field_type)
{
    string val = "";
    switch (field_type) {
        case eFieldType_Unknown:
            // leave blank;
            break;
        case eFieldType_Source:
            val = "Source Qualifier";
            break;
        case eFieldType_Taxname:
            val = "Taxname";
            break;
        case eFieldType_Feature:
            val = "Feature Qualifier";
            break;
        case eFieldType_CDSGeneProt:
            val = "CDS-Gene-Prot Qualifier";
            break;
        case eFieldType_RNA:
            val = "RNA Qualifier";
            break;
        case eFieldType_Gene:
            val = "Gene Field";
            break;
        case eFieldType_Protein:
            val = "Protein Field";
            break;
        case eFieldType_MolInfo:
            val = "MolInfo Qualifier";
            break;
        case eFieldType_Pub:
            val = "Pub Qualifier";
            break;
        case eFieldType_StructuredComment:
            val = "Structured Comment";
            break;
        case eFieldType_DBLink:
            val = "DBLink Field";
            break;
        case eFieldType_Dbxref:
            val = "Dbxref";
            break;
        case eFieldType_Misc:
            val = "Misc Qualifiers";
            break;
        case eFieldType_BankITComment:
            val = kBankITComment;
            break;
        case eFieldType_Comment:
            val = kComment;
            break;
        case eFieldType_DefLine:
            val = kDefinitionLineLabel;
            break;
        case eFieldType_TaxnameAfterBinomial:
            val = kTaxnameAfterBinomial;
            break;
        case eFieldType_FlatFile:
            val = kFlatFile;
            break;
        case eFieldType_FileId:
            val = kFileSeqId;
            break;
        case eFieldType_GeneralId:
            val = kGeneralId;
            break;
        case eFieldType_LocalId:
            val = kLocalId;
            break;
        case eFieldType_SeqId:
            val = kFieldTypeSeqId;
            break;
    }
    return val;
}


CFieldNamePanel::EFieldType CFieldNamePanel::GetFieldTypeFromName(const string & field_type_name)
{
    EFieldType val = eFieldType_Unknown;
    if (NStr::StartsWith(field_type_name, "Source", NStr::eNocase)) {
        val = eFieldType_Source;
    } else if (NStr::StartsWith(field_type_name, kTaxnameAfterBinomial, NStr::eNocase)) {
        val = eFieldType_TaxnameAfterBinomial;
    } else if (NStr::StartsWith(field_type_name, "Taxname", NStr::eNocase)) {
        val = eFieldType_Taxname;
    } else if (NStr::StartsWith(field_type_name, "Feature", NStr::eNocase)) {
        val = eFieldType_Feature;
    } else if (NStr::StartsWith(field_type_name, "CDS", NStr::eNocase)) {
        val = eFieldType_CDSGeneProt;
    } else if (NStr::StartsWith(field_type_name, "RNA", NStr::eNocase)) {
        val = eFieldType_RNA;
    } else if (NStr::EqualNocase(field_type_name, "Gene field")) {
        val = eFieldType_Gene;
    } else if (NStr::EqualNocase(field_type_name, "Protein field")) {
        val = eFieldType_Protein;
    }
    else if (NStr::StartsWith(field_type_name, "Mol", NStr::eNocase)) {
        val = eFieldType_MolInfo;
    } else if (NStr::StartsWith(field_type_name, "Pub", NStr::eNocase)) {
        val = eFieldType_Pub;
    } else if (NStr::StartsWith(field_type_name, "author ", NStr::eNocase) || NStr::EqualNocase(field_type_name, "journal") ||
               NStr::EqualNocase(field_type_name, "volume")  || NStr::EqualNocase(field_type_name, "issue")   ||
               NStr::EqualNocase(field_type_name, "pages")   || NStr::EqualNocase(field_type_name, "title") ||
               NStr::EqualNocase(field_type_name, "pmid") || NStr::EqualNocase(field_type_name, "status")) {
        val = eFieldType_Pub;
    } else if (NStr::StartsWith(field_type_name, "Structured", NStr::eNocase)) {
        val = eFieldType_StructuredComment;
    } else if (NStr::StartsWith(field_type_name, "DBLink", NStr::eNocase)) {
        val = eFieldType_DBLink;
    } else if (NStr::StartsWith(field_type_name, "Dbxref", NStr::eNocase)) {
        val = eFieldType_Dbxref;
    } else if (NStr::StartsWith(field_type_name, "Misc", NStr::eNocase)) {
        val = eFieldType_Misc;
    } else if (NStr::StartsWith(field_type_name, kDefinitionLineLabel, NStr::eNocase)) {
        val = eFieldType_DefLine;
    } else if (NStr::EqualNocase(field_type_name, kBankITComment)) {
        val = eFieldType_BankITComment;
    } else if (NStr::EqualNocase(field_type_name, kComment)) {
        val = eFieldType_Comment;
    } else if (NStr::EqualNocase(field_type_name, kFlatFile)) {
        val = eFieldType_FlatFile;
    } else if (NStr::EqualNocase(field_type_name, kFileSeqId)) {
        val = eFieldType_FileId;
    } else if (NStr::EqualNocase(field_type_name, kGeneralId)) {
        val = eFieldType_GeneralId;
    } else if (NStr::EqualNocase(field_type_name, kLocalId)) {
        val = eFieldType_LocalId;
    } else if (NStr::EqualNocase(field_type_name, kFieldTypeSeqId)
               || NStr::EqualNocase(field_type_name, "accession")) {
        val = eFieldType_SeqId;
    }
    return val;
}

bool CFieldNamePanel::IsFeature(const string& target)
{
    return target == macro::CMacroBioData::sm_CdRegion ||
        target == macro::CMacroBioData::sm_Gene ||
        target == macro::CMacroBioData::sm_ImpFeat ||
        target == macro::CMacroBioData::sm_Miscfeat ||
        target == macro::CMacroBioData::sm_miscRNA ||
        target == macro::CMacroBioData::sm_mRNA ||
        target == macro::CMacroBioData::sm_rRNA ||
        target == macro::CMacroBioData::sm_RNA ||
        target == macro::CMacroBioData::sm_Protein ||
        target == macro::CMacroBioData::sm_SeqFeat;
}

bool CFieldNamePanel::IsDescriptor(const string& target)
{
    return target == macro::CMacroBioData::sm_Seqdesc ||
        target == macro::CMacroBioData::sm_BioSource ||
        target == macro::CMacroBioData::sm_MolInfo ||
        target == macro::CMacroBioData::sm_UserObject ||
        target == macro::CMacroBioData::sm_DBLink ||
        target == macro::CMacroBioData::sm_StrComm ||
        target == macro::CMacroBioData::sm_Pubdesc;
}

END_NCBI_SCOPE


