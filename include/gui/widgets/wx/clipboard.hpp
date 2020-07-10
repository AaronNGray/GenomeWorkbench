#ifndef GUI_WIDGETS_WX___CLIPBOARD__HPP
#define GUI_WIDGETS_WX___CLIPBOARD__HPP

/*  $Id: clipboard.hpp 43777 2019-08-29 19:15:07Z katargir $
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
 *      Clipaboard and D&D helpers.
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbi_process.hpp>


BEGIN_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////
/// TDataExchange - a simple class for use with wxDataObject.
/// The class holds a Process ID and arbitrary data that can be copied across
/// process boundaries. IsLocal() can be used to distinguish local data from
/// the data beloning to another process.
/// Objects created on heap are generally cannot be safely passed across
/// process boundaries.

template <class T> class TDataExchange
{
public:
    void Init(T& t)
    {
        m_Pid = CCurrentProcess::GetPid();
        m_Data = t;
    }
    bool IsLocal() const
    {
        TPid pid = CCurrentProcess::GetPid();
        return pid == m_Pid;
    }
    T& GetData()
    {
        return m_Data;
    }
protected:
    TPid m_Pid;
    T   m_Data;
};

END_NCBI_SCOPE

#endif //GUI_WIDGETS_WX___CLIPBOARD__HPP
