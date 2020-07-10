/*  $Id: wiggle_exporter.cpp 38522 2017-05-22 18:19:49Z evgeniev $
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
 * Authors: Vladislav Evgeniev
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>

#include <objects/seq/Seq_annot.hpp>

#include <gui/framework/app_job_task.hpp>
#include <gui/packages/pkg_sequence/wiggle_export_job.hpp>

#include "wiggle_exporter.hpp"
#include "wiggle_export_page.hpp"

BEGIN_NCBI_SCOPE

class CWiggleExporter :
    public CObject,
    public IUIExportTool,
    public IRegSettings
{
public:
    CWiggleExporter();
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

    CUIObject           m_Descriptor;
    wxWindow            *m_ParentWindow;
    CWiggleExportPage   *m_Panel;
    string              m_RegPath;

    mutable CWiggleExportParams m_Params;
    mutable wxString            m_FileName;

    /// tool manager state (int the Run Tool wizard)
    EState  m_State;
};

CWiggleExporter::CWiggleExporter()
    :   m_Descriptor("Wiggle File", ""),
        m_ParentWindow(0),
        m_Panel(0),
        m_State(eInvalid)
{
    m_Descriptor.SetLogEvent("exporters");
}

const IUIObject& CWiggleExporter::GetDescriptor() const
{
    return m_Descriptor;
}

void CWiggleExporter::SetServiceLocator(IServiceLocator*)
{
}

void CWiggleExporter::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}

void CWiggleExporter::InitUI()
{
    m_State = eParams;
}

void CWiggleExporter::CleanUI()
{
    m_Panel = 0;
    m_State = eInvalid;
}

wxPanel* CWiggleExporter::GetCurrentPanel()
{
    if (m_Panel == NULL) {
        m_Panel = new CWiggleExportPage(m_ParentWindow);
        m_Params.UpdateFileName(m_FileName);
        m_Panel->SetData(m_Params);
        m_Panel->TransferDataToWindow();

        if (!m_RegPath.empty()) {
            m_Panel->SetRegistryPath(m_RegPath + ".WiggleParamsPanel");
            m_Panel->LoadSettings();
        }
    }

    return m_Panel;
}

bool CWiggleExporter::IsFinalState()
{
    return m_State == eParams;
}

bool CWiggleExporter::IsCompletedState()
{
    return m_State == eCompleted;
}

bool CWiggleExporter::CanDo(EAction action)
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

bool CWiggleExporter::DoTransition(EAction action)
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

IAppTask* CWiggleExporter::GetTask()
{
    CIRef<IAppJob> job(new CWiggleExportJob(m_Params));
    return new CAppJobTask(*job, true);
}

void CWiggleExporter::SetObjects(const TConstScopedObjects& input)
{
    ITERATE(TConstScopedObjects, it, input) {
        if (dynamic_cast<const CSerialObject*>(it->object.GetPointer()) != NULL)
            m_Params.SetObjects().push_back(*it);
    }
}

void CWiggleExporter::SetFileName(const string& filename)
{
    m_FileName = wxString::FromUTF8(filename.c_str());
}

/// IUIExportTool
void CWiggleExporter::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_Params.SetRegistryPath(m_RegPath + ".WiggleParams");
}

void CWiggleExporter::SaveSettings() const
{
    m_Params.SaveSettings();
    if (m_Panel) m_Panel->SaveSettings();
}

void CWiggleExporter::LoadSettings()
{
    m_Params.LoadSettings();
}

string  CWiggleExporterFactory::GetExtensionIdentifier() const
{
    static string sid("wiggle_exporter_factory");
    return sid;
}


string CWiggleExporterFactory::GetExtensionLabel() const
{
    static string slabel("Wiggle Exporter Factory");
    return slabel;
}

IUIExportTool*  CWiggleExporterFactory::CreateInstance() const
{
    return new CWiggleExporter();
}

bool CWiggleExporterFactory::TestInputObjects(TConstScopedObjects& objects) const
{
    ITERATE(TConstScopedObjects, it, objects) {
        if (dynamic_cast<const objects::CSeq_annot*>(it->object.GetPointer()) != NULL)
            return true;
    }
    return false;
}

END_NCBI_SCOPE
