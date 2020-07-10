/*  $Id: simple_methods.cpp 45016 2020-05-07 18:51:35Z shkeda $
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
 * Authors:  Andrey Yazhuk
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>

#include <gui/widgets/aln_score/simple_methods.hpp>
#include <gui/widgets/aln_score/gradient_color_panel.hpp>

#include <gui/widgets/wx/color_picker_validator.hpp>

#include <gui/utils/table_names.hpp>

#include <wx/checkbox.h>
#include "wx/valgen.h"

BEGIN_NCBI_SCOPE

static const int kDefGradientSize = 32;

/// Set alignment specifi settings
void IColumnScoringMethod::UpdateSettings(const IScoringAlignment& aln)
{
    auto align_type = aln.GetAlignType();
    switch (align_type) {
        case IAlnExplorer::EAlignType::fDNA:
            m_AmbiguousResidue = 'N';
            break;
        case IAlnExplorer::EAlignType::fProtein:
            m_AmbiguousResidue = 'X';
            break;
        case IAlnExplorer::EAlignType::fMixed:
            m_AmbiguousResidue = 'X';
            break;
        default:
            break;
    }


}


///////////////////////////////////////////////////////////////////////////////
///  CSimpleScoringMethod
///
CSimpleScoringMethod::CSimpleScoringMethod()
: m_Space(0),  m_Gap(0)
{
    m_vCharCounts.resize(256);
    SetOptions(fIgnoreEmptySpace | fIgnoreGaps);

    x_Init(sm_DefGradientSize);
}


CSimpleScoringMethod::~CSimpleScoringMethod()
{
}


CSimpleScoringMethod::CSimpleScoringMethod(int colors)
: m_Space(0),  m_Gap(0)
{
    x_Init(colors);
}


void CSimpleScoringMethod::x_Init(int colors)
{
    m_Worst.Set(1.0f, 0.0f, 0.0f);
    m_Best.Set(0.8f, 0.8f, 0.8f);

    m_vCharCounts.resize(256);
    SetOptions(fIgnoreEmptySpace);

    CreateColorTable(colors);
}


CSimpleScoringMethod::CSimpleScoringMethod(const CSimpleScoringMethod& orig)
:   m_Worst(orig.m_Worst),
    m_Best(orig.m_Best),
    m_Space(orig.m_Space),
    m_Gap(orig.m_Gap)
{
    m_vCharCounts.resize(256);
    SetOptions(orig.m_Options);

    m_ColorTable = orig.m_ColorTable;
}


IUITool* CSimpleScoringMethod::Clone() const
{
    return new CSimpleScoringMethod(*this);
}


void CSimpleScoringMethod::SetOptions(int options)
{
    m_Options = options;
    m_Space = (m_Options & fIgnoreEmptySpace) ? ' ' : 0;
    m_Gap = (m_Options & fIgnoreGaps) ? '-' : 0;
}


int CSimpleScoringMethod::GetOptions()
{
    return m_Options;
}


void CSimpleScoringMethod::CreateColorTable(int size)
{
    _ASSERT(size >= 0);

    m_ColorTable.SetSize(size);
    m_ColorTable.FillGradient(0, size, m_Worst, m_Best);
}


string CSimpleScoringMethod::GetName() const
{
    return "Frequency-Based Difference";
}

#define SIMPLE_DESCR "This is a column-based method that assigns scores \
to residues in a column based on their representation in the column's frequency \
profile.  Residues that occur infrequently are highlighted darkly; columns \
that contain any degree of mismatch will alsobe highlighted."

string  CSimpleScoringMethod::GetDescription() const
{
    return SIMPLE_DESCR;
}


int  CSimpleScoringMethod::GetType() const
{
    return IAlnExplorer::fDNA  |  IAlnExplorer::fProtein;
}


bool    CSimpleScoringMethod::IsAverageable() const
{
    return true;
}

void CSimpleScoringMethod::CalculateScores(char /*cons*/, const string& column,
                                     TScore& col_score, TScoreVector& scores)
{
    _ASSERT(scores.size() == column.size()  &&  m_vCharCounts.size()  == 256);

    // reset histogram
    size_t space_n = 0;
    fill(m_vCharCounts.begin(),  m_vCharCounts.end(),  0);

    // calculate histogram
    ITERATE(string,  it,  column) {
        char c = *it;
        if(c != m_Space  &&  c != m_Gap && c != m_AmbiguousResidue)  {
            ++m_vCharCounts[(size_t) c];
        } else {
            space_n++;
        }
    }

    size_t total = column.size() - space_n;
    col_score = 0.0;

    // calculate individual scores
    for( size_t i = 0; i < column.size(); i++  )  {
        char c = column[i];
        if (c == m_AmbiguousResidue) {
            scores[i] = IColumnScoringMethod::kAmbiguousResidueScore;
        } else if (c != m_Space && c != m_Gap) {
            scores[i] = ((TScore)m_vCharCounts[(size_t)c]) / total;
        } else {
            scores[i] = 1.0;
        }
    }
}


