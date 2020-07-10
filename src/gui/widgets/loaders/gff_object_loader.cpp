/*  $Id: gff_object_loader.cpp 44162 2019-11-08 02:15:57Z ucko $
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

#include <gui/objutils/label.hpp>
#include <gui/widgets/loaders/gff_object_loader.hpp>

#include <objtools/readers/gtf_reader.hpp>
#include <objtools/readers/gff3_reader.hpp>
#include <objtools/readers/gvf_reader.hpp>

#include <util/format_guess.hpp>

#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filename.h>

#include <objmgr/object_manager.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CGffObjectLoader::CGffObjectLoader(const CGffLoadParams& params, const vector<wxString>& filenames)
 : m_Params(params), m_FileNames(filenames)
{
}

IObjectLoader::TObjects& CGffObjectLoader::GetObjects()
{
    return m_Objects;
}

string CGffObjectLoader::GetDescription() const
{
    return "Loading GFF/GTF Files";
}

bool CGffObjectLoader::PreExecute()
{
    return true;
}

bool CGffObjectLoader::Execute(ICanceled& canceled)
{
    CReaderBase::TReaderFlags flags = 0;

    switch(m_Params.GetParseSeqIds()) {
    case 1:
        flags |= CReaderBase::fNumericIdsAsLocal;
        break;
    case 2:
        flags |= CReaderBase::fAllIdsAsLocal;
        break;
    }

    if (m_Params.GetMapAssembly().GetUseMapping()) {
        x_CreateMapper(m_Params.GetMapAssembly().GetAssemblyAcc());
    }

    string annot_name = ToStdString(m_Params.GetNameFeatSet());
    CRef<CScope> scope;
    if (annot_name.empty()) {
        // scope for CLabel::GetLabel
        scope.Reset(new CScope(*CObjectManager::GetInstance()));
        scope->AddDefaults();
    }

    ITERATE(vector<wxString>, it, m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        CRef<CErrorContainer> errCont;
        const wxString& fn = *it;

        try {
            CCompressedFile file(fn);

            CFormatGuess::EFormat fmt = CFormatGuess::eGff3;
            switch (m_Params.GetFileFormat()) {
            case 1 :
                fmt = CFormatGuess::eGff3;
                break;
            case 2 :
                fmt = CFormatGuess::eGtf;
                break;
            case 3 :
                fmt =  CFormatGuess::eGvf;
                break;
            default :
                fmt = file.GuessFormat();
                file.Reset();
                break;
            }

            CReaderBase::TAnnots annots;

            string fileName = ToStdString(wxFileName(fn).GetFullName());

            AutoPtr<CReaderBase> reader;
            if (fmt == CFormatGuess::eGff2) {
                NCBI_THROW(CException, eInvalid, "GFF reader: Unsupported format.");
            } else if (fmt == CFormatGuess::eGff3) { 
                reader.reset(new CGff3Reader(flags)); 
            } else if (fmt == CFormatGuess::eGtf) { 
                reader.reset(new CGtfReader(flags)); 
            } else if (fmt == CFormatGuess::eGvf) {
                reader.reset(new CGvfReader(flags));
            } 
            
            if (!reader) {
                x_UpdateHTMLResults(fn, errCont, NcbiEmptyString, "This file is not recognized as a supported GFF format.");
                continue;
            }
            reader->SetCanceler(&canceled);  
            errCont.Reset(new CErrorContainer(100));
            reader->ReadSeqAnnots(annots, file.GetIstream(), errCont.GetPointer());
            x_UpdateHTMLResults(fn, errCont);
            errCont.Reset();

            NON_CONST_ITERATE(CReaderBase::TAnnots, annot_iter, annots) {
                if (canceled.IsCanceled())
                    return false;

                string label;
                if (annot_name.empty())
                    CLabel::GetLabel(**annot_iter, &label, CLabel::eDefault, &*scope);
                else
                    label = annot_name;

                if (label.empty())
                    label = fileName;

                (*annot_iter)->SetNameDesc(label);
                m_Objects.push_back(SObject(**annot_iter, label, "File: " + fileName));

                x_UpdateMap(**annot_iter);
            }
        }
        catch (const CException& e) {
            x_UpdateHTMLResults(fn, errCont, e.GetMsg());
        }
        catch (const exception& e) {
            x_UpdateHTMLResults(fn, errCont, e.what());
        }
    }

    return true;
}

bool CGffObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("GFF/GTF import errors"));
    return x_ShowMappingDlg(m_Objects);
}

END_NCBI_SCOPE
