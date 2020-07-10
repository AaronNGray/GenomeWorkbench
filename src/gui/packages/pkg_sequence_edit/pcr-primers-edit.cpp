/*  $Id: pcr-primers-edit.cpp 42303 2019-01-30 16:05:06Z asztalos $
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
#include <gui/objutils/descriptor_change.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/packages/pkg_sequence_edit/pcr-primers-edit.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> CPCRPrimerTrimJunk::GetCommand(objects::CSeq_entry_Handle tse)
{
    return x_GetCommand(tse, "Trim Junk from Primer Seqs");
}

bool CPCRPrimerTrimJunk::x_ApplyToBioSource (CBioSource& biosource)
{
    bool modified = false;

    EDIT_EACH_PCRREACTION_IN_PCRREACTIONSET(reaction, biosource.SetPcr_primers())
    {
        EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer, (*reaction)->SetForward())
        {
            if ((*primer)->IsSetSeq())
                modified |= CPCRPrimerSeq::TrimJunk((*primer)->SetSeq().Set());
        }
        EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer, (*reaction)->SetReverse())
        {
            if ((*primer)->IsSetSeq())
                modified |= CPCRPrimerSeq::TrimJunk((*primer)->SetSeq().Set());
        }
    }
    return modified;
}


CRef<CCmdComposite> CPCRPrimerFixI::GetCommand(objects::CSeq_entry_Handle tse)
{
    return x_GetCommand(tse, "Fix i In Primer Seqs");
}

bool CPCRPrimerFixI::x_ApplyToBioSource(CBioSource& biosource)
{
    bool modified = false;

    EDIT_EACH_PCRREACTION_IN_PCRREACTIONSET(reaction, biosource.SetPcr_primers())
    {
        EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer, (*reaction)->SetForward())
        {
            if ((*primer)->IsSetSeq())
                modified |= CPCRPrimerSeq::Fixi((*primer)->SetSeq().Set());
        }
        EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer, (*reaction)->SetReverse())
        {
            if ((*primer)->IsSetSeq())
                modified |= CPCRPrimerSeq::Fixi((*primer)->SetSeq().Set());
        }
    }
    return modified;
}   


CRef<CCmdComposite> CSwapPrimerNameSeq::GetCommand(objects::CSeq_entry_Handle tse)
{
    return x_GetCommand(tse, "Swap Primer Seq and Name");
}
    
bool CSwapPrimerNameSeq::x_ApplyToBioSource(CBioSource& biosource)
{
    bool modified = false;
    
    EDIT_EACH_PCRREACTION_IN_PCRREACTIONSET(reaction,biosource.SetPcr_primers())
    {
        EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer,(*reaction)->SetForward())
        {
            if ((*primer)->IsSetSeq() && (*primer)->IsSetName())
            {
                string seq = (*primer)->SetSeq().Set();
                string name = (*primer)->SetName().Set();
                (*primer)->SetSeq().Set() = name;
                (*primer)->SetName().Set() = seq;
                modified = true;
            }
        }
        EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer,(*reaction)->SetReverse())
        {
            if ((*primer)->IsSetSeq() && (*primer)->IsSetName())
            {
                string seq = (*primer)->SetSeq().Set();
                string name = (*primer)->SetName().Set();
                (*primer)->SetSeq().Set() = name;
                (*primer)->SetName().Set() = seq;
                modified = true;
            }
        }
    }
    return modified;
}


CRef<CCmdComposite> CMergePrimerSets::GetCommand(objects::CSeq_entry_Handle tse)
{
    return x_GetCommand(tse, "Merge Primer Sets");
}

bool CMergePrimerSets::x_ApplyToBioSource(CBioSource& biosource)
{
    bool modified = false;
    
    int num_reaction = 0;
    vector< CRef<CPCRPrimer> > forward, reverse;
    EDIT_EACH_PCRREACTION_IN_PCRREACTIONSET(reaction,biosource.SetPcr_primers())
    {
        if (num_reaction > 0)
        {
            EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer,(*reaction)->SetForward())
            {
                forward.push_back(*primer);
            }
            EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer,(*reaction)->SetReverse())
            {
                reverse.push_back(*primer);
            }
            ERASE_PCRREACTION_IN_PCRREACTIONSET(reaction,biosource.SetPcr_primers());
            modified = true;
        }
            
        num_reaction++;
    }

    for (vector< CRef<CPCRPrimer> >::iterator p = forward.begin(); p != forward.end(); ++p)
        biosource.SetPcr_primers().Set().front()->SetForward().Set().push_back(*p);
    for (vector< CRef< CPCRPrimer> >::iterator p = reverse.begin(); p != reverse.end(); ++p)
        biosource.SetPcr_primers().Set().front()->SetReverse().Set().push_back(*p);
    
    return modified;
}

CRef<CCmdComposite> CSplitPrimerSets::GetCommand(objects::CSeq_entry_Handle tse)
{
    return x_GetCommand(tse, "Split Primer Sets By Position");
}

bool CSplitPrimerSets::x_ApplyToBioSource(CBioSource& biosource)
{
    bool modified = false;
   
    vector< CRef<CPCRPrimer> > forward, reverse;
    EDIT_EACH_PCRREACTION_IN_PCRREACTIONSET(reaction,biosource.SetPcr_primers())
    {
        int num_primer = 0;   
        EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer,(*reaction)->SetForward())
        {
            if (num_primer > 0)
            {
                forward.push_back(*primer);
                ERASE_PCRPRIMER_IN_PCRPRIMERSET(primer, (*reaction)->SetForward());
                modified = true;
            }
            num_primer++;
        }
        num_primer = 0;
        EDIT_EACH_PCRPRIMER_IN_PCRPRIMERSET(primer,(*reaction)->SetReverse())
        {
            if (num_primer > 0)
            {
                reverse.push_back(*primer);
                ERASE_PCRPRIMER_IN_PCRPRIMERSET(primer, (*reaction)->SetReverse());
                modified = true;
            }
            num_primer++;
        }
    }
    if (forward.size() == reverse.size())
        for (unsigned int i = 0; i < forward.size(); i++)
        {
            CRef< CPCRReaction > reaction(new CPCRReaction);
            reaction->SetForward().Set().push_back(forward[i]);
            reaction->SetReverse().Set().push_back(reverse[i]);
            biosource.SetPcr_primers().Set().push_back(reaction);
        }
    
    return modified;
}

END_NCBI_SCOPE
