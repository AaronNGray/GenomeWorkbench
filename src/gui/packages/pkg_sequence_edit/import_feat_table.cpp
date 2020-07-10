/*  $Id: import_feat_table.cpp 45003 2020-05-06 13:38:33Z asztalos $
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
 * Authors:  Colleen Bollin, Andrea Asztalos
 */

#include <ncbi_pch.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <objtools/edit/feattable_edit.hpp>

#include <gui/framework/workbench_impl.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp> 
#include <gui/widgets/loaders/file_load_wizard.hpp>
#include <gui/widgets/loaders/open_objects_dlg.hpp>
#include <gui/widgets/loaders/5col_object_loader.hpp>
#include <gui/widgets/loaders/gff_object_loader.hpp>
#include <gui/widgets/loaders/report_loader_errors.hpp>
#include <gui/utils/execute_unit.hpp>
#include <gui/utils/extension_impl.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <gui/packages/pkg_sequence_edit/locus_tag_euk_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/packages/pkg_sequence_edit/import_feat_table.hpp>

#include <wx/richmsgdlg.h>
#include <wx/clipbrd.h>
#include <unordered_set>

#include "cmd_place_protein_feats.hpp"
#include "cmd_feat_id_xrefs_from_qualifiers.hpp"
#include "cmd_convert_to_delayed_gen-prod-set_quals.hpp"
#include "cmd_cleanup.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const wxString kfeaturetableinstructions = "\
you can import a gff3 file, an ncbi 5-column feature table file, or a fasta file with protein sequences for your coding regions.  click ok to select and load the file.\r\n\
\r\n\
if you are importing a protein fasta file, make sure that the sequence ids for the protein sequences match the nucleotide sequences on which the coding regions should be created.\r\n\
\r\n\
features tables for influenza a and b sequences can be created here:\r\n\
https://www.ncbi.nlm.nih.gov/genomes/flu/database/annotation.cgi \r\n\
\r\n\
an ncbi 5-column feature table must be a plain text file. \r\n\
\r\n\
- the header line begins with >feature lcl| \r\n\
\r\n\
- the text following \"lcl|\" must contain the sequence id of the sequences in your records.\r\n\
  for example: >feature lcl|abc-1\r\n\
  in this example, abc-1 is the sequence id. \r\n\
\r\n\
- the table is composed of 5, tab-separated columns:\r\n\
  column 1- nucleotide position of the start of the feature\r\n\
  column 2- nucleotide location of the end of a feature\r\n\
  column 3- feature type (gene, cds, etc.)\r\n\
  column 4- feature qualifier (note, product, etc.)\r\n\
  column 5- qualifier value (for example: gag protein)\r\n\
\r\n\
- the columns in the table must be separated by tabs. \r\n\
  use the tab key on your keyboard to separate each column.\r\n\
  the qualifiers follow on lines starting with three tabs. \r\n\
\r\n\
- for more feature table format information: \r\n\
https://www.ncbi.nlm.nih.gov/sequin/table.html#table layout \r\n\
\r\n\
- questions about the feature table format? write to: info@ncbi.nlm.nih.gov\r\n\
--------------------------------------------------\r\n\
example feature table for 2 sequences:\r\n\
--------------------------------------------------\r\n\
>feature lcl|abc-1\r\n\
24	1458	gene\r\n\
            gene	pb2\r\n\
24	1458	cds\r\n\
            product	polymerase pb2\r\n\
>feature lcl|abc-2\r\n\
4	985	gene\r\n\
            gene	m2\r\n\
4	29	cds\r\n\
718	985\r\n\
            product	matrix protein 2\r\n\
4	762	gene\r\n\
            gene	m1\r\n\
4	762	cds\r\n\
      product	matrix protein 1\r\n\
";

void CImportFeatTable::ShowTableInfo()
{
    static bool showDialog(true);
    if (!showDialog)
        return;

    wxRichMessageDialog dlg(NULL, wxT("Feature table instructions"), wxEmptyString, wxOK|wxCENTER);
    dlg.ShowDetailedText(kfeaturetableinstructions);
    dlg.ShowCheckBox("Don't show this dialog in the current session");
    dlg.ShowModal();

    if (dlg.IsCheckBoxChecked()) {
        showDialog = false;
    }
}


