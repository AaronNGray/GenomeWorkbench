/*  $Id: gb_object_loader.cpp 39043 2017-07-21 19:03:05Z katargir $
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
* Authors:  Roman Katargin
*/


#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/gb_object_loader.hpp>

#include <util/icanceled.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/na_utils.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/annot_ci.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CGBObjectLoader::CGBObjectLoader(const vector<CRef<CObject> >& ids, const vector<string>& NAs)
    : m_Ids(ids), m_NAs(NAs)
{
}

IObjectLoader::TObjects& CGBObjectLoader::GetObjects()
{
    return m_Objects;
}

string CGBObjectLoader::GetDescription() const
{
    int n = m_Ids.size() + m_NAs.size();
    string s = NStr::IntToString(n) + (n == 1 ? " id" : " ids");
    return "Loading " + s + " from GenBank";
}

bool CGBObjectLoader::PreExecute()
{
    return true;
}

bool CGBObjectLoader::Execute(ICanceled& canceled)
{
    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    CRef<CScope> scope(new CScope(*obj_mgr));
    scope->AddDefaults();

    NON_CONST_ITERATE(vector<CRef<CObject> >, it, m_Ids) {
        if (canceled.IsCanceled())
            return false;
        
        string id_str;
        try {
            CSeq_id*  seq_id = dynamic_cast<CSeq_id*> (it->GetPointer());
            CSeq_loc* seq_loc = dynamic_cast<CSeq_loc*> (it->GetPointer());

            if (seq_id == 0 && seq_loc == 0) {
                LOG_POST(Error << "CGBObjectLoader::Execute(): Invalid object type to open.");
                continue;
            }

            if (seq_id) {
                id_str = seq_id->GetSeqIdString(true);
            } else if (seq_loc) {
                const CSeq_id* loc_id = seq_loc->GetId();
                if (loc_id) {
                    id_str = loc_id->GetSeqIdString(true);
                }
            }

            if (seq_loc && seq_loc->IsInt()) {
                CBioseq_Handle handle = scope->GetBioseqHandle(*seq_loc);
                TSeqPos bioseqLength = handle.GetBioseqLength();
                CSeq_interval& ival = seq_loc->SetInt();

                if (ival.GetFrom() < bioseqLength) {
                    if (ival.GetTo() > bioseqLength)
                        ival.SetTo(bioseqLength);
                } else {
                    ival.SetFrom(0);
                    ival.SetTo(bioseqLength);
                }
            }

            string label;
            CLabel::GetLabel(**it, &label, CLabel::eDefault, scope);
            m_Objects.push_back(SObject(**it, label));
        }
        catch (const CException& e) {
            x_UpdateHTMLResults(id_str, 0, e.GetMsg());
        }
        catch (const exception& e) {
            x_UpdateHTMLResults(id_str, 0, e.what());
        }
    }

    for (const auto& na : m_NAs) {
        if (canceled.IsCanceled())
            return false;

        try {
            CNAUtils::EEntrezDB eedb = CNAUtils::EEDB_All;
            CNAUtils::TGis gis;
            CNAUtils::GetAllGIs(na, eedb, gis);
            if (gis.empty()) {
                x_UpdateHTMLResults(na, 0, "Failed to find associated sequences");
                continue;
            }

            CBioseq_Handle handle;

            for (auto gi : gis) {
                CRef<CSeq_id> seqId(new CSeq_id());
                seqId->SetGi(gi);
                handle = scope->GetBioseqHandle(*seqId);
                if (handle)
                    break;
            }

            if (!handle) {
                x_UpdateHTMLResults(na, 0, "Failed to load associated sequences");
                continue;
            }

            SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
            sel.AddNamedAnnots(na);
            sel.IncludeNamedAnnotAccession(na);

            CAnnot_CI annot_iter(handle, sel);
            if (!annot_iter) {
                x_UpdateHTMLResults(na, 0, "Failed to load annotation");
                continue;
            }

            CConstRef<CSeq_annot> annot = (*annot_iter).GetCompleteSeq_annot();
            if (!annot) {
                x_UpdateHTMLResults(na, 0, "Failed to load annotation");
                continue;
            }

            string label;
            CLabel::GetLabel(*annot, &label, CLabel::eDefault, scope);
            m_Objects.push_back(SObject(const_cast<CSeq_annot&>(*annot), label));
        }
        catch (const CException& e) {
            x_UpdateHTMLResults(na, 0, e.GetMsg());
        }
        catch (const exception& e) {
            x_UpdateHTMLResults(na, 0, e.what());
        }
    }

    return true;
}

bool CGBObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("GenBank import errors"));
    return true;
}

END_NCBI_SCOPE
