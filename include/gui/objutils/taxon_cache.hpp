#ifndef GUI_OBJUTILS___TAXON_CACHE__HPP
#define GUI_OBJUTILS___TAXON_CACHE__HPP

/*  $Id: taxon_cache.hpp 37498 2017-01-14 01:39:56Z evgeniev $
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
 * Authors:  Roman Katargin, Anatoliy Kuznetsov
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/taxon1/taxon1.hpp>

BEGIN_NCBI_SCOPE


class NCBI_GUIOBJUTILS_EXPORT CTaxonCache : public CObject
{
public:
    static void InitStorage(const char* cache_path);

    static CTaxonCache& GetInstance();

    void Initialize(const vector<int>& tax_ids);
    string GetLabel(int tax_id);
    string GetTaxname(int tax_id);
    string GetCommon(int tax_id);
    string GetBlastName(int tax_id);

    /// Drop connection to NCBI taxon service
    void ResetConnection();

private:
    struct STaxonomy
    {
        std::string Label;
        std::string TaxName;
        std::string CommonName;
        std::string BlastName;
    };

    typedef map<int, STaxonomy> TMap;

private:
    /// forbidden
    CTaxonCache();
    ~CTaxonCache();
    CTaxonCache(const CTaxonCache&);
    CTaxonCache& operator=(const CTaxonCache&);

    // internal (not thread-protected)
    void x_QueryTaxonService(int tax_id);
    TMap::const_iterator x_GetItem(int tax_id);

    void x_SaveTaxonCache() const;
    void x_LoadTaxonCache();

private:
    TMap                        m_Map;
    auto_ptr<objects::CTaxon1>  m_TaxonClient;

    static string               m_CachePath;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___TAXON_CACHE__HPP
