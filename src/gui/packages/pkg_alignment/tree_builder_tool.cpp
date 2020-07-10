/*  $Id: tree_builder_tool.cpp 44757 2020-03-05 18:58:50Z evgeniev $
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
 * Authors:  Roman Katargin, Vladimir Tereshkov
 *
 */

#include <ncbi_pch.hpp>

#include <objects/seqloc/Seq_id.hpp>

#include <gui/packages/pkg_alignment/tree_builder_tool.hpp>
#include <gui/packages/pkg_alignment/tree_builder_panel.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <serial/typeinfo.hpp>

#include <objmgr/object_manager.hpp>

#include <gui/objutils/label.hpp>

#include <algo/align/contig_assembly/contig_assembly.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/util/sequence.hpp>

#include <algo/phy_tree/dist_methods.hpp>

#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/taxon1/taxon1.hpp>
#include <objects/biotree/DistanceMatrix.hpp>

#include <objtools/alnmgr/alnmix.hpp>
#include <objtools/alnmgr/alnvec.hpp>

#include <objtools/alnmgr/aln_explorer.hpp>
#include <objtools/alnmgr/aln_container.hpp>
#include <objtools/alnmgr/aln_converters.hpp>
#include <objtools/alnmgr/aln_builders.hpp>
#include <objtools/alnmgr/sparse_aln.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

static const string sc_JukesCantor("Jukes-Cantor (DNA)");
static const string sc_Kimura("Kimura (protein)");
static const string sc_Poisson("Poisson (protein)");

static const string sc_FastMe("Fast Minimum Evolution");
static const string sc_Nj("Neighbor Joining");

static const string sc_SeqId("Sequence ID");
static const string sc_TaxName("Taxonomic Name (if available)");
static const string sc_SeqTitle("Sequence Title (if available)");

///////////////////////////////////////////////////////////////////////////////
/// CTreeBuilderTool
CTreeBuilderTool::CTreeBuilderTool()
:   CAlgoToolManagerBase("Phylogenetic Tree Builder Tool",
                         "",
                         "Build Phylogenetic Tree from Alignment",
                         "Build Phylogenetic Tree from Alignment",
                         "TREE_BUILDER",
                         "Tree Building"),
    m_Panel()
{
}

string CTreeBuilderTool::GetExtensionIdentifier() const
{
    return "tree_building_tool";
}


string CTreeBuilderTool::GetExtensionLabel() const
{
    return "Phylogenetic Tree Builder Tool";
}

void CTreeBuilderTool::InitUI()
{
    CAlgoToolManagerBase::InitUI();
    m_Panel = NULL;
    m_SeqIds.clear();
}

void CTreeBuilderTool::CleanUI()
{
    m_Panel = NULL;
    CAlgoToolManagerBase::CleanUI();
    m_SeqIds.clear();
}

void CTreeBuilderTool::x_CreateParamsPanelIfNeeded()
{
    if (m_Panel == NULL) {
        x_SelectCompatibleInputObjects();

        m_Panel = new CTreeBuilderPanel();
        m_Panel->Hide(); // to reduce flicker
        m_Panel->Create(m_ParentWindow);
        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_SeqIds);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}

bool CTreeBuilderTool::x_ValidateParams()
{
    return true;
}

/// select only Seq-ids
void CTreeBuilderTool::x_SelectCompatibleInputObjects()
{
    x_ConvertInputObjects(CSeq_align::GetTypeInfo(), m_SeqIds);
}

CAlgoToolManagerParamsPanel* CTreeBuilderTool::x_GetParamsPanel()
{
    return m_Panel;
}

IRegSettings* CTreeBuilderTool::x_GetParamsAsRegSetting()
{
    return &m_Params;
}

///////////////////////////////////////////////////////////////////////////////
/// CTreeBuilderJob
class  CTreeBuilderJob  : public CDataLoadingAppJob
{
public:
    CTreeBuilderJob (const CTreeBuilderParams& params);

protected:
    virtual void  x_CreateProjectItems(); // overriding virtual function

    static void x_Divergence( CSparseAln& spaln, CDistMethods::TMatrix& result );

protected:
    CTreeBuilderParams m_Params;
};

