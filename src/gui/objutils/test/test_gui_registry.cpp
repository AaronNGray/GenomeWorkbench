/*  $Id: test_gui_registry.cpp 39670 2017-10-25 18:01:21Z katargir $
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

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbiargs.hpp>
#include <gui/objutils/registry.hpp>
#include <iterator>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


class CTestGuiRegistryApp : public CNcbiApplication
{
private:
    void Init();
    int Run();
};


void CTestGuiRegistryApp::Init()
{
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions());

    arg_desc->AddKey("local", "LocalFile",
                     "Local registry",
                     CArgDescriptions::eInputFile);

    arg_desc->AddKey("site", "SiteFile",
                     "Site-specific registry",
                     CArgDescriptions::eInputFile);

    arg_desc->AddKey("global", "GlobalFile",
                     "Global registry",
                     CArgDescriptions::eInputFile);

    SetupArgDescriptions(arg_desc.release());
}


static void s_DumpKeyInfo(const CGuiRegistry& reg,
                          const CGuiRegistry::TKeys& keys,
                          CNcbiOstream& ostr)
{
    ostr << keys.size() << " keys:" << endl;
    ITERATE (CGuiRegistry::TKeys, iter, keys) {
        const CGuiRegistry::SKeyInfo& key = *iter;

        ostr << key.key << "|" ;

        switch (key.access) {
        case CGuiRegistry::eAccess_ReadOnly:
            ostr << "read-only|";
            break;
        case CGuiRegistry::eAccess_ReadWrite:
            ostr << "read-write|";
            break;
        default:
            ostr << "unknown|";
            break;
        }

        vector<string> strs;
        vector<int> ints;
        vector<double> reals;
        switch (key.type) {
        case CUser_field::TData::e_Bool:
            ostr << "bool|" << NStr::BoolToString(reg.GetBool(key.key));
            break;
        case CUser_field::TData::e_Int:
            ostr << "int|" << reg.GetInt(key.key);
            break;
        case CUser_field::TData::e_Ints:
            ostr << "ints|";
            reg.GetIntVec(key.key, ints);
            copy(ints.begin(), ints.end(), ostream_iterator<int>(cout, ";"));
            break;
        case CUser_field::TData::e_Real:
            ostr << "real|" << reg.GetReal(key.key);
            break;
        case CUser_field::TData::e_Reals:
            ostr << "reals|";
            reg.GetRealVec(key.key, reals);
            copy(reals.begin(), reals.end(), ostream_iterator<double>(cout, ";"));
            break;
        case CUser_field::TData::e_Str:
            ostr << "string|" << reg.GetString(key.key);;
            break;
        case CUser_field::TData::e_Strs:
            ostr << "strings|";
            reg.GetStringVec(key.key, strs);
            copy(strs.begin(), strs.end(), ostream_iterator<string>(cout, ";"));
            break;
        case CUser_field::TData::e_Fields:
            ostr << "subkey|";
            break;
        default:
            ostr << "unknown|";
            break;
        }
        ostr << endl;

    }
}



int CTestGuiRegistryApp::Run()
{
    CArgs args = GetArgs();
    CNcbiIstream& local  = args["local" ].AsInputFile();
    CNcbiIstream& site   = args["site"  ].AsInputFile();
    CNcbiIstream& global = args["global"].AsInputFile();

    CGuiRegistry::TKeys keys;

#if 1

    CGuiRegistry reg;
    reg.SetLocal(local);
    reg.SetGlobal(global);
    reg.AddSite(site, 1);

/*
    reg.GetKeys(keys);
    s_DumpKeyInfo(reg, keys, cout);
    cout << "-- Local site" << endl;
    reg.Write(cout);
    cout << "-- Global site" << endl;
    reg.Write(cout, CGuiRegistry::ePriority_Global);
*/


    int val1 = reg.GetInt("section.key");
    cout << "section.key = " << val1 << endl;

    string string_val("string_val");
    reg.Set("section.key", string_val);
    string val2 = reg.GetString("section.key");
    cout << "section.key = " << val2 << endl;

    string local_str = reg.GetString("section", "local");
    cout << "section.local = " << local_str << endl;
    string global_str = reg.GetString("section.global");
    cout << "section.global = " << global_str << endl;

    /// test views
    {{
        CRegistryReadView view = reg.GetReadView("section");
        string str_val = view.GetString("key");
        cout << "view(section): key = " << str_val << endl;

        str_val = view.GetString("global");
        cout << "view(section): global = " << str_val << endl;

        str_val = view.GetString("local");
        cout << "view(section): local = " << str_val << endl;
    }}

    /// test views
    {{
        CGuiRegistry::TReadWriteView view = reg.GetReadWriteView("section");
        view.Set("view_key", "This was set from the view");
        view.Set("sub1.sub2.sub3.view_key",
                 "This was set from the view and is a new subkey");

        string str_val = view.GetString("view_key");
        cout << "view(section): view_key = " << str_val << endl;

        view.Set("sub1.tobedel", "set from view, will be deleted");
        str_val = view.GetString("sub1.tobedel", "DEFAULT AFTER DeleteField");
        cout << "view(section): sub1.tobedel = " << str_val << endl;
        view.DeleteField("sub1.tobedel");
        str_val = view.GetString("sub1.tobedel", "DEFAULT AFTER DeleteField");
        cout << "view(section): sub1.tobedel = " << str_val << endl;


    }}

    /// test view overlays
    {{
         vector<string> sections;
         CRegistryReadView rv;

         sections.push_back("default");
         rv = reg.GetReadView("section.data", sections);
         cout << "default:" << endl;
         cout << "  name: " << rv.GetString("name") << endl;
         cout << "  key1: " << rv.GetInt("key1") << endl;
         cout << "  key2: " << rv.GetInt("key2") << endl;
         cout << "  key3: " << rv.GetInt("key3") << endl;
         cout << "  key4: " << rv.GetInt("key4") << endl;

         sections.push_back("overloaded");
         rv = reg.GetReadView("section.data", sections);
         cout << "default, overloaded:" << endl;
         cout << "  name: " << rv.GetString("name") << endl;
         cout << "  key1: " << rv.GetInt("key1") << endl;
         cout << "  key2: " << rv.GetInt("key2") << endl;
         cout << "  key3: " << rv.GetInt("key3") << endl;
         cout << "  key4: " << rv.GetInt("key4") << endl;

         sections.push_back("minimal");
         rv = reg.GetReadView("section.data", sections);
         cout << "default, overloaded, minimal:" << endl;
         cout << "  name: " << rv.GetString("name") << endl;
         cout << "  key1: " << rv.GetInt("key1") << endl;
         cout << "  key2: " << rv.GetInt("key2") << endl;
         cout << "  key3: " << rv.GetInt("key3") << endl;
         cout << "  key4: " << rv.GetInt("key4") << endl;
     }}


    reg.Write(cout);

    reg.GetKeys(keys);
    s_DumpKeyInfo(reg, keys, cout);

    reg.GetKeys(keys, "section");
    s_DumpKeyInfo(reg, keys, cout);

