#ifndef GUI_OBJECTS___ASSEMBLY_INFO_HPP
#define GUI_OBJECTS___ASSEMBLY_INFO_HPP

/*  $Id: assembly_info.hpp 42230 2019-01-16 21:48:22Z shkeda $
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
 *   A utility class providing service for assembly information retrieval.
 * The target output is one or a list of related *full* assemblies.
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/objects/DL_AssmInfo.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/gui_export.h>

#include <set>

BEGIN_NCBI_SCOPE

class ICanceled;

class NCBI_GUIOBJECTS_EXPORT CAssemblyInfo
{
public:
    static CRef<objects::CDL_AssmInfo> GetAssms_Gi(TGi gi,
                                            const string& release_type = NcbiEmptyString);
    static CRef<objects::CDL_AssmInfo> GetAssms_TaxId(const string& tax_id,
                                               const string& release_type = NcbiEmptyString);
    static CRef<objects::CDL_AssmInfo> GetAssms_Term(const string& term,
                                              const string& release_type = NcbiEmptyString, ICanceled* cancel = 0);
    
    /// Check if assembly name (e.g. hg19, grch37) matches CGC_Assembly
    static bool IsSameAssembly(const objects::CGC_Assembly& gc_assm, const string& term);                                             

private:
    /// validate if the given tax-id.
    static bool x_ValidateTaxId(const string& tax_id);
    /// filter out the tax ids that don't have assembly on them.
    static void x_FilterTaxIds(CSeqUtils::TUids &uids);
    /// search all related links in the given db with a search term.
    static void x_SearchIds(const string& term, const string& db, CSeqUtils::TUids &uids);
    /// get detailed assembly information for a list of gencoll ids.
    static void x_GetAssmInfo(const CSeqUtils::TUids &uids, objects::CDL_AssmInfo::TAssemblies& assms);

    /// filter the assemblies by release type and category=full-assembly.
    static void x_FilterAssemblies(objects::CDL_AssmInfo::TAssemblies& assms,
                            const string& release_type);
    /// sort assemblies based on release date.
    /// apply special rules for human, mouse, cow and rat.
    static void x_SortAssms(objects::CDL_AssmInfo::TAssemblies& assms);
    /// convert a list of ids into a comma-delimited string
    static string x_CreateIdStr(const CSeqUtils::TUids &uids);

    static CRef<objects::CDL_AssmInfo> x_CreateAssmInfo(objects::CDL_AssmInfo::TAssemblies& assms);
};


END_NCBI_SCOPE

#endif // GUI_OBJECTS___ASSEMBLY_INFO_HPP
