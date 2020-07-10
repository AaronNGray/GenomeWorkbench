/*  $Id: bam_ui_data_source.cpp 44887 2020-04-08 17:09:43Z shkeda $
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
 * Authors:  Liangshou Wu
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbiexec.hpp>
#include <corelib/ncbi_process.hpp>
#include <corelib/ncbi_system.hpp>
#include <corelib/ncbifile.hpp>
#include <serial/iterator.hpp>

#include <sra/readers/sra/csraread.hpp>

#include <gui/widgets/loaders/bam_indexing_panel.hpp>
#include <gui/widgets/loaders/bam_coverage_graph_panel.hpp>
#include <gui/widgets/loaders/assembly_sel_panel.hpp>
#include <gui/widgets/loaders/bam_ref_sequences_panel.hpp>

#include <gui/packages/pkg_sequence/bam_ui_data_source.hpp>
#include <gui/packages/pkg_sequence/bam_utils.hpp>
#include <gui/core/app_dialogs.hpp>
#include <gui/core/loading_app_job.hpp>
#include <gui/core/pt_data_source.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/app_job_task.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/gencoll_svc.hpp>
#include <gui/objutils/id_mapper_num_assembly.hpp>
#include <gui/objutils/utils.hpp>

#include <gui/utils/extension_impl.hpp>

#include <wx/menu.h>

#include <objmgr/object_manager.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqres/Seq_graph.hpp>
#include <objects/gbproj/LoaderDescriptor.hpp>

#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/serial.hpp>

#include <sra/data_loaders/bam/bamloader.hpp>
#include <sra/readers/bam/bamgraph.hpp>
#include <sra/readers/bam/bamread.hpp>
#include <sra/readers/ncbi_traces_path.hpp>
#include <objtools/readers/idmapper.hpp>

#include <sra/data_loaders/csra/csraloader.hpp>

#include <util/format_guess.hpp>
#include <corelib/ncbi_system.hpp>
#include <corelib/ncbiexec.hpp>

#include <sstream>

#include <mutex>
#include <util/thread_pool.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char* kLoaderDataType_File = "BAM file";
static const char* kLoaderDataType_SRZ = "BAM SRZ";
static const char* kLoaderDataType_SRA = "SRA ACC";
static const char* kLoaderDataType_CSra_File = "CSRA file";
static const char* kBamLoadOption = "BAM/CSRA files";
static const char* kBamPath = "path";
static const char* kBamFile = "files";
static const char* kCSraFile = "file";
static const char* kBamIndexFile = "index";
static const char* kTargetAssm = "context";
static const char* kSrzAccession = "SRZ";
static const char* kSraAccession = "SRA";
static const int   kLoaderPriority = 90;


///////////////////////////////////////////////////////////////////////////////
/// CBamLoadingJob
class  CBamLoadingJob : public CDataLoadingAppJob
{
public:
    struct SBamFile
    {
        string m_BamFile;
        string m_IndexFile;
        string m_CoverageGraph;
    };

    typedef vector<SBamFile>        TBamFiles;
    typedef set<string>             TCSraFiles;
    typedef set<string>             TSrzAccs;
    typedef set<string>             TSraAccs;
    typedef CBamUtils::TSeqIdsVector  TSeqIdsVector;
    typedef CRef<objects::CLoaderDescriptor>   TLoaderRef;

    CBamLoadingJob();

    void SetSrzAccs(const TSrzAccs& accs) { m_SrzAccs = accs; }
    void SetSraAccs(const TSraAccs& accs) { m_SraAccs = accs; }
    void SetBamFiles(const TBamFiles& files) { m_BamFiles = files; }
    void SetCSraFiles(const TCSraFiles& files) { m_CSraFiles = files; }
    TSeqIdsVector& SetRefSeqs() {return m_RefSeqIds; }
    TSeqIdsVector& SetSeqIdsToLoad() { return m_SeqIdsToLoad; }
    void SetSamtoolsPath(const string& samtools) { m_SamtoolsPath = samtools; }
    void SetGraphBinSize(int size) { m_BinSize = size; }

    const string& GetTargetAssembly() const { return m_TargetAssm; }
    void SetTargetAssembly(const string& assm) { m_TargetAssm = assm; }

protected:
    virtual void x_CreateProjectItems();

private:
    TLoaderRef x_CreateDataLoader(SBamFile& bam_input);

private:
    TSrzAccs        m_SrzAccs;        // SRZ accessions
    TSraAccs        m_SraAccs;        // SRA accessions
    TBamFiles       m_BamFiles;
    TCSraFiles      m_CSraFiles;
    TSeqIdsVector   m_RefSeqIds;
    TSeqIdsVector   m_SeqIdsToLoad;
    string  m_TargetAssm;
    string  m_SamtoolsPath;
    int     m_BinSize;
};


///////////////////////////////////////////////////////////////////////////////
/// CBamDSEvtHandler - wxEvtHandler-derived adapter for Bam data source.

class CBamDSEvtHandler : public wxEvtHandler
{
    DECLARE_EVENT_TABLE();
public:
    CBamDSEvtHandler(IWorkbench* workbench)
        :   m_Workbench(workbench)  {
    }
    void OnLoadBamFile(wxCommandEvent& event)
    {
        if(m_Workbench)    {
            CAppDialogs::COpenDialog(m_Workbench, kBamLoadOption);
        }
    }
protected:
    IWorkbench* m_Workbench;
};

static
CRef<CSeq_id> s_GetRefSeq_id(const string& id_str)
{
    CRef<CSeq_id> seq_id;
    try {
        seq_id = new CSeq_id(id_str);
    } catch ( CException&) {
        // ignored
    }

    if ( !seq_id && id_str.find('|') != NPOS ) {
        try {
            CBioseq::TId ids;
            CSeq_id::ParseIDs(ids, id_str);
            if ( !ids.empty() ) {
                seq_id = *ids.begin();
            }
        } catch ( CException& /*ignored*/ ) {
        }
    }

    if ( !seq_id || (seq_id->IsGi() && seq_id->GetGi() < GI_CONST(1000)) ) {
        seq_id = new CSeq_id(CSeq_id::e_Local, id_str);
    }
    return seq_id;
}

