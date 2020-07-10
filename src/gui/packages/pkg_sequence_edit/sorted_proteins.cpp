/*  $Id: sorted_proteins.cpp 36969 2016-11-18 19:13:24Z asztalos $
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
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/util/sequence.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/sorted_proteins.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CSortedProteins::CSortedProteins(const CSeq_entry_Handle& seh)
    : m_Seh(seh)
{
}


void CSortedProteins::View()
{
    x_GatherProteinNames();
    x_SortProteinNames();

    string report;
    auto iter = m_Proteins.begin();
    auto prev = iter;
    for (; iter != m_Proteins.end(); ++iter) {
        if (prev != iter && !NStr::EqualCase(iter->second, prev->second)) {
            report += "\n";
        }

        CNcbiOstrstream oss;
        CFastaOstream fasta_ostr(oss);
        fasta_ostr.Write(iter->first);
        report += CNcbiOstrstreamToString(oss);
        prev = iter;
    }

    CGenericReportDlg* reportdlg = new CGenericReportDlg(NULL);
    reportdlg->SetTitle("Sorted Proteins");
    reportdlg->SetText(ToWxString(report));
    reportdlg->Show(true);
}

void CSortedProteins::x_GatherProteinNames()
{
    m_Proteins.clear();

    for (CBioseq_CI b_iter(m_Seh, CSeq_inst::eMol_aa); b_iter; ++b_iter) {
        CFeat_CI feat_it(*b_iter, SAnnotSelector(CSeqFeatData::eSubtype_prot));
        if (feat_it) {
            const CProt_ref& prot = feat_it->GetData().GetProt();
            if (prot.IsSetName() && !prot.GetName().empty()) {
                m_Proteins.emplace_back(*b_iter, prot.GetName().front());
            }
            else {
                string feat_label;
                feature::GetLabel(*feat_it->GetOriginalSeq_feat(), &feat_label, feature::eBoth);
                m_Proteins.push_back(make_pair(*b_iter, feat_label));
            }
        }
        else {
            m_Proteins.push_back(make_pair(*b_iter, kEmptyStr));
        }
    }
}

void CSortedProteins::x_SortProteinNames()
{
    _ASSERT(!m_Proteins.empty());
    m_Proteins.sort([](const pair<CBioseq_Handle, string>& prot1, const pair<CBioseq_Handle, string>& prot2) { return NStr::CompareCase(prot1.second, prot2.second) < 0; });
}

END_NCBI_SCOPE
