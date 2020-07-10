/*  $Id: table_import_data_source.cpp 43697 2019-08-14 19:35:51Z katargir $
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
 * Authors: Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/table_import_data_source.hpp>
#include <corelib/ncbistre.hpp>
#include <corelib/ncbifile.hpp>

#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/CommonString_table.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqfeat/SeqFeatData_.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>

#include <functional>
#include <math.h>
#include <sstream>


BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

/*****************************************************************************/
/*************************** CCharHistogram **********************************/

CCharHistogram::CCharHistogram() 
: m_RowCount(0), m_CharCount(0), m_CaptureTarget(100) 
{
    // Initialize objects used to check for repeated characters or
    // character combinations in table entries.
    CMergedChar space_repeat(256);
    space_repeat.m_Chars = " ";
    m_Repeats.push_back(space_repeat);

    CMergedChar tab_repeat(257);
    tab_repeat.m_Chars = "\t";
    m_Repeats.push_back(tab_repeat);

    CMergedChar whitespace_repeat(258);   
    whitespace_repeat.m_Chars = " \t";
    m_Repeats.push_back(whitespace_repeat);
}

void CCharHistogram::UpdateHistogram(const string& row)
{
    std::vector<int> buf(s_NumDelimiters, 0);
    size_t i, j;

    if (row.size() >= m_CharFrequency.size())
        x_UpdateMaxRowLen((int)row.size());


    // Add to the total number of characters in all rows (used to compute
    // average row length)
    m_CharCount += (int) row.size();
    // Update total number of rows used in computing the statistics
    ++m_RowCount;

    // Initialize checks for repeated characters
    for (j=0; j<m_Repeats.size(); ++j) {
        m_Repeats[j].NewLine();
    }

    // Count the number of occurances for all characters in row "row" and
    // save them in 'buf' - only consider ascii characters
    for (i=0; i<row.size(); ++i) {
        unsigned int idx = (unsigned int) row[i];
        if (idx <256) {
            buf[idx] += 1;

            // perform check for repeated characters
            for (j=0; j<m_Repeats.size(); ++j) {
                if (m_Repeats[j].NextChar(row[i]))
                    buf[m_Repeats[j].m_FreqArrayIdx] += 1;
            }
        }
    }


    // Given the number of times each character appears in the current
    // row, update the corresponding row occurance count for that character
    for (i=0; i<(size_t)s_NumDelimiters; ++i) {
        int occurances = buf[i];      
        m_CharFrequency[occurances][i] += 1;
    }

    /// Record some rows in case the initial delimiter analysis gives
    /// more than one option.  We might want to extend this to randomly
    /// updating from different parts of the stream (after accumulating the
    /// first 'm_CaptureCount' rows).
    if (m_RecordedRows.size() < m_CaptureTarget)
        m_RecordedRows.push_back(row);
}

void CCharHistogram::UpdateHistogramWithDelim(const string& row, char delim)
{
    std::vector<int> buf(s_NumDelimiters, 0);    
    size_t i, j;

    if (row.length() > m_CharFrequency.size())
        x_UpdateMaxRowLen((int)row.length());

    // Add to the total number of characters in all rows (used to compute
    // average row length)
    m_CharCount += (int) row.size();
    // Update total number of rows used in computing the statistics
    ++m_RowCount;

    // Initialize checks for repeated characters
    for (j=0; j<m_Repeats.size(); ++j) {
        m_Repeats[j].NewLine();
    }

    // Update occurance values for all occurances not between quotes 'delim'
    bool quoted = false;
    for (i=0; i<row.size(); ++i) {

        /// Start quoted string
        if (!quoted && row[i] == delim) {
            quoted = true;
            continue;
        }

        // In a quoted string, a new quote mark either stops it unless it is
        // escaped. In CSV format, quotes are escaped as by doubling them, e.g.
        // "".  If quotes are escaped in another way, e.g. \" we will not
        // recognize them.
        if (quoted && row[i] == delim) {
            if (i+1 < row.size() && row[i+1] == '"')
                ++i;
            else quoted = false;
            continue;
        }

        if (!quoted) {

            unsigned int idx = (unsigned int) row[i];
            if (idx < 256) {
                buf[idx] += 1;

                // perform check for repeated characters
                for (j=0; j<m_Repeats.size(); ++j) {
                    if (m_Repeats[j].NextChar(row[i]))
                        buf[m_Repeats[j].m_FreqArrayIdx] += 1;
                }
            }
        }
    }


    for (i=0; i<(size_t)s_NumDelimiters; ++i) {
        int occurances = buf[i];      
        m_CharFrequency[occurances][i] += 1;
    }

    /// Record some rows in case the initial delimiter analysis gives
    /// more than one option.  We might want to extend this to randomly
    /// updating from different parts of the stream (after accumulating the
    /// first 'm_CaptureCount' rows).
    if (m_RecordedRows.size() < m_CaptureTarget)
        m_RecordedRows.push_back(row);
}

void CCharHistogram::x_UpdateMaxRowLen(int len) 
{ 
    while (m_CharFrequency.size() <= (size_t)len) {
        vector<int> zerovec(s_NumDelimiters, 0);
        m_CharFrequency.push_back(zerovec);
    }   
}

void CCharHistogram::GetGroupOccuranceAverage(char from_char, 
                                              char to_char,
                                              float& certainty, 
                                              int& first_non_header) const
{  
    certainty = 0.0f;
    first_non_header = -1;

    // Need some data to analyze...
    if (m_InitialRows.size() < 5) {
        return;
    }

    float num_rows = (float)m_RowCount;   

    // average number of occurances of each character over selected rows
    int num_chars = to_char - from_char + 1;
    vector<float> mean_occurances(num_chars, 0.0f);    
    vector<char> chars(num_chars, ' ');

    // For each character, determine how clustered it is (determine if the majority
    // of rows have the same number of occurances of a character).
    size_t i;
    size_t j;
    size_t k;
    for (i=(size_t)from_char; i<=(size_t)to_char; ++i) { 
        // Compute mean number of occurances for the current character (the average
        // number of times the character appears in a row over all rows)
        for (j=0; j<m_CharFrequency.size(); ++j) {
            // Number of occurances in a row (j) * number of rows that have that
            // number of occurances (m_CharFrequency[j][i]) divided by total
            // number of rows (num_rows).
            mean_occurances[i-from_char] += (((float)j)*((float)m_CharFrequency[j][i]))/num_rows;
        }
        chars[i-from_char] = (char)i;
    }

    // Determine how close all occurances are to the mean (best result
    // is for all rows have the same number of occurances) for current ascii
    // character 'i'.  Start at 1 to ignore (penalize) rows with 0 occurances
    vector<float> mean_delta;
    for (i=0; i<m_InitialRows.size(); ++i) {
        /// Find number of occurances for (from_char..to_char) in this row
        vector<float> row_occurances(num_chars, 0.0f);

        for (j=0; j<m_InitialRows[i].size(); ++j) {
            char c = m_InitialRows[i][j];
            if (c>=from_char && c<=to_char) {
                row_occurances[c-from_char] += 1.0f;
            }         
        }

        // This would be better weighted if we took into account the
        // distribution of occurances by character for non-header
        // rows. (e.g. if a character appears a consistent # of times
        // in non-header rows, weight that more heavily).
        float abs_delta = 0.0f;
        for (k=0; k<(size_t)num_chars; ++k) {
            float delta = row_occurances[k] - mean_occurances[k];
            if (delta < 0.0f)
                delta *= -1.0f;
            abs_delta += delta;           
        }
        //_TRACE("Row: " << i << " delta: " << abs_delta);

        // get delta as an absolute %
        mean_delta.push_back(abs_delta);        
    }

    //*************************************************************************
    // This is good for weighing deltas of individual characters, but we 
    // could also look at occurances of groups, specifically numeric vs non-
    // numeric
    //*************************************************************************

    // Find range of normal deltas in score for lines in second half of 
    // m_InitialRows (which should all be non-headers)
    float normal_delta = 0.0f;
    float max_normal_delta = 0.0f;
    float avg_row_len = 0.0f;
    float avg_row_count = 0;

    for (j=mean_delta.size()-1; j>=mean_delta.size()/2; --j) {
        if (mean_delta[j] > max_normal_delta) {
            max_normal_delta = mean_delta[j];
        }
        normal_delta += mean_delta[j]*(1.0f/(float)(mean_delta.size()/2));
        avg_row_len += (float)m_InitialRows[j].size();
        avg_row_count += 1.0f;
    }

    avg_row_len /= avg_row_count;

    // Find first row at which row scores become similar to the 'normal' rows.
    // check two rows for consistency.
    for (j=0; j<mean_delta.size()-2; ++j) {
        // Get delta row lengths of current row with average row as an absolute % (0..1)
        float row_len_delta = 1.0f - ((float)m_InitialRows[j].size())/avg_row_len;
        if (row_len_delta < 0.0f) 
            row_len_delta *= -1.0f;

        float row_len_delta1 = 1.0f - ((float)m_InitialRows[j+1].size())/avg_row_len;
        if (row_len_delta1 < 0.0f) 
            row_len_delta1 *= -1.0f;

        if (mean_delta[j] + row_len_delta*5 <= max_normal_delta*1.6f &&
            mean_delta[j+1] + row_len_delta1*5 <= max_normal_delta*1.6f) {
            first_non_header = j;
            break;
        }
    }

    // for scoring validate that deltas on one side
    // of mean_delta_index are generally higher than on the 
    // other side.
    float prev_delta_avg = 0.0f;   
    float post_delta_avg = 0.0f;

    if (first_non_header > 0 && 
        first_non_header < (int)mean_delta.size()-1) {
        for (j=0; j<mean_delta.size(); ++j) {
            if (j<(size_t)first_non_header) {
                prev_delta_avg += mean_delta[j]*(1.0f/(float)first_non_header);
            }
            else if (j>(size_t)first_non_header) {
                post_delta_avg += mean_delta[j]*(1.0f/(float)(mean_delta.size()-first_non_header));
            }
        }
    }

    // Larger is better (delta in header should be higher than delta after).  Below approx 1.4
    // probably implies no header).
    if (post_delta_avg > 0)
        certainty = prev_delta_avg/post_delta_avg;
    else
        certainty = prev_delta_avg;
}