BEGIN_EVENT_TABLE(CBamDSEvtHandler, wxEvtHandler)
    EVT_MENU(eCmdLoadBamFile, CBamDSEvtHandler::OnLoadBamFile)
END_EVENT_TABLE();


///////////////////////////////////////////////////////////////////////////////
/// CBamUIDataSource

static const char* kBAM_DS_Icon = "icon::bam_data_source";

CBamUIDataSource::CBamUIDataSource(CBamUIDataSourceType& type)
:   m_Type(&type),
    m_Descr("BAM", kBAM_DS_Icon),
    m_SrvLocator(NULL),
    m_Open(false)
{
}

CBamUIDataSource::~CBamUIDataSource()
{
}


string CBamUIDataSource::GetExtensionIdentifier() const
{
    return "bam_data_source";
}


string CBamUIDataSource::GetExtensionLabel() const
{
    return "Bam Data Source";
}


void CBamUIDataSource::SetServiceLocator(IServiceLocator* locator)
{
    m_SrvLocator = locator;
}


IExplorerItemCmdContributor::TContribution CBamUIDataSource::GetMenu(wxTreeCtrl&, PT::TItems& items)
{
    IExplorerItemCmdContributor::TContribution contrib;

    /// this is not a good solution, but simple
    IWorkbench* workbench = dynamic_cast<IWorkbench*>(m_SrvLocator);

    if(items.size() != 1  ||  workbench == NULL)   {
        return contrib; // return empty object - nothin to contribute
    }

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    PT::CItem* item = items[0];
    int type = item->GetType();

    if(type == PT::eDataSource)    {
        PT::CDataSource* ds_item = dynamic_cast<PT::CDataSource*>(item);

        if(ds_item) {
            CIRef<IUIDataSource> ds = ds_item->GetData();
            CBamUIDataSource* bam_ds = dynamic_cast<CBamUIDataSource*>(ds.GetPointer());

            if(bam_ds)  {
                wxMenu* menu = new wxMenu;
                menu->Append(wxID_SEPARATOR, wxT("Top Actions"));
                cmd_reg.AppendMenuItem(*menu, eCmdLoadBamFile);

                contrib.first = menu;
                contrib.second = new CBamDSEvtHandler(workbench);
            }
        }
    }
    return contrib;
}


IUIDataSourceType& CBamUIDataSource::GetType() const
{
    return *m_Type;
}


const IUIObject& CBamUIDataSource::GetDescr()
{
    return m_Descr;
}


bool CBamUIDataSource::IsOpen()
{
    return m_Open;
}

//static const char* kCmdExtPoint = "scoped_objects::cmd_contributor";
static const char* kAppExpCmdExtPoint = "project_tree_view::context_menu::item_cmd_contributor";

bool CBamUIDataSource::Open()
{
    if (m_Open) {
        LOG_POST(Error << "CBamUIDataSource::Open(): "
            "attempt to open already open data source");
        return false;
    }

    CStopWatch sw;
    sw.Start();

    /// register itself as menu contributor
    CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
    reg->AddExtension(kAppExpCmdExtPoint, *this);

    m_Open = true;
    return true;
}


bool CBamUIDataSource::Close()
{
    if (m_Open) {
        /// remove itself from menu contribution points
        CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
        reg->RemoveExtension(kAppExpCmdExtPoint, *this);

        m_Open = false;
        return true;
    }

    return false;
}


void CBamUIDataSource::EditProperties()
{
    //TODO
}


IUIToolManager* CBamUIDataSource::GetLoadManager()
{
    // TODO may need to link the manager to this particular datasource
    return new CBamUILoadManager();
}

int CBamUIDataSource::GetDefaultCommand() {
    return eCmdLoadBamFile;
}

wxEvtHandler* CBamUIDataSource::CreateEvtHandler() {


    IWorkbench* workbench = dynamic_cast<IWorkbench*>(m_SrvLocator);

    return new CBamDSEvtHandler( workbench );
}


