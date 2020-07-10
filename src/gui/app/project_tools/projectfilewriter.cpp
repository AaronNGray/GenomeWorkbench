/*  $Id: projectfilewriter.cpp 38112 2017-03-31 02:52:31Z ucko $
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
#include "projectfilewriter.hpp"
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
//#include <gui/objutils/utils.hpp> DO NOT INCLUDE THIS IN A NON_GUI APP
#include <objtools/writers/gtf_writer.hpp>
#include <objtools/simple/simple_om.hpp>

#define MARKLINE cout << __FILE__ << ":" << __LINE__ << endl

using namespace ncbi;
using namespace objects;


CProjectFileWriter::CProjectFileWriter(CRef<CScope> tScope) : Scope(tScope)
{

}


void CProjectFileWriter::AddToScope(CConstRef<CSerialObject> Object)
{
    if(Object->GetThisTypeInfo() == CSeq_id::GetTypeInfo()) {
        CConstRef<CSeq_id> Id(dynamic_cast<const CSeq_id*>(Object.GetPointer()));
        CBioseq_Handle Handle = CSimpleOM::GetBioseqHandle(*Id);
        Scope->AddBioseq(*Handle.GetCompleteBioseq());
    }
    else if(Object->GetThisTypeInfo() == CSeq_entry::GetTypeInfo()) {
        CConstRef<CSeq_entry> Entry(dynamic_cast<const CSeq_entry*>(Object.GetPointer()));
        Scope->AddTopLevelSeqEntry(*Entry);
    }
    else if(Object->GetThisTypeInfo() == CSeq_annot::GetTypeInfo()) {
        CConstRef<CSeq_annot> Annot(dynamic_cast<const CSeq_annot*>(Object.GetPointer()));
        Scope->AddSeq_annot(*Annot);
    }
}



bool CProjectFileWriter::WriteFile(const string& Filename, CConstRef<CSerialObject> Object, 
                            CFormatGuess::EFormat Format)
{

    CNcbiOfstream Out(Filename.c_str(), IOS_BASE::out | IOS_BASE::binary);
    if (!Out.is_open()) {
       return false;
    }    
    
    return x_WriteFile(Out, Object, Format);
    //cout << MSerial_AsnText << *Result << endl;
}


bool CProjectFileWriter::x_WriteFile(CNcbiOstream& Out, CConstRef<CSerialObject> Object, 
                              CFormatGuess::EFormat Format)
{    
    switch (Format) {
       
    case CFormatGuess::eUnknown:
        break;

    case CFormatGuess::eBinaryASN: 	
        return x_WriteObject(Out, Object, eSerial_AsnBinary);
        break; 

    case CFormatGuess::eTextASN:
        return x_WriteObject(Out, Object, eSerial_AsnText);
        break; 

    case CFormatGuess::eXml:
        return x_WriteObject(Out, Object, eSerial_Xml);
        break; 

    case CFormatGuess::eGtf:        
        return x_WriteGtf(Out, Object);
        break;
/*
    case CFormatGuess::eRmo:        
        Result = x_LoadRepeatMasker(In);
        break;

    case CFormatGuess::eFasta:        
        Result = x_LoadFasta(In);
        break;
*/
    default:
        NCBI_THROW(CException, eUnknown, "This file is not supported!");
        break;
    }

    return false;
}


bool CProjectFileWriter::x_WriteObject(CNcbiOstream& Out, CConstRef<CSerialObject> Object, 
                                ESerialDataFormat Format)
{
    try {
        auto_ptr<CObjectOStream> ObjS(CObjectOStream::Open(Format, Out));
        *ObjS << *Object;
        return true;
    } catch(std::exception& e) {
        LOG_POST(Error << "Failed to serialize object: " << e.what());
        throw; 
    }
    
    return false;
}