void CCharHistogram::GetDelimiterProbablities(float& max_score, 
                                              vector<char>& delims,
                                              NStr::EMergeDelims& merge)
{
    std::vector<CDelimScore> m_Scores;

    float max_frequency_score = 0.0f;
    max_score = -1.0f;    

    // For each character, determine how clustered it is (determine if the majority
    // of rows have the same number of occurances of a character).
    size_t i;
    for (i=0; i<(size_t)s_NumDelimiters; ++i) {        
        size_t j;
        float num_rows = (float)m_RowCount;
        float num_rows_inv = 1.0f/num_rows;

        int merged_delims;
        if (i>=256)
            merged_delims = true;
            

        // average number of occurances of current character over all rows
        float mean_occurance = 0.0f;
        float score = 0;

        // Compute mean number of occurances for the current character (the average
        // number of times the character appears in a row over all rows)
        for (j=0; j<m_CharFrequency.size(); ++j) {
            // Number of occurances in a row (j) * number of rows that have that
            // number of occurances (m_CharFrequency[j][i]) divided by total
            // number of rows (num_rows).
            mean_occurance += (((float)j)*((float)m_CharFrequency[j][i]))/num_rows;
        }

        // Determine how close all occurances are to the mean (best result
        // is for all rows have the same number of occurances) for current ascii
        // character 'i'.  Start at 1 to ignore (penalize) rows with 0 occurances
        for (j=1; j<m_CharFrequency.size(); ++j) {
            float occurances = (float)j;

            // get abs(delta)
            double delta = (double)(occurances-mean_occurance);
            if (delta < 0.0) delta *= -1.0;

            score += (((float)m_CharFrequency[j][i])/(1.0f + (float)pow(delta,2.0)))*num_rows_inv;
        }

        // Generally, consistent characters with higher mean occurances are better.
        // This is because one or two random special characters can appear at a
        // certain point in every field giving them a high consistency score.        
        CDelimScore char_score(i);
        char_score.m_OccuranceScore = score;
        char_score.m_FrequencyScore = mean_occurance;

        m_Scores.push_back(char_score);

        if (mean_occurance > max_frequency_score)
            max_frequency_score = mean_occurance;
    }

    // Normalize the frequency score to 0..1.  Having the most characters per row
    // isn't necessarily better than having a few - this measure is meant primarily 
    // to filter out characters that only occur one or two per row.  We max out
    // frequency score to 1.0 when it reaches "average row length"/8
    float desired_frequency = ((float)(m_CharCount/m_RowCount))/8.0f;

    // If the max frequency is highter than the 'desired' frequency, use
    // the desired (best-guess) frequency.
    desired_frequency = std::min(max_frequency_score, desired_frequency);

    // So in the end anything with a frequency count at or over once per 8 chars
    // gets a frequency score of 1.
    for (i=0; i<(size_t)s_NumDelimiters; ++i) {
        m_Scores[i].m_FrequencyScore = std::min(m_Scores[i].m_FrequencyScore, 
            desired_frequency)/desired_frequency;      
    }

    
    // The last thing we check is how effectively the delimiters
    // break rows up into fields. One reason is this:
    // my | dog | has | fleas | | |
    // is the delimiter space or pipe? Obviously pipe but the measures so far
    // don't enforce that (space and pipe score the same on frequency and 
    // occurances.  With longer fields, space could win on frequency.)

    // To check for this, pick highest scoring delimiters and then tokenize
    // the saved rows with each of them and check the results for token length.

    // Compute a combined score and then sort potential character delimiters
    // by the combined score. 
    for (i=0; i<m_Scores.size(); ++i) {
        m_Scores[i].m_CombinedScore = (m_Scores[i].m_OccuranceScore + 
            m_Scores[i].m_FrequencyScore*0.25f)/1.25f;
    }

    int delim_candidates_count = 0;
    std::sort(m_Scores.begin(), m_Scores.end(), greater<CDelimScore>());

    /// Get number of delimiters that score over a set value (0.8):
    for (i=0; i<m_Scores.size(); ++i) {
        if (m_Scores[i].m_CombinedScore > 0.0f)
//            _TRACE("Score for char: " << m_Scores[i].m_DelimChar << " is: (" << 
//                 m_Scores[i].m_OccuranceScore << ", " <<  m_Scores[i].m_FrequencyScore << ", " << 
//                 m_Scores[i].m_CombinedScore << ")" );

        // This is a relatively low score, but we should support delimiters even
        // with relatively inconsistent data
        if (m_Scores[i].m_CombinedScore > 0.5f)
            ++delim_candidates_count;
    }

    // If there is more than 1 possible delimiter, add a new criteria based
    // on the length of tokens parsed using each of the (remaining) candidate
    // delimiters.  
    if (delim_candidates_count > 1) {

        // Erase delimiters we are no longer considering
        m_Scores.erase(m_Scores.begin() + delim_candidates_count, m_Scores.end());

        vector<string> token_array;
        for (i=0; i<m_Scores.size(); ++i) {

            merge = NStr::eNoMergeDelims;

            string delim_str;

            // If its < 256, its a single ascii character
            if (m_Scores[i].m_DelimChar < 256) {
                delim_str = string(1, (char)m_Scores[i].m_DelimChar);
            }
            // > 256 the current candidate is a set of characters (e.g. whitespace)
            else {
                merge = NStr::eMergeDelims;

                CMergedChar c(m_Scores[i].m_DelimChar);
                vector<CMergedChar>::iterator iter;
                iter = std::find(m_Repeats.begin(), m_Repeats.end(), c);

                /// Get delimiter string from m_Repeats.
                if (iter != m_Repeats.end()) {
                    delim_str = (*iter).m_Chars;
                }
                else {                   
                    _TRACE("Execution error - missing repeat character: " << 
                        m_Scores[i].m_DelimChar);
                    continue;
                }
            }            

            // We have saved a set of unparsed rows in m_RecordedRows and here
            // we go through those and see which delimiter produces better
            // 'quality' tokens.
            for (size_t j=0; j<m_RecordedRows.size(); ++j) {
                token_array.clear();
                NStr::Split(m_RecordedRows[j], delim_str, token_array, merge == NStr::eMergeDelims ? NStr::fSplit_Tokenize : 0);
                m_Scores[i].m_TokenLenScore = (float)token_array.size();

                // Score delimiters by % of non-trivial tokens.  A trivial token
                // is empty or it has another candidate token inside of it.
                for (size_t tok = 0; tok<token_array.size(); ++tok) {
                    if (token_array[tok].size() == 0) {
                        m_Scores[i].m_TokenLenScore -= 2.0f;
                    }
                    else if (token_array[tok].size() == 1) {
                        for (size_t k=0; k<(size_t)delim_candidates_count; ++k) {
                            if (k != i) {
                                if (token_array[tok] == string(1, m_Scores[k].m_DelimChar) ) {
                                    m_Scores[i].m_TokenLenScore -= 2.0f;
                                }
                            }
                        }
                    }
                }

                // normalize it so that the result is the % of non-zero length
                // tokens + one-length tokens where the token is equal to another
                // delimiter candidate
                m_Scores[i].m_TokenLenScore = m_Scores[i].m_TokenLenScore/(float)token_array.size();                
               
                // If a merged delimiter score is identical to a non-merged score,
                // prefer the non-merged
                float merge_penalty = 0.0f;
                if ( m_Scores[i].m_DelimChar > 255 ) {
                    merge_penalty = 0.01f;

                    // whitespace will allways score equal to or better than blanks and tabs.
                    // This penalizes whitespace for being more general so if scores
                    // are exactly the same, fewer delimiters will be preferred.
                    merge_penalty += (m_Scores[i].m_DelimChar == 258) ? 0.1f : 0.0f;
                }

                // Favor 'standard' tokens over nonstandard ones generally e.g.
                // seq_id 29
                // seq_id 33  
                // the '_' and ' ' should have identical scores but we choose ' '
                // because it is more often used as a token.
                float token_bonus = 0.0f;
                if (char(m_Scores[i].m_DelimChar) == ',' || 
                    char(m_Scores[i].m_DelimChar) == ';' ||
                    char(m_Scores[i].m_DelimChar) == '|' ||
                    char(m_Scores[i].m_DelimChar) == ' ' ||
                    char(m_Scores[i].m_DelimChar) == '\t' ||
                    m_Scores[i].m_DelimChar > 255)
                        token_bonus = 0.2f;


                // Compute a combined score:
                m_Scores[i].m_CombinedScore = (m_Scores[i].m_OccuranceScore + 
                    m_Scores[i].m_FrequencyScore*0.2f + 
                    (m_Scores[i].m_TokenLenScore*0.25f)/1.50f) - merge_penalty + token_bonus;

            }
        }

        // Resort with updated weights.  If scores are even, sort favors 
        // candidates with lower indices. 
        std::sort(m_Scores.begin(), m_Scores.end(), greater<CDelimScore>());


        // /* for debugging delimiter scores
        for (i=0; i<m_Scores.size(); ++i) {
            if (m_Scores[i].m_CombinedScore > 0.0f && i<5) {
                _TRACE("Score for char: " << m_Scores[i].m_DelimChar << " is: (" << 
                     m_Scores[i].m_OccuranceScore << ", " <<  
                     m_Scores[i].m_FrequencyScore << ", " << 
                     m_Scores[i].m_TokenLenScore << ", " << 
                     m_Scores[i].m_CombinedScore << ")" );
            }
        }
        //*/
    }

    
    // Return most likely token
    max_score = m_Scores[0].m_CombinedScore;

    if (m_Scores[0].m_DelimChar < 256) {
        delims.push_back((char)m_Scores[0].m_DelimChar);
        merge = NStr::eNoMergeDelims;
        
    }
    else {
        merge = NStr::eMergeDelims;

        CMergedChar c(m_Scores[0].m_DelimChar);
        vector<CMergedChar>::iterator iter;
        iter = std::find(m_Repeats.begin(), m_Repeats.end(), c);

        /// Should always be found:
        if (iter != m_Repeats.end()) {
            for (size_t j=0; j<(*iter).m_Chars.size(); ++j)
                delims.push_back((*iter).m_Chars[j]);            
        }
        else {                   
            _TRACE("Execution error - missing repeat character: " << 
                m_Scores[i].m_DelimChar);
            max_score = 0.0f;
        }
    }
}



/*****************************************************************************/
/*************************** CTableDelimiterRules ****************************/

bool CTableDelimiterRules::MatchingDelimiters(std::vector<char> other_delims) const
{
    // Return true if we have all the same delimiters in m_Delimeters as we
    // we have in other_delims (even if order is different)
    vector<char> cur_delims = m_Delimiters;

    sort(cur_delims.begin(), cur_delims.end());
    sort(other_delims.begin(), other_delims.end());

    return (cur_delims == other_delims);    
}