#if 0
    {{
        CRegistryReadView view = reg.GetReadView("section");
        view.GetTopKeys(keys);
        cout << "** Dump View's top keys" << endl;
        s_DumpKeyInfo(view, keys, cout);
        view.GetKeys(keys);
        cout << "** Dump All View's keys" << endl;
        s_DumpKeyInfo(view, keys, cout);
    }}
#endif

    reg.GetKeys(keys, "section.sub1");
    s_DumpKeyInfo(reg, keys, cout);

    if (reg.DeleteField("section","sub1")) {
        reg.GetKeys(keys);
        cout << "** deleted section.sub1" << endl;
        s_DumpKeyInfo(reg, keys, cout);
    } else {
        cout << "** Did not delete section.sub1" << endl;
    }

    if (reg.DeleteField("section","data")) {
        reg.GetKeys(keys);
        cout << "** deleted section.data" << endl;
        s_DumpKeyInfo(reg, keys, cout);
    } else {
        cout << "** Did not delete section.data" << endl;
    }

    if (reg.DeleteField("section","key")) {
        reg.GetKeys(keys);
        cout << "** deleted section.key" << endl;
        s_DumpKeyInfo(reg, keys, cout);
    } else {
        cout << "** Did not delete section.key" << endl;
    }

#endif

