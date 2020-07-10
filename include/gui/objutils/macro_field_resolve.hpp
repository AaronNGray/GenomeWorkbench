#ifndef GUI_OBJUTILS___MACRO_FIELD_RESOLVE__HPP
#define GUI_OBJUTILS___MACRO_FIELD_RESOLVE__HPP
/*  $Id: macro_field_resolve.hpp 37809 2017-02-17 15:50:55Z asztalos $
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
 * Authors: Andrea Asztalos
 *
 * File Description: 
 *    Functions used for resolving fields based on their names for editing, removal or setting
 */

/// @file macro_field_resolve.hpp
/// Functions that resolve field names described in asn format

#include <corelib/ncbiobj.hpp>
#include <serial/objectinfo.hpp>
#include <gui/objutils/macro_exec.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <objects/seq/MolInfo.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

// Functions GetFieldsByName(), SetFieldsByName() and RemoveFieldByName() should not be part of the IMacroBioDataIter
// interface, as in many instances, the object information instance is not related to the biodata iterator

/// Resolve existing dot qualified ASN.1 name (field_name) starting from the object information instance 
/// provided as a parameter (oi_i). The result is the list of CObjectInfo pairs, composed of the field and its parent.
///
/// @return false if the name cannot be resolved.
/// @return true and an empty results if the field is valid, but has not been set
NCBI_GUIOBJUTILS_EXPORT bool GetFieldsByName(CMQueryNodeValue::TObs* results, const CObjectInfo& oi_i, const string& field_name);

/// Resolve not necessarily existing dot qualified ASN.1 name (field_name) starting from the 
/// object information instance provided as a parameter (oi_i).
///
/// @return false if the name cannot be resolved.
NCBI_GUIOBJUTILS_EXPORT bool SetFieldsByName(CMQueryNodeValue::TObs* results, CObjectInfo& oi_i, const string& field_name);

/// Remove the object information instance corresponding to field, using information about its parent node
/// @return true if the field was removed
NCBI_GUIOBJUTILS_EXPORT bool RemoveFieldByName(CMQueryNodeValue::SResolvedField& res_field);

/// Resolve dot qualified ASN.1 node_name using the object information instance 
/// provided as a parameter (oi) and sets node data.
NCBI_GUIOBJUTILS_EXPORT bool ResolveAndSetSimpleTypeValue(CObjectInfo& oi, 
    const string& field_name, 
    const CMQueryNodeValue& value, 
    objects::edit::EExistingText existing_text = objects::edit::eExistingText_replace_old);

/// Set single node data to the node specified by parameter of type CObjectInfo.
NCBI_GUIOBJUTILS_EXPORT bool SetSimpleTypeValue(CObjectInfo& oi, 
    const CMQueryNodeValue& value, 
    objects::edit::EExistingText existing_text = objects::edit::eExistingText_replace_old);

/// Get single node data from the node specified by parameter of type CObjectInfo 
/// and additionally field field_name (set to empty string if oi is already at primitive value)
NCBI_GUIOBJUTILS_EXPORT bool GetSimpleTypeValue(CObjectInfo& oi, const string& field_name, CMQueryNodeValue& value);
    
/// Get a primitive value from oi
NCBI_GUIOBJUTILS_EXPORT bool GetPrimitiveValue(const CObjectInfo& oi, CMQueryNodeValue& value);
    
/// Resolve name to simple types value
NCBI_GUIOBJUTILS_EXPORT bool ResolveIdentToSimple(const CObjectInfo& oi, const string& identifier, CMQueryNodeValue& v);

/// Resolve name to the list of objects
NCBI_GUIOBJUTILS_EXPORT bool ResolveIdentToObjects(const CObjectInfo& oi, const string& identifier, CMQueryNodeValue& v);
   
END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_FIELD_RESOLVE__HPP
