#ifndef GUI_OBJUTILS___CMD_ENABLE_HUP_ID__HPP
#define GUI_OBJUTILS___CMD_ENABLE_HUP_ID__HPP

/*  $Id: cmd_enable_hup_id.hpp 38787 2017-06-19 15:17:54Z filippov $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <gui/utils/command_processor.hpp>

#include <objmgr/scope.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CCmdEnableHupId : public CObject, public IEditCommand
{
public:
    CCmdEnableHupId(objects::CScope &scope, bool enable)
        : m_scope(scope), m_enable(enable)
    {
        m_loader = objects::CGBDataLoader::RegisterInObjectManager(m_scope.GetObjectManager(), objects::CGBDataLoader::eIncludeHUP).GetLoader()->GetName();
    }

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}
    
protected:
    objects::CScope   &m_scope;
    bool m_enable;
    string m_loader;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_ENABLE_HUP_ID__HPP
