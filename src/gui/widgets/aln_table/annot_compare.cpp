/*  $Id: annot_compare.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <ncbi_pch.hpp>


#include <ncbi_pch.hpp>
#include <gui/widgets/aln_table/annot_compare.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/utils/view_event.hpp>
#include <serial/iterator.hpp>
#include <algo/sequence/annot_compare.hpp>

#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/util/sequence.hpp>

// #include "annot_filter_dlg.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/////////////////////////////////////////////////////////////////////////////
// CAnnotCompareModel
/*

!!! TODO: EVRYTHING FROM HERE GOES TO  annot_compare_ds, this is now a model

class CAnnotCompareModel : public CwxAbstractTableModel
{
public:
    /// our data is stored as a vector of vectors
    typedef vector<string> TRowData;
    typedef vector<TRowData> TModelData;

    CAnnotCompareModel();

    virtual int GetNumRows() const;
    virtual int GetNumColumns() const;
    virtual wxVariant GetValueAt( int row, int col ) const;

    virtual wxString GetColumnType(int col_idx) const;

    virtual wxString GetColumnName(int col_idx) const;

    /// setters - to be called to push data in from the outside
    void SetDataSource(CAnnotCompareDS& ds);
    void Update();

private:
    CRef<CAnnotCompareDS> m_DS;

    /// saved column names
    mutable vector<string> m_ColNames;
};


CAnnotCompareModel::CAnnotCompareModel()
{
}


int CAnnotCompareModel::GetNumRows() const
{
    return (m_DS ? m_DS->GetRows() : 0);
}


int CAnnotCompareModel::GetNumColumns() const
{
    return CAnnotCompareWidget::eMaxCol;
}


const CAnyType CAnnotCompareModel::GetValueAt( int r, int c ) const
{
    const CAnnotCompareDS::SRow& row = m_DS->GetRow(r);

    switch (c) {
    case CAnnotCompareWidget::eFeat1Label:
        if (row.feat1_label.empty()  &&  row.feat1) {
            CLabel::GetLabel(*row.feat1, &row.feat1_label,
                             CLabel::eDefault, &m_DS->GetScope());
        }
        return row.feat1_label;

    case CAnnotCompareWidget::eFeat1Location:
        if (row.feat1_loc_label.empty()  &&  row.feat1_loc) {
            CLabel::GetLabel(*row.feat1_loc, &row.feat1_loc_label,
                             CLabel::eDefault, &m_DS->GetScope());
        }
        return row.feat1_loc_label;

    case CAnnotCompareWidget::eFeat1Sequence:
        if (row.feat1_seq_label.empty()  &&  row.feat1_seq) {
            CLabel::GetLabel(*row.feat1_seq.GetSeqId(), &row.feat1_seq_label,
                             CLabel::eDefault, &m_DS->GetScope());
        }
        return row.feat1_seq_label;

    case CAnnotCompareWidget::eFeat2Label:
        if (row.feat2_label.empty()  &&  row.feat2) {
            CLabel::GetLabel(*row.feat2, &row.feat2_label,
                             CLabel::eDefault, &m_DS->GetScope());
        }
        return row.feat2_label;

    case CAnnotCompareWidget::eFeat2Location:
        if (row.feat2_loc_label.empty()  &&  row.feat2_loc) {
            CLabel::GetLabel(*row.feat2_loc, &row.feat2_loc_label,
                             CLabel::eDefault, &m_DS->GetScope());
        }
        return row.feat2_loc_label;

    case CAnnotCompareWidget::eFeat2Sequence:
        if (row.feat2_seq_label.empty()  &&  row.feat2_seq) {
            CLabel::GetLabel(*row.feat2_seq.GetSeqId(), &row.feat2_seq_label,
                             CLabel::eDefault, &m_DS->GetScope());
        }
        return row.feat2_seq_label;

    case CAnnotCompareWidget::eFeatType:
        if (row.feat_type_label.empty()) {
            if (row.feat1) {
                row.feat_type_label = row.feat1->GetData().GetKey();
            } else if (row.feat2) {
                row.feat_type_label = row.feat2->GetData().GetKey();
            }
        }
        return row.feat_type_label;

    case CAnnotCompareWidget::eLocationCompare:
        switch (row.loc_state & CAnnotCompare::eLocationMask) {
        case CAnnotCompare::eLocation_Missing:
            {{
                static string str("Not found");
                return str;
            }}
        case CAnnotCompare::eLocation_Same:
            {{
                static string str("Identical");
                return str;
            }}
        case CAnnotCompare::eLocation_MissingExon:
            {{
                static string str("Missing Exons");
                return str;
            }}
        case CAnnotCompare::eLocation_5PrimeExtension:
            {{
                static string str("5' Extension");
                return str;
            }}
        case CAnnotCompare::eLocation_3PrimeExtension:
            {{
                static string str("3' Extension");
                return str;
            }}
        case CAnnotCompare::eLocation_Complex:
            {{
                static string str("Complex");
                return str;
            }}

        default:
            {{
                _ASSERT(false);
                static string str("Unknown");
                return str;
            }}
        }

    case CAnnotCompareWidget::eSequenceCompare:
        if (row.loc_state & CAnnotCompare::eSequence_SameSeq) {
            static string str("Identical");
            return str;
        } else if (row.loc_state & CAnnotCompare::eSequence_DifferentSeq) {
            static string str("Different");
            return str;
        } else {
            return kEmptyStr;
        }
        break;

    case CAnnotCompareWidget::eProductCompare:
        if (row.loc_state & CAnnotCompare::eSequence_SameProduct) {
            static string str("Identical");
            return str;
        } else if (row.loc_state & CAnnotCompare::eSequence_DifferentProduct) {
            static string str("Different");
            return str;
        } else {
            return kEmptyStr;
        }
        break;

    default:
        _ASSERT(false);
        throw runtime_error("Invalid column in CAnnotCompareModel");
    }
}


const type_info& CAnnotCompareModel::GetColumnType(int col) const
{
    return typeid(string);
}


const CAnyType CAnnotCompareModel::GetColumnName(int col) const
{
    static const string sc_ColNames[] = {
        string("Feature 1"),
        string("Sequence 1"),
        string("Location 1"),
        string("Feature 2"),
        string("Sequence 2"),
        string("Location 2"),
        string("Feature Type"),
        string("Location Comparison"),
        string("Sequence Comparison"),
        string("Product Comparison")
    };

    _ASSERT(col < (int)(sizeof(sc_ColNames) / sizeof(string)));
    return sc_ColNames[col];
}


/// setters - to be called to push data in from the outside
void CAnnotCompareModel::SetDataSource(CAnnotCompareDS& ds)
{
    m_DS.Reset(&ds);
    Update();
}


void CAnnotCompareModel::Update()
{
    FireDataChanged();
}
*/


