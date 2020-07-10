/*  $Id: 5col_exporter.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/objutils/registry.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include "5col_exporter.hpp"
#include "5col_export_page.hpp"

#include <gui/framework/app_export_task.hpp>

#include <gui/packages/pkg_sequence/5col_export_job.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

class  C5ColExporter :
    public CObject,
    public IUIExportTool,
    public IRegSettings
{
public:
    C5ColExporter();
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
    virtual void                SetLocations(const TRelatedObjectsMap& input_map);
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
    C5ColExportPage* m_Panel;
    string           m_RegPath;

    TRelatedObjectsMap  m_Objects;
    wxString            m_FileName;
    SConstScopedObject  m_SeqLoc;
    C5ColExportParams   m_Params;

    /// tool manager state (int the Run Tool wizard)
    EState  m_State;
};


C5ColExporter::C5ColExporter()
    : m_Descriptor("5 Column Feature File", ""),
      m_ParentWindow(),
      m_Panel(),
      m_State(eInvalid)
{
    m_Descriptor.SetLogEvent("exporters");
}

const IUIObject& C5ColExporter::GetDescriptor() const
{
    return m_Descriptor;
}

void C5ColExporter::SetServiceLocator(IServiceLocator*)
{
}

void C5ColExporter::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}

void C5ColExporter::InitUI()
{
    m_State = eParams;
}

void C5ColExporter::CleanUI()
{
    m_State = eInvalid;
    m_Panel = NULL;
}

wxPanel* C5ColExporter::GetCurrentPanel()
{
    if (m_Panel == NULL) {
        m_Panel = new C5ColExportPage(m_ParentWindow);
        m_Panel->SetData(m_Params);
        m_Panel->SetFileName(m_FileName);
        m_Panel->SetObjects(&m_Objects);
        m_Panel->TransferDataToWindow();

        if (!m_RegPath.empty()) {
            m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
            m_Panel->LoadSettings();
        }
    }

    return m_Panel;
}

bool C5ColExporter::IsFinalState()
{
    return m_State == eParams;
}

bool C5ColExporter::IsCompletedState()
{
    return m_State == eCompleted;
}

bool C5ColExporter::CanDo(EAction action)
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

bool C5ColExporter::DoTransition(EAction action)
{
    if(m_State == eInvalid  &&  action == eNext) {
        m_State = eParams;
        return true;
    }
    else if (m_State == eParams && action == eNext) {
        if (m_Panel->TransferDataFromWindow()) {
            m_FileName = m_Panel->GetFileName();
            m_SeqLoc = m_Panel->GetSeqLoc();
            m_Params = m_Panel->GetData();
            m_State = eCompleted;
            return true;
        }
        return false;
    }

    return false;
}

IAppTask* C5ColExporter::GetTask()
{
    CIRef<IAppJob> job(new C5ColExportJob(m_Params, m_SeqLoc, m_FileName));
    return new CAppExportTask(*job, m_FileName);
}

void C5ColExporter::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Params.SetRegistryPath(m_RegPath + ".5ColParams");
}

void C5ColExporter::SetLocations(const TRelatedObjectsMap& input_map)
{
    m_Objects = input_map;
}

void C5ColExporter::SetObjects( const TConstScopedObjects& input )
{
    if( m_Objects.empty() ){

        ITERATE(TConstScopedObjects, it, input) {
            CObjectConverter::TRelationVector relations;
            CScope& scope = const_cast<CScope&>(*it->scope);
            CObjectConverter::FindRelations(scope, *it->object,
                CSeq_loc::GetTypeInfo()->GetName(), relations);

            ITERATE(CObjectConverter::TRelationVector, itr, relations) {
                if (relations.size() > 1 && itr == relations.begin())
                    continue;

                const CRelation& rel = **itr;
                string relName = rel.GetProperty(CSeq_loc::GetTypeInfo()->GetName());

                CRelation::TObjects related;
                rel.GetRelated(scope, *it->object, related, CRelation::fConvert_NoExpensive);

                if (!related.empty()) {
                    TConstScopedObjects& vec = m_Objects[relName];

                    ITERATE(CRelation::TObjects, it2, related)
                        vec.push_back(SConstScopedObject(it2->GetObjectPtr(), &scope));
                }
            }
        }
    }
}

static const char* kFileName = "FileName";

void C5ColExporter::SaveSettings() const
{
    m_Params.SaveSettings();

    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        view.Set(kFileName, FnToStdString( m_FileName ));
    }

    if (m_Panel) m_Panel->SaveSettings();
}

void C5ColExporter::LoadSettings()
{
    m_Params.LoadSettings();

    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_FileName =
            FnToWxString(view.GetString(kFileName, FnToStdString(m_FileName)));
    }
}

string  C5ColExporterFactory::GetExtensionIdentifier() const
{
    static string sid("z5col_exporter_factory");
    return sid;
}


string C5ColExporterFactory::GetExtensionLabel() const
{
    static string slabel("5 Column Exporter Factory");
    return slabel;
}

IUIExportTool*  C5ColExporterFactory::CreateInstance() const
{
    return new C5ColExporter();
}

bool C5ColExporterFactory::TestInputObjects(TConstScopedObjects& objects) const
{
    ITERATE(TConstScopedObjects, it, objects) {
        if (dynamic_cast<const CSerialObject*>(it->object.GetPointer()) != NULL)
            return true;
    }
    return false;
}

END_NCBI_SCOPE
