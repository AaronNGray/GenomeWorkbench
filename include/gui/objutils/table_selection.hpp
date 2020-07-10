#ifndef GUI_OBJUTILS___TABLE_SELECTION__HPP
#define GUI_OBJUTILS___TABLE_SELECTION__HPP

/*  $Id: table_selection.hpp 32274 2015-01-29 17:42:50Z katargir $
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
 *  Government have not placed CAnyType restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for CAnyType particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 *
 * File Description:
 *    Event object for view communications
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

BEGIN_NCBI_SCOPE

class CSelectionEvent;
class ITableData;

class ITableSelection
{
public:
    virtual ~ITableSelection() {}

    virtual void GetRows(const ITableData& table, const CSelectionEvent& evt, vector<size_t>& rows) const = 0;
    virtual void GetSelection(const ITableData& table, const vector<size_t>& rows, CSelectionEvent& evt)  const = 0;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___TABLE_SELECTION__HPP
