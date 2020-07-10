/*  $Id: quality_methods.cpp 44842 2020-03-25 17:20:30Z shkeda $
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
 * Authors:  Andrey Yazhuk, Andrei Shkeda, Greg Boratyn
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/aln_score/quality_methods.hpp>

#include <gui/widgets/aln_score/gradient_color_panel.hpp>

#include <gui/utils/table_names.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <algo/blast/core/blast_encoding.h>
#include <algo/blast/core/blast_util.h>

#include "wx/valgen.h"
#include "wx/choice.h"
#include "wx/checkbox.h"

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///  CQualityScoringMethod
///
CQualityScoringMethod::CQualityScoringMethod()
:   m_Space(0), m_Gap(0)
{
    x_Init(sm_DefGradientSize);
}


CQualityScoringMethod::~CQualityScoringMethod()
{
}


CQualityScoringMethod::CQualityScoringMethod(int colors)
:    m_Options(0), m_Space(0), m_Gap(0)
{
    x_Init(colors);
}


void CQualityScoringMethod::x_Init(int colors)
{
    m_Worst.Set(1.0f, 0.0f, 0.0f);
    m_Best.Set(0.8f, 0.8f, 0.8f);
    SetOptions(fIgnoreEmptySpace);

    CreateColorTable(colors);
}


CQualityScoringMethod::CQualityScoringMethod(const CQualityScoringMethod& orig)
:   m_Worst(orig.m_Worst),
    m_Best(orig.m_Best),
    m_Options(orig.m_Options),
    m_Space(orig.m_Space),
    m_Gap(orig.m_Gap)
{
    m_ColorTable = orig.m_ColorTable;
}


void    CQualityScoringMethod::SetOptions(int options)
{
    m_Options = options;
    // for now, always ignore empty spaces.
    m_Space = ' '; // (m_Options & fIgnoreEmptySpace) ? ' ' : 0;
    m_Gap = (m_Options & fIgnoreGaps) ? '-' : 0;
}


int     CQualityScoringMethod::GetOptions() const
{
    return m_Options;
}


bool CQualityScoringMethod::IsScoreWholeColumn() const
{
    return (m_Options & fScoreWholeColumn) ? true : false;
}


void    CQualityScoringMethod::CreateColorTable(int size)
{
    _ASSERT(size >= 0);

    m_ColorTable.SetSize(size);
    m_ColorTable.FillGradient(0, size, m_Worst, m_Best);
}


#define QUALITY_DESCR "This scheme assigns scores to residues" \
" based on how well a particular residue agrees with the others in a column."


string  CQualityScoringMethod::GetDescription() const
{
    return QUALITY_DESCR;
}


bool CQualityScoringMethod::IsAverageable() const
{
    return true;
}


void CQualityScoringMethod::CalculateScores(char /*cons*/,
                                            const string& column,
                                            TScore& col_score,
                                            TScoreVector& scores)
{
    _ASSERT(scores.size() == column.size());

    const string symbols(GetAllowedSymbols());

    size_t n_space = 0;
    vector<int> base_count(NCBI_FSM_DIM, 0);
    ITERATE(string, c_it, column) {
        char c = *c_it;
        if (c != m_Space  && c != m_Gap && c != m_AmbiguousResidue) {
            ++base_count[*c_it];
        } else {
            ++n_space;
        }
    }

    // size of the column w/o spaces we are not counting.
    size_t column_size = column.size() - n_space;
    vector<double> X(symbols.size(), 0);
    for (size_t r = 0; r <symbols.size(); ++r) {
        ITERATE(string, c_it, symbols) {
            X[r] += base_count[*c_it] * GetSubstitutionScore(*c_it, symbols[r]);
        }
        X[r] /= column_size;
    }

    double quality_score = 0;
    for (size_t i = 0; i < column.size(); ++i) {
        char c = column[i];
        if (c == m_AmbiguousResidue) {
            if (!IsScoreWholeColumn()) {
                scores[i] = IColumnScoringMethod::kAmbiguousResidueScore;
            }
        } else if (c != m_Space  && c != m_Gap) {
            double score_sq_sum = 0;
            for (size_t r = 0; r < symbols.size(); ++r) {
                double score_diff = X[r] - GetSubstitutionScore(symbols[r], c);
                score_sq_sum += score_diff * score_diff;
            }
            TScore resid_score = (TScore) sqrt(score_sq_sum);
            if ( ! IsScoreWholeColumn()) {
                scores[i] = resid_score;
            }
            quality_score += resid_score;
        }
    }

    col_score = (TScore) (quality_score / column_size);

    if (IsScoreWholeColumn()) {
        for (size_t i = 0; i < column.size(); ++i) {
            char c = column[i];
            if (c == m_AmbiguousResidue) {
                scores[i] = IColumnScoringMethod::kAmbiguousResidueScore;
            } else if (c != m_Space  && c != m_Gap) {
                scores[i] = col_score;
            }
        }
    }
}


