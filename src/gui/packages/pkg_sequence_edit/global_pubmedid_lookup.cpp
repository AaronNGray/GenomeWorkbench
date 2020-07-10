/*  $Id: global_pubmedid_lookup.cpp 42186 2019-01-09 19:34:50Z asztalos $
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
#include <objects/pub/Pub_equiv.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/packages/pkg_sequence_edit/global_pubmedid_lookup.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> CGlobalPubmedIdLookup::apply(objects::CSeq_entry_Handle tse)
{
    if (!tse)
        return CRef<CCmdComposite>(NULL);

    CRef<CCmdComposite> cmd( new CCmdComposite("Global PubmedId Lookup") ); 
    bool modified = false;

    for (CSeq_entry_CI set_ci(tse, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry); set_ci ; ++set_ci)
    {
        for (CSeqdesc_CI seqdesc(*set_ci, CSeqdesc::e_Pub, 1); seqdesc; ++seqdesc)
        {

            CRef<CSeqdesc> new_desc(new CSeqdesc());
            new_desc->Assign(*seqdesc);
            CRef<CPubdesc> pubdesc;
            if (new_desc->GetPub().IsSetPub() && new_desc->GetPub().GetPub().IsSet())
            {
                for (CPub_equiv::Tdata::iterator pub = new_desc->SetPub().SetPub().Set().begin(); pub != new_desc->SetPub().SetPub().Set().end(); ++pub)
                {
                    if ((*pub)->IsPmid())
                    {
                        int pmid = (*pub)->GetPmid();
                        pubdesc = GetPubdescFromEntrezById(pmid);
                        if (pubdesc)
                            break;
                    }
                }         
            }    
            if (pubdesc)
            {
                new_desc->SetPub(*pubdesc);
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(seqdesc.GetSeq_entry_Handle(), *seqdesc, *new_desc));
                cmd->AddCommand (*ecmd);
                modified = true;
            }
        }
    }
    
    if (!modified)
        cmd.Reset();
    return cmd;
}


END_NCBI_SCOPE