bool CProjectFileWriter::x_WriteGtf(CNcbiOstream& Out, CConstRef<CSerialObject> Object)
{
    if(Object->GetThisTypeInfo() == CSeq_id::GetTypeInfo()) {
        CSeq_id_Handle idh
            = CSeq_id_Handle::GetHandle(dynamic_cast<const CSeq_id&>(*Object));
        CGtfWriter writer(*Scope, Out);
        writer.WriteHeader();
        writer.WriteBioseqHandle(Scope->GetBioseqHandle(idh));
        writer.WriteFooter();
        return true;
    }
    else if(Object->GetThisTypeInfo() == CSeq_annot::GetTypeInfo()) {

        CConstRef<CSeq_annot> Annot(dynamic_cast<const CSeq_annot*>(Object.GetPointer()));

        CSeq_annot_Handle AnnotHandle = Scope->GetSeq_annotHandle(*Annot);
        CGtfWriter writer(*Scope, Out);
        writer.WriteHeader();
        writer.WriteSeqAnnotHandle(AnnotHandle);
        writer.WriteFooter();
        return true;
    }
    else {        
        return x_WriteObject(Out, Object, eSerial_Xml);
    }
}


TGi CProjectFileWriter::x_FindParent(TGi Gi)
{
    CScope::TTSE_Handles  Handles;
    TGi ParentGi = ZERO_GI;
    Scope->GetAllTSEs(Handles, CScope::eManualTSEs);

    ITERATE(CScope::TTSE_Handles, handleiter, Handles) {

        CSeq_entry_Handle Handle = *handleiter;
        // If its a Set() (not a Seq()) then we skip it and move on
        if(Handle.IsSet()) {
            continue;
        }

        ParentGi = x_GetGi(Handle.GetSeq().GetSeqId());        

        const CSeqMap& Map = Handle.GetSeq().GetSeqMap();

        SSeqMapSelector RangeSelector;
        RangeSelector.SetRange(0, Map.GetLength(NULL));
        for(CSeqMap::const_iterator CurrSeg = Map.BeginResolved(NULL, RangeSelector);
            CurrSeg != Map.EndResolved(NULL, RangeSelector) && 
            CurrSeg.GetType() != CSeqMap::eSeqEnd;
            ++CurrSeg) {

            if(CurrSeg.GetType() != CSeqMap::eSeqRef) 
                continue;

            TGi CurrGi = x_GetGi(CurrSeg.GetRefSeqid().GetSeqIdOrNull());
            if(CurrGi == Gi) {
                return ParentGi;
            }           
        } // end seq map loop
    }
    // If we get here, return the input Gi back, 
    // because as far as we know, this Gi is the top level parent Gi.
    return Gi;
}

TGi CProjectFileWriter::x_GetAnnotId(CConstRef<CSeq_annot> Annot)
{
    ITERATE(list<CRef<CSeq_feat> >, featiter, Annot->GetData().GetFtable() ) {
        CRef<CSeq_feat> Feat = *featiter;
        CConstRef<CSeq_loc> Loc(&Feat->GetLocation());
        CConstRef<CSeq_id> Id(Loc->GetId());
        TGi Gi = x_GetGi(Id);
        if(Gi != ZERO_GI) {
            return Gi;
        }
    }

    return ZERO_GI;
}


TGi CProjectFileWriter::x_GetGi(CConstRef<CSeq_id> Id) 
{  
    TGi Gi = ZERO_GI;

    try {
        CBioseq_Handle Handle = Scope->GetBioseqHandle(*Id);
        ITERATE(CBioseq_Handle::TId, iditer, Handle.GetId()) {
            CSeq_id_Handle  IdHandle = *iditer;
//cout << MSerial_AsnText << *IdHandle.GetSeqId() << endl;
            if(IdHandle.GetSeqId()->Which() == CSeq_id::e_Gi) {
                Gi = IdHandle.GetSeqId()->GetGi();
//cout << "Found GI: " << Gi << endl;
            }
        }
    } catch(std::exception& e) {
        cout << "CProjectFileWriter::x_GetGi(): " << e.what() << endl;
    } 
    return Gi;
}

