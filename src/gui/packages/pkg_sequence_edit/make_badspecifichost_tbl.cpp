/*  $Id: make_badspecifichost_tbl.cpp 34744 2016-02-05 18:16:08Z filippov $
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

#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objtools/writers/write_util.hpp>
#include <objtools/validator/utilities.hpp>

#include <gui/widgets/edit/generic_report_dlg.hpp>  
#include <gui/packages/pkg_sequence_edit/make_badspecifichost_tbl.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/msgdlg.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


void CMakeBadSpecificHostTable::MakeTable(const CSeq_entry_Handle& seh)
{
    x_Init();
    for (CBioseq_CI b_iter(seh, CSeq_inst::eMol_na); b_iter; ++b_iter) {
        string seq_id;
        CWriteUtil::GetBestId(CSeq_id_Handle::GetHandle(*(b_iter->GetCompleteBioseq()->GetFirstId())), seh.GetScope(), seq_id);
        const CBioSource* bsrc = sequence::GetBioSource(*b_iter);
        if (bsrc) {
            x_CheckSpecificHosts(*bsrc, seq_id);
        }
    }

    for (CFeat_CI feat_it(seh, CSeqFeatData::e_Biosrc); feat_it; ++feat_it) {
        string seq_id;
        CBioseq_Handle bsh = seh.GetScope().GetBioseqHandle(feat_it->GetLocation());
        CWriteUtil::GetBestId(CSeq_id_Handle::GetHandle(*(bsh.GetCompleteBioseq()->GetFirstId())), seh.GetScope(), seq_id);
        x_CheckSpecificHosts(feat_it->GetData().GetBiosrc(), seq_id);
        
    }

    if (m_Ambiguous.empty() && m_Misspelled.empty() && m_Badcap.empty() && m_Unrecognized.empty()) {
        wxMessageBox(ToWxString("No bad specific-host values found!"), ToWxString("Message"),
            wxOK | wxICON_INFORMATION);
        return;
    }


    CNcbiOstrstream lines;
    if (!m_Misspelled.empty()) {
        lines << "Mis-spelled Specific-Host Values\n";
        x_AppendToTable(lines, m_Misspelled);
    }
    if (!m_Badcap.empty()) {
        lines << "Incorrectly Capitalized Specific-Host Values\n";
        x_AppendToTable(lines, m_Badcap);
    }
    if (!m_Ambiguous.empty()) {
        lines << "Ambiguous Specific-Host Values\n";
        x_AppendToTable(lines, m_Ambiguous);
    }
    if (!m_Unrecognized.empty()) {
        lines << "Unrecognized Specific-Host Values\n";
        x_AppendToTable(lines, m_Unrecognized);
    }

    CGenericReportDlg* report = new CGenericReportDlg(NULL);
    report->SetTitle(ToWxString("Bad Specific-Host Values"));
    report->SetText(ToWxString(CNcbiOstrstreamToString(lines)));
    report->Show(true);

}

void CMakeBadSpecificHostTable::x_Init()
{
    m_Ambiguous.clear();
    m_Misspelled.clear();
    m_Badcap.clear();
    m_Unrecognized.clear();
}

void CMakeBadSpecificHostTable::x_CheckSpecificHosts(const objects::CBioSource& bsrc, const string& seq_id)
{
    FOR_EACH_ORGMOD_ON_BIOSOURCE(orgmod, bsrc) {
        if ((*orgmod)->IsSetSubtype()
            && (*orgmod)->GetSubtype() == COrgMod::eSubtype_nat_host
            && (*orgmod)->IsSetSubname()) {

            string error_msg;
            bool valid = validator::IsSpecificHostValid((*orgmod)->GetSubname(), error_msg);
            if (!valid) {
                x_ParseErrorMessage((*orgmod)->GetSubname(), error_msg, seq_id);
            }
        }
    }
}

void CMakeBadSpecificHostTable::x_ParseErrorMessage(const string& host, const string& error, const string& seq_id)
{
    string prefixed_host = seq_id + "\t" + host;
    if (NStr::FindNoCase(error, "ambiguous") != NPOS) {
        m_Ambiguous.push_back(prefixed_host);
    }
    else if (NStr::FindNoCase(error, "misspelled") != NPOS) {
        m_Misspelled.push_back(prefixed_host);
    }
    else if (NStr::FindNoCase(error, "incorrectly capitalized") != NPOS) {
        m_Badcap.push_back(prefixed_host);
    }
    else if (NStr::FindNoCase(error, "invalid value") != NPOS) {
        m_Unrecognized.push_back(prefixed_host);
    }
}

void CMakeBadSpecificHostTable::x_AppendToTable(CNcbiOstrstream& text, const vector<string>& hosts)
{
    ITERATE(vector<string>, it, hosts) {
        text << *it << NcbiEndl;
    }
}


END_NCBI_SCOPE

