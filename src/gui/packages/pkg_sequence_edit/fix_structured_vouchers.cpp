/*  $Id: fix_structured_vouchers.cpp 42189 2019-01-09 22:13:08Z asztalos $
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
 * Authors:  Igor Filippov
 */

#include <ncbi_pch.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/packages/pkg_sequence_edit/fix_structured_vouchers.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CRef<CCmdComposite> CFixStructuredVouchers::GetCommand(CSeq_entry_Handle tse)
{
    return x_GetCommand(tse, "Fix Structured Vouchers");
}

bool CFixStructuredVouchers::x_ApplyToBioSource(CBioSource& biosource)
{
    bool modified = false;
    EDIT_EACH_ORGMOD_ON_BIOSOURCE(orgmod, biosource)
    {
        if ((*orgmod)->IsSetSubtype() && (*orgmod)->IsSetSubname() &&
            ((*orgmod)->GetSubtype() == COrgMod::eSubtype_specimen_voucher
                || (*orgmod)->GetSubtype() == COrgMod::eSubtype_culture_collection
                || (*orgmod)->GetSubtype() == COrgMod::eSubtype_bio_material))
        {
            string voucher_type;
            if ((*orgmod)->GetSubtype() == COrgMod::eSubtype_specimen_voucher)
                voucher_type = "s";
            else if ((*orgmod)->GetSubtype() == COrgMod::eSubtype_culture_collection)
                voucher_type = "c";
            else if ((*orgmod)->GetSubtype() == COrgMod::eSubtype_bio_material)
                voucher_type = "b";
            string val = (*orgmod)->GetSubname();
            string old_val = val;
            COrgMod::FixStructuredVoucher(val, voucher_type);
            if (old_val != val)
            {
                (*orgmod)->SetSubname(val);
                modified = true;
            }
        }
    }
    return modified;
}

END_NCBI_SCOPE