bool CSimpleScoringMethod::CanCalculateScores(const IScoringAlignment& /* not used */)
{
    return true;
}


const CRgbaColor& CSimpleScoringMethod::GetColorForNoScore(EColorType /*type*/) const
{
    static CRgbaColor not_used;
    _ASSERT(false);
    return not_used;
}


void CSimpleScoringMethod::SetColorForNoScore(EColorType /*type*/,
                                              const CRgbaColor& /*color*/)
{
}


int CSimpleScoringMethod::GetSupportedColorTypes() const
{
    return fBackground;
}


const CRgbaColor& CSimpleScoringMethod::GetColorForScore(TScore score,
                                                       EColorType type) const
{
    int size = (int)m_ColorTable.GetSize();
    _ASSERT(size >= 0);

    if(type == fBackground) {
        int index = (int) floor(score * size);
        index = min(index, size - 1);
        return m_ColorTable[index];
    } else {
        _ASSERT(false); // not supported
        return m_ColorTable[0];
    }
}

/* this is en example of how to define a method-specifc menu
static
WX_DEFINE_MENU(SimpleScoringMenu)
    WX_MENU_ITEM(eCmd1, "Item 1")
    WX_MENU_ITEM(eCmd 2, "Item 2")
WX_END_MENU()
*/

const CMenuItem* CSimpleScoringMethod::GetMenu()
{
    /* this is an example of how method can provide a menu
        if(! m_Menu.get())    {
        m_Menu.reset(CreateMenuItems(SimpleScoringMenu));
    }*/
    return m_Menu.get();
}


bool CSimpleScoringMethod::HasPropertiesPanel() const
{
    return true;
}

class CSimpleScoringPanel : public wxPanel
{
    DECLARE_EVENT_TABLE()
public:
    CSimpleScoringPanel(CSimpleScoringMethod& method, wxWindow* parent, wxWindowID id = wxID_ANY);
protected:
    void CreateControls();
    void Init();
    void OnApply ( wxCommandEvent& event );
    CSimpleScoringMethod& m_Method;
    CGradientColorPanel* m_GradPanel;
// data members
    bool m_Space;
    bool m_Gap;
};

BEGIN_EVENT_TABLE( CSimpleScoringPanel, wxPanel )
    EVT_BUTTON( wxID_APPLY, CSimpleScoringPanel::OnApply )
END_EVENT_TABLE()

CSimpleScoringPanel::CSimpleScoringPanel(
    CSimpleScoringMethod& method, wxWindow* parent, wxWindowID id) :
        m_Method(method), m_GradPanel()
{
    Init();
    Create(parent, id);
    CreateControls();
}

