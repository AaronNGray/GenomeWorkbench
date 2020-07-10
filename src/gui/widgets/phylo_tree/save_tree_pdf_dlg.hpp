#ifndef GUI_WIDGETS_PHY_TREE___SAVE_TREE_PDF_DLG_HPP
#define GUI_WIDGETS_PHY_TREE___SAVE_TREE_PDF_DLG_HPP

/*  $Id: save_tree_pdf_dlg.hpp 27936 2013-04-26 13:19:38Z falkrb $
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
 * Authors: Bob Falk
 *
 * File Description:
 *   
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/widgets/wx/wx_save_pdf_base_dlg.hpp>

#include <wx/defs.h>

BEGIN_NCBI_SCOPE

class CPhyloTreePane;
class CIndProgressBar;

class  NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CSaveTreePdfDlg : public CwxSavePdfBaseDlg
{       
public:
    CSaveTreePdfDlg(CPhyloTreePane* pane, wxWindow *parent=0);
    virtual ~CSaveTreePdfDlg();

    virtual bool TransferDataFromWindow();
    virtual bool TransferDataToWindow();

    void OnPrinter(wxCommandEvent& event);

protected:
    virtual void x_Init(wxBoxSizer* mainsizer, int widget_width);

    virtual void x_SavePdf();

    CPhyloTreePane* m_Pane;
    CIndProgressBar* m_ProgressBar;

private:
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS_NO_COPY(CSaveTreePdfDlg)
};


END_NCBI_SCOPE

#endif // GUI_WIDGETS_PHY_TREE___SAVE_TREE_PDF_DLG_HPP
