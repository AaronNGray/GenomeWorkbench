/* $Id: splign_exon_trim.cpp 517941 2016-10-28 20:00:34Z kiryutin $
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
* Author: Boris Kiyutin
*
* File Description: exon trimming (alignment post processing step)
*                   
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include "messages.hpp"
#include <algo/align/nw/align_exception.hpp>
#include "splign_exon_trim.hpp"

#include <objmgr/feat_ci.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

//check if the exon segments[p] abuts another exon in genomic coordinates, right side
bool CSplignTrim::HasAbuttingExonOnRight(TSegs segments, TSeqPos p)
{
    TSeqPos len = segments.size();
    TSeqPos np = p+1;
    for( ; np < len; ++np) {
        if( segments[np].m_exon ) break;
    }
    if(np == len) {// no exons on the right found
        return false;
    } 
    if( segments[p].m_box[3] + 1 == segments[np].m_box[2] ) { //abutting
        return true;
    }
    return false;
}

//check if the exon segments[p] abuts another exon in genomic coordinates, right side
bool CSplignTrim::HasAbuttingExonOnLeft(TSegs segments, TSeqPos p)
{
    int pp = (int)p-1;
    for( ; pp>=0; --pp ) {
        if( segments[pp].m_exon ) break;
    }
    if(pp < 0) {//no exons found on the left
        return false;
    }
    if( segments[pp].m_box[3] + 1 == segments[p].m_box[2] ) { //abutting
        return true;
    }
    return false;
}    

//legacy check
//it two short throws away and returns true
//otherwise returns false
bool CSplignTrim::ThrowAwayShortExon(TSeg& s)
{
    const size_t min_query_size = 4;
    if( int(s.m_box[1] - s.m_box[0] + 1) < int(min_query_size)) {
        s.SetToGap();
        return true;
    }
    return false;
}    

bool CSplignTrim::ThrowAway20_28_90(TSeg& s)
{
    if( s.m_len < 20 || // 20 rule
        ( s.m_idty < 0.9 && s.m_len < 28 ) ) {// 28_90 rule
        s.SetToGap();
        return true;
    }
    return false;
}

// aka stich holes
//joins exons segments[p1] and segments[p1] into a singe exon
//everithing in between becomes a regular gap in query adjacent to a regular gap in subject 
void CSplignTrim::JoinExons(TSegs& segments, TSeqPos p1, TSeqPos p2)
{
    //sanity check
    if( p1 >= segments.size() ) return;
    if( p2 >= segments.size() ) return;
    if( !segments[p1].m_exon ) return;
    if( !segments[p2].m_exon ) return;
    size_t pos1 = min( p1, p2);
    size_t pos2 = max( p1, p2);
    if( segments[pos1].m_box[1] >= segments[pos2].m_box[0] ||
        segments[pos1].m_box[3] >= segments[pos2].m_box[2] ) {
        return; // segments intersect
    }

    //join

    TSegs new_segments;
    for( size_t pos = 0; pos < pos1; ++pos) {
        new_segments.push_back(segments[pos]);
    }
    //joint exon
    TSeg s(segments[pos1]);
    s.m_box[1] = segments[pos2].m_box[1];
    s.m_box[3] = segments[pos2].m_box[3];
    if( segments[pos1].m_box[1] + 1 < segments[pos2].m_box[0]) {
        s.m_details.append(segments[pos2].m_box[0] - segments[pos1].m_box[1] - 1, 'D');
    }
    if( segments[pos1].m_box[3] + 1 < segments[pos2].m_box[2]) {
        s.m_details.append(segments[pos2].m_box[2] - segments[pos1].m_box[3] - 1, 'I');
    }
    s.m_details += segments[pos2].m_details;
    Update(s);
    new_segments.push_back(s);
    //write the rest
    for( size_t pos = ++pos2; pos < segments.size(); ++pos) {
        new_segments.push_back(segments[pos]);
    }
    
    segments.swap(new_segments);
}

//trims exons around internal alignment gaps to complete codons
//if CDS can be retrieved from bioseq
void CSplignTrim::TrimHolesToCodons(TSegs& segments, CBioseq_Handle& mrna_bio_handle, bool mrna_strand, size_t mrna_len)
{

    if( mrna_bio_handle ) {
        //collect CDS intervals (could be more than one in a case of ribosomal slippage)
        vector<TSeqRange> tr;
        for(CFeat_CI ci(mrna_bio_handle, SAnnotSelector(CSeqFeatData::e_Cdregion)); ci; ++ci) {
            for(CSeq_loc_CI slit(ci->GetLocation()); slit; ++slit) {
                TSeqRange r, ori;
                ori = slit.GetRange();
                if( mrna_strand ) {
                    r = ori;
                } else {//reverse
                    r.SetFrom(mrna_len - ori.GetTo() - 1);
                    r.SetTo(mrna_len - ori.GetFrom() - 1);
                }
                tr.push_back(r);
            }
        }

        if(tr.empty()) return;// CDS not found

        //trim
        AdjustGaps(segments);//make sure there is no adjacent gaps
        size_t pos1 = 0, pos2 = 2;
        for(; pos2 < segments.size(); ++pos1, ++pos2) {
            if( segments[pos1].m_exon && !segments[pos1+1].m_exon && segments[pos2].m_exon ) {//candidate for trimming
                
                //trim left exon    
                TSeqPos p1 = segments[pos1].m_box[1];
                ITERATE(vector<TSeqRange>, it, tr) {
                    if( p1 >= it->GetFrom() && p1 <= it->GetTo() ) {
                        TSeqPos cut_mrna_len = (p1 + 1 - it->GetFrom()) % 3, cnt = 0;
                        string transcript = segments[pos1].m_details;
                        int i = (int)transcript.size() - 1;
                        for(; i>=0; --i) {
                            if( cnt%3 == cut_mrna_len &&  transcript[i] == 'M' ) { //cut point  
                                CutFromRight(transcript.size() - i - 1, segments[pos1]);
                                break;
                            }
                            if( transcript[i] != 'I' ) ++cnt;
                        }
                        if( i < 0 ) {// exon should not be so bad   
                            NCBI_THROW(CAlgoAlignException, eInternal, g_msg_InvalidRange);
                        }
                        break;
                    }
                }
                
                //trim right exon   
                TSeqPos p2 =  segments[pos2].m_box[0];
                ITERATE(vector<TSeqRange>, it, tr) {
                    if( p2 >= it->GetFrom() && p2 <= it->GetTo() ) {
                        TSeqPos cut_mrna_len = ( 3 - ( p2 - it->GetFrom()) % 3  ) %3, cnt = 0;
                        string transcript = segments[pos2].m_details;
                        int i = 0;
                        for( ; i < (int)transcript.size(); ++i) {
                            if( cnt%3 == cut_mrna_len && transcript[i] == 'M' ) { //cut point   
                                CutFromLeft(i, segments[pos2]);
                                break;
                            }
                            if( transcript[i] != 'I' ) ++cnt;
                        }
                        if( i == (int)transcript.size() ) {// exon should not be so bad 
                            NCBI_THROW(CAlgoAlignException, eInternal, g_msg_InvalidRange);
                        }
                        break;
                    }
                }
            }
        }
        AdjustGaps(segments);
    }
}


// updates m_annot for a segment based on SSegment::m_box and CSplignTrim:m_seq
void CSplignTrim::UpdateAnnot(TSeg& s)
{
    if(s.m_exon) {
        s.m_annot = "  <exon>  ";
        if( s.m_box[2] > (size_t)m_seqlen ) {
            NCBI_THROW(CAlgoAlignException, eInternal, g_msg_InvalidRange);
        }
        if( s.m_box[2] > 1 ) {
            s.m_annot[0] = toupper(m_seq[s.m_box[2] - 2]);
        }
        if( s.m_box[2] > 0 ) {
            s.m_annot[1] = toupper(m_seq[s.m_box[2] - 1]);
        }
        if( s.m_box[3] + 2 < (size_t)m_seqlen ) {
            s.m_annot[9] = toupper(m_seq[s.m_box[3] + 2]);
        }
        if( s.m_box[3] + 1 < (size_t)m_seqlen ) {
            s.m_annot[8] = toupper(m_seq[s.m_box[3] + 1]);
        }
    } else {
        s.m_annot = "<GAP>";
    }
}

// implies s.exon, s.m_box, and s.m_details are correct
// updates the rest of segment fields including m_annot
void CSplignTrim::Update(TSeg& s)
{
    if(s.m_exon) {
        UpdateAnnot(s);
        s.Update(m_aligner.GetNonNullPointer());
    } else {
        s.SetToGap();
    }
}
            

void CSplignTrim::AdjustGaps(TSegs& segments)
{
    TSegs new_segments;
        int gap_start_idx (-1);
        if(segments.size() && segments[0].m_exon == false) {
            gap_start_idx = 0;
        }

        for(size_t k (0); k < segments.size(); ++k) {
            TSeg& s (segments[k]);
            if(!s.m_exon) {
                if(gap_start_idx == -1) {
                    gap_start_idx = int(k);
                    if(k > 0) {
                        s.m_box[0] = segments[k-1].m_box[1] + 1;
                        s.m_box[2] = segments[k-1].m_box[3] + 1;
                    }
                }
            }
            else {
                if(gap_start_idx >= 0) {
                    TSeg& g = segments[gap_start_idx];
                    g.m_box[1] = s.m_box[0] - 1;
                    g.m_box[3] = s.m_box[2] - 1;
                    g.m_len = g.m_box[1] - g.m_box[0] + 1;
                    g.m_details.resize(0);
                    new_segments.push_back(g);
                    gap_start_idx = -1;
                }
                new_segments.push_back(s);
            } 
        }

        if(gap_start_idx >= 0) {
            TSeg& g (segments[gap_start_idx]);
            g.m_box[1] = segments[segments.size()-1].m_box[1];
            g.m_box[3] = segments[segments.size()-1].m_box[3];
            g.m_len = g.m_box[1] - g.m_box[0] + 1;
            g.m_details.resize(0);
            new_segments.push_back(g);
        }

        segments.swap(new_segments);
}
    
void CSplignTrim::CutFromLeft(size_t len, TSeg& s) //len is length on alignment to cut
{
    if(len == 0) return;

    if(ThrowAwayShortExon(s)) return;

    string::iterator irs, irs0, irs1;
    irs0 = s.m_details.begin();
    irs1 = s.m_details.end();

    if(irs1 - irs0 <= (int)len) {//cut all
        s.SetToGap();
        return;
    }
    irs1 = irs0 + len;

    int i0 = 0, i1 = 0;
    for(irs = irs0; irs != irs1; ++irs) {        
        switch(*irs) {            
        case 'M': 
        case 'R': {
            ++i0;
            ++i1;
        }
        break;
        case 'I': {
            ++i1;
        }
        break;
        case 'D': {
            ++i0;
        }
        }
    }

    // resize   
    s.m_box[0] += i0;
    if(ThrowAwayShortExon(s)) return;//too short 
    s.m_box[2] += i1;
    s.m_details.erase(0, len);
    s.Update(m_aligner.GetNonNullPointer());
    
    // update the first two annotation symbols      
    if(s.m_annot.size() > 2 && s.m_annot[2] == '<') {
        int  j1 = int(s.m_box[2]) - 2;
        char c1 = j1 >= 0? m_seq[j1]: ' ';
        s.m_annot[0] = c1;
        int  j2 = int(s.m_box[2]) - 1;
        char c2 = j2 >= 0? m_seq[j2]: ' ';
        s.m_annot[1] = c2;
    }
}

void CSplignTrim::CutFromRight(size_t len, TSeg& s) //len is length on alignment to cut
{
    if(len == 0) return;

    if(ThrowAwayShortExon(s)) return;

    string::reverse_iterator irs, irs0, irs1;
    irs0 = s.m_details.rbegin();
    irs1 = s.m_details.rend();

    if(irs1 - irs0 <= (int)len) {//cut all
        s.SetToGap();
        return;
    }
    irs1 = irs0 + len;

    int i0 = 0, i1 = 0;
    for(irs = irs0; irs != irs1; ++irs) {        
        switch(*irs) {            
        case 'M': 
        case 'R': {
            ++i0;
            ++i1;
        }
        break;
        case 'I': {
            ++i1;
        }
        break;
        case 'D': {
            ++i0;
        }
        }
    }

    // resize   
    s.m_box[1] -= i0;
    if(ThrowAwayShortExon(s)) return;//too short 
    s.m_box[3] -= i1;
    s.m_details.resize(s.m_details.size() - len);
    s.Update(m_aligner.GetNonNullPointer());
    
    // update the last two annotation chars
    const size_t adim = s.m_annot.size();
    if(adim > 2 && s.m_annot[adim - 3] == '>') {

        const char c3 (s.m_box[3] + 1 < (size_t)m_seqlen? m_seq[s.m_box[3] + 1]: ' ');
        const char c4 (s.m_box[3] + 2 < (size_t)m_seqlen? m_seq[s.m_box[3] + 2]: ' ');
        s.m_annot[adim-2] = c3;
        s.m_annot[adim-1] = c4;
    }
}

void CSplignTrim::CutToMatchLeft(TSeg& s)
{
    size_t pos = s.m_details.find('M');
    if(pos == string::npos) {
        s.SetToGap();
        return;
    }
    if(pos > 0) {
        CutFromLeft(pos, s);
    }
}

void CSplignTrim::CutToMatchRight(TSeg& s)
{
    size_t pos = s.m_details.rfind('M');
    if(pos == string::npos) {
        s.SetToGap();
        return;
    }
    size_t len = s.m_details.length() - pos - 1;//length to cut
    if(len > 0) {
        CutFromRight(len, s);
    }
}

void CSplignTrim::Cut50FromLeft(TSeg& s)
{
    int score = 0, maxscore = -2;
    string::reverse_iterator rirs0, rirs1, rirs, rirs_max;
    rirs0 = s.m_details.rbegin();
    rirs1 = s.m_details.rend();
    rirs_max = rirs0;
    for(rirs = rirs0; rirs != rirs1; ++rirs) {
        if(*rirs == 'M') {
            ++score;
        } else {
            --score;
        }
        if(score >= maxscore) {
            maxscore = score;
            rirs_max = rirs;
        }
    }
    int len = rirs1 - rirs_max - 1;
    if(len > 0) {
        CutFromLeft(len, s);
    }    
}

void CSplignTrim::Cut50FromRight(TSeg& s)
{
    int score = 0, maxscore = -2;
    string::iterator irs0, irs1, irs, irs_max;
    irs0 = s.m_details.begin();
    irs1 = s.m_details.end();
    irs_max = irs0;
    for(irs = irs0; irs != irs1; ++irs) {
        if(*irs == 'M') {
            ++score;
        } else {
            --score;
        }
        if(score >= maxscore) {
            maxscore = score;
            irs_max = irs;
        }
    }
    int len = irs1 - irs_max - 1;
    if(len > 0) {
        CutFromRight(len, s);
    }
    
}

void CSplignTrim::ImproveFromLeft(TSeg& s)
{    
    CutToMatchLeft(s);
    Cut50FromLeft(s);
    if(ThrowAwayShortExon(s)) return;    

    int len_total = (int)s.m_details.size();
    if(len_total <= 20) return;//two short

    //compute number of matches
    int match_total = 0;
    string::iterator irs0 = s.m_details.begin(),
        irs1 = s.m_details.end(), irs;

    for(irs = irs0; irs != irs1; ++irs) {
        if(*irs == 'M') {
            ++match_total;
        }
    }

    //find the right boundary, 20/20 rule
    {{
      int minlen = max(20, len_total/5);
      size_t pos = irs1 - irs0 - minlen;
      pos = s.m_details.rfind('M', pos);
      if( pos == string::npos ) return;//no M found. Should not happen if  CutToMatchLeft is called above
      pos = s.m_details.find_last_not_of('M', pos); 
      if( pos == string::npos ) return;// 100% id on the left, nothing to trim
      irs1 = irs0 + pos + 1; 
    }}

    //after 20/20 *irs1 is M, irs1-1 is not M and eventually irs1 is a right boundary for trimming 
    
   
    string::iterator irs_tr = s.m_details.end(); //trimming point
    int match = 0, len = 0;

    for(irs = irs0; irs != irs1; ++irs) {        
        if(*irs == 'M') {
            ++match;
        }
        ++len;
        double lid = match / (double)len;
        double rid = (match_total - match) / (double)(len_total - len);

        //dropoff check
        double epsilon = 1e-10;
        if( rid - lid - m_MaxPartExonIdentDrop > epsilon ) {
            irs_tr = irs;
            //do not count trimmed part, adjust values
            match_total -= match;
            len_total -= len;
            match = 0;
            len = 0;
        }
    }            

    if(irs_tr == s.m_details.end()) return;//nothing to trim

    //actual trimming
    CutFromLeft( irs_tr - irs0 + 1, s );
    ThrowAwayShortExon(s);
}

// try improving the segment by cutting it from the left, 20/20 rule
void CSplignTrim::ImproveFromRight(TSeg& s)
{
    CutToMatchRight(s);
    Cut50FromRight(s);
    if(ThrowAwayShortExon(s)) return;    

    int len_total = (int)s.m_details.size();
    if(len_total <= 20) return;//two short

    //compute number of matches
    int match_total = 0;
    string::reverse_iterator irs0 = s.m_details.rbegin(),
        irs1 = s.m_details.rend(), irs;

    for(irs = irs0; irs != irs1; ++irs) {
        if(*irs == 'M') {
            ++match_total;
        }
    }

    //find the left boundary, 20/20 rule
    {{
      size_t pos = max(20, len_total/5) - 1;
      pos = s.m_details.find('M', pos);
      if( pos == string::npos ) return;//no M found. 
      pos = s.m_details.find_first_not_of('M', pos); 
      if( pos == string::npos ) return;// 100% id on the right, nothing to trim
      irs1 = irs1 - pos; 
    }}

    //after 20/20 *ir1s is M, irs1+1 is not M and eventually irs1 is a left boundary for trimming 
    
   
    string::reverse_iterator irs_tr = s.m_details.rend(); //trimming point
    int match = 0, len = 0;

    for(irs = irs0; irs != irs1; ++irs) {        
        if(*irs == 'M') {
            ++match;
        }
        ++len;
        double rid = match / (double)len;
        double lid = (match_total - match) / (double)(len_total - len);

        //dropoff check
        double epsilon = 1e-10;
        if( lid - rid - m_MaxPartExonIdentDrop > epsilon ) {
            irs_tr = irs;
            //do not count trimmed part, adjust values
            match_total -= match;
            len_total -= len;
            match = 0;
            len = 0;
        }
    }            

    if( irs_tr == s.m_details.rend() ) return;//no trimming point found

    //actual trimming
    CutFromRight( irs_tr - irs0 + 1 , s );
    ThrowAwayShortExon(s);
}

END_NCBI_SCOPE
