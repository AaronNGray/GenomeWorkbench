#ifndef GUI_FRAMEWORk___OPTIONS_DLG_EXTENSION__HPP
#define GUI_FRAMEWORk___OPTIONS_DLG_EXTENSION__HPP

/*  $Id: options_dlg_extension.hpp 18397 2008-11-28 19:45:17Z dicuccio $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

class wxWindow;

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// An extension point for dialog panels that are shown in the "Options" dialog.
/// Extensions contributed to this point must implement IOptionsDlgExtension
/// interface
///
#define EXT_POINT__OPTIONS_DLG_EXTENSION     "options_dlg_extension"

///////////////////////////////////////////////////////////////////////////////
/// IOptionsDlgExtension - abstract provider of dialog panels for "Options"
/// dialog.
class IOptionsDlgExtension
{
public:
    virtual ~IOptionsDlgExtension() {}

    // must create a window that will be shown on dialog using parameter as a parent
    virtual wxWindow*   CreateSettingsPage(wxWindow* parent) = 0;

    // returns “priority” according to which pages will be sorted
    // this member is intended to allow ordering of pages
    virtual size_t      GetPriority() const = 0;

    // returns label that will be shown on the tab of the dialog
    virtual string      GetPageLabel() const = 0;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORk___OPTIONS_DLG_EXTENSION__HPP
