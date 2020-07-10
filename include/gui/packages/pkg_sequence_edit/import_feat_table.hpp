#ifndef _GUI_PACKAGES_IMPORT_FEAT_TABLE_INPUT_HPP_
#define _GUI_PACKAGES_IMPORT_FEAT_TABLE_INPUT_HPP_
/*  $Id: import_feat_table.hpp 45001 2020-05-05 20:50:53Z asztalos $
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
 * Authors:  Colleen Bollin, Andrea Asztalos
 */

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <gui/objutils/macro_cmd_composite.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/utils/object_loader.hpp>
#include <gui/packages/pkg_sequence_edit/match_feat_table_dlg.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CSeq_annot;
    class CSeq_feat;
    class CSeq_entry;
    class CBioseq_Handle;
    class CTSE_Handle;
END_SCOPE(objects)


class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CImportFeatTable 
{
public:
    typedef CMatchFeatureTableDlg::TFeatSeqIDMap TFeatSeqIDMap;
    typedef CMatchFeatureTableDlg::TUniqueFeatTblIDs TUniqueFeatTblIDs; 

    CImportFeatTable(objects::CSeq_entry_Handle seh) : m_Seh(seh), m_is_gff(false) {}

    ~CImportFeatTable() {}
    
    CIRef<IEditCommand> ImportFeaturesFromFile(const wxString& workDir = wxEmptyString, const string &format_id = kEmptyStr);
    CIRef<IEditCommand> ImportFeaturesFromClipboard(const wxString& workDir);

    CIRef<IEditCommand> AddSeqAnnotToSeqEntry(const objects::CSeq_annot& orig_annot, TFeatSeqIDMap& ftbl_seqid_map,
                                              unsigned int &startingLocusTagNumber, unsigned int &startingFeatureId, const string &locus_tag, bool euk);
    
    // used for testing
    CIRef<IEditCommand> TestImport(const objects::CSeq_annot& orig_annot, TFeatSeqIDMap& ftbl_seqid_map,
        unsigned int &startingLocusTagNumber, unsigned int &startingFeatureId, const string &locus_tag, bool euk);

    // show informative message about importing feature tables
    static void ShowTableInfo();

private:
    // attach all features to the nucleotide sequence, 
    // even the ones whose location do not fall within the sequence
    CIRef<IEditCommand> x_DoImportFeaturesFromFile(const wxString& workDir, const string &format_id = kEmptyStr);
    CIRef<IEditCommand> x_DoImportFeaturesFromClipboard(const wxString& workDir);

    CIRef<IEditCommand> x_CreateCommand( const IObjectLoader::TObjects& objects);
    
    // translate coding regions whose location falls within the sequence
    // attach coding regions to the existing set or seq level
    // returns true if it is translated, false otherwise
    CIRef<IEditCommand> x_DoImportCDS(CRef<objects::CSeq_feat> feat);

    CIRef<IEditCommand> x_GatherAdditionalChanges(CIRef<IEditCommand>& start_cmd);

    objects::CBioseq_Handle x_FindLocalBioseq_Handle(const objects::CSeq_feat& feat, const objects::CTSE_Handle& tseh) const;
    objects::CSeq_id_Handle x_GetFixedId(const objects::CSeq_id& id, const objects::CTSE_Handle& tseh) const;
    /// returns true if the feature table contains at least one pair of duplicate protein/transcript ids
    bool x_ContainsDuplicateIds(const IObjectLoader::TObjects& objects);

    // returns a message if there are coding regions with and without protein_id qualifier
    string x_CheckConsistentProteinIds(const IObjectLoader::TObjects& objects);

    void xPostProcessAnnot(objects::CSeq_annot& annot, unsigned int &startingLocusTagNumber, unsigned int &startingFeatureId, const string &locus_tag, bool euk);

    CImportFeatTable(const CImportFeatTable&);
    CImportFeatTable& operator=(const CImportFeatTable&);

private:
    objects::CSeq_entry_Handle m_Seh;
    // map to store the next available offset used in creating protein seq-id for a given bioseq
    map<string, int> m_OffsetForId;

    // message about feature tables not added to the top seq_entry
    string m_Error;
    bool m_is_gff;
};

END_NCBI_SCOPE

#endif //_GUI_PACKAGES_IMPORT_FEAT_TABLE_INPUT_HPP_