bool CQualityScoringMethod::CanCalculateScores(const IScoringAlignment&)
{
    return true;
}


const CRgbaColor& CQualityScoringMethod::GetColorForNoScore(EColorType /*type*/) const
{
    static CRgbaColor not_used;
    _ASSERT(false);
    return not_used;
}


void CQualityScoringMethod::SetColorForNoScore(EColorType /*type*/,
                                               const CRgbaColor& /*color*/)
{
}


int     CQualityScoringMethod::GetSupportedColorTypes() const
{
    return fBackground;
}


const CRgbaColor& CQualityScoringMethod::GetColorForScore(TScore score,
                                                       EColorType type) const
{
    int size = (int)m_ColorTable.GetSize();
    _ASSERT(size >= 0);
    static const TScore kMaxScore = 15.0;
    score = (kMaxScore - score) / kMaxScore;
    if (score < 0.0) score = 0.0;

    if(type == fBackground) {
        int index = (int) floor(score * size);
        index = min(index, size - 1);
        return m_ColorTable[index];
    } else {
        _ASSERT(false); // not supported
        return m_ColorTable[0];
    }
}


const CMenuItem*    CQualityScoringMethod::GetMenu()
{
    return 0;
}


bool        CQualityScoringMethod::HasPropertiesPanel() const
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// CQualityScoringPanel

class CQualityScoringPanel : public wxPanel
{
    DECLARE_EVENT_TABLE()
public:
    CQualityScoringPanel(CQualityScoringMethod& method, wxWindow* parent, wxWindowID id = wxID_ANY);
protected:
    void CreateControls();
    void Init();
    void OnApply ( wxCommandEvent& event );
    CQualityScoringMethod& m_Method;
    wxChoice* m_MatrixChoice;
    CGradientColorPanel* m_GradPanel;
// data members
    bool m_ColScore;
    bool m_Gap;
};

BEGIN_EVENT_TABLE( CQualityScoringPanel, wxPanel )
    EVT_BUTTON( wxID_APPLY, CQualityScoringPanel::OnApply )
END_EVENT_TABLE()

CQualityScoringPanel::CQualityScoringPanel(
    CQualityScoringMethod& method, wxWindow* parent, wxWindowID id) :
        m_Method(method),
        m_MatrixChoice(), m_GradPanel()
{
    Init();
    Create(parent, id);
    CreateControls();
}

void CQualityScoringPanel::Init()
{
    int options = m_Method.GetOptions();
    m_ColScore = (options & CQualityScoringMethod::fScoreWholeColumn) != 0;
    m_Gap = (options & CQualityScoringMethod::fIgnoreGaps) != 0;
}

