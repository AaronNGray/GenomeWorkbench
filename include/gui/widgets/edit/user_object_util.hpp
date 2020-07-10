/*  $Id: user_object_util.hpp 45095 2020-05-29 13:13:36Z bollin $
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


#ifndef _USER_OBJECT_UTIL_H_
#define _USER_OBJECT_UTIL_H_

#include <corelib/ncbistd.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>

BEGIN_NCBI_SCOPE

class CSingleUserFieldPanel : public wxPanel
{    
public:
    virtual ~CSingleUserFieldPanel() {};
    virtual CRef<objects::CUser_field> GetUser_field() = 0;

protected:
    CRef<objects::CUser_field> m_Field;

    void x_OnChange();
};


class CUserFieldManager
{
public:
    virtual ~CUserFieldManager() {};
    virtual CSingleUserFieldPanel * MakeEditor(wxWindow* parent, objects::CUser_field& field) = 0;
    virtual bool IsEligible(const objects::CUser_field& field) = 0;
    virtual bool IsEmpty(const objects::CUser_field& field) = 0;
    virtual CRef<objects::CUser_field> GetUserField(wxWindow* ctrl) = 0;
};


class CRefGeneTrackFieldManager : public CUserFieldManager
{
public:
    CRefGeneTrackFieldManager () {};
    virtual ~CRefGeneTrackFieldManager() {};

    virtual CSingleUserFieldPanel * MakeEditor(wxWindow* parent, objects::CUser_field& field);
    virtual bool IsEligible(const objects::CUser_field& field);
    virtual bool IsEmpty(const objects::CUser_field& field);
    virtual CRef<objects::CUser_field> GetUserField(wxWindow* ctrl);
private:
    bool x_IsTypeString(const string& str);

    static bool x_HasPopulatedFields(const objects::CUser_field& field);
};


class CAssemblyTrackFieldManager : public CUserFieldManager
{
public:
    CAssemblyTrackFieldManager () {};
    virtual ~CAssemblyTrackFieldManager () {};

    virtual CSingleUserFieldPanel * MakeEditor(wxWindow* parent, objects::CUser_field& field);
    virtual bool IsEligible(const objects::CUser_field& field);
    virtual bool IsEmpty(const objects::CUser_field& field);
    virtual CRef<objects::CUser_field> GetUserField(wxWindow* ctrl);
};


class CUserFieldListPanel : public wxPanel
{
public:
    virtual void AddLastField (wxWindow* link);
    void SetUser_object(CRef<objects::CUser_object> user);
    void PopulateFields(objects::CUser_object& user_obj);    
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    void SetNeedsEmptyLastRow(bool val) { m_NeedsEmptyLastRow = val; }

protected:
    CRef<objects::CUser_object> m_UserObj;
    vector< CRef< objects::CUser_field > > m_Fields;
    wxScrolledWindow* m_ScrolledWindow;
    wxBoxSizer* m_Sizer;

    int m_NumRows;
    int m_TotalHeight;
    int m_TotalWidth;
    int m_ScrollRate;
    int m_MaxRowsDisplayed;
    bool m_NeedsEmptyLastRow;

    CUserFieldManager * m_FieldManager;

    wxWindow* x_AddRow(CRef<objects::CUser_field> field, wxScrolledWindow* scrolled_window, wxBoxSizer* sizer);
    void x_UpdateFields();
    bool x_IsTPA() const;
};


bool IsLastRow(wxWindow* wnd, wxSizerItemList& itemList);

CRef<objects::CSeq_table> ReadTabDelimitedTable(const wxString& filename, bool read_header);
bool SaveTableFile (const string& save_file_name, CRef<objects::CSeq_table> values_table, bool include_header);
bool SaveTableRowAsListFile (const string& save_file_name, CRef<objects::CSeq_table> values_table, int col);
void AddAssemblyUserFieldsFromSeqTable (CRef<objects::CUser_object> obj, CRef<objects::CSeq_table> table);
CRef<objects::CSeq_table> SeqTableFromAssemblyFields (const objects::CUser_object& obj);


END_NCBI_SCOPE

#endif
// _USER_OBJECT_UTIL_H_