string CBamUIDataSource::AddDataLoader(const objects::CUser_object& obj)
{
    string loader_name = kEmptyStr;

    // figure out the loader name and data from object
    if (obj.GetType().IsStr()) {
        const string& loader_type = obj.GetType().GetStr();
        if (loader_type == kLoaderDataType_File) {
            string path, file, index, target_assm;
            if (x_GetBamInputs(obj, path, file, index, target_assm)) {
                // make sure the given BAM file and it's index exist
                // if they are local
                if (!NStr::StartsWith(path, "http:", NStr::eNocase) &&
                    !NStr::StartsWith(path, "https:", NStr::eNocase))
                {
                    if (!CFile(path + file).Exists()) {
                        string msg = "Can't find the bam file: " + path + file;
                        NCBI_THROW(CException, eUnknown, msg);
                    }

                    if (!CFile(path + index).Exists()) {
                        string msg = "Can't find the bam index file: " + path + index;
                        NCBI_THROW(CException, eUnknown, msg);
                    }
                }

                CBAMDataLoader::SLoaderParams params;
                params.m_DirPath = path;
                params.m_BamFiles.push_back(CBAMDataLoader::SBamFileName(file, index));

                CRef<objects::CObjectManager> obj_mgr = CObjectManager::GetInstance();
                if ( !target_assm.empty() ) {
                    // assembly used for remapping, so a full assembly is needed
                    CRef<CGC_Assembly> assm = CGencollSvc::GetInstance()->GetGCAssembly(target_assm, true, "Gbench");
                    if (assm) 
                        params.m_IdMapper.reset(CGencollIdMapperAdapter::GetIdMapper(assm));
                }
                CBAMDataLoader::SetSkipEmptyPileupGraphsParamDefault(true);
                CBAMDataLoader::TRegisterLoaderInfo info =
                    CBAMDataLoader::RegisterInObjectManager(*obj_mgr, params,
                    CObjectManager::eNonDefault, kLoaderPriority);
                loader_name = info.GetLoader()->GetName();
            }
        } else if (loader_type == kLoaderDataType_CSra_File) {
            string file;
            if (x_GetCSraInputs(obj, file)) {
                // make sure the given CSRA file exists, if it's local
                if (!NStr::StartsWith(file, "http:", NStr::eNocase) &&
                    !NStr::StartsWith(file, "https:", NStr::eNocase))
                {
                    if (!CFile(file).Exists()) {
                        string msg = "Can't find the CSRA file: " + file;
                        NCBI_THROW(CException, eUnknown, msg);
                    }
                }

                CRef<objects::CObjectManager> obj_mgr = CObjectManager::GetInstance();
                CCSRADataLoader::TRegisterLoaderInfo info =
                    CCSRADataLoader::RegisterInObjectManager
                        (*obj_mgr, "", file,
                         CObjectManager::eNonDefault, kLoaderPriority);
                loader_name = info.GetLoader()->GetName();
            }
        } else if (loader_type == kLoaderDataType_SRZ) {
            string srz;
            if (x_GetSrzAccession(obj, srz)) {
                CRef<objects::CObjectManager> obj_mgr = CObjectManager::GetInstance();
                CBAMDataLoader::SetSkipEmptyPileupGraphsParamDefault(true);
                CBAMDataLoader::TRegisterLoaderInfo info =
                    CBAMDataLoader::RegisterInObjectManager(*obj_mgr, srz,
                    CObjectManager::eNonDefault, kLoaderPriority);
                loader_name = info.GetLoader()->GetName();
            }
        } else if (loader_type == kLoaderDataType_SRA) {
            string sra;
            if (x_GetSraAccession(obj, sra)) {
                CRef<objects::CObjectManager> obj_mgr = CObjectManager::GetInstance();
                objects::CCSRADataLoader::TRegisterLoaderInfo info;
                info = objects::CCSRADataLoader::RegisterInObjectManager(*obj_mgr, sra,
                    objects::CObjectManager::eNonDefault, kLoaderPriority);
                loader_name = info.GetLoader()->GetName();
            }
        }
    }

    return loader_name;
}


string CBamUIDataSource::GetLoaderName(const objects::CUser_object& obj) const
{
    string loader_name;

    if (obj.GetType().IsStr()) {
        const string& loader_type = obj.GetType().GetStr();
        if (loader_type == kLoaderDataType_File) {
            string path, file, index, target_assm;
            if (x_GetBamInputs(obj, path, file, index, target_assm)) {
                vector<CBAMDataLoader::SBamFileName> bam_files;
                bam_files.push_back(CBAMDataLoader::SBamFileName(file, index));
                loader_name =
                    CBAMDataLoader::GetLoaderNameFromArgs(path, bam_files);
            }
        } else if (loader_type == kLoaderDataType_CSra_File) {
            string file;
            if (x_GetCSraInputs(obj, file)) {
                loader_name =
                    CCSRADataLoader::GetLoaderNameFromArgs("", file);
            }
        } else if (loader_type == kLoaderDataType_SRZ) {
            string srz;
            if (x_GetSrzAccession(obj, srz)) {
                loader_name = CBAMDataLoader::GetLoaderNameFromArgs(srz);
            }
        } else if (loader_type == kLoaderDataType_SRA) {
            string sra;
            if (x_GetSraAccession(obj, sra)) {
                loader_name = CCSRADataLoader::GetLoaderNameFromArgs(sra);
            }
        }
    }

    return loader_name;
}


bool CBamUIDataSource::x_GetBamInputs(const objects::CUser_object& obj,
                                      string& path, string& file,
                                      string& idx, string& target_assm) const
{
    CConstRef<CUser_field> field_path = obj.GetFieldRef(kBamPath);
    CConstRef<CUser_field> field_file = obj.GetFieldRef(kBamFile);
    CConstRef<CUser_field> field_index = obj.GetFieldRef(kBamIndexFile);
    CConstRef<CUser_field> field_target_assm = obj.GetFieldRef(kTargetAssm);

    if (field_path  &&  field_file  &&  field_index  &&  field_target_assm) {
        path = field_path->GetData().GetStr();
        file = field_file->GetData().GetStr();
        idx = field_index->GetData().GetStr();
        target_assm = field_target_assm->GetData().GetStr();

        return true;
    }

    return false;
}


bool CBamUIDataSource::x_GetSrzAccession(const objects::CUser_object& obj,
                                         string& srz) const
{
    CConstRef<CUser_field> field_srz = obj.GetFieldRef(kSrzAccession);
    if (field_srz) {
        srz = field_srz->GetData().GetStr();
        return true;
    }
    return false;
}


bool CBamUIDataSource::x_GetCSraInputs(const objects::CUser_object& obj,
                                       string& file) const
{
    CConstRef<CUser_field> field_file = obj.GetFieldRef(kCSraFile);

    if (field_file) {
        file = field_file->GetData().GetStr();

        return true;
    }

    return false;
}


