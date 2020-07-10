/*  $Id: asn_object_loader.cpp 44344 2019-12-03 19:23:38Z katargir $
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

#include <gui/widgets/loaders/asn_object_loader.hpp>

#include <util/icanceled.hpp>

#include <gui/widgets/loaders/asn_load_extension.hpp>
#include <gui/utils/extension_impl.hpp>
#include <gui/objutils/label.hpp>

#include <util/format_guess.hpp>
#include <objmgr/util/obj_sniff.hpp>
#include <objmgr/object_manager.hpp>

#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Gb_qual.hpp>

#include <objects/biotree/BioTreeContainer.hpp>
#include <objects/biotree/DistanceMatrix.hpp>
#include <objects/entrezgene/Entrezgene.hpp>
#include <objects/entrezgene/Entrezgene_Set.hpp>
#include <objects/pubmed/Pubmed_entry.hpp>

#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Std_seg.hpp>
#include <objects/submit/Seq_submit.hpp>

#include <objects/seq/NCBI_Sequence_module.hpp>
#include <objects/seqloc/NCBI_Seqloc_module.hpp>
#include <objects/seqalign/NCBI_Seqalign_module.hpp>
#include <objects/seqfeat/NCBI_Seqfeat_module.hpp>
#include <objects/seqset/NCBI_Seqset_module.hpp>
#include <objects/submit/NCBI_Submit_module.hpp>
#include <objects/biotree/NCBI_BioTree_module.hpp>
#include <objects/entrezgene/NCBI_Entrezgene_module.hpp>
#include <objects/pubmed/NCBI_PubMed_module.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <gui/objects/NCBI_GUI_Project_module.hpp>
#include <objects/gbproj/ProjectItem.hpp>

#include <serial/serial.hpp>
#include <serial/pack_string.hpp>

#include <wx/filename.h>

#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///
/// Class CSniffReader reads NCBI object files in number of different formats.
/// Uses try and fail deserialization for binary ASN.1 files
///
class CSniffReader : public CObjectsSniffer, public IAsnLoadRegister
{
public:
    CSniffReader(CAsnObjectLoader& loader);

    // Event function called when parser finds a top level object
    virtual void OnTopObjectFoundPre(const CObjectInfo& object,
        CNcbiStreampos stream_pos);

    // Event function alled after top object deserialization
    virtual void OnTopObjectFoundPost(const CObjectInfo& object);

    // Overload from CObjectsSniffer
    virtual void OnObjectFoundPre(const CObjectInfo& object,
        CNcbiStreampos stream_pos);

    // Overload from CObjectsSniffer
    virtual void OnObjectFoundPost(const CObjectInfo& object);

    virtual void Reset();

    bool ObjectLoaded() const { return m_ObjectLoaded; }

/// IAsnLoadRegister implementation
    virtual void Register(CObjectTypeInfo ti);

private:
    unsigned int                 m_ObjLevel;   // Object level counter, used to identify
    // the TSE

    CAsnObjectLoader& m_Loader;
    bool m_ObjectLoaded;
};

CAsnObjectLoader::CAsnObjectLoader()
{
}

CAsnObjectLoader::CAsnObjectLoader(const vector<wxString>& filenames)
 : m_FileNames(filenames)
{
}

CAsnObjectLoader::CAsnObjectLoader(const string& asnTextData)
 : m_AsnTextData(asnTextData)
{
}

IObjectLoader::TObjects& CAsnObjectLoader::GetObjects()
{
    return m_Objects;
}

string CAsnObjectLoader::GetDescription() const
{
    return "Loading NCBI ASN.1 Files";
}

bool CAsnObjectLoader::PreExecute()
{
    return true;
}

static const wxChar* kMemoryObject = wxT("Memory object");

void CAsnObjectLoader::Init()
{
    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    m_Scope.Reset(new CScope(*obj_mgr));
    m_Scope->AddDefaults();
}

bool CAsnObjectLoader::Execute(ICanceled& canceled)
{
    Init();

    ITERATE(vector<wxString>, it, m_FileNames) {
        if (canceled.IsCanceled())
            return false;

        const wxString& fn = *it;
        try {
            m_CurrentFileName = *it;

            CCompressedFile file(fn);
            CFormatGuess::EFormat fmt = file.GuessFormat();
            file.Reset();

            ESerialDataFormat sfmt = eSerial_None;
            wxString ext;
            switch (fmt) {
            case CFormatGuess::eBinaryASN:
                sfmt = eSerial_AsnBinary;
                break;
            case CFormatGuess::eTextASN:
                sfmt = eSerial_AsnText;
                break;
            case CFormatGuess::eXml:
                sfmt = eSerial_Xml;
                break;
            case CFormatGuess::eUnknown:
                wxFileName::SplitPath(fn, nullptr, nullptr, nullptr, &ext);
                if (ext.IsSameAs(wxT("asn"), false)) {
                    LOG_POST(Info << "Format Guess failed, format assumed to be BinaryASN, based on file extension");
                    sfmt = eSerial_AsnBinary;
                }
                break;
            default:
                break;
            }

            if (sfmt == eSerial_None) {
                x_UpdateHTMLResults(fn, 0, "Serial format could not be determined.");
            }
            else {
                TObjects objects;
                LoadFromStream(file.GetIstream(), objects, sfmt, &canceled);
                for (auto& o : objects)
                    m_Objects.push_back(o);
            }
        }
        catch (const CException& e) {
            string err_msg = e.what();
            vector<string> phrases;
            NStr::Split(err_msg, "\r\n", phrases);
            x_UpdateHTMLResults(fn, 0, phrases[1]);
        }
        catch (const exception& e) {
            x_UpdateHTMLResults(fn, 0, e.what());
        }

        m_CurrentFileName.clear();
    } // ITERATE

    if (!m_AsnTextData.empty()) {
        try {
            m_CurrentFileName = kMemoryObject;

            TObjects objects;
            CNcbiIstrstream istr(m_AsnTextData.c_str());
            LoadFromStream(istr, objects, eSerial_AsnText, &canceled);

            for (auto& o : objects)
                m_Objects.push_back(o);
        }
        catch (const CException& e) {
            string err_msg = e.what();
            vector<string> phrases;
            NStr::Split(err_msg, "\r\n", phrases);
            x_UpdateHTMLResults(m_CurrentFileName, 0, phrases[1]);
        }
        catch (const exception& e) {
            x_UpdateHTMLResults(m_CurrentFileName, 0, e.what());
        }

        m_CurrentFileName.clear();
    }

    return true;
}

void CAsnObjectLoader::LoadFromStream(CNcbiIstream& istr, TObjects& objects, ESerialDataFormat fmt, ICanceled* canceled)
{
    if (fmt == eSerial_None)
        return;

    // use object sniffer to guess
    CSniffReader sniffer(*this);
    unique_ptr<CObjectIStream>
        sinput(CObjectIStream::Open(fmt, istr));

    // memory profile optimization:
    // we should use pooled strings to reduce the memory burden
    // (from Eugene Vasilchenko)
    //
    CObjectTypeInfo info;
    info = CObjectTypeInfo(CType<CObject_id>());
    info.FindVariant("str")
        .SetLocalReadHook(*sinput, new CPackStringChoiceHook);

    info = CObjectTypeInfo(CType<CImp_feat>());
    info.FindMember("key")
        .SetLocalReadHook(*sinput, new CPackStringClassHook(32, 128));

    info = CObjectTypeInfo(CType<CDbtag>());
    info.FindMember("db")
        .SetLocalReadHook(*sinput, new CPackStringClassHook);

    info = CType<CGb_qual>();
    info.FindMember("qual")
        .SetLocalReadHook(*sinput, new CPackStringClassHook);

    sinput->UseMemoryPool();

    // now, probe!
    //

    sniffer.AddCandidate(ncbi::objects::CSeq_table::GetTypeInfo());

    if (sinput->GetDataFormat() == eSerial_AsnText || sinput->GetDataFormat() == eSerial_Xml)
        sniffer.SetReportDataErrors();

    sniffer.Probe(*sinput);
    if (!sniffer.ObjectLoaded()) {
        LOG_POST(Info << "No Top objects found:\n" << m_CurrentFileName.ToUTF8());
    }
    else {
        // We don't want to overwhelm the project tree view by creating
        // a large number of project items. Specifically, we need to
        // address many (>10) seq-aligns or seq-feats problem. 
        TOrigObjects align_objs;
        TOrigObjects feat_objs;
        TOrigObjects other_objs;
        NON_CONST_ITERATE(TOrigObjects, iter, m_OrigObjects) {
            if (dynamic_cast<CSeq_align*>(iter->GetPointer())) {
                align_objs.push_back(*iter);
            }
            else if (dynamic_cast<CSeq_feat*>(iter->GetPointer())) {
                feat_objs.push_back(*iter);
            }
            else {
                other_objs.push_back(*iter);
            }
        }
        m_OrigObjects.clear();

        string annot_title;
        if (!m_CurrentFileName.empty()) {
            wxString fbase;
            wxFileName::SplitPath(m_CurrentFileName, 0, &fbase, 0);
            annot_title = ToStdString(fbase);
        }

        const size_t max_obj_prj = 10;
        if (align_objs.size() > max_obj_prj) {
            // combine all seq-aligns into one seq-annot
            CRef<CSeq_annot> annot(new CSeq_annot);
            annot->SetNameDesc(ToStdString(m_CurrentFileName));
            annot->SetTitleDesc(annot_title);
            CSeq_annot::TData::TAlign& aligns = annot->SetData().SetAlign();
            NON_CONST_ITERATE(TOrigObjects, iter, align_objs) {
                CRef<CSeq_align> align(dynamic_cast<CSeq_align*>(iter->GetPointer()));
                aligns.push_back(align);
            }
            other_objs.push_back(CRef<CObject>(annot.GetPointer()));

        }
        else if (!align_objs.empty()) {
            std::copy(align_objs.begin(), align_objs.end(), back_inserter(other_objs));
        }

        if (feat_objs.size() > max_obj_prj) {
            // combine all seq-feats into one seq-annot
            CRef<CSeq_annot> annot(new CSeq_annot);
            annot->SetNameDesc(ToStdString(m_CurrentFileName));
            annot->SetTitleDesc(annot_title);
            CSeq_annot::TData::TFtable& feats = annot->SetData().SetFtable();
            NON_CONST_ITERATE(TOrigObjects, iter, feat_objs) {
                CRef<CSeq_feat> feat(dynamic_cast<CSeq_feat*>(iter->GetPointer()));
                feats.push_back(feat);
            }

            other_objs.push_back(CRef<CObject>(annot.GetPointer()));
        }
        else if (!feat_objs.empty()) {
            std::copy(feat_objs.begin(), feat_objs.end(), back_inserter(other_objs));
        }


        // initialize a label for each object added
        NON_CONST_ITERATE(TOrigObjects, iter, other_objs) {
            CSerialObject* so = static_cast<CSerialObject*>(iter->GetPointer());

            CBioTreeContainer* btrc = dynamic_cast<CBioTreeContainer*>(so);
            if (btrc && !btrc->IsSetLabel()) {
                // if bio-tree label is not set explicitly -- we assume the filename
                btrc->SetLabel(annot_title);
            }

            string label;
            CLabel::GetLabel(*so, &label, CLabel::eDefault, m_Scope);
            if (label.empty() && btrc  &&  btrc->IsSetLabel()) {
                label = btrc->GetLabel();
            }

            if (kMemoryObject != m_CurrentFileName) {
                string fileName(wxFileName(m_CurrentFileName).GetFullName().ToAscii());
                label = fileName + " - " + label;
            }

            // Convert Bioseq and Bioseq-set to Seq-entry
            CRef<CSeq_entry> se;
            if (CBioseq_set* bioseq_set = dynamic_cast<CBioseq_set*>(so)) {
                se.Reset(new CSeq_entry());
                se->SetSet(*bioseq_set);
            }
            else if (CBioseq* bioseq = dynamic_cast<CBioseq*>(so)) {
                se.Reset(new CSeq_entry());
                se->SetSeq(*bioseq);
            }
            SObject object(se ? *se : *so, label);

            if (kMemoryObject != m_CurrentFileName) {
                string filePath(m_CurrentFileName.ToUTF8());
                object.SetFileName(filePath);
            }
            objects.push_back(object);
        }
    }
}

bool CAsnObjectLoader::x_OnTopObjectFound(const CObjectInfo& objInfo)
{
    if (objInfo.GetTypeFamily() == eTypeFamilyClass ||
        objInfo.GetTypeFamily() == eTypeFamilyChoice) {

        CObject* obj = static_cast<CObject*>(objInfo.GetObjectPtr());
        m_OrigObjects.push_back(CRef<CObject>(obj));
        return true;
    }

    return false;
}

bool CAsnObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("ASN import errors"));
    return true;
}

/////////////////////////////////////////////////////////////////////////////


CSniffReader::CSniffReader(CAsnObjectLoader& loader)
: m_ObjLevel(0), m_Loader(loader), m_ObjectLoaded(false)
{
    /// basic top-level data model types:
    AddCandidate(CObjectTypeInfo(CType<CSeq_entry>()));
    AddCandidate(CObjectTypeInfo(CType<CBioseq>()));
    AddCandidate(CObjectTypeInfo(CType<CBioseq_set>()));
    AddCandidate(CObjectTypeInfo(CType<CSeq_submit>()));

    /// annotations:
    AddCandidate(CObjectTypeInfo(CType<CSeq_annot>()));

    /// alignments:
    AddCandidate(CObjectTypeInfo(CType<CSeq_align>()));
    AddCandidate(CObjectTypeInfo(CType<CSeq_align_set>()));
    AddCandidate(CObjectTypeInfo(CType<CDense_seg>()));
    AddCandidate(CObjectTypeInfo(CType<CStd_seg>()));

    /// locations/identifiers:
    AddCandidate(CObjectTypeInfo(CType<CSeq_loc>()));
    AddCandidate(CObjectTypeInfo(CType<CSeq_id>()));

    /// features:
    AddCandidate(CObjectTypeInfo(CType<CSeq_feat>()));

    /// miscellaneous:
    AddCandidate(CObjectTypeInfo(CType<CBioTreeContainer>()));
    AddCandidate(CObjectTypeInfo(CType<CDistanceMatrix>()));
    AddCandidate(CObjectTypeInfo(CType<CEntrezgene>()));
    AddCandidate(CObjectTypeInfo(CType<CEntrezgene_Set>()));
    AddCandidate(CObjectTypeInfo(CType<CPubmed_entry>()));
    AddCandidate(CObjectTypeInfo(CType<CProjectItem>()));
    AddCandidate(CObjectTypeInfo(CType<CGC_Assembly>()));


    /// add types from certain modules,
    /// being sure not to duplicate what's above (that's trouble)

    /// collect types added above
    set<string> types;
    const TCandidates& cands = GetCandidates();
    ITERATE (TCandidates, cand, cands) {
        types.insert(cand->type_info.GetTypeInfo()->GetName());
    }

    /// register a bunch of modules
    vector<string> modules;

    NCBI_Sequence_RegisterModuleClasses();
    modules.push_back("NCBI-Sequence");

    NCBI_Seqloc_RegisterModuleClasses();
    modules.push_back("NCBI-Seqloc");

    NCBI_Seqalign_RegisterModuleClasses();
    modules.push_back("NCBI-Seqalign");

    NCBI_Seqfeat_RegisterModuleClasses();
    modules.push_back("NCBI-Seqfeat");

    NCBI_Seqset_RegisterModuleClasses();
    modules.push_back("NCBI-Seqset");

    NCBI_Submit_RegisterModuleClasses();
    modules.push_back("NCBI-Submit");

    NCBI_BioTree_RegisterModuleClasses();
    modules.push_back("NCBI-BioTree");

    NCBI_Entrezgene_RegisterModuleClasses();
    modules.push_back("NCBI-Entrezgene");

    NCBI_GUI_Project_RegisterModuleClasses();
    modules.push_back("NCBI-GUI-Project");

    NCBI_PubMed_RegisterModuleClasses();
    modules.push_back("NCBI-PubMed");

    /// add all types in those modules as candidates, avoiding duplication
    ITERATE (vector<string>, module, modules) {
        CClassTypeInfoBase::TRegClasses type_names;
        CClassTypeInfoBase::GetRegisteredClassNames(*module, type_names);
        ITERATE(CClassTypeInfoBase::TRegClasses, type_name, type_names) {
            if (types.find(*type_name) == types.end()) {
                const CTypeInfo* type_info =
                    CClassTypeInfoBase::GetClassInfoByName(*type_name);
                AddCandidate(type_info);
            }
        }
    }

    vector<IAsnLoadExtension*> extensions;
    GetExtensionAsInterface(EXT_POINT__ASN_LOAD_EXTENSION, extensions);

    ITERATE(vector<IAsnLoadExtension*>, it, extensions)
        (*it)->RegisterAsnTypes(*this);
}

void CSniffReader::Register(CObjectTypeInfo ti)
{
    AddCandidate(ti);
}


void CSniffReader::Reset()
{
    m_ObjLevel = 0;
}

void CSniffReader::OnObjectFoundPre(const CObjectInfo& object,
                                    CNcbiStreampos stream_pos)
{
    if (m_ObjLevel == 0) {
        OnTopObjectFoundPre(object, stream_pos);
    }
    ++m_ObjLevel;
}


void CSniffReader::OnObjectFoundPost(const CObjectInfo& object)
{
    _ASSERT(m_ObjLevel > 0);
    if (m_ObjLevel == 1) {
        OnTopObjectFoundPost(object);
    }
    --m_ObjLevel;
}


void CSniffReader::OnTopObjectFoundPre(const CObjectInfo& /*object*/,
                                       CNcbiStreampos /*stream_pos*/)
{
}


void CSniffReader::OnTopObjectFoundPost(const CObjectInfo& object)
{
    m_ObjectLoaded = (m_Loader.x_OnTopObjectFound(object) || m_ObjectLoaded);
}

END_NCBI_SCOPE
