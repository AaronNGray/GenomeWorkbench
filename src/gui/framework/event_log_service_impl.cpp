/*  $Id: event_log_service_impl.cpp 31806 2014-11-17 18:18:01Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>

#include <gui/framework/event_log_service_impl.hpp>

#include <gui/framework/status_bar_service.hpp>


BEGIN_NCBI_SCOPE


CEventRecord::CEventRecord(EType type, const string& title,
                           const string& descr, const TTime& time)
:   m_Type(type),
    m_Title(title),
    m_Description(descr),
    m_Time(time)
{
}


IEventRecord::EType CEventRecord::GetType() const
{
    return m_Type;
}


string CEventRecord::GetTitle() const
{
    return m_Title;
}


string CEventRecord::GetDescription() const
{
    return m_Description;
}


const CEventRecord::TTime& CEventRecord::GetTime() const
{
    return m_Time;
}


void CEventRecord::SetType(EType type)
{
    m_Type = type;
}


void CEventRecord::SetTitle(const string& title)
{
    m_Title = title;
}


void CEventRecord::SetDescription(const string& descr)
{
    m_Description = descr;
}


void CEventRecord::SetTime(const TTime& time)
{
    m_Time = time;
}

///////////////////////////////////////////////////////////////////////////////
/// CEventLogService
CEventLogService::CEventLogService()
{
}


void CEventLogService::InitService()
{
}


void CEventLogService::ShutDownService()
{
    m_Records.clear();
}


void CEventLogService::AddRecord(IEventRecord* record)
{
    _ASSERT(record);

    if(record)  {
        m_Records.push_back(CIRef<IEventRecord>(record));
        Post(CRef<CEvent>(new CLogServiceEvent()));
    }
}


size_t CEventLogService::GetRecordsCount() const
{
    return m_Records.size();
}


const IEventRecord* CEventLogService::GetRecord(size_t index) const
{
    bool valid_index = /*index >= 0  &&*/  index < m_Records.size();
    _ASSERT(valid_index);
    return valid_index ? m_Records[index].GetPointer() : NULL;
}


END_NCBI_SCOPE
