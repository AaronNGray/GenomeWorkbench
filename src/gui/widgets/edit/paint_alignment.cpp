 /*  $Id: paint_alignment.cpp 44314 2019-11-27 14:31:38Z filippov $
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
#include <wx/msgdlg.h> 
#include <sra/data_loaders/wgs/wgsloader.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_vector.hpp>
#include <util/sequtil/sequtil_manip.hpp>
#include <gui/widgets/edit/paint_alignment.hpp>
#include <gui/widgets/edit/alignment_assistant.hpp>

#include <wx/textfile.h>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CPaintAlignment, wxVScrolledWindow )


BEGIN_EVENT_TABLE(CPaintAlignment, wxVScrolledWindow)
    EVT_PAINT(CPaintAlignment::OnPaint)
    EVT_ERASE_BACKGROUND(CPaintAlignment::OnEraseBackground)
    EVT_SIZE(CPaintAlignment::OnResize)
    EVT_KEY_DOWN(CPaintAlignment::OnKeyDown)
    EVT_MOTION(CPaintAlignment::OnMouseDrag)
    EVT_LEFT_DOWN(CPaintAlignment::OnMouseDown)
END_EVENT_TABLE()


CPaintAlignment::CPaintAlignment(wxWindow *parent, CSeq_align_Handle ah, const vector< vector<vector<pair<TSeqPos,TSeqPos> > > > &feat_ranges, 
                                     const vector< vector<pair<string,objects::CSeqFeatData::ESubtype> > > &feat_types,
                                     const vector< vector<objects::CBioseq_Handle::EVectorStrand> > &feat_strand,
                                     wxWindowID id, const wxPoint &pos, const wxSize &size) 
: wxVScrolledWindow(parent,id, pos, size, wxFULL_REPAINT_ON_RESIZE|wxWANTS_CHARS), m_Alignment(ah), m_FeatRanges(feat_ranges), m_FeatTypes(feat_types), m_FeatStrand(feat_strand), 
    m_EnableFeatures(false), m_EnableSubstitute(true)
{
//m_Alignment.GetScope().AddDataLoader(CWGSDataLoader::GetLoaderNameFromArgs());
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(*wxWHITE);
    m_Font = wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_FontHeight = 10;
    m_FontWidth = 10;
    m_NumRows = 1;
    m_NumCols = 1; 
    m_LastRowLength = 0; 
    GetSegments();
    GetFeatWholeRange();
    if (m_TotalLength > 0)
    {
        wxGraphicsContext *gc = wxGraphicsContext::Create();
        gc->SetFont(m_Font, *wxBLACK);
        CalculateFontWidthAndHeight(gc);
        m_NumCols = CalculateNumCols();
        m_NumRows = m_NumAlignRows * (m_TotalLength / m_NumCols);
        m_LastRowLength = m_TotalLength % m_NumCols;
        if (m_LastRowLength > 0)
            m_NumRows += m_NumAlignRows;
    }
    m_CursorCol = 0;
    m_CursorRow = 0;
    m_CursorSeq = -1;
    m_CursorAlign = -1;
    m_DragMin = -1;
    m_DragMax = -1;
    m_SelAlignRow = -1;
    SetRowCount(m_NumRows);
    m_Parent = NULL;
    wxWindow *win = GetParent();
    while (win)
    {
        CAlignmentAssistant  *base = dynamic_cast<CAlignmentAssistant *>(win);
        if (base)
            m_Parent = base;
        win = win->GetParent();
    }
}


string CPaintAlignment::GetSeqTitle(CBioseq_Handle bsh)
{
    string accession;
    string local;
    string label;
    for (CBioseq_Handle::TId::const_iterator it = bsh.GetId().begin(); it != bsh.GetId().end(); ++it)
    {
        const CSeq_id &id = *(it->GetSeqId());
        if (id.IsGenbank() && id.GetGenbank().IsSetAccession())
            accession = id.GetGenbank().GetAccession();
        if (id.IsLocal() && id.GetLocal().IsStr())
            local = id.GetLocal().GetStr();
    }
    if (!accession.empty())
        return accession;
    if (!local.empty())
        return local;

    bsh.GetBioseqCore()->GetLabel(&label, CBioseq::eContent);
    return label;
}

void CPaintAlignment::SetAlign(CSeq_align_Handle ah)
{ 
    m_Alignment = ah;
    GetSegments();
}

void CPaintAlignment::GetSegments() 
{
    m_Seqs.clear();
    m_Labels.clear();
    m_TotalLength = 0;
    if ( m_Alignment &&   m_Alignment.GetSegs().Which() == CSeq_align::C_Segs::e_Denseg )
    {
        const CDense_seg& denseg = m_Alignment.GetSegs().GetDenseg();
        if ( denseg.CanGetDim() && denseg.CanGetNumseg() &&
             denseg.CanGetIds() && denseg.CanGetStarts() &&
             denseg.CanGetLens()) 
        {
            CDense_seg::TDim num_rows = denseg.GetDim();
            CDense_seg::TNumseg num_segs = denseg.GetNumseg();
            for (CDense_seg::TDim row = 0; row < num_rows; ++row)
            {
                const CSeq_id& id = denseg.GetSeq_id(row);
                CBioseq_Handle bsh = m_Alignment.GetScope().GetBioseqHandle(id);
                if (!bsh)
                    continue;
                string label = GetSeqTitle(bsh);
                int length = bsh.GetBioseqLength();
                CSeqVector vec_plus = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
                string seq_plus;
                vec_plus.GetSeqData(0, length, seq_plus);
                string seq_minus;
                if (bsh.IsNucleotide())
                    CSeqManip::ReverseComplement(seq_plus, CSeqUtil::e_Iupacna, 0, seq_plus.size(), seq_minus);  
                else
                    CSeqManip::ReverseComplement(seq_plus, CSeqUtil::e_Iupacaa, 0, seq_plus.size(), seq_minus);  
                string seq;
                for (CDense_seg::TNumseg seg = 0; seg < num_segs; ++seg) 
                {
                    TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + row];
                    TSignedSeqPos len   = denseg.GetLens()[seg];
                    ENa_strand strand = eNa_strand_plus;
                    if (denseg.IsSetStrands())
                        strand = denseg.GetStrands()[seg * num_rows + row];
                    if (start >= 0)
                    {
                        if (strand != eNa_strand_minus)
                        {
                            if (start < seq_plus.size())
                            {
                                seq += seq_plus.substr(start, len);
                            }
                        }
                        else
                        {
                            TSignedSeqPos real_start = length - start - len;
                            if (real_start >= 0 && real_start < seq_minus.size())
                            {
                                seq += seq_minus.substr(real_start, len); 
                            }
                        }
                    }
                    else
                    {
                        seq += string(len, '-');
                    }
                }
                m_Seqs.push_back(seq);
                m_Labels.push_back(label);
                m_visible_to_row.push_back(row);
            }
        }
    }
    if (m_Seqs.empty() || m_Seqs.front().empty())
        NCBI_THROW( CException, eUnknown, "Cannot parse Seq-align" );
    m_TotalLength = m_Seqs.front().length();
    m_NumAlignRows = m_Seqs.size();
}

wxCoord CPaintAlignment::OnGetRowHeight( size_t row ) const
{
    int r  = 0;
    int align_row = row % m_NumAlignRows;
    vector<unsigned int> feats_in_row = GetFeaturesInRow(row, align_row);
    map<unsigned int, vector<unsigned int> > feats_with_exons = GetFeatsWithExons(row, align_row, feats_in_row);
    DrawLabelCell(0, 0, r, row, align_row, feats_in_row, feats_with_exons, NULL); 
    return r;
}

int CPaintAlignment::AlignPosToSeqPos(int pos, int row, bool left) const
{
    int res = -1;
    const CDense_seg& denseg = m_Alignment.GetSegs().GetDenseg();
    const CSeq_id& id = denseg.GetSeq_id(m_visible_to_row[row]);
    CBioseq_Handle bsh = m_Alignment.GetScope().GetBioseqHandle(id);
    if (!bsh)
        return -1;
    int length = bsh.GetBioseqLength();
    CDense_seg::TNumseg num_segs = denseg.GetNumseg();
    CDense_seg::TDim num_rows = denseg.GetDim();
    CDense_seg::TNumseg seg = 0;
    TSignedSeqPos total_len = 0;
    int found_seg = -1;
    while ( seg < num_segs ) 
    {
        TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + m_visible_to_row[row]];
        TSignedSeqPos len   = denseg.GetLens()[seg];
        ENa_strand strand = eNa_strand_plus;
        if (denseg.IsSetStrands())
            strand = denseg.GetStrands()[seg * num_rows + m_visible_to_row[row]];
           
        total_len += len;
        if ((total_len > pos && total_len - len <= pos && strand != eNa_strand_minus) ||
            (total_len - 1>= pos && total_len -1 - len < pos && strand == eNa_strand_minus))
        {
            if (start >= 0)
            {
                res = start + (pos - (total_len - len));
                if (strand == eNa_strand_minus)
                {
                    res = start + total_len - 1 - pos;
                }
                break;
            }
            else
            {
                found_seg = seg;
                break;
            }
        }
        ++seg;
    }
    if (found_seg >= 0)
    {
        seg = found_seg;
        if (left)
        {
            ++seg;
            while ( seg < num_segs ) 
            {
                TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + m_visible_to_row[row]];
                TSignedSeqPos len   = denseg.GetLens()[seg];
                ENa_strand strand = eNa_strand_plus;
                if (denseg.IsSetStrands())
                    strand = denseg.GetStrands()[seg * num_rows + m_visible_to_row[row]];
                if (start >= 0)
                {
                    res = start;
                    if (strand == eNa_strand_minus)
                        res = start + len - 1;
                    break;
                }
                ++seg;
            }
        }
        else
        {
            while (seg > 0)
            {
                --seg;
                TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + m_visible_to_row[row]];
                TSignedSeqPos len   = denseg.GetLens()[seg];
                ENa_strand strand = eNa_strand_plus;
                if (denseg.IsSetStrands())
                    strand = denseg.GetStrands()[seg * num_rows + m_visible_to_row[row]];
                if (start >= 0)
                {
                    res = start + len - 1;
                    if (strand == eNa_strand_minus)
                        res = start;
                    break;
                }
            }
        }
    }
    return res;
}

int CPaintAlignment::SeqPosToAlignPos(int pos, int row, bool left)
{
    int res = 0;
    const CDense_seg& denseg = m_Alignment.GetSegs().GetDenseg();
    const CSeq_id& id = denseg.GetSeq_id(m_visible_to_row[row]);
    CBioseq_Handle bsh = m_Alignment.GetScope().GetBioseqHandle(id);
    if (!bsh)
        return 0;
    int length = bsh.GetBioseqLength();
    CDense_seg::TNumseg num_segs = denseg.GetNumseg();
    CDense_seg::TDim num_rows = denseg.GetDim();
    CDense_seg::TNumseg seg = 0;
    TSignedSeqPos total_len = 0;
    int found_len = 0;
    bool found = false;
    while ( seg < num_segs ) 
    {
        TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + m_visible_to_row[row]];
        TSignedSeqPos len   = denseg.GetLens()[seg];
        ENa_strand strand = eNa_strand_plus;
        int pos2 = pos;
        if (denseg.IsSetStrands())
            strand = denseg.GetStrands()[seg * num_rows + m_visible_to_row[row]];
        if (strand == eNa_strand_minus && start >= 0)
        {
            start = length - start - len;
            pos2 = length - pos - 1;
        }
        if (start >= 0 && pos2 >= start && pos2 < start + len)
        {
            res = total_len + pos2 - start;
            found = true;
            break;
        }
        if (start >= 0 && start > pos2 && left)
        {
            res = total_len;
            found = true;
            break;
        }
        if (start >= 0 && start + len - 1 < pos2 && !left)
        {
            found_len = total_len + len - 1;
        }
        total_len += len;
        ++seg;
    }
    if (!found)
        res = found_len;

    return res;
}

CPaintAlignment::~CPaintAlignment() 
{ 
}

void CPaintAlignment::UpdateFeatures(const vector< vector<vector<pair<TSeqPos,TSeqPos> > > > &feat_ranges, 
                                     const vector< vector<pair<string,objects::CSeqFeatData::ESubtype> > > &feat_types,
                                     const vector< vector<objects::CBioseq_Handle::EVectorStrand> > &feat_strand)
{
    m_FeatRanges = feat_ranges;
    m_FeatTypes = feat_types;
    m_FeatStrand = feat_strand;
    GetFeatWholeRange();
}

void CPaintAlignment::RefreshWithScroll()
{
    SeqPosToColRow();
    NormalizeCursorColRows();
    ScrollWithCursor();
    Refresh();
}

void CPaintAlignment::EnableFeatures(bool enable)
{
    m_EnableFeatures = enable;
    RefreshWithScroll();
}

void CPaintAlignment::EnableSubstitute(bool enable)
{
    m_EnableSubstitute = enable;
    RefreshWithScroll();
}

void CPaintAlignment::ScrollWithCursor()
{
    if (m_CursorRow < GetVisibleRowsBegin() || m_CursorRow > GetVisibleRowsEnd() - 1)
        ScrollToRow(m_CursorRow);    
}


void CPaintAlignment::OnKeyDown(wxKeyEvent& event)
{
    wxSize sz = GetClientSize();
    int num_rows = GetVisibleRowsEnd() - GetVisibleRowsBegin() + 1 - 1;

    int uc = event.GetKeyCode();
    switch ( uc )
    {
    case WXK_LEFT  : m_CursorCol--; m_CursorSeq = -1; m_CursorAlign = -1; break;
    case WXK_RIGHT : m_CursorCol++; m_CursorSeq = -1; m_CursorAlign = -1; break;
    case WXK_UP    : m_CursorRow--; m_CursorSeq = -1; m_CursorAlign = -1; break;
    case WXK_DOWN  : m_CursorRow++; m_CursorSeq = -1; m_CursorAlign = -1; break;
    case WXK_END   : m_CursorSeq = m_TotalLength - 1; SeqPosToColRow(); break;
    case WXK_HOME  : m_CursorSeq = 0; SeqPosToColRow(); break;
    case WXK_PAGEUP : m_CursorRow = GetVisibleRowsBegin() - num_rows; m_CursorSeq = -1; m_CursorAlign = -1; break;
    case WXK_PAGEDOWN : m_CursorRow = GetVisibleRowsBegin() + num_rows; m_CursorSeq = -1; m_CursorAlign = -1; break;
    case WXK_INSERT :
        if (m_CursorSeq >= 0 && m_CursorSeq < m_TotalLength && m_CursorAlign >= 0 && m_CursorAlign < m_NumAlignRows) 
        {
            if (m_DragMin < 0)
            {
                m_DragMin = m_CursorSeq;
                m_SelAlignRow = m_CursorAlign;
            }
            else if (m_DragMax < 0 && m_CursorAlign == m_SelAlignRow)
                m_DragMax = m_CursorSeq;
            else if (m_CursorAlign == m_SelAlignRow)
            {
                if (abs(m_DragMin - m_CursorSeq) < abs(m_DragMax - m_CursorSeq))
                    m_DragMin = m_CursorSeq;
                else
                    m_DragMax = m_CursorSeq;
            }          
            else 
            {
                wxMessageBox (_("Selection should be confined to the same bioseq"), _("Warning"), wxOK);
            }
        }      
        break;
    case WXK_SPACE:
        event.Skip();
        break;
    case WXK_DELETE: 
    case WXK_BACK  :
        if (m_Parent)
        {
            m_Parent->DeleteSelection();
            m_DragMin = -1;
            m_DragMax = -1;
            m_SelAlignRow = -1;
            m_CursorSeq = -1; 
            m_CursorAlign = -1;
        }
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

void CPaintAlignment::NormalizeCursorColRows()
{
    if (m_CursorCol < 0)
        m_CursorCol = 0;
    if (m_CursorCol >= m_NumCols)
        m_CursorCol = m_NumCols-1;
    if (m_CursorRow < 0)
        m_CursorRow = 0;
    if (m_CursorRow >= m_NumRows)
        m_CursorRow = m_NumRows-1;
    if (m_CursorRow >= m_NumRows - m_NumAlignRows && m_CursorCol >= m_LastRowLength && m_LastRowLength > 0)
        m_CursorCol = m_LastRowLength-1;
    if (m_CursorAlign < 0)
        m_CursorAlign = 0;
    if (m_CursorAlign >= m_NumAlignRows)
        m_CursorAlign = m_NumAlignRows - 1;
    if (m_SelAlignRow < 0)
        m_SelAlignRow = 0;
    if (m_SelAlignRow >= m_NumAlignRows)
        m_SelAlignRow = m_NumAlignRows - 1;
}

int CPaintAlignment::LeftMarginWidth() const
{
    return (5 + 15 * m_FontWidth + 5); // leaving 10 spaces for feature labels
}

int CPaintAlignment::FindRowByCoord(int y, int &y_row)
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

bool CPaintAlignment::MouseToSeqPos(wxPoint p, int &row, int &y_row)
{
    bool found = false;
    row = FindRowByCoord(p.y, y_row);
    int col = (p.x - LeftMarginWidth()) / m_FontWidth; 
    int num_groups = col / 12; // 2 spaces between groups
    int pos_in_group = col % 12;
    if (pos_in_group != 10 && pos_in_group != 11)
    {        
        m_CursorCol = num_groups * 10 + pos_in_group;
        m_CursorRow = row;
        ColRowToSeqPos();
        found = true;
    }

    return found;
}

void CPaintAlignment::OnMouseDrag(wxMouseEvent& evt)
{
    if (evt.Dragging())
    {
        int row = 0, y_row;
        bool found = MouseToSeqPos(evt.GetPosition(), row, y_row); 
        int align_row =  row % m_NumAlignRows;
        if (found && (align_row == m_SelAlignRow || m_SelAlignRow < 0))
        {
            if ( m_DragMin < 0)
                m_DragMin = m_CursorSeq;
            else
                m_DragMax = m_CursorSeq;
            m_SelAlignRow = align_row;
            if (m_DragMin >= 0 && m_DragMin > m_TotalLength - 1)
                m_DragMin = m_TotalLength - 1;
            if (m_DragMax >= 0 && m_DragMax > m_TotalLength - 1)
                m_DragMax = m_TotalLength - 1;
           
            Refresh();
        }
    }
    evt.Skip();
}

void CPaintAlignment::OnMouseDown(wxMouseEvent& evt)
{
    int row, y_row;
    bool found = MouseToSeqPos(evt.GetPosition(), row, y_row);
   
    if (evt.GetModifiers() == wxMOD_SHIFT)
    {
        if (m_CursorSeq >= 0 && m_CursorSeq < m_TotalLength && m_CursorAlign >= 0 && m_CursorAlign < m_NumAlignRows) 
        {
            if (m_DragMin < 0)
            {
                m_DragMin = m_CursorSeq;
                m_SelAlignRow = m_CursorAlign;
            }
            else if (m_DragMax < 0 && m_CursorAlign == m_SelAlignRow)
                m_DragMax = m_CursorSeq;
            else if (m_CursorAlign == m_SelAlignRow)
            {
                if (abs(m_DragMin - m_CursorSeq) < abs(m_DragMax - m_CursorSeq))
                    m_DragMin = m_CursorSeq;
                else
                    m_DragMax = m_CursorSeq;
            }          
            else
            {
                wxMessageBox (_("Selection should be confined to the same bioseq"), _("Warning"), wxOK);
            }
        }
        NormalizeCursorColRows();
        ScrollWithCursor();
        Refresh();
        return;
    }
    if (m_DragMin >= 0 && m_DragMax >= 0)
    {
        m_DragMin = -1;
        m_DragMax = -1;
        m_SelAlignRow = -1;
        found = true;
    }

    if (found)
    {
        Refresh();
    }
    evt.Skip();
}

void CPaintAlignment::SeqPosToColRow()
{
    m_CursorRow = m_CursorSeq / m_NumCols; 
    m_CursorRow *= m_NumAlignRows;
    m_CursorRow += m_CursorAlign;
    m_CursorCol = m_CursorSeq % m_NumCols;   
}

void CPaintAlignment::ColRowToSeqPos()
{
    m_CursorSeq = (m_CursorRow / m_NumAlignRows) * m_NumCols + m_CursorCol;
    m_CursorAlign = m_CursorRow % m_NumAlignRows;
}

void CPaintAlignment::SetPos(int pos)
{
    m_CursorSeq = pos - 1;
    if (m_CursorSeq < 0)
        m_CursorSeq = 0;
    if (m_CursorSeq >= m_TotalLength)
        m_CursorSeq = m_TotalLength - 1;
    SeqPosToColRow();
    ScrollWithCursor();
    Refresh();
}

void CPaintAlignment::SetPosSeq(int pos)
{
    if (pos < 1)
        return;
    m_CursorSeq = SeqPosToAlignPos(pos - 1, m_CursorAlign, true);
    if (m_CursorSeq < 0)
        m_CursorSeq = 0;
    if (m_CursorSeq >= m_TotalLength)
        m_CursorSeq = m_TotalLength - 1;
    SeqPosToColRow();
    ScrollWithCursor();
    Refresh();
}

void CPaintAlignment::SetTarget(int row)
{
    if (row < 0 || row >= m_NumAlignRows)
        return;
    m_CursorAlign = row;
    SeqPosToColRow();
    ScrollWithCursor();
    Refresh();
}


/// Painting
void CPaintAlignment::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    wxGraphicsContext *gc = wxGraphicsContext::Create( dc );
    if (gc && m_TotalLength > 0)
    {
        ClearScreen(gc);
        gc->SetFont(m_Font, *wxBLACK);
        m_NumCols = CalculateNumCols();
        m_NumRows = m_NumAlignRows * (m_TotalLength / m_NumCols);
        m_LastRowLength = m_TotalLength % m_NumCols;
        if (m_LastRowLength > 0)
            m_NumRows += m_NumAlignRows;
        SetRowCount(m_NumRows);
        if (m_CursorSeq >= 0)
            SeqPosToColRow();
        else
            ColRowToSeqPos();
        int hidden_rows = GetVisibleRowsBegin(); 
        unsigned int start= (hidden_rows / m_NumAlignRows) * m_NumCols;
        unsigned int row = hidden_rows;
        unsigned int start_align_row = hidden_rows % m_NumAlignRows;
        int client_y = GetClientSize().y;
        int y = TopMarginHeight();
        while (start < m_TotalLength)
        {
            for (size_t align_row = start_align_row; align_row < m_NumAlignRows; ++align_row)
            {
                unsigned int seq_pos = start;
                DrawTextLine(start, y, row, align_row, gc, seq_pos);
                if (y > client_y)
                    break;
                row++;
            }
            if (y > client_y)
                break;
            start += m_NumCols;
            start_align_row = 0;
        }    
        delete gc;
    }

    if (m_Parent)
    {
        int pos1 = m_DragMin;
        int pos2 = m_DragMax;
        if (pos1 > pos2)
            swap(pos1, pos2);
        string label;
        if (IsSelection())
        {
            pos1 = AlignPosToSeqPos(pos1, m_SelAlignRow, true);
            pos2 = AlignPosToSeqPos(pos2, m_SelAlignRow, false);
            label = m_Labels[m_SelAlignRow];
        }
        if ( m_CursorSeq >=0 )
        {
            int pos = AlignPosToSeqPos(m_CursorSeq, m_CursorAlign, false);
            m_Parent->ReportPos(pos+1, m_Labels[m_CursorAlign]);            
        }
        m_Parent->ReportRange(pos1+1, pos2+1, label);
    }
}



// Empty implementation, to prevent flicker
void CPaintAlignment::OnEraseBackground(wxEraseEvent& event)
{
}

////////////////////////////////////////////////////////////
/// Notification for the derived class that moment is good
/// for doing its update and drawing stuff
////////////////////////////////////////////////////////////
void CPaintAlignment::OnUpdate()
{

}

void CPaintAlignment::OnResize(wxSizeEvent& event)
{

}

void CPaintAlignment::ClearScreen(wxGraphicsContext *gc)
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

void CPaintAlignment::CalculateFontWidthAndHeight(wxGraphicsContext *gc)
{
    wxDouble width;
    wxDouble height;
    wxDouble descent;
    wxDouble externalLeading;
    if (m_TotalLength > 1000)
    {
        gc->GetTextExtent(wxString(m_Seqs.front().substr(0,1000)), &width, &height, &descent, &externalLeading);
        width /= 1000;
    }
    else
    {
        gc->GetTextExtent(wxString(m_Seqs.front()), &width, &height, &descent, &externalLeading);
        width /= m_TotalLength;
    }
    width = ceil(1+width);
    height = ceil(2+height);
    m_FontHeight = height;
    m_FontWidth = width;
}

int CPaintAlignment::CalculateNumCols()
{
    int width = int(GetClientSize().x);
    width -= LeftMarginWidth();   
    int line_length = width / m_FontWidth;
    int num_groups = line_length / 12; // leaving 2 spaces for breaks between each 10
    if (num_groups == 0)
        num_groups = 1;
    if (num_groups > 1 && width - 12*num_groups*m_FontWidth < 8*m_FontWidth)
        num_groups--;
    line_length = 10*num_groups;
    return line_length;
}

void CPaintAlignment::DrawTextLine(const unsigned int start, int &y_label, int row, int align_row, wxGraphicsContext *gc, unsigned int &seq_pos)
{
    int x = 5;
    m_ContinueFeatures = false;
    unsigned int orig_pos = seq_pos;
    int orig_y = y_label;
    vector<unsigned int> feats_in_row = GetFeaturesInRow(row, align_row); 
    map<unsigned int, vector<unsigned int> > feats_with_exons = GetFeatsWithExons(row, align_row, feats_in_row); 
    DrawLabelCell(orig_pos,x,y_label,row, align_row, feats_in_row, feats_with_exons, gc);
    x += LeftMarginWidth() - 5;
    for (unsigned int i=0; i<m_NumCols; i++)
    {
        int y = orig_y;
        DrawTextCell(start, i, row, align_row, seq_pos, x, y, feats_in_row, feats_with_exons, gc);
        seq_pos++;
        x += m_FontWidth;
        if (i % 10 == 9)
        {
            x += 2*m_FontWidth; // skipping two spaces after each 10
            m_ContinueFeatures = true;
        }
        else
        {
            m_ContinueFeatures = false;
        }
    }
    x = LeftMarginWidth() + (m_NumCols / 10)* 12 * m_FontWidth;
}

void CPaintAlignment::DrawLabelCell(unsigned int pos, int x, int &y, int row, int align_row, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const
{
    DrawTopLineNumbers(align_row, 0, 0, x, y, NULL);
    DrawLineNumber(align_row, pos, x, y, gc); 
    DrawFeatureLabels(row, align_row, x, y, feats_in_row, feats_with_exons, gc); 
    y += 5;
}

void CPaintAlignment::DrawTopLineNumbers(int align_row, unsigned int seq_pos, int col, int x, int &y, wxGraphicsContext *gc) const
{
    if (align_row != 0)
        return;
    if (gc)
    {
        wxString label;
        label << seq_pos + 1 + (10 - ((col + 1) % 10));
        if ((col + label.Length()) % 10 == 0 )
        {
            gc->SetFont(m_Font, wxColour(255,0,255));
            gc->DrawText(label,x,y);  
            gc->SetFont(m_Font,*wxBLACK);
        }
    }
    y +=  m_FontHeight;
    if (gc)
    {
        gc->SetPen(wxPen(wxColour(255,0,255)));
        if ( (col +1) % 10 == 0)
            gc->StrokeLine(x+m_FontWidth/2, y, x+m_FontWidth/2, y+16);
        else if ( (col +1) % 10 == 5)
            gc->StrokeLine(x+m_FontWidth/2, y+8, x+m_FontWidth/2, y+16);
    }
    y += 16;
}

void CPaintAlignment::DrawTextCell(const unsigned int start, int col, int row, int align_row, unsigned int seq_pos, int x, int &y, const vector<unsigned int> &feats_in_row, 
                                   const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const
{   
    DrawTopLineNumbers(align_row, seq_pos, col, x, y, gc);
    if (gc && start + col < m_TotalLength)
    {
        gc->SetFont(m_Font,*wxBLACK);
        char substr = '\0';
        if (seq_pos < m_Seqs[align_row].size())
            substr =  m_Seqs[align_row][seq_pos];
        if (m_EnableSubstitute && substr != '-' && seq_pos < m_Seqs[m_CursorAlign].size() && substr == m_Seqs[m_CursorAlign][seq_pos] && align_row != m_CursorAlign)
            substr = '.';
        if (substr != '\0')
        {
            int pos1 = m_DragMin;
            int pos2 = m_DragMax;
            if (pos1 > pos2)
                swap(pos1, pos2);
            if (pos1 >= 0 && pos2 >= 0 && seq_pos >= pos1 && seq_pos <= pos2 && align_row == m_SelAlignRow)
                gc->DrawText(wxString(substr),x,y, gc->CreateBrush(*wxLIGHT_GREY_BRUSH));
            else
                gc->DrawText(wxString(substr),x,y); 
        }
    }
    y +=  m_FontHeight;
    DrawCursor(col,row,x,y,gc);
    DrawFeatures(row,align_row,seq_pos,x,y,feats_in_row,feats_with_exons,gc); 
    y += 5;    
}

void CPaintAlignment::DrawCursor(int col, int row, int x, int &y, wxGraphicsContext *gc) const
{
    if (col == m_CursorCol && row == m_CursorRow && gc) 
    {
        gc->SetPen( *wxRED_PEN);
        gc->StrokeLine(x,y,x+m_FontWidth,y);                   
    }
    //y += 1;
}


const wxPen* CPaintAlignment::GetColorForFeature(objects::CSeqFeatData::ESubtype subtype) const
{
    const wxPen* pen = wxBLACK_PEN;
    switch(subtype)
    {
    case CSeqFeatData::eSubtype_cdregion : pen = wxBLUE_PEN; break;
    default                              : pen = wxBLACK_PEN; break;
    }
    return pen;
}

void CPaintAlignment::DrawLineNumber(int align_row, unsigned int seq_pos, int x, int &y, wxGraphicsContext *gc) const
{
    int allowed = LeftMarginWidth() / m_FontWidth;
    wxString pos_label;
    int pos = AlignPosToSeqPos(seq_pos, align_row, false);
    if (pos < 0)
        pos = 0;
    pos_label << pos+1;
    wxString id_label(m_Labels[align_row]); 
    if (allowed > pos_label.Length() && allowed < pos_label.Length() + id_label.Length())
        id_label = id_label.Left(allowed - pos_label.Length());
    int len = allowed - pos_label.Length() - id_label.Length();
    if (len <= 0)
        id_label = wxString(m_Labels[align_row]).Left(allowed);
    
    if (gc)
    {
        if (align_row == m_CursorAlign)
            gc->SetFont(m_Font,*wxRED);
        else
            gc->SetFont(m_Font,*wxBLACK);
        gc->DrawText(id_label, x, y);        

        if (len > 0)
        {
            wxDouble width;
            wxDouble height;
            wxDouble descent;
            wxDouble externalLeading;
            gc->GetTextExtent(id_label + wxString('N',len), &width, &height, &descent, &externalLeading);
            x += width;
            gc->SetFont(m_Font,wxColour(255,0,255));
            gc->DrawText(pos_label, x, y);
        }
        gc->SetFont(m_Font,*wxBLACK);
    }
    y += m_FontHeight; 
}


void CPaintAlignment::DrawFeatureLabels(int row, int align_row, int x, int &y, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const
{ 
    for (unsigned int k = 0; k < feats_in_row.size(); k++) 
    {
        unsigned int i = feats_in_row[k];
        if (m_EnableFeatures)
        {    
            string label = m_FeatTypes[align_row][i].first.substr(0,10); // only take first 10 characters of the label
            if (gc)
                gc->DrawText(wxString(label),x,y);
            y +=  m_FontHeight;                
        }
    }    
}

vector<unsigned int> CPaintAlignment::GetFeaturesInRow(int row, int align_row) const 
{
    vector<unsigned int> feats_in_row;
    
    for (size_t i = 0; i < m_FeatWholeRange[align_row].size(); i++)
    {
        TSeqPos start = m_FeatWholeRange[align_row][i].first;
        TSeqPos stop = m_FeatWholeRange[align_row][i].second;
        int row_start = (row / m_NumAlignRows) * m_NumCols;
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


void CPaintAlignment::DrawFeatures(int row, int align_row, unsigned int seq_pos, int x, int &y, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const
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
                    TSeqPos start = m_FeatRanges[align_row][i][j].first;
                    TSeqPos stop = m_FeatRanges[align_row][i][j].second;
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


            if (gc && seq_pos >= m_FeatWholeRange[align_row][i].first && seq_pos <= m_FeatWholeRange[align_row][i].second)
            {
                    gc->SetPen( *GetColorForFeature(m_FeatTypes[align_row][i].second));
                    gc->SetBrush(*wxBLACK_BRUSH);
                    gc->StrokeLine(x, y+m_FontHeight/2, x+m_FontWidth, y+m_FontHeight/2);     
                    if (found)
                        gc->StrokeLine(x, y+m_FontHeight/2+1, x+m_FontWidth, y+m_FontHeight/2+1);
                    if (m_ContinueFeatures && !startpoint)
                    {
                        gc->StrokeLine(x-2*m_FontWidth, y+m_FontHeight/2, x ,y+m_FontHeight/2);
                        if (found)
                            gc->StrokeLine(x-2*m_FontWidth, y+m_FontHeight/2+1, x ,y+m_FontHeight/2+1);
                    }
                    if (m_FeatStrand[align_row][i] == objects::CBioseq_Handle::eStrand_Plus)
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

    }    
}


bool CPaintAlignment::IsSelection()
{
    bool found = false;
    if (m_DragMin >= 0 && m_DragMax >= 0 && m_SelAlignRow >= 0)
        found = true;
    return found;
}

pair<int,int> CPaintAlignment::GetSelection(int row)
{
    if (IsSelection())
    {
        int pos1 = m_DragMin;
        int pos2 = m_DragMax;
        if (pos1 > pos2)
            swap(pos1, pos2);

        pos1 = AlignPosToSeqPos(pos1, row, true);
        pos2 = AlignPosToSeqPos(pos2, row, false);
        return pair<int,int>(pos1, pos2);
    }
    return  pair<int,int>(-1, -1);
}

pair<int,int> CPaintAlignment::GetAlignSelection()
{
    if (IsSelection())
    {
        int pos1 = m_DragMin;
        int pos2 = m_DragMax;
        if (pos1 > pos2)
            swap(pos1, pos2);
        return pair<int,int>(pos1, pos2);
    }
    return  pair<int,int>(0, m_TotalLength - 1);
}

void CPaintAlignment::GetFeatWholeRange()
{
    for (size_t row = 0; row < m_NumAlignRows; row++)
        for (size_t i = 0; i < m_FeatRanges[row].size(); i++)
            for (size_t j = 0; j < m_FeatRanges[row][i].size(); j++) 
            {
                TSeqPos start = m_FeatRanges[row][i][j].first;
                TSeqPos stop = m_FeatRanges[row][i][j].second;

                m_FeatRanges[row][i][j].first = SeqPosToAlignPos(start, row, true);
                m_FeatRanges[row][i][j].second = SeqPosToAlignPos(stop, row, false);
                if (m_FeatRanges[row][i][j].first > m_FeatRanges[row][i][j].second)
                    swap(m_FeatRanges[row][i][j].first, m_FeatRanges[row][i][j].second);
            }


    m_FeatWholeRange.clear();
    m_FeatLengthBefore.clear();
    m_FeatTotalLength.clear();
    for (size_t row = 0; row < m_NumAlignRows; row++)
    {
        vector<pair<TSeqPos,TSeqPos> >  feat_whole_range;
        vector< vector<TSeqPos> >  feat_length_before;
        vector<TSeqPos> feat_total_length;
        for (size_t i = 0; i < m_FeatRanges[row].size(); i++)
        {
            TSeqPos start = INT_MAX;
            TSeqPos stop = 0;
            TSeqPos length = 0;
            vector<TSeqPos> length_before;
            for (size_t j = 0; j < m_FeatRanges[row][i].size(); j++) 
            {
                if (m_FeatRanges[row][i][j].first < start)
                    start = m_FeatRanges[row][i][j].first;
                if (m_FeatRanges[row][i][j].second > stop)
                    stop = m_FeatRanges[row][i][j].second;
                length_before.push_back(length);
                length += m_FeatRanges[row][i][j].second - m_FeatRanges[row][i][j].first + 1;
            }
            feat_whole_range.push_back(pair<TSeqPos,TSeqPos>(start,stop));
            feat_total_length.push_back(length);
            feat_length_before.push_back(length_before);
        }
        m_FeatWholeRange.push_back(feat_whole_range);
        m_FeatTotalLength.push_back(feat_total_length);
        m_FeatLengthBefore.push_back(feat_length_before);
    }  
}

vector<unsigned int> CPaintAlignment::IsExonPresent(int i, int row, int align_row) const
{
    vector<unsigned int> ranges;
    for (unsigned int j=0; j<m_FeatRanges[align_row][i].size(); j++) 
    {
        TSeqPos start = m_FeatRanges[align_row][i][j].first;
        TSeqPos stop = m_FeatRanges[align_row][i][j].second;
        int row_start = (row / m_NumAlignRows) * m_NumCols;
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

map<unsigned int, vector<unsigned int> > CPaintAlignment::GetFeatsWithExons(int row, int align_row, const vector<unsigned int> &feats_in_row) const 
{
    map<unsigned int, vector<unsigned int> > feats_with_exons;
    for (unsigned int k = 0; k < feats_in_row.size(); k++) 
    {
        unsigned int i = feats_in_row[k];
        vector<unsigned int> ranges = IsExonPresent(i, row, align_row);
        if (!ranges.empty())
            feats_with_exons[i] = ranges;
    }    
    return feats_with_exons;
}

string CPaintAlignment::GetExportInter()
{
    string str;
    string lb(wxString(wxTextFile::GetEOL()).ToStdString());
    size_t pos = 0;
    while (pos < m_TotalLength)
    {
        for (size_t i = 0; i < m_Labels.size(); i++)
        {
            str += m_Labels[i] + "   ";
            if (pos < m_Seqs[i].size())
                str += m_Seqs[i].substr(pos, m_NumCols);
            str += lb;
        }
        str += lb;
        pos += m_NumCols;
    }
    return str;
}

string CPaintAlignment::GetExportCont()
{
    string str;
    string lb(wxString(wxTextFile::GetEOL()).ToStdString());
    for (size_t i = 0; i < m_Labels.size(); i++)
    {
        str += ">" + m_Labels[i] + lb;
        size_t pos = 0;
        while (pos < m_TotalLength)
        {
            if (pos < m_Seqs[i].size())
                str += m_Seqs[i].substr(pos, m_NumCols) + lb;
            pos += m_NumCols;
        }
        str += lb;
    }
    return str;
}

END_NCBI_SCOPE
