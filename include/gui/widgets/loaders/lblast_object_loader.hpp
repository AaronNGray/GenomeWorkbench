#ifndef GUI_WIDGETS___LOADERS___LBLAST_OBJECT_LOADER__HPP
#define GUI_WIDGETS___LOADERS___LBLAST_OBJECT_LOADER__HPP

/*  $Id: lblast_object_loader.hpp 34197 2015-11-24 20:35:39Z asztalos $
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

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/string.h>

#include <gui/utils/execute_unit.hpp>
#include <gui/utils/object_loader.hpp>
#include <objects/gbproj/LoaderDescriptor.hpp>

#include <gui/widgets/loaders/lblast_load_params.hpp>

#include <objmgr/scope.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_CORE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CAsnObjectLoader
class NCBI_GUIWIDGETS_LOADERS_EXPORT CLBLASTObjectLoader : 
    public CObject,
    public IObjectLoader, 
    public IExecuteUnit
{
public:
    CLBLASTObjectLoader(const CLBLASTLoadParams& params);

    /// @name IObjectLoader implementation
    /// @{
    virtual TObjects& GetObjects();
    virtual string GetDescription() const;
    virtual CObject* GetLoader();
    /// @}

    /// @name IExecuteUnit implementation
    /// @{
    virtual bool PreExecute();
    virtual bool Execute(ICanceled& canceled);
    virtual bool PostExecute();
    /// @}

    static CRef<objects::CLoaderDescriptor> CreateLoader(const string& db, bool nucleotide);
    static string GetLoaderName(const objects::CUser_object& obj);
    static string AddDataLoader(const objects::CUser_object& obj);

    static int CountSeqs(const string& db, bool proteins);

private:
    static string x_GetQuotedDBName(const string db);

    string          m_Dir;
    TObjects        m_Objects;
    CRef<objects::CScope> m_Scope;
    CRef<objects::CLoaderDescriptor> m_Loader;

    CLBLASTLoadParams  m_Params;
};

/* @} */

END_NCBI_SCOPE


#endif // GUI_WIDGETS___LOADERS___LBLAST_OBJECT_LOADER__HPP