void CQualityScoringPanel::CreateControls()
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

    if (m_Method.GetType() & IAlnExplorer::fProtein) {
        CQualityScoringMethodAA* aa_method = dynamic_cast<CQualityScoringMethodAA*>(&m_Method);
        _ASSERT(aa_method);

        wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
        itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_RIGHT|wxALL, 5);

        itemBoxSizer2->Add(new wxStaticText(this, wxID_STATIC, _("Substitution matrix:")),
                           0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_MatrixChoice = new wxChoice(this, wxID_ANY);
        itemBoxSizer2->Add(m_MatrixChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

        // add names to popup.
        vector<string> table_names = CTableNames::AllTableNames();
        ITERATE(vector<string>, iter, table_names) {
            m_MatrixChoice->Append(ToWxString(*iter));
        }

        // set the current name.
        string matrix_name = CTableNames::TableToName(aa_method->GetSubstitutionMatrix()).c_str();
        if (!matrix_name.empty()) {
            m_MatrixChoice->SetStringSelection(ToWxString(matrix_name));
        }
    }

    wxCheckBox* itemCheckBox1 = new wxCheckBox(this, wxID_ANY, _("Score column as a whole"));
    itemCheckBox1->SetValidator(wxGenericValidator(&m_ColScore));
    itemBoxSizer1->Add(itemCheckBox1, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox2 = new wxCheckBox(this, wxID_ANY, _("Ignore Gaps"));
    itemCheckBox2->SetValidator(wxGenericValidator(&m_Gap));
    itemBoxSizer1->Add(itemCheckBox2, 0, wxALIGN_LEFT|wxALL, 5);
}

void CQualityScoringPanel::OnApply ( wxCommandEvent& WXUNUSED(event) )
{
    CGradientColorPanel::stParams params;
    m_GradPanel->GetParams(params);
    m_Method.m_Worst = params.m_FirstColor;
    m_Method.m_Best = params.m_LastColor;

    m_Method.CreateColorTable(params.m_Steps);

    int options = 0;
    if(m_Gap) options |= CQualityScoringMethod::fIgnoreGaps;
    if (m_ColScore) options |= CQualityScoringMethod::fScoreWholeColumn;
    m_Method.SetOptions(options);

    if (m_Method.GetType() & IAlnExplorer::fProtein ) {
        CQualityScoringMethodAA* aa_method = dynamic_cast<CQualityScoringMethodAA*>(&m_Method);
        _ASSERT(aa_method);
        wxString matrix_name = m_MatrixChoice->GetStringSelection();
        if (!matrix_name.empty()) {
            const SNCBIPackedScoreMatrix* m = CTableNames::NameToTable(ToStdString(matrix_name));
            if (m != NULL)
                aa_method->SetSubstitutionMatrix(m);
        }
    }
}

wxWindow* CQualityScoringMethod::CreatePropertiesPanel(wxWindow* parent)
{
    return new CQualityScoringPanel(*this, parent);
}

///////////////////////////////////////////////////////////////////////////////
///  CQualityScoringMethodNA
///
CQualityScoringMethodNA::CQualityScoringMethodNA()
    : CQualityScoringMethod()
{
}


CQualityScoringMethodNA::CQualityScoringMethodNA(int colors)
    : CQualityScoringMethod(colors)
{
}


CQualityScoringMethodNA::~CQualityScoringMethodNA()
{
}


string  CQualityScoringMethodNA::GetName() const
{
    return "Column Quality score - DNA";
}


IUITool* CQualityScoringMethodNA::Clone() const
{
    return new CQualityScoringMethodNA(*this);
}


int CQualityScoringMethodNA::GetType() const
{
    return IAlnExplorer::fDNA;
}


string CQualityScoringMethodNA::GetAllowedSymbols() const
{
    return string("ACGTU");
}


TNCBIScore CQualityScoringMethodNA::GetSubstitutionScore(int aa1, int aa2) const
{
    if (aa1 == aa2)
        return 15;
/*
    if ((aa1 == 'T'  &&  aa2 == 'U')  ||  (aa1 == 'U'  &&  aa2 == 'T'))
        return 1;
*/
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
///  CQualityScoringMethodAA
///
CQualityScoringMethodAA::CQualityScoringMethodAA()
    : CQualityScoringMethod()
{
    SetSubstitutionMatrix(&NCBISM_Blosum62);
    // or NCBISM_Blosum45 NCBISM_Blosum80 NCBISM_Pam30 NCBISM_Pam70 NCBISM_Pam250
}


CQualityScoringMethodAA::CQualityScoringMethodAA(int colors)
    : CQualityScoringMethod(colors)
{
    SetSubstitutionMatrix(&NCBISM_Blosum62);
    // or NCBISM_Blosum45 NCBISM_Blosum80 NCBISM_Pam30 NCBISM_Pam70 NCBISM_Pam250
}


CQualityScoringMethodAA::CQualityScoringMethodAA(const CQualityScoringMethodAA& orig)
    : CQualityScoringMethod(orig)
{
    SetSubstitutionMatrix(orig.m_SubstMatrix);
}


CQualityScoringMethodAA::~CQualityScoringMethodAA()
{
}


string  CQualityScoringMethodAA::GetName() const
{
    return "Column Quality score - Protein";
}


IUITool* CQualityScoringMethodAA::Clone() const
{
    return new CQualityScoringMethodAA(*this);
}


int CQualityScoringMethodAA::GetType() const
{
    return IAlnExplorer::fProtein;
}


TNCBIScore CQualityScoringMethodAA::GetSubstitutionScore(int aa1, int aa2) const
{
    return m_FullSubstMatrix.s[aa1][aa2];
}


string CQualityScoringMethodAA::GetAllowedSymbols() const
{
    return m_SubstMatrix->symbols;
}


void  CQualityScoringMethodAA::SetSubstitutionMatrix(const SNCBIPackedScoreMatrix* m)
{
    m_SubstMatrix = m;
    NCBISM_Unpack(m_SubstMatrix, &m_FullSubstMatrix);
}


const SNCBIPackedScoreMatrix* CQualityScoringMethodAA::GetSubstitutionMatrix()
{
    return m_SubstMatrix;
}


///////////////////////////////////////////////////////////////////////////////
///  CConservationScoringMethod
///
CConservationScoringMethod::CConservationScoringMethod()
{
    x_Init(sm_DefGradientSize);
}


CConservationScoringMethod::~CConservationScoringMethod()
{
    if (m_StdProb)
        free(m_StdProb);
}


CConservationScoringMethod::CConservationScoringMethod(int colors)
{
    x_Init(colors);
}


void CConservationScoringMethod::x_Init(int colors)
{
    m_Worst = {30, 144, 255, 255 };
    m_Best = { 255, 26, 26, 255 };
    m_NotScored.Set(0.8f, 0.8f, 0.8f);
    m_StdProb = BLAST_GetStandardAaProbabilities();
    CreateColorTable(colors);
}


CConservationScoringMethod::CConservationScoringMethod(const CConservationScoringMethod& orig)
    : CQualityScoringMethod(orig),
    m_ConservedOption(orig.m_ConservedOption),
    m_NotScored(orig.m_NotScored)
{
    m_ColorTable = orig.m_ColorTable;
    m_StdProb = BLAST_GetStandardAaProbabilities();
}


IUITool* CConservationScoringMethod::Clone() const
{
    return new CConservationScoringMethod(*this);
}

/*
void CConservationScoringMethod::CreateColorTable(int size)
{
    _ASSERT(size >= 0);

    m_ColorTable.SetSize(size);
    m_ColorTable.FillGradient(0, size, m_LessConserved, m_HighlyConserved);
}
*/

string CConservationScoringMethod::GetName() const
{
    return "Conservation";
}

const string CONSERVATION_METOD_DESCR = R"foo(This is a column-based method that highlights highly conserved and less conserved columns based on residues's relative entropy threshold.
Alignment columns with no gaps are colored in blue or red. 
The red color indicates highly conserved columns and blue indicates less conserved ones.)foo";

/*
#define CONSERVATION_METOD_DESCR "This is a column-based method that highlights \
highly conserved and less conserved columns based on residues's relative entropy threshold. \
Alignment columns with no gaps are colored in blue or red. \
The red color indicates highly conserved columns and blue indicates less conserved ones.\
The Conservation Setting can be used to select a threshold for determining which columns are colored in red. \
Numerical setting:\
The number is the relative entropy threshold, in bits, that must be met for an alignment column to be displayed in red.A larger number indicates higher degree of conservation.\
Identity setting: Only columns with one residue type will be colored in red.";
*/

string CConservationScoringMethod::GetDescription() const
{
    return CONSERVATION_METOD_DESCR;
}


int CConservationScoringMethod::GetType() const
{
    return IAlnExplorer::fProtein;
}

// Is it a column of only proper residues
static bool s_IsResidueOnlyColumn(const vector<int>& residue_counts)
{
    const int kGap = AMINOACID_TO_NCBISTDAA[(int)'-'];
    const int kResidueX = AMINOACID_TO_NCBISTDAA[(int)'X'];
    const int kResidueB = AMINOACID_TO_NCBISTDAA[(int)'B'];
    const int kResidueZ = AMINOACID_TO_NCBISTDAA[(int)'Z'];
    const int kResidueU = AMINOACID_TO_NCBISTDAA[(int)'U'];
    const int kResidueStar = AMINOACID_TO_NCBISTDAA[(int)'*'];
    return residue_counts[kGap] == 0 && residue_counts[kResidueX] == 0
        && residue_counts[kResidueB] == 0 && residue_counts[kResidueZ] == 0
        && residue_counts[kResidueU] == 0 && residue_counts[kResidueStar] == 0;
}

// Score calculation borrowed from blast / AlignFormatter / showalign_msa.cpp

static const int kAlphabetSize = 28;

static double s_GetInfoContent(const vector<int>& counts,
                               int num_rows,
                               const double* std_prob)
{
    _ASSERT((int)counts.size() == kAlphabetSize);

    double retval = 0.0;
    for (size_t i = 1; i < counts.size(); i++) {
        if (counts[i] == 0) {
            continue;
        }
        // _ASSERT(std_prob[i] > 0.0);
        // We can't reasonably estimate an information value for AAs with probability
        // zero, so we just skip them
        if (!(std_prob[i] > 0.0)) continue;
        double freq = (double)counts[i] / (double)num_rows;
        retval += freq * log(freq / std_prob[i]) / log(2.0);
    }

    return retval;
}

static bool s_IsIdentity(const vector<int>& counts, int num_rows)
{
    // if there are gaps, there is no identity
    if (counts[0] > 0) {
        return false;
    }

    for (size_t i = 1; i < counts.size(); i++) {
        if (counts[i] > 0 && counts[i] < num_rows) {
            return false;
        }
        if (counts[i] == num_rows) {
            return true;
        }
    }
    return false;
}

bool
CConservationScoringMethod::x_IsConserved(const vector<int>& counts,
int num_rows,
const double* std_prob) const
{
    bool retval = false;

    if (m_ConservedOption == eIdentity) {
        retval = s_IsIdentity(counts, num_rows);
    } else {
        double bit_threshold = 0.0;
        switch (m_ConservedOption) {
        case eOneBit: bit_threshold = 1.0; break;
        case eTwoBits: bit_threshold = 2.0; break;
        case eThreeBits: bit_threshold = 3.0; break;
        case eFourBits: bit_threshold = 4.0; break;
        case eIdentity:
        default:
            NCBI_THROW(CException, eInvalid, (string)"Unsupported conserved "
                       "column option " + NStr::IntToString(m_ConservedOption));
        };
        retval = s_GetInfoContent(counts, num_rows, std_prob) > bit_threshold;
    }

    return retval;
}


void CConservationScoringMethod::CalculateScores(char /*cons*/, const string& column,
                                                 TScore& col_score, TScoreVector& scores)
{

    vector<int> residue_counts(kAlphabetSize, 0);
    for (auto& aa : column) {
        int residue = AMINOACID_TO_NCBISTDAA[(int)aa];
        _ASSERT(residue < kAlphabetSize);
        residue_counts[residue]++;
    }
    col_score = -1.0f;
    if (s_IsResidueOnlyColumn(residue_counts)) {
        if (x_IsConserved(residue_counts, column.size(), m_StdProb)) {
            col_score = 1.f;
        } else {
            col_score = 0.f;
        }
    }
    fill(scores.begin(), scores.end(), col_score);
}


int CConservationScoringMethod::GetSupportedColorTypes() const
{
    return fBackground | fForeground;

}

const CRgbaColor& CConservationScoringMethod::GetColorForScore(TScore score,
                                                               EColorType type) const
{
    if (score < 0.) {
        if (type == fBackground) {
            return m_NotScored;
        } else {
            static const CRgbaColor color_black("black");
            return color_black;
        }
    }

    int size = (int)m_ColorTable.GetSize();
    _ASSERT(size >= 0);

    if (type == fBackground) {
        int index = (int)floor(score * size);
        index = min(index, size - 1);
        return m_ColorTable[index];
    } else {
        static const CRgbaColor color_white("white");
        return color_white;
    }
}

static map<CConservationScoringMethod::EOptions, string> sm_ConservationValues
{
    { CConservationScoringMethod::eOneBit, "one_bit"},
    { CConservationScoringMethod::eTwoBits, "two_bits"},
    { CConservationScoringMethod::eThreeBits, "three_bits"},
    { CConservationScoringMethod::eFourBits , "four_bits" },
    { CConservationScoringMethod::eIdentity , "identity", }
};

void CConservationScoringMethod::SetConservedOption(const string& name)
{
    string l_name(name);
    NStr::ToLower(l_name);
    for (auto& opt : sm_ConservationValues) {
        if (opt.second == l_name) {
            m_ConservedOption = opt.first;
            return;
        }
    }
    ERR_POST("Invalid conservation setting:\"" << name << "\"");
}


///////////////////////////////////////////////////////////////////////////////
/// CConservationScoringPanel

class CConservationScoringPanel : public wxPanel
{
    DECLARE_EVENT_TABLE()
public:
    CConservationScoringPanel(CConservationScoringMethod& method, wxWindow* parent, wxWindowID id = wxID_ANY);
protected:
    void CreateControls();
    void Init();
    void OnApply(wxCommandEvent& event);
    CConservationScoringMethod& m_Method;
    wxChoice* m_ConservedOptionChoice;
    CGradientColorPanel* m_GradPanel;
};

BEGIN_EVENT_TABLE(CConservationScoringPanel, wxPanel)
EVT_BUTTON(wxID_APPLY, CConservationScoringPanel::OnApply)
END_EVENT_TABLE()

CConservationScoringPanel::CConservationScoringPanel(CConservationScoringMethod& method, wxWindow* parent, wxWindowID id) :
    m_Method(method),
    m_ConservedOptionChoice(), 
    m_GradPanel()
{
    Init();
    Create(parent, id);
    CreateControls();
}

void CConservationScoringPanel::Init()
{
}

void CConservationScoringPanel::CreateControls()
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
    itemBoxSizer1->Add(m_GradPanel, 1, wxGROW | wxALL, 5);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_RIGHT | wxALL, 5);

    itemBoxSizer2->Add(new wxStaticText(this, wxID_STATIC, _("Conservation Setting:")),
                       0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_ConservedOptionChoice = new wxChoice(this, wxID_ANY);
    itemBoxSizer2->Add(m_ConservedOptionChoice, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    for (auto& opt : sm_ConservationValues) {
        wxString s = ToWxString(opt.second);
        s.Replace(wxT("_"), wxT(" "));
        s[0] = wxToupper(s[0]);
        auto index = m_ConservedOptionChoice->Append(s);
        if (opt.first == m_Method.m_ConservedOption)
            m_ConservedOptionChoice->SetSelection(index);
    }

}

void CConservationScoringPanel::OnApply(wxCommandEvent& WXUNUSED(event))
{
    CGradientColorPanel::stParams params;
    m_GradPanel->GetParams(params);
    m_Method.m_Worst = params.m_FirstColor;
    m_Method.m_Best = params.m_LastColor;
    m_Method.CreateColorTable(params.m_Steps);
    auto val = m_ConservedOptionChoice->GetString(m_ConservedOptionChoice->GetSelection());
    val.Replace(wxT(" "), wxT("_"));
    m_Method.SetConservedOption(ToStdString(val));
}

wxWindow* CConservationScoringMethod::CreatePropertiesPanel(wxWindow* parent)
{
    return new CConservationScoringPanel(*this, parent);
}


END_NCBI_SCOPE
