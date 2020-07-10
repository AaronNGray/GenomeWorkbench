/*  $Id: flat_file_retrieve_job.cpp 39874 2017-11-15 21:51:02Z katargir $
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
 *
 */

#include <ncbi_pch.hpp>

#include <objmgr/bioseq_handle.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/util/sequence.hpp>

#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/context.hpp>
#include <objtools/format/item_ostream.hpp>
#include <objtools/format/item_formatter.hpp>
#include <objtools/format/gather_items.hpp>

#include <objtools/format/items/item.hpp>
#include <objtools/format/items/locus_item.hpp>
#include <objtools/format/items/defline_item.hpp>
#include <objtools/format/items/accession_item.hpp>
#include <objtools/format/items/version_item.hpp>
#include <objtools/format/items/dbsource_item.hpp>
#include <objtools/format/items/keywords_item.hpp>
#include <objtools/format/items/source_item.hpp>
#include <objtools/format/items/reference_item.hpp>
#include <objtools/format/items/dbsource_item.hpp>
#include <objtools/format/items/comment_item.hpp>
#include <objtools/format/items/feature_item.hpp>
#include <objtools/format/items/gap_item.hpp>
#include <objtools/format/items/basecount_item.hpp>
#include <objtools/format/items/origin_item.hpp>
#include <objtools/format/items/locus_item.hpp>
#include <objtools/format/items/sequence_item.hpp>
#include <objtools/format/items/contig_item.hpp>
#include <objtools/format/items/ctrl_items.hpp>
#include <objtools/format/items/segment_item.hpp>
#include <objtools/format/items/genome_project_item.hpp>
#include <objtools/format/items/primary_item.hpp>
#include <objtools/format/items/wgs_item.hpp>
#include <objtools/format/items/tsa_item.hpp>

#include <gui/widgets/seq/flat_file_context.hpp>
#include <gui/widgets/seq/flat_file_text_item.hpp>
#include <gui/widgets/seq/flat_file_header_item.hpp>
#include <gui/widgets/seq/flat_file_feature_collapsed.hpp>
#include <gui/widgets/seq/flat_file_seqblock_collapsed.hpp>
#include <gui/widgets/seq/flat_file_view_params.hpp>

#include <gui/widgets/text_widget/expand_item.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/plain_text_item.hpp>
#include <gui/widgets/seq/flat_file_locus_item.hpp>

#include "features_group.hpp"
#include "sequence_group.hpp"

#include <gui/widgets/seq/flat_file_retrieve_job.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

namespace // anonymous
{

enum EFlatfileRowType {
    eRow_unknown,

    eRow_comment,
    eRow_reference,
    eRow_feature,
    eRow_locus,
    eRow_defline,
    eRow_accession,
    eRow_genom_project,
    eRow_version,
    eRow_keywords,
    eRow_source,
    eRow_feature_header,
    eRow_source_feature,
    eRow_origin,
    eRow_sequence,
    eRow_dbsource,
    eRow_gap,
    eRow_base_count,
    eRow_contig,
    eRow_primary,
    eRow_wgs,
    eRow_tsa,
    // XXX - add the rest

    // control items
    eRow_begin,
    eRow_begin_section,
    eRow_end_section,
    eRow_end
};

enum EChapterType
{
    eChapter_unknown,

