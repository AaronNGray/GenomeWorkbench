/*  $Id: suc_data_generator.cpp 39066 2017-07-24 19:11:22Z katargir $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/edit/suc_data_generator.hpp>

#include <objtools/format/flat_file_generator.hpp>
#include <objtools/format/flat_file_config.hpp>

#include <objects/submit/Seq_submit.hpp>
#include <objtools/format/items/defline_item.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <gui/objutils/utils.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

class CSUCTextGather: public CFlatFileConfig::CGenbankBlockCallback
{
public:
    CSUCTextGather ( ) : m_Results(new CSUCResults()) {};
    
    virtual ~CSUCTextGather ();
    
    virtual EBioseqSkip notify_bioseq(CBioseqContext& ctx);

    virtual EAction unified_notify( string & block_text, 
                                const CBioseqContext& ctx, const IFlatItem & flat_item, 
                                CFlatFileConfig::FGenbankBlocks which_block);

    CRef<CSUCResults> GetResults() { return m_Results; };

protected:
    CRef<CSUCResults> m_Results;
};


CSUCTextGather::~CSUCTextGather ()
{
#if 0
    for (size_t i = 0; i < m_Blocks.size(); i++) {
        m_Blocks[i]->Print();
    }
#endif
}


CFlatFileConfig::CGenbankBlockCallback::EBioseqSkip CSUCTextGather::notify_bioseq(CBioseqContext& ctx )
{
    if (ctx.IsProt()) {
        return eBioseqSkip_Yes;
    } else {
        return eBioseqSkip_No; 
    }
}


CFlatFileConfig::CGenbankBlockCallback::EAction CSUCTextGather::unified_notify( string & block_text, const CBioseqContext& ctx, const IFlatItem & flat_item, CFlatFileConfig::FGenbankBlocks which_block )
{
    CConstRef<CObject> obj(dynamic_cast<const CObject *>(flat_item.GetObject()));
    if (!obj)
    {
        const CDeflineItem* defLineItem = dynamic_cast<const CDeflineItem*>(&flat_item);
        if (defLineItem) 
        {
            CBioseqContext* ctx = defLineItem->GetContext();
            if (ctx)
            {
                CSeqdesc_CI seqdesc(ctx->GetHandle(), CSeqdesc::e_Title, 1);
                if (seqdesc)
                    obj = ConstRef(&*seqdesc);
            }
        }
    }
    m_Results->AddText(block_text, which_block, ctx.GetAccession(), obj);
    return eAction_Skip;
}

CRef<CSUCResults> CSUCDataGenerator::GetSUCResults(TConstScopedObjects& objects)
{
    CRef<CSUCTextGather> textGather(new CSUCTextGather());

    CFlatFileConfig cfg(
        CFlatFileConfig::eFormat_GenBank,
        CFlatFileConfig::eMode_GBench,
        CFlatFileConfig::eStyle_Normal,
        0,
        CFlatFileConfig::fViewNucleotides);

    cfg.SetGenbankBlocks(
        CFlatFileConfig::fGenbankBlocks_Head |
        CFlatFileConfig::fGenbankBlocks_Locus |
        CFlatFileConfig::fGenbankBlocks_Defline |
        CFlatFileConfig::fGenbankBlocks_Accession |
        CFlatFileConfig::fGenbankBlocks_Version |
        CFlatFileConfig::fGenbankBlocks_Project |
        CFlatFileConfig::fGenbankBlocks_Dbsource |
        CFlatFileConfig::fGenbankBlocks_Keywords |
        CFlatFileConfig::fGenbankBlocks_Segment |
        CFlatFileConfig::fGenbankBlocks_Source |
        CFlatFileConfig::fGenbankBlocks_Reference |
        CFlatFileConfig::fGenbankBlocks_Comment |
        CFlatFileConfig::fGenbankBlocks_Primary |
        CFlatFileConfig::fGenbankBlocks_Sourcefeat |
        CFlatFileConfig::fGenbankBlocks_FeatAndGap |
        CFlatFileConfig::fGenbankBlocks_Basecount |
        CFlatFileConfig::fGenbankBlocks_Origin |
        CFlatFileConfig::fGenbankBlocks_Contig |
        CFlatFileConfig::fGenbankBlocks_Wgs |
        CFlatFileConfig::fGenbankBlocks_Tsa);

    cfg.SetGenbankBlockCallback(textGather.GetPointer());

    cfg.SetShowContigFeatures().SetShowContigSources();

    CRef<CFlatFileGenerator> ffGenerator(new CFlatFileGenerator(cfg));

    CNcbiOstream* os = &cout;
    bool found = false;
    for (TConstScopedObjects::size_type i = 0;  i < objects.size(); ++i) {
        
        CScope* scope = objects[i].scope;
        const CSeq_entry* pSeq_entry = dynamic_cast<const CSeq_entry*>(objects[i].object.GetPointer());
        if (pSeq_entry) {
            CSeq_entry_Handle seh = scope->GetSeq_entryHandle(*pSeq_entry);
            ffGenerator->Generate( seh, *os);
            found = true;
            continue;
        }
        
        const CBioseq_set* pBioseq_set = dynamic_cast<const CBioseq_set*>(objects[i].object.GetPointer());
        if (pBioseq_set)
        {
            CBioseq_set_Handle bssh = scope->GetBioseq_setHandle(*pBioseq_set);
            CSeq_entry_Handle seh = bssh.GetParentEntry();
            ffGenerator->Generate( seh, *os);
            found = true;
            continue;
        }
        
        const CSeq_submit* pSeqSubmit = dynamic_cast<const CSeq_submit*>(objects[i].object.GetPointer());
        if (pSeqSubmit) {
            ffGenerator->Generate(const_cast<CSeq_submit&>(*pSeqSubmit), *scope, *os);
            found = true;
            continue;
        }
        
        const CSeq_id* pSeq_id = dynamic_cast<const CSeq_id*>(objects[i].object.GetPointer());
        if (pSeq_id)
        {
            CBioseq_Handle bsh = scope->GetBioseqHandle(*pSeq_id);
            CSeq_entry_Handle seh = bsh.GetParentEntry();
            ffGenerator->Generate( seh, *os);
            found = true;
            continue;
        }
    }
    if (!found && !objects.empty())
    {
        CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(objects.front());
        ffGenerator->Generate( seh, *os);
    }
    return textGather->GetResults();
}

END_NCBI_SCOPE

