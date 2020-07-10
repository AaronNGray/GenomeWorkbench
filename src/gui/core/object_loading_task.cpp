/*  $Id: object_loading_task.cpp 40456 2018-02-16 17:17:24Z katargir $
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

#include <gui/core/object_loading_task.hpp>
#include <gui/core/prefetch_seq_descr.hpp>
#include <gui/core/project_service.hpp>

#include <objects/seq/Annotdesc.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>

#include <gui/utils/app_job_impl.hpp>


#include <objtools/edit/seq_entry_edit.hpp>

#include <gui/core/project_item_extension.hpp>
#include <gui/utils/extension_impl.hpp>
#include <gui/objutils/project_item_extra.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CObjectLoadingTask

bool CObjectLoadingTask::m_SaveFilePath = false;

CObjectLoadingTask::CObjectLoadingTask(CProjectService* service, IObjectLoader& object_loader, CSelectProjectOptions& options)
 : CAppJobTask(true, object_loader.GetDescription()),
   m_Service(service), m_ObjectLoader(&object_loader), m_Options(options)
{
    CIRef<IExecuteUnit> execute_unit(dynamic_cast<IExecuteUnit*>(&object_loader));
    _ASSERT(execute_unit);
    m_Job.Reset(new CAppJobExecuteUnit(*execute_unit, object_loader.GetDescription()));
}

static void s_CallExtensionProjectItemCreated(vector<IProjectItemExtension*>& clients, CProjectItem& item, IServiceLocator* srvLocator)
{
    ITERATE(vector<IProjectItemExtension*>, it, clients) {
        string extName = "Unknown extension";
        IExtension* ext = dynamic_cast<IExtension*>(*it);
        if (ext) extName = ext->GetExtensionIdentifier();
        try {
            (*it)->ProjectItemCreated(item, srvLocator);
        } NCBI_CATCH("Project item created \"" + extName + "\" error.");
    }
}

void CObjectLoadingTask::AddProjectItemExtra(const string& tag, const string& value)
{
    m_ProjectItemExtra[tag] = value;
}

IAppTask::ETaskState CObjectLoadingTask::Run()
{
    ETaskState state = CAppJobTask::Run();

    if (m_State == eInitial) {
        CIRef<IExecuteUnit> execute_unit(dynamic_cast<IExecuteUnit*>(m_ObjectLoader.GetPointer()));
        if (!execute_unit || !execute_unit->PreExecute())
            return eCanceled;
    }
    else if (state == eCompleted) {
        CIRef<IExecuteUnit> execute_unit(dynamic_cast<IExecuteUnit*>(m_ObjectLoader.GetPointer()));
        if (!execute_unit || !execute_unit->PostExecute())
            return eCanceled;

        if (!AddObjects(m_Service->GetServiceLocator(),
            m_ObjectLoader->GetObjects(), dynamic_cast<CLoaderDescriptor*>(m_ObjectLoader->GetLoader()),
            m_Options, m_ProjectItemExtra)) {
            return eCanceled;
        }
        return eCompleted;
    }

    return state;
}

bool CObjectLoadingTask::AddObjects(
	IServiceLocator* serviceLocator,
	IObjectLoader::TObjects& objects,
	CLoaderDescriptor* loader,
	CSelectProjectOptions&  options,
	const map<string, string>& projectItemExtra)
{
	vector<IProjectItemExtension*> clients;
	GetExtensionAsInterface(EXT_POINT__PROJECT_ITEM_EXTENSION, clients);

	CSelectProjectOptions::TItems items;
	NON_CONST_ITERATE(IObjectLoader::TObjects, it, objects) {
		CSerialObject* so = dynamic_cast<CSerialObject*>(it->GetObjectPtr());
		if (so) {
			if (CSeq_entry* se = dynamic_cast<CSeq_entry*>(so)) {
				edit::AddLocalIdUserObjects(*se);
				se->ReassignConflictingIds();
			}
			else if (CBioseq* bioseq = dynamic_cast<CBioseq*>(so)) {
				CRef<CSeq_entry> se(new CSeq_entry());
				se->SetSeq(*bioseq);
				edit::AddLocalIdUserObjects(*se);
				se->ReassignConflictingIds();
			}
			else if (CBioseq_set* bioseqSet = dynamic_cast<CBioseq_set*>(so)) {
				CRef<CSeq_entry> se(new CSeq_entry());
				se->SetSet(*bioseqSet);
				edit::AddLocalIdUserObjects(*se);
				se->ReassignConflictingIds();
			}
			else if (CSeq_submit* sub = dynamic_cast<CSeq_submit*>(so)) {
				if (sub && sub->IsSetData() && sub->IsEntrys()) {
					NON_CONST_ITERATE(CSeq_submit::TData::TEntrys, ent, sub->SetData().SetEntrys()) {
						edit::AddLocalIdUserObjects(**ent);
						(*ent)->ReassignConflictingIds();
					}
				}
			}

			CRef<CProjectItem> item(new CProjectItem());
			item->SetObject(*so);
			item->SetLabel(it->GetDescription());

			list<string> comment;
			NStr::Split(it->GetComment(), "\n\r", comment, NStr::fSplit_Tokenize);
			ITERATE(list<string>, iter, comment) {
				if (!iter->empty()) {
					CRef<CAnnotdesc> annot(new CAnnotdesc());
					annot->SetComment(*iter);
					item->SetDescr().push_back(annot);
				}
			}

			for (auto t : projectItemExtra)
				CProjectItemExtra::SetStr(*item, t.first, t.second);

			string filePath = it->GetFileName();
			if (!filePath.empty()) {
				if (m_SaveFilePath)
					CProjectItemExtra::SetStr(*item, "OriginalPath", filePath);

				wxFileName fileName(wxString::FromUTF8(filePath.c_str()));
				string fname(fileName.GetFullName().ToUTF8());
				CProjectItemExtra::SetStr(*item, "OriginalFile", fname);
			}

			s_CallExtensionProjectItemCreated(clients, *item, serviceLocator);

			items.push_back(item);
		}
	}

	CSelectProjectOptions::TData data;
	data[CRef<CLoaderDescriptor>(loader)] = items;

	if (!options.AddItemsToWorkspace(serviceLocator->GetServiceByType<CProjectService>(), data))
		return false;

	CPrefetchSeqDescr::PrefetchSeqDescr(serviceLocator, items);
	return true;
}


END_NCBI_SCOPE
