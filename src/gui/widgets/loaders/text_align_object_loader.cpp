/*  $Id: text_align_object_loader.cpp 43812 2019-09-05 15:41:33Z katargir $
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
* Authors:  Mike DiCuccio, Roman Katargin
*/


#include <ncbi_pch.hpp>

#include <util/icanceled.hpp>

#include <gui/widgets/loaders/text_align_object_loader.hpp>

#include <objtools/readers/aln_reader.hpp>
#include <objtools/readers/reader_exception.hpp>
#include <objtools/readers/idmapper.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <serial/iterator.hpp>

#include <gui/objutils/label.hpp>

#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/wx_utils.hpp>


#include <wx/filename.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>

#include <gui/widgets/loaders/job_results_dlg.hpp>
#include <gui/objutils/text_aln_reader.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CTextAlignObjectLoader::CTextAlignObjectLoader(const CTextAlignParams& params)
: m_Params(params)
{
}

CTextAlignObjectLoader::CTextAlignObjectLoader(const CTextAlignParams& params, const vector<wxString>& filenames)
 : m_Params(params), m_FileNames(filenames)
{
}

IObjectLoader::TObjects& CTextAlignObjectLoader::GetObjects()
{
    return m_Objects;
}

string CTextAlignObjectLoader::GetDescription() const
{
    return "Loading Text Alignment Files";
}

bool CTextAlignObjectLoader::PreExecute()
{
    return true;
}

bool CTextAlignObjectLoader::Execute(ICanceled& canceled)
{
    Init();

    ITERATE(vector<wxString>, it, m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        const wxString& fn = *it;
        wxFileName fileName(fn);
        string annot_name = "File: " + ToStdString(fileName.GetFullName());

        try {
            CCompressedFile file(fn);
            TObjects objects; string errMsg;
            LoadFromStream(file.GetIstream(), objects, errMsg);

            if (!errMsg.empty())
                x_UpdateHTMLResults(fn, 0, kEmptyStr, errMsg);

            for (auto& o : objects) {
                string desc = annot_name + " : " + o.GetDescription();
                m_Objects.push_back(SObject(*o.GetObjectPtr(), desc));
            }
        }
        catch (const CException& e) {
            x_UpdateHTMLResults(fn, 0, e.GetMsg());
        }
        catch (const exception& e) {
            x_UpdateHTMLResults(fn, 0, e.what());
        }
    }

    return true;
}

bool CTextAlignObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("Text Alignment import errors"));
    return true;
}

void CTextAlignObjectLoader::Init()
{
    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    m_Scope.Reset(new CScope(*obj_mgr));
    m_Scope->AddDefaults();
}

void CTextAlignObjectLoader::LoadFromStream(CNcbiIstream& istr, TObjects& objects, string& errMsg)
{
    _ASSERT(m_Scope);

    CMessageListenerCount errors(100);
    CTextAlnReader reader(*m_Scope, istr);

    try {
        reader.SetMissing(ToStdString(m_Params.GetUnknown()));
        reader.SetMatch(ToStdString(m_Params.GetMatch()));
        reader.SetBeginningGap(ToStdString(m_Params.GetBegin()));
        reader.SetMiddleGap(ToStdString(m_Params.GetMiddle()));
        reader.SetEndGap(ToStdString(m_Params.GetEnd()));

        if (m_Params.GetSeqType() == 1)
            reader.SetAlphabet(CAlnReader::eAlpha_Nucleotide);
        else if (m_Params.GetSeqType() == 2)
            reader.SetAlphabet(CAlnReader::eAlpha_Protein);

        reader.Read(0, &errors);

        CRef<objects::CSeq_entry> entry = reader.GetFilteredSeqEntry();

        if (!entry || !m_Scope) return;

        for (CTypeIterator<CSeq_align> it(*entry); it; ++it) {
            CRef<CSeq_annot> annot(new CSeq_annot());
            annot->SetData().SetAlign().push_back(CRef<CSeq_align>(&*it));

            string label;
            if (m_Scope) {
                CLabel::GetLabel(*annot, &label, CLabel::eDefault, m_Scope);
            }

            objects.push_back(SObject(*annot, label));
        }

        if (entry->IsSet()  &&  !(entry->GetSet().GetSeq_set().empty())) {
            CRef<CSeq_entry> seq_set_entry(new CSeq_entry);
            seq_set_entry->SetSet().SetSeq_set() = entry->SetSet().SetSeq_set();
            string label;
            if (m_Scope) {
                CLabel::GetLabel(*seq_set_entry, &label, CLabel::eDefault, m_Scope);
            }

            objects.push_back(SObject(*seq_set_entry, label));
        }

    }
    catch (CObjReaderParseException& _DEBUG_ARG(e)) {
        _TRACE("clustal nucleotide failed: " << e.what());
    }

    for (size_t i = 0; i < errors.Count(); ++i) {
        if (!NStr::IsBlank (errMsg))
            errMsg += "\n";
        errMsg += errors.GetError(i).GetText();
    }
}


END_NCBI_SCOPE
