/*  $Id: asn_exporter.cpp 39341 2017-09-13 19:09:28Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>

#include "asn_exporter.hpp"
#include "asn_export_page.hpp"

#include <gui/core/asn_export_job.hpp>
#include <gui/framework/app_export_task.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

class CAsnExporter :
    public CObject,
    public IUIExportTool,
    public IRegSettings
{
public:
    CAsnExporter();
    /// @name IUIToolManager interface implementation
    /// @{
    virtual const IUIObject&    GetDescriptor() const;
    virtual void                SetServiceLocator(IServiceLocator* srv_locator);
    virtual void                SetParentWindow(wxWindow* parent);
    virtual void                InitUI();
    virtual void                CleanUI();
    virtual wxPanel*            GetCurrentPanel();
    virtual bool                IsFinalState();
    virtual bool                IsCompletedState();
    virtual bool                CanDo(EAction action);
    virtual bool                DoTransition(EAction action);
    virtual IAppTask*           GetTask();
    /// @}

    /// @name IUIExportTool interface implementation
    /// @{
    virtual void                SetLocations(const TRelatedObjectsMap& /*input_map*/) {}
    virtual void                SetObjects(const TConstScopedObjects& input);
    virtual void                SetFileName(const string& filename);
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void                SetRegistryPath(const string& path);
    virtual void                SaveSettings() const;
    virtual void                LoadSettings();
    /// @}

protected:
    /// defines states of the Tool Manager
    enum EState {
        eInvalid = -1,  // initial
        eParams,        // displaying parameters panel
        eCompleted      // done
    };

    CUIObject       m_Descriptor;
    wxWindow*       m_ParentWindow;
    CAsnExportPage* m_Panel;
    string          m_RegPath;

    mutable CAsnExportParams m_Params;
    mutable wxString         m_FileName;

    /// tool manager state (int the Run Tool wizard)
    EState  m_State;
};

CAsnExporter::CAsnExporter()
:   m_Descriptor("ASN File", ""),
    m_ParentWindow(),
    m_Panel(),
    m_State(eInvalid)
{
    m_Descriptor.SetLogEvent("exporters");
}

const IUIObject& CAsnExporter::GetDescriptor() const
{
    return m_Descriptor;
}

void CAsnExporter::SetServiceLocator(IServiceLocator*)
{
}

void CAsnExporter::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}

void CAsnExporter::InitUI()
{
    m_State = eParams;
}

void CAsnExporter::CleanUI()
{
    m_State = eInvalid;
    m_Panel = NULL;
}

wxPanel* CAsnExporter::GetCurrentPanel()
{
    if (m_Panel == NULL) {
        m_Panel = new CAsnExportPage(m_ParentWindow);
        m_Params.UpdateFileName(m_FileName);
        m_Panel->SetData(m_Params);
        m_Panel->TransferDataToWindow();

        if (!m_RegPath.empty()) {
            m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
            m_Panel->LoadSettings();
        }
    }

    return m_Panel;
}

bool CAsnExporter::IsFinalState()
{
    return m_State == eParams;
}

bool CAsnExporter::IsCompletedState()
{
    return m_State == eCompleted;
}

bool CAsnExporter::CanDo(EAction action)
{
    switch (m_State)
    {
    case eInvalid:
        return action == eNext;
    case eParams:
        return action == eNext;
    case eCompleted:
        return false; // nothing left to do
    default:
        _ASSERT(false);
        return false;
    }
}

bool CAsnExporter::DoTransition(EAction action)
{
    if(m_State == eInvalid  &&  action == eNext) {
        m_State = eParams;
        return true;
    }
    else if (m_State == eParams && action == eNext) {
        if (m_Panel->TransferDataFromWindow()) {
            m_Params = m_Panel->GetData();
            m_State = eCompleted;
            return true;
        }
        return false;
    }

    return false;
}

IAppTask* CAsnExporter::GetTask()
{
    CIRef<IAppJob> job(new CAsnExportJob(m_Params));
    return new CAppExportTask(*job, m_Params.GetFileName());
}

void CAsnExporter::SetObjects(const TConstScopedObjects& input)
{
    for (const auto& o : input) {
        if (dynamic_cast<const CSerialObject*>(o.object.GetPointer()))
            m_Params.SetObjects().push_back(o);
    }
}

void CAsnExporter::SetFileName(const string& filename)
{
    m_FileName = wxString::FromUTF8(filename.c_str());
}

/// IUIExportTool
void CAsnExporter::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_Params.SetRegistryPath(m_RegPath + ".Params");
}

void CAsnExporter::SaveSettings() const
{
    m_Params.SaveSettings();
    if (m_Panel) m_Panel->SaveSettings();
}

void CAsnExporter::LoadSettings()
{
    m_Params.LoadSettings();
}

string  CAsnExporterFactory::GetExtensionIdentifier() const
{
    static string sid("asn_exporter_factory");
    return sid;
}


string CAsnExporterFactory::GetExtensionLabel() const
{
    static string slabel("ASN Exporter Factory");
    return slabel;
}

IUIExportTool*  CAsnExporterFactory::CreateInstance() const
{
    return new CAsnExporter();
}

bool CAsnExporterFactory::TestInputObjects(TConstScopedObjects& objects) const
{
    ITERATE(TConstScopedObjects, it, objects) {
        if (dynamic_cast<const CSerialObject*>(it->object.GetPointer()) != NULL)
            return true;
    }
    return false;
}

END_NCBI_SCOPE
