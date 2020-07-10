#ifndef GUI_WIDGETS_SNP___SNP_TABLE_DS__HPP
#define GUI_WIDGETS_SNP___SNP_TABLE_DS__HPP

/*  $Id: snp_table_ds.hpp 23347 2011-03-10 16:34:59Z wuliangs $
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
 * Authors:  Melvin Quintos
 *
 * File Description:
 *
 */

//#include <gui/widgets/controls/table_model.hpp>
//#include <gui/widgets/snp/snp_filter.hpp>
#include <gui/widgets/snp/filter/snp_filter.hpp>
#include <objmgr/feat_ci.hpp>


#include <gui/widgets/snp/snp_data.hpp>

BEGIN_NCBI_SCOPE

class /*NCBI_GUIWIDGETS_SNP_EXPORT*/ CSnpTableData
    : public CObject
    , public CAbstractTableModel

{

public:
    CSnpTableData();
    void Init(bool isStudyTable = false);

    void SetData(objects::CScope *          scope,
                 const objects::CSeq_loc &  loc   );

    virtual int                 GetNumColumns() const;
    virtual int                 GetNumRows()    const;
    int                         GetNumRowsUnfiltered() const;

    virtual const type_info&    GetColumnType( int aColIx ) const;
    virtual const CAnyType      GetColumnName( int aColIx ) const;
    virtual const CAnyType      GetValueAt(int row, int column) const;

    virtual bool IsEditableAt( int row, int col ) const;

    void SetFilter(const CRef<SSnpFilter> &filter);

private:
    void x_Update();
    void x_UpdateWithStudies();     // called by x_Update()
    void x_UpdateWithoutStudies();  // called by x_Update()
    void x_ApplyFilter();

private:
    CRef<objects::CScope>           m_Scope;
    CConstRef<objects::CSeq_loc>    m_Loc;
    vector<SSnpData>                m_listOriginalSnps;
    vector<const SSnpData *>        m_listFilteredSnps;

    CRef<SSnpFilter>                m_Filter;
    size_t                          m_numRows;

    CMutex                          m_mutGate;
    mutable CMutex                  m_mutData;
    CSemaphore                      m_semContinue;
    bool                            m_bCancel;
    bool                            m_bStudyTable;

};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SNP___SNP_TABLE_DS__HPP

