#ifndef GUI_WIDGETS___LOADERS___ASSEMBLY_OBJECT_LOADER__HPP
#define GUI_WIDGETS___LOADERS___ASSEMBLY_OBJECT_LOADER__HPP

/*  $Id: assembly_object_loader.hpp 39192 2017-08-18 14:12:57Z evgeniev $
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
#include <gui/gui_export.h>

#include <gui/utils/execute_unit.hpp>
#include <gui/utils/object_loader.hpp>

#include <gui/objutils/gencoll_svc.hpp>

#include <gui/widgets/loaders/report_loader_errors.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CAssemblyObjectLoader
class NCBI_GUIWIDGETS_LOADERS_EXPORT CAssemblyObjectLoader :
    public CObject,
    private CReportLoaderErrors,
    public IObjectLoader,
    public IExecuteUnit,
    public IGencollSvcErrors
{
public:
    CAssemblyObjectLoader(const vector<string>& accessions);

    /// @name IObjectLoader implementation
    /// @{
    virtual TObjects& GetObjects();
    virtual string GetDescription() const;
    /// @}

    /// @name IExecuteUnit implementation
    /// @{
    virtual bool PreExecute();
    virtual bool Execute(ICanceled& canceled);
    virtual bool PostExecute();
    /// @}

    /// @name IGencollSvcErrors implementation
    /// @{
    virtual void ReportError(const string &accession, const string &error);
    /// @}

private:
    vector<string> m_Accessions;
    TObjects       m_Objects;
    string         m_Descr;
};

/* @} */

END_NCBI_SCOPE


#endif // GUI_WIDGETS___LOADERS___ASSEMBLY_OBJECT_LOADER__HPP
