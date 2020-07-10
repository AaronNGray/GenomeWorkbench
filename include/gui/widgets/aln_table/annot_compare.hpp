#ifndef GUI_WIDGETS_ALN_TABLE__ANNOT_COMPARE__HPP
#define GUI_WIDGETS_ALN_TABLE__ANNOT_COMPARE__HPP

/*  $Id: annot_compare.hpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/utils/command.hpp>
#include <gui/widgets/wx/table_listctrl.hpp>
#include <gui/widgets/aln_table/annot_compare_ds.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/registry.hpp>

#include <objmgr/scope.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <wx/timer.h>


BEGIN_NCBI_SCOPE

class CAnnotCompareModel;

///////////////////////////////////////////////////////////////////////////////
/// CAnnotCompareWidget
class NCBI_GUIWIDGETS_ALNTABLE_EXPORT CAnnotCompareWidget
    : public CwxTableListCtrl    
{
public:
    typedef list< CConstRef<objects::CSeq_loc> > TLocations;
    typedef CAnnotCompareDS::SRow   TDSRow;

    enum EEventType {
        eEvent_SelectionChanged
    };
    typedef CEvent TEvent;

    /// the columns we plan to show
    enum EColumns {
        eFeat1Label,
        eFeat1Sequence,
        eFeat1Location,
        eFeat2Label,
        eFeat2Sequence,
        eFeat2Location,
        eFeatType,

        eLocationCompare,
        eSequenceCompare,
        eProductCompare,

        eMaxCol /// must be last!
    };

    CAnnotCompareWidget() {}

    CAnnotCompareWidget(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxLC_REPORT | wxLC_VIRTUAL
    );

    virtual ~CAnnotCompareWidget();

    // void SaveSettings(CRegistryWriteView& view) const;
    // void LoadSettings(CRegistryReadView& view);

    //void SelectionChanged(const ISelectionModel::CSMNotice& msg);
    //void GetSelection(TConstObjects& objs) const;

    /// retrieve a list of the selected locations
    typedef list< CConstRef<objects::CSeq_loc> > TLocs;
    void GetSelectedLocations(TLocs& locs) const;

    void SetDataSource(CAnnotCompareDS& ds);
    CAnnotCompareDS& GetDataSource();

    void Update();
    void UpdateModel();
//    bool PerformAction(CAnyType& source, TSignal signal);

    /// @name ITimerListener interface
    /// @{
    void OnTimeout(int timer_id);
    /// @}

    /// @name Event handlers
    /// @{

    void OnFilter();
    void OnTextReport();

    /// @}

protected:
   // DECLARE_EVENT_MAP();

    /// the table's model
    CRef<CAnnotCompareDS> m_Model;

    /// dialog for text report
    //auto_ptr<CTextReportDlg> m_TextDlg;
   
    string m_LabelStr;

    /// timer for progress updates
    wxTimer m_Timer;

    /// handle row double-click events
    void x_OnDoubleClick();

    /// event handler for data source update events
    void x_OnDataSourceUpdated(CEvent*);

private:
    /// prohibited
    CAnnotCompareWidget(const CAnnotCompareWidget&);
    CAnnotCompareWidget& operator=(const CAnnotCompareWidget&);

    void x_OnRowSelect(CEvent* evt);
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_TABLE__ANNOT_COMPARE__HPP
