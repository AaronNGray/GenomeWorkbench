/* $Id: label_rna.cpp 36403 2016-09-21 19:57:50Z filippov $
 * ===========================================================================
 *
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
 * File:  label RNA
 *  
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbistd.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <wx/filename.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include <objtools/edit/seqid_guesser.hpp>
#include <objtools/edit/rna_edit.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/label_rna.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

void CLabelRna :: AddNewMrnaCommand(CRef <CCmdComposite> composite, CBioseq_Handle bsh, CRef <CSeq_feat> new_mrna, bool negative)
{
    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    composite->AddCommand(*(CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_mrna))));   
    if (negative)
    {
        CRef <CCmdComposite> cmd = GetReverseComplimentSequenceCommand(bsh);
        composite->AddCommand(*cmd);
    }
} 

    
string CLabelRna :: GetGenBankId(CBioseq_Handle bsh)
{
    string accession;
    string tmsmart;
    if (!bsh)  
        return accession;   
    
    ITERATE(CBioseq::TId, it, bsh.GetCompleteBioseq()->GetId()) 
    {
        if ((*it)->IsGenbank() && (*it)->GetGenbank().IsSetAccession()) 
        {
            accession = (*it)->GetGenbank().GetAccession();
        }
        if ((*it)->IsGeneral() && (*it)->GetGeneral().IsSetDb() && (*it)->GetGeneral().GetDb() == "TMSMART" && (*it)->GetGeneral().IsSetTag())
        {
            if ((*it)->GetGeneral().GetTag().IsStr())
                tmsmart = (*it)->GetGeneral().GetTag().GetStr();
            else if ((*it)->GetGeneral().GetTag().IsId())
                tmsmart = NStr::IntToString((*it)->GetGeneral().GetTag().GetId());
        }
    }
    if (!accession.empty())
        return accession;
    return tmsmart;
}

void CLabelRna::PrepareInputFile(CSeq_entry_Handle tse, CNcbiOfstream &ostr)
{
    CBioseq_CI bi(tse, CSeq_inst::eMol_na);
    while (bi) 
    {
        string acc = GetGenBankId(*bi);
        if (!acc.empty())
        {
            ostr << acc << endl;
        }
        ++bi;
    }
}

bool CLabelRna::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title, wxWindow *parent)
{
    if (!tse) return false;

    wxString tmpIn     = wxFileName::CreateTempFileName(wxT("in"));
    CNcbiOfstream ostr(tmpIn.mb_str());
 
    PrepareInputFile(tse,ostr);

    wxString tmpOut     = wxFileName::CreateTempFileName(wxT("out"));

#ifdef NCBI_OS_MSWIN
    wxString script = _("\\\\snowman\\win-coremake\\App\\Ncbi\\smart\\bin\\cmd_sequin_fetch_FindITS.bat");
#else
    wxString script = _("/netopt/genbank/subtool/bin/cmd_sequin_fetch_FindITS");
#endif

    wxString cmdline = script+_(" -i ")+tmpIn+_(" -o ") + tmpOut;
    if (!wxFileName::FileExists(script)) 
    {
       wxMessageBox(_("Path to FindITS executable does not exist"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
       return true;
    }


    long pid = ::wxExecute(cmdline, wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE);

    if (pid != 0)
    {
        wxMessageBox(_("Bad return code from FindITS executable"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return true;
    }

    CRef<CCmdComposite> composite(new CCmdComposite(title)); 

    // read in results
    try
    {
        set<string> msgs;
        edit::CFindITSParser parser(tmpOut.mb_str(), tse);
        do 
        {           
            CRef <CSeq_feat> new_mrna = parser.ParseLine();
             if (new_mrna)
                 AddNewMrnaCommand(composite, parser.GetBSH(), new_mrna, parser.GetNegative());
             string msg(parser.GetMsg());
            if (!msg.empty())
                msgs.insert(msg);
        } while ( !parser.AtEOF() );

        wxString error(NStr::Join(msgs, "\n"));
        if (!error.IsEmpty())
        {
            CGenericReportDlg* report = new CGenericReportDlg(parent);
            report->SetTitle(wxT("Label RNA Error"));
            report->SetText(error);
            report->Show();
        }
    }
    catch(const CException&)
    {
        wxMessageBox(_("Unable to read Label RNA|ITS results"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return true;
    } 

    if (!tmpIn.empty()) 
    {
        ::wxRemoveFile(tmpIn);
    }

    if (!tmpOut.empty()) 
    {
        ::wxRemoveFile(tmpOut);
    }   

    cmdProcessor->Execute(composite.GetPointer());
    return true;
}          


END_NCBI_SCOPE
