#ifndef GUI_OBJUTILS___BLAST_DATABASES__HPP
#define GUI_OBJUTILS___BLAST_DATABASES__HPP

/*  $Id: blast_databases.hpp 35494 2016-05-13 14:16:41Z evgeniev $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <unordered_map>

#include <gui/utils/event_handler.hpp>

#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT  CBLASTDatabases : public CObjectEx, public CEventHandler
{
public:
    typedef std::unordered_map<string, string> TDbMap;

    enum EState
    {
        eInitial,
        eLoaded,
        eLoading,
        eFailed
    };

    static CBLASTDatabases& GetInstance();

    EState GetState() const { return m_State; }
    bool Loaded() const { return (m_State == eLoaded); }

    void Load(const char* path, const char* dbtree);

    const TDbMap& GetDbMap(bool nuc);
    const vector<string>& GetDefaultMRU_DBs(bool nuc_db);
    
private:
    CBLASTDatabases();

    void x_OnJobNotification(CEvent* evt);

    void x_UpdateDbMap(bool nuc);

    TDbMap m_NucDbMap;
    TDbMap m_ProtDbMap;

    vector<string> m_Nuc_DefMRU_DBs;
    vector<string> m_Prot_DefMRU_DBs;

    DECLARE_EVENT_MAP();

    int    m_JobId;
    string m_Path;
    EState m_State;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___BLAST_DATABASES__HPP
