#ifndef GUI_WIDGETS_EDIT___PUB_FIELDTYPE__HPP
#define GUI_WIDGETS_EDIT___PUB_FIELDTYPE__HPP
/*  $Id: pub_fieldtype.hpp 41223 2018-06-15 13:49:08Z asztalos $
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

class NCBI_GUIWIDGETS_EDIT_EXPORT CPubFieldType
{
public:
    enum EPubFieldType {
        ePubFieldType_Title = 0,
        ePubFieldType_AffilAffil,
        ePubFieldType_AffilDept,
        ePubFieldType_AffilStreet,
        ePubFieldType_AffilCity,
        ePubFieldType_AffilState,
        ePubFieldType_AffilCountry,
        ePubFieldType_AffilEmail,
        ePubFieldType_AffilFax,
        ePubFieldType_AffilPhone,
        ePubFieldType_AffilPostalCode,
        ePubFieldType_AuthorFirstName,
        ePubFieldType_AuthorMiddleInitial,
        ePubFieldType_AuthorLastName,
        ePubFieldType_AuthorSuffix,
        ePubFieldType_Authors,
        ePubFieldType_AuthorConsortium,
        ePubFieldType_Journal,
        ePubFieldType_Volume,
        ePubFieldType_Date,
        ePubFieldType_DateYear,
        ePubFieldType_DateMonth,
        ePubFieldType_DateDay,
        ePubFieldType_Issue,
        ePubFieldType_Pages,
        ePubFieldType_SerialNumber,
        ePubFieldType_Citation,
        ePubFieldType_Status,
        ePubFieldType_Pmid,
        ePubFieldType_Unknown
    };

    enum EPubFieldStatus {
        ePubFieldStatus_Published = 0,
        ePubFieldStatus_Unpublished,
        ePubFieldStatus_InPress,
        ePubFieldStatus_Submit,
        ePubFieldStatus_Any
    };

    static vector<string> GetFieldNames(bool extended = false);
    static vector<string> GetMacroFieldNames();
    static EPubFieldType GetTypeForLabel(string label);
    static string GetLabelForType(EPubFieldType field_type);
    static string GetMacroLabelForType(EPubFieldType field_type);
    static string NormalizePubFieldName(string orig_label);
    static bool IsAffilField(EPubFieldType field_type);
    static bool IsAuthorField(EPubFieldType field_type);

    static vector<string> GetChoicesForField(EPubFieldType field_type, bool& allow_other);
    static vector<string> GetValuesForField(EPubFieldType field_type);
    static string GetLabelForStatus(EPubFieldStatus status);
    static EPubFieldStatus GetStatusFromString(const string& str);
};
END_NCBI_SCOPE

#endif 
    // GUI_WIDGETS_EDIT___PUB_FIELDTYPE__HPP


