/*  $Id: fasta_writer.hpp 559758 2018-03-14 15:35:07Z foleyjp $
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
 * Authors:  Sergiy Gotvyanskyy, Justin Foley
 *
 * File Description: Extend CFastaOstream to handle features. 
 *                   Write object as a hierarchy of FASTA objects
 *
 */

#ifndef __OBJTOOLS_WRITERS_FASTA_WRITER_HPP__
#define __OBJTOOLS_WRITERS_FASTA_WRITER_HPP__

#include <corelib/ncbistd.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/writers/writer.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects) 

class CSeq_entry;
class CBioseq;
class CSeq_loc;
class CBioseq_Handle;
class CSeq_entry_Handle;


class NCBI_XOBJWRITE_EXPORT CFastaOstreamEx 
: public CFeatWriter, public CFastaOstream
{
public:

    explicit CFastaOstreamEx(CNcbiOstream& out);
    virtual ~CFastaOstreamEx() = default;
   

    bool WriteFeatures(CFeat_CI feat_it, 
        bool translate_cds);

    void WriteFeature(const CSeq_feat& feat,
        CScope& scope,
        bool translate_cds=false);

    void WriteFeatureTitle(const CSeq_feat& feat,
        CScope& scope,
        bool translate_cds=false);

    void ResetFeatureCount(void);

protected:

    CRef<CSeq_loc> x_TrimLocation(TSeqPos frame,
        ENa_strand strand,
        CScope& scope,
        const CSeq_loc& loc);

    virtual bool xWriteFeature(CFeat_CI feat_it) override;

    bool xWriteFeatureTitle(const CSeq_feat& feat,
        CScope& scope,
        bool translate_cds=false);

    void x_WriteTranslatedCds(const CSeq_feat& cds,
        CScope& scope);

    void x_WriteFeatureAttributes(const CSeq_feat& feat, 
        CScope& scope) const;

    void x_AddGeneAttributes(const CSeq_feat& feat,
        CScope& scope,
        string& defline) const;

    void x_AddProteinNameAttribute(const CSeq_feat& feat,
        CScope& scope,
        string& defline) const;

    void x_AddDbxrefAttribute(const CSeq_feat& feat,
        CScope& scope,
        string& defline) const;

    void x_AddReadingFrameAttribute(const CSeq_feat& feat, 
        string& defline) const;

    void x_AddncRNAClassAttribute(const CSeq_feat& feat,
        string& defline) const;

    void x_AddPseudoAttribute(const CSeq_feat& feat,
        CScope& scope,
        string& defline) const;

    void x_AddPseudoGeneAttribute(const CSeq_feat& feat,
        CScope& scope,
        string& defline) const;

    void x_AddRNAProductAttribute(const CSeq_feat& feat,
        string& defline) const;

    void x_AddPartialAttribute(const CSeq_feat& feat, 
        CScope& scope,
        string& defline) const;

    void x_AddExceptionAttribute(const CSeq_feat& feat, 
        string& defline) const;

    void x_AddProteinIdAttribute(const CSeq_feat& feat,
        CScope& scope,
        string& defline) const;
    
    void x_AddTranscriptIdAttribute(const CSeq_feat& feat,
        CScope& scope,
        string& defline) const;

    void x_AddTranslationExceptionAttribute(const CSeq_feat& feat,
        CScope& scope,
        string& defline) const;

    bool x_GetCodeBreak(const CSeq_feat& feat, 
        const CCode_break& code_break,
        CScope& scope, 
        string& cbstring) const;

    void x_AddLocationAttribute(const CSeq_feat& feat,
        CScope& scope,
        string& defline) const;

    void x_AddDeflineAttribute(const string& label,
        const string& value,
        string& defline) const;

    void x_AddMiscQualifierAttributes(const CSeq_feat& feat,
        string& defline) const;

    void x_AddGBkeyAttribute(const CSeq_feat& feat, 
        string& defline) const;

    void x_AddDeflineAttribute(const string& label,
        bool value,
        string& defline) const;

    string x_GetCDSIdString(const CSeq_feat& cds,
        CScope& scope,
        bool translate_cds=false);

    string x_GetRNAIdString(const CSeq_feat& rna,
        CScope& scope);

    string x_GetProtIdString(const CSeq_feat& prot,
        CScope& scope);

    string x_GetGeneIdString(const CSeq_feat& gene,
        CScope& scope);

    string x_GetOtherIdString(const CSeq_feat& feat,
        CScope& scope);

    bool m_TranslateCds;
    TSeqPos m_FeatCount;
    CRef<CScope> m_InternalScope;
};


// this would generate CDS and genes into own files
// up to three files created: 
// filename_without_ext.fsa for nucleotide sequences
// filename_without_ext_cds_from_genomic.fna for CDS's
// filename_without_ext_rna_from_genomic.fna for RNA's
class /*NCBI_XOBJUTIL_EXPORT*/ CFastaOstreamComp
{
public:
    CFastaOstreamComp(const string& dir, const string& filename_without_ext);
    virtual ~CFastaOstreamComp();

    enum E_FileSection
    {
        eFS_nucleotide,
        eFS_CDS,
        eFS_RNA
    };
    typedef int TFlags; ///< binary OR of CFastaOstream::EFlags

    void Write(const CSeq_entry_Handle& handle, const CSeq_loc* location = 0);

    inline
    void SetFlags(TFlags flags)
    {
        m_Flags = flags;
    }

    inline
    TFlags GetFlags() const
    {
        return m_Flags;
    }
protected:
    struct TStreams
    {
        TStreams() : m_ostream(0), m_fasta_stream(0)
        {
        }
        string         m_filename;
        CNcbiOstream*  m_ostream;
        CFastaOstream* m_fasta_stream;
    };

    void x_Write(const CBioseq_Handle& handle, const CSeq_loc* location);
    // override these method to change filename policy and output stream
    virtual void x_GetNewFilename(string& filename, E_FileSection sel);
    virtual CNcbiOstream* x_GetOutputStream(const string& filename, E_FileSection sel);
    virtual CFastaOstream* x_GetFastaOstream(CNcbiOstream& ostr, E_FileSection sel);

    TStreams& x_GetStream(E_FileSection sel);
protected:
    string m_dir;
    string m_filename_without_ext;
    TFlags m_Flags;
    vector<TStreams> m_streams;
};

class NCBI_XOBJWRITE_EXPORT CQualScoreWriter
{
public:
    CQualScoreWriter(CNcbiOstream& ostr, 
                     bool enable_gi=false);

    virtual ~CQualScoreWriter(void);

    void Write(const CBioseq& bioseq);
private:
    bool x_WriteHeader(const CBioseq& bioseq);

    bool x_GetMaxMin(const vector<char>& values, int& max, int& min);

    string x_ComposeHeaderEnding(
        const string& graph_title,
        TSeqPos length,
        int max,
        int min);

    void x_Advance(int& column, const int num_columns);

    CNcbiOstream& m_Ostr;
    unique_ptr<CFastaOstreamEx> m_FastaOstr;
};

END_SCOPE(objects)
END_NCBI_SCOPE


#endif // end of "include-guard"
