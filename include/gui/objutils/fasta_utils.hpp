#ifndef GUI_UTILS___FASTA_UTILS__HPP
#define GUI_UTILS___FASTA_UTILS__HPP

/*  $Id: fasta_utils.hpp 38900 2017-06-30 20:50:04Z evgeniev $
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
* Authors:  Victor Joukov, Vladislav Evgeniev
* 
* File Description: Set of functions that aid the import of FASTA files (sequences/alignments)
*
*/

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <objects/seqloc/Seq_id.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CScope;
class CSeq_entry;
class CBioseq_Handle;
class CBioseq;
END_SCOPE(objects)

BEGIN_SCOPE(fasta_utils)

/// Status of the FASTA sequence
enum ESeqStatus {
    eSeqStatus_Wellknown,
    eSeqStatus_Edited,
    eSeqStatus_Local
};

/// Compares ids as FASTA strings 
struct NCBI_GUIOBJUTILS_EXPORT CompareSeq_ids {
    bool operator()(const CConstRef<objects::CSeq_id> &lhs, const CConstRef<objects::CSeq_id> &rhs) const {
        return lhs->AsFastaString() < rhs->AsFastaString();
    }
};

/// Maps local ids to well-known ones
typedef std::map<CConstRef<objects::CSeq_id>, CConstRef<objects::CSeq_id>, CompareSeq_ids> TSeq_idMap;

/// Determines the status of a set of FASTA sequences by comparing them to well-known sequences
/// @param[in]  entries set of Bioseq objects whose status is to be determined
/// @param[in]  scope   the objecs scope
/// @param[out] stat    status list
NCBI_GUIOBJUTILS_EXPORT
void GetSeqsStatus(list<CRef<objects::CSeq_entry> >& entries, objects::CScope& scope, list<ESeqStatus>& stat);

/// Removes the well-known sequences from the set and returns their ids (optional)
/// @param[in]  entry           set of Bioseq objects to work on
/// @param[in]  scope           the objecs scope
/// @param[out] wellknown_ids   (optional) pointer to a vector, to hold the well-known Seq-ids, removed the entry
/// @param[out] local_ids       (optional) pointer to a map, to hold the local ids
/// @return New Seq-entry with all well-known sequences removed
NCBI_GUIOBJUTILS_EXPORT
CRef<objects::CSeq_entry> ReplaceWellknownSeqs(objects::CSeq_entry& entry, objects::CScope& scope, vector<CConstRef<objects::CSeq_id>> *wellknown_ids = nullptr, TSeq_idMap *local_ids = nullptr);

/// Updates the organism information for the Seq-entry object by copying the information from a well-known sequence to the local sequence, using the provided mapping
/// @param[in/out]  entry           set of Bioseq objects, whose Org-ref is to be updated
/// @param[in]      scope           the objecs scope
/// @param[in]      local_ids       map, holding the local to well-known ids mapping
NCBI_GUIOBJUTILS_EXPORT
void UpdateOrgInformation(objects::CSeq_entry& entry, objects::CScope& scope, const TSeq_idMap &local_ids);

/// Copies the Org-ref from source to destination
/// @param[in]  source      source of the Org-ref information
/// @param[out] destination destination of the Org-ref information
NCBI_GUIOBJUTILS_EXPORT
void CopyOrg(const objects::CBioseq_Handle &source, objects::CBioseq &destination);

/// Tries to identify the specified local id (works for ids, prefixed with rng_ or mod_, and generated by the GUI tools)
/// @param[in]  fasta_id    local id in FASTA format
/// @return Non-local Seq-id
NCBI_GUIOBJUTILS_EXPORT
CRef<objects::CSeq_id> IdentifyLocalId(const string &fasta_id);

END_SCOPE(fasta_utils)

END_NCBI_SCOPE

#endif  // GUI_UTILS___FASTA_UTILS__HPP
