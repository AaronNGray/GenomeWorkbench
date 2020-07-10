/*  $Id: pubdesc_editor.cpp 43888 2019-09-13 15:12:37Z asztalos $
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

#include <ncbi_pch.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>

#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objects/pubmed/Pubmed_entry.hpp>
#include <objects/medline/Medline_entry.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objtools/cleanup/cleanup_change.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <objtools/edit/publication_edit.hpp>
#include <objtools/edit/apply_object.hpp>

#include <gui/packages/pkg_sequence_edit/pubdesc_editor.hpp>
#include <gui/objutils/gui_eutils_client.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> IPubDescEditor::GetCommand(CSeq_entry_Handle tse, CConstRef<CSeq_submit> submit)
{
    if (!tse)
        return CRef<CCmdComposite>();

    m_tse = tse;
    CRef<CCmdComposite> composite(new CCmdComposite(m_Title));
    
    x_ApplyToSeqAndFeat(composite);
    if (submit && submit->IsSetSub()) {
        m_SubmitBlock.Reset(&(submit->GetSub()));
        x_ApplyToSeqSubmit(composite);
    }
    return composite;
}

void IPubDescEditor::x_ApplyToSeqAndFeat(CCmdComposite* composite)
{
    x_ApplyToDescriptors(*(m_tse.GetCompleteSeq_entry()), composite);

    for (CFeat_CI feat_it(m_tse, SAnnotSelector(CSeqFeatData::e_Pub)); feat_it; ++feat_it) {
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat_it->GetOriginalFeature());
        if (x_ApplyToPubdesc(new_feat->SetData().SetPub())) {
            CRef<CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(*feat_it, *new_feat));
            composite->AddCommand(*cmd);
        }
    }
}

void IPubDescEditor::x_ApplyToDescriptors(const CSeq_entry& se, CCmdComposite* composite)
{
    FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
        if ((*it)->IsPub()) {
            const CSeqdesc& orig_desc = **it;
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(orig_desc);
            if (x_ApplyToPubdesc(new_desc->SetPub())) {
                CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(m_tse.GetScope().GetSeq_entryHandle(se), orig_desc, *new_desc));
                composite->AddCommand(*cmd);
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet()) {
            x_ApplyToDescriptors(**it, composite);
        }
    }
}


bool CFixTitleCap::x_ApplyToPubdesc(CPubdesc& pubdesc)
{
    bool modified = false;
    EDIT_EACH_PUB_ON_PUBDESC(pub_iter, pubdesc) {
        CPub& pub = **pub_iter;
        string title;
        switch (pub.Which()) {
        case CPub::e_Article: {
            CCit_art& art = pub.SetArticle();
            if (art.IsSetTitle())
            {
                title = s_GetTitle(art.GetTitle());
                if (!NStr::IsBlank(title))
                {
                    modified = x_FixTitle(title);
                    if (modified)
                        s_SetTitle(art.SetTitle(), title);
                }
            }
        }
            break;
        case CPub::e_Gen:
            if (pub.GetGen().IsSetTitle()) {
                title = pub.GetGen().GetTitle();
                if (!NStr::IsBlank(title))
                {
                    modified = x_FixTitle(title);
                    if (modified)
                        pub.SetGen().SetTitle(title);
                }
            }
            break;
        case CPub::e_Sub:
            if (pub.GetSub().IsSetDescr()) {
                title = pub.GetSub().GetDescr();
                if (!NStr::IsBlank(title))
                {
                    modified = x_FixTitle(title);
                    if (modified)
                        pub.SetSub().SetDescr(title);
                }
            }
            break;
        case CPub::e_Patent:
            if (pub.GetPatent().IsSetTitle()) {
                title = pub.GetPatent().GetTitle();
                if (!NStr::IsBlank(title))
                {
                    modified = x_FixTitle(title);
                    if (modified)
                        pub.SetPatent().SetTitle(title);
                }
            }
            break;
        default:
            break;
        }
    }
    return modified;
}

string CFixTitleCap::s_GetTitle(const CTitle& title) // CValidError_imp::HasTitle(const CTitle& title) in valid_pub.cpp
{
    ITERATE(CTitle::Tdata, item, title.Get()) {
        const string *str = 0;
        switch ((*item)->Which()) {
        case CTitle::C_E::e_Name:
            str = &(*item)->GetName();
            break;

        case CTitle::C_E::e_Tsub:
            str = &(*item)->GetTsub();
            break;

        case CTitle::C_E::e_Trans:
            str = &(*item)->GetTrans();
            break;

        case CTitle::C_E::e_Jta:
            str = &(*item)->GetJta();
            break;

        case CTitle::C_E::e_Iso_jta:
            str = &(*item)->GetIso_jta();
            break;

        case CTitle::C_E::e_Ml_jta:
            str = &(*item)->GetMl_jta();
            break;

        case CTitle::C_E::e_Coden:
            str = &(*item)->GetCoden();
            break;

        case CTitle::C_E::e_Issn:
            str = &(*item)->GetIssn();
            break;

        case CTitle::C_E::e_Abr:
            str = &(*item)->GetAbr();
            break;

        case CTitle::C_E::e_Isbn:
            str = &(*item)->GetIsbn();
            break;

        default:
            break;
        };
        if (!NStr::IsBlank(*str))  return *str;

    }
    return "";
}

void CFixTitleCap::s_SetTitle(CTitle& title, const string& value) // CValidError_imp::HasTitle(const CTitle& title) in valid_pub.cpp
{
    NON_CONST_ITERATE(CTitle::Tdata, item, title.Set()) {
        switch ((*item)->Which()) {
        case CTitle::C_E::e_Name:
            (*item)->SetName(value);
            break;

        case CTitle::C_E::e_Tsub:
            (*item)->SetTsub(value);
            break;

        case CTitle::C_E::e_Trans:
            (*item)->SetTrans(value);
            break;

        case CTitle::C_E::e_Jta:
            (*item)->SetJta(value);
            break;

        case CTitle::C_E::e_Iso_jta:
            (*item)->SetIso_jta(value);
            break;

        case CTitle::C_E::e_Ml_jta:
            (*item)->SetMl_jta(value);
            break;

        case CTitle::C_E::e_Coden:
            (*item)->SetCoden(value);
            break;

        case CTitle::C_E::e_Issn:
            (*item)->SetIssn(value);
            break;

        case CTitle::C_E::e_Abr:
            (*item)->SetAbr(value);
            break;

        case CTitle::C_E::e_Isbn:
            (*item)->SetIsbn(value);
            break;

        default:
            break;
        };
    }
}

bool CFixTitleCap::x_FixTitle(string& title)
{
    string fixed_title = macro::CMacroFunction_FixPubCaps::s_FixCapitalizationInTitle(title, true, m_tse);
    if (fixed_title != title) {
        title = fixed_title;
        return true;
    }
    return false;
}


bool CFixAllCap::x_ApplyToPubdesc(CPubdesc& pubdesc)
{
    bool modified = false;

    CFixAffilCap affil_cap(m_tse);
    modified |= affil_cap.x_ApplyToPubdesc(pubdesc);

    CFixAuthorCap author_cap(m_tse);
    modified |= author_cap.x_ApplyToPubdesc(pubdesc);

    CFixTitleCap title_cap(m_tse);
    modified |= title_cap.x_ApplyToPubdesc(pubdesc);

    return modified;
}


void CReloadPublications::LookUpByPmid(CPub& pub, int pmid, bool& modified)
{
    // prepare eFetch request
    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(1);

    vector<int> uids;
    uids.push_back(pmid);

    CRef<CPubmed_entry> pubmed_entry(new CPubmed_entry());
    CNcbiStrstream asnPubMedEntry;
    try {
        ecli.Fetch("pubmed", uids, asnPubMedEntry, "asn.1");
        asnPubMedEntry >> MSerial_AsnText >> *pubmed_entry;
    }
    catch (const CException& e) {
        LOG_POST(Error << "CReloadPublications::LookUpByPmid(): error fetching ID " << pmid << ": " << e.GetMsg());
        return;
    }

    if (pubmed_entry->CanGetMedent() && pubmed_entry->GetMedent().CanGetCit())
    {
        CCit_art& article = pubmed_entry->SetMedent().SetCit();
        pub.SetArticle(article);
        modified = true;
    }
}

bool CReloadPublications::x_ApplyToPubdesc(CPubdesc& pubdesc)
{
    bool modified = false;
    int pmid = 0;
    EDIT_EACH_PUB_ON_PUBDESC(pub_iter, pubdesc) {
        CPub& pub = **pub_iter;
        switch (pub.Which()) {
        case CPub::e_Pmid:
            pmid = pub.GetPmid();
            break;
        default:
            break;
        }
    }
    if (pmid != 0)
    {
        EDIT_EACH_PUB_ON_PUBDESC(pub_iter, pubdesc) {
            CPub& pub = **pub_iter;
            switch (pub.Which()) {
            case CPub::e_Article:
                LookUpByPmid(pub, pmid, modified);
                break;
            default:
                break;
            }
        }
    }
    return modified;
}


bool IAuthorEditor::x_ApplyToPubdesc(CPubdesc& pubdesc)
{
    bool modified = false;
    if (pubdesc.IsSetPub())
    {
        CPub_equiv& pe = pubdesc.SetPub();
        x_ApplyToPubequiv(pe, modified);
    }
    return modified;
}

void IAuthorEditor::x_ApplyToPubequiv(CPub_equiv& pe, bool& modified)
{
    EDIT_EACH_PUB_ON_PUBEQUIV (pub_iter, pe) {
        CPub& pub = **pub_iter;
        CAuth_list* authors = 0;
        switch ( pub.Which() ) {
        case CPub::e_Gen:
            if ( pub.GetGen().IsSetAuthors() ) {
                authors = &(pub.SetGen().SetAuthors());
            }
            break;
        case CPub::e_Sub:
            authors = &(pub.SetSub().SetAuthors());
            break;
        case CPub::e_Article:
            if ( pub.GetArticle().IsSetAuthors() ) {
                authors = &(pub.SetArticle().SetAuthors());
            }
            break;
        case CPub::e_Book:
            authors = &(pub.SetBook().SetAuthors());
            break;
        case CPub::e_Proc:
            authors = &(pub.SetProc().SetBook().SetAuthors());
            break;
        case CPub::e_Man:
            authors = &(pub.SetMan().SetCit().SetAuthors());
            break;
        case CPub::e_Patent:
            authors = &(pub.SetPatent().SetAuthors());
            break;
        case CPub::e_Equiv:
            x_ApplyToPubequiv(pub.SetEquiv(),modified);
            break;
        default:
            break;
        }
            
        if ( !authors )  continue;

        modified = x_ApplyToCAuth(*authors);
    }
}

void IAuthorEditor::x_ApplyToSeqSubmit(CCmdComposite* composite)
{
    if (m_SubmitBlock->IsSetCit() && m_SubmitBlock->GetCit().IsSetAuthors())
    {
        CRef<CSubmit_block> new_submit_block(new CSubmit_block);
        new_submit_block->Assign(*m_SubmitBlock);
        CAuth_list* authors = &(new_submit_block->SetCit().SetAuthors());
        bool modified = x_ApplyToCAuth(*authors);
        if (modified)
        {
            CChangeSubmitBlockCommand* cmd = new CChangeSubmitBlockCommand();
            CObject* actual = (CObject*) (m_SubmitBlock.GetPointer());
            cmd->Add(actual, CConstRef<CObject>(new_submit_block));
            composite->AddCommand(*cmd);
        }
    }
}

string IAuthorEditor::s_CapitalizeInitial(const string &orig)
{
    string res;
    for (string::const_iterator p = orig.begin(); p != orig.end(); ++p)
    {
        bool need_dot = false;
        if (isalpha(*p))
        {
            need_dot = true;
            string::const_iterator n = p;
            n++;
            if (n != orig.end() && *n == '.')
                need_dot = false;          
        }
        res += toupper(*p);
        if (need_dot)
            res += '.';
    }
    
    return res;  
}

bool IAuthorEditor::s_FixInitials(CPerson_id& pid) // TransferDataFromWindow in singleauthor_panel.cpp
{
    bool modified = false;
    if (pid.SetName().IsSetInitials()) 
    {
        string first_init;
        if (pid.SetName().IsSetFirst()) 
        {
            string first = pid.GetName().GetFirst();
            first_init = edit::GetFirstInitial(first,true);
        }
        string original_init = pid.GetName().GetInitials();
        string middle_init = s_CapitalizeInitial(original_init);

        if (! NStr::IsBlank(first_init) && NStr::StartsWith(middle_init, first_init, NStr::eNocase)) 
            middle_init = middle_init.substr(first_init.length());
        else if (!NStr::IsBlank(middle_init) && NStr::StartsWith(first_init, middle_init, NStr::eNocase)) 
            middle_init.clear();
        string init = first_init;
        if (!NStr::IsBlank(middle_init)) init += middle_init;
        if (!NStr::IsBlank(init) && init != original_init) 
        {
            pid.SetName().SetInitials(init);
            modified = true;
        }
    }
    return modified;
}

static bool s_CleanVisString( string &str )
{
    bool changed = false;

    if( str.empty() ) {
        return false;
    }

    // chop off initial junk
    {
        string::size_type first_good_char_pos = str.find_first_not_of(" ;,");
        if( first_good_char_pos == string::npos ) {
            // string is completely junk
            str.clear();
            return true;
        } else if( first_good_char_pos > 0 ) {
            copy( str.begin() + first_good_char_pos, str.end(), str.begin() );
            str.resize( str.length() - first_good_char_pos );
            changed = true;
        }
    }

    // chop off end junk

    string::size_type last_good_char_pos = str.find_last_not_of(" ;,");
    _ASSERT( last_good_char_pos != string::npos ); // we checked this case so it shouldn't happen
    if( last_good_char_pos == (str.length() - 1) ) {
        // nothing to chop of the end
        return changed;
    } else if( str[last_good_char_pos+1] == ';' ) {
        // special extra logic for semicolons because it might be part of
        // an HTML character like "&nbsp;"

        // see if there's a '&' before the semicolon
        // ( ' ' and ',' would break the '&' and make it irrelevant, though )
        string::size_type last_ampersand_pos = str.find_last_of("& ,", last_good_char_pos );
        if( last_ampersand_pos == string::npos ) {
            // no ampersand, so just chop off as normal
            str.resize( last_good_char_pos + 1 );
            return true;
        }
        switch( str[last_ampersand_pos] ) {
            case '&':
                // can't chop semicolon, so chop just after it
                if( (last_good_char_pos + 2) == str.length() ) {
                    // semicolon is at end, so no chopping occurs
                    return changed;
                } else {
                    // chop after semicolon
                    str.resize( last_good_char_pos + 2 );
                    return true;
                }
            case ' ':
            case ',':
                // ampersand (if any) is irrelevant due to intervening
                // space or comma
                str.resize( last_good_char_pos + 1 );
                return true;
            default:
                _ASSERT(false);
                return changed;  // should be impossible to reach here
        }

    } else {
        str.resize( last_good_char_pos + 1 );
        return true;
    }
}

void IAuthorEditor::s_ExtractSuffixFromInitials(CName_std& name)
{
    _ASSERT( FIELD_IS_SET(name, Initials)  &&  ! FIELD_IS_SET(name, Suffix) );

    string& initials = GET_MUTABLE(name, Initials);

    if (initials.find('.') == NPOS) {
        return;
    }

// this macro is arguably more convenient than a function
#define EXTRACTSUFFIXFROMINITIALS( OLD, NEW ) \
    if( NStr::EndsWith(initials, OLD) ) { \
        initials.resize( initials.length() - strlen(OLD) ); \
        SET_FIELD(name, Suffix, NEW); \
        return; \
    }

    EXTRACTSUFFIXFROMINITIALS( "III",  "III" )
    EXTRACTSUFFIXFROMINITIALS( "III.", "III" )
    EXTRACTSUFFIXFROMINITIALS( "Jr",   "Jr" )
    EXTRACTSUFFIXFROMINITIALS( "2nd",  "II" )
    EXTRACTSUFFIXFROMINITIALS( "IV",   "IV" )
    EXTRACTSUFFIXFROMINITIALS( "IV.",  "IV" )

#undef EXTRACTSUFFIXFROMINITIALS
}


#define TRUNCATE_SPACES(o, x) \
    if ((o).IsSet##x()) { \
        NStr::TruncateSpacesInPlace((o).Set##x()); \
        if (NStr::IsBlank((o).Get##x())) { \
            (o).Reset##x(); \
        } \
    }

#define CLEAN_STRING_MEMBER(o, x) \
    if ((o).IsSet##x()) { \
    s_CleanVisString((o).Set##x());      \
        if (NStr::IsBlank((o).Get##x())) { \
            (o).Reset##x(); \
        } \
    }

void IAuthorEditor::s_FixEtAl(CName_std& name)
{
    if( FIELD_EQUALS(name, Last, "et") &&
        ( FIELD_EQUALS(name, Initials, "al")  || 
          FIELD_EQUALS(name, Initials, "al.") ||
          FIELD_EQUALS(name, Initials, "Al.") ) &&
        ( RAW_FIELD_IS_EMPTY_OR_UNSET(name, First) ||
          FIELD_EQUALS(name, Initials, "a") ) )
    {
        RESET_FIELD( name, Initials );
        RESET_FIELD( name, First );
        SET_FIELD( name, Last, "et al." );
    }
}

bool IAuthorEditor::s_FixInitialsNew(CPerson_id& pid)
{
    bool modified = false;

    if (!pid.IsName())
        return modified;
    CName_std& name = pid.SetName();
    bool fix_initials = true;
    // there's a lot of shuffling around (e.g. adding and removing
    // periods in initials), so we can't determine
    // if we've actually changed anything until we get to the end of 
    // this function.
    CRef<CName_std> original_name( new CName_std );
    original_name->Assign( name );

    // if initials starts with uppercase, we remember to 
    // upcase the whole thing later
    bool upcaseinits = false;
    if( isupper( GET_STRING_FLD_OR_BLANK(name, Initials)[0] ) ) {
        upcaseinits = true;
    }

    string first_initials;
    // like in C: str = NameStdPtrToTabbedString (nsp, fixInitials);
    {
        if ( ! FIELD_IS_SET(name, Suffix) && FIELD_IS_SET(name, Initials) ) {
            s_ExtractSuffixFromInitials(name);
        }
        TRUNCATE_SPACES(name, First);
        if( FIELD_IS_SET(name, Initials) ) {
            NStr::ReplaceInPlace( GET_MUTABLE(name, Initials), ".", "" );
            NStr::TruncateSpacesInPlace( GET_MUTABLE(name, Initials), NStr::eTrunc_Begin );
        }
        if( FIELD_IS_SET(name, Last) ) {
            NStr::TruncateSpacesInPlace( GET_MUTABLE(name, Last), NStr::eTrunc_Begin );
        }
        if( FIELD_IS_SET(name, Middle) ) {
            NStr::TruncateSpacesInPlace( GET_MUTABLE(name, Middle), NStr::eTrunc_Begin );
        }
        s_FixEtAl( name );

        // extract initials from first name
        // like in C: FirstNameToInitials (first, first_initials, sizeof (first_initials) - 1);
        {
            if ( FIELD_IS_SET(name, First) ) {
                const string &first = GET_FIELD(name, First);
                string::size_type next_pos = 0;
                while ( next_pos < first.length() ) {
                    // skip initial spaces and hyphens
                    next_pos = first.find_first_not_of(" -", next_pos);
                    if( string::npos == next_pos ) break;
                    // if we hit an letter after that, copy the letter to inits
                    if( isalpha( first[next_pos] ) ) {\
                        first_initials += first[next_pos];
                    }
                    // find next space or hyphen
                    next_pos = first.find_first_of(" -", next_pos);
                    if( string::npos == next_pos ) break;
                    // if it's a hyphen, copy it
                    if( first[next_pos] == '-' ) {
                        first_initials += '-';
                    }
                }
            }
        }

        if( FIELD_IS_SET(name, First) ) {
            NStr::ReplaceInPlace( GET_MUTABLE(name, First), ".", "" );
            NStr::TruncateSpacesInPlace( GET_MUTABLE(name, First), NStr::eTrunc_Begin );
        }

        if (fix_initials) {
            if( ! RAW_FIELD_IS_EMPTY_OR_UNSET(name, Initials) ) {
                string & initials = GET_MUTABLE(name, Initials);

                // skip part of initials that matches first_initials
                string::size_type initials_first_good_idx = 0;
                for( ; initials_first_good_idx < initials.length() &&
                        initials_first_good_idx < first_initials.length() && 
                        initials[initials_first_good_idx] == first_initials[initials_first_good_idx] ; 
                    ++initials_first_good_idx )
                {
                    // do nothing
                }

                if( initials_first_good_idx > 0 ) {
                    initials.erase( 0, initials_first_good_idx );
                }
            }
        } else if ( RAW_FIELD_IS_EMPTY_OR_UNSET(name, Initials) && ! first_initials.empty() ) {
            SET_FIELD(name, Initials, first_initials );
        }

        if( FIELD_IS_SET(name, Suffix) ) {
            NStr::ReplaceInPlace( GET_MUTABLE(name, Suffix), ".", "" );
            NStr::TruncateSpacesInPlace( GET_MUTABLE(name, Suffix), NStr::eTrunc_Begin );
        }

        // This differs from C, which just deletes these fields.
        CLEAN_STRING_MEMBER(name, Title);
        CLEAN_STRING_MEMBER(name, Full);
    }

    // like in C: nsp = TabbedStringToNameStdPtr (str, fixInitials);
    {
        // initials = remove_spaces(first_initials+initials)
        if( fix_initials && ! first_initials.empty() ) {
            SET_FIELD(name, Initials, 
                first_initials + GET_STRING_FLD_OR_BLANK(name, Initials) );
        }
        if( FIELD_IS_SET(name, Initials) ) {
            string & initials = GET_MUTABLE(name, Initials);
            NStr::ReplaceInPlace( initials, " ", "" );
            NStr::ReplaceInPlace( initials, ",", "." );
            NStr::ReplaceInPlace( initials, ".ST.", ".St." );

            string new_initials;
            string::const_iterator initials_iter = initials.begin();
            // modify initials.  New version will be built in new_initials
            for( ; initials_iter != initials.end(); ++initials_iter ) {
                const char ch = *initials_iter;
                switch( ch ) {
                case '-':
                    // keep hyphens
                    new_initials += '-';
                    break;
                case '.':
                case ' ':
                    // erase periods and spaces
                    break;
                default:
                    // other characters: keep them, BUT...
                    new_initials += ch;

                    if( (initials_iter + 1) != initials.end()) {
                        const char next_char = *(initials_iter + 1);
                        if (! islower(next_char) ) {
                            // if next character is not lower, add period
                            new_initials += '.';
                        }
                    }
                }
            }
            
            if( initials != new_initials ) {
                initials.swap(new_initials); // swap is faster than assignment
                new_initials.clear();
            }

            // add period if string is not empty and doesn't end with a period
            if( ! initials.empty() && ! NStr::EndsWith(initials, ".") ) {
                initials += '.';
            }
        }

        if( FIELD_IS_SET(name, Suffix) ) {
            string &suffix = GET_MUTABLE(name, Suffix);
            // remove spaces
            NStr::ReplaceInPlace( suffix, " ", "" );

            if ( ! suffix.empty() ) {
                // remove any period, if any, on the end
                if( NStr::EndsWith(suffix, ".") ) {
                    suffix.resize( suffix.length() - 1 );
                }

                if( NStr::EqualNocase(suffix, "1d") ) {
                     suffix = "I";
                } else if( NStr::EqualNocase(suffix, "1st") ) {
                     suffix = "I";
                } else if( NStr::EqualNocase(suffix, "2d") ) {
                     suffix = "II";
                } else if( NStr::EqualNocase(suffix, "2nd") ) {
                     suffix = "II";
                } else if( NStr::EqualNocase(suffix, "3d") ) {
                     suffix = "III";
                } else if( NStr::EqualNocase(suffix, "3rd") ) {
                     suffix = "III";
                } else if( NStr::EqualNocase(suffix, "4th") ) {
                     suffix = "IV";
                } else if( NStr::EqualNocase(suffix, "5th") ) {
                     suffix = "V";
                } else if( NStr::EqualNocase(suffix, "6th") ) {
                     suffix = "VI";
                } else if( NStr::EqualNocase(suffix, "Sr") ) {
                     suffix = "Sr.";
                } else if( NStr::EqualNocase(suffix, "Jr") ) {
                     suffix = "Jr.";
                }
            }
        }

        // add dot to "et al"
        if ( FIELD_EQUALS(name, Last, "et al") ) {
            SET_FIELD(name, Last, "et al." );
        }

        // reset middle if it's blank
        if ( FIELD_EQUALS(name, Middle, kEmptyStr) ) {
            RESET_FIELD(name, Middle);
        }
    }

    if (upcaseinits && FIELD_IS_SET(name, Initials) ) {
        string & initials = GET_MUTABLE(name, Initials);
        if( ! initials.empty() && islower(initials[0]) ) {
            initials[0] = toupper(initials[0]);
        }
    }
    CLEAN_STRING_MEMBER(name, Last);
    CLEAN_STRING_MEMBER(name, First);
    CLEAN_STRING_MEMBER(name, Middle);
    CLEAN_STRING_MEMBER(name, Full);
    CLEAN_STRING_MEMBER(name, Initials);
    CLEAN_STRING_MEMBER(name, Suffix);
    CLEAN_STRING_MEMBER(name, Title);
    s_FixEtAl( name );

    if( ! FIELD_IS_SET(name, Last) ) {
        SET_FIELD(name, Last, kEmptyCStr );
    }
    string &last = GET_MUTABLE(name, Last);
    if( RAW_FIELD_IS_EMPTY_OR_UNSET(name, Suffix) &&
        ( NStr::EndsWith(last, " Jr.") || NStr::EndsWith(last, " Sr.") ) ) 
    {
        SET_FIELD(name, Suffix, last.substr( last.length() - 3 ) );
        last.resize( last.length() - 4 );
        NStr::TruncateSpacesInPlace( last );
    }

    if( FIELD_IS_SET(name, Initials) && RAW_FIELD_IS_EMPTY_OR_UNSET(name, Suffix) ) {
        string & initials = GET_MUTABLE(name, Initials);
        if( NStr::EndsWith(initials, ".Jr.") || NStr::EndsWith(initials, ".Sr.") ) {
            SET_FIELD(name, Suffix, initials.substr( initials.length() - 3 ) );
            initials.resize( initials.length() - 3 );
            NStr::TruncateSpacesInPlace( initials );
        }
    }

    if( ! original_name->Equals(name) ) {
        modified = true;
    }
    return modified;
}

bool CReverseAuthorNames::ApplyToPubdesc(CConstRef<CObject> obj, CSeq_entry_Handle tse, CCmdComposite& composite)
{
    if (!tse || !obj)
        return false;

    m_tse = tse;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointerOrNull());
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(obj.GetPointerOrNull());
    const CSubmit_block* sub_block = dynamic_cast<const CSubmit_block*>(obj.GetPointerOrNull());
    bool modified = false;

    CScope& scope = m_tse.GetScope();
    if (feat) {
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*feat);
        if (x_ApplyToPubdesc(new_feat->SetData().SetPub())) {
            CSeq_feat_Handle fh = scope.GetSeq_featHandle(*feat);
            CRef<CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(fh, *new_feat));
            composite.AddCommand(*cmd);
            modified = true;
        }
    }
    else if (seqdesc) {
        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->Assign(*seqdesc);
        if (x_ApplyToPubdesc(new_desc->SetPub())) {
            CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(Ref(&scope), *seqdesc);
            CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(seh, *seqdesc, *new_desc));
            composite.AddCommand(*cmd);
            modified = true;
        }
    }
    else if (sub_block) {
        m_SubmitBlock.Reset(sub_block);
        x_ApplyToSeqSubmit(&composite);
        modified = true;
    }
    return modified;
}

bool CReverseAuthorNames::x_ApplyToCAuth(CAuth_list& authors)
{
    if (!authors.IsSetNames())
        return false;

    bool modified = false;
    CAuth_list::TNames &author_names = authors.SetNames();
    if (author_names.IsStd()) {
        for ( auto&& auth_it : author_names.SetStd()) {
            CPerson_id &pid = auth_it->SetName();
            modified |= macro::CMacroFunction_AuthorFix::s_ReverseAuthorNames(pid.SetName());
        }
    }
    return modified;
}


bool CFixAuthorInitials::x_ApplyToCAuth(CAuth_list& authors)
{
    if (!authors.IsSetNames())
        return false;

    bool modified = false;
    CAuth_list::TNames &author_names = authors.SetNames();
    if (author_names.IsStd()) {
        for (auto&& auth_it : author_names.SetStd()) {
            CPerson_id &pid = auth_it->SetName();
            if (pid.IsName()) {
                modified |= macro::CMacroFunction_AuthorFix::s_TruncateMiddleInitials(pid.SetName());
            }
        }
    }
    return modified;
}

static bool s_FixNameCap(string& name, bool bApostrophes)
{
    string orig = name;
    FixCapitalizationInElement(name);
    if (bApostrophes) CapitalizeAfterApostrophe(name);
    return (orig != name);
}

bool CFixAuthorCap::x_ApplyToCAuth(CAuth_list& authors)
{
    if (!authors.IsSetNames())
        return false;

    bool modified = false;
    CAuth_list::TNames &author_names = authors.SetNames();
    if (author_names.IsStd()) {
        for (auto&& auth_it : author_names.SetStd()) {
            CPerson_id &pid = auth_it->SetName();
            if (pid.IsName() && pid.GetName().IsSetLast() && pid.GetName().IsSetFirst())
            {
                string last = pid.GetName().GetLast();
                string first = pid.GetName().GetFirst();
                modified |= s_FixNameCap(last, true);
                modified |= s_FixNameCap(first, false);
                pid.SetName().SetLast(last);
                pid.SetName().SetFirst(first);
                modified |= s_FixInitials(pid);
            }
        }
    }
    return modified;
}

bool CStripAuthorSuffix::x_ApplyToCAuth(CAuth_list& authors)
{
    if (!authors.IsSetNames())
        return false;

    bool modified = false;
    CAuth_list::TNames &author_names = authors.SetNames();
    if (author_names.IsStd()) {
        for (auto&& auth_it : author_names.SetStd()) {
            CPerson_id &pid = auth_it->SetName();
            if (pid.IsName() && pid.GetName().IsSetSuffix() && !NStr::IsBlank(pid.GetName().GetSuffix()))
            {
                pid.SetName().ResetSuffix();
                modified = true;
            }
        }
    }
    return modified;
}

bool CRemoveAuthorConsortium::x_ApplyToCAuth(CAuth_list& authors)
{
    if (!authors.IsSetNames())
        return false;

    bool modified = false;
    auto &author_names = authors.SetNames();
    if (author_names.IsStd()) {
        auto auth_it = author_names.SetStd().begin();
        while (auth_it != author_names.SetStd().end()) {
            CPerson_id &pid = (*auth_it)->SetName();
            if (pid.IsConsortium()) {
                auth_it = author_names.SetStd().erase(auth_it);  // TODO - not quite clear what to do if we ever remove all names here
                modified = true;
            }
            else {
                ++auth_it;
            }
        }
    }
    return modified;
}

bool CConvertAllAuthorToConsortium::x_ApplyToCAuth(CAuth_list& authors)
{
    if (!authors.IsSetNames())
        return false;

    bool modified = false;
    CAuth_list::TNames &author_names = authors.SetNames();
    if (author_names.IsStd()) {
        for (auto&& auth_it : author_names.SetStd()) {
            CPerson_id &pid = auth_it->SetName();
            if (pid.IsName() && pid.GetName().IsSetLast())
            {
                string last = pid.GetName().GetLast();
                modified = true;
                pid.Reset();
                pid.Select(CPerson_id::e_Consortium);
                pid.SetConsortium(last);
            }
        }
    }
    return modified;
}


CConvertAuthorToConsortiumWhereConstraint::~CConvertAuthorToConsortiumWhereConstraint()
{
    if (m_Regex) delete m_Regex;
}

void CConvertAuthorToConsortiumWhereConstraint::SetConstraint(const string &input)
{
    m_Regex = new CRegexp("\\b" + input + "\\b", CRegexp::fCompile_ignore_case);
}

bool CConvertAuthorToConsortiumWhereConstraint::x_ApplyToCAuth(CAuth_list& authors)
{
    if (!authors.IsSetNames())
        return false;

    bool modified = false;
    CAuth_list::TNames &author_names = authors.SetNames();
    if (author_names.IsStd()) {
        for (auto&& auth_it : author_names.SetStd()) {
            CPerson_id &pid = auth_it->SetName();
            if (pid.IsName() && pid.GetName().IsSetLast() && m_Regex)
            {
                string last = pid.GetName().GetLast();
                m_Regex->GetMatch(last, 0, 0, CRegexp::fMatch_default, true);
                if (m_Regex->NumFound() > 0)
                {
                    modified = true;
                    pid.Reset();
                    pid.Select(CPerson_id::e_Consortium);
                    pid.SetConsortium(last);
                }
            }
        }
    }
    return modified;
}


bool IAffilEditor::x_ApplyToCAuth(CAuth_list& authors)
{
    bool modified = false;
    if (authors.IsSetAffil()) {
        modified |= x_ApplyToCAffil(authors.SetAffil());
    }
    
    if (authors.IsSetNames()) {
        CAuth_list::TNames& author_names = authors.SetNames();
        switch (author_names.Which()) {
        case CAuth_list::TNames::e_Std:
            for (auto&& auth_it : author_names.SetStd()) {
                if (auth_it->IsSetAffil()) {
                    modified |= x_ApplyToCAffil(auth_it->SetAffil());
                }
            }
            break;
        default:
            break;
        }
    }
    return modified;
}


bool CFixUSA::x_ApplyToCAffil(CAffil& affil)
{
    bool modified = false;
    modified |= FixUSAAbbreviationInAffil(affil);
    modified |= FixStateAbbreviationsInAffil(affil);
    return modified;
}


bool CFixCountryCap::x_ApplyToCAffil(CAffil& affil)  // FixCapitalizationInCountryString
{
    if (!affil.IsStd()) {
        return false;
    }

    bool modified = false;
    auto& std = affil.SetStd();
    if (std.IsSetCountry() && !NStr::IsBlank(std.GetCountry()))    {

        string country = std.GetCountry();
        string result = country;
        NStr::ReplaceInPlace (result, "  ", " ");
        NStr::TruncateSpacesInPlace (result);

        InsertMissingSpacesAfterCommas(result);
        InsertMissingSpacesAfterNo(result);
        FixCapitalizationInElement(result);
        FixShortWordsInElement(result);
        FixAbbreviationsInElement(result);
        FindReplaceString_CountryFixes(result);
     
        if (result != country) 
        {
            modified = true;
            std.SetCountry(result);
        }
    }
    return modified;
}


bool CFixAffilCap::x_ApplyToCAffil(CAffil& affil) // FixCapsInPubAffilEx
{
    if (!affil.IsStd()) {
        return false;
    }

    bool modified = false;
    auto& std = affil.SetStd();
    if (std.IsSetAffil() && !NStr::IsBlank(std.GetAffil())) 
    {
        string affil = std.GetAffil();
        NStr::ReplaceInPlace (affil, "  ", " ");
        NStr::TruncateSpacesInPlace (affil);
        FixCapitalizationInElement(affil);
        FixShortWordsInElement(affil);
        FixAbbreviationsInElement(affil);
        FixAffiliationShortWordsInElement(affil);
        FixKnownAbbreviationsInElement(affil);
        InsertMissingSpacesAfterCommas (affil);
        InsertMissingSpacesAfterNo(affil);
        FixOrdinalNumbers (affil);
        if (affil != std.GetAffil()) 
        {
            modified = true;
            std.SetAffil(affil);
        }
    }

    if (std.IsSetDiv() && !NStr::IsBlank(std.GetDiv())) 
    {
        string div = std.GetDiv();
        NStr::ReplaceInPlace (div, "  ", " ");
        NStr::TruncateSpacesInPlace (div);
        FixCapitalizationInElement(div);
        FixShortWordsInElement(div);
        FixAbbreviationsInElement(div);
        FixAffiliationShortWordsInElement(div);
        FixKnownAbbreviationsInElement(div);
        InsertMissingSpacesAfterCommas(div);
        InsertMissingSpacesAfterNo(div);
        FixOrdinalNumbers (div);
        if (div != std.GetDiv()) 
        {
            modified = true;
            std.SetDiv(div);
        }
    }

    if (std.IsSetCity() && !NStr::IsBlank(std.GetCity())) 
    {
        string city = std.GetCity();
        NStr::ReplaceInPlace (city, "  ", " ");
        NStr::TruncateSpacesInPlace (city);
        FixCapitalizationInElement(city);
        FixShortWordsInElement(city);
        FixAffiliationShortWordsInElement(city);
        FixKnownAbbreviationsInElement(city);        
        InsertMissingSpacesAfterCommas(city);
        InsertMissingSpacesAfterNo(city);
        FixOrdinalNumbers (city);
        if (city != std.GetCity()) 
        {
            modified = true;
            std.SetCity(city);
        }
    }

    if (std.IsSetStreet() && !NStr::IsBlank(std.GetStreet())) 
    {
        string street = std.GetStreet();
        NStr::ReplaceInPlace (street, "  ", " ");
        NStr::TruncateSpacesInPlace (street);
        FixKnownAbbreviationsInElement(street);
        FixCapitalizationInElement(street);
        FixShortWordsInElement(street);
        FixAffiliationShortWordsInElement(street);
        InsertMissingSpacesAfterCommas(street);
        InsertMissingSpacesAfterNo(street);
        FixOrdinalNumbers(street);
        if (street != std.GetStreet()) 
        {
            modified = true;
            std.SetStreet(street);
        }
    }

    if (std.IsSetCountry() && !NStr::IsBlank(std.GetCountry())) 
    {
        string country = std.GetCountry();
        NStr::ReplaceInPlace (country, "  ", " ");
        NStr::TruncateSpacesInPlace (country);
        InsertMissingSpacesAfterCommas(country);
        InsertMissingSpacesAfterNo(country);
        FixCapitalizationInElement(country);
        FixShortWordsInElement(country);
        FixAbbreviationsInElement(country);
        FindReplaceString_CountryFixes(country);
        if (country != std.GetCountry()) 
        {
            modified = true;
            std.SetCountry(country);
        }
    }

    if (std.IsSetSub() && !NStr::IsBlank (std.GetSub()))
    {
        if (std.IsSetCountry() && NStr::EqualCase (std.GetCountry(), "USA")) 
        {
            string state = std.GetSub();
            string result = state;
            NStr::ReplaceInPlace (result, "  ", " ");
            NStr::TruncateSpacesInPlace (result);
            GetStateAbbreviation(result);
            if (result.length() == 2) result = NStr::ToUpper(result);
            if (result != state)
            {
                std.SetSub(result);
                modified = true;
            }
        }
        else
        {
            string sub = std.GetSub();
            NStr::ReplaceInPlace (sub, "  ", " ");
            NStr::TruncateSpacesInPlace (sub);
            FixCapitalizationInElement(sub);
            FixShortWordsInElement(sub);
            FixAffiliationShortWordsInElement(sub);
            InsertMissingSpacesAfterCommas(sub);
            if (sub != std.GetSub()) 
            {
                modified = true;
                std.SetSub(sub);
            }
        }  
    }
    return modified;
}


// Removing pubs
CRef<CCmdComposite> IRemovePubs::GetCommand(CSeq_entry_Handle tse)
{
    if (!tse)
        return CRef<CCmdComposite>();

    m_tse = tse;
    CRef<CCmdComposite> composite(new CCmdComposite(m_Title));
    x_ApplyToSeqAndFeat(composite);
    return composite;
}

void IRemovePubs::x_ApplyToSeqAndFeat(CCmdComposite* composite)
{
    x_ApplyToDescriptors(*(m_tse.GetCompleteSeq_entry()), composite);

    for (CFeat_CI feat_it(m_tse, SAnnotSelector(CSeqFeatData::e_Pub)); feat_it; ++feat_it) {
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat_it->GetOriginalFeature());
        CPubdesc& edited_pub = new_feat->SetData().SetPub();
        if (x_ApplyToPubdesc(edited_pub)) {
            const auto& pubs = edited_pub.GetPub().Get();
            if (pubs.empty()) {
                CRef<CCmdDelSeq_feat> cmd(new CCmdDelSeq_feat(*feat_it));
                composite->AddCommand(*cmd);
            }
            else {
                CRef<CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(*feat_it, *new_feat));
                composite->AddCommand(*cmd);
            }
        }
    }
}

void IRemovePubs::x_ApplyToDescriptors(const CSeq_entry& se, CCmdComposite* composite)
{
    FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
        if ((*it)->IsPub()) {
            const CSeqdesc& orig_desc = **it;
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(orig_desc);
            CPubdesc& edited_pub = new_desc->SetPub();
            if (x_ApplyToPubdesc(edited_pub)) {
                const auto& pubs = edited_pub.GetPub().Get();
                if (pubs.empty()) {
                    CRef<CCmdDelDesc> cmdDelDesc(new CCmdDelDesc(m_tse.GetScope().GetSeq_entryHandle(se), orig_desc));
                    composite->AddCommand(*cmdDelDesc);
                }
                else {
                    CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(m_tse.GetScope().GetSeq_entryHandle(se), orig_desc, *new_desc));
                    composite->AddCommand(*cmd);
                }
                
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet()) {
            x_ApplyToDescriptors(**it, composite);
        }
    }
}


bool CRemoveUnpublishedPubs::x_ApplyToPubdesc(CPubdesc& pubdesc)
{
    bool modified = false;
    auto& pubs = pubdesc.SetPub().Set();
    for (CPub_set::TPub::iterator pub_iter = pubs.begin(); pub_iter != pubs.end(); ) 
    {
        CPub& pub = **pub_iter;
        bool to_delete = false;
        switch( pub.Which() ) {
        case CPub::e_Gen:
        {
            const CCit_gen& gen = pub.GetGen();
            if ( gen.IsSetCit()  &&  !gen.GetCit().empty() ) {
                const string& cit = gen.GetCit();
                if (NStr::StartsWith (cit, "unpublished", NStr::eNocase))
                {
                    // Cit-gen.cit = "unpublished"
                    to_delete = true;
                }
            }
        }
        break;
        case CPub::e_Article:
        {
            const CCit_art& art = pub.GetArticle();
            if ( art.GetFrom().IsJournal() ) 
            {
                const CCit_jour& jour = art.GetFrom().GetJournal();
                if ( jour.IsSetImp() ) 
                {
                    const CImprint& imp = jour.GetImp();
                    if ( imp.IsSetPrepub() && imp.GetPrepub() != CImprint::ePrepub_in_press)
                    {
                        // Cit-art.from.jour.imp.prepub is not 0 or 2
                        to_delete = true;
                    }
                }
            }
            if ( art.GetFrom().IsBook() ) 
            {
                const CCit_book& book = art.GetFrom().GetBook();
                if ( book.IsSetImp() ) 
                {
                    const CImprint& imp = book.GetImp();
                    if ( imp.IsSetPrepub() && imp.GetPrepub() != CImprint::ePrepub_in_press)
                    {
                        // Cit-art.from.book.imp.prepub is not 0 or 2
                        to_delete = true;
                    }
                }
            }
        }
        break;
        case CPub::e_Book:
        {
            const CCit_book& book = pub.GetBook();
            if ( book.IsSetImp() )
            { 
                const CImprint& imp = book.GetImp();
                
                if ( imp.IsSetPrepub() && imp.GetPrepub() != CImprint::ePrepub_in_press)
                {
                    // Cit-book where Cit-book.imp.prepub is not 0 or 2 
                    to_delete = true;
                }
            }
        }
        break;
        case CPub::e_Journal:
        {
            const CCit_jour& jour = pub.GetJournal();
            if ( jour.IsSetImp() ) 
            {
                const CImprint& imp = jour.GetImp();
                    
                if ( imp.IsSetPrepub() && imp.GetPrepub() != CImprint::ePrepub_in_press)
                {
                    // Cit-jour where Cit-jour.imp.prepub is not 0 or 2 
                    to_delete = true;
                }
            }
        }
        break;
        case CPub::e_Man:
        {
            const CCit_book& book = pub.GetMan().GetCit();
            if ( book.IsSetImp() ) 
            {
                const CImprint& imp = book.GetImp();
                    
                if ( imp.IsSetPrepub() && imp.GetPrepub() != CImprint::ePrepub_in_press)
                {
                    // Cit-let where Cit-let.imp.prepub is not 0 or 2
                    to_delete = true;
                }
                    
            }
        }
        break;
        default:
            break;
        }
        if (to_delete)
        {
            modified = true;
            pub_iter = pubs.erase(pub_iter);
        }
        else
            ++pub_iter;
            
    }
        
    return modified;
}


bool CRemoveInPressPubs::x_ApplyToPubdesc (CPubdesc& pubdesc)
{
    bool modified = false;
    auto& pubs = pubdesc.SetPub().Set();
    for (CPub_set::TPub::iterator pub_iter = pubs.begin(); pub_iter != pubs.end(); )
    {
        CPub& pub = **pub_iter;
        bool to_delete = false;
        switch( pub.Which() ) {           
        case CPub::e_Article:
        {
            const CCit_art& art = pub.GetArticle();
            if ( art.GetFrom().IsJournal() ) 
            {
                const CCit_jour& jour = art.GetFrom().GetJournal();
                if ( jour.IsSetImp() ) 
                {
                    const CImprint& imp = jour.GetImp();
                    if ( imp.IsSetPrepub() && imp.GetPrepub() == CImprint::ePrepub_in_press)
                    {
                        to_delete = true;
                    }
                }
            }
            if ( art.GetFrom().IsBook() ) 
            {
                const CCit_book& book = art.GetFrom().GetBook();
                if ( book.IsSetImp() )
                {
                    const CImprint& imp = book.GetImp();
                    if ( imp.IsSetPrepub() && imp.GetPrepub() == CImprint::ePrepub_in_press)
                    {
                        to_delete = true;
                    }
                }
            }
        }
        break;
        case CPub::e_Book:
        {
            const CCit_book& book = pub.GetBook();
            if ( book.CanGetImp() )
            { 
                const CImprint& imp = book.GetImp();
                if ( imp.IsSetPrepub() && imp.GetPrepub() == CImprint::ePrepub_in_press)
                {
                    to_delete = true;
                }
            }
        }
        break;
        case CPub::e_Journal:
        {
            const CCit_jour& jour = pub.GetJournal();
            if ( jour.IsSetImp() )
            {
                const CImprint& imp = jour.GetImp();
                    
                if ( imp.IsSetPrepub() && imp.GetPrepub() == CImprint::ePrepub_in_press)
                {
                    to_delete = true;
                }
            }
        }
        break;
        case CPub::e_Man:
        {
            const CCit_book& book = pub.GetMan().GetCit();
            if ( book.IsSetImp() )
            {
                const CImprint& imp = book.GetImp();
                if ( imp.IsSetPrepub() && imp.GetPrepub() == CImprint::ePrepub_in_press)
                {
                    to_delete = true;
                }
            }
        }
        break;
        default:
            break;
        }
        if (to_delete)
        {
            modified = true;
            pub_iter = pubs.erase(pub_iter);
        }
        else
            ++pub_iter;
    }
    return modified;
}


bool CRemovePublishedPubs::x_ApplyToPubdesc (CPubdesc& pubdesc)
{
    bool modified = false;
    auto& pubs = pubdesc.SetPub().Set();
    for (CPub_set::TPub::iterator pub_iter = pubs.begin(); pub_iter != pubs.end(); )
    {
        CPub& pub = **pub_iter;
        bool to_delete = false;
        switch( pub.Which() ) {
        case CPub::e_Gen:
        {
            const CCit_gen& gen = pub.GetGen();
            if ( gen.IsSetCit()  &&  !gen.GetCit().empty() ) {
                const string& cit = gen.GetCit();
                if (!NStr::StartsWith (cit, "unpublished", NStr::eNocase))
                {
                    // Cit-gen.cit != "unpublished"
                    to_delete = true;
                }
            }
        }
        break;
        case CPub::e_Patent:
            to_delete = true;
            break;
        case CPub::e_Article:
        {
            const CCit_art& art = pub.GetArticle();
            if ( art.GetFrom().IsJournal() ) 
            {
                const CCit_jour& jour = art.GetFrom().GetJournal();
                if ( jour.IsSetImp() ) 
                {
                    const CImprint& imp = jour.GetImp();
                    if ( !imp.IsSetPrepub() )
                    {
                        // Cit-art.from.jour.imp.prepub is 0
                        to_delete = true;
                    }
                }
            }
            if ( art.GetFrom().IsBook() ) 
            {
                const CCit_book& book = art.GetFrom().GetBook();
                if ( book.IsSetImp() )
                {
                    const CImprint& imp = book.GetImp();
                    if ( !imp.IsSetPrepub() )
                    {
                        // Cit-art.from.book.imp.prepub is 0 
                        to_delete = true;
                    }
                }
            }
        }
        break;
        case CPub::e_Book:
        {
            const CCit_book& book = pub.GetBook();
            if ( book.IsSetImp() )
            { 
                const CImprint& imp = book.GetImp();
                
                if ( !imp.IsSetPrepub() )
                {
                    // Cit-book where Cit-book.imp.prepub is 0
                    to_delete = true;
                }
            }
        }
        break;
        case CPub::e_Journal:
        {
            const CCit_jour& jour = pub.GetJournal();
            if ( jour.IsSetImp() )
            {
                const CImprint& imp = jour.GetImp();
                    
                if ( !imp.IsSetPrepub() )
                {
                    // Cit-jour where Cit-jour.imp.prepub is  0
                    to_delete = true;
                }
            }
        }
        break;
        case CPub::e_Man:
        {
            const CCit_book& book = pub.GetMan().GetCit();
            if ( book.IsSetImp() )
            {
                const CImprint& imp = book.GetImp();
                    
                if ( !imp.IsSetPrepub() )
                {
                    // Cit-let where Cit-let.imp.prepub is 0
                    to_delete = true;
                }
                    
            }
        }
        break;
        case CPub::e_Pmid:
            to_delete = true;
            break;
        default:
            break;
        }
        if (to_delete)
        {
            modified = true;
            pub_iter = pubs.erase(pub_iter);
        }
        else
            ++pub_iter;
    }
    return modified;
}


void CRemoveCollidingPubs::x_ApplyToSeqAndFeat(CCmdComposite* composite)
{
    m_SecondPass = false;
    IRemovePubs::x_ApplyToSeqAndFeat(composite);

    m_SecondPass = true;
    IRemovePubs::x_ApplyToSeqAndFeat(composite);
}

bool CRemoveCollidingPubs::x_ApplyToPubdesc(CPubdesc& pubdesc)
{
    bool modified = false;
    FOR_EACH_PUB_ON_PUBDESC(pub_iter, pubdesc) {
        const CPub& pub = **pub_iter;
        string label;
        pub.GetLabel(&label, CPub::eContent, CPub::fLabel_Unique, CPub::eLabel_V1);
        if (!m_SecondPass)
            m_collisions[label]++;
        else if (m_collisions[label] > 1)
            modified = true;
    }
    return modified;
}

bool CRemoveAllPubs::x_ApplyToPubdesc (CPubdesc& pubdesc)
{
    bool modified = false;
    auto& pubs = pubdesc.SetPub().Set();
    for (auto pub_iter = pubs.begin(); pub_iter != pubs.end(); )
    {
        if ((*pub_iter)->IsSub())
        {
            ++pub_iter;
        }
        else
        {
            pub_iter = pubs.erase(pub_iter);
            modified = true;
        }
    }
    return modified;
}


END_NCBI_SCOPE
