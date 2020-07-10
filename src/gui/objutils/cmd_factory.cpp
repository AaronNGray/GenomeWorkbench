/*  $Id: cmd_factory.cpp 26228 2012-08-07 17:18:05Z katargir $
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
* File Description:
*/

#include <ncbi_pch.hpp>
#include <gui/objutils/cmd_factory.hpp>

#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_composite.hpp>

#include <objects/biblio/Cit_gen.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/ArticleIdSet.hpp>
#include <objects/biblio/ArticleId.hpp>
#include <objects/medline/Medline_entry.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/pub/Pub_set.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>

#include <objmgr/feat_ci.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static void s_GetCitIds(const CPub& pub, int& pmid, int& muid);
static void s_GetCitIds(const CCit_art& gen, int& pmid, int& muid);
static void s_GetCitIds(const CMedline_entry& mle, int& pmid, int& muid);
static void s_GetCitIds(const CCit_gen& art, int& pmid, int& muid);

static bool s_CitMatches(const CPub& pub, int pmid, int muid, const string& uniqueStr);

static bool s_IsOnlySerial(const CPub& pub)
{
    if (!pub.IsGen()) {
        return false;
    }

    const CCit_gen& gen = pub.GetGen();

    if ( !gen.IsSetCit() ) 
    {
        if (!gen.IsSetJournal()  &&  !gen.IsSetDate()  &&
            gen.IsSetSerial_number()  &&  gen.GetSerial_number() > 0) {
            return true;
        }
    }

    return false;
}

CIRef<IEditCommand>
    CCmdFactory::RemoveCitations(CSeq_entry_Handle& seh, const CPub_equiv& pubs)
{
    int pmid = 0, muid = 0;
    string uniqueStr;

    ITERATE (CPub_equiv::Tdata, it, pubs.Get()) {
        const CPub& pub = **it;
        s_GetCitIds(pub, pmid, muid);
    }

    if (pmid == 0 && muid == 0) {
        ITERATE (CPub_equiv::Tdata, it, pubs.Get()) {
            const CPub& pub = **it;
            if (pub.IsMuid()  ||  pub.IsPmid()  ||  pub.IsPat_id()  ||  pub.IsEquiv()) {
                continue;
            }
            if (!s_IsOnlySerial(pub)) {
                pub.GetLabel(&uniqueStr, CPub::eContent, CPub::fLabel_Unique, CPub::eLabel_V1 );
            }
        }
    }

    CRef<CCmdComposite> composite;

    for (CFeat_CI mf(seh);  mf;  ++mf) {
        if (!mf->IsSetCit())
            continue;

        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(mf->GetOriginalFeature());
        if (!new_feat->CanGetCit())
            continue;

        CPub_set& pubs = new_feat->SetCit();
        if (!pubs.IsPub())
            continue;

        CPub_set::TPub& cont = pubs.SetPub();

        bool modified = false;

        for (CPub_set::TPub::iterator it = cont.begin(); it != cont.end(); ) {
            if (s_CitMatches(**it, pmid, muid, uniqueStr)) {
                modified = true;
                it = pubs.SetPub().erase(it);
            }
            else
                ++it;
        }

        if (modified) {
            if (!composite)
                composite.Reset(new CCmdComposite("Modify Feature(s)"));

            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(*mf, *new_feat));
            composite->AddCommand(*chgFeat);
        }
    }

    return CIRef<IEditCommand>(composite.GetPointerOrNull());
}

static void s_GetCitIds(const CPub& pub, int& pmid, int& muid)
{
    switch (pub.Which()) {
    case CPub::e_Gen:
        s_GetCitIds(pub.GetGen(), pmid, muid);
        break;

    case CPub::e_Medline:
        s_GetCitIds(pub.GetMedline(), pmid, muid);
        break;

    case CPub::e_Muid:
        if (muid == 0) {
            muid = pub.GetMuid();
        }
        break;

    case CPub::e_Pmid:
        if (pmid == 0) {
            pmid = pub.GetPmid();
        }
        break;

    case CPub::e_Article:
        s_GetCitIds(pub.GetArticle(), pmid, muid);
        break;

    default:
        break;
    }
}

