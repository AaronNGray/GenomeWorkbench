#ifndef GUI_OBJUTILS___ATTACHED_HANDLES__HPP
#define GUI_OBJUTILS___ATTACHED_HANDLES__HPP

/*  $Id: attached_handles.hpp 44371 2019-12-05 22:13:10Z joukovv $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/seq_annot_handle.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CScope;
	class CProjectItem;
END_SCOPE(objects)


class NCBI_GUIOBJUTILS_EXPORT CAttachedHandles : public CObject
{
public:
	CAttachedHandles() {}

	void Attach(objects::CScope& scope, objects::CProjectItem& item,
        bool annot_only = false);
	void Detach(objects::CScope& scope);

private:
    vector<objects::CSeq_entry_Handle> m_Seq_entry_Handles;
    vector<objects::CSeq_annot_Handle> m_Seq_annot_Handles;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___ATTACHED_HANDLES__HPP
