/*  $Id: taxtree_tool_job.cpp 37336 2016-12-23 21:13:16Z katargir $
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
* Authors:  Roman Katargin, Anatoliy Kuznetsov
*/


#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/taxtree_tool_job.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <util/bitset/ncbi_bitset.hpp>

#include <objects/taxon1/taxon1.hpp>

#include <algo/phy_tree/bio_tree.hpp>
#include <algo/phy_tree/bio_tree_conv.hpp>

#include <objects/biotree/BioTreeContainer.hpp>
#include <objects/biotree/FeatureDescr.hpp>
#include <objects/biotree/FeatureDictSet.hpp>
#include <objects/biotree/NodeFeatureSet.hpp>
#include <objects/biotree/NodeFeature.hpp>
#include <objects/biotree/NodeSet.hpp>
#include <objects/biotree/Node.hpp>

#include <objmgr/util/sequence.hpp>

#include <gui/objutils/label.hpp>
#include <objects/gbproj/AbstractProjectItem.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CTaxTreeToolJob
CTaxTreeToolJob::CTaxTreeToolJob(const CTaxTreeToolParams& params) : m_Params(params)
{
    m_Descr = "Common TaxTree Job";
}

/// Sequence information record
///
/// @internal
struct STaxSeqInfo
{
    STaxSeqInfo() : tax_id(0) {}
    STaxSeqInfo(int tax, string l) : tax_id(tax), label(l) {}

    int     tax_id; ///< taxonomy id
    string  label;  ///< sequence label
};

