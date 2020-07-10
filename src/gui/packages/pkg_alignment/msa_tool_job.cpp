/*  $Id: msa_tool_job.cpp 44599 2020-01-30 17:16:29Z filippov $
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
* Authors:  Roman Katargin, Vladislav Evgeniev
*/


#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>
#include <corelib/ncbiexec.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/readers/fasta.hpp>

#include <serial/iterator.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seq/Annotdesc.hpp>

#include <algo/phy_tree/dist_methods.hpp>

#include <gui/objutils/label.hpp>
#include <objects/gbproj/AbstractProjectItem.hpp>

#include <gui/packages/pkg_alignment/msa_tool_job.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <wx/filename.h>
#include <wx/utils.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
TODO:
IdMap is used now only for tree remapping.
It seems more effective to store labels there, not SeqLocs.
It seems the most effective to retrieve this mapping
from CMappingRanges or from CSeq_loc_Mapper. Probably
CRef<CSeq_loc> Map(const CSeq_loc& src_loc) can be used.
*/

typedef map<CSeq_id_Handle, bool> TSeqTypeMap;
typedef CMSAToolJob::TLocPair TLocPair;
typedef CMSAToolJob::TIdMap TIdMap;

struct STreeNodeMapper
{
    TIdMap* idmap;
    STreeNodeMapper()
        : idmap(NULL)
    {
    }

    ETreeTraverseCode operator()(TPhyTreeNode& node, int level)
    {
        _ASSERT(idmap);
        string& label = node.GetValue().SetLabel();
        if (label.empty()) {
            return eTreeTraverse;
        }
        TIdMap::iterator id_iter = idmap->find(label);
        if (id_iter == idmap->end()) {
            return eTreeTraverse;
        }

        TLocPair p = id_iter->second;
        CScope& scope = *p.second;
        const CSeq_id& real_id =
            sequence::GetId(*p.first, &scope);
        label.erase();
        CLabel::GetLabel(real_id, &label, CLabel::eDefault, &scope);
        return eTreeTraverse;
    }
};

