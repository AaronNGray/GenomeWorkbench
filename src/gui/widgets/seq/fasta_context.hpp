#ifndef GUI_WIDGETS_SEQ___FASTA_CONTEXT__HPP
#define GUI_WIDGETS_SEQ___FASTA_CONTEXT__HPP

/*  $Id: fasta_context.hpp 39785 2017-11-03 16:40:10Z katargir $
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

#include <gui/widgets/text_widget/text_item_panel.hpp>

BEGIN_NCBI_SCOPE

class CFastaViewContext : public CTextPanelContext
{
    DECLARE_EVENT_TABLE()
public:
    CFastaViewContext(int leftMargin, objects::CScope* scope, const CSerialObject* so);
    ~CFastaViewContext();

    virtual wxMenu* CreateMenu() const;

protected:
    void OnExpandAll(wxCommandEvent& event);
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FASTA_CONTEXT__HPP
