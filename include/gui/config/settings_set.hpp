#ifndef GUI_CONFIG____SETTINGSSETREG_HPP
#define GUI_CONFIG____SETTINGSSETREG_HPP

/* $Id: settings_set.hpp 39688 2017-10-26 16:55:30Z katargir $
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
 * Author:  Robert G. Smith
 *
 */

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbistr.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/registry.hpp>
#include <gui/utils/rgba_color.hpp>
#include <objects/general/User_object.hpp>

#include <string>
#include <map>

BEGIN_NCBI_SCOPE

 /**
 * @class CSettingsSet
 *
 *   CSettingsSet is an adapter of the CGuiRegistry to provide persistance of
 *  and easy access to view's (or other plugins') configuration settings.
 *  You may use this as a base class for your plugin's configuration class,
 *  or you may include it as a member.  The last is especially handy if you
 *  do not need to add/delete/rename new sets of settings values (styles).
 *
 *  CSettingsSet expects a certain hierachy of keys inside its registry.
 *  Here is an example:
 *
 *      GBPlugins
 *          SequenceViewStuff
 *              name = Prefs for My Plugin
 *              current = default_set
 *              default_values
 *                  name = *
 *                  Key1 = default value 1
 *                  Key2.subkey1 = 32
 *                  Key2.color = blue
 *                  ...
 *              default_set
 *                  name = Default Style
 *              other set
 *                  name = Special Cool Style
 *                  Key2.subkey1 = 64
 *                  ....
 *          AlignViewThings
 *              name = Alignment View
 *              ...
 *
 *  GBPlugins is a required word.
 *  Below it are the keys of the various types of values this system knows about.
 *  Each instance of CSettingsSet tracks one of these types, and is initialized with
 *  its key.
 *
 *  In the next level down, the 'name' key for the type gives a user visible string
 *  shortly describing this type.  Currently it is displayed on the title bar of the
 *  configuration dialog.
 *
 *  The rest of the keys are groups of different value we can
 *  switch between.  These are called 'styles' or sometimes 'sets'.
 *  'current's value is the key of the currently selected style.
 *
 *  There should always be styles with the keys 'default_values' and 'default_set'.
 *  The default_values style should have a name of '*'. Any style with '*' as its name
 *  will not be offered as a choice to the user.
 *
 *  The default_values style should have every key that this CSettingsSet may access
 *  along with a default value for it.  Any other set may omit any key in which case
 *  we will get the value from 'default_values'.
 *  The set 'default_set' will usually be empty, though it can be modified, and will be
 *  what the user is shown first.
 *
 *  When accessing values from CSettingsSet you use the keys relative to the current style.
 *  e.g. (from the above hierarchy)
 *      GetString("Key1");
 *      GetInt("Key2.subkey1);
 *      GetColor("Key2.color");
 *
 */

BEGIN_SCOPE(objects)
    class CUser_field;
END_SCOPE(objects)


class NCBI_GUICONFIG_EXPORT CSettingsSet : public CObject
{
public:

    /// must be the same used as a default in CGuiRegistry.
    static const string kDefaultDelim;

    /// constructor
    /**
        @param type         The text key in the registry containing all values for this type.
        @param registry     A pointer to the registry we are going to wrap.
                If null, use the one initialized by CSettings from the files plugin_config.asn
                found in <sys>/etc and <home>.
    */
    CSettingsSet(const string& type,
                 CGuiRegistry* registry = 0);

    virtual ~CSettingsSet();

    /// uncache any cached lookup data
    void Uncache();

    /// Return the type passed in the constructor
    const string& GetType(void) const;
    string        GetCurrentStyleName(void) const;
    void          SetCurrentStyleName(const string& new_style);

    /// These are so that you can get data that is all default values.
    /// make sure you always restore the current style and you do not do any
    /// Set's to the data in between. Used by mediators.
    void SetDefaultCurrentStyle();
    void RestoreCurrentStyle();

    string GetTypeDescription(void) const;

    /**
        Methods to edit our list of valid styles.
        These do not change the current style, except DeleteStyle().
     */

    /// use to set up menu to chose between saved sets/styles.
    list<string>    GetStyleNames(void) const;
    /// these all return the name of the style just added.
    /// on failure they will return empty strings.
    string          AddStyle(void);
    string          DuplicateStyle(const string& style);
    string          RenameStyle(const string& old_style, const string& new_style);
    bool            CanRenameStyle(const string& style);
    // returns true on success, false on failure.
    bool            DeleteStyle(const string& style);

    /// @name Value Getters
    /// All getters retrieve values from the current style set.
    /// All value getters follow either of the patterns:
    ///
    ///    <value> GetXxx(const string& section, const string& key,
    ///                   <value> default_value = <some-value>);
    ///
    /// or
    ///
    ///    void GetXxxVec(const string& section, const string& key,
    ///                   <value>& output);
    /// @{

    /// Retrieve a named string value from a section.key
    string GetString(const string& key,
                     const string& default_val = kEmptyStr) const;

    /// Retrieve a named int value from a section.key
    int GetInt(const string& key,
               int default_val = 0) const;

