#ifndef GUI_OBJUTILS___CMD_CHANGE_GRAPH__HPP
#define GUI_OBJUTILS___CMD_CHANGE_GRAPH__HPP

/* $Id: cmd_change_graph.hpp 31553 2014-10-22 16:28:19Z katargir $
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
 * Authors:  Roman Katargin, Yoon Choi
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/utils/command_processor.hpp>
#include <objects/seqres/Seq_graph.hpp>
#include <objmgr/seq_graph_handle.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CCmdChangeGraph : 
    public CObject, public IEditCommand
{
public:
    CCmdChangeGraph(const objects::CSeq_graph_Handle& gh, const objects::CSeq_graph& new_graph);

    /// @name IEditCommand interface implementation
    /// @{
    virtual void Execute();
    virtual void Unexecute();
    virtual string GetLabel();
    /// @}

protected:
    objects::CSeq_graph_Handle m_GH;
    CRef<objects::CSeq_graph> m_Graph;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CMD_CHANGE_GRAPH__HPP
