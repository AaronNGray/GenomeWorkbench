#ifndef GUI_UTILS___UI_OBJECT__HPP
#define GUI_UTILS___UI_OBJECT__HPP

/*  $Id: ui_object.hpp 38453 2017-05-10 20:46:05Z evgeniev $
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

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// IUIObject - object that provides basic properties often required
/// in a UI object.
///
/// Attributes:
/// - Label - name of the command (significant words shall be capitalized).
///   Example - “Save as Web Page”.
/// - Icon Alias – symbolic image name that can be resolved an an image using
///    wxArtProvider. Serves as menu, menubar and toolbar icon.
/// - Hint – short (one sentence) explanation of what the command does.
///   Can be used as a tooltip ot status bar hint.
/// - Description – long (one paragraph) explanation of what the command does.
/// - Help ID – symbolic identifier of the help context associated with the command.
/// - Log Event - event name to use when logging information about the object

class  NCBI_GUIUTILS_EXPORT  IUIObject
{
public:
    virtual const string&   GetLabel() const = 0;
    virtual const string&   GetIconAlias() const = 0;
    virtual const string&   GetHint() const = 0;
    virtual const string&   GetDescription() const = 0;
    virtual const string&   GetHelpId() const = 0;
    virtual const string&   GetLogEvent() const = 0;

    virtual ~IUIObject()    {};
};


///////////////////////////////////////////////////////////////////////////////
/// CUIObject - default mix-in implementation of IUIObject

class  NCBI_GUIUTILS_EXPORT  CUIObject :
    public IUIObject
{
public:
    CUIObject(const string& label,
              const string& icon_alias,
              const string& hint = kEmptyStr,
              const string& description = kEmptyStr,
              const string& help_id = kEmptyStr,
              const string& log_event = kEmptyStr);
    CUIObject(const CUIObject& obj);
    virtual ~CUIObject();

    virtual void    Init(const string& label,
                         const string& icon_alias,
                         const string& hint = kEmptyStr,
                         const string& description = kEmptyStr,
                         const string& help_id = kEmptyStr,
                         const string& log_event = kEmptyStr);

    virtual void    SetLabel(const string& label);
    virtual void    SetIconAlias(const string& alias);
    virtual void    SetHint(const string& hint);
    virtual void    SetDescription(const string& descr);
    virtual void    SetHelpId(const string& help_id);
    virtual void    SetLogEvent(const string& log_event);

    virtual const string&   GetLabel() const;
    virtual const string&   GetIconAlias() const;
    virtual const string&   GetHint() const;
    virtual const string&   GetDescription() const;
    virtual const string&   GetHelpId() const;
    virtual const string&   GetLogEvent() const;

protected:
    string  m_Label;
    string  m_IconAlias;
    string  m_Hint;
    string  m_Description;
    string  m_HelpId;
    string  m_LogEvent;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___UI_OBJECT__HPP

