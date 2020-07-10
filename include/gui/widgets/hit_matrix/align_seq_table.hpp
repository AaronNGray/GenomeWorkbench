#ifndef GUI_WIDGETS_HIT_MATRIX___ALIGN_SEQ_TABLE__HPP
#define GUI_WIDGETS_HIT_MATRIX___ALIGN_SEQ_TABLE__HPP

/*  $Id: align_seq_table.hpp 15592 2007-12-20 23:18:51Z voronov $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <gui/widgets/hit_matrix/hit_matrix_ds.hpp>
#include <gui/widgets/wx/table_listctrl.hpp>

#include <gui/objutils/registry.hpp>

#include <objmgr/scope.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAlignSeqTableModel
class CAlignSeqTableModel : public CwxAbstractTableModel
{
public:
    typedef IHitMatrixDataSource::TIdVector TIdVector;

    void Init( IHitMatrixDataSource& ds, const TIdVector& ids );
    void Clear();

    /// @name ITableModel virtual functions implementation
    /// @{
    virtual int GetNumRows() const;
    virtual int GetNumColumns() const;
    virtual wxVariant GetValueAt( int row, int column ) const;

    virtual wxString GetColumnType( int col_ix ) const;
    virtual wxString GetColumnName( int col_ix ) const;
    /// @}

    const IHitSeqId* GetId( int row ) const;

protected:
    struct SRow {
        IHitSeqId*  m_Id;
        string m_IdLabel;
        string m_Title;
        string m_Organism;
        string m_Length;
    };

    string x_GetTitle( objects::CBioseq_Handle& handle ) const;
    string x_GetOrganism( objects::CBioseq_Handle& handle ) const;
    string x_GetLength( objects::CBioseq_Handle& handle ) const;

protected:
    vector<SRow*> m_Rows;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___ALIGN_SEQ_TABLE__HPP
