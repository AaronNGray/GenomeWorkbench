/*  $Id: table_object_loader.cpp 28590 2013-08-02 20:34:31Z katargir $
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

#include <gui/widgets/loaders/table_object_loader.hpp>

#include <gui/widgets/loaders/table_annot_data_source.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CTableObjectLoader::CTableObjectLoader(CTableAnnotDataSource& annotTableData, const vector<wxString>& filenames)
{
    string annot_name;
    if (filenames.size() > 0) {
        wxFileName fileName(filenames[0]);
        annot_name = "File: " + ToStdString(fileName.GetFullName());
    }

    m_Objects.push_back(SObject(*annotTableData.GetContainer(), annot_name));
}

IObjectLoader::TObjects& CTableObjectLoader::GetObjects()
{
    return m_Objects;
}

string CTableObjectLoader::GetDescription() const
{
    return "Loading Table Files";
}

END_NCBI_SCOPE
