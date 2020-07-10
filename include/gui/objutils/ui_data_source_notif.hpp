#ifndef GUI_OBJUTILS___UI_DATA_SOURCE_NOTIF__HPP
#define GUI_OBJUTILS___UI_DATA_SOURCE_NOTIF__HPP

/*  $Id: ui_data_source_notif.hpp 25706 2012-04-25 00:21:04Z voronov $
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

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///
/// Data notification relay class, sends data change notifications to the
/// subscribed listener
///
class NCBI_GUIOBJUTILS_EXPORT CDataChangeNotifier : public CObjectEx
{
public:
    enum EUpdateType {
        eChanged,
        eProgressReport,
        eError
    };

    /// CUpdate  - notification send by CUIDataSource to the listener
    class NCBI_GUIOBJUTILS_EXPORT CUpdate
    {
    public:
        CUpdate(int type);
        CUpdate(int type, const string& msg, double progress_norm = 0.5);
        virtual ~CUpdate();

        int GetType() const;
		const string& GetMessage() const;
        double GetProgressNorm() const;

	protected:
        int     m_Type;
        string  m_Msg;
        double m_ProgressNorm; // Normalized from 0 to 1.
    };

    /// CDataChangeNotifier listener receiving notification about changes in data(source)
    class NCBI_GUIOBJUTILS_EXPORT IListener
    {
    public:
		virtual ~IListener() {};
        virtual void    OnDSChanged(CUpdate& update) = 0;

    };

public:
    CDataChangeNotifier();
    virtual ~CDataChangeNotifier() {};

    /// Subscribe a new listener.
    /// No ownership transfer for pListener
    virtual void    SetListener(IListener* pListener);
    virtual IListener*  GetListener();

    virtual void    NotifyListener(CUpdate& update);

protected:
    IListener* m_Listener;
};


/////////////////////////////////////////////////////////////////////////////

inline
CDataChangeNotifier::CUpdate::CUpdate(int type)
:   m_Type(type)
{
}


inline
CDataChangeNotifier::CUpdate::CUpdate(int type, const string& msg, double progress_norm)
:   m_Type(type), m_Msg(msg), m_ProgressNorm(progress_norm)
{
}


inline
CDataChangeNotifier::CUpdate::~CUpdate()
{
}


inline
int CDataChangeNotifier::CUpdate::GetType() const
{
    return m_Type;
}

inline
const string& CDataChangeNotifier::CUpdate::GetMessage() const
{
    return m_Msg;
}

inline
double CDataChangeNotifier::CUpdate::GetProgressNorm() const
{
    return m_ProgressNorm;
}

NCBI_GUIOBJUTILS_EXPORT inline const string& CDCN_GetMessage( const CDataChangeNotifier::CUpdate& update ){
	return update.GetMessage();
}


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___UI_DATA_SOURCE_NOTIF__HPP
