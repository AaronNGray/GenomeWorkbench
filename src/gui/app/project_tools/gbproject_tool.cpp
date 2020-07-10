/*  $Id: gbproject_tool.cpp 34680 2016-02-01 22:06:08Z katargir $
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
 *     Application for manipulating GBench projects.
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbiargs.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/serial.hpp>
#include <objtools/readers/gff_reader.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/NCBI_Sequence_module.hpp>
#include <objects/seqtable/NCBI_SeqTable_module.hpp>
#include <gui/objects/GBProjectHandle.hpp>
#include <objects/gbproj/GBProject_ver2.hpp>
#include <objects/gbproj/ProjectDescr.hpp>
#include <objects/gbproj/ProjectFolder.hpp>
#include <objects/gbproj/FolderInfo.hpp>
#include <objects/gbproj/ProjectItem.hpp>
#include <gui/objutils/label.hpp>
#include <objects/general/Date.hpp>
#include <objmgr/scope.hpp>
#include <objtools/simple/simple_om.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>

#include "fileloader.hpp"
#include "projectfilewriter.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

typedef CFormatGuess::EFormat EFormat;
auto_ptr<CProjectFileWriter> Writer;

class CGBProjectTool : public CNcbiApplication
{
private:
    void Init();
    int Run();

    void x_LoadGeneralData(CProjectItem& item,
                           ESerialDataFormat fmt,
                           CNcbiIstream& istr);

    void x_LoadGffData(CProjectItem& item,
                       CNcbiIstream& istr);


    void x_AddItem(CGBProjectHandle& project,
                   CNcbiIstream& istr,
                   const string& fmt_str,
                   const string& in_folder);
};


void CGBProjectTool::Init()
{
    NCBI_Sequence_RegisterModuleClasses();
    NCBI_SeqTable_RegisterModuleClasses();

    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions());
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "Application for manipulating GBench projects");

    arg_desc->AddDefaultKey("i", "InputFile",
                            "Input project file",
                            CArgDescriptions::eInputFile,
                            "-");

    arg_desc->AddDefaultKey("o", "OutputFile",
                            "Output project file",
                            CArgDescriptions::eOutputFile,
                            "-");

    arg_desc->AddFlag("noid", "disable access to genbank or id and id2");
    arg_desc->AddFlag("list", "List the contents of a project file");
    arg_desc->AddFlag("create", "Create a new project file");

    arg_desc->AddOptionalKey("expand", "expand",
                             "Create a subdirectory tree from a project file",
                             CArgDescriptions::eInputFile);
    arg_desc->AddDefaultKey("expand-fmt", "expand_format",
                            "Format to expand project file parts out in",
                            CArgDescriptions::eString, "asnb");
    arg_desc->SetConstraint("expand-fmt", &((*new CArgAllow_Strings(NStr::eNocase)), "asnt", "asnb", "xml", "gtf"));


    arg_desc->AddOptionalKey("collapse", "collapse",
                             "Create a project file from a directory tree",
                             CArgDescriptions::eInputFile);


    arg_desc->AddOptionalKey("title", "Title",
                             "Title for new project",
                             CArgDescriptions::eString);

    arg_desc->AddOptionalKey("add", "ItemToAdd",
                             "Add an item to a project file",
                             CArgDescriptions::eInputFile);
    arg_desc->AddOptionalKey("fmt", "ItemFormat",
                             "Format of input item",
                             CArgDescriptions::eString);
    arg_desc->SetConstraint("fmt",
                            &(*new CArgAllow_Strings,
                              "gff", "asn", "asnb", "agp"));
    arg_desc->AddOptionalKey("proj-folder", "FolderName",
                             "Add item into the named folder",
                             CArgDescriptions::eString);
    arg_desc->SetDependency("add", CArgDescriptions::eRequires, "fmt");

    arg_desc->AddOptionalKey("add-folder", "FolderToAdd",
                             "Create a folder in the existing project",
                             CArgDescriptions::eString);

    arg_desc->AddOptionalKey("rm", "RemoveItem",
                             "Remove an item from an existing project",
                             CArgDescriptions::eString);
    arg_desc->AddOptionalKey("rmdir", "RemoveFolder",
                             "Remove a folder from an existing project",
                             CArgDescriptions::eString);

    SetupArgDescriptions(arg_desc.release());
}


static void s_ListProjectFolder(const CProjectFolder& folder,
                                CNcbiOstream& ostr,
                                int level = 0)
{
    const string kSpacer(level * 2, ' ');

    ostr << kSpacer << "[folder] " << folder.GetInfo().GetTitle()
        << " (created: "
        << folder.GetInfo().GetCreate_date().AsCTime().AsString("M/D/Y h:m:s")
        << ")" << endl;

    if (folder.IsSetFolders()) {
        ITERATE (CProjectFolder::TFolders, iter, folder.GetFolders()) {
            s_ListProjectFolder(**iter, ostr, level + 1);
        }
    }

    if (folder.IsSetItems()) {
        ITERATE (CProjectFolder::TItems, iter, folder.GetItems()) {
            string s;
            CLabel::GetLabel(**iter, &s);
            ostr << kSpacer << "  " << "[item:"
                << (*iter)->GetId() << "] " << s << endl;
        }
    }
}



static void s_ScopeProjectFolder(CRef<CScope> Scope,
                                 const CProjectFolder& folder)
{
    if (folder.IsSetFolders()) {
        ITERATE (CProjectFolder::TFolders, iter, folder.GetFolders()) {
            s_ScopeProjectFolder(Scope, **iter);
        }
    }

    if (folder.IsSetItems()) {
        ITERATE (CProjectFolder::TItems, iter, folder.GetItems()) {
            Writer->AddToScope(CConstRef<CSerialObject>(((*iter)->GetObject())));
        }
    }
}


static void s_ExpandProjectFolder(CRef<CScope> scope,
                                  const CProjectFolder& folder,
                                  CDir& parentDir,
                                  EFormat format,
                                  int level = 0)
{
    const string kSpacer(level * 2, ' ');

    string currentName = CDirEntry::ConcatPathEx(parentDir.GetPath(),
                                                 folder.GetInfo().GetTitle());
    CDir   currentDir(currentName);
    if(level == 0)  // get rid of the /Data/ dir thats always on the top when expanding
        currentDir = parentDir;
    currentDir.CreatePath();
    cout << kSpacer << "[folder] " << folder.GetInfo().GetTitle()
        << " (created: "
        << folder.GetInfo().GetCreate_date().AsCTime().AsString("M/D/Y h:m:s")
        << ")" << "    " << currentName << endl;

    if (folder.IsSetFolders()) {
        ITERATE (CProjectFolder::TFolders, iter, folder.GetFolders()) {
            s_ExpandProjectFolder(scope, **iter, currentDir, format, level + 1);
        }
    }

    if (folder.IsSetItems()) {
        ITERATE (CProjectFolder::TItems, iter, folder.GetItems()) {
            string itemTitle;
            CLabel::GetLabel(**iter, &itemTitle);
            string itemFilename = CDirEntry::ConcatPathEx(currentDir.GetPath(), itemTitle);
            cout << kSpacer << "  " << "[item:"
                << (*iter)->GetId() << "] " << itemTitle << "  " << itemFilename << endl;

            Writer->WriteFile(itemFilename,
                             CConstRef<CSerialObject>(((*iter)->GetObject())),
                             format);
            //auto_ptr<CObjectOStream> ObjS(CObjectOStream::Open(format, itemFilename));
            //*ObjS << *((*iter)->GetObject());
        }
    }
}

bool s_Compare_DirEntry_ModDate(CDirEntry* A, CDirEntry* B)
{
	time_t mod_a, mod_b;
	A->GetTimeT(&mod_a);
	B->GetTimeT(&mod_b);
	return (mod_a < mod_b);
}

static void s_CollapseProjectFolder(CRef<CScope> scope,
                                    CGBProjectHandle& project,
                                    CProjectFolder& folder,
                                    CDir& dir)
{
    list< AutoPtr< CDirEntry > > children =
        dir.GetEntries(kEmptyStr, CDir::fIgnoreRecursive);

	vector< CDirEntry* > children_vec;
	ITERATE(list<AutoPtr<CDirEntry> >, childIter, children) {
		CDirEntry* New = new CDirEntry(**childIter);
		children_vec.push_back(New);
	}
	sort(children_vec.begin(), children_vec.end(), s_Compare_DirEntry_ModDate);

    typedef multimap<string, CRef<CProjectItem> > TItemMap;
    TItemMap items;

    string CurrLabel;
    NON_CONST_ITERATE(vector<CDirEntry*>, childIter, children_vec) {

        // Save the file name early, in case its actually a symbolic link.
        CurrLabel = (*childIter)->GetName();

        // Then if its a link, dereference it
        if( (*childIter)->IsLink() ) {
            (*childIter)->DereferenceLink();
        }

        if( (*childIter)->IsDir() ) {
            CRef<CProjectFolder> childFolder(new CProjectFolder);
            childFolder->SetInfo().SetCreateDate(CTime(CTime::eCurrent));
            childFolder->SetInfo().SetTitle(CurrLabel);
            cout << "Dir   " << CurrLabel << endl;
            folder.SetFolders().push_back(childFolder);
            CDir childDir(**childIter);
            s_CollapseProjectFolder(scope, project, *childFolder, childDir);
        }
        else if( (*childIter)->IsFile() ) {

            cout << "File  " << CurrLabel << endl;
            CFileLoader::TSerialObjectList Objects;
            CFileLoader::LoadFile( (*childIter)->GetPath(), Objects);

            if(Objects.empty())
                continue;

            ITERATE(CFileLoader::TSerialObjectList, ObjIter, Objects) {

                CRef<CSerialObject> Object = *ObjIter;
                string label = CurrLabel;

                if(Object->GetThisTypeInfo() != NULL &&
                   Object->GetThisTypeInfo()->GetName() == "Seq-annot") {
                    CRef<CSeq_annot> Annot(static_cast<CSeq_annot*>(Object.GetPointer()));
                    if (Annot->CanGetDesc()) {
                        CConstRef<CAnnotdesc> name;
                        CConstRef<CAnnotdesc> title;
                        ITERATE(CSeq_annot::TDesc::Tdata, desciter, Annot->GetDesc().Get()) {
                            if((*desciter)->IsName()) {
                                name = *desciter;
                            } else if ((*desciter)->IsTitle()) {
                                title = *desciter;
                            }
                        }
                        if (title) {
                            label = title->GetTitle();
                        } else if (name) {
                            label = name->GetName();
                        }
                    }
                    else{
                        Annot->SetNameDesc(CurrLabel);
                        Annot->SetTitleDesc(CurrLabel);
                    }
                } else if(Object->GetThisTypeInfo() != NULL &&
                          Object->GetThisTypeInfo()->GetName() == "Seq-align") {
                    CRef<CSeq_align> Align(static_cast<CSeq_align*>(Object.GetPointer()));
                    string AlignTitle;
                    CSeq_id_Handle Id1, Id2;
                    string Id1Str, Id2Str;
                    Id1 = scope->GetAccVer(CSeq_id_Handle::GetHandle(Align->GetSeq_id(0)));
                    Id2 = scope->GetAccVer(CSeq_id_Handle::GetHandle(Align->GetSeq_id(1)));
                    if(Id1.GetSeqIdOrNull().IsNull())
                        Id1Str = Align->GetSeq_id(0).GetSeqIdString(true);
                    else
                        Id1Str = Id1.GetSeqId()->GetSeqIdString(true);
                    if(Id2.GetSeqIdOrNull().IsNull())
                        Id2Str = Align->GetSeq_id(1).GetSeqIdString(true);
                    else
                        Id2Str = Id2.GetSeqId()->GetSeqIdString(true);
                    AlignTitle = Id1Str + " x " + Id2Str;
                    label = AlignTitle;
                } else if(Object->GetThisTypeInfo() != NULL &&
                          Object->GetThisTypeInfo()->GetName() == "Project-item") {
                    CRef<CProjectItem> Item(static_cast<CProjectItem*>(Object.GetPointer()));
                    items.insert(TItemMap::value_type(Item->GetLabel(), Item));
                    continue;
                }

                CRef<CProjectItem> childItem(new CProjectItem);
                CTime CreateTime;
                (*childIter)->GetTime(&CreateTime);
                childItem->SetCreateDate(CreateTime);

                if (Object->GetThisTypeInfo() == CProjectItem::GetTypeInfo()) {
                    childItem.Reset(dynamic_cast<CProjectItem*>(Object.GetPointer()));
                } else {
                    childItem->SetLabel(label);
                    childItem->SetObject(*Object);
                    cout << "    Title: " << label << endl;
                }
                items.insert(TItemMap::value_type(label, childItem));
            }
        }
    
		delete *childIter;
		*childIter = NULL;
	}

    NON_CONST_ITERATE(TItemMap, it, items) {
        project.AddItem(*it->second, folder);
    }

}


static void s_RemoveItem(CGBProjectHandle& project,
                         CProjectFolder& folder,
                         const string& label)
{
    if (folder.IsSetFolders()) {
        NON_CONST_ITERATE(CProjectFolder::TFolders, iter, folder.SetFolders()) {
            s_RemoveItem(project, **iter, label);
        }
    }

    if (folder.IsSetItems()) {
        NON_CONST_ITERATE(CProjectFolder::TItems, iter, folder.SetItems()) {
            string s;
            CLabel::GetLabel(**iter, &s);

            if(s == label) {
                NCBI_THROW(CException, eUnknown,
                           "FIXME: CGBProjectHandle::RemoveItem() not supported");
                //project.RemoveItem(**iter);
                return;
                //iter = folder.SetItems().erase(iter);
            }
        }
    }
}


static void s_RemoveFolder(CGBProjectHandle& project,
                           CProjectFolder& folder,
                           const string& label)
{
    if (folder.IsSetFolders()) {
        NON_CONST_ITERATE(CProjectFolder::TFolders, iter, folder.SetFolders()) {
            if( (*iter)->GetInfo().GetTitle() == label)
                iter = folder.SetFolders().erase(iter);
            else
                s_RemoveFolder(project, **iter, label);
        }
    }
}


CRef<CProjectFolder> s_GetOrCreateFolder(CGBProjectHandle& project,
                                    CProjectFolder& parent_folder,
                                    const string& folder_name,
                                    bool top = true)
{
    if (parent_folder.IsSetFolders()) {
        NON_CONST_ITERATE(CProjectFolder::TFolders, iter, parent_folder.SetFolders()) {
            if( (*iter)->GetInfo().GetTitle() == folder_name) {
                return (*iter); //    iter = folder.SetFolders().erase(iter);
            }
            else
                s_GetOrCreateFolder(project, **iter, folder_name, false);
        }
    }

    if(top) {
        CRef<CProjectFolder> folder(new CProjectFolder);
        folder->SetInfo().SetCreateDate(CTime(CTime::eCurrent));
        folder->SetInfo().SetTitle(folder_name);
        parent_folder.SetFolders().push_back(folder);
        return folder;
    }
    return CRef<CProjectFolder>();
}


void CGBProjectTool::x_LoadGeneralData(CProjectItem& item,
                                     ESerialDataFormat fmt,
                                     CNcbiIstream& istr)
{
    auto_ptr<CObjectIStream> os(CObjectIStream::Open(fmt, istr));
    string obj_type = os->ReadFileHeader();
    os->SetStreamPos(0);

    if (obj_type == "Seq-annot") {
        CRef<CSeq_annot> obj(new CSeq_annot);
        *os >> *obj;
        item.SetObject(*obj);
    } else if (obj_type == "Seq-loc") {
        CRef<CSeq_loc> obj(new CSeq_loc);
        *os >> *obj;
        item.SetObject(*obj);
    } else if (obj_type == "Seq-id") {
        CRef<CSeq_id> obj(new CSeq_id);
        *os >> *obj;
        item.SetObject(*obj);
    } else {
        NCBI_THROW(CException, eUnknown, "The data type is not supported!");
    }
}


void CGBProjectTool::x_LoadGffData(CProjectItem& item,
                                   CNcbiIstream& istr)
{
    // always treat it as a seq-entry
    CGFFReader reader;
    CRef<CSeq_entry> obj = reader.Read(istr);
    item.SetObject(*obj);
}


void CGBProjectTool::x_AddItem(CGBProjectHandle& project,
                               CNcbiIstream& istr,
                               const string& fmt_str,
                               const string& in_folder)
{
    CRef<CProjectItem> item(new CProjectItem);

    if (fmt_str == "gff") {
        x_LoadGffData(*item, istr);
    } else {
        if (fmt_str == "asn") {
            x_LoadGeneralData(*item, eSerial_AsnText, istr);
        } else if (fmt_str == "asnb") {
            x_LoadGeneralData(*item, eSerial_AsnBinary, istr);
        } else {
            NCBI_THROW(CException, eUnknown, "The file format is not currently supported!");
        }
    }

    // add the new item into the project (under a folder if given)
    if (!in_folder.empty()) {
        CProjectFolder& root = project.SetProject().SetData();

        // check if there is a folder named 'in_folder'
        bool folder_found = false;
        if (root.IsSetFolders()) {
            NON_CONST_ITERATE (CProjectFolder::TFolders, iter, root.SetFolders()) {
                if ((*iter)->GetInfo().GetTitle() == in_folder) {
                    project.AddItem(*item, **iter);
                    folder_found = true;
                    break;
                }
            }
        }

        // otherwise add a new folder
        if (!folder_found) {
            CRef<CProjectFolder> folder(new CProjectFolder);
            folder->SetInfo().SetCreateDate(CTime(CTime::eCurrent));
            folder->SetInfo().SetTitle(in_folder);
            root.SetFolders().push_back(folder);
            project.AddItem(*item, *folder);
        }
    } else {
        project.AddItem(*item, project.SetData());
    }
}


int CGBProjectTool::Run()
{
    CArgs args = GetArgs();

	bool EnableID = true;
	if(args["noid"].HasValue() && args["noid"].AsBoolean())
		EnableID = false;


	CRef<CObjectManager> object_manager = CObjectManager::GetInstance();
    CRef<CScope> scope(new CScope(*object_manager));                                                  
	scope->ResetDataAndHistory();  

	if(EnableID) {
   		CGBDataLoader::RegisterInObjectManager(*object_manager, NULL, CObjectManager::eDefault);  
   		scope->AddDataLoader(CGBDataLoader::GetLoaderNameFromArgs());                         
	}

	Writer.reset(new CProjectFileWriter(scope));
	

    ESerialDataFormat saveformat = eSerial_AsnBinary;

    if (args["create"]) {

        //
        // create a new project
        //
        CGBProject_ver2 projObj;
        CGBProjectHandle project(projObj);

        CTime create_time(CTime::eCurrent);

        /// set the project's basic descriptors
        CProjectDescr& descr = project.SetProject().SetDescr();
        descr.SetCreate_date().SetToTime(create_time);
        if (args["title"].HasValue()) {
            descr.SetTitle(args["title"].AsString());
        } else {
            descr.SetTitle("Default");
        }

        /// set the data descriptors
        CProjectFolder& data = project.SetProject().SetData();
        data.SetInfo().SetTitle("Data");
        data.SetInfo().SetCreate_date().SetToTime(create_time);

        /// serialize
        CNcbiOstream& ostr = args["o"].AsOutputFile();
        project.Save(ostr, saveformat);

    } else if (args["add-folder"].HasValue()) {
        ///
        /// add a folder to an existing project
        ///
        CGBProjectHandle project;
        {{
            CNcbiIstream& istr = args["i"].AsInputFile();
            if ( !project.Load(istr,0) ) {
                NCBI_THROW(CException, eUnknown, "failed to read project");
            }
        }}

        string folder_name = args["add-folder"].AsString();

        CRef<CProjectFolder> folder(new CProjectFolder);
        folder->SetInfo().SetCreateDate(CTime(CTime::eCurrent));
        folder->SetInfo().SetTitle(folder_name);

        CProjectFolder& root = project.SetProject().SetData();
        root.SetFolders().push_back(folder);

        /// serialize
        CNcbiOstream& ostr = args["o"].AsOutputFile();
        project.Save(ostr, saveformat);

    } else if (args["list"]) {
        /// lists the folders of an existing project
        CGBProjectHandle project;
        {{
            CNcbiIstream& istr = args["i"].AsInputFile();
            if ( !project.Load(istr,0) ) {
                NCBI_THROW(CException, eUnknown, "failed to read project");
            }
        }}

        const CProjectFolder& folder = project.GetProject().GetData();
        CNcbiOstream& ostr = args["o"].AsOutputFile();
        s_ListProjectFolder(folder, ostr);
    } else if (args["expand"].HasValue()) {
        /// Expands (saves the folders to directories and items to files)
        //   of an existing project
        CGBProjectHandle project;
        {{
            CNcbiIstream& istr = args["i"].AsInputFile();
            if ( !project.Load(istr,0) ) {
                NCBI_THROW(CException, eUnknown, "failed to read project");
            }
        }}

        EFormat format = CFormatGuess::eBinaryASN;//eSerial_AsnBinary;
        string formatstr = args["expand-fmt"].AsString();
        if(formatstr == "asnb")
            format = CFormatGuess::eBinaryASN;
        else if(formatstr == "asnt")
            format = CFormatGuess::eTextASN;
        else if(formatstr == "xml")
            format = CFormatGuess::eXml;
        //else if(formatstr == "json")
        //    format = eSerial_Json;
        else if(formatstr == "gtf")
            format = CFormatGuess::eGtf;

        const CProjectFolder& folder = project.GetProject().GetData();
        CDir parent(args["expand"].AsString());
        s_ScopeProjectFolder(scope, folder);
        s_ExpandProjectFolder(scope, folder, parent, format);
    } else if (args["collapse"].HasValue()) {
        /// Expands (saves the folders to directories and items to files)
        //   of an existing project
        CGBProjectHandle project(*new CGBProject_ver2);
        {{
            CNcbiIstream& istr = args["i"].AsInputFile();
            if ( !project.Load(istr,0) ) {
            //cerr << "Failed to load, making new proj" << endl;
            //    NCBI_THROW(CException, eUnknown, "failed to read project");
            }
        }}


        CDir parent(args["collapse"].AsString());
        CRef<CProjectFolder> folder = CRef<CProjectFolder>(&project.SetProject().SetData());
        if(args["proj-folder"].HasValue()) {
            string ProjFolder = args["proj-folder"].AsString();
            folder = s_GetOrCreateFolder(project, *folder, ProjFolder);
        }

        if(args["i"].AsString() == "-")
            project.SetDescr().SetTitle(parent.GetBase());

        if(args["title"].HasValue())
            project.SetDescr().SetTitle(args["title"].AsString());

		project.SetDescr().SetCreateDate(CTime(CTime::eCurrent));
        project.SetDescr().SetModifiedDate(CTime(CTime::eCurrent));
        s_CollapseProjectFolder(scope, project, *folder, parent);
        CNcbiOstream& ostr = args["o"].AsOutputFile();
        project.Save(ostr, saveformat);
    } else if (args["add"].HasValue()) {
        ///
        /// add an item to the project
        ///
        CGBProjectHandle project;
        {{
            CNcbiIstream& istr = args["i"].AsInputFile();
            if ( !project.Load(istr,0) ) {
                NCBI_THROW(CException, eUnknown, "failed to read project");
            }
        }}

        string folder;
        if (args["proj-folder"].HasValue()) {
            folder = args["proj-folder"].AsString();
        }
        x_AddItem(project, args["add"].AsInputFile(),
                  args["fmt"].AsString(), folder);
        /// serialize
        CNcbiOstream& ostr = args["o"].AsOutputFile();
        project.Save(ostr, saveformat);

    } else if (args["rm"].HasValue()) {

        // remove an item from the project
        CGBProjectHandle project;
        {{
            CNcbiIstream& istr = args["i"].AsInputFile();
            if ( !project.Load(istr,0) ) {
                NCBI_THROW(CException, eUnknown, "failed to read project");
            }
        }}

        CProjectFolder& folder = project.SetProject().SetData();
        string label = args["rm"].AsString();
        s_RemoveItem(project, folder, label);

        /// serialize
        CNcbiOstream& ostr = args["o"].AsOutputFile();
        project.Save(ostr, saveformat);

    } else if (args["rmdir"].HasValue()) {

        // remove an item from the project
        CGBProjectHandle project;
        {{
            CNcbiIstream& istr = args["i"].AsInputFile();
            if ( !project.Load(istr,0) ) {
                NCBI_THROW(CException, eUnknown, "failed to read project");
            }
        }}

        CProjectFolder& folder = project.SetProject().SetData();
        string label = args["rmdir"].AsString();
        s_RemoveFolder(project, folder, label);

        /// serialize
        CNcbiOstream& ostr = args["o"].AsOutputFile();
        project.Save(ostr, saveformat);

    } else {
        NCBI_THROW(CException, eUnknown, "command not yet implemented");
    }

    return 0;
}


END_NCBI_SCOPE
USING_SCOPE(ncbi);


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    return CGBProjectTool().AppMain(argc, argv);
}
