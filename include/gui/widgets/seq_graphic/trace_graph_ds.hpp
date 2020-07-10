#ifndef GUI_WIDGETS_SEQ_GRAPHIC___TRACE_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___TRACE_DS__HPP

/*  
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
 * Authors:  Andrei Shkeda
 *
 * File Description:
 *
 */

#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job.hpp>
#include <gui/utils/extension.hpp>


BEGIN_NCBI_SCOPE

class CSGTraceData;

///////////////////////////////////////////////////////////////////////////////
/// CSGTraceGraphDS
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGTraceGraphDS 
    : public CSGGenBankDS
{
public:
    typedef CSGAnnotJob::TAxisLimits TAxisLimits;
    CSGTraceGraphDS(objects::CScope& scope, const objects::CSeq_id& id);
    bool HasData()   const;
    void LoadData();
    CRef<CSGTraceData> GetData() const;
private:
    enum EStatus
    {
        eUnknown,
        eNoData,
        eHasData
    };
    mutable EStatus m_Status = eUnknown;
    CRef<CSGTraceData> m_Data;
};


///////////////////////////////////////////////////////////////////////////////
/// CSGTraceGraphDSType
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGTraceGraphDSType :
    public CObject,
    public ISGDataSourceType,
    public IExtension
{
public:
    /// create an instance of the layout track type using default settings.
    virtual ISGDataSource* CreateDS(SConstScopedObject& object) const;

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}

    /// check if the data source can be shared.
    virtual bool IsSharable() const;
};


////////////////////////////////////////////////////////////////////////////////
/// CSGTraceData
class CSGTraceData : public CObject
{
public:
    typedef float   TConfidence;
    typedef double  TFloatSeqPos;
    typedef float   TSignalValue;

    typedef vector<TFloatSeqPos>    TPositions;
    typedef vector<TSignalValue>    TValues;

    enum    EChannel
    {
        eA,
        eC,
        eT,
        eG
    };
public:
    CSGTraceData()
        : m_From(0), m_To(-1)
    {
    }
    void    Init(TSignedSeqPos from, TSignedSeqPos to, int samples);

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

    void SetTitle(const string& title);
    const string& GetTitle() const;

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

    string m_Title = "Trace chromatograms";
};

/// Sequence related information

inline
CRef<CSGTraceData> CSGTraceGraphDS::GetData() const
{
    return m_Data;
}


inline
TSignedSeqPos   CSGTraceData::GetSeqFrom() const
{
    return m_From;
}

inline
TSignedSeqPos   CSGTraceData::GetSeqTo() const
{
    return m_To;
}

inline
TSignedSeqPos   CSGTraceData::GetSeqLength() const
{
    return m_To - m_From + 1;
}

inline bool     CSGTraceData::IsNegative()    const
{
    return m_bNegative;
}

inline
CSGTraceData::TConfidence   CSGTraceData::GetConfidence(TSignedSeqPos pos)   const
{
    return m_Confs[pos - m_From];
}

inline
void    CSGTraceData::SetConfidence(TSignedSeqPos pos, TConfidence conf)
{
    m_Confs[pos - m_From] = conf;
}

// signal related information

inline
int     CSGTraceData::GetSamplesCount() const
{
    return (int)m_Positions.size();
}

inline
CSGTraceData::TPositions&  CSGTraceData::GetPositions()
{
    return m_Positions;
}

inline
CSGTraceData::TValues&      CSGTraceData::GetA()
{
    return m_ASig;
}

inline
CSGTraceData::TValues&      CSGTraceData::GetC()
{
    return m_CSig;
}

inline
CSGTraceData::TValues&      CSGTraceData::GetT()
{
    return m_TSig;
}

inline
CSGTraceData::TValues&      CSGTraceData::GetG()
{
    return m_GSig;
}

inline
const CSGTraceData::TPositions&  CSGTraceData::GetPositions() const
{
    return m_Positions;
}

inline
const CSGTraceData::TValues&      CSGTraceData::GetA() const
{
    return m_ASig;
}

inline
const CSGTraceData::TValues&      CSGTraceData::GetC() const
{
    return m_CSig;
}

inline
const CSGTraceData::TValues&      CSGTraceData::GetT() const
{
    return m_TSig;
}

inline
const CSGTraceData::TValues&      CSGTraceData::GetG() const
{
    return m_GSig;
}

inline
void CSGTraceData::SetTitle(const string& title) 
{
    m_Title = title;
}

inline
const string& CSGTraceData::GetTitle() const
{
    return m_Title;
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_DS__HPP