bool CBamUIDataSource::x_GetSraAccession(const objects::CUser_object& obj,
                                         string& sra) const
{
    CConstRef<CUser_field> field_sra = obj.GetFieldRef(kSraAccession);
    if (field_sra) {
        sra = field_sra->GetData().GetStr();
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
/// CBamUIDataSourceType
CBamUIDataSourceType::CBamUIDataSourceType()
    :   m_Descr("Bam File data Loader", "")
{
   wxFileArtProvider* provider = GetDefaultFileArtProvider();
   provider->RegisterFileAlias(ToWxString(kBAM_DS_Icon),
                               wxT("bam_data_source.png"));
}


const IUIObject& CBamUIDataSourceType::GetDescr()
{
    return m_Descr;
}


IUIDataSource* CBamUIDataSourceType::CreateDataSource()
{
    return new CBamUIDataSource(*this);
}


bool CBamUIDataSourceType::AutoCreateDefaultDataSource()
{
    return true; // we want to create default "Bam" datasource
}


string CBamUIDataSourceType::GetExtensionIdentifier() const
{
    static string ext_id("bam_data_source_type");
    return ext_id;
}


string CBamUIDataSourceType::GetExtensionLabel() const
{
    return m_Descr.GetLabel();
}



///////////////////////////////////////////////////////////////////////////////
/// CBamUILoadManager
CBamUILoadManager::CBamUILoadManager()
:   m_SrvLocator(NULL),
    m_ParentWindow(NULL),
    m_Descriptor(kBamLoadOption, ""),
    m_State(eInvalid),
    m_OptionPanel(NULL),
    m_IndexingPanel(NULL),
    m_RefSequencesPanel(NULL),
    m_GraphPanel(NULL),
    m_ProjectSelPanel(NULL)
{
    m_ProjectParams.m_EnableDecideLater = false;
    m_Descriptor.SetLogEvent("loaders");
}


void CBamUILoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CBamUILoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CBamUILoadManager::GetDescriptor() const
{
    return m_Descriptor;
}


void CBamUILoadManager::InitUI()
{
    m_State = eSelectBam;
}


void CBamUILoadManager::CleanUI()
{
    m_State = eInvalid;
    if(m_OptionPanel)   {
        m_SavedInput = m_OptionPanel->GetInput();
        m_OptionPanel = NULL; // window is destroyed by its parent
    }
    if (m_IndexingPanel) {
        m_SamtoolsPath = m_IndexingPanel->GetSamtoolsPath();
        m_IndexingPanel = NULL; // window is destroyed by its parent
    }
    m_GraphPanel = NULL;        // window is destroyed by its parent
    m_ProjectSelPanel = NULL;   // window is destroyed by its parent
    m_RefSequencesPanel = NULL; // window is destroyed by its parent
}


wxPanel* CBamUILoadManager::GetCurrentPanel()
{
    switch (m_State) {
    case eSelectBam :
        if (m_OptionPanel == NULL) {
            m_OptionPanel = new CBamLoadOptionPanel(m_ParentWindow);
            m_OptionPanel->SetInput(m_SavedInput);
        }
        return m_OptionPanel;
    case eBamIndex:
        if (m_IndexingPanel == NULL) {
            m_IndexingPanel = new CBamIndexingPanel(m_ParentWindow);
            m_IndexingPanel->SetSamtoolsPath(m_SamtoolsPath);
        }
        m_IndexingPanel->SetBamFilePath(m_OptionPanel->GetBamFilePath());
        return m_IndexingPanel;
    case eCoverageGraph:
        if (m_GraphPanel == NULL) {
            m_GraphPanel = new CBamCoverageGraphPanel(m_ParentWindow);
            if (!m_RegPath.empty()) {
                m_GraphPanel->SetRegistryPath(m_RegPath + ".CoverageGraphPanel");
                m_GraphPanel->LoadSettings();
            }
        }
        m_GraphPanel->TransferDataToWindow();
        m_GraphPanel->SetBamFiles(m_OptionPanel->GetBamFiles());
        return m_GraphPanel;
    case eRefSequences:
        {
            if (m_RefSequencesPanel == NULL) {
                m_RefSequencesPanel = new CBamRefSequencesPanel(m_ParentWindow);
            }
            m_RefSequencesPanel->SetReferenceSeqIds(m_RefSeqIds);
            return m_RefSequencesPanel;
        }
    case eSelectProject:
        if(m_ProjectSelPanel == NULL)   {
            CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
            m_ProjectSelPanel = new CProjectSelectorPanel(m_ParentWindow);
            m_ProjectSelPanel->SetProjectService(srv);
            m_ProjectSelPanel->SetParams(m_ProjectParams);
            m_ProjectSelPanel->TransferDataToWindow();
        }
        return m_ProjectSelPanel;
    default:
        return NULL;
    }
    return NULL;
}


bool CBamUILoadManager::CanDo(EAction action)
{
    switch(m_State) {
    case eSelectBam:
        return action == eNext;
    case eBamIndex:
    case eCoverageGraph:
    case eRefSequences:
    case eSelectProject:
        return action == eBack  ||  action == eNext;
    case eCompleted:
        return false; // nothing left to do
    default:
        _ASSERT(false);
        return false;
    }
}


bool CBamUILoadManager::IsFinalState()
{
    return m_State == eSelectProject;
}


bool CBamUILoadManager::IsCompletedState()
{
    return m_State == eCompleted;
}


bool CBamUILoadManager::DoTransition(EAction action)
{
    if (m_State == eSelectBam  &&  action == eNext) {
        if (m_OptionPanel->IsInputValid()) {
            if (m_OptionPanel->HasBamFile()) {
                if (m_OptionPanel->HasIndexFile()) {
                    m_State = eCoverageGraph;
                } else {
                    m_State = eBamIndex;
                }
                return true;
            } else {
                m_RefSeqIds.clear();
                x_GetCSraRefSeqs(m_OptionPanel->GetSraAccs());
                x_GetSrzRefSeqs(m_OptionPanel->GetSrzAccs());
                x_GetCSraRefSeqs(m_OptionPanel->GetCSraFiles());
                m_State = eRefSequences;
                return true;
            }
        }
        return false;
    } else if (m_State == eBamIndex) {
        if (action == eBack) {
            m_State = eSelectBam;
            return true;
        } else if (action == eNext) {
            if (m_IndexingPanel->IsInputValid()) {
                m_State = eCoverageGraph;
                return true;
            }
            return false;
        }
    } else if (m_State == eCoverageGraph) {
        if (action == eBack) {
            if (m_OptionPanel->HasIndexFile()) {
                m_State = eSelectBam;
            } else {
                m_State = eBamIndex;
            }
            return true;
        } else if (action == eNext) {
            if (m_GraphPanel->IsInputValid()) {
                const CMapAssemblyParams& map_params = m_GraphPanel->m_AssemblyPanel->GetData();
                if (map_params.GetUseMapping())
                    m_AssmAcc = map_params.GetAssemblyAcc();
                else
                    m_AssmAcc.clear();
                if (m_OptionPanel->HasBamFile()) {
                    m_RefSeqIds.clear();
                    wxBusyCursor wait;
                    x_GetBamRefSeqs(m_OptionPanel->GetBamFiles(), m_AssmAcc);
                }

                m_State = eRefSequences;
                return true;
            }
            return false;
        }
    } else if (m_State == eRefSequences) {
        if (action == eBack) {
            if (m_OptionPanel->HasBamFile()) {
                m_State = eCoverageGraph;
            } else {
                m_State = eSelectBam;
            }
            return true;
        } else if (action == eNext) {


            m_State = eSelectProject;
            return true;
        }
    } else if (m_State == eSelectProject) {
        if (action == eBack) {
            m_State = eRefSequences;
            return true;
        } else if (action == eNext) {
            if(m_ProjectSelPanel->TransferDataFromWindow()) {
                m_State = eCompleted;
                return true;
            }
            return false;
        }
    }
    _ASSERT(false);
    return false;
}


IAppTask* CBamUILoadManager::GetTask()
{
    // create loading Job
    m_ProjectSelPanel->GetParams(m_ProjectParams);

    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    CBamLoadingJob* job = new CBamLoadingJob();
    // create a wrapper task

    CSelectProjectOptions options;
    m_ProjectParams.ToLoadingOptions(options);

    auto_ptr<CDataLoadingAppTask> task(new CDataLoadingAppTask(srv, options, *job));
    task->SetReportErrors(false); // we handle errors ourselves

    CBamLoadingJob::TBamFiles bam_files;
    ITERATE (CBamLoadOptionPanel::TBamFiles, iter, m_OptionPanel->GetBamFiles()) {
        CBamLoadingJob::SBamFile bam_input;
        bam_input.m_BamFile = *iter;

        string path, file, ext;
        CFile::SplitPath(*iter, &path, &file, &ext);

        if (CFile(path + file + ext + ".bai").Exists()) {
            bam_input.m_IndexFile = file + ext + ".bai";
        }
        else if (CFile(path + file + ".bai").Exists()) {
            bam_input.m_IndexFile = file + ".bai";
        }

        bam_files.push_back(bam_input);
    }

    job->SetSrzAccs(m_OptionPanel->GetSrzAccs());
    job->SetSraAccs(m_OptionPanel->GetSraAccs());
    job->SetBamFiles(bam_files);
    job->SetCSraFiles(m_OptionPanel->GetCSraFiles());
    job->SetRefSeqs() = m_RefSeqIds;

    if (m_RefSequencesPanel)
        m_RefSequencesPanel->GetSelectedReferenceSeqIds(job->SetSeqIdsToLoad());
    if (m_IndexingPanel) {
        job->SetSamtoolsPath(m_IndexingPanel->GetSamtoolsPath());
    }

    job->SetTargetAssembly(m_AssmAcc);

    return task.release();
}


void CBamUILoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


static const char* kBamInputTag = "BamInput2";
static const char* kBamInputListTag = "BamInputList";
static const char* kSamtoolsPathTag = "SamtoolsPath";
static const char* kProjectParamsTag = "ProjectParams";


void CBamUILoadManager::SaveSettings() const
{
    if ( !m_RegPath.empty() ) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        /// remember the selected Format (only if m_OptionPanel exists)
        if (m_OptionPanel) {
            m_SavedInput = m_OptionPanel->GetInput();
        }
        // do not save large texts
        if (m_SavedInput.size() < 10000) {
            list<string> input_list;
            NStr::Split(m_SavedInput, "\n", input_list);
            view.Set(kBamInputListTag, input_list);
        }

        if (m_IndexingPanel) {
            m_SamtoolsPath = m_IndexingPanel->GetSamtoolsPath();
        }

        if ( !m_SamtoolsPath.empty() ) {
            view.Set(kSamtoolsPathTag, m_SamtoolsPath);
        }
        /// save Project Panel settings
        m_ProjectParams.SaveSettings(view, kProjectParamsTag);

        if (m_GraphPanel)
            m_GraphPanel->SaveSettings();
    }
}


void CBamUILoadManager::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
        list<string> input_list;
        view.GetStringList(kBamInputListTag, input_list);
        if (!input_list.empty()) {
            m_SavedInput = NStr::Join(input_list, "\n");
        } else {
            m_SavedInput = view.GetString(kBamInputTag, "");
        }

        m_SamtoolsPath = view.GetString(kSamtoolsPathTag, kEmptyStr);

        /// load Project Panel settings
        m_ProjectParams.LoadSettings(view, kProjectParamsTag);

        if (m_GraphPanel)
            m_GraphPanel->LoadSettings();
    }
}


