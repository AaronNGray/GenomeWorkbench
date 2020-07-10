/*  $Id: rm_object_loader.cpp 35156 2016-03-31 13:34:10Z katargir $
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

#include <gui/widgets/loaders/rm_object_loader.hpp>

#include <util/icanceled.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/compressed_file.hpp>

#include <gui/objutils/label.hpp>

#include <objmgr/object_manager.hpp>

#include <objtools/readers/rm_reader.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRMObjectLoader::CRMObjectLoader(const vector<wxString>& filenames)
 : m_FileNames(filenames)
{
}

IObjectLoader::TObjects& CRMObjectLoader::GetObjects()
{
    return m_Objects;
}

string CRMObjectLoader::GetDescription() const
{
    return "Loading RepeatMasker Output Files";
}

bool CRMObjectLoader::PreExecute()
{
    return true;
}

bool CRMObjectLoader::Execute(ICanceled& canceled)
{
    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    CRef<CScope> scope(new CScope(*obj_mgr));
    scope->AddDefaults();

    ITERATE(vector<wxString>, it, m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        CRef<CErrorContainer> errCont;
        const wxString& fn = *it;
        
        try {
            CCompressedFile file(fn);

            CRepeatMaskerReader reader(CRepeatMaskerReader::fDefaults);
            reader.SetCanceler(&canceled);
            errCont.Reset(new CErrorContainer(100));
            CRef<CSeq_annot> annot(reader.ReadSeqAnnot(file.GetIstream(), errCont.GetPointer()));
            x_UpdateHTMLResults(fn, errCont);
            errCont.Reset();

            string label;
            CLabel::GetLabel(*annot, &label, CLabel::eDefault, scope);
            m_Objects.push_back(SObject(*annot, label));
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

bool CRMObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("RepeatMasker import errors"));
    return true;
}

END_NCBI_SCOPE
