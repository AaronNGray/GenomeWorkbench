#ifndef GUI_WIDGETS_EDIT___SEQID_FIELD_NAME_PANEL__HPP
#define GUI_WIDGETS_EDIT___SEQID_FIELD_NAME_PANEL__HPP
/*  $Id: seqid_field_name_panel.hpp 41677 2018-09-07 17:07:21Z filippov $
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
 * Authors:  Igor Filippov
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

#include <wx/panel.h>


BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */



/*!
 * CSeqIdPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSeqIdPanel : public CFieldNamePanel
{    
    DECLARE_DYNAMIC_CLASS( CSeqIdPanel )
        
public:
    CSeqIdPanel() {Init();};
    CSeqIdPanel(wxWindow* parent)  {Init(); Create(parent);}
    bool Create( wxWindow* parent) 
    {
        CFieldNamePanel::Create( parent);
        if (GetSizer())
        {
            GetSizer()->SetSizeHints(this);
        }
        Centre();
        return true;
    }
    ~CSeqIdPanel() {}
    void Init() {}

    virtual string GetFieldName(const bool subfield = false) {return "SeqId";}
    virtual bool SetFieldName(const string& field) {return true;}
    virtual string GetMacroFieldName(const string &target, const string& selected_field = kEmptyStr) {return "SeqID()";}
};

IMPLEMENT_DYNAMIC_CLASS( CSeqIdPanel, CFieldNamePanel )

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___SEQID_FIELD_NAME_PANEL__HPP
