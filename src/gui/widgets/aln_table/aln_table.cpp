/*  $Id: aln_table.cpp 39666 2017-10-25 16:01:13Z katargir $
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
#include <gui/widgets/aln_table/aln_table.hpp>
#include <gui/objutils/utils.hpp>
#include <serial/iterator.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CAlnTableWidget::CAlnTableWidget(
    wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style
)
    : CwxTableListCtrl( parent, id, pos, size, style )
{
    m_Model.Reset( new CAlnTableDS() );

    SetModel( m_Model );

}

CAlnTableWidget::~CAlnTableWidget()
{
    RemoveModel();
}

/*
void CAlnTableWidget::SaveSettings(CRegistryWriteView& view) const
{
    ncbi::SaveTableSettings(*m_Table, view);
}


void CAlnTableWidget::LoadSettings(CRegistryReadView& view)
{
    ncbi::LoadTableSettings(*m_Table, view);
}
*/

void CAlnTableWidget::Add(CScope& scope, const CSeq_align& aln)
{
    if (m_Model) {
        m_Model->Add(scope, aln);
        Update();
    }
}


void CAlnTableWidget::SetDataSource(CAlnTableDS& ds)
{
    CRef<CAlnTableDS> tmp(m_Model);
    m_Model.Reset(&ds);
    SetModel(&ds);

    Update();
}


void CAlnTableWidget::Update()
{
    m_Model->Update();
}

void CAlnTableWidget::GetSelection(TConstObjects& objs) const
{
    //if( m_Model ){
    wxArrayInt objIxs = GetDataRowsSelected();

    const CAlnTableDS* model = dynamic_cast<const CAlnTableDS*>( GetModel() );
    _ASSERT( model != NULL );

    for( int i = 0; i < (int)objIxs.size(); ++i ){
        const CAlnTableDS::SAlignment& row = model->GetData( objIxs[i] );

        if( row.align ){
            objs.push_back( CConstRef<CObject>( &*row.align ) );
        }
    }
}

/*
void CAlnTableWidget::SelectionChanged(const ISelectionModel::CSMNotice& msg)
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}
*/

END_NCBI_SCOPE
