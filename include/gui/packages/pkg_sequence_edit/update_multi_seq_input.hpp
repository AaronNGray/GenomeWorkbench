#ifndef _GUI_PACKAGES_SEQUPDATE_MULTI_INPUT_HPP_
#define _GUI_PACKAGES_SEQUPDATE_MULTI_INPUT_HPP_
/*  $Id: update_multi_seq_input.hpp 42167 2019-01-08 17:17:20Z filippov $
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

#include <corelib/ncbistd.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objmgr/scope.hpp>

#include <gui/packages/pkg_sequence_edit/update_seq_input.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects);
    class CSeq_entry_Handle;
END_SCOPE(objects);


class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CUpdateMultipleSeq_Input
{
public:
    CUpdateMultipleSeq_Input(void);
    ~CUpdateMultipleSeq_Input();

    typedef map<objects::CSeq_id_Handle, CRef<CUpdateSeq_Input>> TIDToUpdInputMap;
    typedef vector<objects::CSeq_id_Handle> TSeqIDHVector;

    typedef map<objects::CSeq_id_Handle, objects::CSeq_id_Handle> TOldToUpdateMap;


    bool SetOldEntryAndScope(const objects::CSeq_entry_Handle& tse);
    objects::CSeq_entry_Handle GetOldEntry() {return m_OldSeh;}
    
    bool SetUpdateEntry(CRef<objects::CSeq_entry> update);

    // read fasta update sequences from the clipboard
    bool ReadUpdSeqs_FromClipboard(objects::ILineErrorListener* msg_listener);

    // read update sequences from file
    bool ReadUpdSeqs_FromFile(objects::ILineErrorListener* msg_listener);

    const TIDToUpdInputMap& GetNonIdenticalUpdates() const { return m_NonIdentUpdates; }
    const TIDToUpdInputMap& GetIdenticalUpdates() const { return m_IdenticalUpdates; }

    const TSeqIDHVector& GetSeqsWithoutUpdates() const { return m_WithoutUpdates; }
    const TSeqIDHVector& GetUnmatchedUpdateSeqs() const { return m_UnmatchedUpdates; }

    void PrepareMultipleSequenceInputs();

    bool AppendOld_UpdateSeqPairs(const TOldToUpdateMap& update_pairs);

    CRef<CUpdateSeq_Input> GetUpdateInput(const objects::CSeq_id_Handle& idh) const;

    bool HaveIdenticalResidues(const objects::CSeq_id_Handle& idh1, const objects::CSeq_id_Handle& idh2);
    // for unit testing:
    bool PrepareMultipleSequenceInputsForUnitTest();

    objects::CSeq_inst::EMol GetMolType() const { return m_MolType; }

private:
    bool x_MatchIdsAndAddUpdateToScope();

    bool x_ReadFromStream(CNcbiIstream& istr, objects::ILineErrorListener* msg_listener);
    bool x_ReadUpdateSeqs_Fasta(CNcbiIstream& istr, objects::ILineErrorListener* msg_listener);
    bool x_ReadUpdateSeqs_ASN(CNcbiIstream& istr);

    bool x_CreateUpdateSeqInput(const TOldToUpdateMap& updatepairs, TIDToUpdInputMap& inputmap);
    bool x_CreateUpdateSeqInputForUnitTest(const TOldToUpdateMap& updatepairs, TIDToUpdInputMap& inputmap);

    int m_NumUpdated;
    int m_NumSkipped;

    TIDToUpdInputMap m_NonIdentUpdates;       // list of old sequences with non-identical update sequences
    TIDToUpdInputMap m_IdenticalUpdates;      // list of old sequences with identical update sequences
    TSeqIDHVector m_WithoutUpdates;           // list of old sequences with no updates
    TSeqIDHVector m_UnmatchedUpdates;         // list of update sequences that were not matched to any old sequences

    objects::CSeq_entry_Handle m_OldSeh;
    CRef<objects::CSeq_entry> m_UpdEntry;
    CRef<objects::CScope> m_Scope;

    TOldToUpdateMap m_NonIdentPairs;
    TOldToUpdateMap m_IdenticalPairs;

    // prohibit copy constructor and assignment operator
    CUpdateMultipleSeq_Input(const CUpdateMultipleSeq_Input&);
    CUpdateMultipleSeq_Input& operator=(const CUpdateMultipleSeq_Input&);

    objects::CSeq_inst::EMol m_MolType = objects::CSeq_inst::eMol_na;
};

END_NCBI_SCOPE

#endif
    // _GUI_PACKAGES_SEQUPDATE_MULTI_INPUT_HPP_
