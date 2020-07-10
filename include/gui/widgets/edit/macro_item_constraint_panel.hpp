#ifndef GUI_WIDGETS_EDIT___MACRO_ITEM_CONSTRAINT_PANEL__HPP
#define GUI_WIDGETS_EDIT___MACRO_ITEM_CONSTRAINT_PANEL__HPP
/*  $Id: macro_item_constraint_panel.hpp 40769 2018-04-10 20:06:30Z asztalos $
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

#include <corelib/ncbistd.hpp>
#include <objects/macro/Constraint_choice.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>


BEGIN_NCBI_SCOPE


class CMacroMatcherPanelBase
{
public:
    virtual ~CMacroMatcherPanelBase() {}
    virtual pair<string, string> GetMatcher(const pair<string, string>& target, size_t num) = 0;
    virtual void SetMatcher(const string &item) = 0;
    virtual void SetFieldName(const string& field) {}
    virtual void PopulateFeatureListbox(objects::CSeq_entry_Handle seh) {}
    virtual void SetFieldNames(const vector<string>& field_names) {}
    virtual void GetVars(vector<string> &vars, size_t num) {}
    virtual string GetQualName(const pair<string, string>& target) {return kEmptyStr;}
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___MACRO_ITEM_CONSTRAINT_PANEL__HPP
