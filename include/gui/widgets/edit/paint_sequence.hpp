/*  $Id: paint_sequence.hpp 44607 2020-01-31 21:01:07Z filippov $
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
#ifndef _PAINT_SEQUENCE_H_
#define _PAINT_SEQUENCE_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <objmgr/bioseq_handle.hpp>
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

#include <unordered_map>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

class CEditSequence;

class CPaintSequence : public wxVScrolledWindow
{
    DECLARE_DYNAMIC_CLASS( CPaintSequence )
    DECLARE_EVENT_TABLE()
public:
    CPaintSequence() : wxVScrolledWindow() {}
    CPaintSequence(wxWindow *parent, const string &seq, const vector<int> &seq_len, const vector<vector<pair<TSeqPos,TSeqPos> > > &feat_ranges, const vector<pair<string,objects::CSeqFeatData::ESubtype> > &feat_types,
                   const vector<objects::CBioseq_Handle::EVectorStrand> &feat_strand, const  vector<int> &feat_frames, const vector< CRef<objects::CGenetic_code> > &genetic_code, const vector<bool> &feat_partial5,
                   const string &allowed_char_set, const vector<string> &real_prot, const vector<bool> &read_only, const unordered_map<int, vector<vector<pair<TSeqPos,TSeqPos> > > > &prot_feat_ranges, const int start = 0,
                   wxWindowID id = wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
    virtual ~CPaintSequence();
    void InitPanel();
    void OnMouseClick(wxMouseEvent& evt);
    void OnKeyDown(wxKeyEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMouseDrag(wxMouseEvent& evt);
    void OnMouseDown(wxMouseEvent& evt);
    void OnCopy( wxCommandEvent& event );
    void OnCut( wxCommandEvent& event );
    void OnPaste( wxCommandEvent& event );
    string GetSeq();
    int GetLength();
    void SetPos(int pos);
    void SetRange(int pos1, int pos2);
    void Search(const string &val);
    vector<int> &GetSeqLen();
    vector<bool> &GetReadOnly() {return m_read_only;}
    void UpdateData();
    void EnableTranslation(bool enable);
    void EnableTranslation1(bool enable);
    void EnableTranslation2(bool enable);
    void EnableRevTranslation(bool enable);
    void EnableRevTranslation1(bool enable);
    void EnableRevTranslation2(bool enable);
    void EnableComplement(bool enable);
    void EnableFeatures(bool enable);
    void EnableOnTheFly(bool enable);
    void EnableMismatch(bool enable);
    bool IsClipboard();
    bool IsSelection();
    pair<int,int> GetSelection();
    virtual void OnPaint(wxPaintEvent& event);
    virtual void OnEraseBackground(wxEraseEvent& event);
    virtual wxCoord OnGetRowHeight( size_t row ) const;

    virtual void OnUpdate();
    void OnResize(wxSizeEvent&);
    void UpdateFeatures(const vector<vector<pair<TSeqPos,TSeqPos> > > &feat_ranges, 
                        const vector<pair<string,objects::CSeqFeatData::ESubtype> > &feat_types,
                        const vector<objects::CBioseq_Handle::EVectorStrand> &feat_strand,
                        const  vector<int> &feat_frames,
                        const vector< CRef<objects::CGenetic_code> > &genetic_code,
                        const vector<bool> &feat_partial5,
                        const vector<string> &real_prot,
                        const unordered_map<int, vector<vector<pair<TSeqPos,TSeqPos> > > > &prot_feat_ranges);
    void SetClean(bool clean);
    bool GetClean();
    string * GetFindString(bool is_nuc, bool is_revcomp, const string &choice);
    set<int>& SetHighlights();
    TSeqPos GetFeatureStart(int i, int j);
    TSeqPos GetFeatureStop(int i, int j);
    TSeqPos GetProtFeatureStart(int i, int k, int j) {return m_ProtFeatRanges[i][k][j].first;}
    TSeqPos GetProtFeatureStop(int i, int k, int j) {return m_ProtFeatRanges[i][k][j].second;}
private:
    void NormalizeCursorColRows();
    void InsertChar(int uc);
    void DeleteChar();
    void ScrollWithCursor();
    void SeqPosToColRow();
    void ColRowToSeqPos();
    unsigned int PosToSegment();
    void ClearScreen(wxGraphicsContext *gc);
    void CalculateFontWidthAndHeight(wxGraphicsContext *gc);
    int CalculateNumCols();
    void DrawTextLine(const string & substr, int &y_label, int row, wxGraphicsContext *gc, unsigned int &seq_pos);
    void DrawTextCell(const wxString &substr, int col, int row, unsigned int seq_pos, int x, int &y, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const;
    void DrawCursor(int col, int row, int x, int &y, wxGraphicsContext *gc) const;
    void DrawTranslation(int x, int &y, unsigned int seq_pos, wxGraphicsContext *gc) const;
    void DrawOffsetTranslation(int x, int y, unsigned int seq_pos, int offset, const string &prot, wxGraphicsContext *gc) const;
    void DrawComplement(int x, int &y, unsigned int seq_pos, wxGraphicsContext *gc) const;
    const wxPen *GetColorForFeature(objects::CSeqFeatData::ESubtype subtype) const;
    void DrawFeatures(int row, unsigned int seq_pos, int x, int &y, int y_base, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const;
    void DrawFeatureLabels(int row, int x, int &y, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const;
    int LeftMarginWidth();
    void DrawLineNumber(unsigned int seq_pos, int x, int &y, wxGraphicsContext *gc) const;
    void DrawOnTheFly(int x, int &y, int y_base, unsigned int seq_pos, int i, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const;
    char TranslateOnTheFly(unsigned int seq_pos, int i, const vector<unsigned int> &ranges, const vector<string> & translation, bool &left, bool &right) const;
    void DrawMismatch(int x, int &y, int y_base, unsigned int seq_pos, int i, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const;
    bool MouseToSeqPos(wxPoint p, int &row, int &y_row);
    void DrawTripletMismatch(int x, int y, int y_base, int i, wxGraphicsContext *gc) const;
    void DrawComplementLabel(int row, int x, int &y, wxGraphicsContext *gc) const;
    void DrawOnTheFlyLabel(int x, int &y, bool is_exon_present, wxGraphicsContext *gc) const;
    void DrawTranslationLabels(int row, int x, int &y, wxGraphicsContext *gc) const;
    void DrawMismatchLabel(int x, int &y, bool is_exon_present, wxGraphicsContext *gc) const;
    void TranslateCDS();
    void MouseToFeature(wxPoint p, int row, int y0);
    void AdjustFeatureRange(int origin, int offset);
    void AdjustProtFeatRange(int origin, int offset, int i);
    void AdjustFeatStartStop(int origin, int offset, int &start, int &stop);
    int FindRowByCoord(int y, int &y_row);
    int TopMarginHeight() {return 5;}
    void DrawLabelCell(unsigned int pos, int x, int &y, int row, const vector<unsigned int> &feats_in_row, const map<unsigned int, vector<unsigned int> > &feats_with_exons, wxGraphicsContext *gc) const;
    vector<unsigned int> GetFeaturesInRow(int row) const;
    void GetFeatWholeRange();
    vector<unsigned int> IsExonPresent(int i, int row) const;
    map<unsigned int, vector<unsigned int> > GetFeatsWithExons(int row, const vector<unsigned int> &feats_in_row) const;
    void RefreshWithScroll();
    string CutSelection();
    void SetStartPos(void);
  
    string m_Seq, m_Prot, m_Prot1, m_Prot2, m_Complement, m_RevProt, m_RevProt1, m_RevProt2;
    vector<int> m_SeqLen;
    vector<vector<pair<TSeqPos,TSeqPos> > > m_FeatRanges;
    unordered_map<int, vector<vector<pair<TSeqPos,TSeqPos> > > > m_ProtFeatRanges;
    vector<pair<string,objects::CSeqFeatData::ESubtype> > m_FeatTypes;
    vector<int> m_FeatFrames;
    vector< CRef<objects::CGenetic_code> >  m_GeneticCode;
    vector<bool> m_Feat5Partial;
    vector<pair<TSeqPos,TSeqPos> > m_FeatWholeRange;
    vector< vector<TSeqPos> > m_FeatLengthBefore;
    vector<TSeqPos> m_FeatTotalLength;
    vector<string> m_Translated;
    vector<objects::CBioseq_Handle::EVectorStrand> m_FeatStrand;
    wxFont m_Font;
    int m_FontHeight, m_FontWidth;
    int m_NumRows, m_NumCols, m_CursorCol, m_CursorRow, m_LastRowLength, m_CursorSeq;
    bool m_EnableTranslation, m_EnableTranslation1, m_EnableTranslation2, m_EnableComplement, m_EnableFeatures, m_ContinueFeatures, m_EnableOnTheFly,m_EnableMismatch;
    bool m_EnableRevTranslation, m_EnableRevTranslation1, m_EnableRevTranslation2;
    string m_AllowedCharSet;
    vector<string> m_RealProt;
    int m_ShowTriplet;
    int m_DragMin, m_DragMax;
    CEditSequence *m_Parent;
    int m_ShowTripletMismatch;
    int m_ShowTripletTranslation;
    bool m_Clean;
    set<int> m_highlights;
    pair<int,int> m_FeatureStart, m_FeatureStop;
    vector<bool> m_read_only;
    int m_start;
    bool m_Down;
};



END_NCBI_SCOPE

#endif
    // _PAINT_SEQUENCE_H_
