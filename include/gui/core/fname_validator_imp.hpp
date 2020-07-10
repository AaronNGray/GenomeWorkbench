#ifndef GUI_CORE___FNAME_VALIDATOR_IMP__HPP
#define GUI_CORE___FNAME_VALIDATOR_IMP__HPP

/*  $Id: fname_validator_imp.hpp 30032 2014-03-31 18:24:44Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/core/fname_validator.hpp>

BEGIN_NCBI_SCOPE

// a validator for CFolderEditDlg - makes sure that entered folder name is
// unique and not empty
template<class TFolder>
    class CFolderNameValidator : public IFNameValidator
{
public:
    CFolderNameValidator(TFolder* parent, const string& old_name)
        : m_Parent(parent), m_OldName(old_name)  {}

    virtual bool    IsValid(const string& name, string& err)
    {
        err = "";
        if(name.empty())  {
            err = "Folder name cannot be empty!";
            return false;
        }
        if(name != m_OldName)    {   // old name is ok
            if(m_Parent  &&  m_Parent->FindChildFolderByTitle(name))  {
                err = "Folder \"" + name + "\" already exists! Please choose a different name.";
                return false;
            }
        }
        return true;
    }

protected:
    TFolder* m_Parent;
    string  m_OldName;
};


// a validator for CFolderEditDlg - makes sure that entered project item name is
// unique and not empty
class CPrjItemNameValidator : public IFNameValidator
{
public:
    CPrjItemNameValidator(objects::CProjectFolder& parent, const string& old_name)
        : m_Parent(parent), m_OldName(old_name)  {}

    virtual bool    IsValid(const string& name, string& err)
    {
        err = "";
        if(name.empty())  {
            err = "Project Item name cannot be empty!";
            return false;
        }
        if(name != m_OldName)    {   // old name is ok
            if(m_Parent.FindProjectItemByLabel(name))  {
                err = "Project Item \"" + name + "\" already exists!\n Please choose a different name.";
                return false;
            }
        }
        return true;
    }

protected:
    objects::CProjectFolder& m_Parent;
    string  m_OldName;
};

END_NCBI_SCOPE

#endif // GUI_CORE___FNAME_VALIDATOR_IMP__HPP
