#ifndef PKG_SEQUENCE_EDIT___PROTEIN_FIELD_PANEL__HPP
#define PKG_SEQUENCE_EDIT___PROTEIN_FIELD_PANEL__HPP
/*  $Id: protein_field_panel.hpp 33531 2015-08-10 15:33:27Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/packages/pkg_sequence_edit/single_choice_panel.hpp>

BEGIN_NCBI_SCOPE


class CProteinFieldPanel : public CSingleChoicePanel
{
public:
    CProteinFieldPanel(wxWindow* parent);
   
    ~CProteinFieldPanel() {}

    virtual string GetFieldName(const bool subfield = false);
    virtual bool SetFieldName(const string& field);

    static vector<string> GetStrings();
};


END_NCBI_SCOPE

#endif
    // PKG_SEQUENCE_EDIT___PROTEIN_FIELD_PANEL__HPP