void CSimpleScoringPanel::Init()
{
    int options = m_Method.GetOptions();
    m_Space = (options & CSimpleScoringMethod::fIgnoreEmptySpace) != 0;
    m_Gap = (options & CSimpleScoringMethod::fIgnoreGaps) != 0;
}

void CSimpleScoringPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer1);

    CGradientColorPanel::stParams params;
    params.m_FirstColor = m_Method.m_Worst;
    params.m_LastColor = m_Method.m_Best;
    params.m_Steps = (int)m_Method.m_ColorTable.GetSize();
    params.m_ThreeColors = false;
    params.m_Reversable = true;

    m_GradPanel = new CGradientColorPanel(params, this);
    itemBoxSizer1->Add(m_GradPanel, 1, wxGROW|wxALL, 5);

    wxCheckBox* itemCheckBox1 = new wxCheckBox(this, wxID_ANY, _("Ignore Empty Space"));
    itemCheckBox1->SetValidator(wxGenericValidator(&m_Space));
    itemBoxSizer1->Add(itemCheckBox1, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox2 = new wxCheckBox(this, wxID_ANY, _("Ignore Gaps"));
    itemCheckBox2->SetValidator(wxGenericValidator(&m_Gap));
    itemBoxSizer1->Add(itemCheckBox2, 0, wxALIGN_LEFT|wxALL, 5);
}

void CSimpleScoringPanel::OnApply ( wxCommandEvent& event )
{
    wxUnusedVar(event);

    CGradientColorPanel::stParams params;
    m_GradPanel->GetParams(params);
    m_Method.m_Worst = params.m_FirstColor;
    m_Method.m_Best = params.m_LastColor;

    int options = 0;
    if(m_Space) options |= CSimpleScoringMethod::fIgnoreEmptySpace;
    if (m_Gap) options |= CSimpleScoringMethod::fIgnoreGaps;
    m_Method.SetOptions(options);

    m_Method.CreateColorTable(params.m_Steps);
}

wxWindow* CSimpleScoringMethod::CreatePropertiesPanel(wxWindow* parent)
{
    return new CSimpleScoringPanel(*this, parent);
}

///////////////////////////////////////////////////////////////////////////////
/// CSNPScoringMethod
///

CSNPScoringMethod::CSNPScoringMethod()
:   CSimpleScoringMethod(2),
    m_UndefColor(0.8f, 0.8f, 1.0f),
    m_NoScores(false)
{
}


CSNPScoringMethod::CSNPScoringMethod(const CSNPScoringMethod& orig)
: CSimpleScoringMethod(orig),
  m_UndefColor(orig.m_UndefColor)
{
}


IUITool* CSNPScoringMethod::Clone() const
{
    return new CSNPScoringMethod(*this);
}


string CSNPScoringMethod::GetName() const
{
    return "Show Differences";
}


#define SNP_DESCR "This method highlights differences observed from the master \
sequence in an alignment.  In order to see any coloration with this scheme, \
you must select a master sequence.  You can adjust the master sequence \
on the fly and display differences by selecting a new master from the context menu.";

string CSNPScoringMethod::GetDescription() const
{
    return SNP_DESCR;
}


const static float kSameBase =  1.0f;   // base is the same as consensus
const static float kSNP = 0.0f;         // base is different (SNP)
const static float kUndef = -1.0f;      // consensus is not specified

void CSNPScoringMethod::CalculateScores(char cons,  const string& column,
                                     TScore& /*col_score*/,  TScoreVector& scores)
{
    _ASSERT(scores.size() == column.size());
    /**
    _ASSERT(cons != 0);

    for( size_t i = 0; i < column.size(); i++  )  {
        char c = column[i];
        bool same = (c == cons)  ||  (c == m_Gap)  ||  (c == m_Space);
        scores[i] = same ?  kSameBase : kSNP;
    }
    **/
    if(cons == 0)   {
        fill(scores.begin(), scores.end(), kUndef);
    } else {
        for( size_t i = 0; i < column.size(); i++  )  {
            char c = column[i];
            if (c == m_AmbiguousResidue) {
                scores[i] = IColumnScoringMethod::kAmbiguousResidueScore;
            } else {
                bool same = (c == cons)  ||  (c == m_Gap)  ||  (c == m_Space);
                scores[i] = same ?  kSameBase : kSNP;
            }
        }
    }

}

