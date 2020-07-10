/*  $Id: smart_save_to_client.cpp 43899 2019-09-17 12:09:19Z choi $
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
 * Authors:  Yoon Choi
 *
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/edit/smart_save_to_client.hpp>

#include <objects/gbproj/ProjectDescr.hpp>
#include <objects/general/Date.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/seqset/seqset_macros.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>

#include <gui/objutils/project_item_extra.hpp>
#include <connect/ncbi_namedpipe.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <gui/widgets/edit/propagate_descriptors.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


static bool s_FindSmartSeqId(CBioseq_Handle bh)
{
    const CBioseq_Handle::TId& bioseq_ids = bh.GetId();
    for (auto const& id : bioseq_ids) {
        if (id.Which() == CSeq_id::e_General) {
            CConstRef<CSeq_id> seq_id = id.GetSeqIdOrNull();
            if (seq_id) {
                const CDbtag& dbtag = seq_id->GetGeneral();
                if (dbtag.IsSetDb() && dbtag.GetDb() == "TMSMART") {
                    return true;
                }
            }
        }
    }
    return false;
}


static void s_DescriptorPropagateDown(CSeq_entry_EditHandle eh)
{
    if (!eh || !eh.IsSet())
        return;

    CBioseq_set_Handle bioseq_set_h = eh.GetSet();
    if (!bioseq_set_h.IsSetClass() || bioseq_set_h.GetClass() != CBioseq_set::eClass_genbank ||
        !bioseq_set_h.GetCompleteBioseq_set()->IsSetSeq_set() || bioseq_set_h.GetCompleteBioseq_set()->GetSeq_set().empty())
		return;

    vector<CRef<CSeqdesc> > descs;
    vector<const CSeqdesc*> to_remove;
    CSeqdesc_CI desc_ci( eh, CSeqdesc::e_not_set, 1);
    for( ; desc_ci; ++desc_ci ) 
    {
		if (CPropagateDescriptors::ShouldStayInPlace(*desc_ci)) 
			continue;

		descs.push_back(CRef<CSeqdesc>( SerialClone(*desc_ci) ) );
        to_remove.push_back(&*desc_ci);
    }
    for (const auto desc : to_remove)
    {
        eh.RemoveSeqdesc(*desc);
    }

    // copy to all immediate children
    CSeq_entry_CI direct_child_ci( bioseq_set_h, CSeq_entry_CI::eNonRecursive );
    for( ; direct_child_ci; ++direct_child_ci ) 
    {
        CSeq_entry_EditHandle eseh = direct_child_ci->GetEditHandle();
        for (size_t i = 0; i < descs.size(); i++)
        {
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(*descs[i]);
            bool found(false);
            FOR_EACH_SEQDESC_ON_SEQENTRY(desc_it, *eseh.GetCompleteSeq_entry())
            {
                if ((*desc_it)->Equals(*new_desc))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                eseh.AddSeqdesc(*new_desc);
        }
    }        
}


// per SMUPD-86, do not update update-date for external records
bool s_IsExternalRecord(const CSeq_id& id)
{
    if (id.IsEmbl() || id.IsPir() || id.IsSwissprot() || 
        id.IsDdbj() || id.IsPrf() || id.IsPdb() || 
        id.IsTpe() || id.IsTpd()) {
        return true;
    } else {
        return false;
    }
}


bool s_IsExternalRecord(CBioseq_Handle bsh)
{
    for (auto id : bsh.GetId()) {
        if(s_IsExternalRecord(*(id.GetSeqId()))) {
            return true;
        }
    }
    return false;
}


bool s_IsExternalRecord(CSeq_entry_Handle seh)
{
    CBioseq_CI bi(seh);
    while (bi) {
        if (s_IsExternalRecord(*bi)) {
            return true;
        }
        ++bi;
    }
    return false;
}


static void s_ActualizeUpdateDate(CSeq_entry_EditHandle eh)
{
	if (!eh || !eh.IsSet())
		return;

	CBioseq_set_Handle bioseq_set_h = eh.GetSet();

	CRef<CSeqdesc> update_date_desc(new CSeqdesc);
	update_date_desc->SetUpdate_date().SetToTime(CTime(CTime::eCurrent), CDate::ePrecision_day);

	// set it to all immediate children
	CSeq_entry_CI child_ci(bioseq_set_h, CSeq_entry_CI::eNonRecursive);
	for (; child_ci; ++child_ci) {
		CSeq_entry_EditHandle eseh = child_ci->GetEditHandle();

		// remove old update time desc
		vector<const CSeqdesc*> to_remove;
		FOR_EACH_SEQDESC_ON_SEQENTRY(desc_it, *eseh.GetCompleteSeq_entry()) {
			if ((*desc_it)->Which() == CSeqdesc::e_Update_date) {
				to_remove.push_back(*desc_it);
			}
		}
		for (const auto desc : to_remove) {
			eseh.RemoveSeqdesc(*desc);
		}

		// add new update time desc
		CRef<CSeqdesc> new_desc(new CSeqdesc);
		new_desc->Assign(*update_date_desc);
		eseh.AddSeqdesc(*new_desc);
	}
}

static void s_DoSmartProcessing(const CSeq_entry* entry, CScope& scope)
{
    CSeq_entry_Handle seh = scope.GetSeq_entryEditHandle(*entry);

    try {
        // Perform BasicCleanup
        CCleanup cleanup;
        CConstRef<CCleanupChange> changes = cleanup.BasicCleanup(const_cast<CSeq_entry& >(*(seh.GetCompleteSeq_entry())));

        CSeq_entry_EditHandle eh(seh);
        // For smupd, wrap single sequence in genbank set
        if (seh.IsSeq() && !s_FindSmartSeqId(seh.GetSeq())) {
            eh.ConvertSeqToSet(CBioseq_set::eClass_genbank);
        }

        s_DescriptorPropagateDown(eh);
        if (!s_IsExternalRecord(seh)) {
            s_ActualizeUpdateDate(eh);
        }
    }
    catch (CException& e) {
        LOG_POST(Error << "error in cleanup: " << e.GetMsg());
    }
}

static void s_WriteToNamedPipe(const string& pipe_name, const string& msg) {
	size_t n_written;
	size_t total_written = 0;
	CNamedPipeClient cli;

	STimeout to = { 1, 0 };
	if (cli.SetTimeout(eIO_Open, &to) != eIO_Success) {
		throw runtime_error("CNamedPipeClient::SetTimeout failed.");
	}

	if (cli.Open(pipe_name) != eIO_Success) {
		throw runtime_error("CNamedPipeClient::Open failed.");
	}

	int max_number_of_attempts = 5;
	while (max_number_of_attempts) {
		EIO_Status status = cli.Write(msg.data() + total_written,
			msg.size() - total_written, &n_written);
		total_written += n_written;
		if (total_written == msg.size()) {
			break;
		}
		if (status == eIO_Success) {
			continue;
		}
		if (status != eIO_Timeout) {
			break;
		}
		--max_number_of_attempts;
	}
	if (total_written != msg.size()) {
		throw runtime_error("Failed to write to named pipe");
	}
}


static void s_SaveToSmartClient(CScope* scope, const CProjectItem& projitem)
{
    if (!scope) NCBI_THROW(CException, eUnknown, "SaveToSmartClient: scope is NULL");

    const CSerialObject* so = projitem.GetObject();

    string pipe;
    CProjectItemExtra::GetStr(projitem, "SmartPipe", pipe);
    if (pipe.empty())
        NCBI_THROW(CException, eUnknown, "SaveToSmartClient: Empty pipe name");

    // Perform post-processing before sending data back to smart.
    const CSeq_submit* submit = dynamic_cast<const CSeq_submit *>(so);
    if (submit && submit->IsSetData() && submit->IsEntrys()) {
        ITERATE(CSeq_submit::TData::TEntrys, it, submit->GetData().GetEntrys()) {
            s_DoSmartProcessing(*it, *scope);
        }
    }
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry *>(so);
    if (entry) {
        s_DoSmartProcessing(entry, *scope);
    }

    vector<string> tokens;
    NStr::Split(pipe, ":", tokens, NStr::fSplit_Tokenize);
    if (tokens.size() != 2)
        NCBI_THROW(CException, eUnknown, "SaveToSmartClient: Invalid pipe name: " + pipe);
    string pipe_name = tokens[0], objId = tokens[1];

    CNcbiOstrstream ostr;
    ostr << "smart:update:" << objId << '\0';
    ostr << MSerial_AsnText << *so;
    string msg = CNcbiOstrstreamToString(ostr);

	s_WriteToNamedPipe(pipe_name, msg);
}


static void s_CancelEditingSession(const CProjectItem& projitem) 
{
	string pipe;
	CProjectItemExtra::GetStr(projitem, "SmartPipe", pipe);
	if (pipe.empty())
		NCBI_THROW(CException, eUnknown, "SaveToSmartClient: Empty pipe name");

	vector<string> tokens;
	NStr::Split(pipe, ":", tokens, NStr::fSplit_Tokenize);
	if (tokens.size() != 2)
		NCBI_THROW(CException, eUnknown, "SaveToSmartClient: Invalid pipe name: " + pipe);
	string pipe_name = tokens[0], objId = tokens[1];

	CNcbiOstrstream ostr;
	ostr << "smart:cancel:" << objId << '\0';
	ostr << MSerial_AsnText;
	string msg = CNcbiOstrstreamToString(ostr);

	s_WriteToNamedPipe(pipe_name, msg);
}


void CSmartClient::SendResult(objects::CScope* scope,
							  const objects::CProjectItem& projitem,
							  EResultType result_type) {
	switch (result_type) {
	case eUpdate:
		s_SaveToSmartClient(scope, projitem);
		break;
	case eCancel:
		s_CancelEditingSession(projitem);
		break;
	default:
		throw runtime_error("Unknown result type.");
	}
}


END_NCBI_SCOPE

