/*  $Id: windowmasker_tool.cpp 38351 2017-04-27 17:57:28Z katargir $
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
* Authors:  Vladimir Tereshkov
*
*/

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence/windowmasker_tool.hpp>
#include <gui/packages/pkg_sequence/windowmasker_panel.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/submit/Seq_submit.hpp>

#include <gui/widgets/wx/message_box.hpp>

#include <serial/typeinfo.hpp>

#include <objmgr/object_manager.hpp>

#include <gui/objutils/user_type.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>

#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <algo/winmask/seq_masker.hpp>
#include <objects/seqfeat/Imp_feat.hpp>

#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <connect/ncbi_conn_stream.hpp>

#include <gui/widgets/loaders/winmask_files.hpp>

#include <wx/filename.h>
#include <wx/dir.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
/// CWindowMaskerTool
CWindowMaskerTool::CWindowMaskerTool()
:   CAlgoToolManagerBase("Search/Find Repetitive Sequences with WindowMasker",
                         "",
                         "Search/Find Repetitive Sequences with WindowMasker",
                         "Find low-complexity sequences using WindowMasker "
                         "data rearrangements and clarifications",
                         "WINDOWMASKER",
                         "Edit"),
                         m_Panel()
{
}

string CWindowMaskerTool::GetExtensionIdentifier() const
{
    return "windowmasker_tool";
}


string CWindowMaskerTool::GetExtensionLabel() const
{
    return "Windowmasker Tool";
}

void CWindowMaskerTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();
    m_Panel = NULL;
    m_Objects.clear();
}

void CWindowMaskerTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
    m_Objects.clear();
}

void CWindowMaskerTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CWindowMaskerPanel();
        m_Panel->Hide(); // to reduce flicker
        m_Panel->Create(m_ParentWindow);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_Objects);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

wxString s_GetPath(int taxId) {
    if (taxId <= 0)
        return wxString();

    CWinMaskerFileStorage& storage = CWinMaskerFileStorage::GetInstance();
    wxFileName fname(storage.GetPath(), wxEmptyString);
    fname.AppendDir(wxString::Format(wxT("%u"), taxId));

    wxDir dir(fname.GetPath());
    if (dir.IsOpened()) {
        wxString dirName;
        for (
            bool cont = dir.GetFirst(&dirName, wxEmptyString, wxDIR_DIRS);
            cont;
        cont = dir.GetNext(&dirName)
            ) {
            fname.AppendDir(dirName);
            break;
        }
    }

    return fname.GetPath();
}

bool CWindowMaskerTool::x_ValidateParams()
{
    if( !m_Panel ){
        return true;
    }

    m_Params = m_Panel->GetData();

    wxString path = s_GetPath(m_Params.GetTaxId());

    if ( !wxFileName::FileExists(path) ) {
        static const wxChar* sc_Files[] = {
            wxT("wmasker.obinary"),
            wxT("wmasker.oascii"),
            NULL
        };

        for (const wxChar** p = sc_Files;  p  &&  *p;  ++p) {
            wxString filepath = wxFileName(path, *p).GetFullPath();
            if (wxFileName::FileExists(filepath)) {
                path = filepath;
                break;
            }
        }

        if ( !wxFileName::FileExists(path) ) {
            NcbiMessageBoxW( 
                wxT("Failed to find WindowMasker statistics file\nat ") + path,
                eDialog_Ok, eIcon_Stop,
                wxT("WindowMasker Run Tool")
            );

            return false;
        }
    }

    return true;
}

void CWindowMaskerTool::x_SelectCompatibleInputObjects()
{
    TConstScopedObjects objects;
    x_ConvertInputObjects(CSeq_loc::GetTypeInfo(), objects);

    for (auto& o : objects) {
        if (!o.scope) continue;
        const CSeq_loc* seqLoc = dynamic_cast<const CSeq_loc*>(o.object.GetPointerOrNull());
        if (!seqLoc) continue;
        CBioseq_Handle h = o.scope->GetBioseqHandle(*seqLoc->GetId());
        if (!h) continue;
        if (h.IsNa()) m_Objects.push_back(o);
    }
}

CAlgoToolManagerParamsPanel* CWindowMaskerTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CWindowMaskerTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

///////////////////////////////////////////////////////////////////////////////
/// CWindowMaskerJob
class  CWindowMaskerJob  : public CDataLoadingAppJob
{
public:
    CWindowMaskerJob (const CWindowMaskerParams& params);

protected:
    virtual void    x_CreateProjectItems(); // overriding virtual function

    virtual void RequestCancel();

protected:
    CWindowMaskerParams m_Params;

private:
    TProcessHandle m_ProcHandle;
};

CDataLoadingAppJob* CWindowMaskerTool::x_CreateLoadingJob()
{
    if( m_Panel ){
        m_Params = m_Panel->GetData();
    }
    CWindowMaskerJob* job = new CWindowMaskerJob(m_Params);
    return job;
}

