/*  $Id: suc_data.cpp 41183 2018-06-06 16:55:29Z katargir $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/edit/suc_data.hpp>
#include <iterator>   

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CSUCLine::CSUCLine (const string& line, const string& accession, const CObject* obj) : m_Line(line), m_Count(1)
{
    m_RelatedObjects.emplace_back(accession, CConstRef<CObject>(obj));
}


void CSUCLine::Add(const string& accession, const CObject* obj) 
{
    m_Count++; 
    m_RelatedObjects.emplace_back(accession, CConstRef<CObject>(obj));
}

void CSUCBlock::AddBlock(const string& block_text, const string& accession, const CObject* obj)
{
    size_t n_quotes = 0;
    size_t n_brackets_left = 0;
    size_t n_brackets_right = 0;
    bool can_split(true);

    size_t posE = -1;
    do {
        size_t pos = posE + 1;
        posE = block_text.find('\n', pos);
        string line = (posE == string::npos) ? block_text.substr(pos) : block_text.substr(pos, posE - pos);

        if (line.empty())
            continue;

        for (const auto& c : line) {
            if (c == '\"')
                ++n_quotes;
            else if (c == '(')
                ++n_brackets_left;
            else if (c == ')')
                ++n_brackets_right;
        }

        AddLine(line, accession, obj, can_split);
        if (n_quotes % 2 == 0 && n_brackets_left == n_brackets_right)
        {
            m_label.clear();
            can_split = true;
        }
        else
        {
            can_split = false;
        }
    } while (posE != string::npos);
}


void CSUCBlock::AddLine(const string& line, const string& accession, const CObject* obj, bool can_split)
{
    static const char* spaces = " \t";
    if (line.find_first_not_of(spaces) == string::npos)
        return;
   
    {
        auto lit = m_Lines.find(line);
        if (lit == m_Lines.end())
        {            
            CRef<CSUCLine> new_line(new CSUCLine(line, accession, obj));
            m_Lines.emplace(new_line->GetLine(), new_line);
            m_pos = -1;
        }
        else
        {
            lit->second->Add(accession, obj);
        }  
    }

    if ( HasSecondLevel() )
    {
        if (can_split)
        {
            string whole = line;
            whole.erase(0, whole.find_first_not_of(spaces));
            whole.erase(whole.find_last_not_of(spaces) + 1);

            if (!whole.empty() && whole[0] == '/')
                m_label = whole.substr(0, whole.find('='));
            else 
                m_label = whole.substr(0, whole.find_first_of(spaces));
        }
        
        if (!m_label.empty())
        {
            auto& map = m_SecondLevel[m_label];
            auto lit = map.find(line);
            if (lit == map.end())
            {
                CRef<CSUCLine> new_line(new CSUCLine(line, accession, obj));
                map.emplace(new_line->GetLine(), make_pair(can_split, new_line));
            }
            else
            {
                lit->second.second->Add(accession, obj);
            }  
        }
    }
}


void CSUCBlock::Print ()
{
    for (auto& it : m_Lines)
    {
        it.second->Print();
    }
}

CConstRef<CSUCLine> CSUCBlock::GetLine(size_t pos) const 
{ 

    if (m_pos < 0)
    {
        m_pos_it = m_Lines.cbegin();
        advance(m_pos_it, pos);
        m_pos = pos;
    }
    else if (pos != m_pos)
    {
        int diff = pos;
        diff -= m_pos;
        advance (m_pos_it,diff);
        m_pos = pos;
    }

    return CConstRef<CSUCLine>(m_pos_it->second.GetPointer()); 
}

void  CSUCBlock::GetText(wxString &text) const
{
    for (auto& line : m_Lines)
    {
        text << "\t";
        text << line.second->GetCount();
        text << "\t";
        text << line.second->GetLine();
        text << "\n";
    }
}

const char* CSUCBlock::GetBlockLabel(CFlatFileConfig::FGenbankBlocks block_type)
{
    switch (block_type) {
        case CFlatFileConfig::fGenbankBlocks_Locus:
            return "LOCUS";
            break;
        case CFlatFileConfig::fGenbankBlocks_Defline:
            return "DEFLINE";
            break;
        case CFlatFileConfig::fGenbankBlocks_Accession:
            return "ACCESSION";
            break;
        case CFlatFileConfig::fGenbankBlocks_Version:
            return "VERSION";
            break;
        case CFlatFileConfig::fGenbankBlocks_Keywords:
            return "KEYWORDS";
            break;
        case CFlatFileConfig::fGenbankBlocks_Source:
            return "SOURCE";
            break;
        case CFlatFileConfig::fGenbankBlocks_Reference:
            return "REFERENCE";
            break;
        case CFlatFileConfig::fGenbankBlocks_Comment:
            return "COMMENT";
            break;
        case CFlatFileConfig::fGenbankBlocks_Featheader:
        case CFlatFileConfig::fGenbankBlocks_FeatAndGap:
            return "FEATURE";
            break;
        case CFlatFileConfig::fGenbankBlocks_Sourcefeat:
            return "SOURCEFEAT";
            break;
        case CFlatFileConfig::fGenbankBlocks_Basecount:
            return "BASECOUNT";
            break;
        case CFlatFileConfig::fGenbankBlocks_Origin:
            return "ORIGIN";
            break;
        case CFlatFileConfig::fGenbankBlocks_Sequence:
            return "SEQUENCE";
            break;
        case CFlatFileConfig::fGenbankBlocks_Project:
            return "PROJECT";
            break;
        default:
            return "";
            break;
    }
}

bool CSUCBlock::HasSecondLevel()
{
    return m_Block == CFlatFileConfig::fGenbankBlocks_FeatAndGap || m_Block == CFlatFileConfig::fGenbankBlocks_Sourcefeat;
}

CSUCResults::CSUCResults() 
{ 
    m_Blocks.clear(); 
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Locus] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Locus));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Defline] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Defline));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Accession] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Accession));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Version] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Version));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Keywords] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Keywords));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Source] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Source));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Reference] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Reference));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Comment] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Comment));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Sourcefeat] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Sourcefeat));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_FeatAndGap] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_FeatAndGap));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Basecount] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Basecount));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Origin] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Origin));
    m_Blocks[CFlatFileConfig::fGenbankBlocks_Project] = CRef<CSUCBlock>(new CSUCBlock(CFlatFileConfig::fGenbankBlocks_Project));

    m_order.clear();
    m_order.resize(15);
    m_order[0] = CFlatFileConfig::fGenbankBlocks_Locus;
    m_order[1] = CFlatFileConfig::fGenbankBlocks_Defline;
    m_order[2] = CFlatFileConfig::fGenbankBlocks_Accession;
    m_order[3] = CFlatFileConfig::fGenbankBlocks_Version;
    m_order[4] = CFlatFileConfig::fGenbankBlocks_Project;
    m_order[5] = CFlatFileConfig::fGenbankBlocks_Keywords;
    m_order[6] = CFlatFileConfig::fGenbankBlocks_Source;
    m_order[7] = CFlatFileConfig::fGenbankBlocks_Reference;
    m_order[8] = CFlatFileConfig::fGenbankBlocks_Comment;
    m_order[9] = CFlatFileConfig::fGenbankBlocks_Sourcefeat;
    m_order[10] = CFlatFileConfig::fGenbankBlocks_FeatAndGap;
    m_order[11] = CFlatFileConfig::fGenbankBlocks_Basecount;
    m_order[12] = CFlatFileConfig::fGenbankBlocks_Origin;

}


void CSUCResults::AddText(string & block_text, CFlatFileConfig::FGenbankBlocks which_block, const string& accession, CConstRef<CObject> obj)
{
    auto it = m_Blocks.find(which_block);
    if ( it == m_Blocks.end())
    {
        CRef<CSUCBlock> new_block(new CSUCBlock(which_block));
        new_block->AddBlock(block_text, accession, obj);
        m_Blocks[which_block] = new_block;
        m_order.push_back(which_block);
    }
    else
    {
        it->second->AddBlock(block_text, accession, obj);
    }
}


size_t CSUCResults::CountLines() const
{
    size_t count = 0;
    for (auto it = m_Blocks.begin(); it != m_Blocks.end(); ++it)
    {
        if (it->second->GetExpanded()) 
        {
            count += it->second->CountLines();
        }
    }
    return count;
}

void CSUCResults::GetText(wxString& text)
{
    for (size_t i = 0; i < m_order.size(); i++) 
    {
        objects::CFlatFileConfig::FGenbankBlocks which_block = m_order[i];
        auto it = m_Blocks.find(which_block);
        if (it != m_Blocks.end())
        {
            it->second->SetExpanded(true);
            it->second->GetText(text);
        }
    }
}

CConstRef<CSUCLine> CSUCResults::GetLine(size_t pos) const
{
    size_t  block_pos = 0;
    CFlatFileConfig::FGenbankBlocks block_type;
    if (x_GetBlockPos(pos, block_type, block_pos)) 
    {
        auto it = m_Blocks.find(block_type);
        if (it != m_Blocks.end())
            return it->second->GetLine(block_pos);
    }
        
    return CConstRef<CSUCLine>(NULL);
}

CFlatFileConfig::FGenbankBlocks CSUCResults::GetBlockType(size_t pos) const
{
    size_t block_pos = 0;
    CFlatFileConfig::FGenbankBlocks block_type(CFlatFileConfig::fGenbankBlocks_None);
    if (x_GetBlockPos(pos, block_type, block_pos)) 
    {
        return block_type;
    } 
    
  return CFlatFileConfig::fGenbankBlocks_None;
}


void CSUCResults::ExpandBlock(bool val, objects::CFlatFileConfig::FGenbankBlocks which_block)
{
    auto it = m_Blocks.find(which_block);
    if (it != m_Blocks.end())
    {
        it->second->SetExpanded(val);
    }
}


void CSUCResults::ExpandAll(bool val)
{
    for (auto it = m_Blocks.begin(); it != m_Blocks.end(); ++it)
    {
        it->second->SetExpanded(val);
    }
}


bool CSUCResults::GetExpanded(objects::CFlatFileConfig::FGenbankBlocks which_block) const 
{
    bool rval = false;
    auto it = m_Blocks.find(which_block);
    if (it != m_Blocks.end())
    {
        rval = it->second->GetExpanded();
    }
    return rval;
}


bool CSUCResults::x_GetBlockPos(size_t pos, objects::CFlatFileConfig::FGenbankBlocks& block_type, size_t& block_pos) const 
{
    size_t remainder = pos;

    for (size_t i = 0; i < m_order.size(); i++) 
    {
        objects::CFlatFileConfig::FGenbankBlocks which_block = m_order[i];
        auto it = m_Blocks.find(which_block);
        if (it != m_Blocks.end() && it->second->GetExpanded())
        {
            size_t num_lines = it->second->CountLines();
            if (remainder < num_lines) 
            {
                block_type = which_block;
                block_pos = remainder;
                return true;
            }
            remainder -= num_lines;
        }
    }
    return false;
}

END_NCBI_SCOPE

