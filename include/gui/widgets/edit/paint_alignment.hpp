/*  $Id: paint_alignment.hpp 44314 2019-11-27 14:31:38Z filippov $
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
#ifndef _PAINT_ALIGNMENT_H_
#define _PAINT_ALIGNMENT_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/seq_align_handle.hpp>
/*!
 * Includes
 */

////@begin includes
////@end includes


#include <wx/scrolwin.h>
#include <wx/vscroll.h>
#include <wx/textwrapper.h>
#include <wx/dcbuffer.h>
#include <wx/platform.h>


/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);
class CAlignmentAssistant;

class CPaintAlignment : public wxVScrolledWindow
{
    DECLARE_DYNAMIC_CLASS( CPaintAlignment )
    DECLARE_EVENT_TABLE()
public:
    CPaintAlignment() : wxVScrolledWindow() {}
    CPaintAlignment(wxWindow *parent, CSeq_align_Handle ah, const vector<vector<vector<pair<TSeqPos,TSeqPos> > > > &feat_ranges, 
                    const vector< vector<pair<string,objects::CSeqFeatData::ESubtype> > > &feat_types,
                    const vector< vector<objects::CBioseq_Handle::EVectorStrand> > &feat_strand,
                    wxWindowID id = wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
    virtual ~CPaintAlignment();
    void OnKeyDown(wxKeyEvent& event);
    void OnMouseDrag(wxMouseEvent& evt);
    void OnMouseDown(wxMouseEvent& evt);
    void SetPos(int pos);
    void SetPosSeq(int pos);
    void SetTarget(int row);
    void EnableFeatures(bool enable);
    void EnableSubstitute(bool enable);
    bool IsSelection();
    pair<int,int> GetSelection(int row);
    int GetRow() { return m_visible_to_row[m_SelAlignRow];}
    int GetVisibleRow() {return m_SelAlignRow;}
    pair<int,int> GetAlignSelection(void);
    int GetTarget() { return m_visible_to_row[m_CursorAlign];}
    void SetAlign(CSeq_align_Handle ah);
    void RefreshWithScroll();
    virtual void OnPaint(wxPaintEvent& event);
    virtual void OnEraseBackground(wxEraseEvent& event);
    virtual wxCoord OnGetRowHeight( size_t row ) const;

    virtual void OnUpdate();
    void OnResize(wxSizeEvent&);
    void UpdateFeatures(const vector< vector<vector<pair<TSeqPos,TSeqPos> > > > &feat_ranges, 
                        const vector< vector<pair<string,objects::CSeqFeatData::ESubtype> > > &feat_types,
                        const vector< vector<objects::CBioseq_Handle::EVectorStrand> > &feat_strand);

    static string GetSeqTitle(CBioseq_Handle bsh);
    string GetExportInter();
    string GetExportCont();
    int AlignPosToSeqPos(int pos, int row, bool left) const;
    size_t GetTotalLength() const {return m_TotalLength;}
private:
    void NormalizeCursorColRows();
    void ScrollWithCursor();
    void SeqPosToColRow();
    void ColRowToSeqPos();
    unsigned int PosToSegment();
    void ClearScreen(wxGraphicsContext *gc);
    void CalculateFontWidthAndHeight(wxGraphicsContext *gc);
    int CalculateNumCols();
    void DrawTextLine(const unsigned int start, int &y_label, int row, int align_row, wxGraphicsContext *gc, unsigned int &seq_pos);
    void DrawTextCell(const unsigned int start, int col, int row, int align_row, unsigned int seq_pos, int x, int &y, 
                      const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const;
    void DrawCursor(int col, int row, int x, int &y, wxGraphicsContext *gc) const;
    const wxPen *GetColorForFeature(objects::CSeqFeatData::ESubtype subtype) const;
    void DrawFeatures(int row, int align_row, unsigned int seq_pos, int x, int &y, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const;
    void DrawFeatureLabels(int row, int align_row, int x, int &y, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const;
    int LeftMarginWidth() const;
    void DrawLineNumber(int align_row, unsigned int seq_pos, int x, int &y, wxGraphicsContext *gc) const;
    bool MouseToSeqPos(wxPoint p, int &row, int &y_row);
    int FindRowByCoord(int y, int &y_row);
    int TopMarginHeight() {return 5;}
    void DrawLabelCell(unsigned int pos, int x, int &y, int row, int align_row, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const;
    vector<unsigned int> GetFeaturesInRow(int row, int align_row) const;
    void GetFeatWholeRange();
    vector<unsigned int> IsExonPresent(int i, int row, int align_row) const;
    map<unsigned int, vector<unsigned int> > GetFeatsWithExons(int row, int align_row, const vector<unsigned int> &feats_in_row) const;
    void GetSegments();
    void DrawTopLineNumbers(int align_row, unsigned int seq_pos, int col, int x, int &y, wxGraphicsContext *gc) const;
    int SeqPosToAlignPos(int pos, int row, bool left);

    CSeq_align_Handle m_Alignment;
    vector<string> m_Seqs;
    size_t m_TotalLength;
    size_t m_NumAlignRows;
    vector<string> m_Labels;

    vector<vector<vector<pair<TSeqPos,TSeqPos> > > > m_FeatRanges;
    vector<vector<pair<string,CSeqFeatData::ESubtype> > > m_FeatTypes;
    vector<vector<CBioseq_Handle::EVectorStrand> > m_FeatStrand;

    vector<vector<pair<TSeqPos,TSeqPos> > > m_FeatWholeRange;
    vector<vector< vector<TSeqPos> > > m_FeatLengthBefore;
    vector< vector<TSeqPos> >m_FeatTotalLength;

    wxFont m_Font;
    int m_FontHeight, m_FontWidth;
    int m_NumRows, m_NumCols, m_CursorCol, m_CursorRow, m_LastRowLength, m_CursorSeq, m_CursorAlign;
    bool m_EnableFeatures, m_ContinueFeatures;
    bool m_EnableSubstitute;
    int m_DragMin, m_DragMax, m_SelAlignRow;
    CAlignmentAssistant *m_Parent;
    vector<CDense_seg::TDim> m_visible_to_row;
};



END_NCBI_SCOPE

#endif
    // _PAINT_SEQUENCE_H_
