/*  $Id: lblast_object_loader.cpp 35928 2016-07-13 15:09:48Z katargir $
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
* Authors:  Roman Katargin
*/


#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/lblast_object_loader.hpp>
#include <objtools/blast/seqdb_reader/seqdb.hpp>
#include <objtools/data_loaders/blastdb/bdbloader.hpp>

#include <objects/general/User_object.hpp>

#include <objmgr/object_manager.hpp>

#include <gui/objutils/label.hpp>

#include <util/icanceled.hpp>
#include <util/utf8.hpp>

#include <wx/filename.h> 

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CLBLASTObjectLoader::CLBLASTObjectLoader(const CLBLASTLoadParams& params)
    : m_Params(params)
{
}

IObjectLoader::TObjects& CLBLASTObjectLoader::GetObjects()
{
    return m_Objects;
}

string CLBLASTObjectLoader::GetDescription() const
{
    return "Loading Local BLAST DB";
}

CObject* CLBLASTObjectLoader::GetLoader()
{
    return m_Loader.GetPointerOrNull();
}

bool CLBLASTObjectLoader::PreExecute()
{
    return true;
}

static const char* kLoaderDataType = "Local BLAST";
static const char* kBLASTDir = "BLASTDir";
static const char* kProteins = "Proteins";
static const int   kLoaderPriority = 90;
static const char* kUILabel = "UILabel";

CRef<CLoaderDescriptor> CLBLASTObjectLoader::CreateLoader(const string& db, bool proteins)
{
    CRef<CLoaderDescriptor> loader(new CLoaderDescriptor());

    loader.Reset(new CLoaderDescriptor());

    string label;
    try {
        CSeqDB::ESeqType seqType = proteins ? CSeqDB::eProtein : CSeqDB::eNucleotide;
        CRef<CSeqDB> seqdb(new CSeqDB(x_GetQuotedDBName(db), seqType));
        label = seqdb->GetTitle();
    }
    catch (const exception&) {
    }

    if (label.empty())
        label = wxFileName(db).GetFullName().ToUTF8();

    label = "DB: " + label;
    loader->SetLabel(utf8::StringToAscii(label));

    loader->SetPriority(kLoaderPriority);
    loader->SetLoader_type("local_blast_data_source_type");

    CRef<CLoaderDescriptor::TData> data(new CLoaderDescriptor::TData);
    data->SetType().SetStr(kLoaderDataType);
    data->AddField(kBLASTDir, db);
    loader->SetData(*data);

    data->AddField(kProteins, proteins);
    data->AddField(kUILabel, label);

    return loader;
}

string CLBLASTObjectLoader::GetLoaderName(const objects::CUser_object& obj)
{
    string loader_name;

    if (obj.GetType().IsStr()) {
        const string& loader_type = obj.GetType().GetStr();
        if (loader_type != kLoaderDataType)
            return loader_name;

        CConstRef<CUser_field> fieldDir = obj.GetFieldRef(kBLASTDir);
        CConstRef<CUser_field> fieldProteins = obj.GetFieldRef(kProteins);
        if (fieldDir && fieldProteins) {
            string db = fieldDir->GetData().GetStr();
            bool proteins = fieldProteins->GetData().GetBool();

            CSeqDB::ESeqType seqType = proteins ? CSeqDB::eProtein : CSeqDB::eNucleotide;
            try {
                CRef<CSeqDB> seqdb(new CSeqDB(x_GetQuotedDBName(db), seqType));
                loader_name = CBlastDbDataLoader::GetLoaderNameFromArgs(CConstRef<CSeqDB>(seqdb));
            }
            catch (const exception&) {
            }
        }
    }

    return loader_name;
}

string CLBLASTObjectLoader::AddDataLoader(const objects::CUser_object& obj)
{
    string loader_name;

    if (obj.GetType().IsStr()) {
        const string& loader_type = obj.GetType().GetStr();
        if (loader_type != kLoaderDataType)
            return loader_name;

        CConstRef<CUser_field> fieldDir = obj.GetFieldRef(kBLASTDir);
        CConstRef<CUser_field> fieldProteins = obj.GetFieldRef(kProteins);
        if (fieldDir && fieldProteins) {
            string db = fieldDir->GetData().GetStr();
            bool proteins = fieldProteins->GetData().GetBool();

            CBlastDbDataLoader::EDbType seqType = proteins ? CBlastDbDataLoader::eProtein : CBlastDbDataLoader::eNucleotide;

            CRef<objects::CObjectManager> obj_mgr = CObjectManager::GetInstance();
            CBlastDbDataLoader::TRegisterLoaderInfo info =
                CBlastDbDataLoader::RegisterInObjectManager(
                *obj_mgr, x_GetQuotedDBName(db), seqType, true,
                CObjectManager::eNonDefault,
                kLoaderPriority);
            loader_name = info.GetLoader()->GetName();
        }
    }

    return loader_name;
}

bool CLBLASTObjectLoader::Execute(ICanceled& canceled)
{
    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    CRef<CScope> scope(new CScope(*obj_mgr));
    scope->AddDefaults();

    bool proteins = m_Params.GetProtSeqs();
    string db = proteins ? string(m_Params.GetProtDB().ToUTF8()) : string(m_Params.GetNucDB().ToUTF8());
    CSeqDB::ESeqType seqType = proteins ? CSeqDB::eProtein : CSeqDB::eNucleotide;

    CRef<CSeqDB> seqdb(new CSeqDB(x_GetQuotedDBName(db), seqType));
    m_Loader = CreateLoader(db, proteins);

    if (m_Params.GetCreateSeqs()) {
        for (int oid = 0; seqdb->CheckOrFindOID(oid); oid++) {
            list< CRef<CSeq_id> > ids = seqdb->GetSeqIDs(oid);
            NON_CONST_ITERATE(list< CRef<CSeq_id> >, it, ids) {
                if (canceled.IsCanceled())
                    return false;

                if (!(*it)->IsGi()) {
                    string label;
                    CLabel::GetLabel(**it, &label, CLabel::eDefault, scope);
                    SObject object(**it, label);
                    m_Objects.push_back(object);
                    break;
                }
            }
        }
    }

    return true;
}

int CLBLASTObjectLoader::CountSeqs(const string& db, bool proteins)
{
    int count = 0;
    try {
        CSeqDB::ESeqType seqType = proteins ? CSeqDB::eProtein : CSeqDB::eNucleotide;
        CRef<CSeqDB> seqdb(new CSeqDB(x_GetQuotedDBName(db), seqType));
        count = seqdb->GetNumSeqs();
    }
    catch (const exception&) {
    }
    return count;
}

string CLBLASTObjectLoader::x_GetQuotedDBName(const string db)
{
    return "\"" + db + "\"";
}

bool CLBLASTObjectLoader::PostExecute()
{
    return true;
}

END_NCBI_SCOPE
