/*  $Id: load_secondary_accessions.cpp 42186 2019-01-09 19:34:50Z asztalos $
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
#include <util/line_reader.hpp>

#include <objects/seqblock/GB_block.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>   
#include <objects/seq/Seq_hist.hpp>
#include <objects/seq/Seq_hist_rec.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>

#include <objtools/readers/reader_exception.hpp>
#include <objtools/readers/struct_cmt_reader.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/packages/pkg_sequence_edit/load_secondary_accessions.hpp>

#include <wx/filedlg.h>
#include <wx/msgdlg.h> 

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> CLoadSecondaryAccessions::apply(wxWindow *parent, CScope &scope, bool history_takeover)
{
    wxFileDialog file(parent, wxT("Load secondary accessions from file"), wxEmptyString, wxEmptyString,
                      CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (file.ShowModal() != wxID_OK) {
return CRef<CCmdComposite>(NULL);
    }
    map<CBioseq_Handle, vector<string> > secondaries;
    wxString path = file.GetPath();
    CNcbiIfstream istr(path.fn_str());   
    CStreamLineReader line_reader(istr); 
    do {
        string str = *++line_reader; 
        NStr::TruncateSpacesInPlace(str);
        if (str.empty())
            continue;
        list<string> row_values;
        NStr::Split(str, "\t", row_values, NStr::fSplit_MergeDelimiters);
        if (row_values.size() == 2) {
            const string &id_str = row_values.front();
            const string &secondary = row_values.back();
            if (id_str.empty() || secondary.empty())
                continue;
            CRef<CSeq_id> id;
            try
            {
                id.Reset(new CSeq_id(id_str, CSeq_id::fParse_Default));
            } catch(const exception&) {}
            if (!id)
                continue;
            CBioseq_Handle bsh = scope.GetBioseqHandle(*id);
            if (!bsh)
                continue;
            secondaries[bsh].push_back(secondary);
        }
    } while (!line_reader.AtEOF());

    if (secondaries.empty())
        return CRef<CCmdComposite>(NULL);

    CRef<CCmdComposite> cmd(new CCmdComposite("Load Secondary Accessions"));
    for (const auto & s : secondaries)
    {
        CBioseq_Handle bsh = s.first;
        const vector<string> &vec_sec = s.second;

        CSeqdesc_CI desc_iter(bsh, CSeqdesc::e_Genbank);
        if (desc_iter) 
        {
            CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
            new_desc->Assign(*desc_iter);
            CSeqdesc::TGenbank &new_genbank = new_desc->SetGenbank();
            for (const auto& acc : vec_sec)
                new_genbank.SetExtra_accessions().push_back(acc);

            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_iter.GetSeq_entry_Handle(), *desc_iter, *new_desc));
            cmd->AddCommand (*ecmd);
        } 
        else 
        {
            CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
            CSeqdesc::TGenbank &new_genbank = new_desc->SetGenbank();
            for (const auto& acc : vec_sec)
                new_genbank.SetExtra_accessions().push_back(acc);
            cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetSeq_entry_Handle(), *new_desc)) );
        }

        if (history_takeover)
        {
            CRef<CSeq_inst> new_inst(new CSeq_inst());
            if (bsh.GetBioseqCore()->IsSetInst())
                new_inst->Assign(bsh.GetBioseqCore()->GetInst());    
            new_inst->SetHist().ResetReplaces();
            for (const auto& acc : vec_sec)
            {
                CRef<CSeq_id> id(new CSeq_id(acc, CSeq_id::fParse_Default));
                new_inst->SetHist().SetReplaces().SetIds().push_back(id);
            }
        
            if (bsh.GetBioseqCore()->IsSetInst())
            {
                const CSeq_inst &inst = bsh.GetBioseqCore()->GetInst();
                if (inst.IsSetHist() && inst.GetHist().IsSetReplaces() && inst.GetHist().GetReplaces().IsSetIds())
                    for (CSeq_hist_rec::TIds::const_iterator i = inst.GetHist().GetReplaces().GetIds().begin(); i != inst.GetHist().GetReplaces().GetIds().end(); ++i)
                    {
                        CRef<CSeq_id> id(new CSeq_id());
                        id->Assign(**i);
                        new_inst->SetHist().SetReplaces().SetIds().push_back(id);
                    }
            }
            cmd->AddCommand(*CRef<CCmdChangeBioseqInst>(new CCmdChangeBioseqInst(bsh, *new_inst)));
        }
    }
    return cmd;
}


END_NCBI_SCOPE
