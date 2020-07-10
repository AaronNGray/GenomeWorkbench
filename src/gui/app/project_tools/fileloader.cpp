/*  $Id: fileloader.cpp 36463 2016-09-27 03:28:38Z ucko $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Author:  Aleksey Grichenko
 *
 * File Description:
 *   Demo of using the C++ Object Manager (OM)
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include "fileloader.hpp"
#include <gui/gui_export.h>

#include <serial/objistr.hpp>
#include <objtools/readers/gff_reader.hpp>
#include <objtools/readers/rm_reader.hpp>
#include <objtools/readers/fasta.hpp>

#include <objmgr/util/obj_sniff.hpp>
#include <objmgr/error_codes.hpp>

#include <serial/iterator.hpp>
#include <serial/serial.hpp>
#include <serial/objhook.hpp>
#include <serial/iterator.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>


#include <objects/gbproj/ProjectItem.hpp>
#include <objects/seq/NCBI_Sequence_module.hpp>
#include <objects/seqloc/NCBI_Seqloc_module.hpp>
#include <objects/seqalign/NCBI_Seqalign_module.hpp>
#include <objects/seqfeat/NCBI_Seqfeat_module.hpp>
#include <objects/seqset/NCBI_Seqset_module.hpp>
#include <objects/submit/NCBI_Submit_module.hpp>
#include <objects/biotree/NCBI_BioTree_module.hpp>
#include <objects/entrezgene/NCBI_Entrezgene_module.hpp>
#include <objects/pubmed/NCBI_PubMed_module.hpp>
#include <gui/objects/NCBI_GUI_Project_module.hpp>
#include <serial/pack_string.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Std_seg.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/biotree/BioTreeContainer.hpp>
#include <objects/biotree/DistanceMatrix.hpp>
#include <objects/entrezgene/Entrezgene.hpp>
#include <objects/entrezgene/Entrezgene_Set.hpp>
#include <objects/pubmed/Pubmed_entry.hpp>
#include <objtools/format/flat_file_generator.hpp>
//#include <gui/objutils/utils.hpp> DO NOT INCLUDE THIS IN A NON_GUI APP
#include <objtools/format/ostream_text_ostream.hpp>
#include <objtools/simple/simple_om.hpp>
#include <objtools/readers/agp_read.hpp>
#include <objtools/readers/format_guess_ex.hpp>

#define MARKLINE cout << __FILE__ << ":" << __LINE__ << endl

using namespace ncbi;
using namespace objects;


void CFileLoader::LoadFile(const string& Filename, TSerialObjectList& Objects,
                      EFormat Format)
{
    CFile File(Filename);
    if(!File.Exists() || File.GetLength() == 0)
        return;

    CNcbiIfstream In(Filename.c_str(), IOS_BASE::in | IOS_BASE::binary);
    if (!In.is_open()) {
       return;
    }

    // If unknown, try and guess the format. If that doesn't work, just return null
    if(Format == CFormatGuess::eUnknown) {
        Format = x_GuessFormat(In);

        if(Format == CFormatGuess::eUnknown)
            return;
    }

    while(!In.eof()) {
        x_LoadFile(In, Format, Objects);
        //cout << MSerial_AsnText << *Result << endl;
    }

    return;
}


CRef<CSerialObject> CFileLoader::LoadFile(const string& Filename, CFormatGuess::EFormat Format)
{
    TSerialObjectList Objects;

    LoadFile(Filename, Objects, Format);

    if(Objects.empty())
        return CRef<CSerialObject>();
    else
        return Objects.front();
}


CFormatGuess::EFormat CFileLoader::x_GuessFormat(CNcbiIfstream& In)
{
    CFormatGuessEx Guesser(In);
    return Guesser.GuessFormat();
}


void CFileLoader::x_LoadFile(CNcbiIstream& In,
                             CFormatGuess::EFormat Format,
                             TSerialObjectList& Objects)
{
    CRef<CSerialObject> Result;

    switch (Format) {

    case CFormatGuess::eUnknown:
        break;

    case CFormatGuess::eBinaryASN:
        x_LoadAsnBinary(In, Objects);
        break;

    case CFormatGuess::eTextASN:
        x_LoadAsnTextOrXml(In, eSerial_AsnText, Objects);
        break;

    case CFormatGuess::eXml:
        x_LoadAsnTextOrXml(In, eSerial_Xml, Objects);
        break;

    case CFormatGuess::eGtf:
        x_LoadGff(In, Objects);
        break;

    case CFormatGuess::eRmo:
        Result = x_LoadRepeatMasker(In);
        break;

    case CFormatGuess::eFasta:
        Result = x_LoadFasta(In);
        break;

    case CFormatGuess::eAgp:
        x_LoadAGP(In, Objects);
        break;

    default:
        NCBI_THROW(CException, eUnknown, "This file is not supported!");
        break;
    }

    if(!Result.IsNull())
        Objects.push_back(Result);
}

void CFileLoader::x_LoadGff(CNcbiIstream& In, TSerialObjectList& Objects)
{
    CGFFReader::TFlags Flags =
        CGFFReader::fMergeExons |
        CGFFReader::fGBQuals |
        CGFFReader::fCreateGeneFeats |
        CGFFReader::fSetProducts;

    CRef<CSeq_entry> Entry;
    CGFFReader Reader;
    Entry = Reader.Read(In, Flags);

    if(Entry->IsSet()) {
        CRef<CSeq_annot> Annot;
        ITERATE(CBioseq_set::TSeq_set, SeqIter, Entry->GetSet().GetSeq_set()) {
            const CBioseq& Bioseq = (*SeqIter)->GetSeq();

            if(Bioseq.CanGetInst() && Bioseq.GetInst().CanGetRepr() &&
               Bioseq.GetInst().GetRepr() == CSeq_inst::eRepr_virtual) {

                if(Bioseq.CanGetAnnot() && !Bioseq.GetAnnot().empty()) {
                    ITERATE(CBioseq::TAnnot, AnnotIter, Bioseq.GetAnnot()) {
                        Annot = Bioseq.GetAnnot().front();
                        Objects.push_back(CRef<CSerialObject>(Annot));
                    }
                }
            }
        }
    }

    if(!Objects.empty())
        return;

    Objects.push_back(CRef<CSerialObject>(Entry));
}

CRef<CSeq_annot> CFileLoader::x_LoadRepeatMasker(CNcbiIstream& In)
{
    CRmReader::TFlags Flags =
        CRmReader::fIncludeRepeatClass |
        CRmReader::fIncludeRepeatName;
    CRef<CSeq_annot> Result;
    CRmReader* Reader;

    Reader = CRmReader::OpenReader(In);
    Reader->Read(Result, Flags);
    CRmReader::CloseReader(Reader);
    return Result;
}

CRef<CSeq_entry> CFileLoader::x_LoadFasta(CNcbiIstream& In)
{
    CFastaReader Reader(In);
    return Reader.ReadSet();
}


void CFileLoader::x_LoadAGP(CNcbiIstream& In, TSerialObjectList& Objects)
{
    vector< CRef< CSeq_entry > > SeqEntries;
    AgpRead(In, SeqEntries, eAgpRead_ParseId, true);

    CRef<CSeq_entry> Result(new CSeq_entry);
    NON_CONST_ITERATE(vector<CRef<CSeq_entry> >, iter, SeqEntries) {
        Objects.push_back(CRef<CSerialObject>(*iter));
    }
}


void CFileLoader::x_LoadAsnTextOrXml(CNcbiIstream& In,
                                     ESerialDataFormat Format,
                                     TSerialObjectList& Objects)
{

    In.seekg(0);
    auto_ptr<CObjectIStream> ObjS(CObjectIStream::Open(Format, In));
    const string& Type = ObjS->ReadFileHeader();
    ObjS->SetStreamPos(0);
//    cout << "Type:  " << Type << endl;

    while(!In.eof()) {
        CRef<CSerialObject> Result;
        try {
            const CTypeInfo* TypeInfo = CClassTypeInfoBase::GetClassInfoByName(Type);
    //cout << "GetNamespaceName   " << TypeInfo->GetNamespaceName() << endl;
    //cout << "GetNamespacePrefix   " << TypeInfo->GetNamespacePrefix() << endl;
    //cout << "GetModuleName   " << TypeInfo->GetModuleName() << endl;
    //cout << "GetSize   " << TypeInfo->GetSize() << endl;
    //cout << "TypeInfo->IsCObject  " << TypeInfo->IsCObject() << endl;
            if(TypeInfo)
                Result = CRef<CSerialObject>(static_cast<CSerialObject*>(TypeInfo->Create()) );
    //cout << "Result.IsNull  " << Result.IsNull() << endl;
    //cout << "Result " << Result << endl;
            if(!Result.IsNull()) {
                //In >> (*Result);
                ObjS->Read(Result, Result->GetThisTypeInfo());
                Objects.push_back(CRef<CSerialObject>(Result));
            }
        } catch (std::exception& e) {
            if(Objects.empty() || !In.eof()) {
                //Result.Reset();
                LOG_POST(Error << "Failed to deserialize object: " << e.what());
                throw;
            }
        }
    }
}


///
/// Class CSniffReader reads NCBI object files in number of different formats.
/// Uses try and fail deserialization for binary ASN.1 files
///
/// Because CObjectSniffer is only useable when inherited from
class CSniffReader : public CObjectsSniffer
{
public:
    CSniffReader(list<CRef<CSerialObject> >& Results);

    // Event function called when parser finds a top level object
    virtual void OnTopObjectFoundPre(const CObjectInfo& object,
        CNcbiStreampos stream_pos) { }

    // Event function alled after top object deserialization
    virtual void OnTopObjectFoundPost(const CObjectInfo& object);

    // Overload from CObjectsSniffer
    virtual void OnObjectFoundPre(const CObjectInfo& object,
                                  CNcbiStreampos stream_pos) {
        if (m_ObjLevel == 0) {
            OnTopObjectFoundPre(object, stream_pos);
        }
        ++m_ObjLevel;
    }

    // Overload from CObjectsSniffer
    virtual void OnObjectFoundPost(const CObjectInfo& object) {
        _ASSERT(m_ObjLevel > 0);
        if (m_ObjLevel == 1) {
            OnTopObjectFoundPost(object);
        }
        --m_ObjLevel;
    }

    virtual void Reset() {
        m_ObjLevel = 0;
    }

private:

    list<CRef<CSerialObject> >&         m_Results;
    unsigned int                 m_ObjLevel;   // Object level counter, used to identify
};

CSniffReader::CSniffReader(list<CRef<CSerialObject> >& Results)
            : m_Results(Results), m_ObjLevel(0)
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
    AddCandidate(CObjectTypeInfo(CType<CSeq_table>()));

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
}


void CSniffReader::OnTopObjectFoundPost(const CObjectInfo& object)
{
    CRef<CSerialObject> New;
    New.Reset(static_cast<CSerialObject*>(object.GetObjectPtr()));
    m_Results.push_back(New);
}


void CFileLoader::x_LoadAsnBinary(CNcbiIstream& In, TSerialObjectList& Objects)
{
    while(!In.eof()) {
        CRef<CSerialObject> Result;

        auto_ptr<CObjectIStream> ObjS(CObjectIStream::Open(eSerial_AsnBinary, In));
        CSniffReader Reader(Objects);

        try {
            Reader.Probe(*ObjS);
//            size_t StopPos = In.tellg();
//            Objects.push_back(CRef<CSerialObject>(Result));
//            cout << "Stop Pos: " << StopPos << endl;
        } catch(std::exception) { ; }
    }
}



