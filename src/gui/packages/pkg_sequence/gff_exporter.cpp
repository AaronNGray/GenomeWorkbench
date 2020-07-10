/*  $Id: gff_exporter.cpp 40050 2017-12-13 18:32:01Z katargir $
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

#include <gui/objutils/obj_convert.hpp>

#include <serial/typeinfo.hpp>

#include <objects/seqloc/Seq_loc.hpp>

#include "gff_exporter.hpp"
#include "gff_export_page.hpp"

#include <gui/framework/app_export_task.hpp>
#include <gui/core/async_obj_convert.hpp>

#include <gui/packages/pkg_sequence/gff_export_job.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

class  CGffExporter :
    public CObject,
    public IUIExportTool,
    public IRegSettings
{
public:
    CGffExporter();
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

    CUIObject        m_Descriptor;
    wxWindow*        m_ParentWindow;
    CGffExportPage*  m_Panel;
    string           m_RegPath;

    TRelatedObjectsMap  m_Objects;
    CGffExportParams    m_Params;

    /// tool manager state (int the Run Tool wizard)
    EState  m_State;
};


CGffExporter::CGffExporter()
    : m_Descriptor("GFF3 File", ""),
      m_ParentWindow(),
      m_Panel(),
      m_State(eInvalid)
{
    m_Descriptor.SetLogEvent("exporters");
}

const IUIObject& CGffExporter::GetDescriptor() const
{
    return m_Descriptor;
}

void CGffExporter::SetServiceLocator(IServiceLocator*)
{
}

void CGffExporter::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}

void CGffExporter::InitUI()
{
    m_State = eParams;
}

void CGffExporter::CleanUI()
{
    m_State = eInvalid;
    m_Panel = NULL;
}

wxPanel* CGffExporter::GetCurrentPanel()
{
    if (m_Panel == NULL) {
        m_Panel = new CGffExportPage(m_ParentWindow);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_Objects);
        m_Panel->TransferDataToWindow();

        if (!m_RegPath.empty()) {
            m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
            m_Panel->LoadSettings();
        }
    }

    return m_Panel;
}

bool CGffExporter::IsFinalState()
{
    return m_State == eParams;
}

bool CGffExporter::IsCompletedState()
{
    return m_State == eCompleted;
}

bool CGffExporter::CanDo(EAction action)
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

bool CGffExporter::DoTransition(EAction action)
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

IAppTask* CGffExporter::GetTask()
{
    CIRef<IAppJob> job(new CGffExportJob(m_Params));
    return new CAppExportTask(*job, m_Params.GetFileName());
}

void CGffExporter::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Params.SetRegistryPath(m_RegPath + ".GffParams");
}

void CGffExporter::SetObjects( const TConstScopedObjects& input )
{
    m_Objects.clear();

    TConstScopedObjects convert, original;
    set<CBioseq_Handle> seqSet;

    ITERATE(TConstScopedObjects, it, input) {
        const CSeq_align* align = dynamic_cast<const CSeq_align*>(it->object.GetPointerOrNull());
        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(it->object.GetPointerOrNull());
        const CSeq_entry* seqEntry = dynamic_cast<const CSeq_entry*>(it->object.GetPointerOrNull());
        const CBioseq* bioseq = dynamic_cast<const CBioseq*>(it->object.GetPointerOrNull());
        const CSeq_loc* seqLoc = dynamic_cast<const CSeq_loc*>(it->object.GetPointerOrNull());
        const CSeq_id* seqId = dynamic_cast<const CSeq_id*>(it->object.GetPointerOrNull());

        if (align || annot || seqEntry || bioseq || seqLoc || seqId) {
            original.push_back(*it);
        }
        else {
            //convert.push_back(*it);
        }
        convert.push_back(*it);
    }

    if (!convert.empty()) {
        AsyncConvertObjects(CSeq_loc::GetTypeInfo(), convert, m_Objects);
    }

    if (!original.empty())
        m_Objects["Selection"] = original;
}

void CGffExporter::SaveSettings() const
{
    m_Params.SaveSettings();
    if (m_Panel) m_Panel->SaveSettings();
}

void CGffExporter::LoadSettings()
{
    m_Params.LoadSettings();
}

string  CGffExporterFactory::GetExtensionIdentifier() const
{
    static string sid("gff_exporter_factory");
    return sid;
}


string CGffExporterFactory::GetExtensionLabel() const
{
    static string slabel("GFF Exporter Factory");
    return slabel;
}

IUIExportTool*  CGffExporterFactory::CreateInstance() const
{
    return new CGffExporter();
}

bool CGffExporterFactory::TestInputObjects(TConstScopedObjects& objects) const
{
    ITERATE(TConstScopedObjects, it, objects) {
        if (dynamic_cast<const CSerialObject*>(it->object.GetPointer()) != NULL)
            return true;
    }
    return false;
}

END_NCBI_SCOPE
