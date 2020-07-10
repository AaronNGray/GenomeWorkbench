/*  $Id: citsub_updater.cpp 41956 2018-11-20 18:42:42Z asztalos $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/general/Date.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <objects/pub/pub_macros.hpp>
#include <gui/widgets/edit/citsub_updater.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const char* CCitSubUpdater::sm_SubmitterUpdateText = "Sequence update by submitter";
const char* CCitSubUpdater::sm_NoEarlierCitSub = "There is no earlier Cit-sub template";
const char* CCitSubUpdater::sm_ChangeExistingCitSub = "Adding update indication to existing cit-sub";
const char* CCitSubUpdater::sm_ExistingCitSub = "There already exists an update on today's date";
const char* CCitSubUpdater::sm_AddedCitSub = "The update Cit-sub has been placed on the top Seq-entry";
const char* CCitSubUpdater::sm_DifferentCitSub = "There already exists a Cit-sub having today's date with a description different than update";

const char* CCitSubUpdater::sm_TrimmedText = "Sequence update by database staff to remove vector contamination";

static const CDate* s_GetLaterDate(const CDate* d1, const CDate* d2)
{
    if (d1 == 0 || d1->Which() == CDate::e_Str) {
        return d2;
    }

    if (d2 == 0 || d2->Which() == CDate::e_Str) {
        return d1;
    }

    return (d1->Compare(*d2) == CDate::eCompare_after) ? d1 : d2;
}

static bool s_FindLatestCitSub(const CPubdesc& pubdesc,
    CConstRef<CDate>& latestDate,
    CConstRef<CCit_sub>& latestCitSub,
    CConstRef<CPub>& latestPub)
{
    bool found = false;
    FOR_EACH_PUB_ON_PUBDESC(pub_iter, pubdesc) {
        const CPub& pub = **pub_iter;
        if (pub.IsSub() && pub.GetSub().IsSetDate()) {
            latestDate = s_GetLaterDate(latestDate, &pub.GetSub().GetDate());
            if (latestDate == &pub.GetSub().GetDate()) {
                latestCitSub.Reset(&pub.GetSub());
                latestPub.Reset(&pub);
                found = true;
            }
        }
    }
    return found;
}

CRef<CSeqdesc> CCitSubUpdater::s_GetCitSubForUpdatedSequence(const CBioseq_Handle& bsh, string& message, CConstRef<CSeqdesc>& changedSeqdesc, CSeq_entry_Handle& seh)
{
    return sx_GetCitSubForSequence(bsh, message, changedSeqdesc, seh, sm_SubmitterUpdateText);
}

CRef<CSeqdesc> CCitSubUpdater::s_GetCitSubForTrimmedSequence(const CBioseq_Handle& bsh, string& message, CConstRef<CSeqdesc>& changedSeqdesc, CSeq_entry_Handle& seh)
{
    return sx_GetCitSubForSequence(bsh, message, changedSeqdesc, seh, sm_TrimmedText);
}

CRef<CSeqdesc> CCitSubUpdater::sx_GetCitSubForSequence(const CBioseq_Handle& bsh,
    string& message, CConstRef<CSeqdesc>& changedSeqdesc, CSeq_entry_Handle& seh, const CTempString& text)
{
    message.clear();
    changedSeqdesc.Reset();
    seh.Reset();

    CConstRef<CDate> latestDate;
    CConstRef<CCit_sub> latestCitSub;
    CConstRef<CSeqdesc> latestSeqdesc;
    CConstRef<CPub> latestPub;

    for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_Pub); desc_it; ++desc_it) {
        if (s_FindLatestCitSub(desc_it->GetPub(), latestDate, latestCitSub, latestPub)) {
            latestSeqdesc.Reset(&(*desc_it));
            seh = desc_it.GetSeq_entry_Handle();
        }
    }

    if (!latestCitSub) {
        message.append(sm_NoEarlierCitSub);
        return CRef<CSeqdesc>();
    }

    CRef<CDate> today(new CDate);
    today->SetToTime(CurrentTime(), CDate::ePrecision_day);

    _ASSERT(latestDate && latestCitSub && latestSeqdesc);
    if (today && today->Compare(*latestDate) == CDate::eCompare_same) {
        if (!latestCitSub->IsSetDescr()) {

            CRef<CSeqdesc> newDesc(new CSeqdesc);
            newDesc->Assign(*latestSeqdesc);
            CPubdesc& pubdesc = newDesc->SetPub();
            EDIT_EACH_PUB_ON_PUBDESC(pub_iter, pubdesc) {
                CPub& pub = **pub_iter;
                if (pub.IsSub() && pub.SameCitation(*latestPub)) {
                    pub.SetSub().SetDescr(text);
                    changedSeqdesc.Reset(latestSeqdesc);
                    message.append(sm_ChangeExistingCitSub);
                    return newDesc;
                }
            }

        }
        else if (NStr::EqualCase(latestCitSub->GetDescr(), text)) {
            message.append(sm_ExistingCitSub);
            return CRef<CSeqdesc>();
        }
        else {
            message.append(sm_DifferentCitSub);
            return CRef<CSeqdesc>();
        }
    }

    // otherwise, make a new cit-sub from template
    CRef<CSeqdesc> new_desc(new CSeqdesc);
    CRef<CPub> new_pub(new CPub);
    new_pub->SetSub().Assign(*latestCitSub);
    new_pub->SetSub().SetDescr(text);
    new_pub->SetSub().SetDate(*today);
    new_desc->SetPub().SetPub().Set().push_back(new_pub);
    message.append(sm_AddedCitSub);
    return new_desc;
}

END_NCBI_SCOPE

