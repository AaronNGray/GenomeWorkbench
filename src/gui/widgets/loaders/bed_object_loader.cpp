/*  $Id: bed_object_loader.cpp 35155 2016-03-31 13:30:22Z katargir $
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
* Authors:  Josh Cherry, Roman Katargin
*/


#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/bed_object_loader.hpp>

#include <util/line_reader.hpp>
#include <util/icanceled.hpp>

#include <objtools/readers/reader_base.hpp>
#include <objtools/readers/bed_reader.hpp>

#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>

#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filename.h>

#include <objmgr/object_manager.hpp>
#include <gui/objutils/label.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CBedObjectLoader::CBedObjectLoader(const CBedLoadParams& params, const vector<wxString>& filenames)
 : m_Params(params), m_FileNames(filenames)
{
}

IObjectLoader::TObjects& CBedObjectLoader::GetObjects()
{
    return m_Objects;
}

string CBedObjectLoader::GetDescription() const
{
    return "Loading BED Files";
}

bool CBedObjectLoader::PreExecute()
{
    return true;
}

bool CBedObjectLoader::Execute(ICanceled& canceled)
{
    typedef CRef<objects::CSeq_annot> TAnnotRef;

    if (m_Params.GetMapAssembly().GetUseMapping()) {
        x_CreateMapper(m_Params.GetMapAssembly().GetAssemblyAcc());
    }

    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();

    ITERATE(vector<wxString>, it, m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        m_CurrentFile = *it;
        CRef<CErrorContainer> errCont;

        try {
            CBedReader reader(CBedReader::fThreeFeatFormat);
            reader.SetCanceler(&canceled);    

            ITERATE(vector<wxString>, it, m_FileNames) {
                m_CurrentFile = *it;

                const wxString& fn = *it;
                CCompressedFile file(fn);
                CStreamLineReader lineReader(file.GetIstream());

                CReaderBase::TAnnots annots;
                errCont.Reset(new CErrorContainer((size_t)m_Params.GetNumErrors()));
                reader.ReadSeqAnnots(annots, lineReader, errCont);
                x_UpdateHTMLResults(m_CurrentFile, errCont);
                errCont.Reset();

                int index = 1;
                NON_CONST_ITERATE(CReaderBase::TAnnots, a_iter, annots) {
                    TAnnotRef& annot = *a_iter;

                    string label;

                    bool has_name = false;
                    if (annot->CanGetDesc()) {
                        const CAnnot_descr& desc = annot->GetDesc();
                        if (desc.CanGet()) {
                            const CAnnot_descr::Tdata& data = desc.Get();
                            CSeq_annot::TDesc::Tdata::const_iterator desc_iter = data.begin();
                            for ( ;  desc_iter != data.end();  ++desc_iter) {
                                if ((*desc_iter)->IsTitle() ) {
                                    label = (*desc_iter)->GetTitle();
                                    break;
                                }
                                if ((*desc_iter)->IsName() ) {
                                    label = (*desc_iter)->GetName();
                                    has_name = true;
                                }
                            }
                        }
                    }

                    string fileName = ToStdString(wxFileName(m_CurrentFile).GetFullName());

                    if (label.empty())
                        CLabel::GetLabel(*annot, &label, CLabel::eDefault, &*scope);

                    if (label.empty()) {
                        label = fileName;
                        if (annots.size() > 1) {
                            CNcbiOstrstream ostr;
                            ostr << " (BED Seq-annot " << index << ")";
                            label += CNcbiOstrstreamToString(ostr);
                        }
                    }

                    if (!has_name)
                        annot->SetNameDesc(label);

                    m_Objects.push_back(SObject(*annot, label, "File: " + fileName));

                    x_UpdateMap(*annot);

                    ++index;
                }
            }
        }
        catch (const CException& e) {
            x_UpdateHTMLResults(m_CurrentFile, errCont, e.GetMsg());
        }
        catch (const exception& e) {
            x_UpdateHTMLResults(m_CurrentFile, errCont, e.what());
        }
    }

    return true;
}

bool CBedObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("BED import errors"));
    return x_ShowMappingDlg(m_Objects);
}

END_NCBI_SCOPE
