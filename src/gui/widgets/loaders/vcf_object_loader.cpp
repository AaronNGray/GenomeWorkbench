/*  $Id: vcf_object_loader.cpp 38296 2017-04-24 16:17:37Z katargir $
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

#include <gui/widgets/loaders/vcf_object_loader.hpp>

#include <util/line_reader.hpp>
#include <util/icanceled.hpp>

#include <objtools/readers/reader_base.hpp>
#include <objtools/readers/vcf_reader.hpp>

#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>

#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CVcfObjectLoader::CVcfObjectLoader(const CBedLoadParams& params, const vector<wxString>& filenames)
 : m_Params(params), m_FileNames(filenames)
{
}

IObjectLoader::TObjects& CVcfObjectLoader::GetObjects()
{
    return m_Objects;
}

string CVcfObjectLoader::GetDescription() const
{
    return "Loading VCF (Variant Call Format) Files";
}

bool CVcfObjectLoader::PreExecute()
{
    return true;
}

bool CVcfObjectLoader::Execute(ICanceled& canceled)
{
    if (m_Params.GetMapAssembly().GetUseMapping()) {
        x_CreateMapper(m_Params.GetMapAssembly().GetAssemblyAcc());
    }

    ITERATE(vector<wxString>, it, m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        const wxString& fn = *it;
        CRef<CErrorContainer> errCont;

        try {
            CCompressedFile file(fn);
            CStreamLineReader lineReader(file.GetIstream());

            CVcfReader reader;
            reader.SetCanceler(&canceled);
            errCont.Reset(new CErrorContainer((size_t)m_Params.GetNumErrors()));
            CRef<CSeq_annot> annot(reader.ReadSeqAnnot(lineReader, errCont));
            x_UpdateHTMLResults(fn, errCont);
            errCont.Reset();

            if (!annot) continue;

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

            if (!has_name) {
                wxFileName wx_fn(fn);
                string fileName = ToStdString(wx_fn.GetFullName());
                if (label.empty())
                    label = fileName;
                annot->SetNameDesc(fileName);
            }

            m_Objects.push_back(SObject(*annot, label));

            x_UpdateMap(*annot);
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

bool CVcfObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("VCF import errors"));
    return x_ShowMappingDlg(m_Objects);
}

END_NCBI_SCOPE
