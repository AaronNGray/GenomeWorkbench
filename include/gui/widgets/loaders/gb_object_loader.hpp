#ifndef GUI_WIDGETS___LOADERS___GB_OBJECT_LOADER__HPP
#define GUI_WIDGETS___LOADERS___GB_OBJECT_LOADER__HPP

/*  $Id: gb_object_loader.hpp 38406 2017-05-05 16:38:46Z katargir $
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
#include <gui/widgets/loaders/report_loader_errors.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CGBObjectLoader
class NCBI_GUIWIDGETS_LOADERS_EXPORT CGBObjectLoader :
    public CObject,
    private CReportLoaderErrors,
    public IObjectLoader,
    public IExecuteUnit
{
public:
    CGBObjectLoader(const vector<CRef<CObject> >& ids, const vector<string>& NAs);

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

private:
    vector<CRef<CObject> > m_Ids;
    vector<string>         m_NAs;
    TObjects               m_Objects;
};

/* @} */

END_NCBI_SCOPE


#endif // GUI_WIDGETS___LOADERS___GB_OBJECT_LOADER__HPP
