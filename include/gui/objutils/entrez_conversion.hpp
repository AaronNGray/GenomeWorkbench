#ifndef GUI_OBJUTILS___ENTREZ_CONVERSION__HPP
#define GUI_OBJUTILS___ENTREZ_CONVERSION__HPP

/*  $Id: entrez_conversion.hpp 33488 2015-08-03 20:34:48Z evgeniev $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/entrezgene/Entrezgene_Set.hpp>
#include <objects/entrezgene/Entrezgene.hpp>
#include <misc/xmlwrapp/xmlwrapp.hpp>



BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CXmlNodeObject : public CObject {
public:
    CXmlNodeObject(const xml::node &xml_node, const std::string& db_name) :
        m_DBName(db_name)
    {
        m_Node.assign(xml_node);
    }
    const xml::node& GetNode() const
    {
        return m_Node;
    }
    const std::string& GetDBName() const
    {
        return m_DBName;
    }
private:
    xml::node   m_Node;
    std::string m_DBName;
};

///////////////////////////////////////////////////////////////////////////////
/// CEntrezDB
class NCBI_GUIOBJUTILS_EXPORT   CEntrezDB {
public:
    typedef pair<string, string>    TNamePair;

    static void     GetDbNames(vector<string>& names);
    static void     GetDbNames(vector<TNamePair>& name_pairs);
    static string   GetVisibleName(const string& db_name);
    static string   GetDbName(const string& visible_name);

    static void Query(const string &db_name,
                  const string &terms,
                  size_t &total_uids,
                  xml::document &docsums,
                  size_t max_return = 0);


    static CRef<objects::CSeq_annot>
            CreateAnnot_Nuc_Prot(const string& query_str,
                                 const xml::node_set& ds_set);

    static CRef<objects::CSeq_id>
            CreateId_Nuc_Prot(const xml::node& ds);

    static CRef<objects::CEntrezgene> CreateGene_Gene(const xml::node& ds);

    static CRef<objects::CEntrezgene_Set>
            CreateGeneSet_Gene(const xml::node_set& ds_set);

    static int GetUidAttr(const xml::node& ds);
};


END_NCBI_SCOPE

#endif  /// GUI_OBJUTILS___ENTREZ_CONVERSION__HPP
