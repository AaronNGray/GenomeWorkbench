/*  $Id: srcmod_edit_panel.hpp 29060 2013-10-01 19:21:17Z bollin $
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
#ifndef _SRCMOD_EDIT_PANEL_H_
#define _SRCMOD_EDIT_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>

#include <wx/panel.h>

BEGIN_NCBI_SCOPE


/*!
 * CSrcModEditPanel class declaration
 */

class CSrcModEditPanel: public wxPanel
{    
public:
    enum EEditorType {
        eText = 0,
        eCheckbox,
        eVoucher,
        eAltitude,
        eLatLon
    };

    /// Constructors
    CSrcModEditPanel (EEditorType editor_type) : m_EditorType(editor_type) {};

    /// Destructor
    ~CSrcModEditPanel () {};


    virtual string GetValue() = 0;
    virtual void SetValue(const string& val) = 0;
    virtual bool IsWellFormatted(const string& val) = 0;

    EEditorType GetEditorType() { return m_EditorType; }

    static EEditorType GetEditorTypeForSubSource (objects::CSubSource::TSubtype st);
    static EEditorType GetEditorTypeForOrgMod (objects::COrgMod::TSubtype st);

protected:
    EEditorType m_EditorType;
    void x_NotifyParent();
};


class CSrcModEditFactory
{
public:
    static CSrcModEditPanel* Create(wxWindow* parent, CSrcModEditPanel::EEditorType editor_type);
};


END_NCBI_SCOPE

#endif
    // _SRCMOD_EDIT_PANEL_H_
