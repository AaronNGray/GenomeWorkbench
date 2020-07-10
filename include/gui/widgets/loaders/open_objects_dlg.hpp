#ifndef GUI_WIDGETS___LOADERS___OPEN_OBJECTS_DLG__HPP
#define GUI_WIDGETS___LOADERS___OPEN_OBJECTS_DLG__HPP

/*  $Id: open_objects_dlg.hpp 39737 2017-10-31 17:03:07Z katargir $
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

#include <corelib/ncbistl.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/wizard_dlg.hpp>
#include <gui/widgets/loaders/open_objects_panel_client.hpp>

#include "wx/htmllbox.h"

BEGIN_NCBI_SCOPE

class COpenObjectsPanel;
class IObjectLoader;

class NCBI_GUIWIDGETS_LOADERS_EXPORT COpenObjectsDlg: public CWizardDlg
{
    DECLARE_DYNAMIC_CLASS( COpenObjectsDlg )
    DECLARE_EVENT_TABLE()

public:
    COpenObjectsDlg();
    COpenObjectsDlg( wxWindow* parent );

    bool Create( wxWindow* parent );

    ~COpenObjectsDlg();

    void Init();

    virtual void CreatePages();

    void    SetManagers(vector<CIRef<IOpenObjectsPanelClient> >& managers);
    IObjectLoader* GetObjectLoader();

protected:
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

    enum {
        ID_OPEN_OBJECTS_PANEL = 10003,
    };

    COpenObjectsPanel*  m_OpenObjectsPage;
};

END_NCBI_SCOPE


#endif //GUI_WIDGETS___LOADERS___OPEN_OBJECTS_DLG__HPP