void CBamUILoadManager::x_GetBamRefSeqs(const CBamLoadOptionPanel::TBamFiles &files, const string &mappedAcc)
{
    unique_ptr<IIdMapper> idMapper;
    if (!mappedAcc.empty()) {
        CRef<CGC_Assembly> assm = CGencollSvc::GetInstance()->GetGCAssembly(mappedAcc, true, "Gbench");
        if (assm)
            idMapper.reset(CGencollIdMapperAdapter::GetIdMapper(assm));
    }
    set<string> added_ids;
    CBamMgr mgr;
    ITERATE(CBamLoadOptionPanel::TBamFiles, itFile, files) {
        CBamDb db(mgr, *itFile);
        if (idMapper.get())
            db.SetIdMapper(idMapper.get(), eNoOwnership);
        // Get the list of referenced sequences
        for ( CBamRefSeqIterator itRefSeq(db); itRefSeq; ++itRefSeq ) {
            CRef< CSeq_id > seqId = db.GetRefSeq_id(itRefSeq.GetRefSeqId());
            if (seqId.Empty())
                continue;
            auto id_str = seqId->GetSeqIdString(true);
            // if we have multiple bams in the input they might map to the same id
            // in this case we don't need to add this id to m_RefSeqIds twice
            if (added_ids.insert(id_str).second == true) {
                m_RefSeqIds.push_back(seqId);
            }
        }
    }
}

