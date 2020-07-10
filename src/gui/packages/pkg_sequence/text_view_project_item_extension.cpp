/*  $Id: text_view_project_item_extension.cpp 37255 2016-12-20 19:14:10Z katargir $
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

#include "text_view_project_item_extension.hpp"

#include <objects/general/Object_id.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/submit/Seq_submit.hpp>

#include <objmgr/bioseq_ci.hpp>

#include <gui/core/document.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/open_view_task.hpp>
#include <gui/framework/app_task_service.hpp>

#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/context.hpp>
#include <objtools/format/items/locus_item.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CTextViewProjectItemExtension

string CTextViewProjectItemExtension::GetExtensionIdentifier() const
{
    static string sid("text_view_project_item_extension");
    return sid;
}

string CTextViewProjectItemExtension::GetExtensionLabel() const
{
    static string slabel("Text View Project item extension");
    return slabel;
}

static bool s_GetFirstBioseqName(const CSeq_entry* entry, CScope& scope, string& sSeqid)
{
    CBioseq_CI bs_it(scope, *entry);
    if (!bs_it) {
        return false;
    }

    // First bioseq handle
    const CBioseq_Handle& bsh = *bs_it;

    // FF config
    CFlatFileConfig ffcfg(CFlatFileConfig::eFormat_GenBank,
        CFlatFileConfig::eMode_GBench,
        CFlatFileConfig::eStyle_Normal,
        0,
        CFlatFileConfig::fViewFirst);

    // FF context
    CFlatFileContext ffctx(ffcfg);

    // First bioseq context
    CBioseqContext bsctx(bsh, ffctx);

    // FF LOCUS item
    CLocusItem locus(bsctx);
    sSeqid = locus.GetName();
    return !sSeqid.empty();
}

static bool s_GetFirstBioseqName(CProjectItem& item, CGBDocument& doc, string& sSeqid)
{
    const CSerialObject* so = item.GetObject();
    if (!so) return false;

    const CSeq_submit* submit = dynamic_cast<const CSeq_submit *>(so);
    if (submit && submit->IsSetData() && submit->IsEntrys()) {
        ITERATE(CSeq_submit::TData::TEntrys, it, submit->GetData().GetEntrys()) {
            if (s_GetFirstBioseqName(*it, *doc.GetScope(), sSeqid)) {
                return true;
            }
        }
    }
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry *>(so);
    if (entry) {
        if (s_GetFirstBioseqName(entry, *doc.GetScope(), sSeqid)) {
            return true;
        }
    }

    return false;
}

void CTextViewProjectItemExtension::ProjectItemAttached(CProjectItem& item, CGBDocument& doc, IServiceLocator* service_locator)
{
    if (!item.HasTag("open_text_view"))
        return;

    if (!service_locator) return;

    CProjectService* prj_srv = service_locator->GetServiceByType<CProjectService>();

    CRef<CGBWorkspace> ws = prj_srv->GetGBWorkspace();
    if (!ws) return;

    const CSerialObject* so = item.GetObject();
    if (!so) return;

    CRef<CUser_object> params(new CUser_object());
    CRef<CObject_id> type(new CObject_id());
    type->SetStr("TextViewParams");
    params->SetType(*type);

    CUser_object::TData& data = params->SetData();

    {{
            CRef<CUser_field> param(new CUser_field());
            CRef<CObject_id> label(new CObject_id());
            label->SetStr("TextViewType");
            param->SetLabel(*label);
            param->SetData().SetStr("Flat File");
            data.push_back(param);
        }}

    {{
            CRef<CUser_field> param(new CUser_field());
            CRef<CObject_id> label(new CObject_id());
            label->SetStr("ExpandAll");
            param->SetLabel(*label);
            param->SetData().SetBool(true);
            data.push_back(param);
        }}

    string sSeqid;
    if (s_GetFirstBioseqName(item, doc, sSeqid)) {
        CRef<CUser_field> param(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr("InitialSequence");
        param->SetLabel(*label);
        param->SetData().SetStr(sSeqid);
        data.push_back(param);
    }

    SConstScopedObject viewObject(so, doc.GetScope());

    COpenViewTask* task = new COpenViewTask(service_locator, "Text View", viewObject, params, true);
    service_locator->GetServiceByType<CAppTaskService>()->AddTask(*task);
}

END_NCBI_SCOPE
