#ifndef PTRSTR_HPP
#define PTRSTR_HPP

/*  $Id: ptrstr.hpp 546704 2017-09-20 18:15:56Z gouriano $
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
* Author: Eugene Vasilchenko
*
* File Description:
*   C++ class info: includes, used classes, C++ code etc.
*
*/

#include "typestr.hpp"
#include <corelib/ncbiutil.hpp>

BEGIN_NCBI_SCOPE

class CPointerTypeStrings : public CTypeStrings
{
    typedef CTypeStrings CParent;
public:
    CPointerTypeStrings(CTypeStrings* type);
    CPointerTypeStrings(AutoPtr<CTypeStrings> type);
    ~CPointerTypeStrings(void);

    const CTypeStrings* GetDataTypeStr(void) const
        {
            return m_DataTypeStr.get();
        }

    virtual void GenerateTypeCode(CClassContext& ctx) const override;

    virtual EKind GetKind(void) const override;

    virtual string GetCType(const CNamespace& ns) const override;
    virtual string GetPrefixedCType(const CNamespace& ns,
                            const string& methodPrefix) const override;
    virtual string GetRef(const CNamespace& ns) const override;

    virtual string GetInitializer(void) const override;
    virtual string GetDestructionCode(const string& expr) const override;
    virtual string GetIsSetCode(const string& var) const override;
    virtual string GetResetCode(const string& var) const override;

private:
    AutoPtr<CTypeStrings> m_DataTypeStr;
};

class CRefTypeStrings : public CPointerTypeStrings
{
    typedef CPointerTypeStrings CParent;
public:
    CRefTypeStrings(CTypeStrings* type);
    CRefTypeStrings(AutoPtr<CTypeStrings> type);
    ~CRefTypeStrings(void);

    virtual EKind GetKind(void) const override;

    virtual string GetCType(const CNamespace& ns) const override;
    virtual string GetPrefixedCType(const CNamespace& ns,
                            const string& methodPrefix) const override;
    virtual string GetRef(const CNamespace& ns) const override;

    virtual string GetInitializer(void) const override;
    virtual string GetDestructionCode(const string& expr) const override;
    virtual string GetIsSetCode(const string& var) const override;
    virtual string GetResetCode(const string& var) const override;
};

END_NCBI_SCOPE

#endif