CIRef<IEditCommand> CImportFeatTable::ImportFeaturesFromFile(const wxString& workDir, const string &format_id)
{
    CIRef<IEditCommand> cmd = x_DoImportFeaturesFromFile(workDir, format_id);

    if (!NStr::IsBlank(m_Error)) {
        CGenericReportDlg* report = new CGenericReportDlg(NULL);
        report->SetTitle(wxT("Import Feature Table Report"));
        report->SetText(ToWxString(m_Error));
        report->Show(true);
    }

    if (!cmd)
        return cmd;

    return x_GatherAdditionalChanges(cmd);
}

CIRef<IEditCommand> CImportFeatTable::ImportFeaturesFromClipboard(const wxString& workDir)
{
    CIRef<IEditCommand> cmd = x_DoImportFeaturesFromClipboard(workDir);

    if (!NStr::IsBlank(m_Error)) {
        CGenericReportDlg* report = new CGenericReportDlg(NULL);
        report->SetTitle(wxT("Import Feature Table Report"));
        report->SetText(ToWxString(m_Error));
        report->Show(true);
    }

    if (!cmd)
        return cmd;

    return x_GatherAdditionalChanges(cmd);
}

CIRef<IEditCommand> CImportFeatTable::TestImport(const CSeq_annot& orig_annot, TFeatSeqIDMap& ftbl_seqid_map,
    unsigned int &startingLocusTagNumber, unsigned int &startingFeatureId, const string &locus_tag, bool euk)
{
    CIRef<IEditCommand> cmd = AddSeqAnnotToSeqEntry(orig_annot, ftbl_seqid_map,
        startingLocusTagNumber, startingFeatureId, locus_tag, euk);
    if (!cmd) {
        return cmd;
    }

    return x_GatherAdditionalChanges(cmd);
}

CIRef<IEditCommand> CImportFeatTable::x_GatherAdditionalChanges(CIRef<IEditCommand>& start_cmd)
{
    if (!start_cmd)
        return CIRef<IEditCommand>();

    CRef<CCmdComposite> composite(new CCmdComposite("Import Feature Table"));
    composite->AddCommand(*start_cmd);

    composite->AddCommand(*CIRef<IEditCommand>(
        new CCmdMoveCdsToSetLevel(m_Seh)));

    composite->AddCommand(*CIRef<IEditCommand>(
        new CCmdPlaceProteinFeats(m_Seh)));

    // assigns feature local ids to features and places Xrefs between CDS and mRNA
    composite->AddCommand(*CCmdFeatIdXrefsFromQualifiers::Create(m_Seh, { "protein_id", "orig_protein_id" }));

    composite->AddCommand(*CIRef<IEditCommand>(
        new CCmdConvertToDelayedGenProdSetQuals(m_Seh)));

    // removes Xrefs to proteins from CDS
    composite->AddCommand(*CIRef<IEditCommand>(
        new CCmdCleanup(m_Seh, false, false)));

    return CIRef<IEditCommand>(composite);
}



