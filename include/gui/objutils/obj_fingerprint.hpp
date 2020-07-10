#ifndef GUI_OBJUTILS___OBJ_FINGERPRINT__HPP
#define GUI_OBJUTILS___OBJ_FINGERPRINT__HPP

/*  $Id: obj_fingerprint.hpp 41036 2018-05-11 13:30:34Z rudnev $
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
 * Authors: Roman Katargin, Victor Joukov
 *
 * File Description:
 *
 */

#include <gui/gui_export.h>
#include <serial/typeinfo.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects) // namespace ncbi::objects::
class CScope;
class CSeq_feat;
class CSeq_align;
class CSeq_loc;
class CSeq_graph;
class CSeq_id_Handle;
class CSeq_annot_Handle;
END_SCOPE(objects)

class CChecksum;

class NCBI_GUIOBJUTILS_EXPORT CObjFingerprint
{
public:
    typedef void (*TFingerPFunc)(CChecksum& checksum, const CObject& obj,
                                 objects::CScope* scope, const CObjFingerprint& fingerP);

    static void Register(TTypeInfo type_info, TFingerPFunc func);

    static void GetFingerprint(CChecksum& checksum, const CObject& obj,
                               objects::CScope* scope = NULL);

    void Fingerprint(CChecksum& checksum, const CObject& obj, objects::CScope* scope) const;

    static void AddInteger(CChecksum& checksum, unsigned char* pvalue, int size);
    static void AddDouble(CChecksum& checksum, double value);

    // object signatures
    // AAAAAAAA-BBBBBBBB-CCCCCCCC-DDEE-FFFFFFFF-GGGGGGGG[-HHHHHHHH]
    //
    // where:
    // AAAAAAAA = id str as Fasta string,
    //            can be in any form of id and with varied length
    // BBBBBBBB-CCCCCCCC = total bounded range (from-to) of object
    // DD       = type code for object type (alignment, feature, graph, subsequence, etc.)
    //      01 - Feature (CSeq_feat)
    //      02 - Alignment (CSeq_align)
    //      03 - Location (CSeq_loc)
    //      04 - Graph (CSeq_graph)
    // EE       = subtype code for object (i.e., feature subtype)
    // FFFFFFFF = object fingerprint
    // GGGGGGGG = hash code for the data source, i.e., annotation's name. (optional)
    //            For named annotation accession, we use the real annotation name
    //            For remote file URLs, we use the real url
    //            instead of hash code
    // HHHHHHHH = hash code of sAdditionalInfo (currently: filter that was used to create the track where that feature is located;
    //            this helps to distinguish features when several tracks are created from the same data source, but using different filters)

    enum EObjectType {
        eUnknown   = 0,
        eFeature   = 1,
        eAlignment = 2,
        eLocation  = 3,
        eGraph     = 4,
        eSnpTrack  = 5,
        eVarTrack  = 6,
        eComponent = 7,
        eRsite     = 8
    };

    static string GetFeatSignature(const objects::CSeq_feat& feat,
                                   objects::CScope*          scope,
                                   const string& data_source = "",
                                   const string& sAdditionalInfo = "");

    static string GetFeatSignature(const objects::CSeq_feat& feat,
                                   objects::CScope*          scope,
                                   const objects::CSeq_annot_Handle& annot_handle,
                                   const string& sAdditionalInfo = "");

    /// Get signature for a feature with a separate location.
    static string GetFeatSignature(const objects::CSeq_feat& feat,
                                   const objects::CSeq_loc&  loc,
                                   objects::CScope*          scope,
                                   const objects::CSeq_annot_Handle& annot_handle,
                                   const string& sAdditionalInfo = "");

    /// Get signature for a feature with a separate location.
    static string GetFeatSignature(const objects::CSeq_feat& feat,
                                   const objects::CSeq_loc&  loc,
                                   objects::CScope*          scope,
                                   const string& data_source = "",
                                   const string& sAdditionalInfo = "");

    static string GetGraphSignature(const objects::CSeq_graph& graph,
                                    objects::CScope*           scope,
                                    const string& data_source = "");

    static string GetSeqLocSignature(const objects::CSeq_loc& loc,
                                     objects::CScope*         scope);

    static string GetHistogramSignature(const objects::CSeq_loc& loc,
                                        EObjectType obj_type,
                                        objects::CScope*   scope,
                                        const string& title,
                                        const string& data_source = "",
                                        int subtype = 0);

    static string GetComponentSignature(const objects::CSeq_loc& comp_loc,
                                        const objects::CSeq_loc& mapped_loc,
                                        objects::CScope*         scope);

    static string GetAlignSignature(const objects::CSeq_align& align,
                                    const objects::CSeq_loc&   on_loc,
                                    objects::CScope*           scope,
                                    const objects::CSeq_annot_Handle& annot_handle);

    static string GetAlignSignature(const objects::CSeq_align& align,
                                    const objects::CSeq_loc&   on_loc,
                                    objects::CScope*           scope,
                                    const string& data_source = "");

    static string GetRsiteSignature(const objects::CSeq_feat& feat,
                                    int pnt,
                                    objects::CScope* scope);

    /// parse signature with data source info.
    /// data source can either be a checksum or ds name.
    static void ParseSignature(const string&    sig,
                               objects::CSeq_id_Handle&  sih,
                               TSeqPos&         from,
                               TSeqPos&         to,
                               EObjectType&     type,
                               int&             subtype,
                               Uint4&           fingerprint,
                               Uint4&           ds_fingerprint,
                               string&          ds_name,
                               objects::CScope* scope);

    // get a data source part from the signature
    static string ParseSignatureDS(const string&    sig);

private:
    static CObjFingerprint& x_GetInstance();

    void x_Register(TTypeInfo type_info, TFingerPFunc func);

    typedef map<TTypeInfo, TFingerPFunc> TMap;
    TMap m_Map;
};

END_NCBI_SCOPE


#endif  // GUI_OBJUTILS___OBJ_FINGERPRINT__HPP