void CTaxTreeToolJob::x_CreateProjectItems()
{
    TConstScopedObjects& seqs = m_Params.SetObjects();
    if (seqs.size() == 0) {
        LOG_POST("CTaxTreeToolJob: no input!");
        return;
    }

    CTaxon1 taxon1;
    bool tinit_ok = taxon1.Init();
    if (!tinit_ok) {
        return; // TODO: throw an exception
    }

    bm::bvector<> taxbv(bm::BM_GAP);
    CTaxon1::TTaxIdList tax_id_list;


    // sequence-id to tax-id map
    typedef map<CSeq_id_Handle, STaxSeqInfo> TSeqId2Tax;
    TSeqId2Tax seq2tax;


    CScope* scope = seqs[0].scope.GetPointer();

    // Get Tax-ids using bulk-interface
    //    
    map<CSeq_id_Handle, int>  bio2tax;
    try {
        CScope::TSeq_id_Handles   handles;
        ITERATE(TConstScopedObjects, it, seqs) {
            const CObject* object = it->object.GetPointer();

            const CSeq_id* seq_id = 0;
            const CSeq_loc* seq_loc = 0;

            string seqid_label;
            seq_id = dynamic_cast<const CSeq_id*>(object);
            if (!seq_id) {
                seq_loc = dynamic_cast<const CSeq_loc*>(object);
                if (!seq_loc) {
                    continue;  // non sequence id object is ignored at this point
                }
            }

            bool handle_valid = false;
            CBioseq_Handle bio_handle; 
            if (seq_id) {
                bio_handle = scope->GetBioseqHandle(*seq_id);
                handle_valid = true;
            }
            else {
                if (seq_loc  &&  seq_loc->GetId()) {
                    bio_handle = scope->GetBioseqHandle(*seq_loc->GetId());
                    handle_valid = true;
                }
            }
            
            if (handle_valid) {
                CSeq_id_Handle canonical_seq_id_h = 
                                    sequence::GetId(bio_handle,
                                                    sequence::eGetId_Canonical);
                handles.push_back(canonical_seq_id_h);
            }

        }  // ITERATE
        CScope::TTaxIds taxid_vector = scope->GetTaxIds(handles);
        unsigned j = 0;
        ITERATE(CScope::TTaxIds, it, taxid_vector) {
            CSeq_id_Handle bh = handles[j];
            bio2tax[bh] = *it;
            ++j;
        }

    } catch (std::exception&) {
        // it is safe to ignore exceptions here, because we just can use non-bulk interface
    }
    


    // Main algorithm here

    ITERATE(TConstScopedObjects, it, seqs) {
        const CObject* object = it->object.GetPointer();

        const CSeq_id* seq_id = 0;
        const CSeq_loc* seq_loc = 0;

        string seqid_label;
        seq_id = dynamic_cast<const CSeq_id*>(object);
        if (!seq_id) {
            seq_loc = dynamic_cast<const CSeq_loc*>(object);
            if (!seq_loc) {
                continue;  // non sequence id object is ignored at this point
            }
        }

        CBioseq_Handle bio_handle; 
        if (seq_id) {
            bio_handle = scope->GetBioseqHandle(*seq_id);
        }
        else {
            if (seq_loc  &&  seq_loc->GetId()) {
                bio_handle = scope->GetBioseqHandle(*seq_loc->GetId());
            }
        }

        CLabel::GetLabel(*bio_handle.GetSeqId(), &seqid_label,
                         CLabel::eDefault, scope);

        CSeq_id_Handle canonical_seq_id_h = 
                            sequence::GetId(bio_handle,
                                            sequence::eGetId_Canonical);


        //  ------------------------------------------------------------
        // Get the tax id for the sequence
        //
        
        int tax_id = 0;

        map<CSeq_id_Handle, int>::const_iterator tax_it = bio2tax.find(canonical_seq_id_h);
        if (tax_it != bio2tax.end()) {
            tax_id = (*tax_it).second;
        } else {        
            tax_id = sequence::GetTaxId(bio_handle);
        }

        // if not, try the tax server
        if ( !tax_id ) {
            TGi gi = ZERO_GI;
            if ( seq_id && seq_id->IsGi() ) {
                gi = seq_id->GetGi();
            } 
            if (!gi) {
                gi = scope->GetGi(canonical_seq_id_h);
            }
            if (!gi) {
                CConstRef<CSeq_id> seq_id = bio_handle.GetNonLocalIdOrNull();
                if (seq_id && seq_id->IsGi())
                    gi = seq_id->GetGi();
            }

            if (gi) {
                taxon1.GetTaxId4GI(gi, tax_id);
                if (!tax_id) {
                    if (!taxon1.IsAlive()) {
                        const string& err = taxon1.GetLastError();
                        ERR_POST(err);
                    }
                }
            }
        }
        if (tax_id) {
            // check if tax id is present
            if (taxbv[tax_id]) {

            } else {
                tax_id_list.push_back(tax_id);
                taxbv[tax_id] = true;
            }
            seq2tax[canonical_seq_id_h] = STaxSeqInfo(tax_id, seqid_label);
        }
        else {
            ERR_POST("CTaxTreeToolJob: No tax_id for " << seqid_label);
        }
    } // ITERATE

    // ----------------------------------------------------
    // use Taxon client to get the taxonomic tree
    //

    CTaxon1 taxon2;
    taxon2.Init();

    if (tax_id_list.size() == 0) {
         NCBI_THROW(CException, eUnknown,
                   "Can't generate common tree for sequences:\n"
                   "No taxonomy IDs found.");
    }

    CTaxon1::TTaxIdList tax_ids_join;

    if (tax_id_list.size() == 1) {
        tax_ids_join = tax_id_list;
    } else {
        if ( !taxon2.GetPopsetJoin(tax_id_list, tax_ids_join) ) {
            NCBI_THROW(CException, eUnknown,
                       "Can't generate common tree for sequences");
        }
    }

    if (tax_ids_join.size() == 0) {
         NCBI_THROW(CException, eUnknown,
                   "Can't generate common tree for sequences:\n"
                   "No taxonomy IDs found.");
    }

    // load the common tree to taxon
    //
    bool tax_load_ok = false;
    ITERATE(vector<int>, it, tax_ids_join) {
        int tax_id = *it;
        tax_load_ok |= taxon2.LoadNode(tax_id);
    }

    if (!tax_load_ok) {
         NCBI_THROW(CException, eUnknown,
                   "Can't generate common tree for sequences:\n"
                   "Taxonomic load was not successfull.");
    }


    // convert taxon-tree to bio-tree
    //
    CRef<ITreeIterator> tax_tree_iter(taxon2.GetTreeIterator());

	CRef<CBioTreeContainer> btc(new CBioTreeContainer());
    typedef 
	CTaxon1ConvertToBioTreeContainer<CBioTreeContainer, 
			                         CTaxon1,
			                         ITaxon1Node,
			                         ITreeIterator>
    TTaxon1Conv;

	TTaxon1Conv conv_func;
	conv_func(*btc, tax_tree_iter);
    btc->SetTreetype("CommonTaxTree");

    // add fan of sequences to the appropriate taxonomy leafs
    //
    {{
        int max_tax_id = conv_func.GetMaxNodeId();
        // here we take max tax id and just keep counting from that...
        // not sure if this is a good idea to mix-in fake taxids this way?
        ++max_tax_id;

        TTaxon1Conv::TTaxon1Visitor::TNodeList& node_list = btc->SetNodes().Set();
        ITERATE(TSeqId2Tax, mit, seq2tax) {
            string seqid_str = mit->first.AsString();
            int tax_id = mit->second.tax_id;
            string tax_id_str = NStr::IntToString(tax_id);
            const string& label = mit->second.label;

            // construct a sequence node and attach it to org
            {{
                CRef<TTaxon1Conv::TTaxon1Visitor::TCNode> cnode(new TTaxon1Conv::TTaxon1Visitor::TCNode);
                cnode->SetId(max_tax_id);
                cnode->SetParent(tax_id);
                TTaxon1Conv::TTaxon1Visitor::TCNodeFeatureSet& fset = cnode->SetFeatures();
                // seq-id
                {{
			        CRef<TTaxon1Conv::TTaxon1Visitor::TCNodeFeature>  
                                    cfeat(new TTaxon1Conv::TTaxon1Visitor::TCNodeFeature);
			        cfeat->SetFeatureid(eTaxTree_SeqId);
			        cfeat->SetValue(seqid_str);

                    fset.Set().push_back(cfeat);
                }}
                // Label
                {{
			        CRef<TTaxon1Conv::TTaxon1Visitor::TCNodeFeature>  
                                    cfeat(new TTaxon1Conv::TTaxon1Visitor::TCNodeFeature);
			        cfeat->SetFeatureid(eTaxTree_Label);
			        cfeat->SetValue(label);

                    fset.Set().push_back(cfeat);
                }}
                // tax-id
                if (tax_id > 0) {
			        CRef<TTaxon1Conv::TTaxon1Visitor::TCNodeFeature>  
                                    cfeat(new TTaxon1Conv::TTaxon1Visitor::TCNodeFeature);
			        cfeat->SetFeatureid(eTaxTree_TaxId);
			        cfeat->SetValue(tax_id_str);

                    fset.Set().push_back(cfeat);
                }
                node_list.push_back(cnode);
            }}

            ++max_tax_id;
        }
    }}

    // ----------------------------------------------------
    // Prepare project item for the output

    CRef<CProjectItem> item(new CProjectItem());
    item->SetItem().SetOther().Set(*btc);
    string project_item_label("Common TaxTree (");
    project_item_label += NStr::SizetToString(seqs.size());
    project_item_label += " sequences)";
    item->SetLabel(project_item_label);
    AddProjectItem(*item);

}

END_NCBI_SCOPE

