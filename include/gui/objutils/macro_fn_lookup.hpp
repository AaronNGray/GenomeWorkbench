#ifndef GUI_OBJUTILS___MACRO_FN_LOOKUP__HPP
#define GUI_OBJUTILS___MACRO_FN_LOOKUP__HPP
/*  $Id: macro_fn_lookup.hpp 41476 2018-08-02 20:44:57Z asztalos $
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
 * Authors: Andrea Asztalos
 *
 * File Description: Macro functions that look up taxnames
 *
 */

/// @file macro_fn_lookup.hpp

#include <corelib/ncbistd.hpp>
#include <objects/taxon3/T3Reply.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/macro_edit_fn_base.hpp>


/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

class NCBI_GUIOBJUTILS_EXPORT CUnculturedTool
{
public:
    CUnculturedTool()
        : m_Bsrc(0), m_ConvertNote(false), m_RemoveNote_Sp(false), m_RemoveNote_Univ(false) {}

    CUnculturedTool(const objects::CBioSource& bsrc)
        : m_Bsrc(ConstRef(&bsrc)),
        m_ConvertNote(false),
        m_RemoveNote_Sp(false),
        m_RemoveNote_Univ(false) {}

    ~CUnculturedTool() {}

    void SetBioSource(const objects::CBioSource& bsrc) { m_Bsrc.Reset(&bsrc); }
    string GetSuggestedCorrection(void);

    void CorrectBioSource(objects::CBioSource& bsrc, bool& converted_note, bool& removed_note_sp, bool& removed_note_univ);

    static string s_MakeUnculturedName(const string& taxname, const string& suffix = kEmptyStr);
    static string s_GetRank(CRef<objects::CT3Reply> reply);
    static bool s_IsAmbiguous(CRef<objects::CT3Reply> reply);
    static bool s_OrganismNotFound(CRef<objects::CT3Reply> reply);
    static string s_GetSuggestion(CRef<objects::CT3Reply> reply);
    static bool s_CompareOrgnameLineage(CRef<objects::CT3Reply> reply, const string &lineage);
    static void s_AddEnvironmentalSample(objects::CBioSource& bsrc);
    static bool s_OkToTaxFix(const string& taxname);
    static bool s_IsUnculturedName(const string& taxname);
    static bool s_HasTaxId(const objects::CBioSource& bsrc);
private:
    void x_Reset();
    bool x_ShouldLookupTaxname(void);

    string x_GetCorrection();
    string x_StandardFixes();
    CRef<objects::CT3Reply> x_GetReply(const string &standard_taxname);
    bool x_CheckSuggestedFix(string& suggestion);
    string x_TryRankFix(CRef<objects::CT3Reply> reply, string& name);
    void x_ConvertNote(objects::CBioSource& biosource, const char* note_list[], const string& new_note);

private:
    CConstRef<objects::CBioSource> m_Bsrc;
    map<string, CRef<objects::CT3Reply> > m_ReplyCache;
    /// flag to convert species-specific note
    bool m_ConvertNote;
    /// flag to remove species-specific note
    bool m_RemoveNote_Sp;
    /// flag to remove universal note
    bool m_RemoveNote_Univ;

    CUnculturedTool(const CUnculturedTool&);
    CUnculturedTool& operator=(const CUnculturedTool&);
};



class NCBI_GUIOBJUTILS_EXPORT CCulturedTool
{
public:
    CCulturedTool()
        : m_Bsrc(0) {}

    CCulturedTool(const objects::CBioSource& bsrc)
        : m_Bsrc(ConstRef(&bsrc)) {}

    ~CCulturedTool() {}

    void SetBioSource(const objects::CBioSource& bsrc) { m_Bsrc.Reset(&bsrc); }

    void CorrectBioSource(objects::CBioSource& bsrc, bool& remove_note);

private:
    CConstRef<objects::CBioSource> m_Bsrc;

    bool x_RemoveIfFoundSubSourceNote(objects::CBioSource &bsrc, const char* note_list[]);
    bool x_RemoveIfFoundOrgModNote(objects::CBioSource &bsrc, const char* note_list[]);

    CCulturedTool(const CCulturedTool&);
    CCulturedTool& operator=(const CCulturedTool&);
};


DECLARE_FUNC_CLASS(CMacroFunction_UnculturedTaxLookup)

DECLARE_FUNC_CLASS(CMacroFunction_CulturedTaxLookup)


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_FN_LOOKUP__HPP
