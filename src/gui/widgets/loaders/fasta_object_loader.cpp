/*  $Id: fasta_object_loader.cpp 44796 2020-03-17 22:37:42Z evgeniev $
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
#include <serial/iterator.hpp>

#include <gui/widgets/loaders/fasta_object_loader.hpp>

#include <objtools/readers/fasta.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/object_manager.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/seq_fasta_reader.hpp>

#include <gui/widgets/wx/compressed_file.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CFastaObjectLoader::CFastaObjectLoader(const CFastaLoadParams& params)
    : m_Params(params)
{
}

CFastaObjectLoader::CFastaObjectLoader(const CFastaLoadParams& params, const vector<wxString>& filenames)
 : m_Params(params), m_FileNames(filenames)
{
}

IObjectLoader::TObjects& CFastaObjectLoader::GetObjects()
{
    return m_Objects;
}

string CFastaObjectLoader::GetDescription() const
{
    return "Loading FASTA Files";
}

bool CFastaObjectLoader::PreExecute()
{
    return true;
}

bool CFastaObjectLoader::Execute(ICanceled& canceled)
{
    Init();

    ITERATE(vector<wxString>, it, m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        CRef<CErrorContainer> errCont;
        const wxString& fn = *it;

        try {
            errCont.Reset(new CErrorContainer(100));

            CCompressedFile file(fn);
            TObjects objects;
            LoadFromStream(file.GetIstream(), objects, errCont);
            x_UpdateHTMLResults(fn, errCont);

            for (auto& o : objects)
                m_Objects.push_back(o);
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

void CFastaObjectLoader::Init()
{
    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    m_Scope.Reset(new CScope(*obj_mgr));
    m_Scope->AddDefaults();
}

static CFastaReader::TFlags s_GetFlags(const CFastaLoadParams& params)
{
    CFastaReader::TFlags flags = 0;

    /// interpret the sequence type:
    if (params.GetSeqType() == 1) {
        flags |= CFastaReader::fAssumeNuc;
        flags |= CFastaReader::fForceType;
    }
    else if (params.GetSeqType() == 2) {
        flags |= CFastaReader::fAssumeProt;
        flags |= CFastaReader::fForceType;
    }

    if (params.GetForceLocalIDs())
        flags |= CFastaReader::fParseRawID;

    if (params.GetMakeDelta())
        flags |= CFastaReader::fParseGaps;

    if (params.GetIgnoreGaps())
        flags |= CFastaReader::fHyphensIgnoreAndWarn;

    if (params.GetReadFirst())
        flags |= CFastaReader::fOneSeq;

    if (params.GetSkipInvalid())
        flags |= CFastaReader::fRequireID;

    if (params.GetNoSplit())
        flags |= CFastaReader::fNoSplit;


    flags |= CFastaReader::fDLOptional;
    flags |= CFastaReader::fAddMods;

    return flags;
}


void CFastaObjectLoader::LoadFromStream(CNcbiIstream& istr, TObjects& objects, CErrorContainer* errCont, ICanceled* canceled)
{
    CFastaReader::TFlags flags = s_GetFlags(m_Params);
    flags |= CFastaReader::fParseRawID;

    CRef<ILineReader> line_reader(ILineReader::New(istr));
    CSeqFastaReader rdr(*m_Scope, *line_reader, flags);

    if (canceled)
        rdr.SetCanceler(canceled);

    CFastaReader::TMasks lcv;
    rdr.SaveMasks(&lcv);
    vector<CConstRef<CSeq_id>> wellknown_ids;
    CRef<CSeq_entry> entry = rdr.ReadSequences(&wellknown_ids, kMax_Int, errCont);
    
    // Add any lowercase masks as features
    vector<CRef<CSeq_entry> > entries;
    if (entry->IsSeq()) {
        entries.push_back(entry);
    }
    else {
        NON_CONST_ITERATE(CBioseq_set::TSeq_set, iter,
            entry->SetSet().SetSeq_set()) {
            entries.push_back(*iter);
        }
    }
    for (unsigned int i = 0; i < entries.size(); ++i) {
        const CSeq_loc& loc = *lcv[i];
        CSeq_entry& ent = *entries[i];
        // Add unless loc is null (all uppercase) or everything (all lowercase)
        if (!loc.IsNull() &&
            sequence::GetLength(loc, 0) < ent.GetSeq().GetInst().GetLength()) {

            ///
            /// first, expand the location into a set of sublocs
            /// ReadFasta will return a single packed seq-int
            /// and it is much more usable if this can be expressed as a
            /// set of simple intervals
            ///
            vector< CRef<CSeq_loc> > expanded_locs;
            if (loc.IsPacked_int()) {
                /// fast path - no extra allocation
                ITERATE(CSeq_loc::TPacked_int::Tdata, iter, loc.GetPacked_int().Get()) {
                    CRef<CSeq_loc> loc_int(new CSeq_loc());
                    CRef<CSeq_interval> ref = *iter;
                    loc_int->SetInt(*ref);
                    expanded_locs.push_back(loc_int);
                }
            }
            else {
                /// generic path - create intervals
                const CSeq_id& id = sequence::GetId(loc, NULL);
                for (CSeq_loc_CI loc_iter(loc); loc_iter; ++loc_iter) {
                    CRef<CSeq_loc> loc_int(new CSeq_loc());
                    loc_int->SetInt().SetFrom(loc_iter.GetRange().GetFrom());
                    loc_int->SetInt().SetTo(loc_iter.GetRange().GetTo());
                    loc_int->SetId(id);
                    expanded_locs.push_back(loc_int);
                }
            }


            CRef<CSeq_annot> annot(new CSeq_annot);
            if (m_Params.GetLowercaseOption() == 0 ||
                m_Params.GetLowercaseOption() == 1) {
                /// the masked location comes back as a packed seq-int
                /// we unpack this into a set of intervals and package
                /// separate features for each
                NON_CONST_ITERATE(vector< CRef<CSeq_loc> >, iter, expanded_locs) {
                    CRef<CSeq_feat> feat(new CSeq_feat);
                    feat->SetLocation(**iter);

                    if (m_Params.GetLowercaseOption() == 0) {
                        feat->SetData().SetRegion("lowercase in FASTA file");
                    }
                    else {
                        feat->SetData().SetImp().SetKey("repeat_region");
                        feat->SetData().SetImp().SetDescr("lowercase in FASTA file");
                    }
                    annot->SetData().SetFtable().push_back(feat);
                }
            }
            else {
                NON_CONST_ITERATE(vector< CRef<CSeq_loc> >, iter, expanded_locs) {
                    annot->SetData().SetLocs().push_back(*iter);
                }
            }

            ent.SetSeq().SetAnnot().push_back(annot);
        }
    }

    if (!entries.empty()) {
        string label;
        if (m_Scope)
            CLabel::GetLabel(*entry, &label, CLabel::eDefault, m_Scope);

        objects.push_back(SObject(*entry, label));
    }

    for (auto &id : wellknown_ids) {
        string label;
        if (m_Scope) {
            CLabel::GetLabel(*id, &label, CLabel::eDefault, m_Scope);
        }

        CRef<CSeq_id> id2(new CSeq_id());
        id2->Assign(*id);
        objects.push_back(SObject(*id2, label));
    }
}

void CFastaObjectLoader::LoadAlignsFromStream(CNcbiIstream& istr, TObjects& objects, CErrorContainer* errCont, ICanceled* canceled)
{
    CFastaReader::TFlags flags = s_GetFlags(m_Params);

    CRef<ILineReader> line_reader(ILineReader::New(istr));
    CSeqFastaReader rdr(*m_Scope, *line_reader, flags);

    if (canceled)
        rdr.SetCanceler(canceled);

    CFastaReader::TMasks lcv;
    rdr.SaveMasks(&lcv);
    CRef<CSeq_entry> entry = rdr.ReadAlignedSet(-1, errCont);

    if (!entry)
        return;

    CRef<CSeq_align> align;
    for (CTypeIterator<CSeq_align> it(*entry); it; ++it) {
        CRef<CSeq_annot> annot(new CSeq_annot());
        annot->SetData().SetAlign().push_back(CRef<CSeq_align>(&*it));

        string label;
        if (m_Scope)
            CLabel::GetLabel(*entry, &label, CLabel::eDefault, m_Scope);

        objects.push_back(SObject(*annot, label));
    }
}

bool CFastaObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("FASTA import errors"));
    return true;
}

END_NCBI_SCOPE