/////////////////////////////////////////////////////////////////////////////
// CAnnotCompareWidget

const int kStatusBarHeght = 18;


CAnnotCompareWidget::CAnnotCompareWidget(
    wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style
)
    : CwxTableListCtrl( parent, id, pos, size, style )
{
    m_Model.Reset( new CAnnotCompareDS() );

    SetModel( m_Model );

}



CAnnotCompareWidget::~CAnnotCompareWidget()
{
     RemoveModel();
}


/*
BEGIN_EVENT_MAP(CAnnotCompareWidget, CEventHandler)
    ON_EVENT(CEvent, CViewEvent::eWidgetDataChanged,
             &CAnnotCompareWidget::x_OnDataSourceUpdated)
END_EVENT_MAP()


void CAnnotCompareWidget::SaveSettings(CRegistryWriteView& view) const
{
    ncbi::SaveTableSettings(*m_Table, view);
}


void CAnnotCompareWidget::LoadSettings(CRegistryReadView& view)
{
    ncbi::LoadTableSettings(*m_Table, view);
}
*/

void CAnnotCompareWidget::OnTimeout(int timer_id)
{
/*
    if (m_Data) {
        float val = m_Data->GetProgress();

        m_LabelStr = "Computing comparisons, " +
            NStr::IntToString( (int)(100 * val) ) + "% completed";
        m_Label->label(m_LabelStr.c_str());
        m_Label->redraw();
    }
    */
}