void CBamUILoadManager::x_GetCSraRefSeqs(const CBamLoadOptionPanel::TBamFiles &accessions)
{
    CVDBMgr mgr;
    ITERATE(CBamLoadOptionPanel::TBamFiles, itAcc, accessions) {
        CCSraDb csra_db(mgr, *itAcc);
        for ( CCSraRefSeqIterator it(csra_db); it; ++it ) {
            CRef<CSeq_id> seqId = s_GetRefSeq_id(it.GetRefSeqId());
            if (seqId.Empty())
                continue;
            m_RefSeqIds.push_back(seqId);
        }
    }
}

void CBamUILoadManager::x_GetSrzRefSeqs(const CBamLoadOptionPanel::TBamFiles &accessions)
{
    CSrzPath srz_path;
    ITERATE(CBamLoadOptionPanel::TBamFiles, itAcc, accessions) {
        try {
            string def_name =
                CDirEntry::MakePath(srz_path.FindAccPath(*itAcc),
                SRZ_CONFIG_NAME);
            CNcbiIfstream in(def_name.c_str());
            if (!in) {
                continue;
            }
            string line;
            vector<string> tokens;
            while (getline(in, line)) {
                tokens.clear();
                NStr::Split(line, "\t", tokens);
                if (tokens.size() < 4) {
                    break;
                }
                string str_id = tokens[2];
                if (str_id.empty()) {
                    str_id = tokens[1];
                }
                CRef<CSeq_id> seqId;
                try {
                    seqId = new CSeq_id(str_id);
                }
                catch (CException& /*ignored*/) {
                    seqId = new CSeq_id(CSeq_id::e_Local, str_id);
                }
                m_RefSeqIds.push_back(seqId);
            }
        }
        catch (exception&) {
        }
    }
}

/// CBamLoadingJob

CBamLoadingJob::CBamLoadingJob() : m_BinSize(1000)
{
    m_Descr = "Loading BAM file";
}

inline string s_QuotedPath(const string& path)
{
    // We dont need quoting anymore (since SC-13)
//#if defined __WXMSW__
//    return "\"" + path + "\"";
//#else
    return path;
//#endif
}

string s_GetTempSortedBamFile(const string& sorted_file, int file_i)
{
    string i_str = NStr::IntToString(file_i);
    if (i_str.length() < 4) {
        i_str.insert((size_t)0, size_t(4 - i_str.length()), '0');
    }
    return sorted_file + "." + i_str + ".bam";
}

string s_GetTempSortedBamFileNew(const string& sorted_file, int file_i)
{
    string i_str = NStr::IntToString(file_i);
    if (i_str.length() < 4) {
        i_str.insert((size_t)0, size_t(4 - i_str.length()), '0');
    }
    return sorted_file + ".tmp." + i_str + ".bam";
}

string s_GetAnnotName(const CSeq_annot& annot)
{
    if (annot.IsSetDesc()) {
        ITERATE (CSeq_annot::TDesc::Tdata, iter, annot.GetDesc().Get()) {
            if ((*iter)->IsName()) {
                return (*iter)->GetName();
            }
        }
    }

    return "";
}


CRef<CSeq_annot> s_LoadSeqAnnot(const string& file, const string& annot_name)
{
    // Currently, only ASN.1, Binary ASN.1 or XML files are acceptible
    CNcbiIfstream istr(file.c_str(), ios::binary);
    ESerialDataFormat sfmt = eSerial_None;
    CFormatGuess fg;
    CFormatGuess::EFormat f = fg.Format(istr);
    switch (f) {
        case CFormatGuess::eBinaryASN:
            sfmt = eSerial_AsnBinary;
            break;
        case CFormatGuess::eTextASN:
            sfmt = eSerial_AsnText;
            break;
        case CFormatGuess::eXml:
            sfmt = eSerial_Xml;
            break;
        default:
            {{
                string msg("File format could not be determined:\n");
                msg += file;
                LOG_POST(Info << msg);
            }}
            break;
    }
    if (sfmt == eSerial_None) {
        string msg = "The graph file format is not supported!";
        msg += "\nGraph file: " + file;
        NCBI_THROW(CException, eUnknown, msg);
    }

    istr.seekg(0);
    auto_ptr<CObjectIStream> obj_str(CObjectIStream::Open(sfmt, istr));
    CRef<CSeq_annot> annot(new CSeq_annot);
    try {
        // We only consider seq-annot
        *obj_str >> *annot;
    } catch (CException& e) {
        string msg = "Can't deserialize the graph file. A seq-annot is expected. ";
        msg += "Error: " + e.GetMsg();
        msg += "\nGraph file: " + file;
        NCBI_THROW(CException, eUnknown, msg);
    }

    if ( !annot->IsSetData()  ||  !annot->IsGraph()) {
        string msg = "The input graph file doesn't cotain a graph!";
        msg += "\nGraph file: " + file;
        NCBI_THROW(CException, eUnknown, msg);
    }

    if ( s_GetAnnotName(*annot) != annot_name) {
        string msg = "The graph annotatin name (" + s_GetAnnotName(*annot);
        msg += ") needs to match with BAM file: " + annot_name;
        msg += "\nGraph file: " + file;
        NCBI_THROW(CException, eUnknown, msg);
    }

    return annot;
}


