#ifndef GUI_WIDGETS___LOADERS___MAPPING_OBJECT_LOADER__HPP
#define GUI_WIDGETS___LOADERS___MAPPING_OBJECT_LOADER__HPP

/*  $Id: mapping_object_loader.hpp 28565 2013-08-02 16:11:53Z katargir $
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

#include <gui/utils/object_loader.hpp>
#include <objects/seq/seq_id_handle.hpp>

#include <objtools/readers/idmapper.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CSeq_annot;
END_SCOPE(objects)

///////////////////////////////////////////////////////////////////////////////
/// CMappingObjectLoader
class NCBI_GUIWIDGETS_LOADERS_EXPORT CMappingObjectLoader
{
public:
    CMappingObjectLoader() {}

protected:
    void x_CreateMapper(const string& assembly);
    void x_UpdateMap(objects::CSeq_annot& annot);
    bool x_ShowMappingDlg(IObjectLoader::TObjects& objects);

private:
    typedef map<objects::CSeq_id_Handle, objects::CSeq_id_Handle> TIdMap;
    auto_ptr<objects::IIdMapper> m_IdMapper;
    TIdMap  m_IdMap;
};

/* @} */

END_NCBI_SCOPE


#endif // GUI_WIDGETS___LOADERS___MAPPING_OBJECT_LOADER__HPP