    /// Retrieve a named double value from a section.key
    double GetReal(const string& key,
                   double default_val = 0) const;

    /// Retrieve a named bool value from a section.key
    bool GetBool(const string& key,
                 bool default_val = false) const;

    /// Retrieve a named color value from a section.key
    CRgbaColor GetColor(const string& key,
                        const CRgbaColor& default_val = CRgbaColor()) const;

    /// Retrieve a named int vector value from a section.key
    void GetIntVec(const string& key,
                   vector<int>& val) const;

    /// Retrieve a named real vector value from a section.key
    void GetRealVec(const string& key,
                    vector<double>& val) const;

    /// Retrieve a named string vector value from a section.key
    void GetStringVec(const string& key,
                      vector<string>& val) const;

    /// @}

    /// @name Value Setters
    /// All getters write values to the current style set.
    /// @{

    /// set a named integer value
    void Set(const string& key, int val);

    /// set a named real value
    void Set(const string& key, double val);

    /// set a named bool value
    void Set(const string& key, bool val);

    /// ste a named string value
    void Set(const string& key, const string& val);

    /// set a named string value as a const char* (avoids implicit
    /// conversion to bool)
    void Set(const string& key, const char* val);

    /// set a named value as a vector of ints
    void Set(const string& key, const vector<int>& val);

    /// set a named value as a vector of reals
    void Set(const string& key, const vector<double>& val);

    /// set a named value as a vector of strings
    void Set(const string& key, const vector<string>& val);

    /// set a named value as a color
    void Set(const string& key, const CRgbaColor& val);

    /// @}

    /// Delete a key/value in the current settings.
    /// returns true if the key was found and deleted, false if not.
    bool  Delete(const string& key);


    /// we use int instead of CSeqFeatData::ESubtype for two reasons:
    ///  - avoids unnecessary includes
    ///  - gives transparent warning-free compiles with generic serialized
    ///    code (i.e., when we serialize a subtype, it's as an int not as an
    ///    enum)
    typedef int TFeatSubtype;

    /// retrieve the key name to read values for a given feature subtype
    string GetFeatReadKey(TFeatSubtype feat_subtype, const string& section,
                          const string& subkey = kEmptyStr) const;

    /// retrieve the key name to write values for a given feature subtype
    static string GetFeatWriteKey(TFeatSubtype feat_subtype, const string& section);

    /// write all keys and values we know about.
    void    DumpAll(CNcbiOstream& ostr);

    /// convert a color into a vector of ints within a user field
    static void ColorToUserField(const CRgbaColor& c, objects::CUser_field& f);

    /// convert a user field into a color.
    /// The user field can be a vector of ints, a vector of doubles or a string.
    /// If you run across colors encoded in user fields some other way put them in here.
    /// returns true if succesful. false otherwise.
    static bool UserFieldToColor(const objects::CUser_field& f, CRgbaColor& c);

    /// style keys are only for internal use.
    string          GetCurrentStyleKey(void) const;
    void            SetCurrentStyleKey(const string& new_style_key);

protected: //###

    static const string     sm_DefaultValuesKey; ///< the key for the default set of values.


    /// every style has a name and a key. Both should be unique within this Type.
    /// translate between style names and keys.
    /// We store keys.  We display names.
    string        x_GetStyleName(const string& key) const;
    string        x_GetStyleKey(const string& name) const;
    /// make unique names and keys.
    /// result may be different than the argument.
    string      x_MakeUniqueStyleName(const string& name) const;
    string      x_MakeUniqueStyleKey(const string& key) const;

    /// Retrieve a read-only view.  This is protected to make sure that caching can
    /// work effectively; clients should use the GetXXX() functions instead.
    CRegistryReadView
        x_GetReadView(const string& section) const;

private:

    /// get the full key string used to get this type's part of the registry.
    string x_GetSection() const;

    /// get the full key string used to get the current style's part of the registry.
    string x_GetStyleSection() const;

    /// get a view containing the values given a certain style.
    CRegistryReadView x_GetStyleView(const string& style_key) const;

    /// get a view showing the values under the current style.
    CRegistryReadView x_GetCurrentStyleView() const;

    /// @name Cached access to fields
    /// @{

    /// retrieve (and cache) a field
    CConstRef<objects::CUser_field>
        x_GetCurrentViewField(const string& key) const;

    /// uncache a given key
    void x_Uncache(const string& key) const;

    /// invalidate the entire cache
    void x_UncacheAll() const;

    /// dictionary for caching look-ups into CGuiRegistry
    typedef std::map<string, CConstRef<objects::CUser_field> > TFieldDictionary;
    mutable TFieldDictionary m_FieldDictionary;

    /// dictionary to cache string -> color lookups
    typedef std::map<string, CRgbaColor> TColorDictionary;
    mutable TColorDictionary m_ColorDictionary;
    /// @}

    CRef<CGuiRegistry>  m_Registry;     ///> the registry these views come from.
    string              m_Type;         ///> key used in cache.
    string              m_SavedStyle;   ///> remember the current style when getting default values.
};



END_NCBI_SCOPE

#endif // GUI_CONFIG____SETTINGSSETREG_HPP
