/*  $Id: item_constraint_panel.hpp 39226 2017-08-23 20:21:52Z filippov $
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

#ifndef ITEM_CONSTRAINT_PANEL__HPP
#define ITEM_CONSTRAINT_PANEL__HPP


#include <corelib/ncbistd.hpp>


BEGIN_NCBI_SCOPE
struct SFieldTypeAndMatcher;

class CItemConstraintPanel
{
public:
    virtual ~CItemConstraintPanel() {}
    virtual pair<string, SFieldTypeAndMatcher > GetItem(CFieldNamePanel::EFieldType field_type) = 0;
    virtual void SetItem(const SFieldTypeAndMatcher &item) = 0;
};


END_NCBI_SCOPE

#endif  // ITEM_CONSTRAINT_PANEL__HPP