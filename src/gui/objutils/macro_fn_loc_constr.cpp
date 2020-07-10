/*  $Id: macro_fn_loc_constr.cpp 44446 2019-12-19 16:23:05Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 */

#include <ncbi_pch.hpp>
#include <sstream>
#include <gui/objutils/macro_field_resolve.hpp>
#include <gui/objutils/macro_fn_loc_constr.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

/// class CMacroFunction_LocPartialTest - tests whether the location is 5'/3' partial
/// ISPARTIALSTART() - returns true if it is 5' partial
/// ISPARTIALSTOP() - returns true if it is 3' partial
///
const char* CMacroFunction_LocPartialTest::sm_PartialStart = "ISPARTIALSTART";
const char* CMacroFunction_LocPartialTest::sm_PartialStop = "ISPARTIALSTOP";

void CMacroFunction_LocPartialTest::TheFunction()
{
    m_Result->SetNotSet();
    CObjectInfo oi;

    if (m_Args.empty()) {
        oi = m_DataIter->GetEditedObject();
    }
    else {
        CMQueryNodeValue::TObs res_oi = m_Args[0]->GetObjects();
        if (res_oi.size() != 1)
            return;
        oi = res_oi.front().field;
    }

    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!edit_feat || !scope)
        return;


    const CSeq_loc& loc = edit_feat->GetLocation();
    switch (m_TestCase) {
    case ELocEndType::eStart:
        m_Result->SetDataType(CMQueryNodeValue::eBool);
        m_Result->SetBool(loc.IsPartialStart(eExtreme_Biological));
        break;
    case ELocEndType::eStop:
        m_Result->SetDataType(CMQueryNodeValue::eBool);
        m_Result->SetBool(loc.IsPartialStop(eExtreme_Biological));
        break;
    default:
        break;
    }
}

bool CMacroFunction_LocPartialTest::x_ValidArguments() const
{
    return (m_Args.empty() ||
        (m_Args.size() == 1 && (m_Args[0]->GetDataType() == CMQueryNodeValue::eObjects || m_Args[0]->GetDataType() == CMQueryNodeValue::eNotSet)));
}


//////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_LocEnd
/// Start() and Stop() - return the positional extremes of a location
///
const char* CMacroFunction_LocEnd::sm_Start = "START";
const char* CMacroFunction_LocEnd::sm_Stop = "STOP";
void CMacroFunction_LocEnd::TheFunction()
{
    m_Result->SetNotSet();
    CObjectInfo oi;

    oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!edit_feat || !scope)
        return;

    const CSeq_loc& loc = edit_feat->GetLocation();
    TSeqPos pos = 0;
    switch (m_TestCase) {
    case ELocEndType::eStart:
        pos = loc.GetStart(eExtreme_Positional);
        break;
    case ELocEndType::eStop:
        pos = loc.GetStop(eExtreme_Positional);
        break;
    default:
        break;
    }
    
    bool one_based = (m_Args.size() == 1) ? m_Args[0]->GetBool() : false;
    if (one_based) {
        pos++;
    }
    m_Result->SetInt(pos);
}

bool CMacroFunction_LocEnd::x_ValidArguments() const
{
    return (m_Args.empty() || (m_Args.size() == 1 && m_Args[0]->IsBool()));
}


//////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_LocationStrand - function returns the strand of the location
/// STRAND() - returns the corresponding string if it's set.
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_LocationStrand, "STRAND")
void CMacroFunction_LocationStrand::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    if (!edit_feat)
        return;

    m_Result->SetNotSet();

    const CSeq_loc& loc = edit_feat->GetLocation();
    if (loc.IsSetStrand()) {
        string strand = ENUM_METHOD_NAME(ENa_strand)()->FindName(loc.GetStrand(), true);
        m_Result->SetString(strand);
    }
}

bool CMacroFunction_LocationStrand::x_ValidArguments() const
{
    return (m_Args.empty());
}


//////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_LocationStrandSymbol - function returns the strand of the location
/// STRANDSYMBOL() - returns  "." (no strand) or "+" or "-".
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_LocationStrandSymbol, "STRANDSYMBOL")
void CMacroFunction_LocationStrandSymbol::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    if (!edit_feat)
        return;

    m_Result->SetNotSet();

    const CSeq_loc& loc = edit_feat->GetLocation();
    if (!loc.IsSetStrand()) {
        m_Result->SetString(".");
        return;
    }

    switch(loc.GetStrand()) {
        case objects::eNa_strand_plus :
            m_Result->SetString("+");
            break;
        case objects::eNa_strand_minus :
            m_Result->SetString("-");
            break;
        default:
            m_Result->SetString(".");
            break;
    }
}

bool CMacroFunction_LocationStrandSymbol::x_ValidArguments() const
{
    return (m_Args.empty());
}

//////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_LocationType - obtain information about the type of location
/// ISLOCATIONTYPE(loc_type) - returns true if type of location is loc_type
/// parameter "loc_type" can be one of: "single-interval", "ordered", "joined"
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_LocationType, "ISLOCATIONTYPE")
void CMacroFunction_LocationType::TheFunction() 
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    if (!edit_feat)
        return;

    const CSeq_loc& loc = edit_feat->GetLocation();
    const string& location_type = m_Args[0]->GetString();

    CRef<CLocation_constraint> loc_cons(new CLocation_constraint());
    if (NStr::EqualNocase(location_type, "single-interval")) {
        loc_cons->SetLocation_type(eLocation_type_constraint_single_interval);
    } else if (NStr::EqualNocase(location_type, "ordered")) {
        loc_cons->SetLocation_type(eLocation_type_constraint_ordered);
    } else if (NStr::EqualNocase(location_type, "joined")) {
        loc_cons->SetLocation_type(eLocation_type_constraint_joined);
    }

    bool match = x_Match(loc, *loc_cons);
    m_Result->SetDataType(CMQueryNodeValue::eBool);
    m_Result->SetBool(match);
}