void CTableDelimiterRules::LogDelims() const
{
    string delims;
    for (size_t i=0; i<m_Delimiters.size(); ++i) {
        if (m_Delimiters[i] == '\t')
            delims += "\\t";
        else
            delims +=m_Delimiters[i];
    }

    LOG_POST(Info << "Import Table Delimiters: \"" << delims << "\"");
    LOG_POST(Info << "             Quote Character: '" << m_QuoteChar << "'");
    LOG_POST(Info << "             Multi-line Quotes: " << m_MultiLineQuotes);
    LOG_POST(Info << "             Merge Delimiters: " << m_MergeDelimiters);
}

/// Export delimiter rules in ASN user-data format
void CTableDelimiterRules::SaveAsn(CUser_field& user_field) const
{
    // convert character array to a ints to store in user object.
    vector<int> delims;
    for (size_t i=0; i<m_Delimiters.size(); ++i)
        delims.push_back((int)m_Delimiters[i]);
    
    user_field.AddField("delim-chars", delims);
    user_field.AddField("quote-char", (int)m_QuoteChar);
    user_field.AddField("multi-line-quotes", m_MultiLineQuotes);
    user_field.AddField("merge-delimiters", m_MergeDelimiters);
}

void CTableDelimiterRules::LoadAsn(CUser_field& delimiter_object)
{
    if (delimiter_object.HasField("delim-chars") && 
        delimiter_object.GetField("delim-chars").GetData().IsInts()) {
            vector<int> delims = delimiter_object.GetField("delim-chars").GetData().GetInts();

            m_Delimiters.clear();
            for (size_t i=0; i<delims.size(); ++i) {
                m_Delimiters.push_back((char)delims[i]);
            }
    }

    if (delimiter_object.HasField("quote-char") && 
        delimiter_object.GetField("quote-char").GetData().IsInt()) {
            m_QuoteChar = (char)delimiter_object.
                GetField("quote-char").GetData().GetInt();
    }

    if (delimiter_object.HasField("multi-line-quotes") && 
        delimiter_object.GetField("multi-line-quotes").GetData().IsBool()) {
            m_MultiLineQuotes = delimiter_object.
                GetField("multi-line-quotes").GetData().GetBool();
    }

    if (delimiter_object.HasField("merge-delimiters") && 
        delimiter_object.GetField("merge-delimiters").GetData().IsBool()) {
            m_MergeDelimiters = delimiter_object.
                GetField("merge-delimiters").GetData().GetBool();
    }
}

/*****************************************************************************/
/*************************** CTableImportRow *********************************/

CTableImportRow::CTableImportRow(string& s) 
: m_TableEntry(s) 
{
    m_Fields.push_back(pair<size_t,size_t>(0,m_TableEntry.size()));
}

string CTableImportRow::GetField(int column_idx) const
{
    string field("");

    if (column_idx < (int)m_Fields.size()) {
        pair<size_t,size_t> field_idx = m_Fields[column_idx];       

        field = m_TableEntry.substr(field_idx.first, field_idx.second);
    }

    return field;
}

/*****************************************************************************/
/*************************** CTableImportDataSource **************************/

CTableImportDataSource::CTableImportDataSource() 
: m_TableType(eDelimitedTable)
, m_FileType(eUndefinedFile)
, m_MaxRowLen(0)
, m_ImportFromRow(0)
, m_NumImportedRows(0)
, m_CommentChar(' ')
, m_MaxNonImportedRowLength(0)
, m_ColumnHeaderRow(-1)
, m_UseCurrentDelimiters(false)
{
}

void CTableImportDataSource::ClearTable()
{
    m_TableEntries.clear();
    m_Columns.clear();
    m_MaxRowLen = 0;
    m_TableType = eDelimitedTable;
    m_DelimRules.Reset();    
    SetCommentChar(' ');
    m_ImportFromRow = 0;
    m_NumImportedRows = 0;
    m_ColumnHeaderRow = -1;
    m_UseCurrentDelimiters = false;
}

bool CTableImportDataSource::LoadTable(const wxString& fname, CUser_object& user_object)
{   
    Int8 filesize = -1;
    {
        CFile tstfile(string(fname.ToUTF8()));

        if (!tstfile.IsFile()) {            
            LOG_POST("Error opening file: " + fname);
            return false;
        }

        filesize = tstfile.GetLength();
    }

    if (!LoadTable(fname, filesize, NULL))
        return false;

    CTableDelimiterRules delims = m_DelimRules;

    // Now update all parameters to their pre-set values,
    // except possibly for delimiters.
    ImportTableParms(user_object);
    if (m_UseCurrentDelimiters)
        m_DelimRules = delims;

    // Reparse the data but keep the columns read in from the
    // user_object
    if (m_TableType == eDelimitedTable)
        RecomputeFields(false);
    else  //eFixedWidthTable
        ExtractFixedFields();

    return true;
}


bool CTableImportDataSource::LoadTable(const wxString& fname,
                                       Int8 filesize, 
                                       ICanceled* call)
{
    ClearTable();

    CCompressedFile file(fname);
    CNcbiIstream* ifs = &(file.GetIstream());

    if (!ifs->good() || ifs->eof())
        return false;

    m_FileName = fname;
    LOG_POST(Info << "Importing Table: " << m_FileName.ToUTF8());

    try {
        CTableImportColumn c;
        c.SetName("Column 0");
        m_Columns.push_back(c);

        c.SetName("#");
        m_Columns.push_back(c);

        m_MaxNonImportedRowLength = 0;

        CCharHistogram  char_counter;
        int row_count = 0;
        int total_char_count = 0;

        int hist_count1 = 100 + m_ImportFromRow;
        int hist_count2 = 1000 + m_ImportFromRow;

        /// Read all the rows.  Read directly into target row
        // to avoid a copy.  This would probably be faster if we
        // knew the number of rows in advance (could estimate
        // based on a few lines and the file size...)
        CTableImportRow row;
        //while (NcbiGetlineEOL(*ifs, row.GetValue())) {
        while (NcbiGetline(*ifs,  row.GetValue(), "\n\r" )) {

            if (row_count < m_ImportFromRow)
                m_MaxNonImportedRowLength = std::max(row.GetValue().length(),
                                                     m_MaxNonImportedRowLength);

            // Don't add any completely blank rows
            if (!NStr::IsBlank(row.GetValue())) {
                m_TableEntries.push_back(row);
                total_char_count += row.GetValue().size();
                ++row_count;
            }
            else continue;

            // Store copy of initial rows in character histogram class so we
            // can later inspect them there for (possible)headers.
            if (row_count < 50)
                char_counter.AddInitialRows(row.GetValue());
            
            // Gather some statistical info for picking delimiters. Up to hist_count1,
            // gather from all rows, after that, gather less frequently (since data is
            // probably already adequte to take a good guess).  Try to ignore some
            // initial rows to avoid including headers in this process.
            if ((row_count >= m_ImportFromRow && row_count < hist_count1) ||
                (row_count > hist_count1 && row_count < hist_count2 && row_count%10 == 0 ) ||
                (row_count > hist_count2 && row_count%100 == 0)) {

                    // Ignore first "few" (<5) rows to try to skip headers, but don't
                    // ignore initial rows for very small files (e.g. 3 row file)
                    bool analyze_row = true;
                    if (row_count < 5)  {
                        Int8 average_rowlen = (Int8)(total_char_count/row_count);
                        int projected_rowcount = (int)(filesize/average_rowlen);
                        if (projected_rowcount >= 10)
                            analyze_row = false;
                        else if (row_count < projected_rowcount-5)
                            analyze_row = false;
                    }

                    if (analyze_row)
                        char_counter.UpdateHistogram(row.GetValue());
            }

            // Estimate total file size to avoid resizing array multiple times
            // (efficiency for reading large files - provides modest improvement - 
            // about 20%)
            if (row_count == 80 && filesize != -1) {
                Int8 average_rowlen = (Int8)(char_counter.GetCharCount()/row_count);
                int projected_rowcount = (int)(filesize/average_rowlen);
                //_TRACE("Projected rows = " << projected_rowcount);
                m_TableEntries.reserve(projected_rowcount + 0.2*projected_rowcount);
            }
                     

            row.GetValue().clear();

            if (call != NULL && call->IsCanceled()) {
                m_TableEntries.clear();
                return false;
            }
        }

        if (row_count == 0) {
            m_TableEntries.clear();
            return false;
        }


        // If the table type is known, then we can use that info
        // to directly fill in separator character, comment character etc.
        if (x_PickFileType()) {
            m_TableType = eDelimitedTable;
            LOG_POST(Info << "Imported Table Type Guess: Delimited Type");
            m_DelimRules.LogDelims();

            RecomputeFields(true);

            // Save max row len for displaying table in single-column mode
            m_MaxRowLen = char_counter.GetMaxRowLen();
        }
        else {
            // Before we try to find a separating character, lets try to figure
            // out how many lines are part of the header and if there is a 
            // comment character at the front of the header
            x_FindHeaderRows(char_counter);

            float max_score = 0.0f;

            vector<char> delims;
            NStr::EMergeDelims merge;

            // Save max row len for displaying table in single-column mode
            m_MaxRowLen = char_counter.GetMaxRowLen();
            char_counter.GetDelimiterProbablities(max_score, delims, merge);

             /// Clear current delims and set other options to defaults
             m_DelimRules.Reset();

            // Set the table type (for now) based on the max_score.  0.8 is somewhat arbirary
            // (scale is 0..1). 
            if (max_score > 0.8f) {
                m_TableType = eDelimitedTable;                 

                m_DelimRules.SetDelimiters(delims);
                m_DelimRules.SetMergeDelimiters(merge);
                RecomputeFields(true);

                LOG_POST(Info << "Imported Table Type Guess: Delimited Type");
                m_DelimRules.LogDelims();
            }
            else {
                m_TableType = eFixedWidthTable;
                LOG_POST(Info << "Imported Table Type Guess: Fixed Width");

                vector<char>  delim;

                // No delimiters - will have to assign widths on fixed-width page
                // unless choice is overriden.
                m_DelimRules.SetDelimiters(delim);
                RecomputeFields(true);
            }
        }

    }
    catch (...) {
        ClearTable();
       
        return false;
    }

    return true;
}

void CTableImportDataSource::SaveTable(CNcbiOfstream& ofs)
{
    // Go through each entry in the table
    for (size_t row=0; row<m_TableEntries.size(); ++row) {        
        ofs << m_TableEntries[row].GetValue() << endl;
    }
}

