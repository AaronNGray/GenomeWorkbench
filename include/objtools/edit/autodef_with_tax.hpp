#ifndef OBJTOOLS_EDIT___AUTODEF_WITH_TAX__HPP
#define OBJTOOLS_EDIT___AUTODEF_WITH_TAX__HPP

/*  $Id: autodef_with_tax.hpp 530196 2017-03-13 12:59:43Z bollin $
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
* Author:  Colleen Bollin
*
* File Description:
*   Extends CAutodef to include docsum titles (which require a call to taxonomy)
*/

#include <corelib/ncbistd.hpp>
#include <objmgr/util/autodef.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)    


class NCBI_XOBJEDIT_EXPORT CAutoDefWithTaxonomy : public CAutoDef
{
public:
    CAutoDefWithTaxonomy() : CAutoDef() {};
    ~CAutoDefWithTaxonomy() {};

    static string GetDocsumOrgDescription(CSeq_entry_Handle se);
    string GetDocsumDefLine(CSeq_entry_Handle se);
    static bool RegeneratePopsetTitles(CSeq_entry_Handle se);
    static bool RegenerateDefLines(CSeq_entry_Handle se);

};


END_SCOPE(objects)
END_NCBI_SCOPE

#endif //OBJTOOLS_EDIT___AUTODEF_WITH_TAX__HPP
