#ifndef GUI_OBJUTILS___REG_VIEW__HPP
#define GUI_OBJUTILS___REG_VIEW__HPP

/*  $Id: reg_view.hpp 39783 2017-11-03 15:04:54Z katargir $
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


#include <gui/gui.hpp>
#include <gui/gui_export.h>

#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>

BEGIN_NCBI_SCOPE

class CRegistryFile;

///
/// class CRegistryReadView provides a nested hierarchical view at a
/// particular key.  This is a light-weight object that any observer
/// can hold onto for the life of the application.  It provides read-only
/// access to the registry at the key from which it was obtained; it cannot
/// be used to access anything in the list of parent keys.  If the key
/// that was used to create the view doesn't exist and is added subsequent
/// to obtaining the view, that key will not show up in the list of keys
/// for this view.
///

class NCBI_GUIOBJUTILS_EXPORT CRegistryReadView
{
public:
    friend class CRegistryFile;

    struct SKeyInfo {
        string key;
        objects::CUser_field::TData::E_Choice type;
    };

    /// retrieve information about all keys in the registry
    typedef list<SKeyInfo> TKeys;

    CRegistryReadView() {}

    CRegistryReadView& operator+=(const CRegistryReadView& rhs);

    CRegistryReadView GetReadView(const string& section) const;
    CRegistryReadView GetReadView(const string& section,
                                  const vector<string>& subsections) const;

public:
    /// access a named key at this level, with no recursion
    int    GetInt(const string& key, int default_val = 0) const;
    double GetReal(const string& key, double default_val = 0) const;
    bool   GetBool(const string& key, bool default_val = false) const;
    string GetString(const string& key, const string& default_val = kEmptyStr) const;

    void GetIntVec(const string& key, vector<int>& val) const;
    void GetRealVec(const string& key, vector<double>& val) const;
    void GetStringVec(const string& key, vector<string>& val) const;
    void GetStringList(const string& key, list<string>& val) const;

    /// provide raw field access
    CConstRef<objects::CUser_field> GetField(const string& key) const;


    /// Retrieve information about the top level keys in this view.
    /// The information in the access field is invalid.
    void GetTopKeys(TKeys& keys) const;

    /// Retrieve information about all keys in this view.
    /// The information in the access field is invalid.
    void GetKeys(TKeys& keys) const;

    /// Does a field with this section and key exist in this view?
    bool HasField(const string& key) const;

    /// Dump onto the stream all the keys in this view along with their
    /// types and data values.
    void DumpAll(CNcbiOstream& ostr) const;

    /// There is nothing in this view. Probably key(s) used to get it were
    /// wrong.
    bool IsEmpty();

    static int GetInt(const objects::CUser_field* field, int default_val);
    static double GetReal(const objects::CUser_field* field, double default_val);
    static bool GetBool(const objects::CUser_field* field, bool default_val);
    static string GetString(const objects::CUser_field* field, const string& default_val);
    static void GetIntVec(const objects::CUser_field* field, vector<int>& val);
    static void GetRealVec(const objects::CUser_field* field, vector<double>& val);
    static void GetStringVec(const objects::CUser_field* field, vector<string>& val);

    static CConstRef<objects::CUser_field> ResolveField
        (list< CConstRef<objects::CUser_field> >& fields, const objects::CUser_field* meta);

protected:
    CRegistryReadView(const objects::CUser_field* userField);

    /// implementing GetTopKeys and GetKeys.
    void x_GetKeys(TKeys& keys, bool recurse) const;

    void x_AppendToReadView(CRegistryReadView& view, const string& section) const;

    /// ordered list of subfields to scan
    typedef list< CConstRef<objects::CUser_field> > TFields;
    TFields m_Fields;
};

class NCBI_GUIOBJUTILS_EXPORT CRegistryWriteView
{
public:
    friend class CRegistryFile;

    CRegistryWriteView() : m_RegistryFile() {}

    CRegistryWriteView GetWriteView(const string& section);

    /// access a named key at this level, with no recursion
    void Set(const string& key, int val);
    void Set(const string& key, double val);
    void Set(const string& key, bool val);
    void Set(const string& key, const string& val);
    void Set(const string& key, const char* val);
    void Set(const string& key, const vector<int>& val);
    void Set(const string& key, const vector<double>& val);
    void Set(const string& key, const vector<string>& val);
    void Set(const string& key, const list<string>& val);

    /// provide raw field access
    CRef<objects::CUser_field> SetField(const string& key);

    /// same as SetField, but complain loudly if the field has subfields in
    /// it already.
    CRef<objects::CUser_field> SetFieldToValue(const string& key);

    /// delete the specified field (and any of its subfields)
    /// from this view and from its registry.
    /// returns true on success,
    /// false if the field does not exist in the writable site.
    bool DeleteField(const string& key);

private:
    CRegistryWriteView(const string &section, CRegistryFile& regFile);

    string         m_Section;
    CRegistryFile* m_RegistryFile;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___REG_VIEW__HPP