void CTableImportDataSource::SetTableType(EFieldSeparatorType e) 
{    
    m_TableType = e; 

    // Update column information based on type
    if (m_TableType == eDelimitedTable) {
        RecomputeFields(true);
    }
    else {
        // This is called prior to having fixed column widths available,
        // so we just clear out the column info
        if (m_Columns.size() > 2) {
            m_Columns.erase(m_Columns.begin()+2, m_Columns.end());
            m_Columns[1].SetWidth(m_MaxRowLen);
        }
    }
}


bool CTableImportDataSource::x_PickFileType()
{
    m_FileType = eUndefinedFile;

    string::size_type pos = m_TableEntries[0].GetValue().find_first_not_of(" \t");
    char comment_char = ' ';

    if (pos != string::npos)
        comment_char = m_TableEntries[0].GetValue()[pos];

    if (comment_char == '#' &&
        m_TableEntries[0].GetValue().find("BLAST") != string::npos) {
            m_FileType = eBlastResultsFile;

            std::vector<char> delimiters;
            delimiters.push_back('\t');
            delimiters.push_back('|');
            
            m_DelimRules.SetDelimiters(delimiters);
            m_DelimRules.SetMergeDelimiters(true);

            SetCommentChar('#');

            // Synchronize first import row to number of initial comment lines:
            m_ImportFromRow = 0;            
            for (size_t i=0; i<m_TableEntries.size(); ++i) {
                string::size_type spos = 
                    m_TableEntries[i].GetValue().find_first_not_of(" \n");

                if (spos != string::npos && 
                    m_TableEntries[i].GetValue()[spos] == m_CommentChar) {
                        ++m_ImportFromRow;
                }
                else break;
            }            

            return true;
    }

    return false;
}

void CTableImportDataSource::x_FindHeaderRows(const CCharHistogram& char_counter)
{
    // First get first non-blank character from first line and see if it is
    // a header comment (if it appears in the initial rows as first character
    // but is never the first character thereafter, it is a header comment. Must
    // also not be a letter or number: a-z, A-Z, 0-9)        
    int header_rows = 1;
    int non_header_rows = 0;
    bool has_header = true;
    char comment_char = '0'; // Not a valid comment char
    string::size_type pos = m_TableEntries[0].GetValue().find_first_not_of(" \t");
    if (pos != string::npos)
        comment_char = m_TableEntries[0].GetValue()[pos];

    // Standard characters are not supported as header comments:
    if ((comment_char >= '0' && comment_char <= '9') ||
        (comment_char >= 'A' && comment_char <= 'Z') ||
        (comment_char >= 'a' && comment_char <= 'z') ||
        comment_char == ' ' || comment_char == '\t') {
            has_header = false;
            header_rows = 0;
    }

    for (size_t i=1; i<std::min((size_t)500, m_TableEntries.size()) && has_header; ++i) { 
        // Get first character in row:
        char first_char = '0';
        pos = m_TableEntries[i].GetValue().find_first_not_of(" \t");
        if (pos != string::npos)
            first_char = m_TableEntries[i].GetValue()[pos];

        if (first_char == comment_char) {
            // possible comment character has appeared after initial block, so it 
            // must not be a comment character:
            if (non_header_rows > 0) {
                has_header = false;
                break;
            }
            ++header_rows;
        }
        else {
            ++non_header_rows;
        }            
    }

    // If many rows begin with same char, it is probably not a header char
    if (has_header) {
        if (header_rows > 20 || header_rows > non_header_rows) {
            has_header = false;
            header_rows = 0;
        }
        else {
            m_ImportFromRow = header_rows;
            SetCommentChar(comment_char);
            return;
        }
    }

    // If there is no obvious header comment comment character, 
    // look for differences in length and frequency of character 
    // data to identify possible header
    float certainty = 0.0f;
    int first_non_header_row = -1;

    // ! and ~ are the first and last typical, non-space, ascii characters
    char_counter.GetGroupOccuranceAverage('!', '~', certainty, first_non_header_row);
    if (certainty > 1.5f && first_non_header_row > 0) {
        m_ImportFromRow = first_non_header_row;       
    }
    // Last check - check if first row contains certain keywords often found in headers
    else {
        string first_row = m_TableEntries[0].GetValue();
        NStr::ToLower(first_row);

        if (NStr::Find(first_row, "seqid") != NPOS ||
            NStr::Find(first_row, "accession") != NPOS) {
                m_ImportFromRow = 1;
        }
    }

    SetCommentChar(' ');
}


void CTableImportDataSource::x_ParseEntry(const CTempString&    str,
                                          const CTempString&    delim,
                                          NStr::EMergeDelims    merge,
                                          bool multiple_spaces_only,
                                          vector<std::pair<size_t,size_t> >& token_pos)
{
    // Special cases
    if (str.empty()) {
        return;
    } else if (delim.empty() && !multiple_spaces_only) {
        token_pos.push_back(pair<size_t,size_t>(0, str.length()));
        return;
    }

    // Tokenization
    //
    string::size_type pos, prev_pos;
    for (pos = 0;;) {
        prev_pos = ((merge == NStr::eMergeDelims && delim != "") ? 
            str.find_first_not_of(delim, pos) : pos);
        
        // don't allow merging of delimiters between multiple blanks and other chars
        if (multiple_spaces_only && str[pos] == ' ')
            prev_pos = str.find_first_not_of(CTempString(" "), pos);        

        if (prev_pos == CTempString::npos) {
            break;
        }
        pos = str.find_first_of(delim, prev_pos);
        if (multiple_spaces_only) {            
            pos = std::min(pos, str.find(CTempString("  "), prev_pos)); 
        }
        if (pos == CTempString::npos) {
            token_pos.push_back(pair<size_t, size_t>(prev_pos, str.length() - prev_pos));
            break;
        } else {
            token_pos.push_back(pair<size_t, size_t>(prev_pos, pos-prev_pos));
            ++pos;
        }
    } // for        
}

void CTableImportDataSource::x_ParseQuotedEntry(const CTempString&     str,
                                                const CTempString&     delim,
                                                const CTempString&     delim_and_quote,
                                                NStr::EMergeDelims     merge,
                                                bool multiple_spaces_only,
                                                char                   quote_char,                      
                                                vector<std::pair<size_t,size_t> >& token_pos)
{
    // Special cases
    if (str.empty()) {
        return;
    } else if (delim.empty() && !multiple_spaces_only) {
        token_pos.push_back(pair<size_t,size_t>(0, str.length()));
        return;
    }

    // Tokenization
    //
    string::size_type pos;
    string::size_type prev_pos = string::npos;
    string::size_type search_pos = string::npos;

    bool token_added = true;

    for (pos = 0;;) {
        if (token_added) {
            prev_pos = ((merge == NStr::eMergeDelims && delim != "") ? 
                str.find_first_not_of(delim, pos) : pos);

            if (multiple_spaces_only && str[pos] == ' ')
                prev_pos = str.find_first_not_of(CTempString(" "), pos);
            
            search_pos = prev_pos;
        }
        if (prev_pos == CTempString::npos) {
            break;
        }

        // Find the next delimiter OR beginning of a quoted string
        pos = str.find_first_of(delim_and_quote, search_pos);
        if (multiple_spaces_only) {           
            pos = std::min(pos, str.find(CTempString("  "), search_pos)); 
        }
        if (pos == CTempString::npos) {
            token_pos.push_back(pair<size_t, size_t>(prev_pos, str.length() - prev_pos));
            break;
        } else {
            if (str[pos] == quote_char) {
                // proceed to close-quote then search again for next delimiter.  
                // Assume CSV rules for embedding quotes within quoted strings
                // - any embedded quotes are doubled, e.g. 
                // "A quote within a ""quote"" looks like this"
                for (++pos;
                     pos < str.length() &&
                     (str[pos]!=quote_char || 
                      (str[pos]==quote_char && str[pos-1]==quote_char) ||
                      (str[pos]==quote_char && pos<str.length()-1 && 
                       str[pos+1]==quote_char)); ++pos) {                                    
                }

                /// Quote ended at EOL OR may be unbalanced, eg: "not balnaced\n
                if (pos >= str.length()-1) {
                    token_pos.push_back(pair<size_t, size_t>(prev_pos, str.length() - prev_pos));
                    break;
                }

                search_pos = ++pos;
                token_added = false;
                continue;
            } 
            else {
                token_added = true;
                token_pos.push_back(pair<size_t,size_t>(prev_pos, pos-prev_pos));
                ++pos;
                search_pos = prev_pos;
            }
        }
    } // for        
}

void CTableImportDataSource::x_RecomputeRowFields(CTableImportRow& row,
                                                  const CTempString& delims_ts,
                                                  const CTempString& delims_quote_ts,
                                                  NStr::EMergeDelims merge_delims,
                                                  bool multiple_spaces_only)
{
    // parse fields based on current delimiters
    // Use the tokens to set position and length of tokens for the current row
    vector<std::pair<size_t,size_t> >& fields = row.GetFields();
    fields.clear();

    if (m_DelimRules.GetQuoteChar() == ' ') {
        x_ParseEntry(CTempString(row.GetValue()), 
            delims_ts, 
            merge_delims,
            multiple_spaces_only,
            fields);
    }
    else {
        x_ParseQuotedEntry(CTempString(row.GetValue()), 
            delims_ts, 
            delims_quote_ts,
            merge_delims,
            multiple_spaces_only,
            m_DelimRules.GetQuoteChar(),
            fields);
    }


    // Update field positions for this specific table entry 
    // NOTE: We could also do this only when a row is actually displayed
    // if performance is an issue (and maybe tag it as 'updated')
    for (size_t i=0; i<fields.size(); ++i) {
        // Update (if needed) columns for overall table (although all rows
        // will often have the same number of columns that may not always be
        // true. Also, track maximum column widths for formatting).
        if (i >= m_Columns.size()-1) {
            CTableImportColumn c;
            c.SetWidth(fields[i].second);

            if (m_ColumnHeaderRow != -1 && 
                i <m_TableEntries[m_ColumnHeaderRow].GetNumFields()) {
                    string column_name = m_TableEntries[m_ColumnHeaderRow].GetField(i);
                    // First field could be preceded by the comment character for header
                    // rows, if there is one.
                    if (i==0 && column_name.size() > 0 && 
                        column_name[0] == m_CommentChar) {
                            column_name = column_name.substr(1, column_name.size()-1);
                    }
                    c.SetName(column_name);

                    // make sure there is space for the name
                    c.SetWidth(column_name.size());
            }
            else {
                c.SetName("Col " + NStr::NumericToString(i+1));
                c.SetWidth(c.GetName().size());
            }                

            m_Columns.push_back(c);
        }
        m_Columns[i+1].SetWidth(std::max(m_Columns[i+1].GetWidth(), 
            (int)fields[i].second));
    }
}

