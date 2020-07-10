#ifndef GUI_WIDGETS_EDIT___EDIT_OBJECT_FEATURE_PROPAGATE__HPP
#define GUI_WIDGETS_EDIT___EDIT_OBJECT_FEATURE_PROPAGATE__HPP

/*  $Id: edit_object_feature_propagate.hpp 42666 2019-04-01 14:27:25Z filippov $
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

#include <corelib/ncbistd.hpp>

#include <objects/general/Object_id.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/edit/edit_object.hpp>

#include <wx/arrstr.h>

class wxPanel;
class wxSizer;
class wxComboBox;
class wxCheckBox;
class wxRadioBox;
class wxCommandEvent;

BEGIN_NCBI_SCOPE


class CEditObjectFeaturePropagate : public CObject, public IEditObject
{
public:
    CEditObjectFeaturePropagate(objects::CBioseq_Handle bsh, const vector<CConstRef<objects::CSeq_feat>>& propagatedFeats);

    virtual wxWindow* CreateWindow(wxWindow* parent);
    virtual IEditCommand* GetEditCommand();

private:
    void xInit();

    void xGetEditCommandAllSequences(const string& alignName, bool create_general_only);
    void xGetEditCommandSingleSequence(objects::CBioseq_Handle bsh, bool create_general_only);
    void xPropagateFeatures(objects::CBioseq_Handle target_bsh, CRef<CCmdComposite>& composite, bool create_general_only);
    void xReportErrors();

    wxSizer* xAlignSizer();
    wxSizer* xFeatureSelectSizer();
    wxSizer* xGapHandlingSizer();
    wxSizer* xCdsHandlingSizer();

    void OnAlignmentSelected(wxCommandEvent& event);
    void x_FillTargetComboList();

    bool xStopCdsAtStopCodon() const;
    bool xMergeAbuttingIntervals() const;
    bool xExtendOverGaps() const;
    bool xCdsCleanupPartials() const;

    // sequence from where we are going to propagate features
    objects::CBioseq_Handle m_Bsh; 

    CIRef<IEditCommand> m_EditAction;
    CConstRef<objects::CSeq_align> m_pAlign;
    vector<CConstRef<objects::CSeq_feat>> m_PropagatedFeats;

    typedef map<string, CConstRef<objects::CSeq_align>> TAnnotNameToAlignMap;
    TAnnotNameToAlignMap m_AlignMap;

    typedef map<string, objects::CBioseq_Handle> TIdToBSeqMap;
    typedef map<string, TIdToBSeqMap> TAnnotNameToSequencesMap;
    TAnnotNameToSequencesMap m_SeqMap;

    wxPanel* m_Window;
    wxComboBox* m_pTargetCtrl;
    wxComboBox* m_pAlignCtrl;
    wxRadioBox* m_pFeatureSelectCtrl;
    wxRadioBox* m_pGapHandlingCtrl;
    wxCheckBox* m_pStopCdsAtStopCodon;
    wxCheckBox* m_pExtendCdsPastPartialThreePrime;
    wxCheckBox* m_pCleanupCdsPartials;
    wxCheckBox* m_pFuseAdjacentIntervals;

    enum {
        ID_ALIGNCTRL = 10500,
        ID_TARGETCTRL
    };

    CMessageListener_Basic m_Listener;
    objects::CObject_id::TId m_MaxFeatId{ 0 };
};

END_NCBI_SCOPE

#endif  
    // GUI_WIDGETS_EDIT___EDIT_OBJECT_FEATURE_PROPAGATE__HPP

