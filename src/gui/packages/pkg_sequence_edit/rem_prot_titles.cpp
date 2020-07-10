/*  $Id: rem_prot_titles.cpp 40250 2018-01-17 17:31:45Z filippov $
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
#include <gui/packages/pkg_sequence_edit/rem_prot_titles.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

bool CRemProtTitles::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
{
    if (tse) 
    {
        CRef<CCmdComposite> composite( new CCmdComposite(title) );   
        CBioseq_CI bi(tse, CSeq_inst::eMol_aa);
        while (bi) 
        {
            CBioseq_Handle bsh = *bi;
            if (bsh.GetParentBioseq_set() && bsh.GetParentBioseq_set().IsSetClass() && bsh.GetParentBioseq_set().GetClass() == CBioseq_set::eClass_nuc_prot && bsh.IsSetId())
            {
                bool found = false;
                for (vector<CSeq_id_Handle>::const_iterator id = bsh.GetId().begin(); id != bsh.GetId().end(); ++id)
                    if (id->GetSeqId()->Which() == CSeq_id::e_Other)
                    {
                        found = true;
                        break;
                    }
                if (!found)
                {
                    CSeqdesc_CI desc(bsh,CSeqdesc::e_Title,1);
                    while (desc)
                    {
                        CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(bsh.GetSeq_entry_Handle(), *desc));
                        composite->AddCommand(*cmdDelDesc);
                        ++desc;
                    }
                }
            }
            ++bi;
        }
        cmdProcessor->Execute(composite.GetPointer());
        return true;
    }
    else
        return false;
}

bool CRemAllProtTitles::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor)
{
    if (tse) 
    {
        CRef<CCmdComposite> composite( new CCmdComposite("Remove RefSeq Protein Titles") );   
        CBioseq_CI bi(tse, CSeq_inst::eMol_aa);
        while (bi) 
        {
            CBioseq_Handle bsh = *bi;
            if (bsh.GetParentBioseq_set() && bsh.GetParentBioseq_set().IsSetClass() && bsh.GetParentBioseq_set().GetClass() == CBioseq_set::eClass_nuc_prot)
            {
		CSeqdesc_CI desc(bsh,CSeqdesc::e_Title,1);
		while (desc)
		{
		    CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(bsh.GetSeq_entry_Handle(), *desc));
		    composite->AddCommand(*cmdDelDesc);
		    ++desc;
		}
            }
            ++bi;
        }
        cmdProcessor->Execute(composite.GetPointer());
        return true;
    }
    else
        return false;
}


END_NCBI_SCOPE
