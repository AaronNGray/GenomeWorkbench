#ifndef OBJECTS_TRACKMGR_OBJECTS_TRACKMGR_TRACKMGR_CLIENT_HPP
#define OBJECTS_TRACKMGR_OBJECTS_TRACKMGR_TRACKMGR_CLIENT_HPP

/* $Id: trackmgr_client.hpp 542016 2017-07-25 12:17:58Z clausen $
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
 */

#include <objects/trackmgr/trackmgr_client_.hpp>


BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE


class NCBI_TRACKMGR_EXPORT CTrackMgrClient : public CTrackMgrClient_Base
{
    typedef CTrackMgrClient_Base Tparent;


public:
    CTrackMgrClient(const string& service = "TrackMgr");
    CTrackMgrClient(const string& host, unsigned int port);
    virtual ~CTrackMgrClient();
    virtual CRef<CTMgr_DisplayTrackReply> AskDefault_display_tracks(
        const CTMgr_DisplayTrackRequest& req,
        TReply* reply = 0
    );

    using TBatchId = Int8;
    using TAlignId = Int8;

    struct SAlignIds
    {
        TBatchId batch_id;
        vector<TAlignId> align_ids;
    };

    using TAlignIDs = vector<SAlignIds>;
    
    // returns false if unable to parse external_id, else true
    static bool ParseAlignId (const string& external_id, 
                              TAlignIDs& parsed_ids);


    // looks at application ini file with a section constructed like
    //[TrackMgr]
    //type=service
    //name=TrackMgr_QA
    static CRef<CTMgr_DisplayTrackReply> s_Ask(
        const CTMgr_DisplayTrackRequest&);

protected:
    virtual void x_Connect(void);
    void x_Init(void);


private:
    // Prohibit copy constructor and assignment operator
    CTrackMgrClient(const CTrackMgrClient& value);
    CTrackMgrClient& operator=(const CTrackMgrClient& value);


private:
    enum
    {
        eNamed_service,
        eHost_port,
    } m_HostType;
    string m_Host;
    unsigned int m_Port;
};


END_objects_SCOPE
END_NCBI_SCOPE

#endif // OBJECTS_TRACKMGR_OBJECTS_TRACKMGR_TRACKMGR_CLIENT_HPP