CIRef<IEditCommand> CImportFeatTable::x_DoImportFeaturesFromFile(const wxString& workDir, const string &format_id)
{
    CRef<CFileLoadWizard> fileManager(new CFileLoadWizard());
    vector<string> format_ids;
    if (format_id.empty())
    {
        format_ids.push_back("file_loader_gff");
        format_ids.push_back("file_loader_5col");
    }
    else
    {
        format_ids.push_back(format_id);
    }
    fileManager->LoadFormats(format_ids);
    fileManager->SetWorkDir(workDir);

    vector<CIRef<IOpenObjectsPanelClient> > loadManagers;
    loadManagers.push_back(CIRef<IOpenObjectsPanelClient>(fileManager.GetPointer()));

    COpenObjectsDlg dlg(NULL);
    dlg.SetSize(710, 480);

    dlg.SetRegistryPath("Dialogs.Edit.ImportFeatTable");
    dlg.SetManagers(loadManagers);
    if (format_id == "file_loader_5col")
        dlg.SetHelpUrl(_("https://www.ncbi.nlm.nih.gov/tools/gbench/manual11/#5-column-feature-table"));
    if (format_id == "file_loader_gff")
        dlg.SetHelpUrl(_("https://www.ncbi.nlm.nih.gov/tools/gbench/manual11/#gff3-file"));

    if (dlg.ShowModal() != wxID_OK)
        return CIRef<IEditCommand>();

    CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
    if (!object_loader) {
        wxMessageBox(wxT("Failed to get object loader"), wxT("Error"), wxOK | wxICON_ERROR);
        return CIRef<IEditCommand>();
    }

    CGffObjectLoader* gff_loader = dynamic_cast<CGffObjectLoader*>(object_loader.GetPointer());
    if (gff_loader)
        m_is_gff = true;
    
    IExecuteUnit* execute_unit = dynamic_cast<IExecuteUnit*>(object_loader.GetPointer());
    if (execute_unit) {
        if (!execute_unit->PreExecute())
            return CIRef<IEditCommand>();

        if (!GUI_AsyncExecUnit(*execute_unit, wxT("Reading file(s)...")))
            return CIRef<IEditCommand>(); // Canceled

        if (!execute_unit->PostExecute())
            return CIRef<IEditCommand>();
    }

    const IObjectLoader::TObjects& objects = object_loader->GetObjects();
    if (!objects.empty() && x_ContainsDuplicateIds(objects)) {
        return CIRef<IEditCommand>();
    }
    string msg = x_CheckConsistentProteinIds(objects);
    if (!msg.empty()) {
        wxMessageBox(ToWxString(msg), wxT("Error"), wxOK | wxICON_ERROR);
        return CIRef<IEditCommand>();
    }
    return x_CreateCommand(objects);   
}

CIRef<IEditCommand> CImportFeatTable::x_DoImportFeaturesFromClipboard(const wxString& workDir)
{
    string fname;
    if (wxTheClipboard->Open())
    {
        wxTextDataObject data;
        if (wxTheClipboard->IsSupported( wxDF_UNICODETEXT ))
            wxTheClipboard->GetData( data );
        if (data.GetText().length() == 0) {
            wxMessageBox("No data in clipboard for table");
            wxTheClipboard->Close();
            return CIRef<IEditCommand>();
        }

        CTmpFile f(CTmpFile::eNoRemove);
        f.AsOutputFile(CTmpFile::eIfExists_ReturnCurrent) << data.GetText();
        fname = f.GetFileName();
        wxTheClipboard->Close();
    }
            
    vector<wxString> fnames;
    fnames.push_back(ToWxString(fname));

    CRef<C5ColObjectLoader> execute_unit(new C5ColObjectLoader(fnames));
    if (!execute_unit) {
        wxMessageBox(wxT("Failed to get object loader"), wxT("Error"), wxOK | wxICON_ERROR);
        return CIRef<IEditCommand>();
    }
     
    CReportLoaderErrors* reporter = dynamic_cast<CReportLoaderErrors*>(execute_unit.GetPointer());
    if (reporter)
        reporter->SetWorkDir(workDir);

    if (!execute_unit->PreExecute())
    return CIRef<IEditCommand>();
    
    if (!GUI_AsyncExecUnit(*execute_unit, wxT("Reading clipboard...")))
    return CIRef<IEditCommand>(); // Canceled
    
    if (!execute_unit->PostExecute())
    return CIRef<IEditCommand>();

    CFile tmp_file(fname);
    tmp_file.Remove();
    const IObjectLoader::TObjects& objects = execute_unit->GetObjects();
    if (!objects.empty() && x_ContainsDuplicateIds(objects)) {
        return CIRef<IEditCommand>();
    }
    string msg = x_CheckConsistentProteinIds(objects);
    if (!msg.empty()) {
        wxMessageBox(ToWxString(msg), wxT("Error"), wxOK | wxICON_ERROR);
        return CIRef<IEditCommand>();
    }
    return x_CreateCommand(objects);
} 

