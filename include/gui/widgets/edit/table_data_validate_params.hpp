#ifndef GUI_WIDGETS_EDIT___TABLE_DATA_VALIDATE_PARAMS__HPP
#define GUI_WIDGETS_EDIT___TABLE_DATA_VALIDATE_PARAMS__HPP

/*  $Id: table_data_validate_params.hpp 32200 2015-01-20 18:21:29Z katargir $
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
 * Authors: Dmitry Rudnev
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <objects/general/User_object.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CTableDataValidateParams 
{
public:
    static void SetDoAlignment(objects::CUser_object& params, bool val);
    static bool GetDoAlignment(const objects::CUser_object& params);
    static void SetDoInference(objects::CUser_object& params, bool val);
    static bool GetDoInference(const objects::CUser_object& params);
protected:
    static void SetNamedBoolField(objects::CUser_object& params, const string& field_name, bool val);
    static bool GetNamedBoolField(const objects::CUser_object& params, const string& field_name);
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___TABLE_DATA_VALIDATE_PARAMS__HPP