CDataLoadingAppJob* CTreeBuilderTool::x_CreateLoadingJob()
{
    if (m_Panel) m_Params = m_Panel->GetData();
    CTreeBuilderJob* job = new CTreeBuilderJob(m_Params);
    return job;
}

///////////////////////////////////////////////////////////////////////////////
/// CTreeBuilderJob
CTreeBuilderJob::CTreeBuilderJob(const CTreeBuilderParams& params)
:   m_Params(params)
{
    m_Descr = "Tree Builder Job";
}

void CTreeBuilderJob::x_CreateProjectItems()
{
    CDistMethods::TMatrix dmat;
    vector<string> labels;
    vector<string> organisms;
    vector<string> titles;
	vector<string> taxids;

    CRef<CAlnVec> alnvec_ref;
    CRef<CSparseAln> sparse_aln_ref;
    SConstScopedObject seq1 = m_Params.GetObject();
    
    string sDist = ToStdString(m_Params.GetDistanceMethod());
    string sCons = ToStdString(m_Params.GetConstructMethod());
    string sLeaf = ToStdString(m_Params.GetLeafLabels());    

    const CSeq_align& ali = dynamic_cast<const CSeq_align&>(*seq1.object);
    CRef<CScope> scope(seq1.scope);
    if (ali.GetSegs().IsDenseg()) {
        alnvec_ref = new CAlnVec(ali.GetSegs().GetDenseg(), *scope);
        alnvec_ref->SetGapChar('-');
        alnvec_ref->SetEndChar('-');
    } else {

        CAlnContainer container;
        container.insert(ali);

        TIdExtract id_extract;
        TAlnIdMap aln_id_map(id_extract, container.size());
        aln_id_map.push_back(ali);

        TAlnStats aln_stats(aln_id_map);

        CAlnUserOptions aln_user_options;
        TAnchoredAlnVec anchored_aln_vec;
        CreateAnchoredAlnVec(aln_stats, anchored_aln_vec, aln_user_options);

        CAnchoredAln anchored_aln;
        BuildAln(anchored_aln_vec, anchored_aln, aln_user_options);

        if (anchored_aln.GetDim() == 1) {
            anchored_aln = CAnchoredAln();

            aln_user_options.m_MergeAlgo = CAlnUserOptions::ePreserveRows;

            BuildAln(anchored_aln_vec, anchored_aln, aln_user_options);
        }
        sparse_aln_ref = new CSparseAln(anchored_aln, *scope);
        sparse_aln_ref->SetGapChar('-');
    }

    // TODO: use existing distance matrix, if available
    // build distance matrix


    int num_rows = alnvec_ref ? alnvec_ref->GetNumRows() : sparse_aln_ref->GetNumRows();

    // Come up with some labels for the terminal nodes
    labels.resize(num_rows);
    organisms.resize(num_rows);
    titles.resize(num_rows);
	taxids.resize(num_rows);

    for (int i = 0;  i < num_rows;  ++i) {

        /// resolve the org-ref for a given sequence
        /// this may involve fetching the org-ref from the taxonomy server
        CBioseq_Handle bsh = alnvec_ref ? alnvec_ref->GetBioseqHandle(i) : sparse_aln_ref->GetBioseqHandle(i);
        CConstRef<COrg_ref> org_ref;
        try {
            org_ref.Reset(&sequence::GetOrg_ref(bsh));
        }
        catch (std::exception&) {
            try {
                CSeq_id_Handle gi_idh =
                    sequence::GetId(bsh, sequence::eGetId_ForceGi);
                CTaxon1 taxon;
                taxon.Init();
                int tax_id = 0;
                TGi gi = gi_idh.GetGi();
                if (!gi) {
                    CConstRef<CSeq_id> seq_id = bsh.GetNonLocalIdOrNull();
                    if (seq_id && seq_id->IsGi())
                        gi = seq_id->GetGi();
                }
                taxon.GetTaxId4GI(gi, tax_id);
                if (!tax_id) {
                    ERR_POST("CTreeBuilder: No tax_id for gi|" << gi);
                    if (!taxon.IsAlive()) {
                        const string& err = taxon.GetLastError();
                        ERR_POST(err);
                    }
                }

                bool is_species = false;
                bool is_uncultured = false;
                string blast_name;
                org_ref = taxon.GetOrgRef(tax_id, is_species,
                                        is_uncultured, blast_name);
            }
            catch (std::exception&) {
            }
        }
        if (org_ref) {
            organisms[i] = org_ref->GetTaxname();
			taxids[i] = NStr::IntToString(org_ref->GetTaxId());
            if (sLeaf == sc_TaxName) {
                labels[i] = organisms[i];
            }
        }

        //
        // retrieve the title for a sequence
        titles[i] = sequence::CDeflineGenerator().GenerateDefline(bsh);
        if (sLeaf == sc_SeqTitle) {
            labels[i] = titles[i];
        }

        // if sc_SeqId, or if above found no appropriate data
        if (labels[i].empty()) {
            CConstRef<CSeq_id> best_id = sequence::GetId(bsh, sequence::eGetId_Best).GetSeqId();
            CLabel::GetLabel(*best_id, &labels[i], CLabel::eDefault, &*scope);
        }
    }

    // Calculate pairwise distances
    CDistMethods::TMatrix pmat;
    if( alnvec_ref ){
        CDistMethods::Divergence( *alnvec_ref, pmat );
    } else {
        x_Divergence( *sparse_aln_ref, pmat );
    }

    if (!CDistMethods::AllFinite(pmat)) {
        throw runtime_error("The sequence divergence matrix contained "
                            "invalid or infinite values.\n"
                            "This can happen when some pairs of sequences "
                            "share no non-gap positions\n"
                            "in the alignment, leading to fractional "
                            "sequence identities of 0/0.");
    }
    if (sDist == sc_Kimura) {
        CDistMethods::KimuraDist(pmat, dmat);
        if (!CDistMethods::AllFinite(dmat)) {
            throw runtime_error("The calculated distance matrix contained "
                                "invalid or infinite values.\nThis can happen "
                                "with the Kimura method when some "
                                "pairs of sequences are less than about 14.6% "
                                "identical.");
        }
    } else if (sDist == sc_JukesCantor) {
        CDistMethods::JukesCantorDist(pmat, dmat);
        if (!CDistMethods::AllFinite(dmat)) {
            throw runtime_error("The calculated distance matrix contained "
                                "invalid or infinite values.\nThis can happen "
                                "with the Jukes-Cantor method when some "
                                "pairs of sequences are less than 25% "
                                "identical.");
        }
    } else if (sDist == sc_Poisson) {
        CDistMethods::PoissonDist(pmat, dmat);
        if (!CDistMethods::AllFinite(dmat)) {
            throw runtime_error("The calculated distance matrix contained "
                                "invalid or infinite values.");
        }
    } else {
        throw runtime_error(string("Invalid distance calculation type:  ")
                            + sDist);
    }

    // now building a tree based on the distance matrix

     // Use numbers as labels, and sort it out later
    vector<string> numerical_labels(dmat.GetCols());
    for (size_t i = 0;  i < numerical_labels.size();  ++i) {
        numerical_labels[i] = NStr::SizetToString(i);
    }
    
    auto_ptr<TPhyTreeNode> tree;
    if (sCons == sc_FastMe) {
        tree.reset(CDistMethods::FastMeTree(dmat, numerical_labels));
    } else if (sCons == sc_Nj) {
        tree.reset(CDistMethods::NjTree(dmat, numerical_labels));
    } else {
        throw runtime_error(string("Invalid tree reconstruction algorithm:  ")
                            + sCons);
    }

    CRef<CBioTreeContainer> btc = MakeBioTreeContainer(tree.get());
    btc->SetTreetype("Phylogenetic Tree");

    // add attributes to terminal nodes, and replace "label" attributes,
    // which is a number, with the labels we really want
    const int kLabelId = 0;
    const int kSeqIdId = 2;
    const int kOrganismId = 3;
    const int kTitleId = 4;
	const int kTaxId = 5;

    CRef<CFeatureDescr> feat_descr(new CFeatureDescr);
    feat_descr->SetId(kSeqIdId);
    feat_descr->SetName("seq-id");
    btc->SetFdict().Set().push_back(feat_descr);

	feat_descr.Reset(new CFeatureDescr);
    feat_descr->SetId(kOrganismId);
    feat_descr->SetName("organism");
    btc->SetFdict().Set().push_back(feat_descr);
   
	feat_descr.Reset(new CFeatureDescr);
    feat_descr->SetId(kTitleId);
    feat_descr->SetName("seq-title");
    btc->SetFdict().Set().push_back(feat_descr);

    feat_descr.Reset(new CFeatureDescr);
    feat_descr->SetId(kTaxId);
    feat_descr->SetName("tax-id");
    btc->SetFdict().Set().push_back(feat_descr);


    NON_CONST_ITERATE (CNodeSet::Tdata, node, btc->SetNodes().Set()) {
        if ( !(*node)->CanGetFeatures() ) {
            continue;
        }

        NON_CONST_ITERATE (CNodeFeatureSet::Tdata, node_feature,
                            (*node)->SetFeatures().Set()) {
            if ((*node_feature)->GetFeatureid() == kLabelId) {
                // a terminal node
                // figure out which sequence this corresponds to
                // from the numerical id we stuck in as a label
                unsigned int seq_number =
                    NStr::StringToUInt((*node_feature)->GetValue());

                // replace this with real label
                (*node_feature)->SetValue(labels[seq_number]);

                CBioseq_Handle bsh = alnvec_ref ? alnvec_ref->GetBioseqHandle(seq_number) : sparse_aln_ref->GetBioseqHandle(seq_number);
                // add seq-id attribute
                CSeq_id_Handle seq_id = sequence::GetId(bsh, sequence::eGetId_Best);
                CRef<CNodeFeature> id_node_feature(new CNodeFeature);
                id_node_feature->SetFeatureid(kSeqIdId);
                string id_string;
                id_string = seq_id.GetSeqId()->GetSeqIdString(true);
				
                id_node_feature->SetValue(id_string);
                (*node)->SetFeatures().Set().push_back(id_node_feature);

                // add organism attribute if possible
                if (!organisms[seq_number].empty()) {
                    CRef<CNodeFeature>
                        organism_node_feature(new CNodeFeature);
                    organism_node_feature->SetFeatureid(kOrganismId);
                    organism_node_feature->SetValue(organisms[seq_number]);
                    (*node)->SetFeatures().Set()
                        .push_back(organism_node_feature);

					// and... taxid too
					CRef<CNodeFeature>
                        taxid_node_feature(new CNodeFeature);
                    taxid_node_feature->SetFeatureid(kTaxId);
                    taxid_node_feature->SetValue(taxids[seq_number]);
                    (*node)->SetFeatures().Set()
                        .push_back(taxid_node_feature);

                }

                // add seq-title attribute if possible
                if (!titles[seq_number].empty()) {
                    CRef<CNodeFeature>
                        title_node_feature(new CNodeFeature);
                    title_node_feature->SetFeatureid(kTitleId);
                    title_node_feature->SetValue(titles[seq_number]);
                    (*node)->SetFeatures().Set()
                        .push_back(title_node_feature);
                }

                // done with this node
                break;
            }
        }
    }

    // adding biotree
    CRef<CProjectItem> item(new CProjectItem());
    item->SetObject(*btc);
	CLabel::SetLabelByData(*item);
    AddProjectItem(*item);

    // adding distance matrix
    CRef<CProjectItem> item2(new CProjectItem());
    CRef<CDistanceMatrix> dm(new CDistanceMatrix);
    dm->FromMatrix(dmat);
    dm->SetLabels() = labels;
    item2->SetObject(*dm);
	CLabel::SetLabelByData(*item2);
    AddProjectItem(*item2);
}

void CTreeBuilderJob::x_Divergence( CSparseAln& spaln, CDistMethods::TMatrix& result )
{
    // want to change gap char of CAlnVec, but no copy constructor,
    // so effectively copy in a round-about way
    spaln.SetGapChar('-');
    //spaln.SetEndChar('-');

    int nseqs = spaln.GetNumRows();
    result.Resize(nseqs, nseqs);
    vector<string> seq(nseqs);

    for (int i = 0;  i < nseqs;  ++i) {
        spaln.GetAlnSeqString( i, seq[i], spaln.GetAlnRange() );
    }

    for (int i = 0;  i < nseqs;  ++i) {
        result(i, i) = 0;  // 0 difference from itself
        for (int j = i + 1;  j < nseqs;  ++j) {
            result(i, j) = result(j, i) = CDistMethods::Divergence(seq[i], seq[j]);
        }
    }
}


END_NCBI_SCOPE
