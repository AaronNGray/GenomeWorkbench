/*  $Id: edit_object_feature_propagate.cpp 42666 2019-04-01 14:27:25Z filippov $
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
 * Authors: Colleen Bollin, based on a file by Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <objects/seqfeat/Feat_id.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/cleanup/fix_feature_id.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/cmd_propagate_feat.hpp>
#include <gui/widgets/edit/edit_object_feature_propagate.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp> 

#include <wx/panel.h>
#include <wx/treebook.h>
#include <wx/sizer.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CEditObjectFeaturePropagate::CEditObjectFeaturePropagate(CBioseq_Handle bsh, const vector<CConstRef<CSeq_feat>>& propagatedFeats)
    : m_Bsh(bsh), m_PropagatedFeats(propagatedFeats), m_Window(0)
{
    xInit();
}

wxWindow* CEditObjectFeaturePropagate::CreateWindow(wxWindow* parent)
{
    if (m_Window) {
        NCBI_THROW(CException, eUnknown, 
           "CEditObjectFeaturePropagate::CreateWindow - Window already created!");
    }

    //Populate GUI:
    m_Window = new wxPanel(parent);
    wxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);
    m_Window->SetSizer(pTopSizer);

    //pTopSizer->Add(xFeatureSelectSizer(), 0, wxEXPAND);
    pTopSizer->Add(xAlignSizer(), 0, wxEXPAND);
    pTopSizer->Add(xGapHandlingSizer(), 0, wxEXPAND);
    pTopSizer->Add(xCdsHandlingSizer(), 0, wxEXPAND);
    pTopSizer->Fit(m_Window);

    return m_Window;
}

static const char* kAllSequences = "All sequences";

IEditCommand* CEditObjectFeaturePropagate::GetEditCommand()
{
    if (!m_Window) {
        return 0;
    }
    if (m_EditAction) {
        return m_EditAction.GetPointer();
    }

    string alignName = ToStdString(m_pAlignCtrl->GetStringSelection());
    m_pAlign.Reset(m_AlignMap[alignName].GetPointer());
    m_MaxFeatId = CFixFeatureId::s_FindHighestFeatureId(m_Bsh.GetTopLevelEntry());
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_Bsh.GetTopLevelEntry());
    string seqName = ToStdString(m_pTargetCtrl->GetStringSelection());
    if (NStr::EqualNocase(seqName, kAllSequences)) {
        xGetEditCommandAllSequences(alignName, create_general_only);
    }
    else {
        CBioseq_Handle bsh = m_SeqMap[alignName][seqName];
        xGetEditCommandSingleSequence(bsh, create_general_only);
    }

    xReportErrors();

    return (m_EditAction) ? m_EditAction.GetPointer() : NULL;
}

void CEditObjectFeaturePropagate::xGetEditCommandAllSequences(const string& alignName, bool create_general_only)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Propagate features to all sequences"));
    CStopWatch sw;
    sw.Start();

    for (auto&& it : m_SeqMap[alignName]) {
        CBioseq_Handle bsh = it.second;
        if (!bsh  ||  bsh == m_Bsh) {
            continue;
        }
        xPropagateFeatures(bsh, composite, create_general_only);
    }

    LOG_POST(Info << "Time to construct the feature propagation command: " << sw.Elapsed() << "s");
    m_EditAction.Reset(composite);
}

void CEditObjectFeaturePropagate::xGetEditCommandSingleSequence(CBioseq_Handle bsh, bool create_general_only)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Propagate features to one sequence"));
    xPropagateFeatures(bsh, composite, create_general_only);

    m_EditAction.Reset(composite);
}


void CEditObjectFeaturePropagate::xReportErrors()
{
    if (m_Listener.Count() > 0) {
        string msg = "";
        for (size_t i = 0; i < m_Listener.Count(); i++) {
            msg += m_Listener.GetMessage(i).GetText() + "\n";
        }
        CGenericReportDlg* report = new CGenericReportDlg(NULL);
        report->SetTitle(wxT("Unable to propagate"));
        report->SetText(ToWxString(msg));
        report->Show(true);
        report->SetFocus();
    }
    m_Listener.Clear();
}


void CEditObjectFeaturePropagate::xPropagateFeatures(CBioseq_Handle target_bsh, CRef<CCmdComposite>& composite, bool create_general_only)
{
    PropagateToTarget(
        composite, 
        m_PropagatedFeats, 
        m_Bsh, 
        target_bsh, 
        *m_pAlign, 
        xStopCdsAtStopCodon(), 
        xCdsCleanupPartials(), 
        xMergeAbuttingIntervals(), 
        xExtendOverGaps(),
        create_general_only,
        &m_Listener, 
        &m_MaxFeatId);
}

wxSizer* CEditObjectFeaturePropagate::xAlignSizer()
{
    wxFlexGridSizer* flexSizer = new wxFlexGridSizer(0, 2, 0, 0);
    flexSizer->AddGrowableCol(1);

    wxStaticText* pAlignLabel = new wxStaticText(m_Window, -1, "Use alignment:");
    flexSizer->Add(pAlignLabel, 0, wxALL, 4);
    m_pAlignCtrl = new wxComboBox(m_Window, ID_ALIGNCTRL);
    m_pAlignCtrl->Bind(wxEVT_COMBOBOX, &CEditObjectFeaturePropagate::OnAlignmentSelected, this);
    wxArrayString alignNames;
    for (auto&& it : m_AlignMap) {
        alignNames.Add(ToWxString(it.first));
    }
    m_pAlignCtrl->Append(alignNames);
    m_pAlignCtrl->SetSelection(0);
    flexSizer->Add(m_pAlignCtrl, 1, wxEXPAND | wxALL, 4);


    wxStaticText* pTargetLabel = new wxStaticText(m_Window, -1, "Propagation target:");
    flexSizer->Add(pTargetLabel, 0, wxALL, 4);
    m_pTargetCtrl = new wxComboBox(m_Window, ID_TARGETCTRL);
    x_FillTargetComboList();
    flexSizer->Add(m_pTargetCtrl, 1, wxEXPAND | wxALL, 4);
    
    return flexSizer;
}

void CEditObjectFeaturePropagate::x_FillTargetComboList()
{
    _ASSERT(m_pTargetCtrl && m_pAlignCtrl);

    string alignName = ToStdString(m_pAlignCtrl->GetStringSelection());
    wxArrayString seqNames;
    seqNames.Add(ToWxString(kAllSequences));
    for (auto&& it : m_SeqMap[alignName]) {
        seqNames.Add(ToWxString(it.first));
    }
    m_pTargetCtrl->Clear();
    m_pTargetCtrl->Append(seqNames);
    m_pTargetCtrl->SetSelection(0);
}

void CEditObjectFeaturePropagate::OnAlignmentSelected(wxCommandEvent& event)
{
    x_FillTargetComboList();
}

wxSizer* CEditObjectFeaturePropagate::xFeatureSelectSizer()
{
    wxArrayString labels;
    labels.Add("All features");
    labels.Add("Selected feature");
    m_pFeatureSelectCtrl = new wxRadioBox(m_Window, -1, "Feature Selection:", 
        wxDefaultPosition, wxDefaultSize, labels, 2);
    m_pFeatureSelectCtrl->SetSelection(0);
    wxSizer* pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_pFeatureSelectCtrl, 1, wxALL|wxEXPAND, 4);
    return pSizer;
}

wxSizer* CEditObjectFeaturePropagate::xGapHandlingSizer()
{
    wxArrayString labels;
    labels.Add("Extend over gaps");
    labels.Add("Split at gaps");
    m_pGapHandlingCtrl = new wxRadioBox(m_Window, -1, "Gap Handling:", 
        wxDefaultPosition, wxDefaultSize, labels, 2);
    m_pGapHandlingCtrl->SetSelection(0);
    wxSizer* pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->Add(m_pGapHandlingCtrl, 1, wxALL|wxEXPAND, 4);
    return pSizer;
}

wxSizer* CEditObjectFeaturePropagate::xCdsHandlingSizer()
{
    wxSizer* pSizer = new wxBoxSizer(wxVERTICAL);
    //pSizer->AddSpacer(8);

    m_pStopCdsAtStopCodon = new wxCheckBox(m_Window, -1,
        "Stop CDS translation at internal stop codon");
    m_pStopCdsAtStopCodon->SetValue(false);
    m_pStopCdsAtStopCodon->Enable(true);
    pSizer->Add(m_pStopCdsAtStopCodon, 1, wxALL|wxEXPAND, 4);

    //m_pExtendCdsPastPartialThreePrime = new wxCheckBox(m_Window, -1,
    //    "Translate CDS after partial 3\' boundary");
    //m_pExtendCdsPastPartialThreePrime->SetValue(false);
    //m_pExtendCdsPastPartialThreePrime->Enable(true);
    //pSizer->Add(m_pExtendCdsPastPartialThreePrime, 1, wxALL|wxEXPAND, 4);

    m_pCleanupCdsPartials = new wxCheckBox(m_Window, -1,
        "Cleanup CDS partials after propagation");
    m_pCleanupCdsPartials->SetValue(true);
    m_pCleanupCdsPartials->Enable(true);
    m_pCleanupCdsPartials->Enable(true);
    pSizer->Add(m_pCleanupCdsPartials, 1, wxALL|wxEXPAND, 4);

    m_pFuseAdjacentIntervals = new wxCheckBox(m_Window, -1,
        "Fuse adjacent propagated intervals");
    m_pFuseAdjacentIntervals->SetValue(false);
    m_pFuseAdjacentIntervals->Enable(true);
    pSizer->Add(m_pFuseAdjacentIntervals, 1, wxALL|wxEXPAND, 4);

    //pSizer->AddSpacer(8);
    return pSizer;
}

bool CEditObjectFeaturePropagate::xStopCdsAtStopCodon() const
{
    if (!m_pStopCdsAtStopCodon) {
        return false;
    }
    return m_pStopCdsAtStopCodon->GetValue();
}


bool CEditObjectFeaturePropagate::xMergeAbuttingIntervals() const
{
    if (!m_pFuseAdjacentIntervals) {
        return false;
    }
    return m_pFuseAdjacentIntervals->GetValue();
}

bool CEditObjectFeaturePropagate::xExtendOverGaps() const
{
    if (!m_pGapHandlingCtrl) {
        return true;
    }
    return m_pGapHandlingCtrl->GetSelection() == 0;
}

bool CEditObjectFeaturePropagate::xCdsCleanupPartials() const
{
    if (!m_pCleanupCdsPartials) {
        return true;
    }
    return m_pCleanupCdsPartials->GetValue();
}


void CEditObjectFeaturePropagate::xInit()
{
    CSeq_entry_Handle seh = m_Bsh.GetTopLevelEntry();
    if (!seh.IsSet()) {
        return;
    }

    CRef<CSeq_loc> loc = m_Bsh.GetRangeSeq_loc(0,0);

    for (CAlign_CI aln_it(m_Bsh.GetScope(), *loc); aln_it; ++aln_it) {
        const CSeq_align& align = aln_it.GetOriginalSeq_align();
        if (!align.GetSegs().IsDenseg()) {
            // for now, only handle dense-seg type of alignments
            continue;
        }
        
        CSeq_align_Handle alh = aln_it.GetSeq_align_Handle();
        const CSeq_annot_Handle& annot = alh.GetAnnot();
        string name = "From the set";
        if (annot.IsNamed()) {
            name = annot.GetName();
        }
        
        m_AlignMap.emplace(name, ConstRef(&align));
    }

    if (m_AlignMap.empty()) {
        return;
    }

    //Populate the target combobox with the ID strings of all sibling sequences
    // of m_Bsh
    //
    CScope& scope = m_Bsh.GetScope();
    for (auto&& it : m_AlignMap) {
        const CSeq_align& align = it.second.GetObject();

        CSeq_align::TDim dim = align.CheckNumRows();
        TIdToBSeqMap id_bshMap;
        for (CSeq_align::TDim row = 0; row < dim; ++row) {
            const CSeq_id& row_id = align.GetSeq_id(row);
            CBioseq_Handle bsh = scope.GetBioseqHandle(row_id);
            if (bsh == m_Bsh) {
                continue;
            }
            CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*bsh.GetSeqId());
            CSeq_id_Handle best_idh = sequence::GetId(idh, scope, sequence::eGetId_Best);
            if (best_idh) {
                idh = best_idh;
            }
            string label = idh.GetSeqId()->GetSeqIdString(true);
            id_bshMap.emplace(label, bsh);
        }
        m_SeqMap.emplace(it.first, id_bshMap);
    }
}

END_NCBI_SCOPE
