/*  $Id: feature_check_panel.hpp 18397 2008-11-28 19:45:17Z dicuccio $
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
 * Authors:  Roman Katargin
 */
#ifndef _FEATURE_CHECK_PANEL_H_
#define _FEATURE_CHECK_PANEL_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/treectrl.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class CwxTreeCtrlEx;
////@end forward declarations


#include <gui/widgets/wx/treectrl_ex.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFEATURECHECKPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFEATURECHECKPANEL_TITLE _("Tree")
#define SYMBOL_CFEATURECHECKPANEL_IDNAME ID_CTREECHECKPANEL
#define SYMBOL_CFEATURECHECKPANEL_SIZE wxSize(300, 300)
#define SYMBOL_CFEATURECHECKPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFeatureCheckPanel class declaration
 */

BEGIN_NCBI_SCOPE


class CFeatureCheckPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CFeatureCheckPanel )
    DECLARE_EVENT_TABLE()

public:
    typedef objects::CFeatListItem    TFeatTypeItem;
    typedef set<TFeatTypeItem>        TFeatTypeItemSet;
    typedef vector<string>            TStateVector;
    
    class wxFeatItemData : public wxTreeItemData {  
        TFeatTypeItem m_Item;
    public:
        wxFeatItemData(const TFeatTypeItem & item) : m_Item(item){}
        operator const TFeatTypeItem & (){return m_Item;}
        virtual ~wxFeatItemData(){}
    };

    /// Constructors
    CFeatureCheckPanel();
    CFeatureCheckPanel( wxWindow* parent, wxWindowID id = SYMBOL_CFEATURECHECKPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFEATURECHECKPANEL_POSITION, const wxSize& size = SYMBOL_CFEATURECHECKPANEL_SIZE, long style = SYMBOL_CFEATURECHECKPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFEATURECHECKPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFEATURECHECKPANEL_POSITION, const wxSize& size = SYMBOL_CFEATURECHECKPANEL_SIZE, long style = SYMBOL_CFEATURECHECKPANEL_STYLE );

    /// Destructor
    ~CFeatureCheckPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFeatureCheckPanel event handler declarations

////@end CFeatureCheckPanel event handler declarations

////@begin CFeatureCheckPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFeatureCheckPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CFeatureCheckPanel member variables
    CwxTreeCtrlEx* m_Tree;
    /// Control identifiers
    enum {
        ID_CTREECHECKPANEL = 10011,
        ID_TREECTRL1 = 10012
    };
////@end CFeatureCheckPanel member variables    
    void GetSelected(TFeatTypeItemSet& feat_types);
    void SetSelected(TFeatTypeItemSet& feat_types);

    void AccessState(TStateVector & check, TStateVector & expand, bool bRead=true);

    void Filter(string str);
protected:
    void x_InitTree(void);
    void x_GetSelections(const wxTreeItemId &id, TFeatTypeItemSet & set);
    void x_SetSelections(const wxTreeItemId &id, TFeatTypeItemSet & set);
    
    void x_AccessState(const wxTreeItemId &id, TStateVector & check, TStateVector & expand, bool bRead);

    string          m_Filter;
    TStateVector    m_Check;
    TStateVector    m_Expand;
};

END_NCBI_SCOPE

#endif
    // _FEATURE_CHECK_PANEL_H_