    eChapter_header,
    eChapter_references,
    eChapter_comment,
    eChapter_features,
    eChapter_sequence,
    eChapter_primary
};

static EFlatfileRowType s_GetFlatRowType(const IFlatItem* itemp)
{
    if (dynamic_cast<const CCommentItem*>(itemp) != NULL) {
        return eRow_comment;
    } else if (dynamic_cast<const CReferenceItem*>(itemp) != NULL) {
        return eRow_reference;
    } else if (dynamic_cast<const CFeatureItem*>(itemp) != NULL) {
        return eRow_feature;
    } else if (dynamic_cast<const CLocusItem*>(itemp) != NULL) {
        return eRow_locus;
    } else if (dynamic_cast<const CDeflineItem*>(itemp) != NULL) {
        return eRow_defline;
    } else if (dynamic_cast<const CAccessionItem*>(itemp) != NULL) {
        return eRow_accession;
    } else if (dynamic_cast<const CGenomeProjectItem*>(itemp) != NULL) {
        return eRow_genom_project;
    } else if (dynamic_cast<const CVersionItem*>(itemp) != NULL) {
        return eRow_version;
    } else if (dynamic_cast<const CKeywordsItem*>(itemp) != NULL) {
        return eRow_keywords;
    } else if (dynamic_cast<const CSourceItem*>(itemp) != NULL) {
        return eRow_source;
    } else if (dynamic_cast<const CFeatHeaderItem*>(itemp) != NULL) {
        return eRow_feature_header;
    } else if (dynamic_cast<const CSourceFeatureItem*>(itemp) != NULL) {
        return eRow_source_feature;
    } else if (dynamic_cast<const COriginItem*>(itemp) != NULL) {
        return eRow_origin;
    } else if (dynamic_cast<const CSequenceItem*>(itemp) != NULL) {
        return eRow_sequence;
    } else if (dynamic_cast<const CStartSectionItem*>(itemp) != NULL) {
        return eRow_begin_section;
    } else if (dynamic_cast<const CEndSectionItem*>(itemp) != NULL) {
        return eRow_end_section;
    } else if (dynamic_cast<const CDBSourceItem*>(itemp) != NULL) {
        return eRow_dbsource;
    } else if (dynamic_cast<const CGapItem*>(itemp) != NULL) {
        return eRow_gap;
    } else if (dynamic_cast<const CBaseCountItem*>(itemp) != NULL) {
        return eRow_base_count;
    } else if (dynamic_cast<const CContigItem*>(itemp) != NULL) {
        return eRow_contig;
    } else if (dynamic_cast<const CPrimaryItem*>(itemp) != NULL) {
        return eRow_primary;
    } else if (dynamic_cast<const CWGSItem*>(itemp) != NULL) {
        return eRow_wgs;
    } else if (dynamic_cast<const CTSAItem*>(itemp) != NULL) {
        return eRow_tsa;
    } else if (dynamic_cast<const CStartItem*>(itemp) != NULL) {
        return eRow_begin;
    } else if (dynamic_cast<const CEndItem*>(itemp) != NULL) {
        return eRow_end;
    }

    return eRow_unknown;
}

enum EChapterType sChapters[] = {
    eChapter_unknown,    // eRow_unknown

    eChapter_comment,    // eRow_comment
    eChapter_references, // eRow_reference,
    eChapter_features,   // eRow_feature
    eChapter_header,     // eRow_locus
    eChapter_header,     // eRow_defline
    eChapter_header,     // eRow_accession
    eChapter_header,     // eRow_genom_project
    eChapter_header,     // eRow_version
    eChapter_header,     // eRow_keywords
    eChapter_header,     // eRow_source
    eChapter_features,   // eRow_feature_header
    eChapter_features,   // eRow_source_feature
    eChapter_sequence,   // eRow_origin
    eChapter_sequence,   // eRow_sequence
    eChapter_header,     // eRow_dbsource
    eChapter_features,   // eRow_gap
    eChapter_sequence,   // eRow_base_count
    eChapter_sequence,   // eRow_contig
    eChapter_primary,    // eRow_primary
    eChapter_sequence,   // eRow_wgs
    eChapter_sequence,   // eRow_tsa
    // XXX - add the rest

    // control items
    eChapter_unknown,    // eRow_begin
    eChapter_unknown,    // eRow_begin_section
    eChapter_unknown,    // eRow_end_section
    eChapter_unknown     // eRow_end
};

/*
EChapterType s_GetChapterForItem(EFlatfileRowType i_type)
{
    switch (i_type) {
    case eRow_locus:
    case eRow_defline:
    case eRow_accession:
    case eRow_genom_project:
    case eRow_version:
    case eRow_dbsource:
    case eRow_keywords:
    case eRow_source:
        return eChapter_header;

    case eRow_reference:
        return eChapter_references;

    case eRow_comment:
        return eChapter_comment;

    case eRow_feature_header:
    case eRow_source_feature:
    case eRow_feature:
    case eRow_gap:
       return eChapter_features;

    case eRow_base_count:
    case eRow_origin:
    case eRow_contig:
    case eRow_sequence:
        return eChapter_sequence;

    case eRow_begin:
    case eRow_begin_section:
    case eRow_end_section:
    case eRow_end:
    case eRow_unknown:
    default:
        break;
    }

    return eChapter_unknown;
}
*/
} // anonymous namespace

const size_t kMinSeqBlockDigits = 4;

class CFlatFileFactoryBuilder : public CFlatItemOStream
{
public:
    CFlatFileFactoryBuilder(CFlatFileRetrieveJob& job, bool hideVariations, bool hideSTS, CFlatFileContext& ctx)
        : m_Job(job),
        m_HideVariations(hideVariations),
        m_HideSTS(hideSTS),
        m_Ctx(ctx),
        m_ChapterType(eChapter_unknown),
        m_CurrentChapter(0),
        m_SeqBlockDigits(kMinSeqBlockDigits) {}
    ~CFlatFileFactoryBuilder();

    vector<ITextItem*>& GetItems() { return m_Items; }

private:
    virtual void AddItem(CConstRef<IFlatItem> item);

