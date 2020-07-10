/*  $Id: strain_serotype_influenza.hpp 42821 2019-04-18 19:32:56Z joukovv $
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

#ifndef _ADD_STRAIN_SEROTYPE_H_
#define _ADD_STRAIN_SEROTYPE_H_

#include <corelib/ncbistd.hpp>
#include <gui/packages/pkg_sequence_edit/modify_bsrc_interface.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CStrainSerotypeInfluenza : public IModifyBiosourceQuals
{
public:
    enum ECommandInfluenza{ 
        eCommandInfluenza_parse,  // parse strain and serotype from the influenza name
        eCommandInfluenza_add,    // add strain and serotype to the taxname
        eCommandInfluenza_fixup,  // clean white spaces around the strain and serotype
        eCommandSplitQualsAtCommas,         // split orgmod qualifiers at commas - not necessarily influenza viruses
        eCommandSplitStructuredCollections, //split structures collections at semicolon
        eCommandTrimOrganismNames           // trim organism names
    };
    
    CStrainSerotypeInfluenza(ECommandInfluenza type)
        : m_Type(type)
    {}
    
    CRef<CCmdComposite> GetCommand(objects::CSeq_entry_Handle tse, const string& title);

    NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT friend CRef<CCmdComposite> 
        TestCStrainSerotypeInfluenza_Parse(CStrainSerotypeInfluenza& parser, objects::CSeq_entry_Handle tse, const string& title, bool overwrite);

private:
    virtual bool x_ApplyToBioSource(objects::CBioSource& biosource);
    
    bool x_AddToInfluenzaNames (objects::CBioSource& biosource);
    bool x_FixupOrganismNames (objects::CBioSource& biosource);
    bool x_SplittingQualifiersAtCommas(objects::CBioSource& biosource);
    bool x_SplittingStructCollections(objects::CBioSource& biosource);
    bool x_TrimmingOrganismNames(objects::CBioSource& biosource);

    void x_ParseStrainSerotype(CCmdComposite* composite);
    void x_ParseStrainSerotype(CCmdComposite* composite, bool overwrite);

    void x_TestEntryForParsing(const objects::CSeq_entry& se, int& count);
    void x_TestBiosourceForParsing(const objects::CBioSource& biosource, int& count);

    bool x_IsParsableInfluenzaVirusName(const objects::CBioSource& biosource, string& strain, string& serotype);
    int  x_GetStrainSerotypeConflicts(const objects::CBioSource& biosource, const string& strain, const string& serotype);

    void x_ParseEntry(CCmdComposite* composite, bool update);
    void x_ParseDescriptors(const objects::CSeq_entry& se, CCmdComposite* composite, bool update);
    bool x_ParseBiosource(objects::CBioSource& biosource, bool update);
    bool x_UpdateStrainSerotype(objects::CBioSource& biosource, const string& strain, const string& serotype, bool update);
    
    ECommandInfluenza m_Type;
    bool m_Parsable{ false };
};


END_NCBI_SCOPE

#endif
// _ADD_STRAIN_SEROTYPE_H_

