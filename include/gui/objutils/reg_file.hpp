#ifndef GUI_OBJUTILS___REG_FILE__HPP
#define GUI_OBJUTILS___REG_FILE__HPP

/*  $Id: reg_file.hpp 39689 2017-10-26 17:07:16Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */


#include <objects/general/User_object.hpp>
#include <gui/gui.hpp>

#include <gui/objutils/reg_view.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CRegistryFile : public CObject
{
public:

    CRegistryFile();
    CRegistryFile(CNcbiIstream& istr);
    CRegistryFile(const objects::CUser_object& obj);

    CRegistryReadView GetReadView(const string& section) const;
    CRegistryWriteView GetWriteView(const string& section);

    void FromConfigFile(CNcbiIstream& istr);
    void ToConfigFile(CNcbiOstream& istr) const;

    void Read(CNcbiIstream& istr);
    void Write(CNcbiOstream& istr) const;

    bool HasField(const string& key) const;

    CConstRef<objects::CUser_field> GetField(const string& key) const;

    CRef<objects::CUser_field> SetField(const string& key);

    CRef<objects::CUser_field> SetFieldToValue(const string& key);

    bool DeleteField(const string& key);

    objects::CUser_object& SetRegistry();
    /// precondition: IsRegistry(uo) is true.
    void SetRegistry(const objects::CUser_object& uo);
    const objects::CUser_object& GetRegistry() const;

    /// Is this user object suitable to use as a gui-registry site?
    static bool IsRegistry(const objects::CUser_object& uo);
    /// make a new user object suitable to use as a gui-registry site.
    static CRef<objects::CUser_object> NewRegistryObject();

private:
    CRef<objects::CUser_object> m_Registry;

    /// forbidden
    CRegistryFile(const CRegistryFile&);
    CRegistryFile& operator=(const CRegistryFile&);
};


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___REG_FILE__HPP
