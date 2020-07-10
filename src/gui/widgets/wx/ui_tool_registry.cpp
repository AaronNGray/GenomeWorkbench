/*  $Id: ui_tool_registry.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Andrey Yazhuk
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistl.hpp>
#include <corelib/ncbireg.hpp>
#include <corelib/ncbifile.hpp>

#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/wx/ui_tool_registry.hpp>
#include <objects/general/User_field.hpp>

#include <stdio.h>
#include <set>

#include <wx/filename.h>
#include <wx/dir.h>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CUIToolRegistry

CRef<CUIToolRegistry>    CUIToolRegistry::sm_Instance;
DEFINE_STATIC_MUTEX(s_Mutex);


CUIToolRegistry::CUIToolRegistry()
{
}


CUIToolRegistry::~CUIToolRegistry()
{
    Clear();
}


void CUIToolRegistry::Reset()
{
    CMutexGuard LOCK(s_Mutex);
    _ASSERT(sm_Instance->ReferencedOnlyOnce());
    sm_Instance.Reset();
}


void CUIToolRegistry::ShutDown()
{
    GetInstance()->SaveSettings();
    GetInstance()->Clear();
    Reset();
}


CUIToolRegistry* CUIToolRegistry::GetInstance()
{
    if ( !sm_Instance ) {
        CMutexGuard LOCK(s_Mutex);
        if ( !sm_Instance ) {
            sm_Instance.Reset(new CUIToolRegistry());
        }
    }

    return sm_Instance.GetPointer();
}


void CUIToolRegistry::RegisterTool(IUITool* prototype, const string& tool_id)
{
    _ASSERT(prototype);
    if(!prototype) 
        return;
    bool registered = false;
    const string& name = prototype->GetName();
    auto it = m_NameToId.find(name);
    
    if (it == m_NameToId.end())    {   // first time - register
        
        string ltool_id = tool_id;
        if (ltool_id.empty())
            ltool_id = name;
        if (m_IdToRec.count(ltool_id) == 0) {
            m_NameToId[name] = ltool_id;
            // create a registration record for the method
            STemplateToolRecord* rec = new STemplateToolRecord();
            rec->m_Tool = prototype;
            m_IdToRec[ltool_id] = rec;
            registered = true;
        } 
    } 
    if (!registered) {
        ERR_POST("CUIToolRegistry::RegisterTool() - " << name <<
                 " method already registered");
    }
}


void CUIToolRegistry::RegisterTemplateTool(const string& class_name,
                                           ITemplateUITool* prototype)
{
    _ASSERT(prototype);

    TStrToTool::const_iterator it = m_ClassToTemplate.find(class_name);
    if(it != m_ClassToTemplate.end())   {
        _ASSERT(false);
        ERR_POST("CUIToolRegistry::RegisterTemplateTool() - class " <<
                 class_name << " already registred");
    } else {
        IUITool* tool = dynamic_cast<IUITool*>(prototype);
        if(tool)    {
            m_ClassToTemplate[class_name] = CIRef<IUITool>(tool);
        } else {
            _ASSERT(false);
            ERR_POST("CUIToolRegistry::RegisterTemplateTool() - prototype "
                     << " does implement IUITool interface.");
        }
    }
}


bool CUIToolRegistry::IsToolRegistered(const string& name) const
{
    return m_NameToId.count(name) > 0 || m_IdToRec.count(name) > 0;
}


static const char* kRegTag = "Scoring Tool Registry";

void CUIToolRegistry::LoadTemplateToolsInfo(TDirList& dirs)
{
    ITERATE (TDirList, path_iter, dirs) {

        wxString path_name = CSysPath::ResolvePath(*path_iter);
        if( !wxFileName::DirExists(path_name) ) {
            LOG_POST(Info << kRegTag << ": Directory \"" << path_name.ToUTF8() << "\" does not exist");
        } else {
            // scan this directory
            // we consider all files in this directory to be scoring method definitions

            wxString filename;
            wxFileName fname(path_name, wxEmptyString);
            wxDir dir(path_name);
            bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
            while ( cont ) {
                fname.SetFullName(filename);
                x_ReadToolInfo(fname.GetFullPath());
                cont = dir.GetNext(&filename);
            }
        }
    }
}


static const char* kInvalidDef = " - invalid method definition: ";


bool CUIToolRegistry::x_ReadToolInfo(const wxString& full_path)
{
    string error;
    CNcbiIfstream is(full_path.fn_str());

    if( ! is)   {
        ERR_POST(kRegTag  << " - failed to read alignment scoring method: " << full_path.ToUTF8());
        return false;
    }

    try {
        CNcbiRegistry reg(is);

        string tool_class = reg.GetString("Info", "Tool", "");
        if(tool_class.empty())  {
            // try the old key
            tool_class = reg.GetString("Info", "Method", "");
        }
        // check if Tool Class is registered
        TStrToTool::iterator it_class = m_ClassToTemplate.find(tool_class);
        if(it_class == m_ClassToTemplate.end())   {
            ERR_POST(kRegTag << kInvalidDef << " method \"" << tool_class
                     << "\" is not registered. Definition file: " << full_path.ToUTF8());
            return false;
        }

        string tool_id = reg.GetString("Info", "Id", "");
        if (tool_id.empty()) {
            tool_id = wxFileName::FileName(full_path).GetFullName();
        }
        if (m_IdToRec.count(tool_id) != 0) {
            ERR_POST(kRegTag << kInvalidDef << "Tool \"" << tool_id
                     << " is already registered. Definition file: " << full_path.ToUTF8());
            return false;
        }

        IUITool* clone = it_class->second->Clone();
        CIRef<ITemplateUITool> method(dynamic_cast<ITemplateUITool*>(clone));
        _ASSERT(method);

        if(method->LoadInfo(reg))   {
            string method_name = clone->GetName();

            // check if Tool Name is empty
            if (method_name.empty())    {
                ERR_POST(kRegTag << kInvalidDef << "Name is empty in " << full_path.ToUTF8());
                return false;
            }

            // check if Tool Name is unique
            TNameToId::iterator it_name = m_NameToId.find(method_name);
            if(it_name != m_NameToId.end()) {
                ERR_POST(kRegTag << kInvalidDef << "Name \"" << method_name
                        << " is already registered. Definition file: " << full_path.ToUTF8());
                return false;
            }
            m_NameToId[method_name] = tool_id;
            // create a registration record for the method
            STemplateToolRecord* rec = new STemplateToolRecord();
            rec->m_Tool = clone;
            //            rec->m_Template = method;
            rec->m_FileName = full_path;

            m_IdToRec[tool_id] = rec;  // register
            return true;
        } 
    } catch (CException& e) {
        error = e.GetMsg();
    } catch (std::exception& e) {
        error = e.what();
    }
    ERR_POST(kRegTag << kInvalidDef << " in file \"" << full_path.ToUTF8() << "\" " << error);
    return false;
}


const static char* kFailedToRead = " - failed to read alignment scoring method: ";

bool CUIToolRegistry::x_LoadTemplateTool(ITemplateUITool& method,
                                          const wxString& full_path)
{
    CNcbiIfstream is(full_path.fn_str());

    if( ! is)   {
        ERR_POST(kRegTag  <<  kFailedToRead
                          << "cannot open file \"" << full_path.ToUTF8() << "\"");
        return false;
    }

    CNcbiRegistry reg(is);

    bool ok = false;
    string error;
    try {
        ok = method.Load(reg);
    } catch(CException& e)  {
        error = e.GetMsg();
    } catch(std::exception& e)  {
        error = e.what();
    } 
    if( ! ok)    {
        ERR_POST(kRegTag << kFailedToRead << error
                         << " in \"" << full_path.ToUTF8() << "\"");
    }
    return ok;
}


void CUIToolRegistry::GetToolNames(vector<string>& names) const
{
    names.reserve(m_NameToId.size());
    ITERATE(TNameToId, it_r, m_NameToId)  {
         names.push_back(it_r->first);
    }
}


void CUIToolRegistry::GetToolNames(vector<string>& names, ISelector& selector) const
{

    ITERATE(TNameToId, it_r, m_NameToId){
        auto it = m_IdToRec.find(it_r->second);
        _ASSERT(it != m_IdToRec.end());
        if (it == m_IdToRec.end()) 
            continue;
        const STemplateToolRecord& rec = *it->second;
        if(selector.Select(*rec.m_Tool)) {
            names.push_back(it_r->first);
        }
    }
}



void CUIToolRegistry::GetTools(vector<CConstIRef<IUITool> >& tools, ISelector& selector) const
{
    ITERATE(TIdToTempRec, it, m_IdToRec)  {
        const STemplateToolRecord& rec = *it->second;
        //        const STemplateToolRecord& rec = *it_r->second;
        const IUITool& tool = *rec.m_Tool;
        if(selector.Select(tool)) {
            tools.push_back(CConstIRef<IUITool>(&tool));
        }
    }
}

void CUIToolRegistry::GetToolId(const string& name, string& tool_id) const
{
    auto it_name = m_NameToId.find(name);
    _ASSERT(it_name != m_NameToId.end());
    if (it_name == m_NameToId.end()) 
        tool_id = name;
    else
        tool_id = it_name->second;
}


const CUIToolRegistry::STemplateToolRecord*  CUIToolRegistry::x_GetToolRecByName(const string& name) const
{
    
    string tool_id = name;

    auto it_r = m_NameToId.find(name);
    if (it_r != m_NameToId.end()) { // else see if name is tool_id
        tool_id = it_r->second;
    }
    auto it = m_IdToRec.find(tool_id);
    if (it == m_IdToRec.end()) 
        return 0;
    return  it->second.get();
}



string CUIToolRegistry::GetToolDescription(const string& name) const
{
    const STemplateToolRecord* rec = x_GetToolRecByName(name);
    if (rec) 
        return rec->m_Tool->GetDescription();

    static string descr("not implemented");
    return descr;
}


CIRef<IUITool> CUIToolRegistry::CreateToolInstance(const string& method_name)
{

    const STemplateToolRecord* rec = x_GetToolRecByName(method_name);
    if (rec) {
        const IUITool& proto = *rec->m_Tool;
        // create the instance
        CIRef<IUITool> clone(proto.Clone());
        if (rec->m_FileName.IsEmpty()) // came from RegisterTool
            return clone;
            
        ITemplateUITool* tool =
        dynamic_cast<ITemplateUITool*>(clone.GetPointer());
        _ASSERT(tool);
        if(x_LoadTemplateTool(*tool, rec->m_FileName)) 
            return clone; // success
    }

    return CIRef<IUITool>(0);
}


void CUIToolRegistry::Clear()
{
    m_NameToId.clear();
    m_IdToRec.clear();
    m_ClassToTemplate.clear();
}



bool CUIToolRegistry::MRUListExists(const string& list) const
{
    TNameToMRUList::const_iterator it = m_NameToMRUList.find(list);
    return it != m_NameToMRUList.end();
}


static const int kDefMRUSize = 10;
static const int kMaxMRUSize = 20;


void CUIToolRegistry::CreateMRUList(const string& list, int size)
{
    if(MRUListExists(list)) {
        _ASSERT(false);
        ERR_POST("CUIToolRegistry::CreateMRUList() - MRU list " + list + " already exists.");
    } else {
        if(size < 1  ||  size > kMaxMRUSize) {
            _ASSERT(false);
            size = kDefMRUSize;
        }
        m_NameToMRUList[list] = TMRUList(size);
    }
}


void CUIToolRegistry::SetMaxMRUSize(const string& list, int size)
{
    TNameToMRUList::iterator it = m_NameToMRUList.find(list);
    if(it != m_NameToMRUList.end()) {
        if(size < 1  ||  size > kMaxMRUSize) {
            _ASSERT(false);
            ERR_POST("CUIToolRegistry::SetMaxMRUSize() - the provided value " <<
                     size << " is  out of range.");
        } else {
            it->second.SetMaxSize(size);
        }
    } else {
        _ASSERT(false);
        ERR_POST("CUIToolRegistry::SetMaxMRUSize() - the list " << list << " does not exist");
    }
}


void CUIToolRegistry::SetMRUToolNames(const string& list_name, const vector<string>& tool_names)
{
    TNameToMRUList::iterator it = m_NameToMRUList.find(list_name);
    if(it == m_NameToMRUList.end()) {
        _ASSERT(false);
        ERR_POST("CUIToolRegistry::SetMRUToolNames() - the list " << list_name
                 << " does not exist");
    } else {
        list<string> tmp;
        set<string> dups;
        ITERATE(vector<string>, it_n, tool_names)   {
            const string& name = *it_n;
            if(IsToolRegistered(name)  &&  dups.insert(name).second )   {
                tmp.push_back(name);
            }
        }

        it->second.SetItems(tmp);
    }
}


void CUIToolRegistry::GetMRUToolNames(const string& list_name, vector<string>& tool_names)
{
    TNameToMRUList::iterator it = m_NameToMRUList.find(list_name);
    if(it == m_NameToMRUList.end()) {
        _ASSERT(false);
        ERR_POST("CUIToolRegistry::SetMRUToolNames() - the list " << list_name <<
                 " does not exist");
    } else {
        const TMRUList& mru = it->second;
        const list<string>& items = mru.GetItems();

        //tool_names.insert(tool_names.begin(), items.begin(), items.end());
        std::copy(items.begin(), items.end(), back_inserter(tool_names));
    }
}


void CUIToolRegistry::AddToolToMRU(const string& list_name, const string& tool_name)
{
    TNameToMRUList::iterator it = m_NameToMRUList.find(list_name);
    if(it == m_NameToMRUList.end()) {
        _ASSERT(false);
        ERR_POST("CUIToolRegistry::SetMRUToolNames() - the list " << list_name <<
                 " does not exist");
    } else {
        it->second.AddItem(tool_name);
    }
}

void CUIToolRegistry::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}

static const char* kMRUSectionRegKey = "MRU Lists";

void CUIToolRegistry::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        /// load MRU lists
        CRegistryReadView mru_view = gui_reg.GetReadView(CGuiRegistryUtil::MakeKey(m_RegPath, kMRUSectionRegKey));

        m_NameToMRUList.clear();

        /// all key in mru_view should correspond to MRU lists
        CRegistryReadView::TKeys keys;
        mru_view.GetKeys(keys);
        ITERATE(CRegistryReadView::TKeys, it, keys) {
            const CRegistryReadView::SKeyInfo& key = *it;
            _ASSERT(key.type == objects::CUser_field::C_Data::e_Strs);

            string list_name = key.key;
            vector<string> names;
            mru_view.GetStringVec(list_name, names);

            CreateMRUList(list_name);
            SetMRUToolNames(list_name, names);
        }
    } else {
        _ASSERT(false);
        ERR_POST("CUIToolRegistry::LoadSettings() - empty registry path.");
    }
}


void CUIToolRegistry::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        CRegistryWriteView mru_view = view.GetWriteView(kMRUSectionRegKey);

        ITERATE(TNameToMRUList, it, m_NameToMRUList)    {
            const string& list_name = it->first;

            const TMRUList& mru = it->second;
            const list<string>& items = mru.GetItems();

            mru_view.Set(list_name, items);
        }
    } else {
        _ASSERT(false);
        ERR_POST("CUIToolRegistry::LoadSettings() - empty registry path.");
    }
}


END_NCBI_SCOPE