void CTableImportDataSource::RecomputeRowFields(size_t row_idx)
{
    // Concatenate all delimiters into one string for use by tokenize
    string delims_str;
    string delims_quote_str;
    CTempString delims_ts;
    CTempString delims_quote_ts;
    for (size_t j=0; j<m_DelimRules.GetDelimiters().size(); ++j) {
        if (!(m_DelimRules.GetMultipleSpacesOnly() && m_DelimRules.GetDelimiters()[j] == ' '))
            delims_str.push_back(m_DelimRules.GetDelimiters()[j]);
    }

    delims_ts = delims_str; 

    delims_quote_str = delims_str + m_DelimRules.GetQuoteChar();
    delims_quote_ts = delims_quote_str;

    NStr::EMergeDelims merge_delims = m_DelimRules.GetMergeDelimiters() ? 
        NStr::eMergeDelims : NStr::eNoMergeDelims;

    x_RecomputeRowFields(m_TableEntries[row_idx], 
                         delims_ts, 
                         delims_quote_ts, 
                         merge_delims, 
                         m_DelimRules.GetMultipleSpacesOnly());
}

void CTableImportDataSource::RecomputeFields(bool recreate_columns,
                                             int recompute_count)
{   
    // Concatenate all delimiters into one string for use by tokenize
    string delims_str;
    string delims_quote_str;
    CTempString delims_ts;
    CTempString delims_quote_ts;
    for (size_t j=0; j<m_DelimRules.GetDelimiters().size(); ++j) {
        if (!(m_DelimRules.GetMultipleSpacesOnly() && m_DelimRules.GetDelimiters()[j] == ' '))
            delims_str.push_back(m_DelimRules.GetDelimiters()[j]);
    }

    delims_ts = delims_str; 

    delims_quote_str = delims_str + m_DelimRules.GetQuoteChar();
    delims_quote_ts = delims_quote_str;

    NStr::EMergeDelims merge_delims = m_DelimRules.GetMergeDelimiters() ? 
        NStr::eMergeDelims : NStr::eNoMergeDelims;

    int row_count = 0;
    m_MaxNonImportedRowLength = 0;

    // Iterate over the data to determine the number of fields and set
    // up those fields as columns with undefined types and
    // simple names (column 1, column 2....)
    vector<CTableImportRow>::iterator iter;

    // If we are not reparsing all the rows, need to remember the widths
    vector<CTableImportColumn> prev_columns = m_Columns;

    if (recreate_columns) {               
        m_Columns.clear();
        CTableImportColumn c;
        c.SetName("#");
        m_Columns.push_back(c);
    }
    int count = 0;

    // If user wants to use one of the rows to get column names, parse that
    // first and then use its results for column headers (until they run out)
    if (m_ColumnHeaderRow != -1) { 
        string header_row = m_TableEntries[m_ColumnHeaderRow].GetValue();

        vector<std::pair<size_t,size_t> >& fields = m_TableEntries[m_ColumnHeaderRow].GetFields();
        fields.clear();

        if (m_DelimRules.GetQuoteChar() == ' ') {
            x_ParseEntry(CTempString(header_row), 
                delims_ts, 
                merge_delims,
                m_DelimRules.GetMultipleSpacesOnly(),
                fields);
        }
        else {
            x_ParseQuotedEntry(CTempString(header_row), 
                delims_ts, 
                delims_quote_ts,
                merge_delims,
                m_DelimRules.GetMultipleSpacesOnly(),
                m_DelimRules.GetQuoteChar(),
                fields);
        }
    }


    // Go through each entry in the table
    for (iter = m_TableEntries.begin(); iter != m_TableEntries.end(); ++iter) {
        
        // Don't parse rows that are not designated to be imported
        if (row_count++ < m_ImportFromRow || (*iter).GetRowNum() == -1) {
            m_MaxNonImportedRowLength = std::max((*iter).GetValue().length(),
                                                 m_MaxNonImportedRowLength);        
            continue;
        }

        x_RecomputeRowFields(*iter, 
                             delims_ts, 
                             delims_quote_ts, 
                             merge_delims, 
                             m_DelimRules.GetMultipleSpacesOnly());
 

        // Caller may set a limited number of (initial) rows to
        // be updated (used when # of header rows is updated)
        if (recompute_count != -1 && ++count > recompute_count) {

            // Retain widths from previous update
            for (size_t i=1; i<m_Columns.size(); ++i) {                
                if (prev_columns.size() > i) {
                    m_Columns[i].SetWidth(max(m_Columns[i].GetWidth(), prev_columns[i].GetWidth()));
                }
                else break;
            }

            RecomputeHeaders();
            return;
        }
    }

    RecomputeHeaders();
}

void CTableImportDataSource::ExtractFixedFields()
{   
    int row_count = 0;
    m_MaxNonImportedRowLength = 0;

    // Iterate over the data to determine the number of fields and set
    // up those fields as columns with undefined types and
    // simple names (column 1, column 2....)
    vector<CTableImportRow>::iterator iter;

    // Go through each entry in the table
    for (iter = m_TableEntries.begin(); iter != m_TableEntries.end(); ++iter) {

        // Don't parse rows that are not designated to be imported
        if (row_count++ < m_ImportFromRow || (*iter).GetRowNum() == -1) {
            m_MaxNonImportedRowLength = std::max((*iter).GetValue().length(),
                m_MaxNonImportedRowLength);        
            continue;
        }

        size_t len = (*iter).GetValue().length();
        std::vector<std::pair<size_t,size_t> >& fields = (*iter).GetFields();

        fields.clear();

        size_t start_idx = 0;
        for (size_t i=1; i<m_Columns.size(); ++i) {
            size_t w = (size_t)m_Columns[i].GetWidth();
            if (start_idx < len) {
                w = std::min(w, len-start_idx);
                fields.push_back(std::pair<size_t,size_t>(start_idx,w));
            }
            else {
                w = 0;
                fields.push_back(std::pair<size_t,size_t>(len-1, w));
            }

            start_idx += w;
        }        
    }

    RecomputeHeaders();
}

void CTableImportDataSource::LogFixedFieldWidths() const
{
    LOG_POST(Info << "Table Import: Fixed field widths: ");
    size_t start_idx = 0;
    for (size_t i=1; i<m_Columns.size(); ++i) {
        size_t w = (size_t)m_Columns[i].GetWidth();
        
        LOG_POST(Info << "Field #: " << i << " (" << start_idx << ", " 
            << start_idx + w << ")");       

        start_idx += w;
    }        
}

void CTableImportDataSource::RecomputeHeaders()
{   
    // If a row has been designated as the source of the column names, parse
    // it according to the same rules as the other rows and then use
    // the results to update the column names.
    if (m_ColumnHeaderRow == -1 ||
        m_TableEntries.size() <= (size_t)m_ColumnHeaderRow)
            return;

    // Concatenate all delimiters into one string for use by tokenize
    if (m_TableType == eDelimitedTable) {
        string delims_str;
        string delims_quote_str;
        CTempString delims_ts;
        CTempString delims_quote_ts;

        for (size_t j=0; j<m_DelimRules.GetDelimiters().size(); ++j) {
            if (!(m_DelimRules.GetMultipleSpacesOnly() && m_DelimRules.GetDelimiters()[j] == ' '))
                delims_str.push_back(m_DelimRules.GetDelimiters()[j]);
        }

        delims_ts = delims_str; 

        delims_quote_str = delims_str + m_DelimRules.GetQuoteChar();
        delims_quote_ts = delims_quote_str;

        NStr::EMergeDelims merge_delims = m_DelimRules.GetMergeDelimiters() ? 
            NStr::eMergeDelims : NStr::eNoMergeDelims;

        // If user wants to use one of the rows to get column names, parse that
        // first and then use its results for column headers (until they run out)
        string header_row = m_TableEntries[m_ColumnHeaderRow].GetValue();

        vector<std::pair<size_t,size_t> >& fields = m_TableEntries[m_ColumnHeaderRow].GetFields();
        fields.clear();

        if (m_DelimRules.GetQuoteChar() == ' ') {
            x_ParseEntry(CTempString(header_row), 
                delims_ts, 
                merge_delims,
                m_DelimRules.GetMultipleSpacesOnly(),
                fields);
        }
        else {
            x_ParseQuotedEntry(CTempString(header_row), 
                delims_ts, 
                delims_quote_ts,
                merge_delims,
                m_DelimRules.GetMultipleSpacesOnly(),
                m_DelimRules.GetQuoteChar(),
                fields);
        }
    }
    else  {// (m_TableType == eFixedWidthTable)
        size_t len = m_TableEntries[m_ColumnHeaderRow].GetValue().length();
        std::vector<std::pair<size_t,size_t> >& fields = m_TableEntries[m_ColumnHeaderRow].GetFields();

        fields.clear();

        size_t start_idx = 0;
        for (size_t i=1; i<m_Columns.size(); ++i) {
            size_t w = (size_t)m_Columns[i].GetWidth();
            if (start_idx < len) {
                w = std::min(w, len-start_idx);
                fields.push_back(std::pair<size_t,size_t>(start_idx,w));
            }
            else {
                w = 0;
                fields.push_back(std::pair<size_t,size_t>(len-1, w));
            }

            start_idx += w;
        } 
    }

    // Update column names with parsed values or generated names (column.[1..n])
    size_t parsed_field_idx = 0;
    for (size_t i=0; i<m_Columns.size()-1; ++i) {           
        if (i < m_TableEntries[m_ColumnHeaderRow].GetNumFields()) {
            string column_name = m_TableEntries[m_ColumnHeaderRow].GetField(parsed_field_idx++);
            // First field could be preceded by the comment character for header
            // rows, if there is one.  The comment character could either be 
            // attached to the name: "#accession " or separate: "# accession".
            // check for both.
            if (i==0 && column_name.size() > 0 && 
                column_name[0] == m_CommentChar) {
                    column_name = column_name.substr(1, column_name.size()-1);
                    
                    // If the field was just the comment character, jump to next
                    // parsed field.  GetField() returns empty strings if it 
                    // runs out of tokens,
                    if (column_name.length() == 0) {
                        column_name = m_TableEntries[m_ColumnHeaderRow].GetField(parsed_field_idx++);
                    }
            }

            column_name = NStr::TruncateSpaces(column_name);
            if (column_name == "")
                column_name = "Column " + NStr::NumericToString(i+1);

            m_Columns[i+1].SetName(column_name);


            // make sure there is space for the name, which we don't do for
            // the generated names.
            m_Columns[i+1].SetWidth(std::max(m_Columns[i+1].GetWidth(),
                (int)column_name.size()));
        }
        else {
            m_Columns[i+1].SetName("Column " + NStr::NumericToString(i+1));
        }                
    }       
}

