#ifndef GUI_OBJUTILS___EDIT_EVENT__HPP
#define GUI_OBJUTILS___EDIT_EVENT__HPP

/*  $Id: edit_event.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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

#include <gui/utils/event.hpp>
#include <objects/seqfeat/Seq_feat.hpp>


BEGIN_NCBI_SCOPE

///
/// class CEditEvent defines a single type for all editing events.
/// The primary purpose of this class is to provide type specificity for
/// all edit activities.  Also, this class provides a set of event IDs that
/// correspond to editing actions.
///
class CEditEvent : public CEvent
{
public:
    enum EEditEventType {
        /// unknown editing event
        eEditEvent_Unknown = eEvent_MinClientID,

        eEditEvent_Alignment,   ///< Alignment was edited
        eEditEvent_Descr,       ///< Sequence descriptor was edited
        eEditEvent_Feature,     ///< Features were changed
        eEditEvent_Pub,         ///< Publication was changed
        eEditEvent_Sequence     ///< Sequence itself was altered
    };

    /// default ctor
    CEditEvent(EEditEventType evt_id)
        : CEvent(CEvent::eEvent_Message, evt_id)
    {
    }
};


///
/// class CSeq_featChangedEvent provides an interface for notifying classes
/// that a feature or set of features have changed.  This should be used in
/// cases in which a single or bulk update has been performed, but the features
/// involved have neither been added or removed.
///
class CSeq_featChangedEvent : public CEditEvent
{
public:
    /// typedef to make the syntax easier on the eye and a bit more generic
    typedef vector< CConstRef<objects::CSeq_feat> > TFeatures;

    /// ctor: create an event for a single feature
    CSeq_featChangedEvent()
        : CEditEvent(eEditEvent_Feature)
    {
    }

    CSeq_featChangedEvent(const objects::CSeq_feat& feat)
        : CEditEvent(eEditEvent_Feature)
    {
        m_Feats.push_back(CConstRef<objects::CSeq_feat>(&feat));
    }

    /// ctor: create an event for a set of features
    CSeq_featChangedEvent(const TFeatures& feats)
        : CEditEvent(eEditEvent_Feature),
          m_Feats(feats)
    {
    }

    /// access the features for this event
    const TFeatures& GetFeatures() const { return m_Feats; }

private:
    TFeatures m_Feats;
};


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___EDIT_EVENT__HPP
