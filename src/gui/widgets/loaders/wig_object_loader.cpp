/*  $Id: wig_object_loader.cpp 39927 2017-11-27 18:54:17Z katargir $
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

#include <gui/widgets/loaders/wig_object_loader.hpp>
#include <util/line_reader.hpp>
#include <util/icanceled.hpp>

#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_single_data.hpp>

#include <objtools/readers/reader_base.hpp>
#include <objtools/readers/wiggle_reader.hpp>

#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>

#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const CSeq_id* s_GetAnnotId(const CSeq_annot& annot)
{
    if (annot.IsSeq_table()) {
        const CSeq_table& table = annot.GetData().GetSeq_table();
        ITERATE (CSeq_table::TColumns, iter, table.GetColumns()) {
            if ((*iter)->GetHeader().CanGetField_name()  &&
                (*iter)->GetHeader().GetField_name() == "Seq-table location"  &&
                (*iter)->CanGetDefault()  &&  (*iter)->GetDefault().IsLoc()) {
                const CSeq_loc& loc = (*iter)->GetDefault().GetLoc();
                return loc.GetId();
            }
        }
    }
    return 0;
}


CWigObjectLoader::CWigObjectLoader(const CWigLoadParams& params, const vector<wxString>& filenames)
 : m_Params(params), m_FileNames(filenames)
{
}

IObjectLoader::TObjects& CWigObjectLoader::GetObjects()
{
    return m_Objects;
}

string CWigObjectLoader::GetDescription() const
{
    return "Loading WIG Files";
}

bool CWigObjectLoader::PreExecute()
{
    return true;
}

bool CWigObjectLoader::Execute(ICanceled& canceled)
{
    typedef CRef<objects::CSeq_annot> TAnnotRef;

    if (m_Params.GetMapAssembly().GetUseMapping()) {
        x_CreateMapper(m_Params.GetMapAssembly().GetAssemblyAcc());
    }
    CRef<CScope> scope;

    ITERATE(vector<wxString>, it, m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        const wxString& fn = *it;
        CRef<CErrorContainer> errCont;

        try {
            CCompressedFile file(fn);
            CStreamLineReader lineReader(file.GetIstream());

            CWiggleReader reader;
            reader.SetCanceler(&canceled);
            CReaderBase::TAnnots annots;
            errCont.Reset(new CErrorContainer((size_t)m_Params.GetNumErrors()));
            reader.ReadSeqAnnots(annots, lineReader, errCont);
            x_UpdateHTMLResults(fn, errCont);
            errCont.Reset();

            string track_name;
            int index = 1;
            NON_CONST_ITERATE(CReaderBase::TAnnots, a_iter, annots) {
                TAnnotRef& annot = *a_iter;

                /// By default, Wiggle reader convert wig file into seq-tables.
                /// Usually, the original data are graphs, we need to mark them
                /// as 'graph'
                CRef<CUser_object> user_obj;
                if (annot->CanGetDesc()) {
                    CSeq_annot::TDesc::Tdata& descs = annot->SetDesc().Set();
                    CSeq_annot::TDesc::Tdata::iterator d_iter = descs.begin();
                    while (d_iter != descs.end()) {
                        if ((*d_iter)->IsUser()  &&  (*d_iter)->GetUser().GetType().IsStr()  &&
                            (*d_iter)->GetUser().GetType().GetStr() == "Track Data") {
                                user_obj.Reset(&(*d_iter)->SetUser());
                                break;
                        }
                        ++d_iter;
                    }
                }

                if ( !user_obj ) {
                    user_obj.Reset(new CUser_object);
                    user_obj->SetType().SetStr("Track Data");
                    annot->AddUserObject(*user_obj);
                }
                user_obj->AddField("track type", string("graph"));

                if (annot->CanGetDesc()) {
                    const CAnnot_descr& desc = annot->GetDesc();
                    if (desc.CanGet()) {
                        const CAnnot_descr::Tdata& data = desc.Get();
                        CSeq_annot::TDesc::Tdata::const_iterator desc_iter = data.begin();
                        for ( ;  desc_iter != data.end();  ++desc_iter) {
                            if ((*desc_iter)->IsTitle() ) {
                                track_name = (*desc_iter)->GetTitle();
                                break;
                            }
                            if ((*desc_iter)->IsName() ) {
                                track_name = (*desc_iter)->GetName();
                            }
                        }
                    }
                }

                string label;
                wxFileName wx_fn(fn);
                string fileName(wx_fn.GetFullName().ToAscii());
                label = fileName + " - ";
                if (track_name.empty()) {
                    CNcbiOstrstream ostr;
                    ostr << "(WIG Seq-annot " << index << ")";
                    label += CNcbiOstrstreamToString(ostr);
                    annot->SetNameDesc(fileName);
                }
                else {
                    label += track_name;
                    annot->SetNameDesc(track_name);
                }


                if (annot->IsSeq_table()) {
                    const CSeq_id* id = s_GetAnnotId(*annot);
                    if (id != 0) {
                        label += " (on ";
                        label += id->GetSeqIdString();
                        label += ")";
                    }
                }

                m_Objects.push_back(SObject(*annot, label));
                x_UpdateMap(*annot);
                ++index;
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

bool CWigObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("WIG import errors"));
    return x_ShowMappingDlg(m_Objects);
}

END_NCBI_SCOPE
