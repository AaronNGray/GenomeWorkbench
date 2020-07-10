#ifndef GUI_WIDGETS___LOADERS___ASN_OBJECT_LOADER__HPP
#define GUI_WIDGETS___LOADERS___ASN_OBJECT_LOADER__HPP

/*  $Id: asn_object_loader.hpp 41358 2018-07-13 16:18:43Z katargir $
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
#include <gui/widgets/loaders/report_loader_errors.hpp>

#include <objmgr/scope.hpp>

BEGIN_NCBI_SCOPE

class CObjectInfo;

/** @addtogroup GUI_CORE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CAsnObjectLoader
class NCBI_GUIWIDGETS_LOADERS_EXPORT CAsnObjectLoader : 
    public CObject,
    private CReportLoaderErrors,
    public IObjectLoader, 
    public IExecuteUnit
{
friend class CSniffReader;

public:
    typedef vector< CRef<CObject> > TOrigObjects;

    CAsnObjectLoader();
    CAsnObjectLoader(const vector<wxString>& filenames);
    CAsnObjectLoader(const string& asnTextData);

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

    void Init();
    void LoadFromStream(CNcbiIstream& istr, TObjects& objects, ESerialDataFormat fmt, ICanceled* canceled = 0);

private:
    bool x_OnTopObjectFound(const CObjectInfo& objInfo);

    vector<wxString> m_FileNames;
    const string     m_AsnTextData;
    /// Final complete object list with description set.
    TObjects         m_Objects;
    /// Original object list for current file.
    TOrigObjects     m_OrigObjects;

    wxString         m_CurrentFileName;

    CRef<objects::CScope> m_Scope;
};

/* @} */

END_NCBI_SCOPE


#endif // GUI_WIDGETS___LOADERS___ASN_OBJECT_LOADER__HPP
