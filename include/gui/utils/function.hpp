#ifndef GUI_GUI___FUNCTION__HPP
#define GUI_GUI___FUNCTION__HPP

/*  $Id: function.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *    Function classes
 */


#include <corelib/ncbiobj.hpp>


/** @addtogroup GUI_UTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE


class CFuncPtr : public CObject
{
public:
    void operator()()
    {
        Execute();
    }

    virtual void Execute(void) = 0;
};


template <typename T>
class CFuncPtrTmpl : public CFuncPtr
{
public:
    typedef void (T::*TMemFun)(void);

    CFuncPtrTmpl(T* objptr, TMemFun memfun)
        : m_ObjPtr(objptr), m_MemFun(memfun)
    {
    }

    void Execute(void)
    {
        (m_ObjPtr->*m_MemFun)();
    }

private:
    T* m_ObjPtr;
    TMemFun m_MemFun;
};



template <typename RetType, typename Arg1>
class CFuncPtr1 : public CObject
{
public:
    RetType operator()(Arg1 arg1)
    {
        return Execute(arg1);
    }

    virtual RetType Execute(Arg1 arg1) = 0;
};


template <typename T, typename RetType, typename Arg1>
class CFuncPtrTmpl1 : public CFuncPtr1<RetType, Arg1>
{
public:
    typedef RetType (T::*TMemFun)(Arg1);

    CFuncPtrTmpl1(T* objptr, TMemFun memfun)
        : m_ObjPtr(objptr), m_MemFun(memfun)
    {
    }


    RetType Execute(Arg1 arg1)
    {
        return (m_ObjPtr->*m_MemFun)(arg1);
    }

private:
    T* m_ObjPtr;
    TMemFun m_MemFun;
};

/*****************************************************************************/

template <typename Arg1>
class CVoidFuncPtr1 : public CObject
{
public:
    void operator()(Arg1 arg1)
    {
        Execute(arg1);
    }

    virtual void Execute(Arg1 arg1) = 0;
};


template <typename T, typename Arg1>
class CVoidFuncPtrTmpl1 : public CVoidFuncPtr1<Arg1>
{
public:
    typedef void (T::*TMemFun)(Arg1);

    CVoidFuncPtrTmpl1(T* objptr, TMemFun memfun)
        : m_ObjPtr(objptr), m_MemFun(memfun)
    {
    }


    void Execute(Arg1 arg1)
    {
        (m_ObjPtr->*m_MemFun)(arg1);
    }

private:
    T* m_ObjPtr;
    TMemFun m_MemFun;
};

END_NCBI_SCOPE

/* @} */

#endif // GUI_GUI___FUNCTION__HPP
