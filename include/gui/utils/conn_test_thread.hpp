#ifndef GUI_UTILS___CONN_TEST_THREAD__HPP
#define GUI_UTILS___CONN_TEST_THREAD__HPP

/*  $Id: conn_test_thread.hpp 26184 2012-07-30 18:21:23Z katargir $
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

#include <corelib/ncbistd.hpp>
#include <corelib/ncbithr.hpp>

#include <util/icanceled.hpp>

#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE

class CConnTest;

class NCBI_GUIUTILS_EXPORT CConnTestThread : public CThread, public ICanceled
{
public:
    CConnTestThread(unsigned int timeout);

    bool Finished() const { return (m_Finished.Get() != 0); }

    void RequestCancel() { m_Canceled.Set(1); }
    virtual bool IsCanceled(void) const { return (m_Canceled.Get() != 0); }

// This function must be called only after Finished() returned true
    bool GoodConnection() const { return m_GoodConnection; }
    string GetConnInfo() const { return m_ConnInfo; }

protected:
    /// Do job delegated processing to the main class
    virtual void* Main(void);

    unsigned int m_Timeout;
    bool m_GoodConnection;
    string m_ConnInfo;
    CAtomicCounter_WithAutoInit m_Canceled;
    CAtomicCounter_WithAutoInit m_Finished;
};

END_NCBI_SCOPE

#endif  // GUI_UTILS___CONN_TEST_THREAD__HPP
