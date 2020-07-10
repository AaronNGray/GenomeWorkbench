#ifndef GUI_WIDGETS_WX__DIALOG__HPP
#define GUI_WIDGETS_WX__DIALOG__HPP

/*  $Id: dialog.hpp 39737 2017-10-31 17:03:07Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/objutils/registry.hpp>

#include <wx/dialog.h>

BEGIN_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////
/// CDialog
class NCBI_GUIWIDGETS_WX_EXPORT CDialog : public wxDialog
{
public:
    CDialog();

    /// @name IRegSettings interface
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    virtual void InitDialog();
    virtual void EndModal(int retCode);

protected:
    /// override these functions in derived classes
    virtual void    x_LoadSettings(const CRegistryReadView& /*view*/) {}
    virtual void    x_SaveSettings(CRegistryWriteView /*view*/) const {}

protected:
    string  m_RegPath;
};


END_NCBI_SCOPE

#endif //GUI_WIDGETS_WX__DIALOG__HPP
