#ifndef GUI_CORE___APP_EXPLORER_DATA_OBJECT__HPP
#define GUI_CORE___APP_EXPLORER_DATA_OBJECT__HPP

/*  $Id: app_explorer_data_object.hpp 33078 2015-05-21 15:06:52Z katargir $
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
 *  CAppExplorerData and CAppExplorerDataObject classes used in
 *  App Explorer Service for implementing D&D and Clipboard operations.
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>
#include <corelib/ncbi_process.hpp>

#include <gui/gui_export.h>

#include <gui/core/pt_item.hpp>

#include <gui/widgets/wx/clipboard.hpp>

#include <wx/dataobj.h>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CAppExplorerDataObject - wxDataObject for use with CAppExplorerService.
/// This object holds a collection of CItems and, optionally, a string
/// of text, generated from the items. The object holds a reference to
/// CAppExplorerData object. CAppExplorerService also can have a reference
/// to this objects, this the allows the service to invalidate the data if needed.

class  NCBI_GUICORE_EXPORT  CAppExplorerDataObject : public wxDataObjectSimple
{
public:
    static wxDataFormat m_ItemsFormat;

    CAppExplorerDataObject();
    CAppExplorerDataObject(wxTreeCtrl* treeCtrl, wxArrayTreeItemIds& items, bool cut);

    virtual size_t GetDataSize() const;
    virtual bool GetDataHere(void * buf) const;
    virtual bool SetData(size_t len, const void * buf);

    void    GetItems(wxTreeCtrl& treeCtrl, PT::TItems& items);
    bool    IsCut() const { return m_Cut; }

protected:
    struct TData {
        TPid            m_Pid;
        wxTreeCtrl*     m_TreeCtrl;
        bool            m_Cut;
        size_t          m_ItemCount;
    };

    wxTreeCtrl*        m_TreeCtrl;
    wxArrayTreeItemIds m_Items;
    bool m_Cut;
};


END_NCBI_SCOPE

#endif  /// GUI_CORE___APP_EXPLORER_DATA_OBJECT__HPP
