#ifndef GUI_UTILS___OBJECT_LOADER__HPP
#define GUI_UTILS___OBJECT_LOADER__HPP

/*  $Id: object_loader.hpp 38443 2017-05-10 15:49:30Z katargir $
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
#include <corelib/ncbiobj.hpp>

BEGIN_NCBI_SCOPE

class IObjectLoader
{
public:
    virtual ~IObjectLoader() {}

    struct SObject
    {
        SObject() {}
        SObject(CObject& obj, const string& desc)
            : object(&obj), description(desc) {}
        SObject(CObject& obj, const string& desc, const string& comm)
            : object(&obj), description(desc), comment(comm) {}

        const string&  GetDescription() const { return description; }
        const string&  GetComment() const { return comment; }
        const CObject& GetObject() const { return *object; }
        const CObject* GetObjectPtr() const { return object; }
        CObject* GetObjectPtr() { return object; }

        const string&  GetFileName() const { return fileName; }
        void SetFileName(const string& f) { fileName = f; }

    private:
        CRef<CObject> object;
        string description;
        string comment;
        string fileName;
    };

    typedef vector<SObject> TObjects;

    virtual TObjects& GetObjects() = 0;
    virtual string GetDescription() const = 0;

    virtual CObject* GetLoader() { return 0; }
};

END_NCBI_SCOPE


#endif  // GUI_UTILS___OBJECT_LOADER__HPP

