/*  $Id: fasta_align_export_page.hpp 39318 2017-09-12 16:00:18Z evgeniev $
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
 * Authors:  Vladislav Evgeniev
 *
 * File Description:
 *
 */

#ifndef PKG_ALIGNMENT___FASTA_ALIGN_EXPORT_PAGE_HPP
#define PKG_ALIGNMENT___FASTA_ALIGN_EXPORT_PAGE_HPP


#include <corelib/ncbistd.hpp>

/*!
* Includes
*/

#include <wx/panel.h>
#include <wx/listctrl.h>

#include <gui/widgets/wx/save_file_helper.hpp>

#include <gui/packages/pkg_alignment/fasta_align_export_params.hpp>

#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE

/*!
 * Forward declarations
 */

////@begin forward declarations
class CObjectListWidget;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFASTAALIGNEXPORTPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFASTAALIGNEXPORTPAGE_TITLE _("FASTA Alignment Export Page")
#define SYMBOL_CFASTAALIGNEXPORTPAGE_IDNAME ID_CFASTAALIGNEXPORTPAGE
#define SYMBOL_CFASTAALIGNEXPORTPAGE_SIZE wxSize(400, 300)
#define SYMBOL_CFASTAALIGNEXPORTPAGE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFastaAlignExportPage class declaration
 */

class CFastaAlignExportPage: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CFastaAlignExportPage )
    DECLARE_EVENT_TABLE()

public:
    CFastaAlignExportPage();
    CFastaAlignExportPage( wxWindow* parent, wxWindowID id = SYMBOL_CFASTAALIGNEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CFASTAALIGNEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_CFASTAALIGNEXPORTPAGE_SIZE, long style = SYMBOL_CFASTAALIGNEXPORTPAGE_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFASTAALIGNEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CFASTAALIGNEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_CFASTAALIGNEXPORTPAGE_SIZE, long style = SYMBOL_CFASTAALIGNEXPORTPAGE_STYLE );

    ~CFastaAlignExportPage();

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path);
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    void CreateControls();

    virtual bool TransferDataFromWindow();

    void SetObjects(TConstScopedObjects &objects);

    void OnObjectSelected(wxListEvent& event);

////@begin CFastaAlignExportPage event handler declarations

    void OnFileNameEdited( wxKeyEvent& event );

    void OnSelectFileClick( wxCommandEvent& event );

////@end CFastaAlignExportPage event handler declarations

////@begin CFastaAlignExportPage member function declarations

    CFastaAlignExportParams& GetData() { return m_data; }
    const CFastaAlignExportParams& GetData() const { return m_data; }
    void SetData(const CFastaAlignExportParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CFastaAlignExportPage member function declarations

    static bool ShowToolTips();

////@begin CFastaAlignExportPage member variables
    CObjectListWidget* m_ObjectSel;
    CFastaAlignExportParams m_data;
    enum {
        ID_CFASTAALIGNEXPORTPAGE = 10051,
        ID_PANEL4 = 10077,
        ID_TEXTCTRL14 = 10078,
        ID_BITMAPBUTTON = 10079
    };
////@end CFastaAlignExportPage member variables

private:
    void x_SanitizePath(wxString &path);

private:
    string      m_RegPath;
    bool        m_FileSelected;
    std::unique_ptr<CSaveFileHelper>    m_SaveFile;
};

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___FASTA_ALIGN_EXPORT_PAGE_HPP
