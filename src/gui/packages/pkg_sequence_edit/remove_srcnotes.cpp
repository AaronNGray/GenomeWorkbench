/*  $Id: remove_srcnotes.cpp 42188 2019-01-09 21:48:14Z asztalos $
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
#include <gui/packages/pkg_sequence_edit/remove_srcnotes.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> CRemoveSrcNotes::GetCommand(CSeq_entry_Handle tse)
{
    return x_GetCommand(tse, "Remove SrcNotes");
}

bool CRemoveSrcNotes::x_ApplyToBioSource(CBioSource& biosource)
{
    bool modified = false;
    EDIT_EACH_ORGMOD_ON_BIOSOURCE(orgmod, biosource)
    {
        if ((*orgmod)->IsSetSubtype() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_other)
        {
            ERASE_ORGMOD_ON_BIOSOURCE(orgmod, biosource);
            modified = true;
        }
    }

    EDIT_EACH_SUBSOURCE_ON_BIOSOURCE(subsource, biosource)
    {
        if ((*subsource)->IsSetSubtype() && (*subsource)->GetSubtype() == CSubSource::eSubtype_other)
        {
            ERASE_SUBSOURCE_ON_BIOSOURCE(subsource, biosource);
            modified = true;
        }
    }
    return modified;
}


CRef<CCmdComposite> CConsolidateLikeMods::GetCommand(CSeq_entry_Handle tse)
{
    return x_GetCommand(tse, "Consolidate like modifiers");
}

bool CConsolidateLikeMods::x_ApplyToBioSource(CBioSource& biosource)
{
    map<int, vector<string> > orgmods;
    FOR_EACH_ORGMOD_ON_BIOSOURCE(orgmod, biosource)
    {
        if ((*orgmod)->IsSetSubtype() && (*orgmod)->IsSetSubname() && 
            (*orgmod)->GetSubtype() != COrgMod::eSubtype_specimen_voucher &&
            (*orgmod)->GetSubtype() != COrgMod::eSubtype_culture_collection && 
            (*orgmod)->GetSubtype() != COrgMod::eSubtype_bio_material)
        {
            orgmods[(*orgmod)->GetSubtype()].push_back((*orgmod)->GetSubname());
        }
    }

    map<int, vector<string> > subsources;
    FOR_EACH_SUBSOURCE_ON_BIOSOURCE(subsource, biosource)
    {
        if ((*subsource)->IsSetSubtype() && (*subsource)->IsSetName())
        {
            subsources[(*subsource)->GetSubtype()].push_back((*subsource)->GetName());
        }
    }

    bool modified = false;
    EDIT_EACH_ORGMOD_ON_BIOSOURCE(orgmod, biosource)
    {
        if ((*orgmod)->IsSetSubtype() && 
            (*orgmod)->IsSetSubname() && 
            orgmods.find((*orgmod)->GetSubtype()) != orgmods.end() && 
            orgmods[(*orgmod)->GetSubtype()].size() > 1 )
        {
            if ((*orgmod)->GetSubname() == orgmods[(*orgmod)->GetSubtype()].front())
            {
                (*orgmod)->SetSubname(NStr::Join(orgmods[(*orgmod)->GetSubtype()], m_delimiter));
            }
            else
            {
                ERASE_ORGMOD_ON_BIOSOURCE(orgmod, biosource);
            }
            modified = true;
        }
    }

    EDIT_EACH_SUBSOURCE_ON_BIOSOURCE(subsource, biosource)
    {
        if ((*subsource)->IsSetSubtype() &&
            (*subsource)->IsSetName() && 
            subsources.find((*subsource)->GetSubtype()) != subsources.end() && 
            subsources[(*subsource)->GetSubtype()].size() > 1 )
        {
            if ((*subsource)->GetName() == subsources[(*subsource)->GetSubtype()].front())
            {
                (*subsource)->SetName(NStr::Join(subsources[(*subsource)->GetSubtype()], m_delimiter));
            }
            else
            {
                ERASE_SUBSOURCE_ON_BIOSOURCE(subsource, biosource);
            }
            modified = true;
        }
    }
    
    return modified;
}

END_NCBI_SCOPE

