#ifndef GUI_PACKAGES_PKG_SEQUENCE_EDIT___RESTORE_LOCAL_FILE__HPP
#define GUI_PACKAGES_PKG_SEQUENCE_EDIT___RESTORE_LOCAL_FILE__HPP

/*  $Id: restore_local_file.hpp 35587 2016-05-27 18:19:42Z asztalos $
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
 * Authors: Andrea Asztalos
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <objects/gbproj/ProjectItem.hpp>
#include <gui/framework/workbench.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CScope;
END_SCOPE(objects);

class IProjectView;
class CGBDocument;

class CLocalFileRestorer
{
public:
    CLocalFileRestorer(IWorkbench* workbench);

    // swaps the top level object associated with text view
    // when convert is true, the seq-submit is converted into a seq-entry
    void RestoreFile(bool convert = false);

    IProjectView* FindSMARTPrjTextView() const;

    bool ReadObjectFromFile();
    
    void CloseOpenViews(CGBDocument* doc);
    
private:

    bool x_DetachOrigObject(CRef<objects::CProjectItem> item, objects::CScope& scope);
    bool x_AttachNewObject(CRef<objects::CProjectItem> item, objects::CScope& scope, bool convert);

    IWorkbench* m_Workbench;
    CRef<CSerialObject> m_NewSerialObj;
    CIRef<CProjectService> m_PrjService;
};

END_NCBI_SCOPE

#endif
    // GUI_PACKAGES_PKG_SEQUENCE_EDIT___RESTORE_LOCAL_FILE__HPP
