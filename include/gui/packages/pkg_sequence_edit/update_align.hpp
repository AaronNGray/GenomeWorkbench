/*  $Id: update_align.hpp 37316 2016-12-22 19:24:06Z asztalos $
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
#ifndef _UPDATE_ALIGN_H_
#define _UPDATE_ALIGN_H_

#include <corelib/ncbistd.hpp>
#include <gui/objutils/cmd_composite.hpp>

#include <wx/window.h>
#include <wx/string.h>

BEGIN_NCBI_SCOPE

class CTextAlignParams;

class CUpdateAlign
{
public:
    typedef vector<pair<string, int>> TIdToRowVec;

    void apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, wxWindow *parent);
    
private:
    void x_ReadAlign(const CTextAlignParams& params);

    void x_FindNotPresentIDs(objects::CSeq_entry_Handle tse);

    bool x_ProcessNotPresentIDs(wxWindow *parent);

    void x_RemoveIDsFromAlign();

    void x_ChangeIDsToFarPointers();

    void x_MapNonPresentIDs(wxWindow *parent);
   
//    static int FindMostFrequentLength(const map<string,string> &id_to_seq);
//    static bool ProcessDiffLengths(map<string,string> &id_to_seq, int length);
private:

    wxString m_FilePath;
    vector<string> m_Ids;

    CRef<objects::CSeq_align> m_SeqAlign;
    TIdToRowVec m_NonPresentIDs;
};

END_NCBI_SCOPE

#endif
    // _UPDATE_ALIGN_H_
