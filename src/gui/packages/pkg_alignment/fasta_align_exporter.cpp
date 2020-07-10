/*  $Id: fasta_align_exporter.cpp 38593 2017-06-01 17:31:50Z evgeniev $
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

#include <gui/objutils/obj_convert.hpp>

#include <serial/typeinfo.hpp>

#include <objects/seqloc/Seq_loc.hpp>

#include "fasta_align_exporter.hpp"
#include "fasta_align_export_page.hpp"

#include <gui/framework/app_job_task.hpp>

#include <gui/packages/pkg_alignment/fasta_align_export_job.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

class  CFastaAlignExporter :
    public CObject,
    public IUIExportTool,
    public IRegSettings
{
public:
    CFastaAlignExporter();
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
    virtual void                SetLocations(const TRelatedObjectsMap&) {}
    virtual void                SetObjects(const TConstScopedObjects& input);
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

    CUIObject               m_Descriptor;
    wxWindow                *m_ParentWindow;
    CFastaAlignExportPage   *m_Panel;
    string                  m_RegPath;

    TConstScopedObjects     m_Objects;
    CFastaAlignExportParams m_Params;

    /// tool manager state (int the Run Tool wizard)
    EState  m_State;
};

CFastaAlignExporter::CFastaAlignExporter()
    : m_Descriptor("FASTA Alignment File", ""),
    m_ParentWindow(),
    m_Panel(),
    m_State(eInvalid)
{
    m_Descriptor.SetLogEvent("exporters");
}

const IUIObject& CFastaAlignExporter::GetDescriptor() const
{
    return m_Descriptor;
}

void CFastaAlignExporter::SetServiceLocator(IServiceLocator* srv_locator)
{
}

void CFastaAlignExporter::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}

void CFastaAlignExporter::InitUI()
{
    m_State = eParams;
}

void CFastaAlignExporter::CleanUI()
{
    m_State = eInvalid;
    m_Panel = NULL;
}

wxPanel* CFastaAlignExporter::GetCurrentPanel()
{
    if (m_Panel == NULL) {
        m_Panel = new CFastaAlignExportPage(m_ParentWindow);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(m_Objects);
        m_Panel->TransferDataToWindow();

        if (!m_RegPath.empty()) {
            m_Panel->SetRegistryPath(m_RegPath + ".FastaAlignParamsPanel");
            m_Panel->LoadSettings();
        }
    }

    return m_Panel;
}

bool CFastaAlignExporter::IsFinalState()
{
    return m_State == eParams;
}

bool CFastaAlignExporter::IsCompletedState()
{
    return m_State == eCompleted;
}

bool CFastaAlignExporter::CanDo(EAction action)
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

bool CFastaAlignExporter::DoTransition(EAction action)
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

IAppTask* CFastaAlignExporter::GetTask()
{
    CIRef<IAppJob> job(new CFastaAlignExportJob(m_Params));
    return new CAppJobTask(*job, true);
}

void CFastaAlignExporter::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Params.SetRegistryPath(m_RegPath + ".FastaAlignParams");
}

void CFastaAlignExporter::SetObjects( const TConstScopedObjects& input )
{
    ITERATE(TConstScopedObjects, it, input) {
        const objects::CSeq_annot* annot = dynamic_cast<const objects::CSeq_annot*>(it->object.GetPointer());
        if (nullptr == annot)
            continue;

        if (!annot->IsAlign())
            continue;

        for (auto &align : annot->GetData().GetAlign()) {
            m_Objects.push_back(SConstScopedObject(align.GetPointer(), const_cast<CScope*>(it->scope.GetPointer())));
        }
    }
}

void CFastaAlignExporter::SaveSettings() const
{
    m_Params.SaveSettings();
    if (m_Panel) m_Panel->SaveSettings();
}

void CFastaAlignExporter::LoadSettings()
{
    m_Params.LoadSettings();
}

string  CFastaAlignExporterFactory::GetExtensionIdentifier() const
{
    static string sid("fasta_align_exporter_factory");
    return sid;
}


string CFastaAlignExporterFactory::GetExtensionLabel() const
{
    static string slabel("FASTA Alignment Exporter Factory");
    return slabel;
}

IUIExportTool*  CFastaAlignExporterFactory::CreateInstance() const
{
    return new CFastaAlignExporter();
}

bool CFastaAlignExporterFactory::TestInputObjects(TConstScopedObjects& objects) const
{
    ITERATE(TConstScopedObjects, it, objects) {
        const objects::CSeq_annot* annot = dynamic_cast<const objects::CSeq_annot*>(it->object.GetPointer());
        if (nullptr == annot)
            continue;

        if (!annot->IsAlign())
            continue;

        return true;
    }
    return false;
}

END_NCBI_SCOPE
