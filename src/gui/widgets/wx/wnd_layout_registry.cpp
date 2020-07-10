/*  $Id: wnd_layout_registry.cpp 39926 2017-11-27 16:55:21Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/wnd_layout_registry.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const wxChar* kLayoutPath = wxT("<home>/window_layout2.asn");
static const char* kWindowLayouts2 = "WindowLayouts2";
static const char* kWindowPos = "WindowPos.";

CWndLayoutReg::CWndLayoutReg()
{
}

CWndLayoutReg& CWndLayoutReg::GetInstance()
{
    static CWndLayoutReg inst;
    return inst;
}

CRegistryReadView CWndLayoutReg::GetReadView(const string& section) const
{
    if (!m_Registry)
        return CRegistryReadView();

    return m_Registry->GetReadView(kWindowPos + GetVideoId() + "." + section);
}

CRegistryWriteView CWndLayoutReg::GetWriteView(const string& section)
{
    if (!m_Registry)
        return CRegistryWriteView();

    return m_Registry->GetWriteView(kWindowPos + GetVideoId() + "." + section);
}

void CWndLayoutReg::Load()
{
    wxString path = CSysPath::ResolvePathExisting(wxT("<std>/etc/def_window_layout.asn"));
    if (!path.empty()) {
        try {
            CNcbiIfstream istr(path.fn_str());
            m_Default.Reset(new CRegistryFile(istr));
        } NCBI_CATCH("Reading " + string(path.ToUTF8()));
    }

    path = CSysPath::ResolvePathExisting(kLayoutPath);
    if (!path.empty()) {
        try {
            CNcbiIfstream istr(path.fn_str());
            m_Registry.Reset(new CRegistryFile(istr));
        } NCBI_CATCH("Reading " + string(path.ToUTF8()));
    }

    if (!m_Registry)
        m_Registry.Reset(new CRegistryFile());
}

void CWndLayoutReg::Save()
{
    if (!m_Registry)
        return;

    wxString path = CSysPath::ResolvePath(kLayoutPath);
    try {
        CNcbiOfstream ostr(path.fn_str());
        m_Registry->Write(ostr);
    } NCBI_CATCH("Writing " + string(path.ToUTF8()));
}

CConstRef<CUser_object> CWndLayoutReg::GetDefaultLayout()
{
    if (m_Default) {
        CRegistryReadView view = m_Default->GetReadView(kWindowLayouts2);
        CConstRef<CUser_field> field = view.GetField("Default");

        if (field && field->CanGetData() && field->GetData().IsObject()) {
            CConstRef<CUser_object> layout(&field->GetData().GetObject());
            return layout;
        }
    }

    return CConstRef<CUser_object>();
}

CConstRef<CUser_object> CWndLayoutReg::GetLayout()
{
    if (m_Registry) {
        CRegistryReadView view = m_Registry->GetReadView(kWindowLayouts2);
        CConstRef<CUser_field> field = view.GetField(GetVideoId());

        if (field && field->CanGetData() && field->GetData().IsObject()) {
            CConstRef<CUser_object> layout(&field->GetData().GetObject());
            return layout;
        }

        LOG_POST(Error << "Failed to load window layout: " << GetVideoId());
    }

    return GetDefaultLayout();
}

void CWndLayoutReg::SetLayout(objects::CUser_object* layout)
{
    if (!m_Registry)
        return;

    CRegistryWriteView view = m_Registry->GetWriteView(kWindowLayouts2);
    if (layout) {
        CRef<CUser_field> field = view.SetField(GetVideoId());
        field->SetData().SetObject(*layout);
    }
    else {
        view.DeleteField(GetVideoId());
    }
}

void CWndLayoutReg::Reset()
{
    if (!m_Registry)
        return;
    m_Registry->GetWriteView(kWindowLayouts2).DeleteField(GetVideoId());
    m_Registry->GetWriteView(kWindowPos).DeleteField(GetVideoId());
}

END_NCBI_SCOPE
