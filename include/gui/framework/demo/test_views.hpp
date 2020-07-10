#ifndef GUI_FRAMEWORK_DEMO___TEST_VIEWS__HPP
#define GUI_FRAMEWORK_DEMO___TEST_VIEWS__HPP

/*  $Id: test_views.hpp 20272 2009-10-27 20:56:42Z katargir $
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
 * Authors: Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/framework/view.hpp>

#include <gui/widgets/wx/test_clients.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CwxGLTestView
class CwxGLTestView :
    public CObject,
    public IView,
    public IWMClient
{
    friend class CwxGLTestViewFactory;
public:
    CwxGLTestView();

    /// @name IView interface implementation
    /// @{
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    SetWorkbench(IWorkbench* workbench);
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    /// @}

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow*      GetWindow();
    virtual wxEvtHandler* GetCommandHandler() { return GetWindow()->GetEventHandler(); }
    virtual string GetClientLabel(IWMClient::ELabel ltype) const;
    virtual string  GetClientLabel() const;
    virtual string  GetIconAlias() const;
    virtual const CRgbaColor* GetColor() const;
    virtual void SetColor(const CRgbaColor& color);
    virtual const wxMenu*  GetMenu();
    virtual void    UpdateMenu(wxMenu& root_menu);
    virtual CFingerprint  GetFingerprint() const;
    /// @}


protected:
    static CViewTypeDescriptor m_TypeDescr;
    CGLTestWMClient* m_Client;
};


///////////////////////////////////////////////////////////////////////////////
/// CwxGLTestViewFactory
class CwxGLTestViewFactory :
    public CObject,
    public IViewFactory
{
public:
    virtual void    RegisterIconAliases(wxFileArtProvider& provider);
    virtual const CViewTypeDescriptor&   GetViewTypeDescriptor() const;
    virtual IView*  CreateInstance() const;
    virtual IView*  CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
};


///////////////////////////////////////////////////////////////////////////////
/// CClockView
class CClockView :
    public CObject,
    public IView
{
    friend class CClockViewFactory;
public:
    /// @name IView interface implementation
    /// @{
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    SetWorkbench(IWorkbench* workbench);
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    /// @}

    /// overriding CClockPanelWMClient::GetFingerprint()
    virtual IWMClient::CFingerprint  GetFingerprint() const;

protected:
    static CViewTypeDescriptor m_TypeDescr;

    CClockPanelWMClient* m_Client;
};


///////////////////////////////////////////////////////////////////////////////
/// CClockViewFactory
class CClockViewFactory :
    public CObject,
    public IViewFactory
{
public:
    virtual void    RegisterIconAliases(wxFileArtProvider& provider);
    virtual const CViewTypeDescriptor&   GetViewTypeDescriptor() const;
    virtual IView*  CreateInstance() const;
    virtual IView*  CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
};


END_NCBI_SCOPE


#endif
// GUI_FRAMEWORK_DEMO___TEST_VIEWS__HPP
