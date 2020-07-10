#ifndef PKG_SEQUENCE_EDIT___INDEXER_APP_MODE__HPP
#define PKG_SEQUENCE_EDIT___INDEXER_APP_MODE__HPP

/*  $Id: indexer_app_mode.hpp 39241 2017-08-28 15:44:36Z katargir $
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
* Authors: Roman Katargin
*
* File Description:
*
*/

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/framework/app_mode_extension.hpp>
#include <gui/utils/extension.hpp>

class wxCommandEvent;

BEGIN_NCBI_SCOPE

class CWorkbench;

class CIndexerAppModeExtension :
    public CObject,
    public IAppModeExtension,
    public IExtension
{
public:
    CIndexerAppModeExtension();

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IAppModeExtension interface implementation
    /// @{
    virtual string GetModeName() const;
    virtual void SetWorkbench(CWorkbench* wb);
    /// @}

    void OnFileOpen(wxCommandEvent& event);
    void OnCloseWorkspace(wxCommandEvent& event);

private:
    CWorkbench* m_Workbench;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE_EDIT___INDEXER_APP_MODE__HPP