void CBamLoadingJob::x_CreateProjectItems()
{
    NON_CONST_ITERATE (TBamFiles, iter, m_BamFiles) {
        const string& bam_file = iter->m_BamFile;
        LOG_POST(Info << "Importing BAM file: " << bam_file);
        TLoaderRef bam_loader = x_CreateDataLoader(*iter);
        if (IsCanceled()) 
            return;
        if (!bam_loader) 
            continue;
        // skip the one that doesn't have an index file
        if (iter->m_IndexFile.empty()) 
            continue;
        AddDataLoader(*bam_loader);
    }

    // add data loader from SRZ accessions
    ITERATE (TSrzAccs, iter, m_SrzAccs) {
        if (IsCanceled()) {
            return;
        }

        LOG_POST(Info << "Importing SRZ accession: " << *iter);

        CRef<CLoaderDescriptor> loader(new CLoaderDescriptor());
        CBamUIDataSourceType data_source_type;

        loader->SetLabel(*iter);
        loader->SetPriority(kLoaderPriority);
        loader->SetLoader_type(data_source_type.GetExtensionIdentifier());

        CRef<CLoaderDescriptor::TData> bam_data(new CLoaderDescriptor::TData);
        bam_data->SetType().SetStr(kLoaderDataType_SRZ);
        bam_data->AddField(kSrzAccession, *iter);

        loader->SetData(*bam_data);
        AddDataLoader(*loader);
    }

    // add data loader from CSRA files
    ITERATE (TCSraFiles, iter, m_CSraFiles) {
        if (IsCanceled()) {
            return;
        }

        LOG_POST(Info << "Importing cSRA file: " << *iter);

        CRef<CLoaderDescriptor> loader(new CLoaderDescriptor());
        CBamUIDataSourceType data_source_type;

        loader->SetLabel(*iter);
        loader->SetPriority(kLoaderPriority);
        loader->SetLoader_type(data_source_type.GetExtensionIdentifier());

        CRef<CLoaderDescriptor::TData> bam_data(new CLoaderDescriptor::TData);
        bam_data->SetType().SetStr(kLoaderDataType_CSra_File);
        bam_data->AddField(kCSraFile, *iter);

        loader->SetData(*bam_data);
        AddDataLoader(*loader);
    }

    // add data loader from SRA accessions
    ITERATE (TSraAccs, iter, m_SraAccs) {
        if (IsCanceled()) {
            return;
        }

        LOG_POST(Info << "Importing SRA accession: " << *iter);

        CRef<CLoaderDescriptor> loader(new CLoaderDescriptor());
        CBamUIDataSourceType data_source_type;

        loader->SetLabel(*iter);
        loader->SetPriority(kLoaderPriority);
        loader->SetLoader_type(data_source_type.GetExtensionIdentifier());

        CRef<CLoaderDescriptor::TData> bam_data(new CLoaderDescriptor::TData);
        bam_data->SetType().SetStr(kLoaderDataType_SRA);
        bam_data->AddField(kSraAccession, *iter);

        loader->SetData(*bam_data);
        AddDataLoader(*loader);
    }

    // add data loader from reference sequences
    for (auto& id : m_SeqIdsToLoad) {
        CRef<CProjectItem>   item(new CProjectItem());
        CRef<CSeq_id> seqId(new CSeq_id());
        seqId->Assign(*id);
        item->SetObject(*seqId);
        CLabel::SetLabelByData(*item, m_Scope.GetPointer());
        // add item to a list
        AddProjectItem(*item);
    }
}


