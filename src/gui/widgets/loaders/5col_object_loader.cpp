/*  $Id: 5col_object_loader.cpp 41556 2018-08-21 14:48:12Z katargir $
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

#include <gui/widgets/loaders/5col_object_loader.hpp>

#include <objects/seqset/Seq_entry.hpp>
#include <objtools/readers/readfeat.hpp>

#include <objects/seq/Seq_annot.hpp>

#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

C5ColObjectLoader::C5ColObjectLoader(const vector<wxString>& filenames)
 : m_FileNames(filenames)
{
}

IObjectLoader::TObjects& C5ColObjectLoader::GetObjects()
{
    return m_Objects;
}

string C5ColObjectLoader::GetDescription() const
{
    return "Loading 5 Column Feature Files";
}

bool C5ColObjectLoader::PreExecute()
{
    return true;
}

bool C5ColObjectLoader::Execute(ICanceled& canceled)
{
    for (const auto& fn : m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        CRef<CErrorContainer> errCont;

        wxFileName fileName(fn);
        string annot_name = "File: " + ToStdString(fileName.GetFullName());

        try {
            CCompressedFile file(fn);
            CNcbiIstream& istream = file.GetIstream();
            CStreamLineReader reader(istream);

            errCont.Reset(new CErrorContainer(1000));
            CFeature_table_reader ftable_reader(reader, errCont.GetPointer());

            for (size_t counter = 1;; ++counter) {
                if (canceled.IsCanceled())
                    return false;

                if (reader.AtEOF() || !istream.good() || istream.eof())
                    break;

                auto seq_annot = ftable_reader.ReadSequinFeatureTable(CFeature_table_reader::fReportBadKey);
                if (!seq_annot || !seq_annot->IsFtable() || seq_annot->GetData().GetFtable().empty())
                    continue;

                m_Objects.push_back(SObject(*seq_annot, annot_name + " (" + NStr::NumericToString(counter) + ")"));
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

bool C5ColObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("5 Column Feature import errors"));
    return true;
}

END_NCBI_SCOPE
