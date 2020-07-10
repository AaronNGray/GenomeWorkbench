/*  $Id: ui_data_source_notif.cpp 25706 2012-04-25 00:21:04Z voronov $
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

#include <ncbi_pch.hpp>

#include <gui/objutils/ui_data_source_notif.hpp>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CUIDataSource
CDataChangeNotifier::CDataChangeNotifier()
:   m_Listener(NULL)
{
}


void CDataChangeNotifier::SetListener(IListener* listener)
{
    m_Listener = listener;
}


CDataChangeNotifier::IListener* CDataChangeNotifier::GetListener()
{
    return m_Listener;
}


void CDataChangeNotifier::NotifyListener(CUpdate& update)
{
    if(m_Listener)  {
        m_Listener->OnDSChanged(update);
    }
}

/*
const string& CDataChangeNotifier::CUpdate::GetMessage() const
{
	return m_Msg;
}
*/

END_NCBI_SCOPE
