 /*  $Id: paint_sequence.cpp 44935 2020-04-21 20:16:21Z asztalos $
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


#include <ncbi_pch.hpp>

#include <wx/graphics.h>
#include <wx/settings.h>
#include <wx/clipbrd.h>
#include <wx/msgdlg.h> 
#include <wx/evtloop.h>
#include <objmgr/util/sequence.hpp>
#include <util/sequtil/sequtil_manip.hpp>
#include <gui/widgets/edit/paint_sequence.hpp>
#include <gui/widgets/edit/edit_sequence.hpp>

BEGIN_NCBI_SCOPE

static const int space_between_groups = 1;
static const int chars_per_group = 10;

IMPLEMENT_DYNAMIC_CLASS( CPaintSequence, wxVScrolledWindow )


BEGIN_EVENT_TABLE(CPaintSequence, wxVScrolledWindow)
    EVT_PAINT(CPaintSequence::OnPaint)
    EVT_ERASE_BACKGROUND(CPaintSequence::OnEraseBackground)
    EVT_SIZE(CPaintSequence::OnResize)
    EVT_LEFT_UP(CPaintSequence::OnMouseClick)
    EVT_KEY_DOWN(CPaintSequence::OnKeyDown)
    EVT_CHAR(CPaintSequence::OnChar)
    EVT_MOTION(CPaintSequence::OnMouseDrag)
    EVT_LEFT_DOWN(CPaintSequence::OnMouseDown)
END_EVENT_TABLE()


CPaintSequence::CPaintSequence(wxWindow *parent, const string &seq, const vector<int> &seq_len, const vector<vector<pair<TSeqPos,TSeqPos> > > &feat_ranges, 
                               const vector<pair<string,objects::CSeqFeatData::ESubtype> > &feat_types,
                               const vector<objects::CBioseq_Handle::EVectorStrand> &feat_strand,
                               const  vector<int> &feat_frames,
                               const vector< CRef<CGenetic_code> > &genetic_code, const vector<bool> &feat_partial5,
                               const string &allowed_char_set, const vector<string> &real_prot, const vector<bool> &read_only, 
                               const unordered_map<int, vector<vector<pair<TSeqPos,TSeqPos> > > > &prot_feat_ranges, const int start,
                               wxWindowID id, const wxPoint &pos, const wxSize &size) 
: wxVScrolledWindow(parent,id, pos, size, wxFULL_REPAINT_ON_RESIZE|wxWANTS_CHARS), m_Seq(seq), m_SeqLen(seq_len), m_FeatRanges(feat_ranges), m_FeatTypes(feat_types), m_FeatStrand(feat_strand), 
    m_FeatFrames(feat_frames), m_GeneticCode(genetic_code), m_Feat5Partial(feat_partial5),
    m_AllowedCharSet(allowed_char_set), m_RealProt(real_prot), m_read_only(read_only), m_start(start), m_ProtFeatRanges(prot_feat_ranges),
    m_EnableTranslation(true), m_EnableTranslation1(true), m_EnableTranslation2(true), m_EnableComplement(true), m_EnableFeatures(true), m_EnableOnTheFly(true), m_EnableMismatch(true),
    m_EnableRevTranslation(true), m_EnableRevTranslation1(true), m_EnableRevTranslation2(true), m_Down(false)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    NStr::ToLower(m_AllowedCharSet);
    SetBackgroundColour(*wxWHITE);
    m_Font = wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_FontHeight = 9;
    m_FontWidth = 9;
    m_NumRows = 1;
    m_NumCols = 1; 
    m_LastRowLength = 0; 
    NStr::ToLower(m_Seq);
    m_CursorCol = 0;
    m_CursorRow = 0;
    m_CursorSeq = -1;
    m_DragMin = -1;
    m_DragMax = -1;
    m_FeatureStart = pair<int,int>(-1,-1);
    m_FeatureStop = pair<int,int>(-1,-1);
    if (!m_Seq.empty())
        m_Seq += " ";
    UpdateData();
    TranslateCDS();    
    m_EnableTranslation = false;
    m_EnableTranslation1 = false;
    m_EnableTranslation2 = false;
    m_EnableRevTranslation = false;
    m_EnableRevTranslation1 = false;
    m_EnableRevTranslation2 = false;
    m_EnableComplement = false;
    m_EnableFeatures = false;
    m_EnableOnTheFly = false;
    m_EnableMismatch = false;
    m_ShowTripletMismatch = -1; 
    m_ShowTripletTranslation = -4;
    m_Clean = true;
    m_Parent = NULL;
    wxWindow *win = GetParent();
    while (win)
    {
        CEditSequence  *base = dynamic_cast<CEditSequence *>(win);
        if (base)
            m_Parent = base;
        win = win->GetParent();
    }
    GetFeatWholeRange();
}

void CPaintSequence::InitPanel()
{
    if (!m_Seq.empty())
    {
        wxGraphicsContext *gc = wxGraphicsContext::Create();
        gc->SetFont(m_Font, *wxBLACK);
        CalculateFontWidthAndHeight(gc);
        m_NumCols = CalculateNumCols();
        m_NumRows = m_Seq.size() / m_NumCols;
    }
    SetRowCount(m_NumRows);   
}

wxCoord CPaintSequence::OnGetRowHeight( size_t row ) const
{
    int r  = 0;
    vector<unsigned int> feats_in_row = GetFeaturesInRow(row);
    map<unsigned int, vector<unsigned int> > feats_with_exons = GetFeatsWithExons(row, feats_in_row);
    DrawLabelCell(0,0,r,row,feats_in_row,feats_with_exons,NULL); 
    return r;
}

CPaintSequence::~CPaintSequence() 
{ 
}

void CPaintSequence::UpdateFeatures(const vector<vector<pair<TSeqPos,TSeqPos> > > &feat_ranges, 
                                    const vector<pair<string,objects::CSeqFeatData::ESubtype> > &feat_types,
                                    const vector<objects::CBioseq_Handle::EVectorStrand> &feat_strand,
                                    const  vector<int> &feat_frames,
                                    const vector< CRef<CGenetic_code> > &genetic_code,
                                    const vector<bool> &feat_partial5,
                                    const vector<string> &real_prot,
                                    const unordered_map<int, vector<vector<pair<TSeqPos,TSeqPos> > > > &prot_feat_ranges)
{
    m_FeatRanges = feat_ranges;
    m_FeatTypes = feat_types;
    m_FeatStrand = feat_strand;
    m_FeatFrames = feat_frames;
    m_GeneticCode = genetic_code;
    m_Feat5Partial = feat_partial5;
    m_RealProt = real_prot;
    m_ProtFeatRanges = prot_feat_ranges;
    GetFeatWholeRange();
    UpdateData();
    TranslateCDS();
}

void CPaintSequence::RefreshWithScroll()
{
    SeqPosToColRow();
    NormalizeCursorColRows();
    ScrollWithCursor();
    Refresh();
}

void CPaintSequence::EnableTranslation(bool enable)
{
    m_EnableTranslation = enable;
    if (!enable)
        m_ShowTripletTranslation = -4;
    RefreshWithScroll();
}

void CPaintSequence::EnableTranslation1(bool enable)
{
    m_EnableTranslation1 = enable;
    if (!enable)
        m_ShowTripletTranslation = -4;
    RefreshWithScroll();
}

void CPaintSequence::EnableTranslation2(bool enable)
{
    m_EnableTranslation2 = enable;
    if (!enable)
        m_ShowTripletTranslation = -4;
    RefreshWithScroll();
}

void CPaintSequence::EnableRevTranslation(bool enable)
{
    m_EnableRevTranslation = enable;
    if (!enable)
        m_ShowTripletTranslation = -4;
    RefreshWithScroll();
}

void CPaintSequence::EnableRevTranslation1(bool enable)
{
    m_EnableRevTranslation1 = enable;
    if (!enable)
        m_ShowTripletTranslation = -4;
    RefreshWithScroll();
}

void CPaintSequence::EnableRevTranslation2(bool enable)
{
    m_EnableRevTranslation2 = enable;
    if (!enable)
        m_ShowTripletTranslation = -4;
    RefreshWithScroll();
}


void CPaintSequence::EnableComplement(bool enable)
{
    m_EnableComplement = enable;
    RefreshWithScroll();
}

void CPaintSequence::EnableFeatures(bool enable)
{
    m_EnableFeatures = enable;
    RefreshWithScroll();
}

void CPaintSequence::EnableOnTheFly(bool enable)
{
    m_EnableOnTheFly = enable;
    if (!enable)
        m_ShowTripletMismatch = -1;
    RefreshWithScroll();
}

void CPaintSequence::EnableMismatch(bool enable)
{
    m_EnableMismatch = enable;
    if (!enable)
        m_ShowTripletMismatch = -1;
    RefreshWithScroll();
}

void CPaintSequence::UpdateData()
{    
    bool minus = false;
    bool plus = false;
    for (unsigned int i=0; i < m_FeatStrand.size(); i++)
        if (m_FeatStrand[i] == objects::CBioseq_Handle::eStrand_Minus)
        {
            minus = true;
        }
        else
        {
            plus = true;
        }

    if (m_EnableTranslation || (m_EnableOnTheFly && plus))
    {
        CSeqTranslator::Translate(m_Seq,m_Prot, CSeqTranslator::fRemoveTrailingX | CSeqTranslator::fIs5PrimePartial);
    }
    if (m_EnableTranslation1 || (m_EnableOnTheFly && plus))
    {
        CSeqTranslator::Translate(m_Seq.substr(1),m_Prot1, CSeqTranslator::fRemoveTrailingX | CSeqTranslator::fIs5PrimePartial);
    }
    if (m_EnableTranslation2 || (m_EnableOnTheFly && plus))
    {
        CSeqTranslator::Translate(m_Seq.substr(2),m_Prot2, CSeqTranslator::fRemoveTrailingX | CSeqTranslator::fIs5PrimePartial);
    }
    if (m_EnableComplement)
    {
        CSeqManip::Complement(m_Seq, CSeqUtil::e_Iupacna, 0, m_Seq.size(), m_Complement);         
        NStr::ToLower(m_Complement);
    }  

    string rev_comp;   
    if (m_EnableRevTranslation || m_EnableRevTranslation1 || m_EnableRevTranslation2 || (m_EnableOnTheFly && minus))
    {
        CSeqManip::ReverseComplement(m_Seq, CSeqUtil::e_Iupacna, 0, m_Seq.size(), rev_comp);   
        NStr::ToLower(rev_comp);
    }

    if (m_EnableRevTranslation || (m_EnableOnTheFly && minus))
    {
        CSeqTranslator::Translate(rev_comp.substr(1),m_RevProt, CSeqTranslator::fRemoveTrailingX | CSeqTranslator::fIs5PrimePartial);
    }

    if (m_EnableRevTranslation1 || (m_EnableOnTheFly && minus))
    {
        CSeqTranslator::Translate(rev_comp.substr(2),m_RevProt1, CSeqTranslator::fRemoveTrailingX | CSeqTranslator::fIs5PrimePartial);
    }
    
    if (m_EnableRevTranslation2 || (m_EnableOnTheFly && minus))
    {
        CSeqTranslator::Translate(rev_comp.substr(3),m_RevProt2, CSeqTranslator::fRemoveTrailingX | CSeqTranslator::fIs5PrimePartial);
    }
}

string CPaintSequence::GetSeq()
{
    return NStr::TruncateSpaces(m_Seq);
}

vector<int> &CPaintSequence::GetSeqLen()
{
    return m_SeqLen;
}

unsigned int CPaintSequence::PosToSegment()
{
    int start = 0;
    unsigned int i=0;
    while ( i < m_SeqLen.size()-1) // if not found in any segment place in the last one
    {
        if (m_CursorSeq >= start && m_CursorSeq < start+m_SeqLen[i])
            break;
        start += m_SeqLen[i];
        i++;
    }
    return i;
}

void CPaintSequence::SetClean(bool clean)
{
    m_Clean = clean;
    m_Parent->EnableCommit(!clean);    
}

bool CPaintSequence::GetClean()
{
    return m_Clean;
}

void CPaintSequence::InsertChar(int uc)
{
    if (m_CursorSeq < 0)
        ColRowToSeqPos();
    unsigned int seg = PosToSegment();
    if (m_read_only[seg])
    {
        m_Parent->ShowReadOnlyWarning();
        return;
    }
    string str1;
    if (m_CursorSeq > 0)
        str1 = m_Seq.substr(0,m_CursorSeq);
    string str2 = m_Seq.substr(m_CursorSeq);
    str1.push_back(tolower(uc));
    m_Seq = str1+str2;
    m_SeqLen[seg]++;
    m_CursorSeq++;   
    SeqPosToColRow(); 
    UpdateData();
    AdjustFeatureRange(m_CursorSeq - 1, 1);
    SetClean(false);
}

void CPaintSequence::DeleteChar()
{
    string result = CutSelection();
    if (!result.empty())
        return;
    if (m_CursorSeq < 0 || m_CursorSeq >= m_Seq.size() - 1)
        return;
    if (m_Seq.size() == 2)
    {
        wxMessageBox (_("Unable to delete the whole sequence"), _("Error"), wxOK|wxICON_ERROR);
        return;
    }   
    unsigned int seg = PosToSegment();
    if (m_read_only[seg])
    {
        m_Parent->ShowReadOnlyWarning();
        return;
    }
    string str1;
    if (m_CursorSeq > 0)
        str1 = m_Seq.substr(0,m_CursorSeq);
    string str2;
    if (m_CursorSeq < m_Seq.size()-1)
        str2 = m_Seq.substr(m_CursorSeq+1);
    m_Seq = str1+str2;
    m_SeqLen[seg]--;
    if (m_SeqLen[seg] < 0)
        m_SeqLen[seg] = 0;
    UpdateData();
    AdjustFeatureRange(m_CursorSeq, -1);
    SetClean(false);
}

void CPaintSequence::ScrollWithCursor()
{
    if (m_CursorRow < GetVisibleRowsBegin() || m_CursorRow > GetVisibleRowsEnd() - 1)
        ScrollToRow(m_CursorRow);    
}

void CPaintSequence::OnChar(wxKeyEvent& event)
{
    int uc = tolower(event.GetKeyCode());
    if ( !event.HasModifiers() && uc != WXK_NONE && uc >= 32 && uc < 255 && m_AllowedCharSet.find(uc) != string::npos)
    {
        InsertChar(uc);       
        NormalizeCursorColRows();
        ScrollWithCursor();
        Refresh();
    }
    event.Skip();   
}

void CPaintSequence::OnKeyDown(wxKeyEvent& event)
{
    wxSize sz = GetClientSize();
    int num_rows = GetVisibleRowsEnd() - GetVisibleRowsBegin() + 1 - 1;

    int uc = event.GetKeyCode();
    switch ( uc )
    {
    case WXK_LEFT  : m_CursorCol--; m_CursorSeq = -1; break;
    case WXK_RIGHT : m_CursorCol++; m_CursorSeq = -1; break;
    case WXK_UP    : m_CursorRow--; m_CursorSeq = -1; break;
    case WXK_DOWN  : m_CursorRow++; m_CursorSeq = -1; break;
    case WXK_END   : m_CursorSeq = m_Seq.size() - 1; SeqPosToColRow(); break;
    case WXK_HOME  : m_CursorSeq = 0; SeqPosToColRow(); break;
    case WXK_PAGEUP : m_CursorRow = GetVisibleRowsBegin() - num_rows; m_CursorSeq = -1; break;
    case WXK_PAGEDOWN : m_CursorRow = GetVisibleRowsBegin() + num_rows; m_CursorSeq = -1; break;
    case WXK_DELETE: 
        DeleteChar(); 
        break;
    case WXK_BACK  :
        m_CursorSeq--;
        DeleteChar();
        if (m_CursorSeq < 0)
            m_CursorSeq = 0;
        SeqPosToColRow();
        break;
    case WXK_INSERT :
        if (m_CursorSeq >= 0 && m_CursorSeq < m_Seq.size() - 1)
        {
            if (m_DragMin < 0)
                m_DragMin = m_CursorSeq;
            else if (m_DragMax < 0)
                m_DragMax = m_CursorSeq;
            else
            {
                if (abs(m_DragMin - m_CursorSeq) < abs(m_DragMax - m_CursorSeq))
                    m_DragMin = m_CursorSeq;
                else
                    m_DragMax = m_CursorSeq;
            }          
        }
        break;
    case WXK_SPACE:
        event.Skip();
        break;
    default: 
        event.Skip(); 
        return;
        break;
    }

    NormalizeCursorColRows();
    ScrollWithCursor();
    Refresh();
}

void CPaintSequence::NormalizeCursorColRows()
{
    if (m_CursorCol < 0)
        m_CursorCol = 0;
    if (m_CursorCol >= m_NumCols)
        m_CursorCol = m_NumCols-1;
    if (m_CursorRow < 0)
        m_CursorRow = 0;
    if (m_CursorRow >= m_NumRows)
        m_CursorRow = m_NumRows-1;
    if (m_CursorRow == m_NumRows-1 && m_CursorCol >= m_LastRowLength)
        m_CursorCol = m_LastRowLength-1;
}

int CPaintSequence::LeftMarginWidth()
{
    return (5 + chars_per_group * m_FontWidth + 5); 
}

int CPaintSequence::FindRowByCoord(int y, int &y_row)
{
    y_row = TopMarginHeight();
    if (y <= 0)
        return 0;
    int row =  GetVisibleRowsBegin();   
    int row_height = OnGetRowHeight(row);
    while ( y_row < y && row <= GetVisibleRowsEnd()+1  )
    {
        y_row += row_height;
        row++;
        row_height = OnGetRowHeight(row);
    }
    row--;
    y_row -= OnGetRowHeight(row);
    return row;
}

bool CPaintSequence::MouseToSeqPos(wxPoint p, int &row, int &y_row)
{
    bool found = false;
    row = FindRowByCoord(p.y, y_row);    
    int col = (p.x - LeftMarginWidth()) / m_FontWidth; 
    if (col < 0)
        return false;
    int num_groups = col / (chars_per_group + space_between_groups); 
    int pos_in_group = col % (chars_per_group + space_between_groups);
    if (pos_in_group < chars_per_group)
    {        
        m_CursorCol = num_groups * chars_per_group + pos_in_group;
        m_CursorRow = row;        
        ColRowToSeqPos();
        found = true;
    }
    return found;
}

void CPaintSequence::MouseToFeature(wxPoint p, int row, int y0)
{ 
    if (!m_EnableFeatures)
        return;
    vector<unsigned int> feats_in_row = GetFeaturesInRow(row);
    map<unsigned int, vector<unsigned int> > feats_with_exons = GetFeatsWithExons(row, feats_in_row);
    DrawLabelCell(0,0,y0,row,feats_in_row,feats_with_exons,NULL);
    int y1 = 0;
    DrawFeatureLabels( row, 0, y1, feats_in_row, feats_with_exons, NULL); 
    y0 -= y1;
    y0 -= 5;

    for (unsigned int k = 0; k < feats_in_row.size(); k++) 
    {
        unsigned int i = feats_in_row[k];
        auto it = feats_with_exons.find(i);
   
        bool found = false;
        if (it != feats_with_exons.end())
        {
            for (unsigned int m = 0; m < it->second.size(); m++) 
            {
                unsigned int j = it->second[m];
                TSeqPos start = m_FeatRanges[i][j].first;
                TSeqPos stop = m_FeatRanges[i][j].second;
                if (start == numeric_limits<int>::max() || stop == numeric_limits<int>::max())
                    continue;
                if (p.y >= y0 && p.y < y0 + m_FontHeight)
                {
                    if (m_CursorSeq == start || m_CursorSeq == start-1 || m_CursorSeq == start+1)
                    {
                        m_FeatureStart = pair<int,int>(i,j);
                        m_FeatureStop = pair<int,int>(-1,-1);
                        found = true;
                        break;
                    }
                    if (m_CursorSeq == stop || m_CursorSeq == stop-1 || m_CursorSeq == stop+1)
                    {
                        m_FeatureStart = pair<int,int>(-1,-1);
                        m_FeatureStop = pair<int,int>(i,j);
                        found = true;
                        break;
                    }
                }                                 
            }
        }
        y0 += m_FontHeight;
        bool is_exon_present = (it != feats_with_exons.end() && m_FeatTypes[i].second == CSeqFeatData::eSubtype_cdregion);
        DrawMismatchLabel(0, y0, is_exon_present, NULL); 
        DrawOnTheFlyLabel(0, y0, is_exon_present, NULL);
        if (found)
            break;
    } 
}

void CPaintSequence::OnMouseClick(wxMouseEvent& evt)
{      
    if (!m_Down)
    {
        evt.Skip();
        return;
    }
    m_Down = false;
    if (evt.GetModifiers() == wxMOD_SHIFT)
    {
        wxPoint p = evt.GetPosition();
        //wxPoint p = this->ScreenToClient(wxGetMousePosition());
        int y_row;
        int current_row;
        bool found = MouseToSeqPos(p, current_row, y_row);
        if (m_CursorSeq >= 0 && m_CursorSeq < m_Seq.size() - 1)
        {
            if (m_DragMin < 0)
                m_DragMin = m_CursorSeq;
            else if (m_DragMax < 0)
                m_DragMax = m_CursorSeq;
            else
            {
                if (abs(m_DragMin - m_CursorSeq) < abs(m_DragMax - m_CursorSeq))
                    m_DragMin = m_CursorSeq;
                else
                    m_DragMax = m_CursorSeq;
            }          
        }
        NormalizeCursorColRows();
        ScrollWithCursor();
        Refresh();
        return;
    }
    if (m_FeatureStart.first >= 0 || m_FeatureStop.first >= 0)
    {
        TranslateCDS();
    }
    m_FeatureStart = pair<int,int>(-1,-1);
    m_FeatureStop = pair<int,int>(-1,-1);
    wxPoint p = evt.GetPosition();
    //wxPoint p = this->ScreenToClient(wxGetMousePosition());
    int y_row;
    int current_row;
    bool found = MouseToSeqPos(p, current_row, y_row);
    if (found )
    {
        if (m_EnableTranslation || m_EnableTranslation1 || m_EnableTranslation2 || m_EnableOnTheFly || m_EnableMismatch ||
            m_EnableRevTranslation || m_EnableRevTranslation1 || m_EnableRevTranslation2) 
        {
            int current = p.y - y_row;
            int y = m_FontHeight;
            y +=  m_FontHeight; // top line numbers
            y += 16;
            DrawComplementLabel(current_row,0,y,NULL);
           
            int y_trial = y;
            DrawTranslationLabels(current_row,0,y_trial,NULL);
            if (m_ShowTripletTranslation > -4)
            {
                if ( current > y && current < y_trial)
                    m_ShowTripletTranslation = -4;
            }
            else
            {
                if (m_EnableTranslation)
                {
                    if ( current > y && current < y+m_FontHeight)
                    {
                        m_ShowTripletTranslation = 0;
                        m_ShowTripletMismatch = -1;
                    }               
                    y += m_FontHeight;
                }    
                if (m_EnableTranslation1)
                {
                    if ( current > y && current < y+m_FontHeight)
                    {
                        m_ShowTripletTranslation = 1;
                        m_ShowTripletMismatch = -1;
                    }       
                    y += m_FontHeight;
                }    
                if (m_EnableTranslation2)
                {
                    if ( current > y && current < y+m_FontHeight)
                    {
                        m_ShowTripletTranslation = 2;
                        m_ShowTripletMismatch = -1;
                    }      
                    y += m_FontHeight;
                }               
                if (m_EnableRevTranslation)
                {
                    if ( current > y && current < y+m_FontHeight)
                    {
                        m_ShowTripletTranslation = -1;
                        m_ShowTripletMismatch = -1;
                    }               
                    y += m_FontHeight;
                }    
                if (m_EnableRevTranslation1)
                {
                    if ( current > y && current < y+m_FontHeight)
                    {
                        m_ShowTripletTranslation = -2;
                        m_ShowTripletMismatch = -1;
                    }       
                    y += m_FontHeight;
                }    
                if (m_EnableRevTranslation2)
                {
                    if ( current > y && current < y+m_FontHeight)
                    {
                        m_ShowTripletTranslation = -3;
                        m_ShowTripletMismatch = -1;
                    }      
                    y += m_FontHeight;
                }               
            }


            {    
                vector<unsigned int> feats_in_row = GetFeaturesInRow(current_row);
                map<unsigned int, vector<unsigned int> > feats_with_exons = GetFeatsWithExons(current_row, feats_in_row);
                y = y_trial;
                for (size_t k = 0; k < feats_in_row.size(); k++)
                {
                    if (m_EnableFeatures)
                        y += m_FontHeight;
                    unsigned int i = feats_in_row[k];
                    int y_next = y;
                    bool is_exon_present = (feats_with_exons.find(i) != feats_with_exons.end()) && (m_FeatTypes[i].second == CSeqFeatData::eSubtype_cdregion);
                    DrawMismatchLabel(0,y_next,is_exon_present,NULL);  
                    DrawOnTheFlyLabel(0,y_next,is_exon_present,NULL);
                    if ( current > y && current < y_next)
                    {
                        if (m_ShowTripletMismatch < 0)
                        {
                            m_ShowTripletMismatch = i;
                            m_ShowTripletTranslation = -4;
                        }
                        else
                            m_ShowTripletMismatch = -1;
                    }
                    y = y_next;
                }
            }

        }
        Refresh();
    }
    evt.Skip();
}

void CPaintSequence::OnMouseDrag(wxMouseEvent& evt)
{
    if (m_Down && evt.Dragging()) // && evt.GetModifiers() != wxMOD_SHIFT
    {
        int row, y_row;   
        //wxPoint p = this->ScreenToClient(wxGetMousePosition());
        wxPoint p = evt.GetPosition();
        bool found = MouseToSeqPos(p, row, y_row);        
        if (found )
        {
            if (m_FeatureStart.first >= 0 && m_CursorSeq < m_Seq.size() - 1 && m_CursorSeq >= 0)
            {
                if (m_FeatWholeRange[m_FeatureStart.first].first == m_FeatRanges[m_FeatureStart.first][m_FeatureStart.second].first)
                    m_FeatWholeRange[m_FeatureStart.first].first = m_CursorSeq;
                m_FeatRanges[m_FeatureStart.first][m_FeatureStart.second].first = m_CursorSeq;
                SetClean(false);
            }
            else if (m_FeatureStop.first >= 0 && m_CursorSeq < m_Seq.size() - 1 && m_CursorSeq >= 0)
            {
                if (m_FeatWholeRange[m_FeatureStop.first].second == m_FeatRanges[m_FeatureStop.first][m_FeatureStop.second].second)
                    m_FeatWholeRange[m_FeatureStop.first].second = m_CursorSeq;
                m_FeatRanges[m_FeatureStop.first][m_FeatureStop.second].second = m_CursorSeq;
                SetClean(false);
            }
            else 
            {
                if ( m_DragMin < 0)
                    m_DragMin = m_CursorSeq;
                else
                    m_DragMax = m_CursorSeq;
            }
            
            
            if (m_DragMin >= 0 && m_DragMin > m_Seq.size() - 2)
                m_DragMin = m_Seq.size() - 2;
            if (m_DragMax >= 0 && m_DragMax > m_Seq.size() - 2)
                m_DragMax = m_Seq.size() - 2;
           
            Refresh();
        }
    }
    evt.Skip();
}

void CPaintSequence::OnMouseDown(wxMouseEvent& evt)
{    
    m_Down = true;
    int row, y_row;
    //wxPoint p = this->ScreenToClient(wxGetMousePosition());
    wxPoint p = evt.GetPosition();
    bool found = MouseToSeqPos(p, row, y_row);
    if (found )
    {
        MouseToFeature(p, row, y_row);        
    }
    if (m_DragMin >= 0 && m_DragMax >= 0 && evt.GetModifiers() != wxMOD_SHIFT)
    {
        m_DragMin = -1;
        m_DragMax = -1;
        Refresh();
    }
    evt.Skip();
}

void CPaintSequence::SeqPosToColRow()
{
    if (m_CursorSeq < 0)
        return;
    m_CursorRow = m_CursorSeq / m_NumCols;
    m_CursorCol = m_CursorSeq % m_NumCols;   
}

void CPaintSequence::ColRowToSeqPos()
{
    m_CursorSeq = m_CursorRow * m_NumCols + m_CursorCol;   
}

void CPaintSequence::SetPos(int pos)
{
    m_CursorSeq = pos - 1;
    if (m_CursorSeq < 0)
        m_CursorSeq = 0;
    if (m_CursorSeq >= m_Seq.size())
        m_CursorSeq = m_Seq.size() - 1;
    SeqPosToColRow();
    ScrollWithCursor();
    Refresh();
}

void CPaintSequence::SetRange(int pos1, int pos2)
{
    m_DragMin = pos1 - 1;
    m_DragMax = pos2 - 1;
    m_CursorSeq = pos1 - 1;
    if (m_CursorSeq < 0)
        m_CursorSeq = 0;
    if (m_CursorSeq >= m_Seq.size())
        m_CursorSeq = m_Seq.size() - 1;
    SeqPosToColRow();
    ScrollWithCursor();
    Refresh();
}

void CPaintSequence::Search(const string &val)
{
    int pos = NStr::FindNoCase(m_Seq,val,m_CursorSeq);
    if (pos == m_CursorSeq && m_CursorSeq+val.size() < m_Seq.size()-1)
        pos = NStr::FindNoCase(m_Seq,val,m_CursorSeq+val.size());
    if (pos == NPOS)
        return;
    m_CursorSeq = pos;
    if (m_CursorSeq >= m_Seq.size())
        m_CursorSeq = m_Seq.size() - 1;
    SeqPosToColRow();
    ScrollWithCursor();
    Refresh();
}

void CPaintSequence::SetStartPos()
{
  if (IsShownOnScreen())
    {
      m_CursorSeq = m_start - 1;
      m_start = 0;
      m_NumCols = CalculateNumCols();
      SeqPosToColRow();
      ScrollWithCursor();
      CEditSequence *parent = dynamic_cast<CEditSequence*>(GetParent()->GetParent());
      if (parent)
	{
	  if ( m_CursorSeq >=0 )
	    {
	      parent->ReportPos(m_CursorSeq+1);
	    }
	}
    }
}

/// Painting
void CPaintSequence::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    wxGraphicsContext *gc = wxGraphicsContext::Create( dc );
    if (gc && !m_Seq.empty())
    {
        ClearScreen(gc);
        gc->SetFont(m_Font, *wxBLACK);
        m_NumCols = CalculateNumCols();
        m_NumRows = m_Seq.size() / m_NumCols;
        m_LastRowLength = m_Seq.size() % m_NumCols;
        if (m_LastRowLength > 0)
            m_NumRows++;
        SetRowCount(m_NumRows);
        if (m_CursorSeq >= 0)
            SeqPosToColRow();
        else
            ColRowToSeqPos();
        int hidden_rows = GetVisibleRowsBegin();
        unsigned int start= hidden_rows * m_NumCols;
        unsigned int row = hidden_rows;
        int client_y = GetClientSize().y;
        unsigned int seq_pos = start;
        int y = TopMarginHeight();
        while (start < m_Seq.size())
        {
            string substr = m_Seq.substr(start,m_NumCols);            
            DrawTextLine(substr, y, row, gc, seq_pos);
            if (y > client_y)
                break;
            start += m_NumCols;
            row++;
        }    
        delete gc;
    }

    CEditSequence *parent = dynamic_cast<CEditSequence*>(GetParent()->GetParent());
    if (parent)
    {
        int pos1 = m_DragMin;
        int pos2 = m_DragMax;

        if ( m_CursorSeq >=0 )
        {
            parent->ReportPos(m_CursorSeq+1);
            set<int>::iterator pos = m_highlights.find(m_CursorSeq);
            if ( pos != m_highlights.end())
            {
                pos1 = *pos;
                if (pos != m_highlights.begin())
                {
                    pos--;
                    while ((*pos) + 1 == pos1)
                    {
                        pos1 = *pos;
                        if (pos == m_highlights.begin())
                            break;
                        pos--;
                    }
                }
                if (*pos != pos1)
                    pos++;
                pos2 = *pos;
                pos++;
                if (pos != m_highlights.end())
                {
                    while ((*pos) - 1 == pos2)
                    {
                        pos2 = *pos;                       
                        pos++;
                        if (pos == m_highlights.end())
                            break;
                    }             
                }   
            }
        }
        if (pos1 > pos2)
            swap(pos1, pos2);
        parent->ReportRange(pos1+1, pos2+1);
    }
    if (m_start > 0)
    {
        CallAfter(&CPaintSequence::SetStartPos);
    }
}


// Empty implementation, to prevent flicker
void CPaintSequence::OnEraseBackground(wxEraseEvent& event)
{
}

////////////////////////////////////////////////////////////
/// Notification for the derived class that moment is good
/// for doing its update and drawing stuff
////////////////////////////////////////////////////////////
void CPaintSequence::OnUpdate()
{

}

void CPaintSequence::OnResize(wxSizeEvent& event)
{

}

void CPaintSequence::ClearScreen(wxGraphicsContext *gc)
{
    wxColour backgroundColour = GetBackgroundColour();
    if (!backgroundColour.Ok())
        backgroundColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);    
    wxSize sz = GetClientSize();
    wxRect windowRect(wxPoint(0,0), sz);    
    gc->SetBrush(wxBrush(backgroundColour));
    gc->SetPen(wxPen(backgroundColour, 1));
    gc->DrawRectangle(windowRect.GetX(),windowRect.GetY(),windowRect.GetWidth(),windowRect.GetHeight());
}

void CPaintSequence::CalculateFontWidthAndHeight(wxGraphicsContext *gc)
{
    wxDouble width;
    wxDouble height;
    wxDouble descent;
    wxDouble externalLeading;
    if (m_Seq.size() > 1000)
    {
        gc->GetTextExtent(wxString(m_Seq.substr(0,1000)), &width, &height, &descent, &externalLeading);
        width /= 1000;
    }
    else
    {
        gc->GetTextExtent(wxString(m_Seq), &width, &height, &descent, &externalLeading);
        width /= m_Seq.size();
    }
    width = ceil(1+width);
    height = ceil(2+height);
    m_FontHeight = height;
    m_FontWidth = width;
}

int CPaintSequence::CalculateNumCols()
{
    int width = int(GetClientSize().x);    
    width -= LeftMarginWidth();       
    int line_length = width / m_FontWidth;
    int num_groups = line_length / (chars_per_group + space_between_groups); 
    if (num_groups == 0)
        num_groups = 1;
    if (num_groups > 1 && width - (chars_per_group + space_between_groups) * num_groups * m_FontWidth < space_between_groups * m_FontWidth) // TODO verify
        num_groups--;
    line_length = chars_per_group * num_groups;
    return line_length;
}

void CPaintSequence::DrawTextLine(const string & substr, int &y_label, int row, wxGraphicsContext *gc, unsigned int &seq_pos)
{
    int x = 5;
    m_ContinueFeatures = false;
    unsigned int orig_pos = seq_pos;
    int orig_y = y_label;
    vector<unsigned int> feats_in_row = GetFeaturesInRow(row); 
    map<unsigned int, vector<unsigned int> > feats_with_exons = GetFeatsWithExons(row, feats_in_row);
    DrawLabelCell(orig_pos,x,y_label,row, feats_in_row, feats_with_exons, gc);
    x += LeftMarginWidth() - 5;
    for (unsigned int i=0; i<substr.size(); i++)
    {
        int y = orig_y;
        DrawTextCell(wxString(substr[i]),i,row,seq_pos,x,y, feats_in_row, feats_with_exons, gc);
        seq_pos++;
        x += m_FontWidth;
        if (i % chars_per_group == chars_per_group - 1)
        {
            x += space_between_groups * m_FontWidth; 
            m_ContinueFeatures = true;
        }
        else
        {
            m_ContinueFeatures = false;
        }
    }
    x = LeftMarginWidth() + (m_NumCols / chars_per_group)* (chars_per_group + space_between_groups) * m_FontWidth;
    //DrawLineNumber(orig_pos+m_NumCols-1, x, orig_y, gc);
}

void CPaintSequence::DrawLabelCell(unsigned int pos, int x, int &y, int row, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const
{
    y +=  m_FontHeight; // top line numbers
    y += 16;
    DrawLineNumber(pos, x, y, gc);
    DrawComplementLabel(row, x, y, gc);   
    DrawTranslationLabels(row, x, y,gc);
    DrawFeatureLabels(row, x, y, feats_in_row, feats_with_exons, gc);
    y += 5;
}

void CPaintSequence::DrawTextCell(const wxString &substr, int col, int row, unsigned int seq_pos, int x, int &y, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const
{
    if (gc)
    {
        wxString label;
        label << seq_pos + 1 + (chars_per_group - ((col + 1) % chars_per_group));
        if ((col + label.Length()) % chars_per_group == 0 )
        {
            gc->SetFont(m_Font, wxColour(255,0,255));
            gc->DrawText(label,x,y);  // TODO
            gc->SetFont(m_Font,*wxBLACK);
        }
    }
    y +=  m_FontHeight;
    if (gc)
    {
        gc->SetPen(wxPen(wxColour(255,0,255)));
        if ( (col +1 ) % chars_per_group  == 0)
            gc->StrokeLine(x+m_FontWidth/2, y, x+m_FontWidth/2, y+16);
        else if ( (col + 1) % chars_per_group == chars_per_group / 2)
            gc->StrokeLine(x+m_FontWidth/2, y+8, x+m_FontWidth/2, y+16);
    }
    y += 16;
    if (gc)
    {
        // printing actual sequence
        if (m_highlights.find(seq_pos) != m_highlights.end())
        {
            gc->SetFont(m_Font, wxColour(237,222,14));
        }
        else
        {
            gc->SetFont(m_Font,*wxBLACK);
        }
        int pos1 = m_DragMin;
        int pos2 = m_DragMax;
        if (pos1 > pos2)
            swap(pos1, pos2);
        if (pos1 >= 0 && pos2 >= 0 && seq_pos >= pos1 && seq_pos <= pos2)
            gc->DrawText(substr,x,y, gc->CreateBrush(*wxLIGHT_GREY_BRUSH));
        else
            gc->DrawText(substr,x,y); 
        gc->SetFont(m_Font,*wxBLACK);
        if (m_ShowTripletTranslation > -4)
        {
            gc->SetPen( *wxGREY_PEN);

            if (m_ShowTripletTranslation >= 0 && seq_pos % 3 == m_ShowTripletTranslation)
            {
                gc->StrokeLine(x, y+m_FontHeight/2, x, y+m_FontHeight);  
            }
            else if (m_ShowTripletTranslation < 0 && (m_Seq.size() - 2 - seq_pos) % 3 == -1 - m_ShowTripletTranslation && seq_pos < m_Seq.size()-1)
            {
                gc->StrokeLine(x+m_FontWidth, y+m_FontHeight/2, x+m_FontWidth, y+m_FontHeight);  
            }
        }
    }
    int y_base = y;
    y +=  m_FontHeight;
    DrawCursor(col,row,x,y,gc);
    DrawComplement(x,y,seq_pos,gc);
    DrawTranslation(x,y,seq_pos,gc);
    DrawFeatures(row,seq_pos,x,y,y_base,feats_in_row,feats_with_exons,gc);
    y += 5;
}

void CPaintSequence::DrawCursor(int col, int row, int x, int &y, wxGraphicsContext *gc) const
{
    if (col == m_CursorCol && row == m_CursorRow && gc) 
    {
        gc->SetPen( *wxRED_PEN);
        gc->StrokeLine(x,y,x+m_FontWidth,y);                   
    }
    //y += 1;
}

void CPaintSequence::DrawTranslation(int x, int &y, unsigned int seq_pos, wxGraphicsContext *gc) const
{
    if (m_EnableTranslation)
    {
        int offset = 0;
        DrawOffsetTranslation(x,y,seq_pos,offset,m_Prot,gc);
        y += m_FontHeight;
    }    
    if (m_EnableTranslation1)
    {
        int offset = 1;
        DrawOffsetTranslation(x,y,seq_pos,offset,m_Prot1,gc);
        y += m_FontHeight;
    }    
    if (m_EnableTranslation2)
    {
        int offset = 2;
        DrawOffsetTranslation(x,y,seq_pos,offset,m_Prot2,gc);
        y += m_FontHeight;
    }    
   if (m_EnableRevTranslation)
    {
        int offset = 0;
        DrawOffsetTranslation(x,y,m_Seq.size()-2-seq_pos,offset,m_RevProt,gc);
        y += m_FontHeight;
    }    
    if (m_EnableRevTranslation1)
    {
        int offset = 1;
        DrawOffsetTranslation(x,y,m_Seq.size()-2-seq_pos,offset,m_RevProt1,gc);
        y += m_FontHeight;
    }    
    if (m_EnableRevTranslation2)
    {
        int offset = 2;
        DrawOffsetTranslation(x,y,m_Seq.size()-2-seq_pos,offset,m_RevProt2,gc);
        y += m_FontHeight;
    }    
}

void CPaintSequence::DrawOffsetTranslation(int x, int y, unsigned int seq_pos, int offset, const string &prot, wxGraphicsContext *gc) const
{
  
    if ( seq_pos >= offset+1  && (seq_pos - 1 - offset) % 3 == 0 )
        {
            int prot_pos = (seq_pos - 1 - offset) / 3 ;
            if (gc && prot_pos < prot.size() && prot_pos >= 0)
                gc->DrawText (wxString(prot[prot_pos]),x,y);
        }
    
}

char CPaintSequence::TranslateOnTheFly(unsigned int seq_pos, int i, const vector<unsigned int> &ranges, const vector<string> & translation, bool &left, bool &right) const 
{
    char prot = 0;
    left = false;
    right = false;
    if (m_FeatStrand[i] == objects::CBioseq_Handle::eStrand_Plus)
        for (unsigned int m = 0; m < ranges.size(); m++) 
        {
            unsigned int j = ranges[m];
            TSeqPos start = m_FeatRanges[i][j].first;
            TSeqPos stop = m_FeatRanges[i][j].second;
            if (seq_pos >= start && seq_pos <= stop )
            {
                int prot_pos = m_FeatLengthBefore[i][j] + seq_pos - start - m_FeatFrames[i];
                if (prot_pos >= 0)
                {
                    if ( prot_pos % 3 == 1)
                    {
                        if (i < translation.size() && prot_pos / 3 < translation[i].size())
                            prot = translation[i][prot_pos / 3];                  
                    }
                    else if (prot_pos % 3 == 0)
                        left = true;
                    else if (prot_pos % 3 == 2)
                        right = true;
                    break;
                }
            }
        }
    if (m_FeatStrand[i] == objects::CBioseq_Handle::eStrand_Minus)
        for (int m = ranges.size() - 1; m >= 0; m--) 
        {
            unsigned int j = ranges[m];
            TSeqPos start = m_FeatRanges[i][j].first;
            TSeqPos stop = m_FeatRanges[i][j].second;
            if (seq_pos >= start && seq_pos <= stop )
            {
                int prot_pos = m_FeatTotalLength[i] - m_FeatLengthBefore[i][j] - (stop - start + 1) + stop - seq_pos - m_FeatFrames[i];
                if (prot_pos >= 0)
                {
                    if ( prot_pos % 3 == 1)
                    {
                        if (i < translation.size() && prot_pos / 3 < translation[i].size())
                            prot = translation[i][prot_pos / 3];
                    }
                    else if (prot_pos % 3 == 0)
                        right = true;
                    else if (prot_pos % 3 == 2)
                        left = true;
                    break;              
                }                    
            }
        }
    
    return prot;
}

void CPaintSequence::DrawTripletMismatch(int x, int y, int y_base, int i, wxGraphicsContext *gc) const
{
    if (m_ShowTripletMismatch == i)
    {
        gc->SetPen( *wxGREY_PEN);       
        gc->StrokeLine(x, y, x, y+m_FontHeight);  
        gc->StrokeLine(x, y_base, x, y_base+m_FontHeight);  
    }
}

void CPaintSequence::DrawOnTheFly(int x, int &y, int y_base, unsigned int seq_pos, int i, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const
{
    map<unsigned int, vector<unsigned int> >::const_iterator f = feats_with_exons.find(i);
    bool is_exon_present = (f != feats_with_exons.end()) && (m_FeatTypes[i].second == CSeqFeatData::eSubtype_cdregion);
    if (m_EnableOnTheFly &&  is_exon_present) 
    {
        bool left, right;
        char prot = TranslateOnTheFly(seq_pos, i, f->second, m_Translated, left, right);
        if (gc)
        {
            if (prot != 0)
            {
                gc->SetFont(m_Font, *wxBLUE);
                gc->DrawText (wxString(prot),x,y);
                gc->SetFont(m_Font,*wxBLACK);
            }
            if (left)
                DrawTripletMismatch(x,y,y_base,i,gc);
            if (right)
                DrawTripletMismatch(x+m_FontWidth,y,y_base,i,gc);
        }
        y += m_FontHeight;
    }
}


void CPaintSequence::DrawMismatch(int x, int &y, int y_base, unsigned int seq_pos, int i, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const
{
    map<unsigned int, vector<unsigned int> >::const_iterator f = feats_with_exons.find(i);
    bool is_exon_present = (f != feats_with_exons.end()) && (m_FeatTypes[i].second == CSeqFeatData::eSubtype_cdregion);
    if (m_EnableMismatch && is_exon_present) 
    {
        bool left, right;
        char translated_prot = TranslateOnTheFly(seq_pos, i, f->second, m_Translated, left, right);
        char prot = TranslateOnTheFly(seq_pos, i, f->second, m_RealProt, left, right);
        if (gc)
        {
            if (prot != 0)
            {
                if (prot != translated_prot)
                    gc->SetFont(m_Font,*wxRED);
                else
                    gc->SetFont(m_Font, *wxBLUE);
                gc->DrawText(wxString(prot),x,y);
                gc->SetFont(m_Font,*wxBLACK);
            }
            if (left)
                DrawTripletMismatch(x,y,y_base,i,gc);
            if (right)
                DrawTripletMismatch(x+m_FontWidth,y,y_base,i,gc);
        }
        y += m_FontHeight;
    }
}


void CPaintSequence::DrawMismatchLabel(int x, int &y, bool is_exon_present, wxGraphicsContext *gc) const
{
    if (m_EnableMismatch &&  is_exon_present) 
    {
        y += m_FontHeight;
    }
}

void CPaintSequence::DrawComplement(int x, int &y, unsigned int seq_pos, wxGraphicsContext *gc) const
{
    if (m_EnableComplement)
    {
        if (gc && seq_pos < m_Complement.size())
            gc->DrawText (wxString(m_Complement[seq_pos]),x,y);
        y += m_FontHeight;
    }    
}

const wxPen* CPaintSequence::GetColorForFeature(objects::CSeqFeatData::ESubtype subtype) const
{
    const wxPen* pen = wxBLACK_PEN;
    switch(subtype)
    {
    case CSeqFeatData::eSubtype_cdregion : pen = wxBLUE_PEN; break;
    default                              : pen = wxBLACK_PEN; break;
    }
    return pen;
}

void CPaintSequence::DrawLineNumber(unsigned int seq_pos, int x, int &y, wxGraphicsContext *gc) const
{
    wxString label;
    label << seq_pos+1;
    if (gc)
    {
        gc->SetFont(m_Font, wxColour(255,0,255));
        gc->DrawText(label,x,y);  
        gc->SetFont(m_Font,*wxBLACK);
    }
    y += m_FontHeight; 
}


void CPaintSequence::DrawFeatureLabels(int row, int x, int &y, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const
{ 
    for (unsigned int k = 0; k < feats_in_row.size(); k++) 
    {
        unsigned int i = feats_in_row[k];
        if (m_EnableFeatures)
        {    
            string label = m_FeatTypes[i].first.substr(0,10); // only take first 10 characters of the label
            if (gc)
            {
                if (m_FeatTypes[i].second == CSeqFeatData::eSubtype_cdregion)
                    gc->SetFont(m_Font,*wxBLUE);
                gc->DrawText(wxString(label),x,y);
                gc->SetFont(m_Font,*wxBLACK);
            }
            y +=  m_FontHeight;                
        }
        bool is_exon_present = (feats_with_exons.find(i) != feats_with_exons.end()) && (m_FeatTypes[i].second == CSeqFeatData::eSubtype_cdregion);
        DrawMismatchLabel(x, y, is_exon_present, gc); 
        DrawOnTheFlyLabel(x, y, is_exon_present, gc);
    }    
}

vector<unsigned int> CPaintSequence::GetFeaturesInRow(int row) const 
{
    vector<unsigned int> feats_in_row;
    
    for (size_t i = 0; i < m_FeatWholeRange.size(); i++)
    {
        TSeqPos start = m_FeatWholeRange[i].first;
        TSeqPos stop = m_FeatWholeRange[i].second;
        int row_start = row * m_NumCols;
        int row_end = row_start + m_NumCols - 1;
        if ( (row_start >= start && row_end <= stop) ||
             (start >= row_start && start <= row_end) ||
             (stop >= row_start && stop <= row_end) )
        {
                feats_in_row.push_back(i);
        }
        
    }
    return feats_in_row;
}

void CPaintSequence::DrawComplementLabel(int row, int x, int &y, wxGraphicsContext *gc) const
{ 
    if (m_EnableComplement)
    {
        if (gc)
        {
            gc->DrawText(_("complement"),x,y);
        }
        y += m_FontHeight;
    }
}

void CPaintSequence::DrawOnTheFlyLabel(int x, int &y, bool is_exon_present, wxGraphicsContext *gc) const
{
    if (m_EnableOnTheFly && is_exon_present) 
    {        
        if (gc)
        {
            gc->SetFont(m_Font, *wxBLUE);
            gc->DrawText (_("on-the-fly"),x,y);
            gc->SetFont(m_Font, *wxBLACK);
        }
        y += m_FontHeight;
    }
}

void CPaintSequence::DrawTranslationLabels(int row, int x, int &y, wxGraphicsContext *gc) const
{  
    if (m_EnableTranslation)
    {
        if (gc)
            gc->DrawText (_("frame +1"),x,y);
        y += m_FontHeight;
    }    
    if (m_EnableTranslation1)
    {
        if (gc)
            gc->DrawText (_("frame +2"),x,y);
        y += m_FontHeight;
    }    
    if (m_EnableTranslation2)
    {
        if (gc)
            gc->DrawText (_("frame +3"),x,y);
        y += m_FontHeight;
        }    
    if (m_EnableRevTranslation)
    {
        if (gc)
            gc->DrawText (_("frame -1"),x,y);
            y += m_FontHeight;
    }    
    if (m_EnableRevTranslation1)
    {
        if (gc)
            gc->DrawText (_("frame -2"),x,y);
        y += m_FontHeight;
        }    
    if (m_EnableRevTranslation2)
    {
        if (gc)
            gc->DrawText (_("frame -3"),x,y);
        y += m_FontHeight;
    }    
}    

void CPaintSequence::DrawFeatures(int row, unsigned int seq_pos, int x, int &y, int y_base, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const
{
    for (unsigned int k = 0; k < feats_in_row.size(); k++) 
    {
        unsigned int i = feats_in_row[k];
        map<unsigned int, vector<unsigned int> >::const_iterator f = feats_with_exons.find(i);

        if (m_EnableFeatures)
        {
            bool found = false;
            bool startpoint = false;
            bool endpoint = false;
            
            if (f != feats_with_exons.end())
            {
                for (unsigned int m = 0; m < f->second.size(); m++) 
                {
                    unsigned int j = f->second[m];
                    TSeqPos start = m_FeatRanges[i][j].first;
                    TSeqPos stop = m_FeatRanges[i][j].second;
                    
                    if (seq_pos >= start && seq_pos <= stop)
                    {
                        found = true; 
                        if (seq_pos == start)
                            startpoint = true;
                        if (seq_pos == stop)
                            endpoint = true;
                        break;
                    }
                }
            }


            if (gc && seq_pos >= m_FeatWholeRange[i].first && seq_pos <= m_FeatWholeRange[i].second)
            {
                    gc->SetPen( *GetColorForFeature(m_FeatTypes[i].second));
                    gc->SetBrush(*wxBLACK_BRUSH);
                    gc->StrokeLine(x, y+m_FontHeight/2, x+m_FontWidth, y+m_FontHeight/2);     
                    if (found)
                        gc->StrokeLine(x, y+m_FontHeight/2+1, x+m_FontWidth, y+m_FontHeight/2+1);
                    if (m_ContinueFeatures && !startpoint)
                    {
                        gc->StrokeLine(x - space_between_groups * m_FontWidth, y+m_FontHeight/2, x ,y+m_FontHeight/2);
                        if (found)
                            gc->StrokeLine(x - space_between_groups * m_FontWidth, y+m_FontHeight/2+1, x ,y+m_FontHeight/2+1);
                    }
                    if (m_FeatStrand[i] == objects::CBioseq_Handle::eStrand_Plus)
                    {
                        if (startpoint)
                            gc->DrawRectangle(x, y+m_FontHeight/2-3, 6, 6);   
                        if (endpoint)
                        {                       
                            wxPoint2DDouble lines[] = {wxPoint2DDouble(x+m_FontWidth-4, y-4+m_FontHeight/2), wxPoint2DDouble(x+m_FontWidth-4, y+4+m_FontHeight/2), wxPoint2DDouble(x+m_FontWidth,y+m_FontHeight/2)};
                            gc->DrawLines(3,lines);  
                        }
                    }
                    else
                    {
                        if (startpoint)
                        {
                            wxPoint2DDouble lines[] = {wxPoint2DDouble(x+4, y-4+m_FontHeight/2), wxPoint2DDouble(x+4, y+4+m_FontHeight/2), wxPoint2DDouble(x,y+m_FontHeight/2)};
                            gc->DrawLines(3,lines);  
                        }
                        if (endpoint)
                            gc->DrawRectangle(x+m_FontWidth-6, y+m_FontHeight/2-3, 6, 6);   
                    }                
            }
            y += m_FontHeight;            
        }

        DrawMismatch(x,y,y_base,seq_pos,i,feats_with_exons,gc); 
        DrawOnTheFly(x,y,y_base,seq_pos,i,feats_with_exons,gc); 
    }    
}

void CPaintSequence::OnCopy( wxCommandEvent& event )
{
    if (wxTheClipboard->Open())
    {
        // This data objects are held by the clipboard,
        // so do not delete them in the app.
        int pos1 = m_DragMin;
        int pos2 = m_DragMax;
        if (pos1 > pos2)
            swap(pos1, pos2);
        if (pos1 >= 0 && pos2 >= 0)
            wxTheClipboard->SetData( new wxTextDataObject(m_Seq.substr(pos1, pos2 - pos1 + 1)) );
        wxTheClipboard->Close();
    }
}


string CPaintSequence::CutSelection()
{
    string result;
    int pos1 = m_DragMin;
    int pos2 = m_DragMax;
    if (pos1 > pos2)
        swap(pos1, pos2);
    if (pos1 >= 0 && pos2 >= 0)
    {
        if (pos1 == 0 && pos2 == m_Seq.size() - 2)
        {
            wxMessageBox (_("Unable to delete the whole sequence"), _("Error"), wxOK|wxICON_ERROR);
            result = m_Seq;
            return result;
        }
        bool read_only = false;
        int old_pos = m_CursorSeq;
        for (int i = pos1; i <= pos2; i++)
        {
            m_CursorSeq = i;
            unsigned int seg = PosToSegment();
            if (m_read_only[seg])
            {
                read_only = true;
                break;
            }
        }
        m_CursorSeq = old_pos;
        if (read_only)
        {
            m_Parent->ShowReadOnlyWarning();
            return result;
        }

        result = m_Seq.substr(pos1, pos2 - pos1 + 1);
        string str1;
        if (pos1 > 0)
            str1 = m_Seq.substr(0, pos1);
        string str2;
        if (pos2 + 1 < m_Seq.size())
            str2 = m_Seq.substr(pos2 + 1);
        m_Seq = str1+str2;
        m_CursorSeq = pos1;
        for (int i = pos1; i <= pos2; i++)
        {
            unsigned int seg = PosToSegment();
            m_SeqLen[seg]--;
            if (m_SeqLen[seg] < 0)
                m_SeqLen[seg] = 0;
        }
        AdjustFeatureRange(pos1, -(pos2 - pos1 + 1));
        m_DragMin = -1;
        m_DragMax = -1;
        SeqPosToColRow(); 
        UpdateData();
        SetClean(false);
        ScrollWithCursor();
        Refresh();
    }
    return result;
}

void CPaintSequence::OnCut( wxCommandEvent& event )
{
  
    if (wxTheClipboard->Open())
    {
        string result = CutSelection();
        if (!result.empty())
        {
            // This data objects are held by the clipboard,
            // so do not delete them in the app.
            wxTheClipboard->SetData( new wxTextDataObject(result) );
        }

        wxTheClipboard->Close();
    }

}

void CPaintSequence::OnPaste( wxCommandEvent& event )
{
    unsigned int seg = PosToSegment();
    if (m_read_only[seg])
    {
        m_Parent->ShowReadOnlyWarning();
        return;
    }
    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported( wxDF_UNICODETEXT ) && m_CursorSeq >= 0)
        {
            wxTextDataObject data;
            wxTheClipboard->GetData( data );
            string str =  data.GetText().ToStdString();
            NStr::ToLower(str);
            bool allowed = true;
            for (size_t i=0; i<str.size(); i++)
                if (m_AllowedCharSet.find(str[i]) == string::npos)
                {
                    allowed = false;
                    break;
                }
            if (!str.empty() && allowed)
            {
                string str1;
                if (m_CursorSeq > 0)
                    str1 = m_Seq.substr(0,m_CursorSeq);
                string str2 = m_Seq.substr(m_CursorSeq);
                m_Seq = str1+str+str2;               
                m_SeqLen[seg] += str.size();
                AdjustFeatureRange(m_CursorSeq,str.size());
                m_CursorSeq += str.size();
                m_DragMin = -1;
                m_DragMax = -1;
                SeqPosToColRow(); 
                UpdateData();
                SetClean(false);
                ScrollWithCursor();
                Refresh();
            }
        }
        wxTheClipboard->Close();
    }
}

bool CPaintSequence::IsClipboard()
{
    bool found = false;
    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported( wxDF_UNICODETEXT ))
        {
            wxTextDataObject data;
            wxTheClipboard->GetData( data );
            string str =  data.GetText().ToStdString();
            NStr::ToLower(str);
            bool allowed = true;
            for (size_t i=0; i<str.size(); i++)
                if (m_AllowedCharSet.find(str[i]) == string::npos)
                {
                    allowed = false;
                    break;
                }
            if (!str.empty() && allowed)
            {
                found = true;
            }
        }
        wxTheClipboard->Close();
    }
    return found;
}

bool CPaintSequence::IsSelection()
{
    bool found = false;
    if (m_DragMin >= 0 && m_DragMax >= 0)
        found = true;
    return found;
}

pair<int,int> CPaintSequence::GetSelection()
{
    int pos1 = m_DragMin;
    int pos2 = m_DragMax;
    if (pos1 > pos2)
        swap(pos1, pos2);
    return pair<int,int>(pos1, pos2);
}

string * CPaintSequence::GetFindString(bool is_nuc, bool is_revcomp, const string &choice)
{
    string *str = NULL; 
    if (is_nuc)
    {
        if (is_revcomp)
            str = &m_Complement;
        else
            str = &m_Seq;
    }
    else
    {
        if (choice == "+1")
            str = &m_Prot;
        if (choice == "+2")
            str = &m_Prot1;
        if (choice == "+3")
            str = &m_Prot2;
        if (choice == "-1")
            str = &m_RevProt;
        if (choice == "-2")
            str = &m_RevProt1;
        if (choice == "-3")
            str = &m_RevProt2;
    }
    return str;
}

int CPaintSequence::GetLength()
{
    return m_Seq.size() - 1;
}

set<int>& CPaintSequence::SetHighlights()
{
    m_DragMin = -1;
    m_DragMax = -1;
    return m_highlights;
}


void CPaintSequence::TranslateCDS()
{
    m_Translated.clear();
    m_Translated.resize(m_FeatRanges.size());
    for (unsigned int i = 0; i < m_FeatRanges.size(); i++)
        if ( m_FeatTypes[i].second == CSeqFeatData::eSubtype_cdregion)
        {
            string seq;
            for (unsigned int j=0; j<m_FeatRanges[i].size(); j++)
            {
                TSeqPos start = m_FeatRanges[i][j].first;
                TSeqPos stop = m_FeatRanges[i][j].second;
		if (start != numeric_limits<int>::max() && stop != numeric_limits<int>::max())
		  seq += m_Seq.substr(start, stop-start+1);
            }

            string rev_comp;
            if (m_FeatStrand[i] == objects::CBioseq_Handle::eStrand_Minus)
            {
                CSeqManip::ReverseComplement(seq, CSeqUtil::e_Iupacna, 0, seq.size(), rev_comp);  
                NStr::ToLower(rev_comp);
                swap(rev_comp,seq);
            }
            CGenetic_code *code = NULL;
            if (m_GeneticCode[i])
                code = m_GeneticCode[i].GetPointer();
            int flags = CSeqTranslator::fRemoveTrailingX;
            if (m_Feat5Partial[i])
                flags |= CSeqTranslator::fIs5PrimePartial;
            CSeqTranslator::Translate(seq.substr(m_FeatFrames[i]), m_Translated[i],  flags, code); 
        }
}

TSeqPos CPaintSequence::GetFeatureStart(int i, int j)
{
    if (m_FeatStrand[i] == objects::CBioseq_Handle::eStrand_Minus)
        return m_FeatRanges[i][m_FeatRanges[i].size() - 1 - j].first;
    return m_FeatRanges[i][j].first;
}

TSeqPos CPaintSequence::GetFeatureStop(int i, int j)
{
    if (m_FeatStrand[i] == objects::CBioseq_Handle::eStrand_Minus)
        return m_FeatRanges[i][m_FeatRanges[i].size() - 1 - j].second;
    return m_FeatRanges[i][j].second;
}

void CPaintSequence::AdjustFeatureRange(int origin, int offset)
{
  for (unsigned int i = 0; i < m_FeatRanges.size(); i++)
  {
      for (unsigned int j=0; j<m_FeatRanges[i].size(); j++)
      {
          int start = m_FeatRanges[i][j].first;
          int stop = m_FeatRanges[i][j].second;
          AdjustFeatStartStop(origin, offset, start, stop);
          m_FeatRanges[i][j].first = start;
          m_FeatRanges[i][j].second = stop;
      }
      AdjustProtFeatRange(origin, offset, i);
  }
  GetFeatWholeRange();
  TranslateCDS();
}

void CPaintSequence::AdjustProtFeatRange(int origin, int offset, int i)
{
    if (m_ProtFeatRanges.find(i) == m_ProtFeatRanges.end())
        return;
    for (unsigned int k = 0; k < m_ProtFeatRanges[i].size(); k++)
    {
        for (unsigned int j=0; j<m_ProtFeatRanges[i][k].size(); j++)
        {
            int start = m_ProtFeatRanges[i][k][j].first;
            int stop = m_ProtFeatRanges[i][k][j].second;
            AdjustFeatStartStop(origin, offset, start, stop);
            m_ProtFeatRanges[i][k][j].first = start;
            m_ProtFeatRanges[i][k][j].second = stop;
        }
    }
}

void CPaintSequence::AdjustFeatStartStop(int origin, int offset, int &start, int &stop)
{
    if (offset < 0 && origin <= start && origin - offset >= stop)
    {
        start = numeric_limits<int>::max();
        stop = numeric_limits<int>::max();
    }
    else
    {
        if (start >= origin)
        {
            if (offset >= 0)
            {    
                start += offset;
            }
            else 
            {
                if (origin - offset <= start)
                {
                    start += offset;
                }
                else
                {
                    start = origin;
                }
            }
        }
        if (stop >= origin)
        {
            if (offset >= 0)
            {             
                stop += offset;
            }
            else 
            {
                if (origin - offset <= stop)
                {
                    stop += offset;
                }
                else
                {
                    stop = origin;
                }
            }
        }
    }
    if (start < 0)
        start = 0;
    if (stop < 0)
        stop = 0;
    if (start > m_Seq.size() - 2 && start != numeric_limits<int>::max())
        start = m_Seq.size() - 2;
    if (stop > m_Seq.size() - 2 && stop != numeric_limits<int>::max())
        stop = m_Seq.size() - 2;        
            
    if (stop < start)
    {
        start = numeric_limits<int>::max();
        stop = numeric_limits<int>::max();
    }
}

void CPaintSequence::GetFeatWholeRange()
{
  m_FeatWholeRange.clear();
  m_FeatLengthBefore.clear();
  m_FeatTotalLength.clear();
  for (size_t i = 0; i < m_FeatRanges.size(); i++)
    {
      TSeqPos start = numeric_limits<int>::max();
      TSeqPos stop = 0;
      TSeqPos length = 0;
      vector<TSeqPos> length_before;
      for (size_t j = 0; j < m_FeatRanges[i].size(); j++) 
	{
	  if (m_FeatRanges[i][j].first < start && m_FeatRanges[i][j].first !=  numeric_limits<int>::max())
	    start = m_FeatRanges[i][j].first;
	  if (m_FeatRanges[i][j].second > stop && m_FeatRanges[i][j].second !=  numeric_limits<int>::max())
	    stop = m_FeatRanges[i][j].second;
	  length_before.push_back(length);
	  if (m_FeatRanges[i][j].first !=  numeric_limits<int>::max() && m_FeatRanges[i][j].second !=  numeric_limits<int>::max())
	    length += m_FeatRanges[i][j].second - m_FeatRanges[i][j].first + 1;
        }
      m_FeatWholeRange.push_back(pair<TSeqPos,TSeqPos>(start,stop));
      m_FeatTotalLength.push_back(length);
      m_FeatLengthBefore.push_back(length_before);
    }
}

vector<unsigned int> CPaintSequence::IsExonPresent(int i, int row) const
{
    vector<unsigned int> ranges;
    for (unsigned int j=0; j<m_FeatRanges[i].size(); j++) 
    {
        TSeqPos start = m_FeatRanges[i][j].first;
        TSeqPos stop = m_FeatRanges[i][j].second;
        int row_start = row * m_NumCols;
        int row_end = row_start + m_NumCols - 1;
        if ( (row_start >= start && row_end <= stop) ||
             (start >= row_start && start <= row_end) ||
             (stop >= row_start && stop <= row_end) )
        {
            ranges.push_back(j);
        }
    }
    
    return ranges;
}

map<unsigned int, vector<unsigned int> > CPaintSequence::GetFeatsWithExons(int row, const vector<unsigned int> &feats_in_row) const 
{
    map<unsigned int, vector<unsigned int> > feats_with_exons;
    for (unsigned int k = 0; k < feats_in_row.size(); k++) 
    {
        unsigned int i = feats_in_row[k];
        vector<unsigned int> ranges = IsExonPresent(i, row);
        if (!ranges.empty())
            feats_with_exons[i] = ranges;
    }    
    return feats_with_exons;
}

// test: CP005969.asn1:        length 6094821
END_NCBI_SCOPE
