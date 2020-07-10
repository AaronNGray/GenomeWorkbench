#ifndef GUI_WIDGETS___LOADERS___GFF_OBJECT_LOADER__HPP
#define GUI_WIDGETS___LOADERS___GFF_OBJECT_LOADER__HPP

/*  $Id: gff_object_loader.hpp 34195 2015-11-24 14:11:57Z asztalos $
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

#include <gui/widgets/loaders/gff_load_params.hpp>

#include <gui/widgets/loaders/mapping_object_loader.hpp>
#include <gui/widgets/loaders/report_loader_errors.hpp>

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CGffObjectLoader
class NCBI_GUIWIDGETS_LOADERS_EXPORT CGffObjectLoader : 
    public CObject, 
    private CMappingObjectLoader,
    private CReportLoaderErrors,
    public IObjectLoader, 
    public IExecuteUnit
{
public:
    CGffObjectLoader(const CGffLoadParams& params, const vector<wxString>& filenames);

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
    CGffLoadParams    m_Params;
    vector<wxString>  m_FileNames;
    TObjects          m_Objects;
};

/* @} */

END_NCBI_SCOPE


#endif // GUI_WIDGETS___LOADERS___GFF_OBJECT_LOADER__HPP