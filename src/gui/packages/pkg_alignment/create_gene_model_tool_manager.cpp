/*  $Id: create_gene_model_tool_manager.cpp 42821 2019-04-18 19:32:56Z joukovv $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/packages/pkg_alignment/create_gene_model_tool_manager.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/loaders/job_results_dlg.hpp>
#include <gui/core/document.hpp>
#include <gui/core/prefetch_seq_descr.hpp>

#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_add_seq_annot.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/label.hpp>
#include <objects/gbproj/ProjectItem.hpp>

#include <algo/sequence/align_group.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <algo/sequence/gene_model.hpp>
#include <gui/objutils/label.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objects/misc/sequence_macros.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CCreateGeneModelToolManager::CCreateGeneModelToolManager()
:   CAlgoToolManagerBase("Create Gene Model",
                         "",
                         "Create a gene model from an alignment",
                         "Create a new gene model based on one or more "
                         "alignments of related transcripts to a genome",
                         "CREATE_GENE_MODEL",
                         "Alignment Creation"),
    m_ParamsPanel(NULL)
{
}


string CCreateGeneModelToolManager::GetExtensionIdentifier() const
{
    return "create_gene_model_tool_manager";
}


string CCreateGeneModelToolManager::GetExtensionLabel() const
{
    return "Create Gene Model Tool";
}


void CCreateGeneModelToolManager::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_ParamsPanel = NULL;
}


void CCreateGeneModelToolManager::CleanUI()
{
    m_ParamsPanel = NULL;

    CAlgoToolManagerBase::CleanUI();
}


void CCreateGeneModelToolManager::x_CreateParamsPanelIfNeeded()
{
    if(m_ParamsPanel == NULL)   {
        x_SelectCompatibleInputObjects();

        m_ParamsPanel = new CCreateGeneModelParamsPanel();
        m_ParamsPanel->Hide(); // to reduce flicker
        m_ParamsPanel->Create(m_ParentWindow);
        m_ParamsPanel->SetParams(&m_Params, &m_Alignments);

        m_ParamsPanel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_ParamsPanel->LoadSettings();
    }
}


bool CCreateGeneModelToolManager::x_ValidateParams()
{
    string err;
    if(m_Params.m_Alignments.size() == 0)  {
        err = "Please select at least one set of alignments!";
    }
    if( ! err.empty()) {
        NcbiErrorBox(err, m_Descriptor.GetLabel());
        return false;
    }
    return true;
}


void CCreateGeneModelToolManager::x_SelectCompatibleInputObjects()
{
    m_Params.m_Alignments.clear();
    m_Alignments.clear();
    ITERATE(vector<TConstScopedObjects>, it, m_InputObjects) {
        ITERATE(TConstScopedObjects, it2, *it) {
            const CSeq_align* align = dynamic_cast<const CSeq_align*>(it2->object.GetPointerOrNull());
            if (align) m_Alignments.push_back(*it2);
        }
    }

    if (m_Alignments.empty()) {
        x_ConvertInputObjects(CSeq_align::GetTypeInfo(), m_Alignments);
    }

    x_RemoveDuplicates();
}

void CCreateGeneModelToolManager::x_RemoveDuplicates()
{
    TConstScopedObjects temp;
    for (auto& it: m_Alignments) {
        if (find(temp.begin(), temp.end(), it) == temp.end()) {
            temp.push_back(it);
        }
    }

    m_Alignments.assign(temp.begin(), temp.end());
}


CAlgoToolManagerParamsPanel* CCreateGeneModelToolManager::x_GetParamsPanel()
{
    return m_ParamsPanel;
}


IRegSettings* CCreateGeneModelToolManager::x_GetParamsAsRegSetting()
{
    return &m_Params;
}


CDataLoadingAppJob* CCreateGeneModelToolManager::x_CreateLoadingJob()
{
    return 0;
}

IAppTask* CCreateGeneModelToolManager::GetTask()
{
    if (m_ProjectSelPanel)
        m_ProjectSelPanel->GetParams(m_ProjectParams);
    CSelectProjectOptions options;
    m_ProjectParams.ToLoadingOptions(options);

    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    CRef<CCreateGeneModelTask> task(new CCreateGeneModelTask("Creating gene models", srv, options, m_Params));
    return task.Release();
}

//////////////////////////////////////////////////////////////
///SGeneCdsmRnaFeats

CConstRef<CSeq_id> SGeneCdsmRnaFeats::GetID(CScope* scope) const 
{
    CConstRef<CSeq_id> seq_id;
    if (gene) {
        CSeq_id_Handle gene_idh = sequence::GetIdHandle(gene->GetLocation(), scope);
        if (gene_idh) {
            seq_id = gene_idh.GetSeqId();
        }
    }
    if (!seq_id && cds) {
        CSeq_id_Handle cds_idh = sequence::GetIdHandle(cds->GetLocation(), scope);
        if (cds_idh) {
            seq_id = cds_idh.GetSeqId();
            if (!cds_idh.GetSeqId()->Match(*seq_id)) {
                NCBI_USER_THROW("CDS and Gene Seq_ids are not matching!");
            }
        }
    }
    if (!seq_id && mRNA) {
        CSeq_id_Handle mRNA_idh = sequence::GetIdHandle(mRNA->GetLocation(), scope);
        if (mRNA_idh) {
            seq_id = mRNA_idh.GetSeqId();
            if (!mRNA_idh.GetSeqId()->Match(*seq_id)) {
                NCBI_USER_THROW("mRNA and Gene Seq_ids are not matching!");
            }
        }
    }
    return seq_id;
}

void SGeneCdsmRnaFeats::CrossLinkTriple()
{
    string dbname = CCreateGeneModelTask::sGeneDbName;
    if (gene && mRNA) {
        s_CreateDBXref(*(gene), *(mRNA), dbname);
        s_CreateXRefLink(*(mRNA), *(gene));
        s_CreateXRefLink(*(gene), *(mRNA));
    }
    if (gene && cds) {
        s_CreateDBXref(*(gene), *(cds), dbname);
        s_CreateXRefLink(*(cds), *(gene));
        s_CreateXRefLink(*(gene), *(cds));
    }
    if (cds && mRNA) {
        s_CreateXRefLink(*(cds), *(mRNA));
        s_CreateXRefLink(*(mRNA), *(cds));
    }
}

void SGeneCdsmRnaFeats::s_CreateXRefLink(CSeq_feat& from_feat, CSeq_feat& to_feat)
{
    CRef<CSeqFeatXref> xref(new CSeqFeatXref);
    xref->SetId(to_feat.SetId());
    from_feat.SetXref().push_back(xref);
}

void SGeneCdsmRnaFeats::s_CreateDBXref(const CSeq_feat& from_feat, CSeq_feat& to_feat, const string& dbname)
{
    if (NStr::IsBlank(dbname) || !from_feat.IsSetDbxref())
        return;

    CConstRef<CDbtag> tag = from_feat.GetNamedDbxref(dbname);
    if (tag && tag->IsSetTag()) {
        if (tag->GetTag().IsId()) {
            CObject_id::TId id = tag->GetTag().GetId();
            to_feat.AddDbxref(dbname, id);
        } else if (tag->GetTag().IsStr()) {
            CObject_id::TStr str = tag->GetTag().GetStr();
            to_feat.AddDbxref(dbname, str);
        }
    }
}


///////////////////////////////////////////////////////////////
/// CCreateGeneModelTask

const string CCreateGeneModelTask::sGeneDbName("GeneID");

CCreateGeneModelTask::~CCreateGeneModelTask()
{
}

IAppTask::ETaskState CCreateGeneModelTask::x_Run()
{
    CRef<CCmdComposite> cmd = x_GetCommand();
    if (cmd) {
        if (m_Params.m_MergeResults) {
            
            _ASSERT(m_Service);
            _ASSERT(m_Scope);

            CRef<CGBWorkspace> ws = m_Service->GetGBWorkspace();
            if (!ws) return eFailed; 

            CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*m_Scope));
            if (!doc) return eFailed; 

            ICommandProccessor* cmdProcessor = &doc->GetUndoManager();
            if (!cmdProcessor) return eFailed;

            cmdProcessor->Execute(cmd);
            return eCompleted;

        } else {
            if (!m_Items.empty()) {
                CSelectProjectOptions::TData data;
                data[CRef<CLoaderDescriptor>()] = m_Items;

                if (!m_Options.AddItemsToWorkspace(m_Service, data))
                    return eCanceled;
                CPrefetchSeqDescr::PrefetchSeqDescr(m_Service->GetServiceLocator(), m_Items);
            }
            return eCompleted;
        }
    }

    return eFailed;
}

static string s_RetrieveMessage(const string& msg)
{
    if (NStr::IsBlank(msg))
        return msg;

    string error(msg);
    string msg_gmodel("CreateGeneModelFromAlign()");
    if (NStr::StartsWith(error, msg_gmodel)) {
        error = error.substr(msg_gmodel.length());
        error = NStr::Sanitize(error, NStr::fSS_Reject | NStr::fSS_punct);
    }
    
    if (!NStr::IsBlank(error) 
        && isalpha((unsigned char)error[0]) 
        && islower((unsigned char)error[0])) {
        error[0] = Upcase(error[0]);
    }
    return error;
}

CRef<CCmdComposite> CCreateGeneModelTask::x_GetCommand()
{
    TConstScopedObjects& aligns = m_Params.m_Alignments;
    ///
    /// assure we're all in one scope
    ///
    CRef<CScope> scope;
    {{
        ITERATE( TConstScopedObjects, iter, aligns ){
            if( !scope ){
                scope.Reset( const_cast<CScope*>(&*iter->scope) );
            } else if( scope != &*iter->scope ){
                scope.Reset();
                NCBI_THROW(CException, eUnknown,
                    "All alignments must be within the same project"
                );
            }
        }
    }}

    m_Scope = scope;

    ///
    /// meat goes here
    ///
    CFeatureGenerator::TFeatureGeneratorFlags opts = 0;
    if( m_Params.m_CreateGene ){
        opts |= CFeatureGenerator::fCreateGene;
    }
    if( m_Params.m_CreateMrna ){
        opts |= CFeatureGenerator::fCreateMrna;
    }
    if( m_Params.m_CreateCds ){
        opts |= CFeatureGenerator::fCreateCdregion;
    }
    if( m_Params.m_PropagateNcrnaFeats ){
        opts |= CFeatureGenerator::fPropagateNcrnaFeats;
    }
    if( m_Params.m_TranscribeMrna ){
        opts |= CFeatureGenerator::fForceTranscribeMrna;
    }
    if( m_Params.m_TranslateCds ){
        opts |= CFeatureGenerator::fForceTranslateCds;
    }
    if( m_Params.m_PropagateLocalIds ){
        opts |= CFeatureGenerator::fGenerateLocalIds;
    }

    bool f_group_by_gene_id  = m_Params.m_GroupByGeneId;
    f_group_by_gene_id &= (opts & CGeneModel::fCreateGene) != 0;


    list< CRef<CSeq_annot> > annots;
    CRef<CSeq_entry> entry;

    TGeneCDSmRNAList generatedFeats;
    ITERATE( TConstScopedObjects, iter, aligns ){
        const CSeq_align& align = dynamic_cast<const CSeq_align&>(*iter->object);
        try 
        {
            CRef<CSeq_annot> annot( new CSeq_annot() );
            CRef<CBioseq_set> translated_proteins( new CBioseq_set() );
            CFeatureGenerator FeatureGenerator(*m_Scope);
            FeatureGenerator.SetAllowedUnaligned(CFeatureGenerator::kDefaultAllowedUnaligned);
            FeatureGenerator.SetFlags(opts);
            CConstRef<CSeq_align> clean_align = FeatureGenerator.CleanAlignment(align);
            FeatureGenerator.ConvertAlignToAnnot(*clean_align, *annot, *translated_proteins);

            if( annot->IsSetData() ){
                annots.push_back( annot );
                if( translated_proteins->IsSetSeq_set() ){
                    if( !entry ){
                        entry.Reset( new CSeq_entry() );
                    }
                    CRef<CSeq_entry> proteins( new CSeq_entry() );
                    proteins->SetSet( *translated_proteins );
                    entry->SetSet().SetSeq_set().push_back( proteins );
                }
                x_StoreGeneratedFeats(*annot, generatedFeats);
            }
         } catch( CException& e ){
            LOG_POST( Error << "CreateGeneModel tool failed: " << e.GetMsg() );
            string msg = s_RetrieveMessage(e.GetMsg());
            NcbiErrorBox(msg) ;
            return CRef<CCmdComposite>();
         }
    }

    if (annots.empty())
        return CRef<CCmdComposite>();

    m_GeneModelMap.clear();
    ITERATE(TGeneCDSmRNAList, it, generatedFeats) {
        CConstRef<CSeq_id> id = (*it).GetID(scope);
        CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*id);
        m_GeneModelMap[idh].push_back(*it);
    }
    ///
    /// first, separate by seq-id
    /// we plan to create a single annotation for each placed sequence
    ///
    typedef map<CSeq_id_Handle, list< CRef<CSeq_feat> > > TFeatMap;
    TFeatMap feats;

    CRef<CScope> gb_scope;
    NON_CONST_ITERATE (list< CRef<CSeq_annot> >, iter, annots) {
        NON_CONST_ITERATE (CSeq_annot::TData::TFtable, i, (*iter)->SetData().SetFtable()) {
            CSeq_feat& feat = **i;
            CSeq_id_Handle idh = sequence::GetIdHandle(feat.GetLocation(), scope);
            if (idh) { // check for an empty handle (e.g., when the location contains more than one id)

                do { // convert eligible product to GIs
                    if (m_Params.m_PropagateLocalIds
                        || !feat.CanGetProduct() 
                        || !feat.GetProduct().IsWhole() 
                        || feat.GetProduct().GetWhole().IsGi()) 
                        break;

                    CBioseq_Handle bsh = scope->GetBioseqHandle(feat.GetProduct().GetWhole());
                    if (!bsh)
                        break;
                    if (bsh.GetTSE_Handle().CanBeEdited()) // was edited with OM methods
                        break;
                    // Available in SC-15
                    //if (bsh.GetTSE_Handle().GetDataLoader() == 0) // was added to the scope by AddBioseq etc.
                    //    break;

                    if (!gb_scope) {
                        CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
                        gb_scope.Reset(new CScope(*obj_mgr));
                        gb_scope->AddDefaults();
                    }
                    TGi gi = sequence::GetGiForId(feat.GetProduct().GetWhole(), *gb_scope);
                    if (gi > ZERO_GI) 
                        feat.SetProduct().SetWhole().SetGi(gi);
                } while (false);


                // set all seq-id elements to be represented as GI numbers (CR0001)
                const CSeq_id* seq_id = feat.GetLocation().GetId();
                if (seq_id && !seq_id->IsGi()) {
                    TGi loc_gi = sequence::GetGiForId(*seq_id, *scope);
                    if (loc_gi > ZERO_GI) {
                        CRef<CSeq_id> new_id(new CSeq_id);
                        new_id->SetGi(loc_gi);
                        feat.SetLocation().SetId(*new_id);
                    }
                }
                
                if (feat.GetData().IsCdregion()) {
                    CCdregion& cds = feat.SetData().SetCdregion();
                    // set CDS genetic code id to one by default (CR0022)
                    if (!cds.IsSetCode()) {
                        int id = 1;
                        CRef<CGenetic_code> code(new CGenetic_code);
                        code->SetId(id);
                        cds.SetCode(*code);
                    }
                    // set CDS frame to one by default (CR0019)
                    if (!cds.IsSetFrame()) {
                        cds.SetFrame(CCdregion::eFrame_one);
                    }
                }
                
                feats[idh].push_back(CRef<CSeq_feat>(&feat));
            }
        }
    }
    annots.clear();

    _ASSERT(m_GeneModelMap.size() == feats.size());

    CTime time(CTime::eCurrent);
    if (feats.empty())
        return CRef<CCmdComposite>();

    TGeneModelMap::iterator model_iter = m_GeneModelMap.begin();
    CRef<CCmdComposite> cmd(new CCmdComposite("Create New Annotation from Alignment"));	
    NON_CONST_ITERATE( TFeatMap, iter, feats ){
        list< CRef<CSeq_feat> >::const_iterator feat_it = iter->second.begin();
        if( feat_it == iter->second.end() ){
            continue;
        }

        TSeqRange range = (*feat_it)->GetLocation().GetTotalRange();
        for( ++feat_it;  feat_it != iter->second.end();  ++feat_it ){
            range += (*feat_it)->GetLocation().GetTotalRange();
        }

        CSeq_id_Handle idh = iter->first;
        CRef<CSeq_annot> annot(new CSeq_annot());
        if (!m_Params.m_MergeResults) { 
            string label;
            CSeq_id_Handle idhbest = sequence::GetId( idh, *scope, sequence::eGetId_Best );
            idhbest.GetSeqId()->GetLabel( &label, CSeq_id::eContent );
            label += " (";
            label += NStr::IntToString( range.GetFrom() + 1, NStr::fWithCommas );
            label += "..";
            label += NStr::IntToString( range.GetTo() + 1, NStr::fWithCommas );
            label += ") ";
            label += "Gene Models from Alignments";
            annot->SetNameDesc( "Gene Models from Alignments" );
            annot->SetTitleDesc( label );
            annot->SetCreateDate( time );
        }
        annot->SetData().SetFtable().insert(
            annot->SetData().SetFtable().end(),
            iter->second.begin(), iter->second.end());

        ///
        /// make sure we deal with single gene requirements
        ///
        if( f_group_by_gene_id ){
            x_Group_By_GeneID(*annot, model_iter->first);
        }

        annots.push_back( annot );

        if (m_Params.m_MergeResults) {
            CBioseq_Handle bsh = scope->GetBioseqHandle(idh);
            if (bsh) {
                CSeq_entry_Handle seh = bsh.GetTopLevelEntry();
                    
                if ( m_Params.m_CreateGene ) {
                    CObject_id::TId max_id = s_FindHighestFeatId(seh);
                    x_AssignIDsAndCrossLinkFeats(*annot, idh, max_id);
                } else {
                    CRef<CCmdComposite> upd_genes = x_AssignIDsAndUpdateGenes(*annot, idh, seh);
                    if (upd_genes) {
                        cmd->AddCommand(*upd_genes);
                    }
                }
                cmd->AddCommand(*CRef<CCmdCreateSeq_annot>(new CCmdCreateSeq_annot(seh, annot.GetObject())));	
            }
        }

        ///
        /// add as appropriate
        ///
        if( !m_Params.m_MergeResults ){
            NON_CONST_ITERATE( list< CRef<CSeq_annot> >, iter, annots ){
                /// now create a Project Item for the data
                CObject_id::TId max_id = 0;
                x_AssignIDsAndCrossLinkFeats(**iter, idh, max_id);
                CRef<CProjectItem> item(new CProjectItem());
                item->SetItem().SetAnnot(**iter);

                string name;
                CLabel::GetLabel(**iter, &name, CLabel::eDefault, scope.GetPointer());
                item->SetLabel(name);

                m_Items.push_back(item);
            }

        } else {
            // We do not need any message like "No items created".
        }

        ++model_iter;
    }
    return cmd;
}


CObject_id::TId CCreateGeneModelTask::s_GetGeneID(const CSeq_feat& feat)
{
    CObject_id::TId gene_id = 0;
    CConstRef<CDbtag> tag = feat.GetNamedDbxref(sGeneDbName);
    if( !tag ){
        tag = feat.GetNamedDbxref("LocusLink");
    }
    if( tag ){
        gene_id = tag->GetTag().GetId();
    }
    return gene_id;
}

CObject_id::TId CCreateGeneModelTask::s_FindHighestFeatId(const CSeq_entry_Handle entry)
{
    CObject_id::TId id = 0;
    for (CFeat_CI feat_it(entry); feat_it; ++feat_it) {
        if (feat_it->IsSetId()) {
            const CFeat_id& feat_id = feat_it->GetId();
            if (feat_id.IsLocal() && feat_id.GetLocal().IsId() && feat_id.GetLocal().GetId() > id) {
                id = feat_id.GetLocal().GetId();
            }
        }
    }
    return id;
}

bool CCreateGeneModelTask::s_FeaturesHaveIDs(const CSeq_annot& annot)
{
    FOR_EACH_SEQFEAT_ON_SEQANNOT(feat_it, annot) {
        const CSeq_feat& feat = **feat_it;
        if (feat.IsSetId() && feat.GetId().IsLocal()) {
            return true;
        }
    }
    return false;
}

bool CCreateGeneModelTask::s_FeaturesHaveXrefs(const CSeq_annot& annot)
{
    FOR_EACH_SEQFEAT_ON_SEQANNOT(feat_it, annot) {
        const CSeq_feat& feat = **feat_it;
        if (feat.IsSetXref()) {
            FOR_EACH_SEQFEATXREF_ON_SEQFEAT(it, feat) {
                if ((*it)->IsSetId() && (*it)->GetId().IsLocal())
                    return true;
            }
        }
    }
    return false;
}

void CCreateGeneModelTask::x_StoreGeneratedFeats(const CSeq_annot& annot, TGeneCDSmRNAList& gene_cds_rna) const
{
    SGeneCdsmRnaFeats feats;
    CSeq_annot::C_Data::TFtable::const_iterator it = annot.GetData().GetFtable().begin();
    for ( ; it != annot.GetData().GetFtable().end(); ++it) {
        if ((*it)->IsSetData()) {
            const CSeqFeatData& data = (*it)->GetData();
            if (data.IsGene())
                feats.gene = *it;
            else if (data.IsCdregion()) {
                feats.cds = *it;
            } else if (data.GetSubtype() == CSeqFeatData::eSubtype_mRNA) {
                feats.mRNA = *it;
            }
        }
    }
    gene_cds_rna.push_back(feats);
}

void CCreateGeneModelTask::x_Group_By_GeneID(CSeq_annot& annot, const CSeq_id_Handle& idh)
{
    ///
    /// now, find the gene for each list of features
    /// when we're done, create an annotation for each
    ///
    typedef map<int, list< CRef<CSeq_feat> > > TGeneFeats;
    TGeneFeats feats_by_gene;
    
    ITERATE( CSeq_annot::TData::TFtable, it, annot.GetData().GetFtable() ){
        int gene_id = s_GetGeneID(**it);
        feats_by_gene[gene_id].push_back(*it);
    }

    list< CRef<CSeq_feat> > feats;
    NON_CONST_ITERATE( TGeneFeats, it, feats_by_gene ){
        if( !it->first ){
            /// failed to find gene id - pass through
            feats.insert( feats.end(), it->second.begin(), it->second.end() );

        } else {
            /// scan for gene feature
            CRef<CSeq_feat> gene;
            TSeqRange longest_range;
            list< CRef<CSeq_feat> >::iterator i = it->second.begin();
            while( i != it->second.end() ){
                CSeq_feat& feat = **i;
                if( !feat.GetData().IsGene() ){
                    ++i;
                } else {
                    if( !gene ){
                        gene = *i;
                        longest_range = gene->GetLocation().GetTotalRange();
                    } else {
                        TSeqRange range = (*i)->GetLocation().GetTotalRange();
                        longest_range += range;
                    }
                    i = it->second.erase(i);
                }
            }

            if( gene ){
                gene->SetLocation().SetInt().SetFrom( longest_range.GetFrom() );
                gene->SetLocation().SetInt().SetTo( longest_range.GetTo() );
                feats.push_back( gene );
                x_GroupGenes(idh, gene);
            }

            feats.insert( feats.end(), it->second.begin(), it->second.end() );
        }
    }

    annot.SetData().SetFtable().swap( feats );
}

void CCreateGeneModelTask::x_GroupGenes(const CSeq_id_Handle& idh,  CRef<CSeq_feat> gene)
{
    CObject_id::TId gene_id = s_GetGeneID(*gene);
    if (!gene_id) return;
    
    NON_CONST_ITERATE(TGeneCDSmRNAList, it, m_GeneModelMap[idh]) {
        SGeneCdsmRnaFeats& feats = *it;
        if (feats.gene) {
            CObject_id::TId this_id = s_GetGeneID(*(feats.gene));
            if (this_id == gene_id)
                feats.gene = gene;
        }
    }
}

void CCreateGeneModelTask::x_AssignIDsAndCrossLinkFeats(CSeq_annot& annot, const CSeq_id_Handle& idh, CObject_id::TId max_id)
{
    if (!s_FeaturesHaveIDs(annot) && !s_FeaturesHaveXrefs(annot)) {
        s_AssignFeatureIds(annot, max_id);

        NON_CONST_ITERATE (TGeneCDSmRNAList, it, m_GeneModelMap[idh]) {
            SGeneCdsmRnaFeats& feats = *it;
            feats.CrossLinkTriple();
        }
    }
}

void CCreateGeneModelTask::s_AssignFeatureIds(CSeq_annot& annot, CObject_id::TId& max_id)
{
    EDIT_EACH_SEQFEAT_ON_SEQANNOT(feat_it, annot) {
        CSeq_feat& feat = **feat_it;
        if (!feat.IsSetId()) {
            feat.SetId().SetLocal().SetId(++max_id);
        } else if (!feat.GetId().IsLocal()){
            feat.ResetId();
            feat.SetId().SetLocal().SetId(++max_id);
        }
    }
}

static CConstRef<CSeq_feat> s_GetCDSformRNA(const CSeq_feat& mrna, CScope& scope)
{
    CConstRef<CSeq_feat> cds;
    
    bool has_xref = false;
    if (mrna.IsSetXref()) {
        /// using FeatID from feature cross-references:
        ///if mRNA refers to a CDS by feature ID, use that feature
        CBioseq_Handle bsh = scope.GetBioseqHandle(mrna.GetLocation());
        CTSE_Handle tse = bsh.GetTSE_Handle();
        FOR_EACH_SEQFEATXREF_ON_SEQFEAT (it, mrna) {
            if ((*it)->IsSetId() && (*it)->GetId().IsLocal() && (*it)->GetId().GetLocal().IsId()) {
                CSeq_feat_Handle cdsh = tse.GetFeatureWithId(CSeqFeatData::eSubtype_cdregion, (*it)->GetId().GetLocal().GetId());
                if (cdsh) {
                    cds = cdsh.GetSeq_feat();
                }
                has_xref = true;
            }
        }
    }
    if (!has_xref) {
        /// using location to find CDS: 
        /// mRNA must include the CDS location and the internal interval boundaries need to be identical
        cds = sequence::GetBestOverlappingFeat( mrna.GetLocation(), 
            CSeqFeatData::eSubtype_cdregion, 
            sequence::eOverlap_CheckIntervals, 
            scope);
    }
    return cds;
}

CRef<CCmdComposite> CCreateGeneModelTask::x_AssignIDsAndUpdateGenes(CSeq_annot& annot, const CSeq_id_Handle& idh, CSeq_entry_Handle seh)
{
    if (s_FeaturesHaveIDs(annot) || s_FeaturesHaveXrefs(annot)) {
        return CRef<CCmdComposite>();
    }

    CRef<CCmdComposite> cmd(new CCmdComposite("Replace older version of transcripts and update related genes"));
    if (!m_Params.m_PropagateLocalIds) {
        NON_CONST_ITERATE(TGeneCDSmRNAList, it, m_GeneModelMap[idh]) {
            if (!it->mRNA)
                continue;
            const CSeq_id* it_id = it->mRNA->GetProduct().GetId();
            string new_id = sequence::GetAccessionForId(*it_id, *m_Scope, sequence::eWithAccessionVersion);
            if (NStr::IsBlank(new_id)) {
                NCBI_USER_THROW("Accession for new mRNA transcript_id is not found");
            }

            CSeq_feat_Handle fh;
            for (CFeat_CI mrna_it(seh, SAnnotSelector(CSeqFeatData::eSubtype_mRNA)); mrna_it; ++mrna_it) {
                CConstRef<CSeq_id> prod_id = mrna_it->GetProductId().GetSeqId();
                string old_id = sequence::GetAccessionForId(*prod_id, *m_Scope, sequence::eWithAccessionVersion);
                if (NStr::IsBlank(old_id)) {
                    NCBI_USER_THROW("Accession for mRNA transcript_id is not found");
                }
                SIZE_TYPE pos = NStr::CommonPrefixSize(old_id, new_id);
                if (pos == NStr::FindNoCase(old_id, ".") + 1) {
                    string old_version = old_id.substr(pos, NPOS);
                    string new_version = new_id.substr(pos, NPOS);
                    if (NStr::StringToInt(new_version) > NStr::StringToInt(old_version)) {
                        fh = *mrna_it;
                    }
                    break;
                }
            }

            if (!fh)
                continue;

            // update the new mRNA with old mRNA's feat-id
            if (fh.IsSetId()) {
                CRef<CFeat_id> mrna_id(new CFeat_id);
                mrna_id->Assign(fh.GetId());
                it->mRNA->SetId(*mrna_id);
            }

            // delete the old mRNA
            const CSeq_annot_Handle& annot_handle = fh.GetAnnot();
            CSeq_entry_EditHandle eh = annot_handle.GetParentEntry().GetEditHandle();
            CRef<CCmdDelSeq_feat> del_mrna(new CCmdDelSeq_feat(fh));
            cmd->AddCommand(*del_mrna);

            // update the coding region
            CConstRef<CSeq_feat> cds = s_GetCDSformRNA(*fh.GetOriginalSeq_feat(), *m_Scope);
            if (!cds || !it->cds)
                continue;

            // update the new CDS with old CDS's feat-id
            if (cds->IsSetId()) {
                CRef<CFeat_id> cds_id(new CFeat_id);
                cds_id->Assign(cds->GetId());
                it->cds->SetId(*cds_id);
            }

            // delete the old CDS
            CSeq_feat_Handle cdsh = m_Scope->GetSeq_featHandle(*cds);
            CRef<CCmdDelSeq_feat> del_cds(new CCmdDelSeq_feat(cdsh));
            cmd->AddCommand(*del_cds);

        }
    }

    CObject_id::TId max_id = s_FindHighestFeatId(seh);
    s_AssignFeatureIds(annot, max_id);

    x_GetUpdatedGeneCommand(seh, idh, cmd.GetPointer());
    return cmd;
}

void CCreateGeneModelTask::x_GetUpdatedGeneCommand(CSeq_entry_Handle seh, const CSeq_id_Handle& idh, CCmdComposite* cmd)
{
    typedef map<CConstRef<CSeq_feat>, TGeneCDSmRNAList> TMapGeneFeats;
    TMapGeneFeats gene_feats_map;

    feature::CFeatTree feat_tree;
    SAnnotSelector sel;
    sel.IncludeFeatType(CSeqFeatData::e_Gene);
    sel.IncludeFeatType(CSeqFeatData::e_Rna);
    sel.IncludeFeatType(CSeqFeatData::e_Cdregion);
    CFeat_CI feat_iter(seh, sel);

    feat_tree.AddFeatures(feat_iter);

    ITERATE(TGeneCDSmRNAList, it, m_GeneModelMap[idh]) {
        const SGeneCdsmRnaFeats& feats = *it;
        if (feats.cds) {
            CBioseq_Handle proth = seh.GetScope().GetBioseqHandle(feats.cds->GetProduct());
            const CMappedFeat& mapped_cds = sequence::GetMappedCDSForProduct(proth);
            feat_tree.AddGenesForCds(mapped_cds);

            CSeq_feat_Handle fh = feat_tree.GetBestGene(mapped_cds);
            CConstRef<CSeq_feat> gene_feat;
            if (fh) {
                CSeq_id_Handle found_idh = fh.GetLocationId();
                if (!seh.GetScope().IsSameBioseq(idh, found_idh, CScope::eGetBioseq_All)) {
                    const CSeq_feat& found_feat = *fh.GetOriginalSeq_feat();
                    if (found_feat.GetData().IsGene()) {
                        const CGene_ref& gene_ref = found_feat.GetData().GetGene();
                        if (gene_ref.IsSetLocus()) {
                            for (CFeat_CI feat_it(seh, CSeqFeatData::eSubtype_gene); feat_it; ++feat_it) {
                                if (feat_it->GetData().GetGene().IsSetLocus()
                                    && NStr::EqualCase(feat_it->GetData().GetGene().GetLocus(), gene_ref.GetLocus())) {
                                    gene_feat = feat_it->GetOriginalSeq_feat();
                                    break;
                                }
                            }
                        }
                    }
                }
                else {
                    gene_feat = fh.GetOriginalSeq_feat();
                }
            }
            if (gene_feat) {
                gene_feats_map[gene_feat].push_back(feats);
            }
        }
    }
    
    NON_CONST_ITERATE(TMapGeneFeats, it, gene_feats_map) {
        if (it->first) {
            const CSeq_feat& gene = *(it->first);
            CRef<CSeq_feat> new_gene(new CSeq_feat);
            new_gene->Assign(gene);
            
            bool modified = x_UpdateGeneOnSequence(it->second, new_gene);
            if (modified) {
                CSeq_feat_Handle fh = m_Scope->GetSeq_featHandle(gene);
                CIRef<IEditCommand> chg_feat(new CCmdChangeSeq_feat(fh, *new_gene));
                cmd->AddCommand(*chg_feat);
            }
        }
    }
}

bool CCreateGeneModelTask::x_UpdateGeneOnSequence(TGeneCDSmRNAList& gene_cds_rna, CRef<CSeq_feat> gene)
{
    if (!gene) return false;

    // link related features via feature id Xrefs:
    NON_CONST_ITERATE(TGeneCDSmRNAList, it, gene_cds_rna) {
        SGeneCdsmRnaFeats& feat = *it;
        feat.gene.Reset(&(*gene));
        feat.CrossLinkTriple();
    }
 
    // update the gene range, and set the partialness of the updated gene
    // gene range is updated to the union of all of its child features' location
    CConstRef<CDbtag> gene_tag = gene->GetNamedDbxref(sGeneDbName);
    if (!gene_tag) {
        NCBI_USER_THROW("Could not update gene range, as no GeneID dbxref was found");
    }

    TSeqPos gene_start = gene->GetLocation().GetStart(eExtreme_Positional);
    TSeqPos gene_stop = gene->GetLocation().GetStop(eExtreme_Positional);
    TSeqPos gene_start_upd = gene_start, gene_stop_upd = gene_stop;
    bool partial_start = false, partial_stop = false;
    ITERATE (TGeneCDSmRNAList, it, gene_cds_rna) {
        const SGeneCdsmRnaFeats& feat = *it;
        if (feat.mRNA) {
            _ASSERT(gene->GetLocation().GetStrand() == feat.mRNA->GetLocation().GetStrand());
            _ASSERT(feat.mRNA->GetNamedDbxref(sGeneDbName)->Match(*gene_tag));
            
            TSeqPos mrna_start = feat.mRNA->GetLocation().GetStart(eExtreme_Positional);
            TSeqPos mrna_stop = feat.mRNA->GetLocation().GetStop(eExtreme_Positional);
            if (mrna_start < gene_start_upd) {
                gene_start_upd = mrna_start;
                partial_start = feat.mRNA->GetLocation().IsPartialStart(eExtreme_Positional);
            }
            if (mrna_stop > gene_stop_upd) {
                gene_stop_upd = mrna_stop;
                partial_stop = feat.mRNA->GetLocation().IsPartialStop(eExtreme_Positional);
            }

        } else if (feat.cds) {
            _ASSERT(gene->GetLocation().GetStrand() == feat.cds->GetLocation().GetStrand());
            _ASSERT(feat.cds->GetNamedDbxref(sGeneDbName)->Match(*gene_tag));
            
            TSeqPos cds_start = feat.cds->GetLocation().GetStart(eExtreme_Positional);
            TSeqPos cds_stop = feat.cds->GetLocation().GetStop(eExtreme_Positional);
            if (cds_start < gene_start_upd) {
                gene_start_upd = cds_start;
                partial_start = feat.cds->GetLocation().IsPartialStart(eExtreme_Positional);
            }
            if (cds_stop > gene_stop_upd) {
                gene_stop_upd = cds_stop;
                partial_stop = feat.cds->GetLocation().IsPartialStop(eExtreme_Positional);
            }
        }
    }

    // for one gene end:
    // if gene is complete and its child is partial, that end becomes partial
    // if gene is partial and is child is complete, that end becomes complete 
    bool modified = false;
    if (gene_start_upd < gene_start) {
        gene->SetLocation().SetInt().SetFrom(gene_start_upd);
        modified = true;
        if (gene->GetLocation().IsPartialStart(eExtreme_Positional)) {
            if (partial_start) {
                // do nothing
            } else {
                gene->SetLocation().SetPartialStart(false, eExtreme_Positional);
            }
        
        } else {
            if (partial_start) {
                gene->SetLocation().SetPartialStart(true, eExtreme_Positional);
            } else {
                // do nothing
            }
        }
    }

    
    if (gene_stop_upd > gene_stop) {
        gene->SetLocation().SetInt().SetTo(gene_stop_upd);
        modified = true;
        if (gene->GetLocation().IsPartialStop(eExtreme_Positional)) {
            if (partial_stop) {
                // do nothing
            } else {
                gene->SetLocation().SetPartialStop(false, eExtreme_Positional);
            }
        } else {
            if (partial_stop) {
                gene->SetLocation().SetPartialStop(true, eExtreme_Positional);
            } else {
                // do nothing
            }
        }
    }

    // set the partial flag if at least one end of the gene is partial
    // unset the flag, if both ends are complete
    if (gene->GetLocation().IsPartialStart(eExtreme_Positional) ||
        gene->GetLocation().IsPartialStop(eExtreme_Positional)) {
        gene->SetPartial(true);
    } else {
        gene->ResetPartial();
    }
        
    
    return modified;
}

END_NCBI_SCOPE