string CTableImportDataSource::GetField(size_t row, size_t col) const 
{
    // Get a field out of the specified row 'row'.  This function is
    // used by the wxListCtrl to get fields for display.
    if (m_TableEntries.size() <= row) { 
        return "";
    }

    return (m_TableEntries[row].GetField(col));
}


void CTableImportDataSource::SetFirstImportRow(int r) 
{ 
    m_ImportFromRow = r; 
    LOG_POST(Info << "Import table: first import row: " << m_ImportFromRow);

    SetCommentChar(GetCommentChar());
    RecomputeFields(true, m_ImportFromRow);
}

void CTableImportDataSource::SetColumnHeaderRow(int c) 
{ 
    m_ColumnHeaderRow = c;
    
    // Need to update row numbers if this will change where import starts (import
    // will always start after this line).  SetCommentChar will do this update so
    // just reset the same character.
    SetCommentChar(m_CommentChar);

    LOG_POST(Info << "Import table: column header row: " << m_ColumnHeaderRow);
}

 void CTableImportDataSource::SetHeaderAndFirstRow(int column_header_row, 
                                                   int first_row)
 {
    m_ColumnHeaderRow = column_header_row;
    m_ImportFromRow = first_row; 

    LOG_POST(Info << "Import table: column header row: " << m_ColumnHeaderRow);
    LOG_POST(Info << "Import table: first import row: " << m_ImportFromRow);

    // Need to update row numbers if this will change where import starts (import
    // will always start after this line).  SetCommentChar will do this update so
    // just reset the same character.
    SetCommentChar(m_CommentChar);
    RecomputeFields(true);
 }

void CTableImportDataSource::SetCommentChar(char c) 
{ 
    m_CommentChar = c; 
    LOG_POST(Info << "Import table: row comment character: " << m_CommentChar);

    m_MaxNonImportedRowLength = 0;
    int row_num = 0;
    int count = 0;

    vector<CTableImportRow>::iterator iter;
    for (iter = m_TableEntries.begin(); iter != m_TableEntries.end(); ++iter, ++count) {         
        if (count >= m_ImportFromRow && count > m_ColumnHeaderRow) {
            string::size_type spos = (*iter).GetValue().find_first_not_of(" \n");

            if (spos == string::npos || (*iter).GetValue()[spos] != m_CommentChar) {
                (*iter).SetRowNum(row_num++);
            }
            else {
                (*iter).SetRowNum(-1);
                m_MaxNonImportedRowLength = std::max((*iter).GetValue().length(),
                    m_MaxNonImportedRowLength);
            }
        }
        else {
            (*iter).SetRowNum(-1);
            m_MaxNonImportedRowLength = std::max((*iter).GetValue().length(),
                m_MaxNonImportedRowLength);
        }
    }

    m_NumImportedRows = row_num;

    RecomputeHeaders();
}

void CTableImportDataSource::MergeColumns(vector<size_t> col_indices, char ch, 
                                          bool no_merge_char)
{
    std::sort(col_indices.begin(), col_indices.end());

    // make sure indices not > number of columns
    if (col_indices.back() >= m_Columns.size())
        return;

    // We are interested here in fields, not columns.  The first column is
    // always the row # column which is not a field parsed from the text. So
    // we subtract 1 here to convert from columns to fields
    for (size_t i=0; i<col_indices.size(); ++i)
        col_indices[i] -= 1;

    // Go through each entry in the table
    for (size_t row=0; row<m_TableEntries.size(); ++row) {        
        
        // Don't parse comment rows (but do get the header, if any)       
        if (row != m_ColumnHeaderRow && 
            (row < (size_t)m_ImportFromRow || m_TableEntries[row].GetRowNum() == -1)) {  
            continue;
        }
        
        // This number of columns in this row is below the number specified for merging:
        if (col_indices.back() >= size_t(m_TableEntries[row].GetNumFields()))
            continue;

        vector<size_t> merge_cols;
        // Remove any columns which are not available in this row:
        for (size_t i=0; i<col_indices.size(); ++i)
            if ( col_indices[i] < m_TableEntries[row].GetNumFields())
                merge_cols.push_back(col_indices[i]);

        // Get edited row (replace each separation between given cols with new
        // single character).  Don't care here if separators were merged (or multiple) -
        // each is replaced with a single char.
        string& str = m_TableEntries[row].GetValue();
        // get string up to end of first col (the 'from_col')
        size_t field_end_idx = m_TableEntries[row].GetFields()[ merge_cols.front()].first +  
                               m_TableEntries[row].GetFields()[ merge_cols.front()].second;
        string merged_str = str.substr(0, field_end_idx);

        // Can also merge without inserting a character (no_merge_char)
        for (size_t i=1; i<merge_cols.size(); ++i) {
            size_t col = merge_cols[i];
            if (!no_merge_char)
                merged_str += ch;                    
            merged_str += str.substr(m_TableEntries[row].GetFields()[col].first,
                                     m_TableEntries[row].GetFields()[col].second);
        }

        for (size_t col=merge_cols.front()+1; col<m_TableEntries[row].GetFields().size(); ++col) {
            // If this is not one of the merged columns, add it to the end of the row
            // we are (re)building
            if (std::find(merge_cols.begin(), merge_cols.end(), col) == merge_cols.end()) {
                // get field start plus preceeding delimiter
                size_t field_start_idx = m_TableEntries[row].GetFields()[col].first-1;
                if (m_DelimRules.GetMultipleSpacesOnly()) {
                    merged_str += " "; 
                }

                merged_str += str.substr(field_start_idx,
                                         m_TableEntries[row].GetFields()[col].second+1);
            }
        }

        m_TableEntries[row].GetValue() = merged_str;
    }

    RecomputeFields(true);
}


bool CTableImportDataSource::SplitColumn(size_t col_idx, char ch, 
                                         bool split_on_whitespace)
{
    // We are interested here in fields, not columns.  The first column is
    // always the row # column which is not a field parsed from the text. So
    // we subtract 1 here to convert from columns to fields
    col_idx -= 1;

    if (col_idx >= m_Columns.size())
        return false;

    // What is our delimiter character?  If there are multiple we can choose any.
    // If the delimiter character is space, with multiple spaces required,
    // we will use 2 blanks.
    string delimiter = "";
    if (m_DelimRules.GetMultipleSpacesOnly()) {
        delimiter = "  ";
    }
    // should be true unless user unchecked all delims...
    else if (m_DelimRules.GetDelimiters().size() > 0) {
        delimiter = string(1, m_DelimRules.GetDelimiters()[0]);
    }
    else {
        return false; 
    }

    // If any of the rows are split, we will add a new column to each row,
    // although that column will be empty in rows without a split. That is 
    // likely what the user wants in most cases.  

    // Initial run to determine if there are splits in any rows. If not
    // return.
    bool has_splits = false;
    for (size_t row=0; row<m_TableEntries.size() && !has_splits; ++row) {        
        // Don't parse header or comment rows for this check
        if (row < (size_t)m_ImportFromRow || m_TableEntries[row].GetRowNum() == -1) {  
            continue;
        }
        
        // Skip if this particular row has fewer than 'col_idx' fields
        if (col_idx >= m_TableEntries[row].GetNumFields())
            continue;

        // Get row and replace the first instance of 'ch' in column 'col_idx' with
        // a separator character.  If there are no instances or multiple instances 
        // of 'ch' only the first (or none) are updated
        string& str = m_TableEntries[row].GetValue();
                
        size_t start_idx = m_TableEntries[row].GetFields()[col_idx].first;
        size_t chars = m_TableEntries[row].GetFields()[col_idx].second;

        for (size_t i=start_idx; i<start_idx+chars; ++i) {
            if (split_on_whitespace && (str[i]==' ' || str[i]=='\t')) {
                has_splits = true;
                break;
            }
            else if (!split_on_whitespace && str[i] == ch) {
                has_splits = true;
                break;
            }
        }
    }

    if (!has_splits)
        return false;

    // Go through each entry in the table
    for (size_t row=0; row<m_TableEntries.size(); ++row) {        
        
        // Don't parse comment rows (but do get the header, if any)       
        if (row != m_ColumnHeaderRow && 
            (row < (size_t)m_ImportFromRow || m_TableEntries[row].GetRowNum() == -1)) {  
            continue;
        }

        // Skip if this particular row has fewer than 'col_idx' fields
        if (col_idx >= m_TableEntries[row].GetNumFields())
            continue;

        // Get row and replace the first instance of 'ch' in column 'col_idx' with
        // a separator character.  If there are no instances or multiple instances 
        // of 'ch' only the first (or none) are updated
        string& str = m_TableEntries[row].GetValue();
                
        size_t start_idx = m_TableEntries[row].GetFields()[col_idx].first;
        size_t chars = m_TableEntries[row].GetFields()[col_idx].second;

        // get string up to first character of split column
        string merged_str = str.substr(0, start_idx);

        bool col_split = false;
        for (size_t i=start_idx; i<start_idx+chars; ++i) {
            // We only split once per row, maximum
            if (split_on_whitespace && !col_split &&
                (str[i]==' ' || str[i]=='\t')) {
                    merged_str += delimiter;
                    col_split = true;

                    // Get end of the whitespace and restart there
                    string ws(" \t");
                    size_t next_non_whitespace_idx = str.find_first_not_of(ws, i+1);
                    if (next_non_whitespace_idx == string::npos ||
                        next_non_whitespace_idx >= start_idx+chars)
                            break;
                    else
                        i = next_non_whitespace_idx-1;               
            }
            else if (str[i] == ch && !col_split) {
                merged_str += delimiter;
                col_split = true;
            }
            else {
                merged_str += str[i];
            }
        }

        // Add delimiter to end of the field to add a new, blank, field.
        // Note that this won't work if user chooses to merge delimiters.
        if (!col_split)
            merged_str += delimiter;

        // Add all remaining characters after the split column
        merged_str += str.substr(start_idx + chars, str.length()-(start_idx+chars));
        m_TableEntries[row].GetValue() = merged_str;
    }

    RecomputeFields(true);
    return true;
}