    CFlatFileRetrieveJob& m_Job;
    bool m_HideVariations;
    bool m_HideSTS;
    CFlatFileContext& m_Ctx;

    EChapterType m_ChapterType;
    auto_ptr<CCompositeTextItem> m_CurrentSection;
    CCompositeTextItem* m_CurrentChapter;

    size_t m_SeqBlockDigits;
    string m_SeqName;
	bool   m_Nucleotide;
    vector<ITextItem*> m_Items;
};

static size_t s_CalcSeqBlockDigits(size_t length)
{
    const size_t kLineSize = 60;

    if (length == 0)
        return kMinSeqBlockDigits;

    size_t numLines = (length + kLineSize - 1)/kLineSize;
    size_t maxNumber = (numLines - 1)*kLineSize + 1;
    size_t digits = 0;
    do {
        maxNumber /= 10;
        ++digits;
    } while (maxNumber > 0);

    return (kMinSeqBlockDigits < digits) ? digits : kMinSeqBlockDigits;
}

CFlatFileFactoryBuilder::~CFlatFileFactoryBuilder()
{
    for (auto i : m_Items) delete i;
}


class CFlatFileExpandItem : public CExpandItem
{
public:
    CFlatFileExpandItem(CTextItem* collapsedItem, CTextItem* expandedItem,
        bool expand = false) : CExpandItem(collapsedItem, expandedItem, expand) {}

    virtual bool Traverse(ICompositeTraverser& traverser)
    {
        if (!traverser.ProcessItem(*this))
            return false;

        return true;
    }
};

void CFlatFileFactoryBuilder::AddItem(CConstRef<IFlatItem> item)
{
    m_Job.x_CheckCancelled();

    const IFlatItem* itemp = item.GetPointerOrNull();
    if (itemp == NULL)
        return;

    EFlatfileRowType row_type = s_GetFlatRowType(itemp);
    EChapterType chap_type = sChapters[row_type];

    if (row_type == eRow_begin)
        return;

    if (row_type == eRow_begin_section) {
        m_SeqBlockDigits = kMinSeqBlockDigits;
        m_CurrentSection.reset(new CCompositeTextItem());
        return;
    }
    else if (row_type == eRow_end_section) {
        m_CurrentChapter = 0;

        CPlainTextItem* textItem = new CPlainTextItem();
        textItem->AddLine("//");
        textItem->AddLine("");
        m_CurrentSection->AddItem(textItem, false);

        CExpandItem* expandItem =
            new CExpandItem(new CFlatFileSectionHeader(m_SeqName, m_Nucleotide, m_Ctx), m_CurrentSection.release(), true);
        expandItem->SetSelectable(false);
        m_Items.push_back(expandItem);

        m_SeqName.clear();
		m_Nucleotide = false;

        return;
    }
    else if (row_type == eRow_locus) {
        const CLocusItem* locusItem = dynamic_cast<const CLocusItem*>(itemp);
        m_SeqName = locusItem->GetName();
        CBioseqContext* ctx = locusItem->GetContext();
        if (ctx) {
            CBioseq_Handle& handle = ctx->GetHandle();
			m_Nucleotide = handle.IsNucleotide();
            if (handle) {
                m_SeqBlockDigits = s_CalcSeqBlockDigits(handle.GetBioseqLength());
            }
        }
    }

    if (!m_CurrentSection.get()) {
        return;
    }

    if (row_type == eRow_unknown)
        return;

    if (row_type == eRow_end || row_type == eRow_end_section || chap_type != m_ChapterType)
        m_CurrentChapter = 0;

    if (row_type == eRow_end)
        return;

    if (m_CurrentChapter == 0) {

        m_CurrentChapter = new CCompositeTextItem();
        m_ChapterType = chap_type;

        switch (m_ChapterType) {
        case eChapter_header:
            {{
                //CExpandItem* expandItem =
                    //new CExpandItem(new CFlatFileHeader("HEADER ..."), m_CurrentChapter, true);
                //expandItem->SetSelectable(false);
                //m_Job.x_AddItem(expandItem, m_CurrentSection);
                m_CurrentSection->AddItem(m_CurrentChapter, false);
            }}
            break;
        case eChapter_references:
            {{
                CExpandItem* expandItem =
                    new CExpandItem(new CFlatFileHeader("PUBLICATIONS ..."), m_CurrentChapter, true);
                expandItem->SetSelectable(false);
                m_CurrentSection->AddItem(expandItem, false);
            }}
            break;
        case eChapter_comment:
            {{
                CExpandItem* expandItem =
                    new CExpandItem(new CFlatFileHeader("COMMENT ..."), m_CurrentChapter, true);
                expandItem->SetSelectable(false);
                m_CurrentSection->AddItem(expandItem, false);
            }}
            break;
        case eChapter_primary:
        {{
            CExpandItem* expandItem =
                new CExpandItem(new CFlatFileHeader("PRIMARY ..."), m_CurrentChapter, true);
            expandItem->SetSelectable(false);
            m_CurrentSection->AddItem(expandItem, false);
            }}
            break;
        case eChapter_features:
            {{
                CExpandItem* expandItem =
                    new CFeaturesGroup(new CFlatFileHeader("FEATURES ..."), m_CurrentChapter, true);
                m_CurrentSection->AddItem(expandItem, false);
            }}
            break;
        case eChapter_sequence:
            m_CurrentSection->AddItem( new CSequenceGroup(new CFlatFileHeader("SEQUENCE ..."), m_CurrentChapter, true), false);
            break;
        default:
            m_CurrentSection->AddItem(m_CurrentChapter, false);
            return;
        }
    }

    if (eRow_feature == row_type && (m_HideVariations || m_HideSTS)) {
        const CMappedFeat& feature = static_cast<const CFeatureItem&>(*itemp).GetFeat();
        CSeqFeatData::ESubtype subType = feature.GetData().GetSubtype();
        if (m_HideVariations && subType == CSeqFeatData::eSubtype_variation)
            return;
        if (m_HideSTS && subType == CSeqFeatData::eSubtype_STS)
            return;
    }

    if (eRow_source_feature == row_type ||eRow_feature == row_type || eRow_gap == row_type) {
        CFlatFileTextItem* flatFileItem = new CFlatFileTextItem(item);
        flatFileItem->SetEditFlags(0);
        m_CurrentChapter->AddItem(new CFlatFileExpandItem(new CFlatFileFeatureCollapsed(item), flatFileItem, false), false);
    }
    else if (eRow_sequence == row_type) {
        CFlatFileTextItem* flatFileItem = new CFlatFileTextItem(item);
        flatFileItem->SetEditFlags(0);
        m_CurrentChapter->AddItem(new CFlatFileExpandItem(new CFlatFileSeqBlockCollapsed(item, m_SeqBlockDigits), flatFileItem, false), false);
    }
    else if (eRow_locus == row_type) {
        m_CurrentChapter->AddItem(new CFlatFileLocusItem(item));
    }
    else
        m_CurrentChapter->AddItem(new CFlatFileTextItem(item));
}