void CAnnotCompareWidget::OnFilter()
{
/*
    SAnnotCompareFilter& filt =
        dynamic_cast<SAnnotCompareFilter&>(*m_Table->GetFilter());

    CAnnotCompareFilterDlg dlg(filt);
    if (dlg.ShowModal() == eOK) {
        m_Table->ApplyFiltering();
        redraw();
    }
    */
}

void CAnnotCompareWidget::OnTextReport()
{
    /*
    CNcbiOstrstream ostr;
    m_Data->DumpTextReport(ostr);

    if ( !m_TextDlg.get() ) {
        m_TextDlg.reset(new CTextReportDlg());
        m_TextDlg->SetTitle("Annotation Comparison Report");
    }
    m_TextDlg->SetText(string(CNcbiOstrstreamToString(ostr)));
    m_TextDlg->CenterOnActive();
    m_TextDlg->Show();
    */
}


CAnnotCompareDS& CAnnotCompareWidget::GetDataSource()
{
    return dynamic_cast<CAnnotCompareDS&>(*m_Model);
}


void CAnnotCompareWidget::SetDataSource(CAnnotCompareDS& ds)
{
    m_Model.Reset(&ds);
    m_Model->RegisterListener(this);
    m_Model->AddListener(this, ePool_Parent);   
}

/*
void CAnnotCompareWidget::GetSelection(TConstObjects& objs) const
{
    if( m_Table && m_Model.get() ){
        CAnyRef<ISelectionModel> sel_model = m_Table->GetSelectionModel();
        for (int row = 0; row < m_Model->GetNumRows(); ++row) {
            if (sel_model->IsSelectedIndex(row)) {
                int row_i = m_Table->RowVisibleToData(row);
                const CAnnotCompareDS::SRow& row = m_Data->GetRow(row_i);

                if (row.feat1) {
                    objs.push_back(CConstRef<CObject>(&*row.feat1));
                }
                if (row.feat2) {
                    objs.push_back(CConstRef<CObject>(&*row.feat2));
                }
            }
        }
    }
}

void CAnnotCompareWidget::SelectionChanged(const ISelectionModel::CSMNotice& msg)
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}

bool CAnnotCompareWidget::PerformAction(CAnyType& aSource, TSignal aSignal)
{
    if (aSignal == CStandardEP::sm_SigContextAction) {
        /// double-click
        x_OnDoubleClick();
    }
    return false;
}
*/


void CAnnotCompareWidget::x_OnDoubleClick()
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetRangeChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}


void CAnnotCompareWidget::GetSelectedLocations(TLocs& locs) const
{
/*
    if (m_Table && m_Model.get()) {
        CAnyRef<ISelectionModel> sel_model = m_Table->GetSelectionModel();
        for (int row = 0; row < m_Model->GetNumRows(); ++row) {
            if (sel_model->IsSelectedIndex(row)) {
                int row_i = m_Table->RowVisibleToData(row);
                const CAnnotCompareDS::SRow& row = m_Data->GetRow(row_i);

                if (row.feat1_loc) {
                    locs.push_back(row.feat1_loc);
                }
                if (row.feat2_loc) {
                    locs.push_back(row.feat2_loc);
                }
            }
        }
    }
    */
}


void CAnnotCompareWidget::Update()
{
    //m_Panel->hide();

//    m_Timer.Init(1, 2, true, this);
    //m_Timer.Start();
    GetDataSource().Update();
    UpdateModel();
}


void CAnnotCompareWidget::UpdateModel()
{
    if (m_Model) {
        m_Model->Update();
    }
}

void CAnnotCompareWidget::x_OnDataSourceUpdated(CEvent*)
{
    UpdateModel();
    m_Timer.Stop();
//    m_StatusBar->SetMessage("Found " + NStr::IntToString(m_Model->GetNumRows()) + " comparisons");
}

END_NCBI_SCOPE
