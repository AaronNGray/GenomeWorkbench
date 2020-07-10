#ifndef GUI_WIDGETS___LOADERS___OPEN_CLIPBOARD_OBJECT_DLG__HPP
#define GUI_WIDGETS___LOADERS___OPEN_CLIPBOARD_OBJECT_DLG__HPP

/*  $Id: open_data_source_object_dlg.hpp 39737 2017-10-31 17:03:07Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>

#include <gui/widgets/wx/wizard_dlg.hpp>
#include <gui/core/ui_tool_manager.hpp>

#include "wx/htmllbox.h"

BEGIN_NCBI_SCOPE

class COpenObjectsPanel;
class IObjectLoader;

class COpenDataSourceObjectDlg: public CWizardDlg
{
    DECLARE_DYNAMIC_CLASS( COpenDataSourceObjectDlg )
    DECLARE_EVENT_TABLE()

public:
    COpenDataSourceObjectDlg();
    COpenDataSourceObjectDlg( wxWindow* parent, CIRef<IUIToolManager> manager  );

    bool Create( wxWindow* parent );

    ~COpenDataSourceObjectDlg();

    void Init();

    virtual void CreatePages();

    void SetManager(CIRef<IUIToolManager> manager);

    IObjectLoader* GetObjectLoader();

protected:
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

    CIRef<IUIToolManager> m_Manager;
};

END_NCBI_SCOPE


#endif //GUI_WIDGETS___LOADERS___OPEN_CLIPBOARD_OBJECT_DLG__HPP