static void s_GetCitIds(const CCit_gen& gen, int& pmid, int& muid)
{
    if (gen.CanGetMuid()  &&  muid == 0) {
        muid = gen.GetMuid();
    }
    if (gen.CanGetPmid()  &&  pmid == 0) {
        pmid = gen.GetPmid();
    }
}

static void s_GetCitIds(const CMedline_entry& mle, int& pmid, int& muid)
{
    if (mle.CanGetUid()  &&  muid == 0) {
        muid = mle.GetUid();
    }

    if (mle.CanGetPmid()  &&  pmid == 0) {
        pmid = mle.GetPmid();
    }

    if (mle.CanGetCit()) {
        s_GetCitIds(mle.GetCit(), pmid, muid);
    }
}

static void s_GetCitIds(const CCit_art& art, int& pmid, int& muid)
{
    if (!art.CanGetIds()) {
        return;
    }

    ITERATE (CArticleIdSet::Tdata, it, art.GetIds().Get()) {
        switch ((*it)->Which()) {
        case CArticleId::e_Pubmed:
            if (pmid == 0) {
                pmid = (*it)->GetPubmed();
            }
            break;
        case CArticleId::e_Medline:
            if (muid == 0) {
                muid = (*it)->GetMedline();
            }
            break;
        default:
            break;
        }
    }
}

static bool s_CitMatches(const CPub& pub, int pmid, int muid, const string& uniqueStr)
{
    switch (pub.Which()) {
    case CPub::e_Muid:
        return pub.GetMuid() == muid;
    case CPub::e_Pmid:
        return pub.GetPmid() == pmid;
    case CPub::e_Equiv:
        ITERATE (CPub::TEquiv::Tdata, it, pub.GetEquiv().Get()) {
            if ( s_CitMatches(**it, pmid, muid, uniqueStr) ) {
                return true;
            }
        }
        break;
    default:
        // compare based on unique string
        {{
            // you can only compare on unique string if the reference
            // does not have a pmid or muid (example accession: L40362.1)
            if( muid == 0 && pmid == 0 ) {
                string pub_unique;
                pub.GetLabel(&pub_unique, CPub::eContent, CPub::fLabel_Unique, CPub::eLabel_V1 );

                size_t len = pub_unique.length();
                if (len > 0  &&  pub_unique[len - 1] == '>') {
                    --len;
                }
                len = min(len , uniqueStr.length());
                pub_unique.resize(len);
                if (!NStr::IsBlank(uniqueStr)  &&  !NStr::IsBlank(pub_unique)) {
                    if (NStr::StartsWith(uniqueStr, pub_unique, NStr::eNocase)) {
                        return true;
                    }
                }
            }
        }}
        break;
    }
    return false;
}

static bool s_EquivGenes(const CGene_ref& gene1, const CGene_ref& gene2)
{
    if (gene2.CanGetLocus()) {
        if (!gene1.CanGetLocus())
            return false;
        if (gene1.GetLocus() != gene2.GetLocus())
            return false;
    }

    if (gene2.CanGetLocus_tag()) {
        if (!gene1.CanGetLocus_tag())
            return false;
        if (gene1.GetLocus_tag() != gene2.GetLocus_tag())
            return false;
    }

    if (gene2.CanGetAllele()) {
        if (!gene1.CanGetAllele())
            return false;
        if (gene1.GetAllele() != gene2.GetAllele())
            return false;
    }

    return true;
}

CIRef<IEditCommand>
    CCmdFactory::DeleteGeneXRefs(CSeq_entry_Handle& seh, const CGene_ref& gene)
{
    CRef<CCmdComposite> composite;

    for (CFeat_CI mf(seh);  mf;  ++mf) {
        if (mf->GetData().IsGene() || !mf->IsSetXref())
            continue;

        ITERATE(CSeq_feat::TXref, it, mf->GetXref()) {
            if ((**it).CanGetData()) {
                const CSeqFeatData& data = (**it).GetData();
                if (data.IsGene() && s_EquivGenes(gene, data.GetGene())) {
                    if (!composite)
                        composite.Reset(new CCmdComposite("Delete Feature(s)"));

                    CIRef<IEditCommand> delFeat(new CCmdDelSeq_feat(*mf));
                    composite->AddCommand(*delFeat);
                }
            }
        }
    }

    return CIRef<IEditCommand>(composite.GetPointerOrNull());
}

END_NCBI_SCOPE
