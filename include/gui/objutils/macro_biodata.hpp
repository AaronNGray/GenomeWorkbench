#ifndef GUI_OBJUTILS___MACRO_BIODATA__HPP
#define GUI_OBJUTILS___MACRO_BIODATA__HPP
/*  $Id: macro_biodata.hpp 45047 2020-05-15 19:30:14Z asztalos $
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
 * Authors: Andrea Asztalos
 *
 * File Description: Biological data for macro execution
 *
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/macro_biodata_iter.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

/// The following asn-selectors are defined to be used in the FOR EACH statement:
/// - Seq
/// - SeqNA
/// - SeqAA
/// - SeqSet
/// - Seqdesc
/// - BioSource
/// - MolInfo
/// - UserObject
/// - StructComment
/// - DBLink
/// - Pubdesc
/// - SeqFeat
/// - Gene
/// - Cdregion
/// - Protein
/// - MiscFeat
/// - ImpFeat
/// - RNA
/// - rRNA
/// - mRNA
/// - miscRNA
/// - SNP
/// - SeqAlign
/// - TSEntry

///////////////////////////////////////////////////////////////////////////////
/// CMacroBioData - stores the top seq-entry (and the optional seq-submit) used 
/// as input data for the macro. It also creates the FOR EACH iterator.
///
class NCBI_GUIOBJUTILS_EXPORT CMacroBioData
{
public:
    static const char* sm_Seq;
    static const char* sm_SeqNa;
    static const char* sm_SeqAa;
    static const char* sm_SeqSet;

    static const char* sm_BioSource;
    static const char* sm_MolInfo;
    static const char* sm_Pubdesc;
    static const char* sm_UserObject;
    static const char* sm_DBLink;
    static const char* sm_StrComm;
    static const char* sm_Seqdesc;

    static const char* sm_SeqFeat;
    static const char* sm_Gene;
    static const char* sm_CdRegion;
    static const char* sm_Protein;
    static const char* sm_Miscfeat;
    static const char* sm_ImpFeat;
    static const char* sm_RNA;
    static const char* sm_rRNA;
    static const char* sm_mRNA;
    static const char* sm_miscRNA;
    static const char* sm_SNP;

    static const char* sm_SeqAlign;
    static const char* sm_TSEntry;
    
    CMacroBioData(const objects::CSeq_entry_Handle& entry, CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>());
    ~CMacroBioData(){}
    
    bool IsSetSeqSubmit( void ) const { return m_SeqSubmit.NotEmpty(); }
    const objects::CSeq_submit& GetSeqSubmit( void ) const { return *m_SeqSubmit; }
    objects::CSeq_entry_Handle GetTSE() const { return m_TopSeqEntry; }

    /// Create specific iterator as a subclass of IMacroBioDataIter.
    ///
    /// @param selector
    ///   String that defines the type of iterator, should be one of static members from above
    /// @param named_annot
    ///   The NA accession for feature retrieval
    /// @param range
    ///   Sequence range
    /// @param ostr
    ///   Output stream for printing
    /// @return
    ///   Pointer to new iterator or nullptr
    /// @note User is responsible for deleting the created iterator
    ///
    IMacroBioDataIter* CreateIterator(const string& selector,
        const string& named_annot, const TSeqRange& range, CNcbiOstream* ostr = nullptr) const;

    /// Create specific iterator as a subclass of IMacroBioDataIter.
    ///
    /// @param selector
    ///   String that defines the type of iterator, should be one of static members from above
    /// @param named_annot
    ///   The NA accession for feature retrieval
    /// @param feat_interval
    ///   Feature interval specified by a sequence range and the type of boundaries
    /// @param ostr
    ///   Output stream for printing
    /// @return
    ///   Pointer to new iterator or nullptr
    /// @note User is responsible for deleting the created iterator. This function is intended to be used in
    /// parallel runs
    IMacroBioDataIter* CreateIterator(const string& selector,
        const string& named_annot, const SFeatInterval& feat_interval, CNcbiOstream* ostr = nullptr) const;

    static bool s_IsFeatSelector(const string& selector);

private:
    IMacroBioDataIter* x_CreateNonFeatIterator(const string& selector) const;
    IMacroBioDataIter* x_CreateFeatIterator(const string& selector, const string& named_annot, const TSeqRange& range) const;
    IMacroBioDataIter* x_CreateFeatIterator(const string& selector, const string& named_annot, const SFeatInterval& feat_interval) const;

    objects::SAnnotSelector x_GetAnnotSelector(const string& selector, const string& named_annot) const;

    objects::CSeq_entry_Handle m_TopSeqEntry;
    CConstRef<objects::CSeq_submit> m_SeqSubmit; 

    // prohibit copying and assigning
    CMacroBioData(const CMacroBioData&);
    CMacroBioData& operator=(const CMacroBioData&);
};

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_BIODATA__HPP