CBamLoadingJob::TLoaderRef
CBamLoadingJob::x_CreateDataLoader(CBamLoadingJob::SBamFile& bam_input)
{
    TLoaderRef bam_loader;
    const string& bam_file = bam_input.m_BamFile;
    if (bam_file.empty()) {
        LOG_POST(Error << "CBamLoadingJob::x_CreateDataLoader(): "
            "Invalid (empty) bam file.");
        return bam_loader;
    }

    // bam file
    string path, file_name, ext;
    CDirEntry::SplitPath(bam_file, &path, &file_name, &ext);

    if (path.empty()  ||  file_name.empty()) {
        return bam_loader;
    }

    if (NStr::StartsWith(path, "http:", NStr::eNocase) ||
        NStr::StartsWith(path, "https:", NStr::eNocase)) {
        bam_input.m_IndexFile = file_name + ext + ".bai";
    }

    if (bam_input.m_IndexFile.empty()) {
        LOG_POST(Info << "Generating Bam index file for " << bam_file);
        x_SetStatusText("Generating Bam index file");

        // need to create an index file
        if (m_SamtoolsPath.empty()) {
            return bam_loader;
        }

        string index_file = bam_file + ".bai";

        CExec::CResult result;
        try {
            result = CExec::SpawnL(CExec::eNoWait,
                m_SamtoolsPath.c_str(), "index",
                s_QuotedPath(bam_file).c_str(), NULL);

            // wait here until it finishes
            CProcess proc(result.GetProcessHandle());
            while(proc.IsAlive()) {
                if (IsCanceled()  &&  proc.Kill()) {
                    // need clean up the possible generated index file?
                    CFile t_file(index_file);
                    if (t_file.Exists()) {
                        t_file.Remove();
                    }
                    return bam_loader;
                }
                proc.Wait(500);
            }
        } catch (exception&) {
            string msg = "Failed to generate the index file for " + bam_file;
            msg += " with the provided Samtools executable: " + m_SamtoolsPath;
            NCBI_THROW(CException, eUnknown, msg);
        }

        // if failed, try sorting the Bam file, and do it again
        if ( !CFile(index_file).Exists() ) {
            LOG_POST(Warning << "Maybe the input Bam file is not sorted. "
                << "Try sorting it and do indexing again ...");

            x_SetStatusText("Sorting Bam file");
            // sorting BAM file
            file_name += ".sorted";
            string sorted_file = path + file_name;

            // wait here until it finishes
            try {
                result = CExec::SpawnL(CExec::eNoWait,
                    m_SamtoolsPath.c_str(), "sort",
                    s_QuotedPath(bam_file).c_str(),
                    s_QuotedPath(sorted_file).c_str(), NULL);
                CProcess sort_proc(result.GetProcessHandle());

                while(sort_proc.IsAlive()) {
                    if (IsCanceled()  &&  sort_proc.Kill()) {
                        // need clean up the possible generated index file?
                        int file_i = 0;
                        string t_file = s_GetTempSortedBamFile(sorted_file, file_i);
                        while (file_i < 10000  &&  CFile(t_file).Exists()) {
                            CFile(t_file).Remove();
                            ++file_i;
                            t_file = s_GetTempSortedBamFile(sorted_file, file_i);
                        }
                        return bam_loader;
                    }
                    sort_proc.Wait(2000);
                }
            } catch (exception&) {
                string msg = "Failed to sort the BAM file: " + bam_file;
                msg += " with the provided Samtools executable: " + m_SamtoolsPath;
                NCBI_THROW(CException, eUnknown, msg);
            }

            // samtools automatically append '.bam' to the given sorted file name.
            // so we need to adjust it
            sorted_file += ".bam";

            // Newer versions of samtools use different input parameters
            // try it on failure
            if (!CFile(sorted_file).Exists()) {
                try {
                    result = CExec::SpawnL(CExec::eNoWait,
                            m_SamtoolsPath.c_str(), "sort",
                            "-o",
                            s_QuotedPath(sorted_file).c_str(),
                            s_QuotedPath(bam_file).c_str(),
                            NULL);

                    CProcess sort_proc(result.GetProcessHandle());

                    while(sort_proc.IsAlive()) {
                        if (IsCanceled()  &&  sort_proc.Kill()) {
                            // need clean up the possible generated index file?
                            int file_i = 0;
                            string t_file = s_GetTempSortedBamFileNew(sorted_file, file_i);
                            while (file_i < 10000  &&  CFile(t_file).Exists()) {
                                CFile(t_file).Remove();
                                ++file_i;
                                t_file = s_GetTempSortedBamFileNew(sorted_file, file_i);
                            }
                            return bam_loader;
                        }
                        sort_proc.Wait(2000);
                    }
                } catch (exception&) {
                    string msg = "Failed to sort the BAM file: " + bam_file;
                    msg += " with the provided Samtools executable: " + m_SamtoolsPath;
                    NCBI_THROW(CException, eUnknown, msg);
                }
            }

            if (CFile(sorted_file).Exists()) {
                x_SetStatusText("Generating Bam index file");

                // use the sorted Bam file instead of the original user-provided one
                index_file = sorted_file + ".bai";
                try {
                    result = CExec::SpawnL(CExec::eNoWait,
                        m_SamtoolsPath.c_str(), "index",
                        s_QuotedPath(sorted_file).c_str(), NULL);

                    // wait here until it finishes
                    CProcess index_proc(result.GetProcessHandle());
                    while(index_proc.IsAlive()) {
                        if (IsCanceled()  &&  index_proc.Kill()) {
                            // need clean up the possible generated index file
                            CFile t_file(index_file);
                            if (t_file.Exists()) {
                                t_file.Remove();
                            }
                            return bam_loader;
                        }
                        index_proc.Wait(500);
                    }
                } catch (exception&) {
                }

                if (CFile(index_file).Exists()) {
                    bam_input.m_BamFile = sorted_file;
                } else {
                    string msg = "Failed to generate index file for " + bam_file;
                    NCBI_THROW(CException, eUnknown, msg);
                }
            } else {
                string msg = "Failed to sort the Bam file: " + bam_file;
                NCBI_THROW(CException, eUnknown, msg);
            }
        }

        bam_input.m_IndexFile = file_name + ext + ".bai";
    }

    x_SetStatusText("Creating Bam data loader");
    file_name += ext;
    bam_loader.Reset(new CLoaderDescriptor());
    CBamUIDataSourceType data_source_type;

    bam_loader->SetLabel(file_name);
    bam_loader->SetPriority(kLoaderPriority);
    bam_loader->SetLoader_type(data_source_type.GetExtensionIdentifier());

    CRef<CLoaderDescriptor::TData> bam_data(new CLoaderDescriptor::TData);
    bam_data->SetType().SetStr(kLoaderDataType_File);

    bam_data->AddField(kBamFile, file_name);
    bam_data->AddField(kBamPath, path);
    bam_data->AddField(kBamIndexFile, bam_input.m_IndexFile);
    bam_data->AddField(kTargetAssm, GetTargetAssembly());

    bam_loader->SetData(*bam_data);
    return bam_loader;
}

END_NCBI_SCOPE
