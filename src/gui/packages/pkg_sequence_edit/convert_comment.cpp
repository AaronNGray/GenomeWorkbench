/*  $Id: convert_comment.cpp 42186 2019-01-09 19:34:50Z asztalos $
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
#include <objmgr/scope.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/packages/pkg_sequence_edit/convert_comment.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;


static void s_split(const string &s, string delim, vector<string> &elems) 
{
    size_t start = 0;
    while (start < s.length())
    {
        size_t finish = s.find(delim,start);
        if (finish == string::npos) finish = s.length();
        string item = s.substr(start,finish-start);
        NStr::TruncateSpacesInPlace (item);
        if (!item.empty()) elems.push_back(item);
        start = finish+delim.length();
    }
}

string CConvertComment::ApplyToComment (string comment, CCmdComposite* composite,CSeq_entry_Handle seh)
{
    string result = comment;

    if (comment.find("-START##") != string::npos && comment.find("-END##") != string::npos && comment.find(m_DelimiterNameValue) != string::npos)
    {
        string prefix,suffix;
        size_t e = comment.find("-START##");
        size_t b = comment.rfind("##",e);
        if (b == string::npos) b = 0;
        e += strlen("-START##");
        prefix = comment.substr(b,e-b);
        comment.erase(0,e);

        e = comment.find("-END##");
        b = comment.rfind("##",e);
        if (b == string::npos) b = 0;
        e += strlen("-END##");
        suffix = comment.substr(b,e-b);
        comment.erase(b,string::npos);

        vector<string> lines;
        string name;
        vector< pair<string,string> > fields;
        s_split(comment,m_DelimiterFields,lines);  
        for (unsigned int i=0; i<lines.size(); ++i)
        {
            NStr::ReplaceInPlace (lines[i], "\r", "");
            vector<string> field;
            s_split(lines[i],m_DelimiterNameValue,field);
            if (field.size() == 2)
                fields.push_back(pair<string,string>(field[0],field[1]));
            else if (field.size() == 1)                                       
            {
                if (!name.empty())
                {
                    fields.push_back(pair<string,string>(name,field[0]));
                    name.clear();
                }
                else
                    name = field[0];
            }
        }
        if (!fields.empty())
        {
            CRef<CSeqdesc> descr(new CSeqdesc());
            descr->Select(CSeqdesc::e_User);
            CUser_object& user = descr->SetUser();
            user.SetType().SetStr("StructuredComment");
            if (!prefix.empty())
                user.AddField("StructuredCommentPrefix",prefix);
            for (unsigned int i=0; i<fields.size(); ++i)
                user.AddField(fields[i].first,fields[i].second);
            if (!suffix.empty())
                user.AddField("StructuredCommentSuffix",suffix);
            
            CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(seh, *descr));
            composite->AddCommand(*cmdAddDesc);

            e = result.find("-START##");
            b = result.rfind("##",e);
            if (b == string::npos) b = 0;
            e = result.find("-END##");
            e += strlen("-END##");
            result.erase(b,e-b);
        }
    }

    return result;
}

void CConvertComment::ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite)  
{

    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
    {
        if ((*it)->IsComment()) 
        {
            CSeq_entry_Handle seh = tse.GetScope().GetSeq_entryHandle(se);
            string modified = ApplyToComment((*it)->GetComment(),composite,seh);
            if (modified != (*it)->GetComment())
            {
                if (modified.empty())
                {
                    CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(seh, **it));
                    composite->AddCommand(*cmdDelDesc);
                }
                else
                {
                    CRef<CSerialObject> edited_object;
                    edited_object.Reset((CSerialObject*)CSeqdesc::GetTypeInfo()->Create());
                    edited_object->Assign(**it);
                    CSeqdesc& edited_seqdesc = dynamic_cast<CSeqdesc&>(*edited_object);
                    edited_seqdesc.SetComment(modified);
                    CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(seh, **it, edited_seqdesc));
                    composite->AddCommand(*cmd);
                }
            }
        }
    }

    if (se.IsSet()) 
    {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) 
        {
            ApplyToCSeq_entry (tse, **it, composite);
        }
    }
}


bool CConvertComment::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title, string delim_name_value, string delim_fields)
{
    if (tse) 
    {
        m_DelimiterNameValue = delim_name_value;
        m_DelimiterFields = delim_fields;
        CRef<CCmdComposite> composite(new CCmdComposite(title)); 
        ApplyToCSeq_entry (tse, *(tse.GetCompleteSeq_entry()), composite);   
        cmdProcessor->Execute(composite.GetPointer());
        return true;
    }
    else
        return false;
}




END_NCBI_SCOPE
