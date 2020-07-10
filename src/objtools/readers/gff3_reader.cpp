/*  $Id: gff3_reader.cpp 607807 2020-05-07 18:58:43Z ivanov $
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
 * Author:  Frank Ludwig
 *
 * File Description:
 *   GFF3 file reader
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <util/line_reader.hpp>


#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_point.hpp>

#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/Annot_id.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/so_map.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Code_break.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Feat_id.hpp>

#include <objtools/readers/gff3_reader.hpp>
#include "reader_message_handler.hpp"

#include <algorithm>

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE

unsigned int CGff3Reader::msGenericIdCounter = 0;

//  ----------------------------------------------------------------------------
string CGff3Reader::xNextGenericId()
//  ----------------------------------------------------------------------------
{
    return string("generic") + NStr::IntToString(msGenericIdCounter++);
}

//  ----------------------------------------------------------------------------
bool CGff3ReadRecord::AssignFromGff(
    const string& strRawInput )
//  ----------------------------------------------------------------------------
{
    if (!CGff2Record::AssignFromGff(strRawInput)) {
        return false;
    }
    if (m_strType == "pseudogene") {
        m_strType = "gene";
        m_Attributes["pseudo"] = "true";
        return true;
    }
    if (m_strType == "pseudogenic_transcript") {
        m_strType = "transcript";
        m_Attributes["pseudo"] = "true";
        return true;
    }        
    if (m_strType == "pseudogenic_tRNA") {
        m_strType = "tRNA";
        m_Attributes["pseudo"] = "true";
        return true;
    }
    if (m_strType == "pseudogenic_rRNA") {
        m_strType = "rRNA";
        m_Attributes["pseudo"] = "true";
        return true;
    }
    if (m_strType == "pseudogenic_exon") {
        m_strType = "exon";
        return true;
    }
    if (m_strType == "pseudogenic_CDS") {
        m_strType = "CDS";
        m_Attributes["pseudo"] = "true";
        return true;
    }
    if (m_strType == "transcript") {
        m_strType = "misc_RNA";
        return true;
    }
    return true;
}

//  ----------------------------------------------------------------------------
string CGff3ReadRecord::x_NormalizedAttributeKey(
    const string& strRawKey )
//  ---------------------------------------------------------------------------
{
    string strKey = CGff2Record::xNormalizedAttributeKey( strRawKey );
    if ( 0 == NStr::CompareNocase( strRawKey, "ID" ) ) {
        return "ID";
    }
    if ( 0 == NStr::CompareNocase( strKey, "Name" ) ) {
        return "Name";
    }
    if ( 0 == NStr::CompareNocase( strKey, "Alias" ) ) {
        return "Alias";
    }
    if ( 0 == NStr::CompareNocase( strKey, "Parent" ) ) {
        return "Parent";
    }
    if ( 0 == NStr::CompareNocase( strKey, "Target" ) ) {
        return "Target";
    }
    if ( 0 == NStr::CompareNocase( strKey, "Gap" ) ) {
        return "Gap";
    }
    if ( 0 == NStr::CompareNocase( strKey, "Derives_from" ) ) {
        return "Derives_from";
    }
    if ( 0 == NStr::CompareNocase( strKey, "Note" ) ) {
        return "Note";
    }
    if ( 0 == NStr::CompareNocase( strKey, "Dbxref" )  ||
        0 == NStr::CompareNocase( strKey, "Db_xref" ) ) {
        return "Dbxref";
    }
    if ( 0 == NStr::CompareNocase( strKey, "Ontology_term" ) ) {
        return "Ontology_term";
    }
    return strKey;
}

//  ----------------------------------------------------------------------------
CGff3Reader::CGff3Reader(
    unsigned int uFlags,
    const string& name,
    const string& title,
    SeqIdResolver resolver,
    CReaderListener* pRL):
//  ----------------------------------------------------------------------------
    CGff2Reader( uFlags, name, title, resolver, pRL )
{
    CGff2Record::ResetId();
}

//  ----------------------------------------------------------------------------
CGff3Reader::CGff3Reader(
    unsigned int uFlags,
    CReaderListener* pRL):
//  ----------------------------------------------------------------------------
    CGff3Reader(uFlags, "", "", CReadUtil::AsSeqId, pRL)
{
}

//  ----------------------------------------------------------------------------
CGff3Reader::~CGff3Reader()
//  ----------------------------------------------------------------------------
{
}

//  ----------------------------------------------------------------------------                
CRef<CSeq_annot>
CGff3Reader::ReadSeqAnnot(
    ILineReader& lr,
    ILineErrorListener* pEC ) 
//  ----------------------------------------------------------------------------                
{
 
    mCurrentFeatureCount = 0;
    mParsingAlignment = false;
    auto pAnnot = CReaderBase::ReadSeqAnnot(lr, pEC);
    if (pAnnot  &&  pAnnot->GetData().Which() == CSeq_annot::TData::e_not_set) {
        return CRef<CSeq_annot>();
    }
    return pAnnot;
}

//  ----------------------------------------------------------------------------
void
CGff3Reader::xProcessData(
    const TReaderData& readerData,
    CSeq_annot& annot) 
//  ----------------------------------------------------------------------------
{
    for (const auto& lineData: readerData) {
        const auto& line = lineData.mData;
        if (xParseStructuredComment(line)  &&  
                !NStr::StartsWith(line, "##sequence-region") ) {
            continue;
        }
        if (xParseBrowserLine(line, annot)) {
            continue;
        }
        if (xParseFeature(line, annot, nullptr)) {
            continue;
        }
    }
}

//  ----------------------------------------------------------------------------
bool CGff3Reader::IsInGenbankMode() const
//  ----------------------------------------------------------------------------
{
    return (m_iFlags & CGff3Reader::fGenbankMode);
}

//  ----------------------------------------------------------------------------
void
CGff3Reader::xGetData(
    ILineReader& lr,
    TReaderData& readerData)
//  ----------------------------------------------------------------------------
{
    readerData.clear();
    string line;
    if (xGetLine(lr, line)) {
        if (xNeedsNewSeqAnnot(line)) {
            lr.UngetLine();
            return;
        }
        if (xIsTrackLine(line)) {
            if (!mCurrentFeatureCount) {
                xParseTrackLine(line);
                xGetData(lr, readerData);
                return;
            }
            m_PendingLine = line;
            return;
        }
        if (xIsTrackTerminator(line)) {
            if (!mCurrentFeatureCount) {
                xParseTrackLine("track");
                xGetData(lr, readerData);
            }
            return;
        }
        if (!xIsCurrentDataType(line)) {
            xUngetLine(lr);
            return;
        }
    readerData.push_back(TReaderLine{m_uLineNumber, line});
    }
    ++m_uDataCount;
}

//  ----------------------------------------------------------------------------
void CGff3Reader::xProcessAlignmentData(
    CSeq_annot& annot) 
//  ----------------------------------------------------------------------------
{
    for (const auto id : mAlignmentData.mIds) {
        CRef<CSeq_align> pAlign = Ref(new CSeq_align());
        if (x_MergeAlignments(mAlignmentData.mAlignments.at(id), pAlign)) {
            // if available, add current browser information
            if ( m_CurrentBrowserInfo ) {
                annot.SetDesc().Set().push_back( m_CurrentBrowserInfo );
            }

            annot.SetNameDesc("alignments");

            if ( !m_AnnotTitle.empty() ) {
                annot.SetTitleDesc(m_AnnotTitle);
            }
            // Add alignment
            annot.SetData().SetAlign().push_back(pAlign);
        }
    }
}

//  ----------------------------------------------------------------------------
bool
CGff3Reader::xParseFeature(
    const string& line,
    CSeq_annot& annot,
    ILineErrorListener* pEC)
//  ----------------------------------------------------------------------------
{
    if (CGff2Reader::IsAlignmentData(line)) {
        return xParseAlignment(line);
    }

    //parse record:
    shared_ptr<CGff2Record> pRecord(x_CreateRecord());
    try {
        if (!pRecord->AssignFromGff(line)) {
			return false;
		}
    }
    catch(CObjReaderLineException& err) {
        ProcessError(err, pEC);
        return false;
    }

    //make sure we are interested:
    if (xIsIgnoredFeatureType(pRecord->Type())) {
        return true;
    }
    if (xIsIgnoredFeatureId(pRecord->Id())) {
        return true;
    }

    //append feature to annot:
    if (!xUpdateAnnotFeature(*pRecord, annot, pEC)) {
        return false;
    }

    ++mCurrentFeatureCount;
    mParsingAlignment = false;
    return true;
}


//  ----------------------------------------------------------------------------
bool CGff3Reader::xParseAlignment(
    const string& strLine)
//  ----------------------------------------------------------------------------
{
    if (IsInGenbankMode()) {
        return true;
    }
    auto& ids = mAlignmentData.mIds;
    auto& alignments = mAlignmentData.mAlignments;

    unique_ptr<CGff2Record> pRecord(x_CreateRecord());

    if ( !pRecord->AssignFromGff(strLine) ) {
        return false;
    }
    
    string id;
    if ( !pRecord->GetAttribute("ID", id) ) {
        id = pRecord->Id();
    }

    if (alignments.find(id) == alignments.end()) {
       ids.push_back(id);
    }

    CRef<CSeq_align> alignment;
    if (!x_CreateAlignment(*pRecord, alignment)) {
        return false;
    }

    alignments[id].push_back(alignment);

    ++mCurrentFeatureCount;
    mParsingAlignment = true;
    return true;
}


//  ----------------------------------------------------------------------------
bool CGff3Reader::xUpdateAnnotFeature(
    const CGff2Record& record,
    CSeq_annot& annot,
    ILineErrorListener* pEC)
//  ----------------------------------------------------------------------------
{
    CRef< CSeq_feat > pFeature(new CSeq_feat);

    string type = record.Type();
    NStr::ToLower(type);
    if (type == "exon" || type == "five_prime_utr" || type == "three_prime_utr") {
        return xUpdateAnnotExon(record, pFeature, annot, pEC);
    }
    if (type == "cds"  ||  type == "start_codon"  || type == "stop_codon") {
        return xUpdateAnnotCds(record, pFeature, annot, pEC);
    }
    if (type == "gene") {
        return xUpdateAnnotGene(record, pFeature, annot, pEC);
    }
    if (type == "mrna") {
        return xUpdateAnnotMrna(record, pFeature, annot, pEC);
    }
    if (type == "region") {
        return xUpdateAnnotRegion(record, pFeature, annot, pEC);
    }
    return xUpdateAnnotGeneric(record, pFeature, annot, pEC);
}

//  ----------------------------------------------------------------------------
void CGff3Reader::xVerifyExonLocation(
    const string& mrnaId,
    const CGff2Record& exon)
//  ----------------------------------------------------------------------------
{
    map<string,CRef<CSeq_interval> >::const_iterator cit = mMrnaLocs.find(mrnaId);
    if (cit == mMrnaLocs.end()) {
        string message = "Bad data line: ";
        message += exon.Type();
        message += " referring to non-existent parent feature.";
        CReaderMessage error(
            eDiag_Error,
            m_uLineNumber,
            message);
        throw error;
    }
    const CSeq_interval& containingInt = cit->second.GetObject();
    const CRef<CSeq_loc> pContainedLoc = exon.GetSeqLoc(m_iFlags, mSeqIdResolve);
    const CSeq_interval& containedInt = pContainedLoc->GetInt();
    if (containedInt.GetFrom() < containingInt.GetFrom()  ||  
            containedInt.GetTo() > containingInt.GetTo()) {
        string message = "Bad data line: ";
        message += exon.Type();
        message += " extends beyond parent feature.";
        CReaderMessage error(
            eDiag_Error,
            m_uLineNumber,
            message);
        throw error;
    }
}

//  ----------------------------------------------------------------------------
bool CGff3Reader::xUpdateAnnotExon(
    const CGff2Record& record,
    CRef<CSeq_feat> pFeature,
    CSeq_annot& annot,
    ILineErrorListener* pEC)
//  ----------------------------------------------------------------------------
{
    list<string> parents;
    if (record.GetAttribute("Parent", parents)) {
        for (list<string>::const_iterator it = parents.begin(); it != parents.end(); 
                ++it) {
            const string& parentId = *it; 
            CRef<CSeq_feat> pParent;
            if (!x_GetFeatureById(parentId, pParent)) {
                xAddPendingExon(parentId, record);
                return true;
            }
            if (pParent->GetData().IsRna()) {
                xVerifyExonLocation(parentId, record);
            }
            if (pParent->GetData().IsGene()) {
                if  (!xInitializeFeature(record, pFeature)) {
                    return false;
                }
                return xAddFeatureToAnnot(pFeature, annot);            
            }
            IdToFeatureMap::iterator fit = m_MapIdToFeature.find(parentId);
            if (fit != m_MapIdToFeature.end()) {
                CRef<CSeq_feat> pParent = fit->second;
                if (!pParent->GetData().IsGene()  &&  !record.UpdateFeature(m_iFlags, pParent)) {
                    return false;
                }
            }
        }
    }
    return true;
}


//  ----------------------------------------------------------------------------
bool CGff3Reader::xUpdateAnnotCds(
    const CGff2Record& record,
    CRef<CSeq_feat> pFeature,
    CSeq_annot& annot,
    ILineErrorListener* pEC)
//  ----------------------------------------------------------------------------
{
    xVerifyCdsParents(record);

    list<string> parents;
    record.GetAttribute("Parent", parents);
    map<string, string> impliedCdsFeats;

    // Preliminary:
    //  We do not support multiparented CDS features in -genbank mode yet.
    if (IsInGenbankMode()  &&  parents.size() > 1){
        CReaderMessage error(
            eDiag_Error,
            m_uLineNumber,
            "Unsupported: CDS record with multiple parents.");
        throw error;
    }

    // Step 1:
    // Locations of parent mRNAs are constructed on the fly, by joining in the 
    //  locations of child exons and CDSs as we discover them. Do this
    //  first.
    // IDs for CDS features are derived from the IDs of their parent features.
    //  Generate a list of CDS IDs this record pertains to as we cycle through
    //  the parent list.
    //
    for (list<string>::const_iterator it = parents.begin(); it != parents.end(); 
            ++it) {
        string parentId = *it;
        bool parentIsGene = false;
        //update parent location:
        IdToFeatureMap::iterator featIt = m_MapIdToFeature.find(parentId);
        if (featIt != m_MapIdToFeature.end()) {
            CRef<CSeq_feat> pParent = featIt->second;
            parentIsGene = pParent->GetData().IsGene();
            if (!parentIsGene  &&  !record.UpdateFeature(m_iFlags, pParent)) {
                return false;
            }
            //rw-143:
            // if parent type is miscRNA then change it to mRNA:
            if (pParent->GetData().IsRna()  &&  
                    pParent->GetData().GetRna().GetType()  ==  CRNA_ref::eType_other) {
                pParent->SetData().SetRna().SetType(CRNA_ref::eType_mRNA);
            }
        }

        //generate applicable CDS ID:
        string siblingId("cds");
        if (!record.GetAttribute("ID", siblingId)  ||  !parentIsGene) {
            siblingId = string("cds:") + parentId;
        }
        impliedCdsFeats[siblingId] = parentId;
    }
    // deal with unparented cds
    if (parents.empty()) {
        string cdsId;
        if (!record.GetAttribute("ID", cdsId)) {
            if (IsInGenbankMode()) {
                cdsId = xNextGenericId();
            }
            else {
                cdsId = "cds";
            }
        }
        impliedCdsFeats[cdsId] = "";
    }

    // Step 2:
    // For every sibling CDS feature, look if there is already a feature with that
    //  ID under construction.
    // If there is, use record to update feature under construction.
    // If there isn't, use record to initialize a brand new feature.
    //
    for (auto featIt = impliedCdsFeats.begin(); featIt != impliedCdsFeats.end(); ++featIt) {
        auto cdsId = featIt->first;
        auto parentId = featIt->second;
        auto idIt = m_MapIdToFeature.find(cdsId);
        if (idIt == m_MapIdToFeature.end()) {
            auto altId =  string("cds:") + parentId;
            idIt = m_MapIdToFeature.find(altId);
        }
        if (idIt != m_MapIdToFeature.end()) {
            //found feature with ID in question: update
            record.UpdateFeature(m_iFlags, idIt->second);
        }
        else {
            //didn't find feature with that ID: create new one
            pFeature.Reset(new CSeq_feat);
            xInitializeFeature(record, pFeature);
            if (!parentId.empty()) {
                xFeatureSetQualifier("Parent", parentId, pFeature);
                xFeatureSetXrefParent(parentId, pFeature);
                if (m_iFlags & fGeneXrefs) {
                    xFeatureSetXrefGrandParent(parentId, pFeature);
                }
            }
            xAddFeatureToAnnot(pFeature, annot);
            m_MapIdToFeature[cdsId] = pFeature;
        }
    }
    return true;
}


//  ----------------------------------------------------------------------------
bool CGff3Reader::xFeatureSetXrefGrandParent(
    const string& parent,
    CRef<CSeq_feat> pFeature)
//  ----------------------------------------------------------------------------
{
    IdToFeatureMap::iterator it = m_MapIdToFeature.find(parent);
    if (it == m_MapIdToFeature.end()) {
        return false;
    }
    CRef<CSeq_feat> pParent = it->second;
    const string &grandParentsStr = pParent->GetNamedQual("Parent");
    list<string> grandParents;
    NStr::Split(grandParentsStr, ",", grandParents, 0);
    for (list<string>::const_iterator gpcit = grandParents.begin();
            gpcit != grandParents.end(); ++gpcit) {
        IdToFeatureMap::iterator gpit = m_MapIdToFeature.find(*gpcit);
        if (gpit == m_MapIdToFeature.end()) {
            return false;
        }
        CRef<CSeq_feat> pGrandParent = gpit->second;

        //xref grandchild->grandparent
        CRef<CFeat_id> pGrandParentId(new CFeat_id);
        pGrandParentId->Assign(pGrandParent->GetId());
        CRef<CSeqFeatXref> pGrandParentXref(new CSeqFeatXref);
        pGrandParentXref->SetId(*pGrandParentId);  
        pFeature->SetXref().push_back(pGrandParentXref);

        //xref grandparent->grandchild
        CRef<CFeat_id> pGrandChildId(new CFeat_id);
        pGrandChildId->Assign(pFeature->GetId());
        CRef<CSeqFeatXref> pGrandChildXref(new CSeqFeatXref);
        pGrandChildXref->SetId(*pGrandChildId);
        pGrandParent->SetXref().push_back(pGrandChildXref);
    }
    return true;
}

//  ----------------------------------------------------------------------------
bool CGff3Reader::xFeatureSetXrefParent(
    const string& parent,
    CRef<CSeq_feat> pChild)
//  ----------------------------------------------------------------------------
{
    IdToFeatureMap::iterator it = m_MapIdToFeature.find(parent);
    if (it == m_MapIdToFeature.end()) {
        return false;
    }
    CRef<CSeq_feat> pParent = it->second;

    //xref child->parent
    CRef<CFeat_id> pParentId(new CFeat_id);
    pParentId->Assign(pParent->GetId());
    CRef<CSeqFeatXref> pParentXref(new CSeqFeatXref);
    pParentXref->SetId(*pParentId);  
    pChild->SetXref().push_back(pParentXref);

    //xref parent->child
    CRef<CFeat_id> pChildId(new CFeat_id);
    pChildId->Assign(pChild->GetId());
    CRef<CSeqFeatXref> pChildXref(new CSeqFeatXref);
    pChildXref->SetId(*pChildId);
    pParent->SetXref().push_back(pChildXref);
    return true;
}

//  ----------------------------------------------------------------------------
bool CGff3Reader::xFindFeatureUnderConstruction(
    const CGff2Record& record,
    CRef<CSeq_feat>& underConstruction)
//  ----------------------------------------------------------------------------
{
    string id;
    if (!record.GetAttribute("ID", id)) {
        return false;
    }
    IdToFeatureMap::iterator it = m_MapIdToFeature.find(id);
    if (it == m_MapIdToFeature.end()) {
        return false;
    }

    CReaderMessage fatal(
        eDiag_Fatal,
        m_uLineNumber,
        "Bad data line:  Duplicate feature ID \"" + id + "\".");
    if (record.Id() != mIdToSeqIdMap[id]) {
        throw fatal;
    }
    if (it->second->GetData().IsRna()) {
        throw fatal;
    }
    CSeq_feat tempFeat;
    if (CSoMap::SoTypeToFeature(record.Type(), tempFeat)) {
        if (it->second->GetData().GetSubtype() != tempFeat.GetData().GetSubtype()) {
            throw fatal;
        }
    }

    underConstruction = it->second;
    return true;
}

//  ----------------------------------------------------------------------------
bool CGff3Reader::xUpdateAnnotGeneric(
    const CGff2Record& record,
    CRef<CSeq_feat> pFeature,
    CSeq_annot& annot,
    ILineErrorListener* pEC)
//  ----------------------------------------------------------------------------
{
    CRef<CSeq_feat> pUnderConstruction(new CSeq_feat);
    if (xFindFeatureUnderConstruction(record, pUnderConstruction)) {
        return record.UpdateFeature(m_iFlags, pUnderConstruction);
    }

    string featType = record.Type();
    if (featType == "stop_codon_read_through"  ||  featType == "selenocysteine") {
        string cdsParent;
        if (!record.GetAttribute("Parent", cdsParent)) {
            CReaderMessage error(
                eDiag_Error,
                m_uLineNumber,
                "Bad data line: Unassigned code break.");
            throw error;
        }
        IdToFeatureMap::iterator it = m_MapIdToFeature.find(cdsParent);
        if (it == m_MapIdToFeature.end()) {
            CReaderMessage error(
                eDiag_Error,
                m_uLineNumber,
                "Bad data line: Code break assigned to missing feature.");
            throw error;
        }

        CRef<CCode_break> pCodeBreak(new CCode_break); 
        CSeq_interval& cbLoc = pCodeBreak->SetLoc().SetInt();        
        CRef< CSeq_id > pId = mSeqIdResolve(record.Id(), m_iFlags, true);
        cbLoc.SetId(*pId);
        cbLoc.SetFrom(static_cast<TSeqPos>(record.SeqStart()));
        cbLoc.SetTo(static_cast<TSeqPos>(record.SeqStop()));
        if (record.IsSetStrand()) {
            cbLoc.SetStrand(record.Strand());
        }
        pCodeBreak->SetAa().SetNcbieaa(
            (featType == "selenocysteine") ? 'U' : 'X');
        CRef<CSeq_feat> pCds = it->second;
        CCdregion& cdRegion = pCds->SetData().SetCdregion();
        list< CRef< CCode_break > >& codeBreaks = cdRegion.SetCode_break();
        codeBreaks.push_back(pCodeBreak);
        return true;
    }
    if (!xInitializeFeature(record, pFeature)) {
        return false;
    }
    if (! xAddFeatureToAnnot(pFeature, annot)) {
        return false;
    }
    string strId;
    if ( record.GetAttribute("ID", strId)) {
        m_MapIdToFeature[strId] = pFeature;
    }
    if (pFeature->GetData().IsRna()  ||  
            pFeature->GetData().GetSubtype() == CSeqFeatData::eSubtype_misc_RNA) {
        CRef<CSeq_interval> rnaLoc(new CSeq_interval);
        rnaLoc->Assign(pFeature->GetLocation().GetInt());
        mMrnaLocs[strId] = rnaLoc;
    }
    return true;
}

//  ----------------------------------------------------------------------------
bool CGff3Reader::xUpdateAnnotMrna(
    const CGff2Record& record,
    CRef<CSeq_feat> pFeature,
    CSeq_annot& annot,
    ILineErrorListener* pEC)
//  ----------------------------------------------------------------------------
{
    CRef<CSeq_feat> pUnderConstruction(new CSeq_feat);
    if (xFindFeatureUnderConstruction(record, pUnderConstruction)) {
        return record.UpdateFeature(m_iFlags, pUnderConstruction);
    }

    if (!xInitializeFeature(record, pFeature)) {
        return false;
    }
    string parentsStr;
    if ((m_iFlags & fGeneXrefs)  &&  record.GetAttribute("Parent", parentsStr)) {
        list<string> parents;
        NStr::Split(parentsStr, ",", parents, 0);
        for (list<string>::const_iterator cit = parents.begin();
                cit != parents.end();
                ++cit) {
            if (!xFeatureSetXrefParent(*cit, pFeature)) {
                CReaderMessage error(
                    eDiag_Error,
                    m_uLineNumber,
                    "Bad data line: mRNA record with bad parent assignment.");
                throw error;
            }
        }
    }

    string strId;   
    if ( record.GetAttribute("ID", strId)) {
        m_MapIdToFeature[strId] = pFeature;
    }
    CRef<CSeq_interval> mrnaLoc(new CSeq_interval);
    CSeq_loc::E_Choice choice = pFeature->GetLocation().Which();
    if (choice != CSeq_loc::e_Int) {
        CReaderMessage error(
            eDiag_Error,
            m_uLineNumber,
            "Internal error: Unexpected location type.");
        throw error;
    }
    mrnaLoc->Assign(pFeature->GetLocation().GetInt());
    mMrnaLocs[strId] = mrnaLoc;

    list<CGff2Record> pendingExons;
    xGetPendingExons(strId, pendingExons);
    for (auto exonRecord: pendingExons) {
        CRef< CSeq_feat > pFeature(new CSeq_feat);
        xUpdateAnnotExon(exonRecord, pFeature, annot, pEC);
    }
    if (! xAddFeatureToAnnot(pFeature, annot)) {
        return false;
    }
    return true;
}

//  ----------------------------------------------------------------------------
bool CGff3Reader::xUpdateAnnotGene(
    const CGff2Record& record,
    CRef<CSeq_feat> pFeature,
    CSeq_annot& annot,
    ILineErrorListener* pEC)
//  ----------------------------------------------------------------------------
{
    CRef<CSeq_feat> pUnderConstruction(new CSeq_feat);
    if (xFindFeatureUnderConstruction(record, pUnderConstruction)) {
        return record.UpdateFeature(m_iFlags, pUnderConstruction);
    }

    if (!xInitializeFeature(record, pFeature)) {
        return false;
    }
    if (! xAddFeatureToAnnot(pFeature, annot)) {
        return false;
    }
    string strId;
    if ( record.GetAttribute("ID", strId)) {
        m_MapIdToFeature[strId] = pFeature;
    }
    return true;
}

//  ----------------------------------------------------------------------------
bool CGff3Reader::xUpdateAnnotRegion(
    const CGff2Record& record,
    CRef<CSeq_feat> pFeature,
    CSeq_annot& annot,
    ILineErrorListener* pEC)
//  ----------------------------------------------------------------------------
{
    if (!record.InitializeFeature(m_iFlags, pFeature)) {
        return false;
    }

    if (! xAddFeatureToAnnot(pFeature, annot)) {
        return false;
    }
    string strId;
    if ( record.GetAttribute("ID", strId)) {
        mIdToSeqIdMap[strId] = record.Id();
        m_MapIdToFeature[strId] = pFeature;
    }
    return true;
}

//  ----------------------------------------------------------------------------
bool CGff3Reader::xAddFeatureToAnnot(
    CRef< CSeq_feat > pFeature,
    CSeq_annot& annot )
//  ----------------------------------------------------------------------------
{
    annot.SetData().SetFtable().push_back( pFeature ) ;
    return true;
}

//  ----------------------------------------------------------------------------
void CGff3Reader::xVerifyCdsParents(
    const CGff2Record& record)
//  ----------------------------------------------------------------------------
{
    string id;
    string parents;
    if (!record.GetAttribute("ID", id)) {
        return;
    }
    record.GetAttribute("Parent", parents);
    map<string, string>::iterator it = mCdsParentMap.find(id);
    if (it == mCdsParentMap.end()) {
        mCdsParentMap[id] = parents;
        return;
    }
    if (it->second == parents) {
        return;
    }
    CReaderMessage error(
        eDiag_Error,
        m_uLineNumber,
        "Bad data line: CDS record with bad parent assignments.");
    throw error;
}

//  ----------------------------------------------------------------------------
bool CGff3Reader::xReadInit()
//  ----------------------------------------------------------------------------
{
    if (!CGff2Reader::xReadInit()) {
        return false;
    }
    mCdsParentMap.clear();
    return true;
}

//  ----------------------------------------------------------------------------
bool CGff3Reader::xIsIgnoredFeatureType(
    const string& featureType)
//  ----------------------------------------------------------------------------
{
    typedef CStaticArraySet<string, PNocase> STRINGARRAY;

    string ftype(CSoMap::ResolveSoAlias(featureType));

    static const char* const ignoredTypesAlways_[] = {
        "protein"
    };
    DEFINE_STATIC_ARRAY_MAP(STRINGARRAY, ignoredTypesAlways, ignoredTypesAlways_);    
    STRINGARRAY::const_iterator cit = ignoredTypesAlways.find(ftype);
    if (cit != ignoredTypesAlways.end()) {
        return true;
    }
    if (!IsInGenbankMode()) {
        return false;
    }

    /* -genbank mode:*/
    static const char* const specialTypesGenbank_[] = {
        "antisense_RNA",
        "autocatalytically_spliced_intron",
        "guide_RNA",
        "hammerhead_ribozyme",
        "lnc_RNA",
        "miRNA",
        "piRNA",
        "rasiRNA",
        "ribozyme",
        "RNase_MRP_RNA",
        "RNase_P_RNA",
        "scRNA",
        "selenocysteine",
        "siRNA",
        "snoRNA",
        "snRNA",
        "SRP_RNA",
        "stop_codon_read_through",
        "telomerase_RNA",
        "vault_RNA",
        "Y_RNA"
    };
    DEFINE_STATIC_ARRAY_MAP(STRINGARRAY, specialTypesGenbank, specialTypesGenbank_);    

    static const char* const ignoredTypesGenbank_[] = {
        "apicoplast_chromosome",
        "assembly",
        "cDNA_match",
        "chloroplast_chromosome",
        "chromoplast_chromosome",
        "chromosome",
        "contig",
        "cyanelle_chromosome",
        "dna_chromosome",
        "EST_match",
        "expressed_sequence_match",
        "intron",
        "leucoplast_chromosome",
        "macronuclear_chromosome",
        "match",
        "match_part",
        "micronuclear_chromosome",
        "mitochondrial_chromosome",
        "nuclear_chromosome",
        "nucleomorphic_chromosome",
        "nucleotide_motif",
        "nucleotide_to_protein_match",
        "partial_genomic_sequence_assembly",
        "protein_match",
        "replicon",
        "rna_chromosome",
        "sequence_assembly",
        "supercontig",
        "translated_nucleotide_match",
        "ultracontig",
    };
    DEFINE_STATIC_ARRAY_MAP(STRINGARRAY, ignoredTypesGenbank, ignoredTypesGenbank_);    

    cit = specialTypesGenbank.find(ftype);
    if (cit != specialTypesGenbank.end()) {
        return false;
    }

    cit = ignoredTypesGenbank.find(ftype);
    if (cit != ignoredTypesGenbank.end()) {
        return true;
    }

    return false;
}

