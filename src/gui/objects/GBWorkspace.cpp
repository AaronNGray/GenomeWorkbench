/* $Id: GBWorkspace.cpp 39528 2017-10-05 15:27:37Z katargir $
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
 * Author:  .......
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using the following specifications:
 *   'gui_project.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>

// generated includes
#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <gui/objects/GBProject.hpp>
#include <gui/objects/GBProjectHandle.hpp>
#include <serial/iterator.hpp>
#include <gui/objutils/label.hpp>

#include <algorithm>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::


class CGBWorkspaceLabelHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* str,
                  CLabel::ELabelType type,
                  objects::CScope* scope) const
    {
        const CGBWorkspace* wks = dynamic_cast<const CGBWorkspace*>(&obj);
        if (wks  &&  wks->IsSetWorkspace()) {
            CTypeConstIterator<CGBProject> iter(wks->GetWorkspace());
            string s;
            for ( ;  iter;  ++iter) {
                if ( !s.empty() ) {
                    s += "; ";
                }
                CLabel::GetLabel(*iter, &s, type, scope);
            }
            *str += s;
        }
    }
};


///////////////////////////////////////////////////////////////////////////////
/// CGBWorkspace

CAtomicCounter CGBWorkspace::m_IdCounter;

CGBWorkspace::CGBWorkspace(void)
{
    m_Id = m_IdCounter.Add(1) - 1;

    if ( !CLabel::HasHandler(*CGBWorkspace::GetTypeInfo()) ) {
        CLabel::RegisterLabelHandler(*CGBWorkspace::GetTypeInfo(),
                                     *new CGBWorkspaceLabelHandler());
    }
}


// destructor
CGBWorkspace::~CGBWorkspace(void)
{
}


CGBProjectHandle* CGBWorkspace::GetProjectFromScope(const CScope& scope)
{
    for (CTypeIterator<CGBProjectHandle> it(SetWorkspace()); it; ++it) {
        if (&scope == it->GetScope()) {
            return &*it;
        }
    }
    return 0;
}


CGBProjectHandle* CGBWorkspace::GetProjectFromId(CGBProjectHandle::TId id)
{
    for (CTypeIterator<CGBProjectHandle> it(SetWorkspace()); it; ++it) {
        if (id == it->GetId()) {
            return &*it;
        }
    }
    return 0;
}


bool CGBWorkspace::RemoveProject(CGBProjectHandle::TId id)
{
    CWorkspaceFolder* parent = SetWorkspace().FindParentFolder(id);
    if (parent) {
        parent->RemoveProject(id);
        return true;
    }
    return false;
}


string CGBWorkspace::MakeUniqueProjectTitle(const string& label) const
{
    CUniqueLabelGenerator generator;
    const CWorkspaceFolder& root = GetWorkspace();
    for ( CTypeConstIterator<CGBProjectHandle> it(root);  it;  ++it) {
        const string& s = it->GetTitle();
        generator.AddExistingLabel(s);
    }
    return generator.MakeUniqueLabel(label);
}


vector<int> CGBWorkspace::GetUnloadedProjects() const
{
    vector<int> projIds;
    for (CTypeConstIterator<CGBProjectHandle> it(GetWorkspace()); it; ++it) {
        if (!it->IsLoaded())
            projIds.push_back(it->GetId());
    }
    return projIds;
}


vector<int> CGBWorkspace::GetLoadedProjects() const
{
    vector<int> projIds;
    for (CTypeConstIterator<CGBProjectHandle> it(GetWorkspace()); it; ++it) {
        if (it->IsLoaded())
            projIds.push_back(it->GetId());
    }
    return projIds;
}


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 65, chars: 1894, CRC32: 164c029b */