CFlatFileRetrieveJob::CFlatFileRetrieveJob(
    CTextPanelContext& context,
    const vector<pair<CBioseq_Handle, string> >& handles,
    const SFlatFileParams& params)
    : CTextRetrieveJob(context), m_Handles(handles), m_Params(params)
{
}

CFlatFileRetrieveJob::~CFlatFileRetrieveJob()
{
}

IAppJob::EJobState CFlatFileRetrieveJob::x_Run()
{
    CFlatFileConfig::TFlags flags = 0;
    if (m_Params.m_ShowComponentFeats || m_Params.m_Style == CFlatFileConfig::eStyle_Contig)
        flags = CFlatFileConfig::fShowContigFeatures | 
                CFlatFileConfig::fShowContigSources |
                CFlatFileConfig::fShowFarTranslations;

    CFlatFileConfig config(CFlatFileConfig::eFormat_GenBank, m_Params.m_Mode, m_Params.m_Style, flags, CFlatFileConfig::fViewAll);

    for (auto& i : m_Handles) {

        x_CheckCancelled();

        ITextItem* item = 0;
        if (i.first) {
            CSeq_entry_Handle seh = i.first.GetParentEntry();
            if (seh) {
                CRef<CFlatFileContext> ctx(new CFlatFileContext(config));
                ctx->SetEntry(seh);
                if (m_Params.m_SeqSubmit) ctx->SetSubmit(*m_Params.m_SeqSubmit);
                ctx->SetLocation(m_Params.m_SeqLoc);
                if (m_Params.m_ShowComponentFeats || m_Params.m_Style == CFlatFileConfig::eStyle_Contig)
                    ctx->SetAnnotSelector().SetResolveAll();

                CRef<CFlatFileFactoryBuilder> builder(new CFlatFileFactoryBuilder(*this, m_Params.m_HideVariations, m_Params.m_HideSTS, *ctx));

                CRef<CFlatGatherer> gatherer(CFlatGatherer::New(config.GetFormat()));
                gatherer->Gather(*ctx, *builder);

                vector<ITextItem*>& items = builder->GetItems();
                if (!items.empty()) {
                    item = items.front();
                    items.front() = 0;
                }
                else {
                    NCBI_THROW(CException, eUnknown, "No FlatFile data for sequence \"" + i.second + "\".");
                }
            }
        }
        x_AddItem(item);
    }

    x_CreateResult();

    return eCompleted;
}

END_NCBI_SCOPE
