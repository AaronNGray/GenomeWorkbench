/*  $Id: gene_field_panel.cpp 33929 2015-10-01 20:21:13Z asztalos $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/gene_field_panel.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>

BEGIN_NCBI_SCOPE

CGeneFieldPanel::CGeneFieldPanel(wxWindow* parent)
    : CSingleChoicePanel(parent, GetStrings())
{
}

static const string kGenePrefix("gene "); 

string CGeneFieldPanel::GetFieldName(const bool subfield)
{
    string qual_name = "";
    int val = m_FieldChoice->GetSelection();
    if (val > -1) {
        qual_name = ToStdString(m_FieldChoice->GetString(val));
        NStr::ReplaceInPlace(qual_name, "-", "_");
        qual_name = kGenePrefix + qual_name;
    }
    return qual_name;
}


bool CGeneFieldPanel::SetFieldName(const string& field)
{
    bool rval = false;
    string subfield(field);
    SIZE_TYPE pos = NStr::FindNoCase(field, kGenePrefix);
    if (pos == 0) {
        subfield = field.substr(kGenePrefix.length(), NPOS);
    }
    for (size_t i = 0; i < m_FieldChoice->GetStrings().size(); i++) {
        string qual_name = ToStdString(m_FieldChoice->GetString(i));
        if (QualifierNamesAreEquivalent(qual_name, subfield)) {
            m_FieldChoice->SetSelection(i);
            rval = true;
            break;
        }
    }
    x_UpdateParent();
    return rval;
}

vector<string> CGeneFieldPanel::GetStrings()
{
    vector<string> options;
    options.push_back("locus");
    options.push_back("description");
    options.push_back("comment");
    options.push_back("allele");
    options.push_back("maploc");
    options.push_back("locus-tag");
    options.push_back("synonym");
    return options;
}

END_NCBI_SCOPE

