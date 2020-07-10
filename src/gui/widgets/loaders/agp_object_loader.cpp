/*  $Id: agp_object_loader.cpp 35142 2016-03-30 15:20:04Z katargir $
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
* Authors:  Roman Katargin
*/


#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>

#include <util/icanceled.hpp>

#include <gui/widgets/loaders/agp_object_loader.hpp>

#include <objmgr/object_manager.hpp>
#include <objtools/readers/agp_seq_entry.hpp>
#include <objtools/readers/fasta.hpp>

#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/label.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CAgpObjectLoader::CAgpObjectLoader(const CAgpLoadParams& params, const vector<wxString>& filenames)
 : m_Params(params), m_FileNames(filenames)
{
}

IObjectLoader::TObjects& CAgpObjectLoader::GetObjects()
{
    return m_Objects;
}

string CAgpObjectLoader::GetDescription() const
{
    return "Loading AGP Files";
}

bool CAgpObjectLoader::PreExecute()
{
    return true;
}

bool CAgpObjectLoader::Execute(ICanceled& canceled)
{
    wxString fasta_file = m_Params.GetFastaFile();

    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    CRef<CScope> scope(new CScope(*obj_mgr));
    scope->AddDefaults();

    CRef<CSeq_entry> fasta_seqs;
    if ( !fasta_file.empty() ) {
        CRef<CErrorContainer> errCont(new CErrorContainer(100));
        try {
            CCompressedFile file(fasta_file);
            CRef<ILineReader> line_reader(ILineReader::New(file.GetIstream()));
            CFastaReader reader(*line_reader, CFastaReader::fParseGaps);
            reader.SetCanceler(&canceled);
            fasta_seqs = reader.ReadSet(kMax_Int, errCont.GetPointer());
            x_UpdateHTMLResults(fasta_file, errCont);
        }
        catch (const CException& e) {
            x_UpdateHTMLResults(fasta_file, errCont, e.GetMsg());
        }
        catch (const exception& e) {
            x_UpdateHTMLResults(fasta_file, errCont, e.what());
        }
    }

    CAgpToSeqEntry::TFlags parse_flags = (m_Params.GetParseIDs() == 1) ? CAgpToSeqEntry::fForceLocalId : 0;
    parse_flags |= m_Params.GetSetGapInfo() ? CAgpToSeqEntry::fSetSeqGap : 0;


    ITERATE(vector<wxString>, it, m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        const wxString& fn = *it;

        try {
            CCompressedFile file(fn);

            vector<CRef<CSeq_entry> > entries;
            //--- AgpRead(*istr_p, entries, comp_id_rule, set_gap_info);

            CAgpToSeqEntry agpToSeqEntry( parse_flags );
            agpToSeqEntry.ReadStream( file.GetIstream() );
            // swap is faster than "="
            entries.swap( agpToSeqEntry.GetResult() );
            x_UpdateHTMLResults(fn, 0, kEmptyStr, agpToSeqEntry.GetErrorMessage());

            NON_CONST_ITERATE(vector<CRef<CSeq_entry> >, it2, entries) {
                CRef<CSeq_entry> &entry = *it2;

                string label;
                CLabel::GetLabel(*entry, &label, CLabel::eDefault, scope);
                m_Objects.push_back(SObject(*entry, label));
            }
        }
        catch (const CException& e) {
            x_UpdateHTMLResults(fn, 0, e.GetMsg());
        }
        catch (const exception& e) {
            x_UpdateHTMLResults(fn, 0, e.what());
        }
    }

    if (fasta_seqs) {
        string label;
        CLabel::GetLabel(*fasta_seqs, &label, CLabel::eDefault, scope);
        m_Objects.push_back(SObject(*fasta_seqs, label));
    }

    return true;
}

bool CAgpObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("AGP import errors"));
    return true;
}

END_NCBI_SCOPE