bool CTableImportDataSource::ReplaceSpaces()
{
    // What is our delimiter character?  If there are multiple we can choose any.
    // If the delimiter character is space, with multiple spaces required,
    // we will use 2 blanks.
    string delimiter = "";
    if (m_DelimRules.GetMultipleSpacesOnly()) {
        delimiter = "  ";
    }
    // should be true uless user unchecked all delims...
    else if (m_DelimRules.GetDelimiters().size() > 0) {
        delimiter = string(1, m_DelimRules.GetDelimiters()[0]);
    }
    else {
        return false; 
    }

    // If any of the rows are split, we will add a new column to each row,
    // although that column will be empty in rows without a split. That is 
    // likely what the user wants in most cases.  

    // Initial run to determine if there are splits in any rows. If not
    // return.
    bool has_splits = false;
    for (size_t row=0; row<m_TableEntries.size() && !has_splits; ++row) {        
        // Don't parse header or comment rows for this check
        if (row < (size_t)m_ImportFromRow || m_TableEntries[row].GetRowNum() == -1) {  
            continue;
        }

        // Get row and replace the first instance of 'ch' in column 'col_idx' with
        // a separator character.  If there are no instances or multiple instances 
        // of 'ch' only the first (or none) are updated
        string& str = m_TableEntries[row].GetValue();               

        if (NStr::Find(str, "  ") != NPOS)
            has_splits = true;
    }

    if (!has_splits)
        return false;

    // Go through each entry in the table
    for (size_t row=0; row<m_TableEntries.size(); ++row) {        
        
        // Don't parse comment rows (but do get the header, if any)       
        if (row != m_ColumnHeaderRow && 
            (row < (size_t)m_ImportFromRow || m_TableEntries[row].GetRowNum() == -1)) {  
            continue;
        }

        // Get row and replace any instance of 2 or more spaces with a single instance of 'delimiter'
        string& str = m_TableEntries[row].GetValue();

        string result;
        int spaces = 0;
        for (size_t i=0; i<str.size(); ++i) {
            if (str[i] != ' ' && (spaces==0)) {
                result += str[i];
            }
            else if (str[i] == ' ') {
                spaces += 1;
            }            
            else if (str[i] != ' ' && (spaces > 0)) {
                if (spaces == 1) {
                    result += ' ';
                }
                else {
                    result += delimiter;
                }
                result += str[i];
                spaces = 0;
            }
        }

        // handle trailing space or spaces
        if (spaces > 1)
            result += delimiter;
        else if (spaces == 1)
            result += ' ';
      
        m_TableEntries[row].GetValue() = result;
    }

    RecomputeFields(true);
    return true;
}

