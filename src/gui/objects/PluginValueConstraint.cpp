/* $Id: PluginValueConstraint.cpp 14666 2007-07-09 13:40:22Z dicuccio $
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
 * Author:  .......
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using specifications from the data definition file
 *   'plugin.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>
#include <gui/objects/plugin_exception.hpp>

// generated includes
#include <gui/objects/PluginValueConstraint.hpp>
#include <gui/objects/PluginValueRangeConstraint.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CPluginValueConstraint::~CPluginValueConstraint(void)
{
}


// static creators
CPluginValueConstraint*
CPluginValueConstraint::CreateLower(const string& low_val)
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetLower(low_val);
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateUpper(const string& hi_val)
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetUpper(hi_val);
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateRange(const string& low_val,
                                  const string& hi_val)
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetRange().SetLower(low_val);
    con->SetRange().SetUpper(hi_val);
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateSet(void)
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetSet();
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateSeqRepr(void)
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetSeq_repr();
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateSeqMol(void)
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetSeq_mol();
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateSeqSameMol()
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetSeq_mol_same_type();
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateSeqLenLower(TSeqPos min_len)
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetSeq_length_lower(min_len);
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateSeqLenUpper(TSeqPos max_len)
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetSeq_length_upper(max_len);
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateSeqLenRange(TSeqPos min_len, TSeqPos max_len)
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    CPluginValueRangeConstraint& range = con->SetSeq_length_range();
    range.SetLower(NStr::IntToString(min_len));
    range.SetUpper(NStr::IntToString(max_len));
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateAnnotType(CSeq_annot::TData::E_Choice type)
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetAnnot_type(type);
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateFeatType()
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetFeat_type();
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateFeatSubtype()
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetFeat_subtype();
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateFeatProduct()
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetFeat_has_product();
    return con.Release();
}


CPluginValueConstraint*
CPluginValueConstraint::CreateProjectItemType(const CTypeInfo* info)
{
    CRef<CPluginValueConstraint> con(new CPluginValueConstraint());
    con->SetProject_item_type(info->GetName());
    return con.Release();
}


//
// operator, for adding values
//
CPluginValueConstraint& CPluginValueConstraint::operator, (const string& val)
{
    if (Which() != e_Set) {
        NCBI_THROW(CPluginException, eInvalidArg,
                   "CPluginValueConstraint::operator,() is only valid for "
                   "set constraints");
    }

    SetSet().push_back(val);
    return *this;
}


//
// operator, for adding values to non-set types
//
CPluginValueConstraint& CPluginValueConstraint::operator, (int val)
{
    switch (Which()) {
    case e_Seq_mol:
        SetSeq_mol().push_back(val);
        return *this;

    case e_Seq_repr:
        SetSeq_repr().push_back(val);
        return *this;

    case e_Feat_type:
        SetFeat_type().push_back(val);
        return *this;

    case e_Feat_subtype:
        SetFeat_subtype().push_back(val);
        return *this;

    default:
        NCBI_THROW(CPluginException, eInvalidArg,
                   "CPluginValueConstraint::operator,(int): "
                   "inavlid constraint type");
    }
}


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 64, chars: 1912, CRC32: ba941e07 */
