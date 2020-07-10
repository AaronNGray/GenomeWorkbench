/*  $Id: ui_object.cpp 38453 2017-05-10 20:46:05Z evgeniev $
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
 */

#include <ncbi_pch.hpp>

#include <gui/utils/ui_object.hpp>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CUIObject
CUIObject::CUIObject(const string& label,
                     const string& image_alias,
                     const string& hint,
                     const string& description,
                     const string& help_id,
                     const string& log_event)
:   m_Label(label),
    m_IconAlias(image_alias),
    m_Hint(hint),
    m_Description(description),
    m_HelpId(help_id),
    m_LogEvent(log_event)
{
}


CUIObject::CUIObject(const CUIObject& cmd)
:   m_Label(cmd.m_Label),
    m_IconAlias(cmd.m_IconAlias),
    m_Hint(cmd.m_Hint),
    m_Description(cmd.m_Description),
    m_HelpId(cmd.m_HelpId),
    m_LogEvent(cmd.m_LogEvent)
{
}


CUIObject::~CUIObject()
{
}


void CUIObject::Init(const string& label,
                     const string& image_alias,
                     const string& hint,
                     const string& description,
                     const string& help_id,
                     const string& log_event)
{
    m_Label = label;
    m_IconAlias = image_alias;
    m_Hint = hint;
    m_Description = description;
    m_HelpId = help_id;
    m_LogEvent = log_event;
}


void CUIObject::SetLabel(const string& label)
{
    m_Label = label;
}


void CUIObject::SetIconAlias(const string& alias)
{
    m_IconAlias = alias;
}


void CUIObject::SetHint(const string& hint)
{
    m_Hint = hint;
}


void CUIObject::SetDescription(const string& descr)
{
    m_Description = descr;
}


void CUIObject::SetHelpId(const string& help_id)
{
    m_HelpId = help_id;
}


void CUIObject::SetLogEvent(const string& log_event)
{
    m_LogEvent = log_event;
}


const string& CUIObject::GetLabel() const
{
    return m_Label;
}


const string& CUIObject::GetIconAlias() const
{
    return m_IconAlias;
}


const string& CUIObject::GetHint() const
{
    return m_Hint;
}


const string& CUIObject::GetDescription() const
{
    return m_Description;
}


const string& CUIObject::GetHelpId() const
{
    return m_HelpId;
}


const string& CUIObject::GetLogEvent() const
{
    return m_LogEvent;
}


END_NCBI_SCOPE
