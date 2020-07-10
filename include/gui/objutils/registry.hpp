#ifndef GUI_OBJUTILS___REGISTRY__HPP
#define GUI_OBJUTILS___REGISTRY__HPP

/*  $Id: registry.hpp 44845 2020-03-25 22:06:56Z rudnev $
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

#include <corelib/ncbiobj.hpp>

#include <objects/general/User_field.hpp>
#include <gui/objutils/reg_file.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CGuiRegistry : public CObject
{
public:
    static const string kDecimalDot;

    /// enum controlling priority of added registries
    enum EPriority {
        ePriority_Temp_Inc = 4,
        ePriority_Local = 0,
        ePriority_Global = 16384

    };

    CGuiRegistry();
    ~CGuiRegistry();

    /// @name Set-up and initialization functions
    /// @{

    /// establish our "local" repository.  The local repository is the
    /// highest priority repository; it is also the only repository that can
    /// be serialized directly with this interface
    void SetLocal(CNcbiIstream& istr);

    /// Set the "global" repository.  This is the lowest priority repostory
    /// and is read-only for the application.
    void SetGlobal(CNcbiIstream& istr);

    /// Add a site-specific repository.  This is a set of user-, role-, or
    /// site-wide common features that should be included.
    CRef<CRegistryFile> AddSite(CNcbiIstream& istr, int priority);
    CRef<CRegistryFile> AddSite(const objects::CUser_object& reg, int priority);


    /// @}

    /// @name Serialization functions
    /// @{

    /// Write the local policy to a specified stream.
    void Write(CNcbiOstream& ostr, int priority = ePriority_Local) const;

    /// Write the local policy to a specified stream, in INI format
    void ToConfigFile(CNcbiOstream& ostr) const;

    /// Initialize the local repository from an ini-format config file.
    void FromConfigFile(CNcbiIstream& istr);

    /// @}

    /// @name View-related functions
    /// @{

    /// get a read-only view at a particular level.  This provides limited
    /// access to a sub-key and all of its components.
    CRegistryReadView GetReadView(const string& section) const;

    /// get a read view that will overlay several named sub-sections in a given
    /// section.  This is useful for developing a polymorphic data structure
    /// that contains overloaded data members.  Note that the sections listed
    /// in sub_sections are presumed to be in increasing order of importance,
    /// so if you want a section named "overloaded" to replace one named
    /// "default", add them to the sections in the order "default",
    /// "overloaded"
    CRegistryReadView GetReadView(const string& section, const vector<string>& sub_sections) const;

protected:
    /// Add a section to an existing view at a lower priority than any previously existing
    /// sections in that view. Implementation for Get*View() methods.
    void    x_AppendToView(CRegistryReadView& view, const string& section) const;

public:
    /// get a read-write view at a particular level.  This provides limited
    /// access to a sub-key and all of its components.
    CRegistryWriteView GetWriteView(const string& section);
    CRegistryWriteView GetWriteView(const string& section, int writeSite);

    /// get a read-write view that will overlay several named sub-sections in a
    /// given section.  This is useful for developing a polymorphic data
    /// structure that contains overloaded data members.  Note that the
    /// sections listed in sub_sections are presumed to be in increasing order
    /// of importance, so if you want a section named "overloaded" to replace
    /// one named "default", add them to the sections in the order "default",
    /// "overloaded"
    CRegistryWriteView GetWriteView(const string& section, const vector<string>& sub_sections);

    /// @}

    /// @name Const-Access Functions
    /// @{

    /// retrieve values by section and key.  Sections and keys are recursively
    /// defined and use a specified set of delimiters; the delimiters are not
    /// serialized.  A policy for agglomeration is also provided; that is, if
    /// the specified key is found in more than one place, the default
    /// behavior can be modified.
    int GetInt(const string& key, int default_val = 0) const;

    double GetReal(const string& key, double default_val = 0) const;

    bool GetBool(const string& key, bool default_val = false) const;

    string GetString(const string& key, const string& default_val = kEmptyStr) const;

    void GetIntVec(const string& key, vector<int>& val) const;

    void GetRealVec(const string& key, vector<double>& val) const;

    void GetStringVec(const string& key, vector<string>& val) const;

    void GetStringList(const string& key, list<string>& val) const;

    /// retrieve the best user field object for our key, taking into account
    /// our multiple stores.
    /// throw an exception if not found.
    CConstRef<objects::CUser_field> GetField(const string& key) const;

    /// Does a field with this section and key exist in this view?
    bool HasField(const string& key) const;

    /// @}

    /// @name Non-Const accessors
    /// @{

    /// set specific values
    void Set(const string& key, int val);

    void Set(const string& key, double val);

    void Set(const string& key, bool val);

    void Set(const string& key, const string& val);

    void Set(const string& key, const char* val);

    void Set(const string& key, const vector<int>& val);

    void Set(const string& key, const vector<double>& val);

    void Set(const string& key, const vector<string>& val);

    void Set(const string& key, const list<string>& val);

    void Set(int site, const string& key, int val);

    void Set(int site, const string& key, double val);

    void Set(int site, const string& key, bool val);

    void Set(int site, const string& key, const string& val);

    void Set(int site, const string& key, const char* val);

    void Set(int site, const string& key, const vector<int>& val);

    void Set(int site, const string& key, const vector<double>& val);

    void Set(int site, const string& key, const vector<string>& val);

    void Set(int site, const string& key, const list<string>& val);

    /// retrieve the best user field object for our key, taking into account
    /// our multiple stores
    CRef<objects::CUser_field> SetField(const string& key);

    CRef<objects::CUser_field> SetField(int site, const string& key);

    /// same as SetField, but complain loudly if the field has subfields in it already.
    CRef<objects::CUser_field> SetFieldToValue(const string& key);

    CRef<objects::CUser_field> SetFieldToValue(int site, const string& key);

    /// delete the specified field (and any of its subfields).
    /// returns true on success,
    /// false if the field does not exist in the writable site,
    /// or if the field is at the top level of the registry.
    bool DeleteField(const string& key);

    bool DeleteField(int site, const string& key);

    /// Get the priority of the site that Set methods modify.
    int  GetWritableSite();

    /// Set which site Set methods modify.
    /// Creates the site if it did not exist.
    void  SetWritableSite(int priority = ePriority_Local);

    /// @}

    /// @name Singleton maintenance
    /// @{

    /// access the application-wide singleton
    static CGuiRegistry& GetInstance();

    // set this instance of registry to read-only mode
    // after this all GetWriteView() methods will fail
    // views created before that won't be invalidated
    void SetReadOnly() { m_isReadOnly = true; }

    // get an exact snapshot of this registry
    // it will be writable even if this registry is read-only 
    CRef<CGuiRegistry> Clone();

    /// @}

private:

    /// our registry stores
    /// this is a hierarchical organization

    /// the prioritized list of policies we're considering
    typedef map<int, CRef<CRegistryFile> > TRegistries;
    TRegistries m_Registries;

    /// The priority of the site that writes go to.
    int         m_WritableSite;

    bool m_isReadOnly{false};

    /// return the registry at a particular priority.
    /// Do not create if it does not exist.
    CRef<CRegistryFile>     x_GetRegistryRef(int) const;

    /// return a registry at a particular priority, creating it if need be.
    CRegistryFile&          x_SetRegistry(int);

    /// forbidden
    CGuiRegistry(const CGuiRegistry&);
    CGuiRegistry& operator=(const CGuiRegistry&);
};


class NCBI_GUIOBJUTILS_EXPORT CGuiRegistryUtil
{
public:
    /// create a key from a section and a subkey
    static string MakeKey(const string& section, const string& key,
                          const string& delim = CGuiRegistry::kDecimalDot);
};



END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___REGISTRY__HPP
