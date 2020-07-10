/*  $Id: test_views.cpp 23394 2011-03-18 18:36:24Z falkrb $
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

#include <gui/framework/demo/test_views.hpp>
#include <wx/sizer.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CwxGLTestView
CViewTypeDescriptor CwxGLTestView::m_TypeDescr(
    "OpenGL Test View", // type name
    "", // icon alias TODO
    "OpenGL Test View hint",
    "OpenGL Test View description",
    "", // help ID
    "Test",     // category
    false); // not a singleton


CwxGLTestView::CwxGLTestView()
:   m_Client(NULL)
{
}


const CViewTypeDescriptor& CwxGLTestView::GetTypeDescriptor() const
{
    return m_TypeDescr;
}


void CwxGLTestView::SetWorkbench(IWorkbench* workbench)
{
}


void CwxGLTestView::CreateViewWindow(wxWindow* parent)
{
    m_Client = new CGLTestWMClient(parent);
}


void CwxGLTestView::DestroyViewWindow()
{
    if(m_Client)    {
        m_Client->Destroy();
        m_Client = NULL;
    }
}


wxWindow* CwxGLTestView::GetWindow()
{
    return m_Client;
}

string CwxGLTestView::GetClientLabel(IWMClient::ELabel) const
{   
    return "CwxGLTestView WM Client";   
}

string CwxGLTestView::GetClientLabel() const
{
    return m_TypeDescr.GetLabel();
}


string CwxGLTestView::GetIconAlias() const
{
    return m_TypeDescr.GetIconAlias();
}


const CRgbaColor* CwxGLTestView::GetColor() const
{
    return m_Client->GetColor();
}


void CwxGLTestView::SetColor(const CRgbaColor& color)
{
    m_Client->SetColor(color);
}


const wxMenu* CwxGLTestView::GetMenu()
{
    return m_Client->GetMenu();
}


void CwxGLTestView::UpdateMenu(wxMenu& root_menu)
{
    return m_Client->UpdateMenu(root_menu);
}


IWMClient::CFingerprint CwxGLTestView::GetFingerprint() const
{
    return CFingerprint(m_TypeDescr.GetLabel(), false);
}


///////////////////////////////////////////////////////////////////////////////
/// CwxGLTestViewFactory
void CwxGLTestViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    //TODO
}


const CViewTypeDescriptor& CwxGLTestViewFactory::GetViewTypeDescriptor() const
{
    return CwxGLTestView::m_TypeDescr;
}


IView* CwxGLTestViewFactory::CreateInstance() const
{
    return new CwxGLTestView();
}


IView* CwxGLTestViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    TFingerprint print(CwxGLTestView::m_TypeDescr.GetLabel(), false);
    if(print == fingerprint)   {
        return new CwxGLTestView();
    }
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
/// CClockView
CViewTypeDescriptor CClockView::m_TypeDescr(
    "Clock View", // type name
    "", // icon alias TODO
    "Clock Test View hint.",
    "Clock Test View description.",
    "", // help ID
    "Test",     // category
    false); // not a singleton


const CViewTypeDescriptor& CClockView::GetTypeDescriptor() const
{
    return m_TypeDescr;
}


void CClockView::SetWorkbench(IWorkbench* workbench)
{
}


void CClockView::CreateViewWindow(wxWindow* parent)
{
    m_Client = new CClockPanelWMClient(parent);
}


void CClockView::DestroyViewWindow()
{
    if(m_Client)    {
        m_Client->Destroy();
        m_Client = NULL;
    }
}


IWMClient::CFingerprint CClockView::GetFingerprint() const
{
    return IWMClient::CFingerprint(m_TypeDescr.GetLabel(), false);
}


///////////////////////////////////////////////////////////////////////////////
/// CClockViewFactory
void CClockViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
}


const CViewTypeDescriptor& CClockViewFactory::GetViewTypeDescriptor() const
{
    return CClockView::m_TypeDescr;
}


IView* CClockViewFactory::CreateInstance() const
{
    return new CClockView();
}


IView* CClockViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    IWMClient::CFingerprint print(CClockView::m_TypeDescr.GetLabel(), false);
    if(print == fingerprint)   {
        return new CClockView();
    }
    return NULL;
}


END_NCBI_SCOPE
