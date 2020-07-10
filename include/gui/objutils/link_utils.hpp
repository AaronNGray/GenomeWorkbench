#ifndef LINK_UTILS__HPP
#define LINK_UTILS__HPP
/*  $Id: link_utils.hpp 41725 2018-09-14 21:07:12Z evgeniev $
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
 * Author:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>

BEGIN_NCBI_SCOPE


BEGIN_SCOPE(objects)
class CSeq_id_Handle;
class CSeq_feat;
class CSeq_loc;
class CSeq_align;
class CDbtag;
struct SAnnotSelector;
END_SCOPE(objects)


/////////////////////////////////////////////////////////////////////////////
//  CLinkUtils
//

class CLinkUtils
{
public:
    typedef map<string, vector<pair<string, string>>> TLinks;
    typedef map<string, TLinks> TLinksMap;
    
    static void AddSequenceLinks(const objects::CSeq_id_Handle& idh,
                                 const string& tax_id_file,
                                 objects::CScope& scope,
                                 TLinksMap& links,
                                 TSeqPos from = 0,
                                 TSeqPos to = 0,
                                 bool relative = true);

    static void AddFeatureLinks(const objects::CSeq_feat& feat,
                                const objects::CBioseq_Handle& handle,
                                const string& tax_id_file,
                                TLinksMap& links,
                                bool relative = true);

    static void AddFeatureLinks(const objects::CSeq_feat& feat,
                                const objects::CSeq_loc& loc,
                                const string& tax_id_file,
                                objects::CScope& scope,
                                TLinksMap& links,
                                bool relative = true);

    static void AddBlastProductLink(const objects::CSeq_feat& feat,
                                    objects::CScope& scope,
                                    TLinksMap& links,
                                    bool relative);

    static void AddDbxrefFeatureLinks(const objects::CSeq_feat& feat,
                                const objects::CBioseq_Handle& handle,
                                TLinksMap& links,
                                bool add_parent_gene_dbxrefs = true,
                                bool relative = true);

    static void AddDbxrefFeatureLinks(const objects::CSeq_feat& feat,
                                const objects::CSeq_loc& loc,
                                objects::CScope& scope,
                                TLinksMap& links,
                                bool add_parent_gene_dbxrefs = true,
                                bool relative = true);

    static void AddDbxrefLinksForSNP(const objects::CSeq_feat& feat,
                                     const CRef<objects::CDbtag>& dbt,
                                     objects::CScope& scope,
                                     TLinksMap& links,
                                     bool relative = true);

    static void AddLink(TLinksMap& links,
                        const string& type, const string& name,
                        const string& label, const string& url,
                        bool relative = true)
        {
            x_AddLink(links, type, name, label, url, relative);
        }

private:
    // helper for adding a link to the list
    static void x_AddLink(TLinksMap& links,
                          const string& type, const string& name,
                          const string& label, const string& url,
                          bool relative = true);

    static void x_AddDbxrefFeatureLinks(const objects::CSeq_feat& feat,
                                        const objects::CSeq_loc& loc,
                                        objects::CScope& scope,
                                        objects::CBioseq_Handle& bsh,
                                        int tax_id,
                                        TLinksMap& links,
                                        bool add_parent_gene_dbxrefs = true,
                                        bool relative = true);

private:
    CLinkUtils() {}
    ~CLinkUtils() {}

};


END_NCBI_SCOPE

#endif //LINK_UTILS__HPP
