/*  $Id: bulk_cmd_panel.hpp 32483 2015-03-06 22:14:30Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */
#ifndef _BULKCMDPANEL_H_
#define _BULKCMDPANEL_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_location_panel.hpp>
#include <wx/panel.h>

BEGIN_NCBI_SCOPE


/*!
 * CBulkCmdPanel class declaration
 */

class CBulkCmdPanel: public wxPanel
{    
public:
    /// Constructors
    CBulkCmdPanel () {};
    CBulkCmdPanel(bool add_location_panel) : m_add_location_panel(add_location_panel), m_loc(NULL), m_Location(NULL) {}
    /// Destructor
    virtual ~CBulkCmdPanel () {};


    virtual CRef<CCmdComposite> GetCommand() = 0;
    virtual string GetErrorMessage() = 0;

    void SetLoc(CRef<objects::CSeq_loc> loc) {m_loc = loc;}
    virtual void AddOneCommand(const objects::CBioseq_Handle& bsh, CRef<CCmdComposite> cmd) {}
protected:
    virtual bool ShouldAddToExisting()
        {
            if (m_Location)
                return m_Location->ShouldAddToExisting();
            return true;
        }
    
    virtual CRef<objects::CSeq_loc> GetLocForBioseq (const objects::CBioseq& bioseq)
        {
            if (m_Location)
                return m_Location->GetLocForBioseq(bioseq);
            return m_loc;
        }
    bool m_add_location_panel;
    CRef<objects::CSeq_loc> m_loc;
    CBulkLocationPanel* m_Location;
};

END_NCBI_SCOPE

#endif
    // _BULKCMDPANEL_H_
