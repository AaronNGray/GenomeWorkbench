#ifndef GUI_OBJUTILS___PROJECT_ITEM_EXTRA__HPP
#define GUI_OBJUTILS___PROJECT_ITEM_EXTRA__HPP

/*  $Id: project_item_extra.hpp 35093 2016-03-22 18:51:23Z katargir $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CProjectItem;
END_SCOPE(objects)

class NCBI_GUIOBJUTILS_EXPORT CProjectItemExtra
{
public:
    static bool GetStr(const objects::CProjectItem& pi, const string& tag, string& value);
    static void SetStr(objects::CProjectItem& pi, const string& tag, const string& value);
};


END_NCBI_SCOPE


#endif  // GUI_OBJUTILS___PROJECT_ITEM_EXTRA__HPP
