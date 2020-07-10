#ifndef GUI_WIDGETS_EDIT___CITSUB_UPDATER__HPP
#define GUI_WIDGETS_EDIT___CITSUB_UPDATER__HPP
/*  $Id: citsub_updater.hpp 41956 2018-11-20 18:42:42Z asztalos $
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
#include <objmgr/bioseq_handle.hpp>
#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CScope;
    class CSeqdesc;
END_SCOPE(objects)


class NCBI_GUIWIDGETS_EDIT_EXPORT CCitSubUpdater
{
public:
    /// Generates/Creates a Cit-sub publication that will be added to the sequence
    ///
    ///@param[in] bsh
    ///  Sequence that will be receiving a Cit-sub publication
    ///@param[out] message 
    ///  Information presented to the user
    ///@param[out] changedSeqdesc
    ///  If it's not null, it points to an existing publication that have to be removed before the new one is added
    ///@return 
    ///  A publication descriptor that will be added to the sequence

    static CRef<objects::CSeqdesc> s_GetCitSubForUpdatedSequence(const objects::CBioseq_Handle& bsh,
        string& message, CConstRef<objects::CSeqdesc>& changedSeqdesc, objects::CSeq_entry_Handle& seh);

    static CRef<objects::CSeqdesc> s_GetCitSubForTrimmedSequence(const objects::CBioseq_Handle& bsh,
        string& message, CConstRef<objects::CSeqdesc>& changedSeqdesc, objects::CSeq_entry_Handle& seh);

    static const char* sm_SubmitterUpdateText;
    static const char* sm_NoEarlierCitSub;
    static const char* sm_ChangeExistingCitSub;
    static const char* sm_ExistingCitSub;
    static const char* sm_AddedCitSub;
    static const char* sm_DifferentCitSub;

    static const char* sm_TrimmedText;
private:
    static CRef<objects::CSeqdesc> sx_GetCitSubForSequence(const objects::CBioseq_Handle& bsh,
            string& message, CConstRef<objects::CSeqdesc>& changedSeqdesc, objects::CSeq_entry_Handle& seh, const CTempString& text);
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___CITSUB_UPDATER__HPP
