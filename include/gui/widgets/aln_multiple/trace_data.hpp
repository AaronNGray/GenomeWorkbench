#ifndef __GUI_WIDGETS_ALNMULTI___TRACE_DATA__HPP
#define __GUI_WIDGETS_ALNMULTI___TRACE_DATA__HPP

/*  $Id: trace_data.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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

#include <corelib/ncbistl.hpp>
#include <gui/gui_export.h>

#include <objmgr/bioseq_handle.hpp>

BEGIN_NCBI_SCOPE

class CTraceData;

////////////////////////////////////////////////////////////////////////////////
/// CTraceDataProxy - an object controlling availability and creation of
/// CTraceData instance.
class   NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CTraceDataProxy
{
public:
    typedef map<string, int>    TTitleToType;

    CTraceDataProxy(const objects::CBioseq_Handle& handle, bool b_neg_strand);
    bool    HasData()   const;
    CTraceData* LoadData();
private:
    enum EStatus    {
        eUnknown,
        eNoData,
        eHasData
    };
    const objects::CBioseq_Handle& m_Handle; /// handle to Bioseq with traces
    bool    m_bNegativeStrand;
    mutable EStatus m_Status;

    TTitleToType   m_TitleToType;
};


////////////////////////////////////////////////////////////////////////////////
/// CTraceData
class CTraceData
{
public:
    typedef float   TConfidence;
    typedef double  TFloatSeqPos;
    typedef float   TSignalValue;

    typedef vector<TFloatSeqPos>    TPositions;
    typedef vector<TSignalValue>    TValues;

    enum    EChannel    {
        eA,
        eC,
        eT,
        eG
    };
public:
    CTraceData()
    : m_From(0), m_To(-1)
    {
    }
    void    Init(TSignedSeqPos from, TSignedSeqPos to, int samples, bool negative);

    /// Sequence related information

    inline TSignedSeqPos   GetSeqFrom() const;
    inline TSignedSeqPos   GetSeqTo() const;
    inline TSignedSeqPos   GetSeqLength() const;
    inline bool     IsNegative()    const;

    inline TConfidence   GetConfidence(TSignedSeqPos pos)   const;
    inline void    SetConfidence(TSignedSeqPos pos, TConfidence conf);

    // signal related information

    inline int     GetSamplesCount() const;
    inline TPositions&  GetPositions();
    inline TValues&      GetA();
    inline TValues&      GetC();
    inline TValues&      GetT();
    inline TValues&      GetG();
    TValues&      GetValues(EChannel signal);

    inline const TPositions&  GetPositions() const;
    inline const TValues&      GetA() const;
    inline const TValues&      GetC() const;
    inline const TValues&      GetT() const;
    inline const TValues&      GetG() const;
    const TValues&      GetValues(EChannel signal) const;

    void    CalculateMax();
    TConfidence     GetMaxConfidence() const;
    TSignalValue    GetMax(EChannel signal) const;

protected:
    TSignedSeqPos   m_From;
    TSignedSeqPos   m_To;
    bool    m_bNegative;

    vector<TConfidence>   m_Confs;
    TPositions      m_Positions;
    TValues         m_ASig;
    TValues         m_CSig;
    TValues         m_TSig;
    TValues         m_GSig;

    TConfidence     m_MaxConfidence;
    TSignalValue    m_MaxA;
    TSignalValue    m_MaxC;
    TSignalValue    m_MaxT;
    TSignalValue    m_MaxG;
};


/// Sequence related information

TSignedSeqPos   CTraceData::GetSeqFrom() const
{
    return m_From;
}
TSignedSeqPos   CTraceData::GetSeqTo() const
{
    return m_To;
}
TSignedSeqPos   CTraceData::GetSeqLength() const
{
    return m_To - m_From + 1;
}
inline bool     CTraceData::IsNegative()    const
{
    return m_bNegative;
}
CTraceData::TConfidence   CTraceData::GetConfidence(TSignedSeqPos pos)   const
{
    return m_Confs[pos - m_From];
}
void    CTraceData::SetConfidence(TSignedSeqPos pos, TConfidence conf)
{
    m_Confs[pos - m_From] = conf;
}

// signal related information

int     CTraceData::GetSamplesCount() const
{
    return (int) m_Positions.size();
}

CTraceData::TPositions&  CTraceData::GetPositions()
{
    return m_Positions;
}
CTraceData::TValues&      CTraceData::GetA()
{
    return m_ASig;
}
CTraceData::TValues&      CTraceData::GetC()
{
    return m_CSig;
}
CTraceData::TValues&      CTraceData::GetT()
{
    return m_TSig;
}
CTraceData::TValues&      CTraceData::GetG()
{
    return m_GSig;
}
const CTraceData::TPositions&  CTraceData::GetPositions() const
{
    return m_Positions;
}
const CTraceData::TValues&      CTraceData::GetA() const
{
    return m_ASig;
}
const CTraceData::TValues&      CTraceData::GetC() const
{
    return m_CSig;
}
const CTraceData::TValues&      CTraceData::GetT() const
{
    return m_TSig;
}
const CTraceData::TValues&      CTraceData::GetG() const
{
    return m_GSig;
}

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___TRACE_DATA__HPP
