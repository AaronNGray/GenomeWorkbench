#ifndef GUI_WIDGETS_EDIT___BIOSOURCE_AUTOCOMPLETE__HPP
#define GUI_WIDGETS_EDIT___BIOSOURCE_AUTOCOMPLETE__HPP

/*  $Id: biosource_autocomplete.hpp 37422 2017-01-09 17:10:43Z bollin $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <objects/seqfeat/Org_ref.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CBioSourceAutoComplete 
{
public:
    CBioSourceAutoComplete();
    ~CBioSourceAutoComplete();

    static bool AutoFill (objects::COrg_ref& org);


};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___USERFIELD_STRINGLIST_VALIDATOR__HPP