int CSNPScoringMethod::GetSupportedColorTypes() const
{
    return fBackground;
}


bool CSNPScoringMethod::CanCalculateScores(const IScoringAlignment& /*aln*/)
{
    return true;
}


const CRgbaColor& CSNPScoringMethod::GetColorForNoScore(EColorType type) const
{
    _VERIFY(type == fBackground);
    return m_UndefColor;
}


void CSNPScoringMethod::SetColorForNoScore(EColorType type,
                                           const CRgbaColor& color)
{
    _VERIFY(type == fBackground);
    m_UndefColor = color;
}


const CRgbaColor& CSNPScoringMethod::GetColorForScore(TScore score,
                                                    EColorType type) const
{
    if(score < 0)   {
        return m_UndefColor;
    } else {
        return CSimpleScoringMethod::GetColorForScore(score, type);
    }
}


const CMenuItem* CSNPScoringMethod::GetMenu()
{
    return m_Menu.get();
}


bool CSNPScoringMethod::HasPropertiesPanel() const
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// CSNPScoringPanel

class CSNPScoringPanel : public wxPanel
{
    DECLARE_EVENT_TABLE()
public:
    CSNPScoringPanel(CSNPScoringMethod& method, wxWindow* parent, wxWindowID id = wxID_ANY);
protected:
    void CreateControls();
    void Init();
    void OnApply ( wxCommandEvent& event );
    CSNPScoringMethod& m_Method;
// data members
    bool m_Space;
    bool m_Gap;
};

BEGIN_EVENT_TABLE( CSNPScoringPanel, wxPanel )
    EVT_BUTTON( wxID_APPLY, CSNPScoringPanel::OnApply )
END_EVENT_TABLE()

CSNPScoringPanel::CSNPScoringPanel(
    CSNPScoringMethod& method, wxWindow* parent, wxWindowID id) :
        m_Method(method)
{
    Init();
    Create(parent, id);
    CreateControls();
}

void CSNPScoringPanel::Init()
{
    int options = m_Method.GetOptions();
    m_Space = (options & CSimpleScoringMethod::fIgnoreEmptySpace) != 0;
    m_Gap = (options & CSimpleScoringMethod::fIgnoreGaps) != 0;
}

void CSNPScoringPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);

    wxColourPickerCtrl* itemColorPicker1 = new wxColourPickerCtrl(this, wxID_ANY);
    itemColorPicker1->SetValidator(CColorPickerValidator(&m_Method.m_UndefColor));
    itemBoxSizer2->Add(itemColorPicker1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemBoxSizer2->Add(new wxStaticText(this, wxID_STATIC, _("Neutral (no master specified)")),
                       0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_LEFT|wxALL, 0);

    itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    wxColourPickerCtrl* itemColorPicker2 = new wxColourPickerCtrl(this, wxID_ANY);
    itemColorPicker2->SetValidator(CColorPickerValidator(&m_Method.m_Worst));
    itemBoxSizer2->Add(itemColorPicker2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemBoxSizer2->Add(new wxStaticText(this, wxID_STATIC, _("SNP")),
                       0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_LEFT|wxALL, 0);

    itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    wxColourPickerCtrl* itemColorPicker3 = new wxColourPickerCtrl(this, wxID_ANY);
    itemColorPicker3->SetValidator(CColorPickerValidator(&m_Method.m_Best));
    itemBoxSizer2->Add(itemColorPicker3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemBoxSizer2->Add(new wxStaticText(this, wxID_STATIC, _("Normal")),
                       0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_LEFT|wxALL, 0);

    wxCheckBox* itemCheckBox1 = new wxCheckBox(this, wxID_ANY, _("Ignore Empty Space"));
    itemCheckBox1->SetValidator(wxGenericValidator(&m_Space));
    itemBoxSizer1->Add(itemCheckBox1, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox2 = new wxCheckBox(this, wxID_ANY, _("Ignore Gaps"));
    itemCheckBox2->SetValidator(wxGenericValidator(&m_Gap));
    itemBoxSizer1->Add(itemCheckBox2, 0, wxALIGN_LEFT|wxALL, 5);
}

void CSNPScoringPanel::OnApply ( wxCommandEvent& event )
{
    wxUnusedVar(event);

    int options = 0;
    if(m_Space) options |= CSimpleScoringMethod::fIgnoreEmptySpace;
    if (m_Gap) options |= CSimpleScoringMethod::fIgnoreGaps;
    m_Method.SetOptions(options);
}

wxWindow* CSNPScoringMethod::CreatePropertiesPanel(wxWindow* parent)
{
    return new CSNPScoringPanel(*this, parent);
}


///////////////////////////////////////////////////////////////////////////////
/// CTestSeqBasedMethod

CTestSeqBasedMethod::CTestSeqBasedMethod()
{
    CreateColorTable();
}


IUITool* CTestSeqBasedMethod::Clone() const
{
    return new CTestSeqBasedMethod();
}


void    CTestSeqBasedMethod::CreateColorTable()
{
    m_vColors.resize(5);
    m_vColors[0] = CRgbaColor(1.0f,  1.0f,  1.0f);
    m_vColors[1] = CRgbaColor(1.0f,  0.8f,  0.8f);
    m_vColors[2] = CRgbaColor(0.8f,  1.0f,  0.8f);
    m_vColors[3] = CRgbaColor(0.8f,  0.8f,  1.0f);
    m_vColors[4] = CRgbaColor(0.6f,  1.0f,  1.0f);
}


string CTestSeqBasedMethod::GetName() const
{
    return "Test Sequence-based";
}


string CTestSeqBasedMethod::GetDescription() const
{
    return "";
}


int CTestSeqBasedMethod::GetType() const
{
    return IAlnExplorer::fDNA;
}


bool CTestSeqBasedMethod::IsAverageable() const
{
    return false;
}


void CTestSeqBasedMethod::CalculateScores(IAlnExplorer::TNumrow row,
                                          const IScoringAlignment& aln,
                                          TScoreColl& scores)
{
    IAlnExplorer::TSignedRange range(aln.GetAlnStart(), aln.GetAlnStop());
    string  buf;
    aln.GetAlnSeqString(row, buf, range);

    scores.SetFrom(range.GetFrom());

    size_t color_index = 0;
    for(  int i = 0;  i < range.GetLength();  i++ ) {
        switch(buf[i]) {
        case 'A':
        case 'a':
            color_index = 1;
            break;

        case 'C':
        case 'c':
            color_index = 2;
            break;

        case 'T':
        case 't':
            color_index = 3;
            break;

        case 'G':
        case 'g':
            color_index = 4;
            break;

        default:
            color_index = 0;
            break;
        }
        scores.push_back((TScore) color_index, (int) 1);
    }
}


int CTestSeqBasedMethod::GetSupportedColorTypes() const
{
    return fBackground;
}


const CRgbaColor& CTestSeqBasedMethod::GetColorForScore(TScore score,
                                                       EColorType type) const
{
    _VERIFY(type == fBackground); // the only type supported

    size_t index = (size_t) score;
    _ASSERT(index < m_vColors.size());
    return m_vColors[index];
}


const CRgbaColor& CTestSeqBasedMethod::GetColorForNoScore(IScoringMethod::EColorType type) const
{
    _VERIFY(type == fBackground);
    return m_NoScoreColor;
}


void CTestSeqBasedMethod::SetColorForNoScore(IScoringMethod::EColorType type, const CRgbaColor& color)
{
    wxUnusedVar(type);
    wxUnusedVar(color);
}




///////////////////////////////////////////////////////////////////////////////
///  CNonSynonymousScoringMethod
///
CNonSynonymousScoringMethod::CNonSynonymousScoringMethod()
{
    x_Init();
}


CNonSynonymousScoringMethod::~CNonSynonymousScoringMethod()
{

}


void CNonSynonymousScoringMethod::x_Init()
{
    m_ColumnCount = 0;
    //    m_Colors[eMismatch] = { 255, 231, 198 }; // original FLU viewer yellowish
    m_Colors[eMismatch] = { 255, 0, 0 };

    m_Colors[eMatch] = { 204, 204, 204 };
    m_Colors[eMatchLight] = m_Colors[eMatch];
    m_Colors[eMatchLight].Lighten(0.3f);
    
    m_ColorTable.SetSize(kDefGradientSize);
    m_ColorTable.FillGradient(0, kDefGradientSize, m_Colors[eMismatch], m_Colors[eMatch]);

}


CNonSynonymousScoringMethod::CNonSynonymousScoringMethod(const CNonSynonymousScoringMethod& orig)
    : m_SequenceLevel(orig.m_SequenceLevel),
    m_ColumnCount(0)
{
    for (auto i = 0; i < EColors::eLast; ++i)
      m_Colors[i] = orig.m_Colors[i];
    m_ColorTable = orig.m_ColorTable;
}


IUITool* CNonSynonymousScoringMethod::Clone() const
{
    return new CNonSynonymousScoringMethod(*this);
}


string CNonSynonymousScoringMethod::GetName() const
{
    return "Nonsynonymous substitutions";
}

const string CNONSYNONYMOUS_METHOD_DESCR = R"foo(All triplets with nonsynonymous substitutions are colored in red.
Note: nonsynonymous substitution coloring is available only for CDS alignments.
Any sequence in alignment or pre-calculated consensus with only ACGT letters can be set as a master.)foo";


string CNonSynonymousScoringMethod::GetDescription() const
{
    return CNONSYNONYMOUS_METHOD_DESCR;
}


int CNonSynonymousScoringMethod::GetType() const
{
    return IAlnExplorer::fDNA;
}


void CNonSynonymousScoringMethod::CalculateScores(char cons, const string& column,
    TScore& col_score, TScoreVector& scores)
{
    if (cons == 0) {
        fill(scores.begin(), scores.end(), -1.);
    } else {
        for (size_t i = 0; i < column.size(); i++) {
            bool same = (column[i] == cons);
            if (same) {
                scores[i] = m_SequenceLevel && m_ColumnCount % 2 != 0 ? kSameBase * 0.5 : kSameBase;
            } else {
                scores[i] = kSNP;
            }
        }
        m_ColumnCount++;
    }
}


int CNonSynonymousScoringMethod::GetSupportedColorTypes() const
{
    return fBackground;

}

const CRgbaColor& CNonSynonymousScoringMethod::GetColorForScore(TScore score, EColorType type) const
{
    
    if (m_SequenceLevel) {
        if (score == kSNP)
            return m_Colors[eMismatch];
        if (score == kSameBase * 0.5)
            return m_Colors[eMatchLight];
        return m_Colors[eMatch];
    }
    
    int size = (int)m_ColorTable.GetSize();
    _ASSERT(size >= 0);
    int index = (int) floor(score * size);
    index = min(index, size - 1);
    return m_ColorTable[index];
}

const CRgbaColor& CNonSynonymousScoringMethod::GetColorForNoScore(EColorType /*type*/) const
{
    static CRgbaColor not_used;
    _ASSERT(false);
    return not_used;
}


void CNonSynonymousScoringMethod::SetColorForNoScore(EColorType /*type*/,
                                                     const CRgbaColor& /*color*/)
{
}

END_NCBI_SCOPE
