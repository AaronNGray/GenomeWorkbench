/*  $Id: tms_utils.cpp 44915 2020-04-17 20:19:41Z evgeniev $
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
#include <corelib/ncbistr.hpp>

#include <serial/serial.hpp>
#include <serial/objistr.hpp>
#include <serial/objistrasn.hpp>

#include <gui/objects/TrackConfigResult.hpp>
#include <gui/objects/TrackConfigSet.hpp>
#include <gui/objects/TrackConfig.hpp>
#include <gui/objects/Category.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/gui_http_session_request.hpp>
#include <gui/objects/tms_utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

// define this to use a development copy of seqconfig.cgi to get TMS information
// #define USE_DEV_TMS

static string s_GetParams(const string& accession, const string& assembly, const string& context)
{
    string params =
        "id=" + NStr::URLEncode(accession) +
        "&app_context="+ NStr::URLEncode(context) +
        "&assm_context=" + NStr::URLEncode(assembly) +
        //"&assm_context=GCF_000001405.25" +
        "&appname=gbench" \
        "&req=gbench" \
        "&ofmt=asnt";

    return params;
}

static string s_GetHost()
{
#ifdef USE_DEV_TMS
    return  "dev.ncbi.nlm.nih.gov";
#else
    return  "www.ncbi.nlm.nih.gov";
#endif
}

static string s_GetPath()
{
    return "/projects/sviewer/seqconfig.cgi";
}

static void s_ConvertMetaData(vector<CRef<CAnnotMetaData> >& data, const CTrackConfigSet& tracks, const string& assembly)
{
    if (!tracks.CanGet()) return;

    CRef<CTrackConfigSet> ignored(new CTrackConfigSet());

    for(const auto& i : tracks.Get()) {
        string trackId;
        if(i->CanGetId()) trackId = i->GetId();
        if(NStr::StartsWith(trackId, "STD", NStr::eNocase) || NStr::StartsWith(trackId, "RSTD", NStr::eNocase)) {
            ignored->Set().push_back(CRef<CTrackConfig>((CTrackConfig*)i.GetPointer()));
            continue;
        }

        string accession, subkey, annotType, key = i->GetKey();
        if(key == "alignment_track")       annotType = "align";
        else if(key == "graph_track")      annotType = "graph";
        else if(key == "feature_track")    annotType = "ftable";
        else if(key == "gene_model_track") annotType = "ftable", subkey = "gene";
        else if(key == "SNP_track")        annotType = "ftable";
        else if(key == "SNP_Bins_track")   annotType = "seq-table", subkey = "citedvar";
        else if(key == "dbvar_track")      annotType = "ftable", subkey = "variation";
        else if(key == "aggregate_feature_track") annotType = "biological_region"; // Default to biological_region
        else {
            ignored->Set().push_back(CRef<CTrackConfig>((CTrackConfig*)i.GetPointer()));
            continue;
        }

        if(key == "SNP_track") {
            if(i->CanGetFilter()) {
                accession = i->GetFilter();
            }

            if (!i->CanGetAnnots() || i->GetAnnots().empty()) {
                ignored->Set().push_back(CRef<CTrackConfig>((CTrackConfig*)i.GetPointer()));
                continue;
            }
            subkey = i->GetAnnots().front();
            if(subkey == "SNP" || CSeqUtils::IsExtendedNAA(subkey)) {
                accession += subkey;
            }
            if(accession.empty()) {
                ignored->Set().push_back(CRef<CTrackConfig>((CTrackConfig*)i.GetPointer()));
                continue;
            }
        } else {
            if(!i->CanGetAnnots() || i->GetAnnots().empty()) {
                ignored->Set().push_back(CRef<CTrackConfig>((CTrackConfig*)i.GetPointer()));
                continue;
            }
            accession = i->GetAnnots().front();
        }

        CRef<CAnnotMetaData> track(new CAnnotMetaData());

        track->m_Name = accession;
        track->m_AssmAcc = assembly;

        if (i->CanGetDisplay_name())
            track->m_Title = NStr::HtmlDecode(i->GetDisplay_name());

        if (i->CanGetHelp())
            track->m_Descr = i->GetHelp();

        if (i->CanGetCategory())
            track->m_xClass = i->GetCategory().GetName();

        if (i->CanGetSubcategory())
            track->m_SubCategory = i->GetSubcategory().GetName();

        if (!subkey.empty())
            track->m_Subtypes.insert(subkey);

        if (i->CanGetSubkey()) {
            subkey = i->GetSubkey();
            NStr::ToLower(subkey);
            track->m_Subtypes.insert(subkey);
        }

        if (i->CanGetShown())
            track->m_Shown = i->GetShown();

		if ((key == "aggregate_feature_track") && !subkey.empty()) {
			annotType = subkey;
			if ((subkey != "biological_region") && (subkey != "aggregate_features")) {
				if (i->CanGetId() && i->CanGetDisplay_name()) {
					LOG_POST(Warning << "CTMSUtils: ignored track " << i->GetId() << " - \"" << i->GetDisplay_name() << "\", due to invalid subkey - " << subkey);
				}
				ignored->Set().push_back(CRef<CTrackConfig>((CTrackConfig*)i.GetPointer()));
				continue;
			}
		}
        
        track->m_AnnotType = annotType;

        data.push_back(track);
    }

    //CNcbiOfstream ostr("C:\\Users\\katargir\\AppData\\Roaming\\GenomeWorkbench2\\tms_ignored.asn", ios::binary);
    //ostr << MSerial_AsnText << *ignored;
}

void CTMSUtils::GetTrackMetaData(vector<CRef<CAnnotMetaData> >& data, const string& accession, const string& assembly, const string& tms_context)
{
    string host = s_GetHost();
    string path = s_GetPath();
    string params = s_GetParams(accession, assembly, tms_context);
    string url = "https://" + host + path + "?" + params;

    CRef<CTrackConfigResult> res(new CTrackConfigResult());
    try {
        CGuiHttpSessionRequest httpRequest(url);
        unique_ptr<CObjectIStream> obj_strm(new CObjectIStreamAsn(httpRequest.GetResponseStream()));
        *obj_strm >> *res;
    } NCBI_CATCH("CTMSUtils::GetTrackMetaData()");

    if (!res->CanGetSuccess()) {
        LOG_POST(Error << "CTMSUtils: failed to get valid response from server");
        return;
    }

    if (!res->GetSuccess()) {
        string errMsg;
        if (res->CanGetErr_msg())
            errMsg = res->GetErr_msg();
        else
            errMsg = "Unknown error";
        LOG_POST(Error << "CTMSUtils: failed to get TMS tracks. " << errMsg);
        return;
    }

    if (res->CanGetTrack_config_set())
        s_ConvertMetaData(data, res->GetTrack_config_set(), assembly);
    else
        LOG_POST(Error << "CTMSUtils: no TMS tracks.");
}

END_NCBI_SCOPE