//  ----------------------------------------------------------------------------
bool
CGff3Reader::xInitializeFeature(
    const CGff2Record& record,
    CRef<CSeq_feat> pFeature)
//  ----------------------------------------------------------------------------
{
    if (!record.InitializeFeature(m_iFlags, pFeature)) {
        return false;
    }
    const auto& attrs = record.Attributes();
    const auto it = attrs.find("ID");
    if (it != attrs.end()) {
        mIdToSeqIdMap[it->second] = record.Id();
    }
    return true;
}

//  ----------------------------------------------------------------------------
void
CGff3Reader::xAddPendingExon(
    const string& rnaId,
    const CGff2Record& exonRecord)
//  ----------------------------------------------------------------------------
{
    PENDING_EXONS::iterator it = mPendingExons.find(rnaId);
    if (it == mPendingExons.end()) {
        mPendingExons[rnaId] = list<CGff2Record>();
    }
    mPendingExons[rnaId].push_back(exonRecord);
}

//  ----------------------------------------------------------------------------
void
CGff3Reader::xGetPendingExons(
    const string& rnaId,
    list<CGff2Record>& pendingExons)
//  ----------------------------------------------------------------------------
{
    PENDING_EXONS::iterator it = mPendingExons.find(rnaId);
    if (it == mPendingExons.end()) {
        return;
    }
    pendingExons.swap(mPendingExons[rnaId]);
    mPendingExons.erase(rnaId);
}

