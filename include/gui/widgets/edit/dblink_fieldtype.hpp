#ifndef GUI_WIDGETS_EDIT___DBLINK_FIELDTYPE__HPP
#define GUI_WIDGETS_EDIT___DBLINK_FIELDTYPE__HPP
/*  $Id: dblink_fieldtype.hpp 39649 2017-10-24 15:22:12Z asztalos $
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

#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CDBLinkFieldType
{
public:
    enum EDBLinkFieldType {
        eDBLinkFieldType_Trace = 0,
        eDBLinkFieldType_BioSample,
        eDBLinkFieldType_ProbeDB,
        eDBLinkFieldType_SRA,
        eDBLinkFieldType_BioProject,
        eDBLinkFieldType_Assembly,
        eDBLinkFieldType_Unknown
    };


    static vector<string> GetFieldNames();
    static EDBLinkFieldType GetTypeForLabel(string label);
    static string GetLabelForType(EDBLinkFieldType field_type);
    static void NormalizeDBLinkFieldName(string& orig_label);
};
END_NCBI_SCOPE

#endif 
    // GUI_WIDGETS_EDIT___DBLINK_FIELDTYPE__HPP