// testing for errors when keys conflict.
#if 1
// following code will assert false when run in debug mode.
// will throw exceptions in release mode.
    {
        CGuiRegistry testreg;
        testreg.Set("key1", 5);
        testreg.Set("s1.key2", 6);
        testreg.GetKeys(keys);
        s_DumpKeyInfo(testreg, keys, cout);
        try {
            testreg.Set("key1.badsub1", 200);
            cout << "Too many keys (key1.badsub1) NOT caught" << endl;
        } catch (CException& e) {
            cout << "Exception caught: " << e.GetMsg() << endl;
        }
        try {
            testreg.Set("s1.key2.badsub2", 300);
            cout << "Too many keys (s1.key2.badsub2) NOT caught" << endl;
        } catch (CException& e) {
            cout << "Exception caught: " << e.GetMsg() << endl;
        }
        try {
            CGuiRegistry::TReadWriteView v = testreg.GetReadWriteView("key1");
            v.Set("badsub1", 201);
            cout << "Set view: Too many keys (badsub1) NOT caught" << endl;
        } catch (CException& e) {
            cout << "W/View Exception caught: " << e.GetMsg() << endl;
        }
        try {
            CGuiRegistry::TReadWriteView v = testreg.GetReadWriteView("s1");
            v.Set("key2.badsub2", 301);
            cout << "Set view: Too many keys (key2.badsub2) NOT caught" << endl;
        } catch (CException& e) {
            cout << "W/View Exception caught: " << e.GetMsg() << endl;
        }
    }

    {
        CGuiRegistry testreg;
        testreg.Set("s1.k1", 1.2);
        testreg.Set("s2.k1.k2", 3.4);
        try {
           testreg.Set("s1", 212.3);
            cout << "Too few keys (s1) NOT caught" << endl;
        } catch (CException& e) {
            cout << "Exception caught: " << e.GetMsg() << endl;
        }
        try {
           testreg.Set("s2.k1", 314.1);
            cout << "Too few keys (s2.k1) NOT caught" << endl;
        } catch (CException& e) {
            cout << "Exception caught: " << e.GetMsg() << endl;
        }
        try {
            CGuiRegistry::TReadWriteView v = testreg.GetReadWriteView("s2");
            v.Set("k1", 201);
            cout << "Set view: Too few keys (s2.k1) NOT caught" << endl;
        } catch (CException& e) {
            cout << "W/View Exception caught: " << e.GetMsg() << endl;
        }
    }

    {
        CGuiRegistry merge_reg;
        merge_reg.Set("key1", 5);
        merge_reg.PushWritableTempSite();
        merge_reg.Set("key1.badsub1", 555);
        try {
            merge_reg.MergeTempSite();
            cout << "Merge w/too many keys (key1.badsub1) NOT caught" << endl;
        } catch (CException& e) {
            cout << "During Merge Exception caught: " << e.GetMsg() << endl;
        }
    }

    {
        CGuiRegistry merge_reg;
        merge_reg.Set("s1.key2", 7);
        merge_reg.PushWritableTempSite();
        merge_reg.Set("s1.key2.badsub2", 777);
        try {
            merge_reg.MergeTempSite();
            cout << "Merge w/too many keys (s1.key2.badsub2) NOT caught" << endl;
        } catch (CException& e) {
            cout << "During Merge Exception caught: " << e.GetMsg() << endl;
        }
    }

    {
        CGuiRegistry merge_reg;
        merge_reg.Set("s1.k1", 5);
        merge_reg.PushWritableTempSite();
        merge_reg.Set("s1", 555);
        try {
            merge_reg.MergeTempSite();
            cout << "Merge w/ too few keys (s1) NOT caught" << endl;
        } catch (CException& e) {
            cout << "During Merge Exception caught: " << e.GetMsg() << endl;
        }
    }

    {
        CGuiRegistry merge_reg;
        merge_reg.Set("s1.k1.k2", 5);
        merge_reg.PushWritableTempSite();
        merge_reg.Set("s1.k1", 555);
        try {
            merge_reg.MergeTempSite();
            cout << "Merge w/ too few keys (s1) NOT caught" << endl;
        } catch (CException& e) {
            cout << "During Merge Exception caught: " << e.GetMsg() << endl;
        }
    }
