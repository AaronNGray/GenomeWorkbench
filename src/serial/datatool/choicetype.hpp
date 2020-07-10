#ifndef CHOICETYPE_HPP
#define CHOICETYPE_HPP

/*  $Id: choicetype.hpp 546704 2017-09-20 18:15:56Z gouriano $
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
*   Type description of CHOICE type
*
*/

#include "blocktype.hpp"

BEGIN_NCBI_SCOPE

class CChoiceDataType : public CDataMemberContainerType
{
    typedef CDataMemberContainerType CParent;
public:
    virtual void PrintASN(CNcbiOstream& out, int indent) const override;
    void PrintJSONSchema(CNcbiOstream& out, int indent, list<string>& required, bool contents_only=false) const override;

    virtual void FixTypeTree(void) const override;
    virtual bool CheckValue(const CDataValue& value) const override;

    virtual const char* XmlMemberSeparator(void) const override;

    virtual CTypeInfo* CreateTypeInfo(void) override;
    virtual AutoPtr<CTypeStrings> GenerateCode(void) const override;
    virtual AutoPtr<CTypeStrings> GetRefCType(void) const override;
    virtual AutoPtr<CTypeStrings> GetFullCType(void) const override;
    virtual const char* GetASNKeyword(void) const override;
    virtual string      GetSpecKeyword(void) const override;
    virtual const char* GetDEFKeyword(void) const override;
};

END_NCBI_SCOPE

#endif
