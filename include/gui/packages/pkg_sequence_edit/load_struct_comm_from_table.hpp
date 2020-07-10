/*  $Id: load_struct_comm_from_table.hpp 40927 2018-05-01 17:14:12Z katargir $
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
 * Authors:  Igor Filippov
 */
#ifndef _LOAD_STRUCT_COMMENTS_H_
#define _LOAD_STRUCT_COMMENTS_H_

#include <corelib/ncbistd.hpp>
#include <gui/objutils/cmd_composite.hpp>

#include <wx/window.h>
#include <wx/string.h>

BEGIN_NCBI_SCOPE


class CLoadStructComments
{
public:
    static CRef<CCmdComposite> apply(wxWindow *parent, const wxString& cwd, objects::CSeq_entry_Handle tse);
private:
    static CRef<CCmdComposite> LoadWithAccessions(objects::CSeq_entry_Handle tse, const wxString& file);
    static CRef<CCmdComposite> LoadSingle(objects::CSeq_entry_Handle tse, const wxString& file);
};

END_NCBI_SCOPE

#endif
    // _LOAD_STRUCT_COMMENTS_H_