#endif


    // adding and deleting sites to the registry
    CGuiRegistry reg1;
    reg1.Set("top", 5);
    reg1.Set("top1.sub", 6);
    reg1.Set("local.level", 1);
    reg1.Set("tree.sub.leaf1", 3);
    cout << "New reg with local." << endl;
    reg1.Write(cout);
    reg1.GetKeys(keys);
    s_DumpKeyInfo(reg1, keys, cout);

    reg1.PushWritableTempSite();
    reg1.Set("top1.sub1", 500);
    reg1.Set("top2", 600);
    reg1.Set("local.lsub", 512);
    reg1.Set("tree.sub.leaf2", 256); // over ride local up tree.sub.
    cout << "New reg with write temp." << endl;
    reg1.GetKeys(keys);
    s_DumpKeyInfo(reg1, keys, cout);
    reg1.Write(cout);
    reg1.Write(cout, -4);


    CNcbiIfstream temp_stream("temp.asn");
    reg1.PushTempSite(temp_stream, false);
    reg1.GetKeys(keys);
    cout << "Pushed temp.asn readonly." << endl;
    s_DumpKeyInfo(reg, keys, cout);

    reg1.PopTempSite();
    reg1.GetKeys(keys);
    cout << "Popped temp read only site." << endl;
    s_DumpKeyInfo(reg, keys, cout);

    reg1.MergeTempSite();
    cout << "New reg after merge." << endl;
    //reg1.Write(cout);
    reg1.GetKeys(keys);
    s_DumpKeyInfo(reg1, keys, cout);

    CGuiRegistry vreg;
    vreg.Set("top.s1", 1);
    vreg.PushWritableTempSite();
    vreg.Set("top.s1", 2);
    CGuiRegistry::TReadWriteView rwv = vreg.GetReadWriteView("top");
    rwv.DumpAll(cout);
    rwv.Set("s1", 5);
    rwv.DumpAll(cout);

#if 0
    CGuiRegistry treg;
    treg.Set("section", "i", 1);
    treg.Set("section", "s", "stuff");
    treg.Set("section", "a.d", 2.3);
    treg.Set("section", "a.s", "overwritten");
    treg.Set("section1", "a.s", "junk");
    treg.PushWritableTempSite();
    treg.Set("section1", "a.b", false);
    treg.Set("section", "a.sk.i", 2);
    treg.Set("section1", "a.sk.d", 3.1416);
    treg.Set("topi", "", 4);
    treg.Set("tops", "", "junk");
    treg.GetKeys(keys);
    cout << "** all keys in registry" << endl;
    s_DumpKeyInfo(treg, keys, cout);
    vector<string> sects;
    sects.push_back("section");
    sects.push_back("section1");
    sects.push_back("topi");
    CRegistryReadView v = treg.GetReadView("", sects);
    v.GetTopKeys(keys);
    cout << "** Dump View (section, section1)'s top keys" << endl;
    s_DumpKeyInfo(v, keys, cout);
    v.GetKeys(keys);
    cout << "** Dump View (section, section1)'s Full keys" << endl;
    s_DumpKeyInfo(v, keys, cout);
#endif

    return 0;
}


END_NCBI_SCOPE
USING_SCOPE(ncbi);


int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    return CTestGuiRegistryApp().AppMain(argc, argv);
}
