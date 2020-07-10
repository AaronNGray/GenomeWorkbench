#ifndef GUI_CORE___QUICK_LAUNCH_LIST__HPP
#define GUI_CORE___QUICK_LAUNCH_LIST__HPP

/*  $Id: quick_launch_list.hpp 26405 2012-09-12 16:40:31Z katargir $
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

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/utils/mru_list.hpp>

class wxMenu;

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CQuickLaunchList
class NCBI_GUICORE_EXPORT CQuickLaunchList : public CObject
{
public:
    static CQuickLaunchList& GetInstance();

    bool IsEmpty() const;
    const list<string>& GetItems() const;
    void AddItem(const string& item);
    void RemoveItem(const string& item);

    string GetItemPath(const string& item) const;

    void UpdateMenu(wxMenu& menu, int cmdStart, int cmdEnd, size_t pos = (size_t)-1);

private:
    CQuickLaunchList();
    CQuickLaunchList(const CQuickLaunchList&);
    CQuickLaunchList& operator=(const CQuickLaunchList&);

    void x_Load();
    void x_Save() const;

    CMRUList<string> m_ToolList;
};

END_NCBI_SCOPE


#endif  // GUI_CORE___QUICK_LAUNCH_LIST__HPP