bool CMacroFunction_LocationType::x_Match(const CSeq_loc& loc, const CLocation_constraint& loc_cons)
{
    bool has_null = false;
    int  num_intervals = 0;

    if (loc_cons.GetLocation_type() == eLocation_type_constraint_any)
        return false; // different than the one present in the toolkit

    for (CSeq_loc_CI loc_iter(loc); loc_iter; ++loc_iter) {
        if (loc_iter.GetEmbeddingSeq_loc().Which() == CSeq_loc::e_Null) {
            has_null = true;
        } else if (!loc_iter.IsEmpty()) {
            num_intervals ++;
        }
    }

    if (loc_cons.GetLocation_type() == eLocation_type_constraint_single_interval) {
        if (num_intervals == 1) {
            return true;
        }
    } else if (loc_cons.GetLocation_type() == eLocation_type_constraint_joined) {
        if (num_intervals > 1 && !has_null) {
            return true;
        }
    } else if (loc_cons.GetLocation_type() == eLocation_type_constraint_ordered) {
        if (num_intervals > 1 && has_null && loc.Which() == CSeq_loc::e_Mix) { // different
            return true;
        }
    }
  
  return false;
}

bool CMacroFunction_LocationType::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eString);
}

//////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_LocationSeqType
/// IS_SEQ_AA(asn_path_to_location) - returns true if the sequence identified by this location is a protein
/// IS_SEQ_NA(asn_path_to_location) - returns true if the sequence identified by this location is a nucleotide sequence
///
const char* CMacroFunction_LocationSeqType::sm_SeqNa = "IS_SEQ_NA";
const char* CMacroFunction_LocationSeqType::sm_SeqAa = "IS_SEQ_AA";
void CMacroFunction_LocationSeqType::TheFunction()
{
    const CSeq_loc* loc = s_GetLocation(m_Args[0]->GetString(), m_DataIter);
    if (!loc)
        return;

    SConstScopedObject sobject = m_DataIter->GetScopedObject();
    CBioseq_Handle bsh;
    try {
        bsh = sobject.scope->GetBioseqHandle(*loc);
    }
    catch (const CException&) {
        return;
    }
    
    bool value = false;
    if ((bsh.IsNucleotide() && m_Seqtype == eSeqtype_constraint_nuc) ||
        (bsh.IsProtein() && m_Seqtype == eSeqtype_constraint_prot)) { 
        value = true;
    }
    
    m_Result->SetDataType(CMQueryNodeValue::eBool);
    m_Result->SetBool(value);
}

bool CMacroFunction_LocationSeqType::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eString);
}


CSeq_loc* CMacroFunction_LocationSeqType::s_GetLocation(const string& field_name, CIRef<IMacroBioDataIter> iter)
{
    if (field_name.empty())
        return nullptr;

    CObjectInfo objInfo = iter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    if (!GetFieldsByName(&res_oi, objInfo, field_name) || res_oi.size() != 1)
        return nullptr;

    // location - is a pointer to a choice type
    if (res_oi.front().field.GetTypeFamily() == eTypeFamilyPointer) {
        CObjectInfo oi = res_oi.front().field.GetPointedObject();
        if (oi.GetTypeFamily() == eTypeFamilyChoice) {
            CSeq_loc* loc = CTypeConverter<CSeq_loc>::SafeCast(oi.GetObjectPtr());
            if (loc) {
                return loc;
            }
        }
    }
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_LocationDistConstraint 
/// DISTFROMSTART() [>][=][<] distance
/// DISTFROMSTOP()
///
const char* CMacroFunction_LocationDistConstraint::sm_FromStart = "DISTFROMSTART";
const char* CMacroFunction_LocationDistConstraint::sm_FromStop = "DISTFROMSTOP";
void CMacroFunction_LocationDistConstraint::TheFunction()
{
    m_Result->SetNotSet();
    CObjectInfo oi;

    if (m_Args.empty()) {
        oi = m_DataIter->GetEditedObject();
    }
    else {
        CMQueryNodeValue::TObs res_oi = m_Args[0]->GetObjects();
        if (res_oi.size() != 1)
            return;
        oi = res_oi.front().field;
    }

    CSeq_feat* feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!feat || !scope)
        return;

    const CSeq_loc& loc = feat->GetLocation();
    CBioseq_Handle bsh;
    try {
        bsh = scope->GetBioseqHandle(loc);
    }
    catch (const CException&) {
        return;
    }
    
    TSeqPos start = loc.GetStart(eExtreme_Positional);
    TSeqPos stop = loc.GetStop(eExtreme_Positional);
    ENa_strand strand = loc.GetStrand();
    int diff = 0; // difference between respective ends of feature and sequence

    switch (m_TestCase) {
    case ELocEndType::eStart:
        if (strand == eNa_strand_minus) {
            diff = bsh.GetBioseqLength() - 1 - stop;
        } else {
            diff = start;
        }
        break;
    case ELocEndType::eStop:
        if (strand == eNa_strand_minus) {
            diff = start;
        } else {
            diff = bsh.GetBioseqLength() - 1 - stop;
        }
        break;
    default:
        break;
    }
    
    m_Result->SetInt(diff);
}
 
bool CMacroFunction_LocationDistConstraint::x_ValidArguments() const
{
    return (m_Args.empty() || 
        (m_Args.size() == 1 && (m_Args[0]->GetDataType() == CMQueryNodeValue::eObjects || m_Args[0]->GetDataType() == CMQueryNodeValue::eNotSet)));
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
