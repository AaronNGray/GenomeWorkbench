#ifndef GUI_WIDGETS_EDIT___MOLINFO_FIELDTYPE__HPP
#define GUI_WIDGETS_EDIT___MOLINFO_FIELDTYPE__HPP
/*  $Id: molinfo_fieldtype.hpp 42116 2018-12-21 18:26:17Z asztalos $
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
 * Authors:  Andrea Asztalos
 */


#include <corelib/ncbistd.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/MolInfo.hpp>
#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CMolInfoFieldType
{
public:
    enum EMolInfoFieldType {
        e_MoleculeType = 0,
        e_Technique,
        e_Completedness,
        e_Class,
        e_Topology,
        e_Strand,
        e_Repr, // it is not part of the 'Molinfo' field
        e_Unknown
    };

    static vector<string> GetFieldNames();
    static vector<string> GetMacroFieldNames();
    static string GetFieldName(EMolInfoFieldType field_type);
    static string GetMacroFieldName(EMolInfoFieldType field_type);
    static EMolInfoFieldType GetFieldType(const string& field_name);

    static vector<string> GetChoicesForField(CMolInfoFieldType::EMolInfoFieldType field_type, bool& allow_other);
    static vector<string> GetValuesForField(EMolInfoFieldType field_type);
    static string GetBiomolLabel(objects::CMolInfo::TBiomol biomol);
    static objects::CMolInfo::TBiomol GetBiomolFromLabel(const string& val);
    static string GetTechLabel(objects::CMolInfo::TTech tech);
    static objects::CMolInfo::TTech GetTechFromLabel(const string& val);
    static string GetCompletenessLabel(objects::CMolInfo::TCompleteness tech);
    static objects::CMolInfo::TCompleteness GetCompletenessFromLabel(const string& val);
    static string GetMolLabel(objects::CSeq_inst::TMol val);
    static objects::CSeq_inst::TMol GetMolFromLabel(const string& val);
    static string GetTopologyLabel(objects::CSeq_inst::TTopology tech);
    static objects::CSeq_inst::TTopology GetTopologyFromLabel(const string& val);
    static string GetStrandLabel(objects::CSeq_inst::TStrand val);
    static objects::CSeq_inst::TStrand GetStrandFromLabel(const string& val);
    static objects::CSeq_inst::TRepr GetReprFromLabel(const string& val);
    static string GetReprLabel(objects::CSeq_inst::TRepr val);
};
END_NCBI_SCOPE

#endif 
    // GUI_WIDGETS_EDIT___MOLINFO_FIELDTYPE__HPP