bool CImportFeatTable::x_ContainsDuplicateIds(const IObjectLoader::TObjects& objects)
{
    unordered_set<string> product_ids;
    // check protein ids as well as transcript ids
    for (auto& obj_it : objects) {
        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(obj_it.GetObjectPtr());
        if (annot && annot->IsFtable()) {
            const CSeq_annot::C_Data::TFtable& ftbl = annot->GetData().GetFtable();
            for (auto& feat_it : ftbl) {
                if (feat_it->IsSetProduct() && 
                    feat_it->IsSetData() && 
                    (feat_it->GetData().IsCdregion() || feat_it->GetData().GetSubtype() == CSeqFeatData::eSubtype_mRNA)) {
                    if (const CSeq_id* id = feat_it->GetProduct().GetId()) {
                        auto id_string = id->GetSeqIdString(true);
                        auto res = product_ids.insert(id_string);
                        if (!res.second) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

string CImportFeatTable::x_CheckConsistentProteinIds(const IObjectLoader::TObjects& objects)
{
    if (objects.empty())
        return kEmptyStr;

    string msg;
    for (auto& obj_it : objects) {
        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(obj_it.GetObjectPtr());
        if (annot && annot->IsFtable()) {
            const auto& ftbl = annot->GetData().GetFtable();
            if (ftbl.size() == 1)
                continue;

            auto feat_it = ftbl.begin();
            while (feat_it != ftbl.end()) {
                if ((*feat_it)->IsSetData() && (*feat_it)->GetData().IsCdregion()) {
                    auto it_cds = feat_it;

                    ++feat_it;
                    while (feat_it != ftbl.end() &&
                        !((*feat_it)->IsSetData() && (*feat_it)->GetData().IsCdregion())) {
                        ++feat_it;
                    }
                    auto next_cds = feat_it;
                    if (next_cds != ftbl.end()) {
                        bool has_pid = !((*it_cds)->GetNamedQual("protein_id").empty());
                        bool next_has_pid = !((*next_cds)->GetNamedQual("protein_id").empty());
                        if (!has_pid != !next_has_pid) {
                            msg = "Some coding regions have a protein_id qualifier and others do not. "
                                "Protein id qualifiers are used to generate protein sequence ids and they should "
                                "be consistently present or absent in the feature table";
                            return msg;
                        }
                    }
                }
                else {
                    ++feat_it;
                }
            }
        }
    }
    return msg;
}

CIRef<IEditCommand> CImportFeatTable::x_CreateCommand( const IObjectLoader::TObjects& objects)
{
    TUniqueFeatTblIDs non_matched_ftbl_ids;
    TFeatSeqIDMap autofix_map;

    CTSE_Handle tseh = m_Seh.GetTSE_Handle();
    ITERATE (IObjectLoader::TObjects, obj_it, objects) {
        const CObject& ptr = obj_it->GetObject();
        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(&ptr);
        if (annot && annot->IsFtable()) {
            // assume that all features within a seq_annot would go to the same sequence
            const CSeq_annot::C_Data::TFtable& ftbl = annot->GetData().GetFtable();
            if (!ftbl.empty()) {
                const CSeq_feat& feat = *ftbl.front();
                const CSeq_id* feat_id = feat.GetLocation().GetId();
                if (feat_id) {
                    CSeq_id_Handle match_id = x_GetFixedId(*feat_id, tseh);
                    if (!match_id) {
                        non_matched_ftbl_ids.insert(CSeq_id_Handle::GetHandle(*feat_id));
                    } else if (!match_id.GetSeqId()->Equals(*feat_id)) {
                        autofix_map[CSeq_id_Handle::GetHandle(*feat_id)] = match_id;
                    }
                }
            }
        }
    }

    TFeatSeqIDMap ftbl_seqid_map;
    if (!non_matched_ftbl_ids.empty()) {
        CMatchFeatureTableDlg dlg(NULL, m_Seh, non_matched_ftbl_ids);
        if (dlg.ShowModal() == wxID_OK) {
            dlg.GetFtableIDToSeqIDMap(ftbl_seqid_map);
        } else {
            return CIRef<IEditCommand>();
        }
    }

    ITERATE(TFeatSeqIDMap, it, autofix_map) {
        ftbl_seqid_map[it->first] = it->second;
    }

    unsigned int startingLocusTagNumber = 1;
    unsigned int startingFeatureId = 1;
    string locus_tag;
    bool euk = false;
    if (m_is_gff)
    {
        ITERATE(IObjectLoader::TObjects, obj_it, objects) 
        {
            const CObject& ptr = obj_it->GetObject();
            const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(&ptr);
            if (annot && annot->IsFtable())
            {
                for ( auto&& feat_it : annot->GetData().GetFtable()) 
                {
                    const CSeq_feat& feat = *feat_it;
                    if (feat.IsSetData() && feat.GetData().IsGene() && feat.GetData().GetGene().IsSetLocus_tag())
                        locus_tag = feat.GetData().GetGene().GetLocus_tag();
                    else if (feat.GetGeneXref() != nullptr && feat.GetGeneXref()->IsSetLocus_tag())
                        locus_tag =  feat.GetGeneXref()->GetLocus_tag();
                    else
                        locus_tag = feat.GetNamedQual("locus_tag");
                    if (!locus_tag.empty())
                        break;
                }
            }
            if (!locus_tag.empty())
                break;
        }

        string prefix, offset;
        if (NStr::SplitInTwo(locus_tag, "_", prefix, offset)) 
        {
            int tail = NStr::StringToNonNegativeInt(offset);
            if (tail != -1) 
            {
                startingLocusTagNumber = tail;
            }
            else 
            {
                if (!offset.empty()) 
                {
                    ERR_POST(Error << "Invalid locus tag: Only one \"_\", and suffix must be numeric");
                    prefix = locus_tag;
                }
            }
        }
        else 
        {
            prefix = locus_tag;
        }

        CLocusTagEukDlg dlg(NULL, prefix);
        if (dlg.ShowModal() == wxID_OK) 
        {
            locus_tag = dlg.GetLocusTag();
            euk = dlg.GetEuk();
            if (locus_tag.empty())
                startingLocusTagNumber = 1;
        } else 
        {
            return CIRef<IEditCommand>();
        }
    }

   
    
    m_Error.clear();
    CRef<CCmdComposite> result;

    ITERATE(IObjectLoader::TObjects, obj_it, objects) {
        const CObject& ptr = obj_it->GetObject();
        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(&ptr);
        //LOG_POST(Info << "Read seq-annot:\n" << MSerial_AsnText << *annot);
        if (annot) {
            CStopWatch sw;
            sw.Start();
            CIRef<IEditCommand> cmd = AddSeqAnnotToSeqEntry(*annot, ftbl_seqid_map, startingLocusTagNumber, startingFeatureId, locus_tag,  euk);
            LOG_POST(Info << "AddSeqAnnotToSeqEntry took " << sw.Elapsed() << " seconds");
            if (cmd) {
                if (!result)
                    result.Reset(new CCmdComposite("Import feature tables"));
                result->AddCommand(*cmd);
            }
        }
    }
    return CIRef<IEditCommand>(result);
}


static void s_RemapFeatureSeqIds(CSeq_feat& feat, const CSeq_id& set_id)
{
    CTypeIterator<CSeq_loc> visitor(feat);

    while (visitor){
        CSeq_loc& loc = *visitor;
        if (feat.IsSetProduct() && (&loc == &feat.GetProduct())) {
            // don't change product ids
        }
        else {
            CRef<CSeq_id> new_id(new CSeq_id());
            new_id->Assign(set_id);
            loc.SetId(*new_id);
        }
        ++visitor;
    }
}

CIRef<IEditCommand> CImportFeatTable::AddSeqAnnotToSeqEntry(
    const CSeq_annot& orig_annot, 
    CImportFeatTable::TFeatSeqIDMap& ftbl_seqid_map,
    unsigned int &startingLocusTagNumber, 
    unsigned int &startingFeatureId, 
    const string &locus_tag, 
    bool euk)
{
    CTSE_Handle tseh = m_Seh.GetTSE_Handle();

    CRef<CCmdComposite> result;
    if (orig_annot.IsFtable()) {
        CRef<CSeq_annot> annot(new CSeq_annot);
        annot->Assign(orig_annot);
        if (m_is_gff)
        {
            xPostProcessAnnot(*annot, startingLocusTagNumber, startingFeatureId, locus_tag,  euk);
        }
        vector<CRef<CSeq_feat>> imported_cds;
        for (auto&& feat_it: annot->GetData().GetFtable()) {
            const CSeq_feat& feat = *feat_it;
            CBioseq_Handle bsh = x_FindLocalBioseq_Handle(feat, tseh);
            
            CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*feat.GetLocation().GetId());
            TFeatSeqIDMap::const_iterator it = ftbl_seqid_map.find(idh);
            // don't add feature tables whose IDs were not found:
            if (!bsh && it == ftbl_seqid_map.end()) {
                if (NStr::IsBlank(m_Error)) {
                    m_Error.assign("Feature table identifiers do not match record\n");
                }
                m_Error += CSeqFeatData::SubtypeValueToName(feat.GetData().GetSubtype());
                m_Error += "->";
                m_Error += feat.GetLocation().GetId()->AsFastaString();
                m_Error += "\n";
                continue;
            } 

            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(feat);
            if (!bsh && it != ftbl_seqid_map.end()) {
                s_RemapFeatureSeqIds(*new_feat, *(it->second.GetSeqId()));
                bsh = m_Seh.GetScope().GetBioseqHandle(it->second.GetSeqId().GetObject());
            }

            _ASSERT(bsh);
            if (!result)
                result.Reset(new CCmdComposite("Create Features"));

            if (new_feat->IsSetData() && new_feat->GetData().IsCdregion()) {
                imported_cds.push_back(new_feat);
            }           
            CIRef<IEditCommand> cmd(new CCmdCreateFeatBioseq(bsh, *new_feat));
            result->AddCommand(*cmd);
        }            

        if (result)    {
            result->Execute();
            CRef<CCmdComposite> cds_cmd(new CCmdComposite("Import CDS"));
            for (auto new_feat : imported_cds)    {
                CIRef<IEditCommand> cmd = x_DoImportCDS(new_feat);
                if (cmd)
                    cds_cmd->AddCommand(*cmd);
            }
            result->Unexecute();
            result->AddCommand(*cds_cmd);
        }
    }
    return CIRef<IEditCommand>(result);
}


void CImportFeatTable::xPostProcessAnnot(CSeq_annot& annot, unsigned int &startingLocusTagNumber, unsigned int &startingFeatureId, const string &locus_tag, bool euk)
//  ----------------------------------------------------------------------------
{  
    edit::CFeatTableEdit fte(annot, locus_tag, startingLocusTagNumber, startingFeatureId);
    fte.InferPartials();
    fte.GenerateMissingParentFeatures(euk);
    fte.GenerateLocusTags();
    fte.GenerateProteinAndTranscriptIds();
    //fte.InstantiateProducts();
    fte.InstantiateProductsNames();
    fte.EliminateBadQualifiers();
    fte.SubmitFixProducts();

    startingLocusTagNumber = fte.PendingLocusTagNumber();
    startingFeatureId = fte.PendingFeatureId();

    CCleanup cleanup;
    CConstRef<CCleanupChange> changed = cleanup.BasicCleanup(annot);
}

static string s_GetIdBase(const CSeq_id& id)
{
    string id_base;
    id.GetLabel(&id_base, CSeq_id::eContent);
    return id_base;
}

static string s_GetIdBase(CBioseq_Handle bsh)
{
    string id_base;
    CSeq_id_Handle hid;

    ITERATE(CBioseq_Handle::TId, it, bsh.GetId()) {
        if (!hid || !it->IsBetter(hid)) {
            hid = *it;
        }
    }

    hid.GetSeqId()->GetLabel(&id_base, CSeq_id::eContent);
    return id_base;
}


CIRef<IEditCommand> CImportFeatTable::x_DoImportCDS(CRef<CSeq_feat> feat)
{
    _ASSERT(feat->GetData().IsCdregion());
    CCdregion& cds = feat->SetData().SetCdregion();
    CBioseq_Handle bsh = m_Seh.GetScope().GetBioseqHandle(feat->GetLocation());
    // set genetic code if it's not set already
    if (!cds.IsSetCode()) {
        CRef<CGenetic_code> code = edit::GetGeneticCodeForBioseq(bsh);
        if (code) {
            cds.SetCode(*code);
        }
    }
    // if frame is not set, it defaults to 1
    if (!cds.IsSetFrame()) {
        cds.SetFrame(CCdregion::eFrame_one);
    }

    CCleanup::ParseCodeBreaks(*feat, m_Seh.GetScope());

    // store all ids from protein_id qualifiers and add it later to the protein sequence
    CBioseq::TId prot_ids;
    CSeq_id::ParseFastaIds(prot_ids, feat->GetNamedQual("protein_id"));
    
    // translate coding region only if its location falls within the sequence
    const CSeq_loc& feat_loc = feat->GetLocation();
    TSeqPos stop = feat_loc.GetStop(eExtreme_Positional);
    if (stop >= bsh.GetBioseqLength()) {
        return CIRef<IEditCommand>();
    }

    if (sequence::IsPseudo(*feat, m_Seh.GetScope())) {
        return CIRef<IEditCommand>();
    }

    CRef<CBioseq> protein(new CBioseq);

    string prot_str;
    try {
        CSeqTranslator::Translate(*feat, bsh.GetScope(), prot_str);
    } catch(const CSeqVectorException&) {}
    
    if (prot_str.empty())
        return CIRef<IEditCommand>();

    if (NStr::EndsWith(prot_str, "*")) {
        prot_str = prot_str.substr(0, prot_str.length() - 1);
    }

    protein->SetInst().ResetExt();
    protein->SetInst().SetRepr(CSeq_inst::eRepr_raw);
    if (NStr::Find(prot_str, "-") == NPOS) {
        protein->SetInst().SetSeq_data().SetIupacaa().Set(prot_str);
    } else {
        protein->SetInst().SetSeq_data().SetNcbieaa().Set(prot_str);
    }
    protein->SetInst().SetLength(TSeqPos(prot_str.length()));     
    protein->SetInst().SetMol(CSeq_inst::eMol_aa);

    if (feat->IsSetProduct()
        && feat->GetProduct().IsWhole()
        && (feat->GetProduct().GetWhole().IsGenbank() || feat->GetProduct().GetWhole().IsGeneral())) {
            // don't change CDS product_id
            for (auto& it : prot_ids) {
                protein->SetId().push_back(it);
            }
    }
    else {
        const string& prot_id_qual = feat->GetNamedQual("protein_id");
        if (!prot_id_qual.empty()) {
            for (auto& it : prot_ids) {
                protein->SetId().push_back(it);
            }
            // don't update existing CDS product id
            if (!feat->IsSetProduct()) {
                CRef<CSeq_id> best_id = FindBestChoice(prot_ids, CSeq_id::BestRank);
                feat->SetProduct().SetWhole().Assign(*best_id);
            }
        }
        else {
            int offset = 0;
            string base;
            if (feat->IsSetProduct()) {
                // get the base name of a protein id as there might be more than one
                // in the seq-entry; here the offset starts from 0
                base = s_GetIdBase(*(feat->GetProduct().GetId()));

                if (feat->GetProduct().IsWhole() && feat->GetProduct().GetWhole().IsLocal()) {
                    string base_na = s_GetIdBase(bsh);
                    string product_id;
                    feat->GetProduct().GetId()->GetLabel(&product_id, CSeq_id::eContent);
                    if (NStr::StartsWith(product_id, base_na + "_", NStr::eCase)) {
                        base = base_na;
                        offset = 1;
                    }
                }
            }
            else {
                // get the base name of a na sequence to generate protein ids for one 
                // nuc-prot set; here the offset starts from 1
                base = s_GetIdBase(bsh);
                offset = 1;
            }

            if (m_OffsetForId.find(base) != m_OffsetForId.end()) {
                offset = m_OffsetForId[base];
                offset++;
            }
            // change CDS product_id
            CRef<CSeq_id> prot_id = edit::GetNewLocalProtId(base, m_Seh.GetScope(), offset);
            m_OffsetForId[base] = offset;
            feat->SetProduct().SetWhole().Assign(*prot_id);

            bool has_protid = false;
            for (auto& it : prot_ids) {
                string base = s_GetIdBase(*it);
                if (m_OffsetForId.find(base) == m_OffsetForId.end()) {
                    protein->SetId().push_back(it);
                    has_protid |= (prot_id->Match(*it));
                }
            }
            if (!has_protid) {
                protein->SetId().push_back(prot_id);
            }
        }
    }

    // remove protein_id qualifiers
    feat->RemoveQualifier("protein_id");

    CRef<CSeq_entry> prot_entry(new CSeq_entry());
    prot_entry->SetSeq().Assign(*protein);
    SetMolinfoForProtein(prot_entry,
        feat->GetLocation().IsPartialStart(eExtreme_Biological),
        feat->GetLocation().IsPartialStop(eExtreme_Biological));
        
        
    CRef<CSeq_feat> prot = AddProteinFeatureToProtein(prot_entry,
        feat_loc.IsPartialStart(eExtreme_Biological),
        feat_loc.IsPartialStop(eExtreme_Biological));
    const CProt_ref* orig_ref = feat->GetProtXref();
    if (orig_ref) {
        CRef<CProt_ref> prot_ref(new CProt_ref());
        prot_ref->Assign(*orig_ref);
        prot->SetData().SetProt(*prot_ref);
    }

    CSeq_entry_Handle add_seh = bsh.GetSeq_entry_Handle();
    return CIRef<IEditCommand>(new CCmdAddSeqEntry(prot_entry, add_seh));
}

CSeq_id_Handle CImportFeatTable::x_GetFixedId(const CSeq_id& id, const CTSE_Handle& tseh) const
{
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);
    CBioseq_Handle bsh = tseh.GetBioseqHandle(idh);
    if (bsh) {
        return idh;
    }

    if (!id.IsLocal()) {
        CSeq_id_Handle none;
        return none;
    }
    // try alternate form of ID
    if (id.GetLocal().IsStr()) {
        CRef<CSeq_id> tmp_id(new CSeq_id());
        try {
            tmp_id->SetLocal().SetId(NStr::StringToNonNegativeInt(id.GetLocal().GetStr()));
            idh = CSeq_id_Handle::GetHandle(*tmp_id);
            bsh = tseh.GetBioseqHandle(idh);
            if (bsh) {
                return idh;
            }
        } catch (const CException&) {
        }
    } else if (id.GetLocal().IsId()) {
        CRef<CSeq_id> tmp_id(new CSeq_id());
        tmp_id->SetLocal().SetStr(NStr::NumericToString(id.GetLocal().GetId()));
        idh = CSeq_id_Handle::GetHandle(*tmp_id);
        bsh = tseh.GetBioseqHandle(idh);
        if (bsh) {
            return idh;
        }
    }
    CSeq_id_Handle none;
    return none;
}


CBioseq_Handle CImportFeatTable::x_FindLocalBioseq_Handle(const CSeq_feat& feat, const CTSE_Handle& tseh) const
{
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*feat.GetLocation().GetId());
    CBioseq_Handle bsh = m_Seh.GetScope().GetBioseqHandleFromTSE(idh, tseh);
    return bsh;
}

END_NCBI_SCOPE