static const TProcessHandle kInvalidProcHandle = 
    #if defined(NCBI_OS_UNIX)
    (pid_t)-1
    #elif defined(NCBI_OS_MSWIN)
    INVALID_HANDLE_VALUE
    #else
    -1
    #endif
;

CWindowMaskerJob::CWindowMaskerJob( const CWindowMaskerParams& params )
    : m_Params( params )
    , m_ProcHandle( kInvalidProcHandle )
{
    CFastMutexGuard lock(m_Mutex);

    m_Descr = "Windowmasker Job"; //TODO
}

void CWindowMaskerJob::RequestCancel()
{
    if( m_ProcHandle != kInvalidProcHandle ){
        CProcess( m_ProcHandle ).Kill();
    }

    CDataLoadingAppJob::RequestCancel();
}

struct SMaskResult {
    string name;
    vector< CRef<CSeq_loc> > mask_locs;
};

void CWindowMaskerJob::x_CreateProjectItems()
{
    wxString path = s_GetPath(m_Params.GetTaxId());

    if ( !wxFileName::FileExists(path) ) {
        static const wxChar* sc_Files[] = {
            wxT("wmasker.obinary"),
            wxT("wmasker.oascii"),
            NULL
        };

        for (const wxChar** p = sc_Files;  p  &&  *p;  ++p) {
            wxString filepath = wxFileName(path, *p).GetFullPath();
            if (wxFileName::FileExists(filepath)) {
                path = filepath;
                break;
            }
        }

        if ( !wxFileName::FileExists(path) ) {
            NCBI_THROW(CException, eUnknown, "Failed to find WindowMasker statistics file.");
        }
    }

    vector<SMaskResult> mask_res;


    //vector< CRef<CSeq_loc> > mask_locs;
    if (m_Params.GetStandalone()) {
        wxString prog_name = wxT("windowmasker");
#ifdef NCBI_OS_MSWIN
        prog_name += wxT(".exe");
#endif

        wxString agent_path = CSysPath::GetStdPath();

        wxFileName fname(agent_path, wxEmptyString);
#if !defined( NCBI_OS_DARWIN ) || defined( _DEBUG )
        fname.AppendDir(wxT("bin"));
#endif
        fname.SetFullName(prog_name);

        string agent_path_utf8(fname.GetFullPath().ToUTF8());

        vector<string> args;
        args.push_back("-ustat");
        string lstat_name(path.ToUTF8());
        args.push_back(lstat_name);
        args.push_back("-outfmt");
        args.push_back("seqloc_asn1_text");
        args.push_back("-parse_seqids");
        //args.push_back("-break");

        string agent_args;
        ITERATE(vector<string>, arg, args){
            agent_args += *arg + " ";
        }

        for (const auto& o : m_Params.GetObjects()) {
            try {
                ERR_POST(Trace << "We are about to start program on path '" << agent_path_utf8 << "'");
                ERR_POST(Trace << "with parameters '" << agent_args << "'");

                if (IsCanceled())
                    break;

                const CSeq_loc& loc = dynamic_cast<const CSeq_loc&>(o.object.GetObject());
                CScope& scope = const_cast<CScope&>(o.scope.GetObject());

                string name;
                CLabel::GetLabel(loc, &name, CLabel::eDefault, &scope);
                name += ": WindowMasker Repeat Regions";

                CBioseq_Handle handle = scope.GetBioseqHandle(loc);

                CConn_PipeStream ps(agent_path_utf8, args, CPipe::fStdErr_Share);
                CFastaOstream fastm(ps);
                m_ProcHandle = ps.GetPipe().GetProcessHandle();

                fastm.Write(handle, &loc);
                ps.flush();
                ps.GetPipe().CloseHandle(CPipe::eStdIn);

                if (IsCanceled())
                    break;

                CRef<CSeq_loc> res_loc(new CSeq_loc());
                ps >> MSerial_AsnText >> *res_loc;

                int exitcode;
                EIO_Status status = ps.GetPipe().Close(&exitcode);
                ERR_POST(
                    Info << "Command completed with status "
                    << IO_StatusStr(status) << " and exitcode " << exitcode
                    );
                m_ProcHandle = kInvalidProcHandle;

                SMaskResult rv;

                switch (res_loc->Which()){
                case CSeq_loc::e_Packed_int:

                    rv.name = name;

                    NON_CONST_ITERATE(list< CRef<CSeq_interval> >, int_iter, res_loc->SetPacked_int().Set()){
                        CRef<CSeq_loc> pair_loc;
                        pair_loc.Reset(new CSeq_loc());
                        pair_loc->SetInt(**int_iter);
                        pair_loc->SetInt().SetId().Assign(*loc.GetId());

                        rv.mask_locs.push_back(CSeqUtils::RemapChildToParent(loc, *pair_loc));
                    }
                    mask_res.push_back(rv);

                    break;

                case CSeq_loc::e_Int:

                    rv.name = name;

                    res_loc->SetInt().SetId().Assign(*loc.GetId());

                    rv.mask_locs.push_back(CSeqUtils::RemapChildToParent(loc, *res_loc));
                    mask_res.push_back(rv);
                    break;

                default:
                    continue;
                }
            } catch (const CException& ex) {
                if (!IsCanceled()) ERR_POST("Failed to run windowmasker: " << ex.GetMsg());
            }
        }
    }
    else {
        // Starting tool locally
        string lstat_name(path.ToUTF8());
        CSeqMasker masker(lstat_name,
            0 /*!cmd["window"].IsEmpty() ? cmd["window"].AsInteger() : 0*/,
            1 /* window step */,
            1 /* unit step */,
            0 /* !cmd["t_extend"].IsEmpty() ? cmd["t_extend"].AsInteger() : 0*/,
            0 /*!cmd["t_thres"].IsEmpty() ? cmd["t_thres"].AsInteger() : 0*/,
            0 /*!cmd["t_high"].IsEmpty() ? cmd["t_high"].AsInteger() : 0*/,
            0 /*!cmd["t_low"].IsEmpty() ? cmd["t_low"].AsInteger() : 0*/,
            0 /*!cmd["set_t_high"].IsEmpty() ? cmd["set_t_high"].AsInteger() : 0*/,
            0 /*!cmd["set_t_low"].IsEmpty() ? cmd["set_t_low"].AsInteger() : 0*/,
            false /* merge pass */,
            0 /* merge cutoff */,
            0 /* abs merge cutoff dist */,
            0 /* mean merge cutoff dist */ ,
            1 /* merge unit step */,
            "mean" /* trigger */,
            0 /* tmin count */,
            false /* discontig */,
            0 /* pattern */,
            true /* use ba ?? */
        );

        size_t count_failures = 0;
        for(const auto& o : m_Params.GetObjects()) {
            const CSeq_loc& loc = dynamic_cast<const CSeq_loc&>(o.object.GetObject());
            CScope& scope = const_cast<CScope&>(o.scope.GetObject());

            if (IsCanceled())
                break;

            string name;
            CLabel::GetLabel( loc, &name, CLabel::eDefault, &scope );
            name += ": WindowMasker Repeat Regions";

            CSeqVector vec( loc, scope, CBioseq_Handle::eCoding_Iupac );

            // Do the window masking
            auto_ptr<CSeqMasker::TMaskList> mask_list( masker( vec ) );

            SMaskResult rv;
            rv.name = name;

            // Make sequence locations (remapped)
            CSeq_loc tmp_loc;
            tmp_loc.SetInt().SetId().Assign( *loc.GetId() );
            for(
                CSeqMasker::TMaskList::const_iterator it = mask_list->begin();
                it != mask_list->end(); 
                ++it
            ){
                tmp_loc.SetInt().SetFrom( it->first );
                tmp_loc.SetInt().SetTo( it->second );
                rv.mask_locs.push_back( CSeqUtils::RemapChildToParent( loc, tmp_loc ) );
            }

            if( rv.mask_locs.empty() ){
                ++count_failures;
                continue;
            }
            mask_res.push_back( rv );
        }
    }
            
    int output_type = m_Params.GetOutputType();

    if (output_type == 0 || output_type == 2) {

        NON_CONST_ITERATE(vector<SMaskResult>, mritr, mask_res){

            // Make corresponding features
            CRef<CSeq_annot> annot(new CSeq_annot());

            annot->SetNameDesc(mritr->name);
            annot->SetCreateDate(CTime(CTime::eCurrent));

            NON_CONST_ITERATE(vector<CRef<CSeq_loc> >, locitr, mritr->mask_locs) {
                CRef<CSeq_feat> feat(new CSeq_feat());
                feat->SetData().SetImp().SetKey("repeat_region");
                feat->SetLocation(**locitr);
                annot->SetData().SetFtable().push_back(feat);
            }
            //reply.AddObject(*annot, &project);

            /// now create a Project Item for the data
            CRef<CProjectItem> item(new CProjectItem());
            item->SetItem().SetAnnot(*annot);

            // TODO we need to generate title properly
            item->SetLabel(mritr->name);
            AddProjectItem(*item);
        }
    }


    if (output_type == 1 || output_type == 2) {

        NON_CONST_ITERATE(vector<SMaskResult>, mritr, mask_res){

            // Make one big loc
            CRef<CSeq_loc> big_mask_loc(new CSeq_loc());
            NON_CONST_ITERATE(vector<CRef<CSeq_loc> >, locitr, mritr->mask_locs) {
                big_mask_loc->Add(**locitr);
            }
            //reply.AddObject(*big_mask_loc, &project);
            CRef<CProjectItem> item(new CProjectItem());
            CSeq_id & idref = const_cast<CSeq_id &>(*big_mask_loc->GetId());
            item->SetItem().SetId(idref);

            // TODO we need to generate title properly
            item->SetLabel(mritr->name);
            AddProjectItem(*item);
        }
    }
}

END_NCBI_SCOPE
