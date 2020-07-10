#ifndef PKG_ALIGNMENT___NET_BLAST_JOB_DESCR__HPP
#define PKG_ALIGNMENT___NET_BLAST_JOB_DESCR__HPP

/*  $Id: net_blast_job_descr.hpp 32023 2014-12-12 18:53:05Z katargir $
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
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/objutils/objects.hpp>

#include <algo/blast/api/remote_blast.hpp>


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */


BEGIN_SCOPE(objects)
    class CSeq_align_set;
    class CBioseq_set;
END_SCOPE(objects)


class CBLASTParams;
class CNetBLASTUIDataSource;

///////////////////////////////////////////////////////////////////////////////
/// CNetBlastJobDescriptor
class CNetBlastJobDescriptor :
    public CObject
{
    friend class CNetBLASTUIDataSource;
public:
    enum    EState  {
        eInvalidState = -1,
        eInitial,
        eSubmitted,
        eCompleted,
        eFailed,
        eRetrieved,
        eExpired
    };

public:
    static string  GetStateLabel(EState state);

    virtual ~CNetBlastJobDescriptor();

    EState  GetState() const;
    string  GetRID() const;
    string  GetJobTitle() const;
    string  GetDescription() const;
    string  GetErrors() const;
    time_t  GetSubmitTime() const;

    void    Init(const CBLASTParams& params, CNetBLASTUIDataSource& ds);
    void    Init(const string& rid, CNetBLASTUIDataSource& ds);

    void    Submit(objects::CBioseq_set& seq_set);
    void    Check();
    CRef<objects::CSeq_align_set>    Retrieve();
    CRef<blast::CRemoteBlast>   GetRemoteBlast();

    /// release CRemoteBlast object to free associated resources
    void    Release();

    void    MarkDeleted(bool deleted);
    bool    IsDeleted() const;

    objects::CUser_object*  ToUserObject() const;
    void    FromUserObject(const objects::CUser_object& user_obj);

protected:
    CNetBlastJobDescriptor();
    CNetBlastJobDescriptor(CNetBLASTUIDataSource& ds);
    CNetBlastJobDescriptor(const CNetBlastJobDescriptor&);
    CNetBlastJobDescriptor& operator=(const CNetBlastJobDescriptor&);

    void    x_CreateRemoteBlastIfNeeded();
    void    x_NotifyDataSource();

protected:
    mutable CMutex m_Mutex;

    CNetBLASTUIDataSource*  m_NetBlastDS;

    EState  m_State;

    /// Job attributes
    string  m_RID;
    string  m_JobTitle;
    string  m_Description;
    string  m_Errors;
    time_t  m_SubmitTime;

    bool    m_Deleted;

    /// a proxy to Net BLAST that is used for updating Job state
    CRef<blast::CRemoteBlast>   m_RemoteBlast;
};


/* @} */

END_NCBI_SCOPE;


#endif  // PKG_ALIGNMENT___NET_BLAST_JOB_DESCR__HPP