//  ----------------------------------------------------------------------------
void CGff3Reader::xPostProcessAnnot(
    CSeq_annot& annot)
//  ----------------------------------------------------------------------------
{
    if (mAlignmentData) {
        xProcessAlignmentData(annot);
        return;
    }
    if (!mCurrentFeatureCount) {
        return;
    }

    for (const auto& it: mPendingExons) {
        CReaderMessage warning(
            eDiag_Warning,
            m_uLineNumber,
            "Bad data line: Record references non-existant Parent=" + it.first);
        m_pMessageHandler->Report(warning);
    }
    return CGff2Reader::xPostProcessAnnot(annot);
}

//  ---------------------------------------------------------------------------
bool
CGff3Reader::xNeedsNewSeqAnnot(
    const string& line)
//  ---------------------------------------------------------------------------
{
    if (IsInGenbankMode()) {
        vector<string> columns;
        NStr::Split(line, "\t ", columns, NStr::eMergeDelims);
        string seqId = columns[0];
        if (m_CurrentSeqId == seqId) {
            return false;
        }
        m_CurrentSeqId = seqId;
        if (mCurrentFeatureCount == 0) {
            return false;
        }
        m_PendingLine = line;
        return true;
    }
    return false;
}


END_objects_SCOPE
END_NCBI_SCOPE
