#ifndef GUI_OBJUTILS___TABLE_DATA_ALN_SUMMARY__HPP
#define GUI_OBJUTILS___TABLE_DATA_ALN_SUMMARY__HPP

/*  $Id: table_data_aln_summary.hpp 32240 2015-01-22 21:53:03Z katargir $
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
 * Authors: Liangshou Wu
 *
 * File Description:
 *
 */

#include <gui/objutils/table_data_base.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqloc/Na_strand.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CTableDataAlnSummary 
    : public CObjectEx
    , public CTableDataBase
{
public:
    enum EColType {
        eQuery = 0,
        eSubject,
        eQStart,
        eQStop,
        eQStrand,
        eSStart,
        eSStop,
        eSStrand,
        ePctIdentity,
        ePctCoverage,
        eMismatches,
        eGaps,
        eGappedBases,
        eScore,
        eEValue,
        eQDefline,
        eSDefline,
        eMaxColNum
    };

    CTableDataAlnSummary(TConstScopedObjects& objects);

    /// @name ITableData interface implementation
    /// @{
    virtual void       LoadData();
    virtual ColumnType GetColumnType(size_t col) const;
    virtual string     GetColumnLabel(size_t col) const;

    virtual size_t GetRowsCount() const;
    virtual size_t GetColsCount() const;

    virtual void GetStringValue(size_t row, size_t col, string& value) const;
    virtual long   GetIntValue(size_t row, size_t col) const;
    virtual double GetRealValue(size_t row, size_t col) const;
    virtual SConstScopedObject GetObjectValue(size_t row, size_t col) const;
    virtual SConstScopedObject GetRowObject(size_t row) const;

    CRef<objects::CScope> GetScope() { return m_Scope; }
    /// @}

    class CAlnSummary : public CObject
    {
    public:
        string                m_Query;
        string                m_Subject;
        TSeqPos               m_QStart;
        TSeqPos               m_QStop;
        objects::ENa_strand   m_QStrand;
        TSeqPos               m_SStart;
        TSeqPos               m_SStop;
        objects::ENa_strand   m_SStrand;
        double                m_PctIdentity;
        double                m_PctCoverage;
        size_t                m_Mismatches;
        size_t                m_Gaps;
        size_t                m_GappedBases;
        int                   m_Score;
        double                m_EValue;
        string                m_QDefline;
        string                m_SDefline;
    };

private:
    const CAlnSummary& x_GetAlnSummary(size_t row) const;
    string x_StrandToStr(objects::ENa_strand strand) const;

private:
    CRef<objects::CScope> m_Scope;
    vector< CConstRef<objects::CSeq_align> > m_Aligns;
    vector<int> m_RowsUpTo;

    /// cached summary for one row
    typedef map<int, CRef<CAlnSummary> > TAlnSummary;
    mutable TAlnSummary m_CachedSummary;
};


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___TABLE_DATA__HPP
