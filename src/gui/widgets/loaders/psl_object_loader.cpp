/*  $Id: psl_object_loader.cpp 44439 2019-12-18 19:51:52Z katargir $
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

#include <gui/widgets/loaders/psl_object_loader.hpp>

#include <objects/seqset/Seq_entry.hpp>
#include <objtools/readers/psl_reader.hpp>

#include <objects/seq/Seq_annot.hpp>

#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CPslObjectLoader::CPslObjectLoader(const CPslLoadParams& params, const vector<wxString>& filenames)
 : m_Params(params), m_FileNames(filenames)
{
}

IObjectLoader::TObjects& CPslObjectLoader::GetObjects()
{
    return m_Objects;
}

string CPslObjectLoader::GetDescription() const
{
    return "Loading PSL Alignmemnt Files";
}

bool CPslObjectLoader::PreExecute()
{
    return true;
}

bool CPslObjectLoader::Execute(ICanceled& canceled)
{
    if (m_Params.GetMapAssembly().GetUseMapping()) {
        x_CreateMapper(m_Params.GetMapAssembly().GetAssemblyAcc());
    }

    for (const auto& fn : m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        CRef<CErrorContainer> errCont;

        wxFileName fileName(fn);
        string annot_name = "File: " + ToStdString(fileName.GetFullName());

        try {
            CCompressedFile file(fn);
            CNcbiIstream& istream = file.GetIstream();
            CStreamLineReader lineReader(istream);

            errCont.Reset(new CErrorContainer(1000));
            CPslReader reader(CReaderBase::fNormal);

            for (size_t counter = 1;; ++counter) {
                if (canceled.IsCanceled())
                    return false;

                if (lineReader.AtEOF() || !istream.good() || istream.eof())
                    break;

                auto seq_annot = reader.ReadSeqAnnot(lineReader, errCont);
                if (!seq_annot)
                    continue;

                m_Objects.push_back(SObject(*seq_annot, annot_name + " (" + NStr::NumericToString(counter) + ")"));

                x_UpdateMap(*seq_annot);
            }

            x_UpdateHTMLResults(fn, errCont);
            errCont.Reset();
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

bool CPslObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("PSL Alignments import errors"));
    return x_ShowMappingDlg(m_Objects);
}

END_NCBI_SCOPE
