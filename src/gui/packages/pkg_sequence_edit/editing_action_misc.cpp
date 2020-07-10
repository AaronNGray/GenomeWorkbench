/*  $Id: editing_action_misc.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objects/seq/seq_macros.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/object_manager.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>
#include <objtools/format/flat_expt.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_misc.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CEditingActionFlatFile::CEditingActionFlatFile(CSeq_entry_Handle seh, const string &name)
    : IEditingAction(seh, name)
{
}

void CEditingActionFlatFile::Find(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        CBioseq_Handle bsh = *b_iter; 
        m_CurrentSeqEntry = bsh.GetSeq_entry_Handle();
        if (m_constraint->Match(bsh))
        {     
                m_flat_file.clear();
                CConstRef<CBioseq> bioseq = bsh.GetBioseqCore();
                if (bioseq) 
                {
                    // generate flat file without the features and sequence for the bioseq
                    try {
                        CFlatFileConfig cfg;
                        cfg.SetFormatGenbank();
                        cfg.SetModeGBench();
                        cfg.SetStyleNormal();
                        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_Sequence);
                        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_Basecount);
                        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_FeatAndGap);
                        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_Segment);
                        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_Featheader);
                        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_Origin);
                        CRef<CFlatFileGenerator> ffgen(new CFlatFileGenerator(cfg));
                        CNcbiOstrstream ostr;
                        ffgen->Generate(*bioseq, bsh.GetScope(), ostr);
                        m_flat_file =  (IsOssEmpty(ostr)) ? kEmptyStr : CNcbiOstrstreamToString(ostr);
                    }
                    catch (const CFlatException& e) {
                        LOG_POST("Failed to generate flat file: " << e.GetMsg());
                        m_flat_file.clear();
                    }
                    Modify(action);              
                }
        }
        ++count;
        if (count >= m_max_records)
            break;   
    }
}

void CEditingActionFlatFile::FindRelated(EActionType action)
{
    Find(action); 
}

void CEditingActionFlatFile::Modify(EActionType action)
{
    Action(action);    
}

bool CEditingActionFlatFile::SameObject()
{
    CEditingActionFlatFile *other = dynamic_cast<CEditingActionFlatFile*>(m_Other);
    if (other)
        return true;
    return false;
}

void CEditingActionFlatFile::SwapContext(IEditingAction* source)
{
    CEditingActionFlatFile *ff_source = dynamic_cast<CEditingActionFlatFile*>(source);
    if (ff_source)
    {
        swap(m_flat_file, ff_source->m_flat_file);
    }
    IEditingAction::SwapContext(source);
}

bool CEditingActionFlatFile::IsSetValue()
{
    return !m_flat_file.empty();
}

void CEditingActionFlatFile::SetValue(const string &value)
{
}

string CEditingActionFlatFile::GetValue()
{
    return m_flat_file;
}

void CEditingActionFlatFile::ResetValue()
{
}


IEditingAction* CreateActionMisc(CSeq_entry_Handle seh, const string &field)
{
    if (field == "GenBank FlatFile")
        return new CEditingActionFlatFile(seh);   

    return NULL;
}



END_NCBI_SCOPE