static void s_DumpSequences_wRange(
    CNcbiOstream& ostr,
    TConstScopedObjects& locations,
    TSeqTypeMap& seq_types,
    CMappingRanges& ranges,
    map<string, TLocPair>& idmap
    ){
    int dump_count = 0;

    NON_CONST_ITERATE(TConstScopedObjects, iter, locations){
        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(iter->object.GetPointer());
        CScope* scope = iter->scope;

        string loc_id = "seq-" + NStr::SizetToString(++dump_count);
        idmap["lcl|" + loc_id] = make_pair(CConstRef<CSeq_loc>(loc), CRef<CScope>(scope));

        ostr << ">" << "lcl|" << loc_id << endl;

        CSeqVector vec(*loc, *scope, CBioseq_Handle::eCoding_Iupac);
        string data;
        vec.GetSeqData(0, vec.size(), data);
        ostr << data << endl;

        CBioseq_Handle handle = scope->GetBioseqHandle(*loc->GetId());

        CRef<CSeq_id> seq_id(new CSeq_id(CSeq_id::e_Local, loc_id));
        CSeq_id_Handle local_sihd = CSeq_id_Handle::GetHandle(*seq_id);
        seq_types[local_sihd] = handle.IsNucleotide();

        CSeq_id_Handle base_sihd = CSeq_id_Handle::GetHandle(*loc->GetId());
        seq_types[base_sihd] = handle.IsNucleotide();

        int trcf = handle.IsNucleotide() ? 1 : 3;

        // prepare for final mapping
        ranges.AddConversion(
            local_sihd,
            0 * trcf, sequence::GetLength(*loc, scope) *trcf,
            eNa_strand_plus,
            base_sihd,
            sequence::GetStart(*loc, scope) *trcf,
            objects::eNa_strand_unknown
            );
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CMSAToolJob
CMSAToolJob::CMSAToolJob(const wxString &tool_name)
    : m_PId(0),
    m_ToolName(tool_name)
{

}

bool CMSAToolJob::BeforeRun()
{
    /// we serialize this through a set of temporary files
    /// we serialize this through a set of temporary files
    m_TmpIn = wxFileName::CreateTempFileName(wxT("in"));
    m_TmpOut = wxFileName::CreateTempFileName(wxT("out"));

    bool is_nuc = true;

    /// scoped to make sure we flush and close our file before we start!
    {{
        CNcbiOfstream ostr(m_TmpIn.fn_str());

        GUI_AsyncExec([this, &ostr](ICanceled&) { s_DumpSequences_wRange(ostr, x_GetObjects(), m_SeqTypes, m_Ranges, m_IdMap); },
            wxT("Preparing data for alignment..."));

        bool ors = false, ands = true;
        ITERATE( TSeqTypeMap, iter, m_SeqTypes ){
            ors = ors || iter->second;
            ands = ands && iter->second;
        }

        if( ands != ors ){
            // we have mix, quit
            wxMessageBox(
                m_ToolName + wxT(" tool: All input sequences must be either DNA or protein."),
                wxT("Error"), wxOK | wxICON_ERROR
            );
            return false;
        }

        is_nuc = ors;
    }}
       
    m_CmdLine = x_GetCommandLine(m_TmpIn, m_TmpOut, is_nuc);
    LOG_POST(Info << "Launching " << m_ToolName << " executabe:");
    LOG_POST(Info << m_CmdLine);

    wxString working_dir = x_GetWorkingDirectory();
    if (working_dir.IsEmpty()) {
        m_PId = ::wxExecute(m_CmdLine);
    }
    else {
        wxExecuteEnv env;
        env.cwd = working_dir;
        m_PId = ::wxExecute(m_CmdLine, wxEXEC_ASYNC, nullptr, &env);
    }
    if (m_PId <= 0) {
        string error("Failed to launch ");
        error += m_ToolName;
        error += " executable.";
        NCBI_THROW(CException, eUnknown, error);
    }

    return true;
}

void CMSAToolJob::x_CreateProjectItems()
{
    if (m_PId <= 0) {
        NCBI_THROW(CException, eUnknown, string(m_ToolName + " not launched!"));
    }

    while (true) {
        ::wxSleep(3);

        if (IsCanceled()) {
            ::wxKill(m_PId, wxSIGKILL);
            return;
        }

        ncbi::CProcess proc(m_PId, ncbi::CProcess::ePid);
        if (!proc.IsAlive()) {
            break;
        }
    }

    wxULongLong size = wxFileName::GetSize(m_TmpOut);
    if (size == 0 || size ==  wxInvalidSize) {
        NCBI_THROW(CException, eUnknown, string(m_ToolName + " executabale failed to produce results."));
    }

    string err_msg;
    try {
        CNcbiIfstream istr(m_TmpOut.fn_str());
        CStreamLineReader stream_line(istr);
        CFastaReader fasta_reader(stream_line, CFastaReader::fParseGaps);

        /// -1 = multiple alignment
        CRef<CSeq_entry> entry = fasta_reader.ReadAlignedSet(-1);
        CRef<CSeq_align> align;
        CTypeIterator<CSeq_align> iter(*entry);
        if (iter) {
            align.Reset(&*iter);
            ++iter;
            if (iter) {
                LOG_POST(Error << "CMSAToolJob::x_CreateProjectItems(): more than one alignment!");
            }
        }

        ///
        /// perform any necessary remappings
        ///
        CSeq_loc_Mapper mapper( &m_Ranges );
        ITERATE( TSeqTypeMap, st_itr, m_SeqTypes ){
            mapper.SetSeqTypeById(
                st_itr->first, 
                st_itr->second ? CSeq_loc_Mapper::eSeq_nuc : CSeq_loc_Mapper::eSeq_prot
            );
        }

        //- translate locals  to proper ids 
        //- translate coords to proper offsets
        align = mapper.Map( *align );

        ///
        /// create an annotation to hold our results
        CRef<CSeq_annot> annot(new CSeq_annot());
        annot->SetData().SetAlign().push_back(align);

        string title;
        CLabel::GetLabel(*align, &title, CLabel::eDefault, m_Scope.GetPointer());
        string align_title(m_ToolName + " alignment: ");
        align_title += title;
        annot->SetNameDesc(align_title);
        annot->SetCreateDate(CTime(CTime::eCurrent));

        string comment = "Generated by ";
        comment += m_ToolName;
        comment += " tool with command line : \n";
        comment += m_CmdLine.ToUTF8();

        if (annot) {
            CRef<CProjectItem> item(new CProjectItem());
            item->SetItem().SetAnnot(*annot);
            item->SetLabel(align_title);
            CRef<CAnnotdesc> descr(new CAnnotdesc());
            descr->SetComment(comment);
            item->SetDescr().push_back(descr);
            AddProjectItem(*item);
        }

        if (!m_TmpTreeOut.empty())
            x_AddTreeProjectItem(title, comment);

    }
    catch (CException& e) {
        err_msg = "Failed to generate alignment:\n" + e.GetMsg();
    }
    catch (std::exception& e) {
        err_msg = "Failed to generate alignment:\n" + string(e.what());
    }

    if (!err_msg.empty())
        NCBI_THROW(CException, eUnknown, err_msg);
}

void CMSAToolJob::x_AddTreeProjectItem(const string &title, const string &comment)
{
    if (m_TmpTreeOut.empty())
        return;

    ///
    /// read the tree, if we created one
    ///
    CRef<CBioTreeContainer> btc;

    try {
        CNcbiIfstream tree_istr(m_TmpTreeOut.fn_str());
        auto_ptr<TPhyTreeNode> tree(ReadNewickTree(tree_istr));

        /// we need to convert the labels to the correct seq-id strings
        STreeNodeMapper mapper;
        mapper.idmap = &m_IdMap;
        TreeDepthFirstTraverse(*tree, mapper);

        /// convert to the serializable form
        btc = MakeBioTreeContainer(tree.get());

        /// adding seq-id prop
        const int kLabelId = 0;
        const int kSeqIdId = 2;

        CRef<CFeatureDescr> feat_descr(new CFeatureDescr);
        feat_descr->SetId(kSeqIdId);
        feat_descr->SetName("seq-id");
        btc->SetFdict().Set().push_back(feat_descr);

        NON_CONST_ITERATE(CNodeSet::Tdata, node, btc->SetNodes().Set()) {
            if (!(*node)->CanGetFeatures()) {
                continue;
            }
            NON_CONST_ITERATE(CNodeFeatureSet::Tdata, node_feature,
                (*node)->SetFeatures().Set()) {
                if ((*node_feature)->GetFeatureid() == kLabelId) {
                    CRef<CNodeFeature> id_node_feature(new CNodeFeature);
                    id_node_feature->SetFeatureid(kSeqIdId);
                    id_node_feature->SetValue((*node_feature)->GetValue());
                    (*node)->SetFeatures().Set().push_back(id_node_feature);
                }
            }
        }
    }
    catch (const CException& e) {
        LOG_POST(Error
            << "Failed to read phylogenetic tree:\n"
            << e.GetMsg()
            << "\nPhylogenetic tree output will not be available.");
        btc.Reset();
        m_TmpTreeOut.erase();
    }
    catch (const std::exception& e) {
        LOG_POST(Error
            << "Failed to read phylogenetic tree:\n"
            << e.what()
            << "\nPhylogenetic tree output will not be available.");
        btc.Reset();
        m_TmpTreeOut.erase();
    }

    if (!btc)
        return;

    string tree_title = "Phylogenetic tree: " + title;
    btc->SetTreetype(tree_title);

    CRef<CProjectItem> item(new CProjectItem());
    item->SetItem().SetOther().Set(*btc);
    item->SetLabel(tree_title);
    CRef<CAnnotdesc> descr(new CAnnotdesc());
    descr->SetComment(comment);
    item->SetDescr().push_back(descr);
    AddProjectItem(*item);
}

CMSAToolJob::~CMSAToolJob()
{
    if (!m_TmpIn.empty())
        ::wxRemoveFile(m_TmpIn);
    if (!m_TmpOut.empty())
        ::wxRemoveFile(m_TmpOut);
    if (!m_TmpTreeOut.empty())
        ::wxRemoveFile(m_TmpTreeOut);
}

END_NCBI_SCOPE

