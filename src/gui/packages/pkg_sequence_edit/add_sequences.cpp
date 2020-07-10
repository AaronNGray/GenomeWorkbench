/*  $Id: add_sequences.cpp 42167 2019-01-08 17:17:20Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>


#include <objmgr/scope.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/widgets/loaders/file_load_wizard.hpp>
#include <gui/widgets/loaders/open_objects_panel_client.hpp>
#include <gui/widgets/loaders/open_objects_dlg.hpp>
#include <gui/utils/object_loader.hpp>
#include <gui/utils/execute_unit.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <gui/packages/pkg_sequence_edit/seqid_fix_dlg.hpp>

#include <wx/msgdlg.h> 

#include <gui/packages/pkg_sequence_edit/add_sequences.hpp>


BEGIN_NCBI_SCOPE

void CAddSequences::apply(CSeq_entry_Handle topSeqEntry, ICommandProccessor* cmdProccessor)
{
    CRef<CFileLoadWizard> fileManager(new CFileLoadWizard());
    vector<string> format_ids;
    format_ids.push_back("file_loader_fasta");
    format_ids.push_back("file_loader_asn");
    format_ids.push_back("file_loader_text_align");
    fileManager->LoadFormats(format_ids);

    vector<CIRef<IOpenObjectsPanelClient> > loadManagers;
    loadManagers.push_back(CIRef<IOpenObjectsPanelClient>(fileManager.GetPointer()));

    COpenObjectsDlg dlg(NULL);
    dlg.SetSize(710, 480);

    dlg.SetRegistryPath("Dialogs.Edit.OpenTable");
    dlg.SetManagers(loadManagers);

    if (dlg.ShowModal() == wxID_OK) {
        CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
        if (!object_loader) {
            wxMessageBox(wxT("Failed to get object loader"), wxT("Error"),
                         wxOK | wxICON_ERROR);
        } else {
            IExecuteUnit* execute_unit = dynamic_cast<IExecuteUnit*>(object_loader.GetPointer());
            if (execute_unit) {
                if (!execute_unit->PreExecute())
                    return;

                if (!GUI_AsyncExecUnit(*execute_unit, wxT("Reading file(s)...")))
                    return; // Canceled
                
                if (!execute_unit->PostExecute())
                    return;
            }
            CRef<CCmdComposite> cmd (new CCmdComposite("Import File"));
            const IObjectLoader::TObjects& objects = object_loader->GetObjects();
            bool has_nuc = false;
            bool has_prot = false;
            bool success = true;
            CRef<objects::CSeq_entry> entry_to_add(new objects::CSeq_entry());
            entry_to_add->SetSet().SetClass(objects::CBioseq_set::eClass_genbank);
            ITERATE(IObjectLoader::TObjects, obj_it, objects) {
                const CObject& ptr = obj_it->GetObject();
                const objects::CSeq_entry* entry = dynamic_cast<const objects::CSeq_entry*>(&ptr);
                if (entry) {
                    CRef<objects::CSeq_entry> add(new objects::CSeq_entry());
                    add->Assign(*entry);
                    entry_to_add->SetSet().SetSeq_set().push_back(add);
                } else {
                    const objects::CSeq_submit* submit = dynamic_cast<const objects::CSeq_submit*>(&ptr);
                    if (submit) {
                        ITERATE (objects::CSeq_submit::TData::TEntrys, sit, submit->GetData().GetEntrys()) {
                            CRef<objects::CSeq_entry> add(new objects::CSeq_entry());
                            add->Assign(**sit);
                            entry_to_add->SetSet().SetSeq_set().push_back(add);
                        }
                    }
                }
                if (!success) {
                    break;
                }
            }
            if (objects.size() == 0) {
                success = false;
                wxMessageBox(wxT("Unable to read from file"), wxT("Error"),
                             wxOK | wxICON_ERROR, NULL);
            } else {
                if (entry_to_add->GetSet().IsSetSeq_set() && entry_to_add->GetSet().GetSeq_set().size() > 0) {
                    CRef<objects::CSeq_table> new_ids = GetIdsFromSeqEntry(*entry_to_add);
                    CRef<objects::CSeq_table> old_ids = GetIdsFromSeqEntry(*(topSeqEntry.GetCompleteSeq_entry()));
                    CRef<objects::CSeqTable_column> problems = GetSeqIdProblems(new_ids, old_ids, 100);
                    if (problems) {
                        if (wxMessageBox(ToWxString(SummarizeIdProblems(problems) + " Do you want to fix the new sequence IDs?"), wxT("Error"),
                                         wxOK | wxCANCEL | wxICON_ERROR, NULL) == wxCANCEL) {
                            success = false;
                        } else {
                            CSeqIdFixDlg dlg(NULL, entry_to_add, topSeqEntry.GetCompleteSeq_entry());
                            bool done = false;
                            while (!done) {
                                if (dlg.ShowModal() == wxID_OK) {
                                    CRef<objects::CSeq_table> repl_ids = dlg.GetReplacementTable();
                                    if (repl_ids) {
                                        ApplyReplacementIds (*entry_to_add, repl_ids);
                                        dlg.Close();        
                                        done = true;
                                    }
                                } else {
                                    done = true;
                                    success = false;
                                }
                            }
                        }
                    }
                    if (success) {
                        bool create_general_only = objects::edit::IsGeneralIdProtPresent(topSeqEntry);
                        success = AddSeqEntryToSeqEntry(entry_to_add, topSeqEntry, cmd, has_nuc, has_prot, create_general_only);
                    }
                }
            }
            if (success) {
                cmdProccessor->Execute(cmd);
            }
        }
    }
}

END_NCBI_SCOPE
