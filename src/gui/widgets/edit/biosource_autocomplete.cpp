/*  $Id: biosource_autocomplete.cpp 42439 2019-02-26 17:58:10Z filippov $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <util/line_reader.hpp>

#include <objects/seqfeat/OrgName.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>

#include <gui/widgets/edit/biosource_autocomplete.hpp>



BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CBioSourceAutoComplete
 */


CBioSourceAutoComplete::CBioSourceAutoComplete()
{
}


CBioSourceAutoComplete::~CBioSourceAutoComplete()
{
}


bool CBioSourceAutoComplete::AutoFill (objects::COrg_ref& org)
{
    if (!org.IsSetTaxname() || NStr::IsBlank (org.GetTaxname()) || org.GetTaxId() != 0 || !org.TableLookup(org.GetTaxname())) {
        return false;
    }

    return org.UpdateFromTable();
}


END_NCBI_SCOPE
