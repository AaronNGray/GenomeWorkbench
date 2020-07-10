#ifndef _GUI_PACKAGES_UPDATE_SEQ_PARAMS_HPP_
#define _GUI_PACKAGES_UPDATE_SEQ_PARAMS_HPP_
/*  $Id: update_seq_params.hpp 37873 2017-02-24 22:12:47Z asztalos $
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
 *  Structure holding information about how to perform the sequence update
 */

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE

/// Stores parameters regarding the type of sequence update, on how to handle existing features and
/// on how to handle imported features
///
class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT SUpdateSeqParams
{
public:
    enum ESequenceUpdateType {
        eSeqUpdateNoChange = 0,  /// do not change the old sequence
        eSeqUpdateReplace,       /// replace old sequence with update sequence
        eSeqUpdatePatch,         /// patch old sequence with update in the aligned region
        eSeqUpdateExtend5,       /// extend the 5' end of old sequence with the update
        eSeqUpdateExtend3,       /// extend the 3' end of old sequence with the update 
        CountSeqUpdateType
    };

    enum EFeatRemoveType {
        eFeatRemoveNone = 0,
        eFeatRemoveAligned,
        eFeatRemoveNotAligned,
        eFeatRemoveAll,
        CountFeatRemoveType
    };

    enum EFeatUpdateType {
        eFeatUpdateAllExceptDups = 0,
        eFeatUpdateAllMergeDups,
        eFeatUpdateAllReplaceDups,
        eFeatUpdateAll,
        CountFeatUpdateType
    };

    enum EProteinUpdateType {
        eProtUpdate          = (1 << 0), ///< retranslate coding regions and update the proteins
        eProtUpdateTruncate  = (1 << 1), ///< truncate retranslated proteins at stops
        eProtUpdateExtend5   = (1 << 2), ///< extend retranslated proteins without starts
        eProtUpdateExtend3   = (1 << 3), ///< extend retranslated proteins without stops
        CountUpdateProteinType
    };
    typedef int TProtUpdFlags;   ///< bitwise OR of "EProteinUpdateType"

    ESequenceUpdateType m_SeqUpdateOption;
    EFeatRemoveType m_FeatRemoveOption;
    bool m_IgnoreAlignment;

    /// Flag to indicate that features from the update sequence will be imported
    bool m_ImportFeatures;
    /// Defines the imported feature subtype
    objects::CSeqFeatData::ESubtype m_FeatImportType;  
    EFeatUpdateType m_FeatImportOption; 
    TProtUpdFlags m_UpdateProteins;
    /// flag to update protein IDs, default is to update (false) - relevant only to imported protein IDs
    bool m_KeepProteinId;
    /// flag to attach a citation with the current date (false)
    bool m_AddCitSub;

    SUpdateSeqParams(ESequenceUpdateType seq_update = eSeqUpdateNoChange, bool ignore_align = false)
        : m_SeqUpdateOption(seq_update),
        m_FeatRemoveOption(eFeatRemoveNone),
        m_IgnoreAlignment(ignore_align),
        m_ImportFeatures(false),
        m_FeatImportType(objects::CSeqFeatData::eSubtype_any),
        m_FeatImportOption(eFeatUpdateAll),
        m_UpdateProteins(0),
        m_KeepProteinId(false),
        m_AddCitSub(false) {}
    
    bool AreUpdateParamsValid(void);
};


END_NCBI_SCOPE

#endif
    // _GUI_PACKAGES_UPDATE_SEQ_PARAMS_HPP_