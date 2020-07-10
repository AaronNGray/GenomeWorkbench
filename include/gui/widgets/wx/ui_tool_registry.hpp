#ifndef GUI_WIDGETS_WX___UI_TOOL_REGISTRY__HPP
#define GUI_WIDGETS_WX___UI_TOOL_REGISTRY__HPP

/*  $Id: ui_tool_registry.hpp 37435 2017-01-10 23:15:14Z shkeda $
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
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistl.hpp>

#include <map>

#include <gui/utils/mru_list.hpp>

#include <gui/widgets/wx/ui_tool.hpp>
#include <gui/objutils/registry.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CUIToolRegistry - is a registry providing access to various tools
/// registred in the application. Tools can be based on data files stored
/// in dedicated directories or they can be purely programmatic.
/// The registry may create methods of two types - concrete and template.  A
/// concrete method is fully described by its class name and does not require
/// any addition information. A template method describes a class of similar
/// methods (such as matrix-based), a specific instance from this class can be
/// created based on the external data (such as matrix).
/// The registry supports a MRU list of methods.

class NCBI_GUIWIDGETS_WX_EXPORT CUIToolRegistry
    :   public  CObject
{
    typedef list<wxString>    TDirList;

public:
    class ISelector {
    public:
        virtual ~ISelector()    {};
        virtual bool    Select(const IUITool& tool) = 0;
    };

    static CUIToolRegistry* GetInstance();
    static void             Reset();
    static void             ShutDown();

    ~CUIToolRegistry();

    /// register a concrete method, every method shall be registered only once
    /// the Regisrty assumes ownership of the provided object
    void    RegisterTool(IUITool* prototype, const string& tool_id = NcbiEmptyString);

    /// register a template method with its class name
    /// the Regisrty assumes ownership of the provided object
    void    RegisterTemplateTool(const string& class_name,
                                 ITemplateUITool* prototype);

    bool    IsToolRegistered(const string& class_name) const;

    /// loads descriptors for all template methods installed in the system
    void    LoadTemplateToolsInfo(TDirList& dirs);

    /// returns names for all Tools available
    void    GetToolNames(vector<string>& names) const;

    /// returns names for all Tools selected by the given Selector
    void    GetToolNames(vector<string>& names, ISelector& selector) const;

    void    GetTools(vector<CConstIRef<IUITool> >& tools, ISelector& selector) const;
    
    // returns tool id by name
    void GetToolId(const string& name, string& tool_id) const;

    string GetToolDescription(const string& name) const;

    /// factory method, returns NULL if name is invalid. The caller must
    /// take ownership of the object.
    CIRef<IUITool>  CreateToolInstance(const string& method_name);

    /// clears descriptors and unregisters all methods
    void    Clear();

    /// @name MRU API
    /// @{
    bool    MRUListExists(const string& list_name) const;
    void    CreateMRUList(const string& list_name, int size = 10);
    void    SetMaxMRUSize(const string& list_name, int size);
    void    SetMRUToolNames(const string& list_name, const vector<string>& tool_names);
    void    GetMRUToolNames(const string& list_name, vector<string>& tool_names);
    /// add a method to the MRU list
    void    AddToolToMRU(const string& list, const string& tool_name);
    /// @}

    /// @name Settings support
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    bool    x_LoadTemplateTool(ITemplateUITool& method,
                               const wxString& full_path);

    bool    x_ReadToolInfo(const wxString& full_path);

private:
    CUIToolRegistry(); // this is a singleton


protected:
    static  CRef<CUIToolRegistry> sm_Instance;

    struct STemplateToolRecord    {
        // we store both pointers to avoid dynamic casts at runtime
        CIRef<IUITool>  m_Tool;
        //        CIRef<ITemplateUITool> m_Template;
        wxString m_FileName;
    };
    const STemplateToolRecord*  x_GetToolRecByName(const string& name) const;

    typedef map<string, CIRef<IUITool> >    TStrToTool;
    typedef map<string, AutoPtr<STemplateToolRecord> >     TIdToTempRec;
    typedef map<string, string> TNameToId;

    TDirList    m_List;
    TStrToTool  m_ClassToTemplate;   // storage for template methods
    TIdToTempRec  m_IdToRec;    // storage for template methods
    TNameToId m_NameToId;

    typedef CMRUList<string>    TMRUList;
    typedef map<string, TMRUList>   TNameToMRUList;
    TNameToMRUList  m_NameToMRUList;

    string m_RegPath; // path to the setting in CGuiRegistry
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___UI_TOOL_REGISTRY__HPP