void CTableImportDataSource::ConvertToSeqAnnot(CRef<CSeq_annot> annot_container)
{
    // The seq-table
    CRef<CSeq_table> table;
    table.Reset(new CSeq_table());

    CRef<CUser_object> column_meta_info;
    column_meta_info.Reset(new CUser_object());

    table->SetNum_rows(m_NumImportedRows);

    // Field id for data type - initialized based on type of each column
    CSeqTable_column_info_Base::EField_id field_id = 
        CSeqTable_column_info_Base::eField_id_comment;

    // Skip first column since it's just a row number
    for (size_t i=1; i<m_Columns.size(); ++i) {
        CRef<CSeqTable_column_info> cinfo;
        cinfo.Reset(new CSeqTable_column_info());

        string label;
        string value_type;
        string properties;

        label = "Column.";
        label += NStr::NumericToString(i); 

        value_type = CTableImportColumn::GetStringFromDataType(m_Columns[i].GetDataType());

        switch (m_Columns[i].GetType()) {
            case CTableImportColumn::eSkippedColumn:
            continue;

            case CTableImportColumn::eSeqIdColumn:
            {
                switch (m_Columns[i].GetDataType()) {
                   case CTableImportColumn::eLocationID:
                       field_id = CSeqTable_column_info_Base::eField_id_location_id;
                       break;
                   case CTableImportColumn::eLocationGI:
                        // Want to use location IDs in all cases because then table 
                        // can do broadcasting (doesn't work with GIs which are stored
                        // as ints)
                        //field_id = CSeqTable_column_info_Base::eField_id_location_gi;
                        field_id = CSeqTable_column_info_Base::eField_id_location_id;
                        break;
                   case CTableImportColumn::eLocalID:
                        field_id = CSeqTable_column_info_Base::eField_id_id_local;
                        break;
                   case CTableImportColumn::eChromosome:
                        field_id = CSeqTable_column_info_Base::eField_id_location_id;
                        if (m_Columns[i].GetAssembly().GetUseMapping() && !m_Columns[i].GetAssembly().GetAssemblyAcc().empty()) {
                            properties += " &genome_assembly=" + m_Columns[i].GetAssembly().GetAssemblyAcc();
                        }
                       break;
                   case CTableImportColumn::eUnspecifiedID:
                       field_id = CSeqTable_column_info_Base::eField_id_location_id;
                       break;
                   case CTableImportColumn::eRsid:
                       // None of the different column types seem to apply to snps/variations
                       field_id = CSeqTable_column_info_Base::eField_id_location_id;
                        if (m_Columns[i].GetAssembly().GetUseMapping() && !m_Columns[i].GetAssembly().GetAssemblyAcc().empty()) {
                            properties += " &genome_assembly=" + m_Columns[i].GetAssembly().GetAssemblyAcc();
                        }
                       break;
                   default:
                       field_id = CSeqTable_column_info_Base::eField_id_location_id;
                       break;
               };
                break;
            }

            case CTableImportColumn::eNumberColumn:
            {
                switch (m_Columns[i].GetDataType()) {
                    case CTableImportColumn::eStartPosition:
                        field_id = CSeqTable_column_info_Base::eField_id_location_from;
                        break;
                    case CTableImportColumn::eStopPosition:
                        field_id = CSeqTable_column_info_Base::eField_id_location_to;
                        break;
                    case CTableImportColumn::eLength:
                        field_id = CSeqTable_column_info_Base::eField_id_comment; 
                        break;
                    case CTableImportColumn::eUnspecifiedInt:
                        field_id = CSeqTable_column_info_Base::eField_id_comment;
                        break;
                    default:
                        field_id = CSeqTable_column_info_Base::eField_id_comment; 
                        break;
                };

                // If start or stop positions are one-based, we subtract one when
                // we copy the field to the seq-annot, so we set the property false
                if (m_Columns[i].GetOneBased() && 
                    m_Columns[i].GetDataType() != CTableImportColumn::eStartPosition &&
                    m_Columns[i].GetDataType() != CTableImportColumn::eStopPosition)
                    properties += " &one_based=true";
                else
                    properties += " &one_based=false";                
               break;
            }

            case CTableImportColumn::eRealNumberColumn:
            {
                switch (m_Columns[i].GetDataType()) {
                    case CTableImportColumn::eUnspecifiedReal:
                        field_id = CSeqTable_column_info_Base::eField_id_comment; 
                        break;
                    default:
                        field_id = CSeqTable_column_info_Base::eField_id_comment;
                        break;
                };
                break;
            }

            case CTableImportColumn::eTextColumn:
            {
                switch (m_Columns[i].GetDataType()) {
                    case CTableImportColumn::eGeneName:
                        field_id = CSeqTable_column_info_Base::eField_id_comment;
                        break;
                    case CTableImportColumn::eGenotype:
                        field_id = CSeqTable_column_info_Base::eField_id_comment;
                        break;
                    case CTableImportColumn::eStrand:
                        field_id = CSeqTable_column_info_Base::eField_id_location_strand;
                        break;
                    case CTableImportColumn::eVariationName:
                        field_id = CSeqTable_column_info_Base::eField_id_comment;
                        break;
                    case CTableImportColumn::eSnpName:
                        field_id = CSeqTable_column_info_Base::eField_id_comment;
                        break;
                    case CTableImportColumn::eChromosomeNumber:
                        field_id = CSeqTable_column_info_Base::eField_id_comment;
                        break;
                    case CTableImportColumn::eDataRegion:                 
                        field_id = CSeqTable_column_info_Base::eField_id_data_region;
                        break;
                    case CTableImportColumn::eUnspecifiedText:
                        field_id = CSeqTable_column_info_Base::eField_id_comment;
                        break;
                    default:
                        field_id = CSeqTable_column_info_Base::eField_id_comment;
                        break;
                 };
                 break;
             }

             default:
                 // All options already accounted for.
                 break;
         };
       
         properties = "&xtype=" + value_type + properties;
         cinfo->SetField_id(field_id);
         cinfo->SetTitle(m_Columns[i].GetName());

         // Add any other properties set by the caller
         const map<string,string>& pmap = m_Columns[i].GetPropertyValues();
         map<string,string>::const_iterator iter;

         for (iter=pmap.begin(); iter!=pmap.end(); ++iter) {
             string prop = "&" + (*iter).first + "=" + (*iter).second;
             properties += prop;
         }

         CRef< CSeqTable_column > column;
         column.Reset(new CSeqTable_column());

         column->SetHeader(*cinfo);
         CRef<CSeqTable_multi_data> data;
         data.Reset(new CSeqTable_multi_data());

         switch (m_Columns[i].GetType()) {
             case CTableImportColumn::eSeqIdColumn:
                 if (field_id == CSeqTable_column_info_Base::eField_id_location_gi)
                    data->Select(CSeqTable_multi_data_Base::e_Int);
                 else 
                    data->Select(CSeqTable_multi_data_Base::e_Id);
                 break;
             case CTableImportColumn::eNumberColumn:
                 data->Select(CSeqTable_multi_data_Base::e_Int);                
                 break;
             case CTableImportColumn::eRealNumberColumn:
                 data->Select(CSeqTable_multi_data_Base::e_Real);                
                 break;
             case CTableImportColumn::eTextColumn:
                 data->Select(CSeqTable_multi_data_Base::e_String);
                 break;
             case CTableImportColumn::eSkippedColumn:
                 break;
             default:
                 break;
         }

         // The number of columns is one greater than the number of fields since
         // the first column is the row number (and the only rows to import are
         // those rows were the row number is an integer, not '-').
         int field_num = i-1;

         // subtract 1 from numeric start/stop position fields that are one based
         int num_delta = 0;
         if (m_Columns[i].GetOneBased() && 
             (m_Columns[i].GetDataType() == CTableImportColumn::eStartPosition ||
              m_Columns[i].GetDataType() == CTableImportColumn::eStopPosition)) {
                 num_delta = 1;
         }

         string field_string_value;

         for (size_t j=m_ImportFromRow; j<m_TableEntries.size(); ++j) {      

             // If this is a comment row, ignore it.  (Header rows can also
             // be ignored this way, but we start at m_ImportFromRow, which is
             // after the header rows)
             if (m_TableEntries[j].GetRowNum() == -1)
                 continue;

             field_string_value = "";

             // Some rows may have fewer fields (data problems).  If so,
             // we will put in default values (since each column is in a separate
             // array missing entries would cause fields to not line up as
             // in the original rows)
             if (field_num < (int)m_TableEntries[j].GetNumFields()) {
                 field_string_value = m_TableEntries[j].GetField(field_num);

                 NStr::TruncateSpacesInPlace(field_string_value);

                 switch (m_Columns[i].GetType()) {
                     case CTableImportColumn::eSeqIdColumn:
                         {
                             if ( value_type == "Chromosome" || value_type == "Rsid" ) {
                                 data->SetString().push_back(field_string_value);
                             }
                             else {
                                 try {
                                     // Currently we save gi's as ids so we won't use this (int) gi
                                     // since it doesn't work with broadcasting when table is loaded.
                                     if (field_id == CSeqTable_column_info_Base::eField_id_location_gi) {
                                         CSeq_id* gid = new CSeq_id(field_string_value);                                         
                                         data->SetInt().push_back(GI_TO(int, gid->GetGi()));
                                     }
                                 }
                                 catch (CException& e) {
                                     e.ReportThis();
                                     data->SetInt().push_back(0);
                                 }

                                 try {
                                     if (field_id == CSeqTable_column_info_Base::eField_id_id_local) {
                                         CRef<CSeq_id>  id(new CSeq_id(
                                             CSeq_id_Base::e_Local, field_string_value));
                                         data->SetId().push_back(id);
                                     }
                                     else {
                                         CRef<CSeq_id>  id(new CSeq_id(field_string_value));
                                         data->SetId().push_back(id);
                                     }
                                 }
                                 catch (CException& e) {
                                     e.ReportThis(); //e_not_set 0
                                     CRef<CSeq_id>  id(new CSeq_id());
                                     data->SetId().push_back(id);
                                 }
                             }
                         }
                         break;
                     case CTableImportColumn::eNumberColumn:
                         {
                             // We use the unsigned conversion since it supports
                             // suffices like KB, MB.  But since its unsigned 
                             // we have to handle negative numbers ourselves
                             int sign_val = 1;

                             if (field_string_value.length() > 1 && 
                                 field_string_value[0] == '-') {
                                     field_string_value = field_string_value.substr(1, field_string_value.length()-1);
                                     sign_val = -1;
                             }

                             int val = (int)NStr::StringToUInt8_DataSize(field_string_value, 
                                 NStr::fConvErr_NoThrow|NStr::fAllowCommas);

                             // need to set 1-based ints to 0-based in some cases.  Only do this
                             // if number is positive since a 0 or negative number in a field that is
                             // supposed to be 'one-based' is not really meaningful
                             if (sign_val == 1 && val > 0)
                                val -= num_delta;

                             val *= sign_val;

                             data->SetInt().push_back(val);
                         }
                         break;
                     case CTableImportColumn::eRealNumberColumn:
                         {
                             double val = NStr::StringToDouble(field_string_value, 
                                 NStr::fConvErr_NoThrow);
                             data->SetReal().push_back(val);            
                         }
                         break;
                     case CTableImportColumn::eTextColumn:
                         {
                             data->SetString().push_back(field_string_value);
                         }
                         break;
                     default:
                         break;
                 }
             }
             else {
                 // No data was in the table for this field - log an error showing what
                 // data was missing (1-based indices for log)
                 ERR_POST(Error << "Table missing column value for row, column: (" 
                     << j-m_ImportFromRow + 1 << ", " << i << ")");
                                  
                 // Put in some default value in based on column type
                 switch (m_Columns[i].GetType()) {
                     case CTableImportColumn::eSeqIdColumn:
                         {
                             if ( value_type == "Chromosome") {
                                 data->SetString().push_back("");
                             }
                             else {
                                 try {
                                     // Currently we save gi's as ids so we won't use this (int) gi
                                     // since it doesn't work with broadcasting when table is loaded.
                                     if (field_id == CSeqTable_column_info_Base::eField_id_location_gi) {                                       
                                         data->SetInt().push_back(0); //??
                                     }
                                     if (field_id == CSeqTable_column_info_Base::eField_id_id_local) {
                                        CRef<CSeq_id>  id(new CSeq_id(
                                            CSeq_id_Base::e_Local, ""));
                                        data->SetId().push_back(id);
                                     }
                                     else {
                                        CRef<CSeq_id>  id(new CSeq_id(""));
                                        data->SetId().push_back(id);
                                     }
                                 }
                                 catch (CException& e) {
                                     e.ReportThis();
                                 }
                             }
                         }
                         break;
                     case CTableImportColumn::eNumberColumn:
                         {
                             // What is a reasonable default for a number? -1, 0, maxint?
                             // Use -1 to force an error if an attempt is made to convert
                             // it to a seq-loc or feature.
                             data->SetInt().push_back(-1);
                         }
                         break;
                     case CTableImportColumn::eRealNumberColumn:
                         {
                             // default: -1.0 
                             data->SetReal().push_back(-1.0);            
                         }
                         break;
                     case CTableImportColumn::eTextColumn:
                         {
                             data->SetString().push_back("");
                         }
                         break;
                     default:
                         break;
                 }
             }
         }


         column->SetData(*data);    
         table->SetColumns().push_back(column);

         // Not required for qualifier tables
         column_meta_info->AddField(label, properties);
     }

     CRef<CObject_id> column_meta_info_id;
     column_meta_info_id.Reset(new CObject_id());
     column_meta_info_id->SetStr("Column Meta Info");

     column_meta_info->SetType(*column_meta_info_id);
     // Clear any existing user objects;
     annot_container->SetDesc().Set().clear();
     // Add user object with meta info for all fields in this table
     annot_container->AddUserObject(*column_meta_info);    

     /// file name of source table (fname is a temporary asn file)
     CFile f(string(m_FileName.ToUTF8()));
     annot_container->SetTitleDesc(f.GetName());

     /// See e:\src\gbench\include\objects\seqfeat\SeqFeatData_.hpp
     table->SetFeat_type(CSeqFeatData_Base::e_not_set);

     CRef<CSeq_annot::TData> d;
     d.Reset(new CSeq_annot::TData());
     d->SetSeq_table(*table);

     annot_container->SetData(*d);
 }

 void CTableImportDataSource::LogColumnInfo() const
 {
     LOG_POST(Info << "Table Import column descriptions: ");
     for (size_t i=1; i<m_Columns.size(); ++i) {
         m_Columns[i].LogColumnInfo();
     }
 }

 void CTableImportDataSource::ExportTableParms(CUser_object& user_object)
 {
     user_object.AddField("comment-char", (int)m_CommentChar);
     user_object.AddField("table-type", (int)m_TableType);
     user_object.AddField("file-type", (int)m_FileType);     
     user_object.AddField("first-row", m_ImportFromRow);
     user_object.AddField("column-header-row", m_ColumnHeaderRow);
     user_object.AddField("recompute-delimiters", m_UseCurrentDelimiters);

     CRef<CUser_field> delimiter_object(new CUser_field());
     delimiter_object->SetLabel().SetStr() = "delimiters";
     m_DelimRules.SaveAsn(delimiter_object.GetObject());
     user_object.SetData().push_back(delimiter_object);
     
     CRef<CUser_field> column_vec(new CUser_field());
     column_vec->SetLabel().SetStr() = "column-array";

     vector<CRef<CUser_field> > columns;

     for (size_t i=1; i<m_Columns.size(); ++i) {
         CRef<CUser_field> column(new CUser_field());
         column->SetLabel().SetStr() = "column";

         m_Columns[i].SaveAsn(column.GetObject());

         columns.push_back(column);
     }

     column_vec->SetData().SetFields() = columns;
     user_object.SetData().push_back(column_vec);
 }

void CTableImportDataSource::ImportTableParms(CUser_object& user_object)
{
    if (user_object.HasField("comment-char") && 
        user_object.GetField("comment-char").GetData().IsInt()) {
            m_CommentChar = (char)user_object.
                GetField("comment-char").GetData().GetInt();
    }

    if (user_object.HasField("table-type") && 
        user_object.GetField("table-type").GetData().IsInt()) {
            m_TableType = (EFieldSeparatorType)user_object.
                GetField("table-type").GetData().GetInt();
    }

    if (user_object.HasField("file-type") && 
        user_object.GetField("file-type").GetData().IsInt()) {
            m_FileType = (ETableFileType)user_object.
                GetField("file-type").GetData().GetInt();
    }

    if (user_object.HasField("first-row") && 
        user_object.GetField("first-row").GetData().IsInt()) {
            m_ImportFromRow = user_object.
                GetField("first-row").GetData().GetInt();
    }

    if (user_object.HasField("column-header-row") && 
        user_object.GetField("column-header-row").GetData().IsInt()) {
            m_ColumnHeaderRow = user_object.
                GetField("column-header-row").GetData().GetInt();
    }

    if (user_object.HasField("recompute-delimiters") && 
        user_object.GetField("recompute-delimiters").GetData().IsBool()) {
            m_UseCurrentDelimiters = user_object.
                GetField("recompute-delimiters").GetData().GetBool();
    }

    if (user_object.HasField("delimiters")) {      
        CUser_field& delimiter_object = user_object.SetField("delimiters");
        m_DelimRules.LoadAsn(delimiter_object);
    }

    if (user_object.HasField("column-array")) {        
        const CUser_field& columns = user_object.GetField("column-array");

        // Erase any existing columns excpet the first (line number) column since that
        // is never stored
        if (m_Columns.size() > 1)
            m_Columns.erase(m_Columns.begin()+1, m_Columns.end());

        if (columns.GetData().IsFields()) {
            vector<CRef<CUser_field> > col_fields = columns.GetData().GetFields();

            for (size_t i=0; i<col_fields.size(); ++i) {
                CTableImportColumn col;
                col.LoadAsn(col_fields[i].GetObject());
                m_Columns.push_back(col);
            }
        }
    }
}


END_NCBI_SCOPE
