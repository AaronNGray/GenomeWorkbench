#ifndef GUI_WIDGETS_SEQ_GRAPHIC___DATA_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___DATA_TRACK__HPP

/* $Id: data_track.hpp 38675 2017-06-07 22:46:36Z rudnev $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
 */


#include <gui/widgets/seq_graphic/glyph_container_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/utils/event_translator.hpp>

BEGIN_NCBI_SCOPE

class CLogPerformance;

///////////////////////////////////////////////////////////////////////////////
/// CDataTrack - a abstract base class for layout tracks which need to deal
/// with background data retrieval.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CDataTrack :
    public CGlyphContainer,
    public CEventHandler
{
public:
    CDataTrack(CRenderingContext* r_cntx);
    virtual ~CDataTrack();

    /// @name CSeqGlyph interface
    /// @{
    virtual void Update(bool layout_only);
    /// @}

    /// Method for accessing the data source.
    virtual CRef<CSGGenBankDS>  GetDataSource() = 0;
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const = 0;
    virtual void DeleteAllJobs();
    virtual bool AllJobsFinished() const;

    /// event handlers for job notification event.
    void OnAJNotification(CEvent* evt);
    void OnAJProgress(CEvent* evt);

    void SetVisRange(const TSeqRange& range);

protected:
    DECLARE_EVENT_MAP();

    virtual void x_Expand(bool expand);

    /// update track content.
    virtual void x_UpdateData();
    void x_UpdateLayout();

    /// @name job-related interfaces.
    /// @{
    virtual bool x_IsJobNeeded(CAppJobDispatcher::TJobID id) const;
    virtual void x_OnJobCompleted(CAppJobNotification& notify) = 0;
    virtual void x_OnJobFailed(CAppJobNotification& notify);
    virtual void x_OnAllJobsFinished();
    /// @}

    const TSeqRange& x_GetVisRange() const;

protected:

private:
    /// prohibited copy constructor and assignment operator.
    CDataTrack(const CDataTrack&);
    CDataTrack& operator=(const CDataTrack&);

private:
    TSeqRange   m_Range;    ///< visible range
    unique_ptr<CLogPerformance> m_PerfLog;    ///< performamce tracker

    /// it is possible that OnAJNotification() is recursively called several times and therefore excessively
    /// tries to call m_PerfLog->Post();
    /// to prevent this, we count number of nested calls to OnAJNotification()
    int m_JNotificationDepth{0};

};


inline
void CDataTrack::SetVisRange(const TSeqRange& range)
{
    m_Range = range;
}

inline
const TSeqRange& CDataTrack::x_GetVisRange() const
{
    return m_Range;
}

inline
void CDataTrack::x_UpdateLayout()
{
    CGlyphContainer::Update(true);
    x_OnLayoutChanged();
}

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___DATA_TRACK__HPP
