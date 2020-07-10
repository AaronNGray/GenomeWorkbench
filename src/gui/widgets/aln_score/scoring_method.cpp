/*  $Id: scoring_method.cpp 37049 2016-11-30 19:40:50Z shkeda $
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
 * Authors:  Andrey Yazhuk
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistl.hpp>
#include <corelib/ncbireg.hpp>
#include <corelib/ncbifile.hpp>

#include <gui/widgets/aln_score/scoring_method.hpp>
#include <gui/objutils/utils.hpp>

#include <math.h>
#include <stdio.h>

BEGIN_NCBI_SCOPE


const char* kDNATag = "DNA";
const char* kProteinTag = "Protein";
const char* kMixedTag = "Mixed";
const char* kInvalidTag = "Invalid";


const char* IScoringMethod::GetAlignmentTagByType(IAlnExplorer::EAlignType type)
{
    switch(type)    {
    case IAlnExplorer::fDNA:
        return kDNATag;
    case IAlnExplorer::fProtein:
        return kProteinTag;
    case IAlnExplorer::fMixed:
        return kMixedTag;
    default:
        _ASSERT(false);
        return "Invalid";
    }
}


IAlnExplorer::EAlignType IScoringMethod::GetAlignmentTypeByTag(const string& tag)
{
    if(tag == kDNATag)  {
        return IAlnExplorer::fDNA;
    } else if(tag == kProteinTag)   {
        return IAlnExplorer::fProtein;
    } else if(tag == kMixedTag)   {
        return IAlnExplorer::fMixed;
    } else {
        return IAlnExplorer::fInvalid;
    }
}

int IScoringAlignment::GetGenCode(IAlnExplorer::TNumrow row) const
{
    int gencode = 1;
    if (!m_GenCodeCache || (*m_GenCodeCache)[row] == -1) {
        gencode = CSeqUtils::GetGenCode(GetBioseqHandle(row));
        m_GenCodeCache.reset(new vector<int>(GetNumRows(), -1));
        (*m_GenCodeCache)[row] = gencode;
    } else {
        gencode = (*m_GenCodeCache)[row];
    }
    return gencode;
}
///////////////////////////////////////////////////////////////////////////////
/// CTemplateScoringMethod

CTemplateScoringMethod::CTemplateScoringMethod() :
    m_Averageable(false),
    m_ColorGradType(eTwoColorGradient)
{
}


CTemplateScoringMethod::~CTemplateScoringMethod()
{
}


string CTemplateScoringMethod::GetName() const
{
    return m_Name;
}


string CTemplateScoringMethod::GetDescription() const
{
    return m_Descr;
}


int CTemplateScoringMethod::GetType() const
{
    return m_Type;
}


bool CTemplateScoringMethod::IsAverageable() const
{
    return m_Averageable;
}


int CTemplateScoringMethod::GetSupportedColorTypes() const
{
    return fBackground;
}


bool    CTemplateScoringMethod::LoadInfo(CNcbiRegistry& reg)
{
    m_Name = reg.GetString("Info", "Name", "");
    m_Descr = reg.GetString("Info", "Description", "");

    // process method type definition
    string type = reg.GetString("Info", "Type", "");
    list<string> values;
    NStr::Split(type, ", ", values, NStr::fSplit_Tokenize); // parse into tokens

    m_Type = 0;
    ITERATE(list<string>, it, values)   { // convert tokens to flags
        const string& val = *it;
        if(val == "DNA")    {
            m_Type |= IAlnExplorer::fDNA;
        } else if(val == "Protein")    {
            m_Type |= IAlnExplorer::fProtein;
        } else {
            ERR_POST("CTemplateScoringMethod::Load() - unsupported method type \""
                     << val << "\".");
        }
    }

    string gradient = reg.Get("Info", "Gradient");

    m_ColorGradType = (gradient == "3Colors") ? eThreeColorGradient
                                            : eTwoColorGradient;
    return true;
}


bool    CTemplateScoringMethod::SaveInfo(CNcbiRegistry& reg)
{
    reg.Set("Info", "Name", m_Name, IRegistry::fPersistent);
    reg.Set("Info", "Description", m_Descr, IRegistry::fPersistent);

    string types;
    if (m_Type & IAlnExplorer::fDNA) {
        types += "DNA ";
    }
    if (m_Type & IAlnExplorer::fProtein) {
        types += "Protein ";
    }
    reg.Set("Info", "Type", types,
        IRegistry::fPersistent | IRegistry::fTruncate);

    string gradtype;
    if (m_ColorGradType == eThreeColorGradient) {
        gradtype = "3Colors";
    } else {
        gradtype = "2Colors";
    }
    reg.Set("Info", "Gradient", gradtype, IRegistry::fPersistent);

    return true;
}


bool    CTemplateScoringMethod::Load(CNcbiRegistry& reg)
{
    return LoadInfo(reg);
}


void CTemplateScoringMethod::SetName(const string& s)
{
    m_Name = s;
}


void CTemplateScoringMethod::SetDescription(const string& s)
{
    m_Descr = s;
}


bool    CTemplateScoringMethod::Save(CNcbiRegistry& reg)
{
    return SaveInfo(reg);
}


CRgbaColor    CTemplateScoringMethod::x_GetColor(CNcbiRegistry& reg,
                                               const string& key,
                                               const string def_value)
{
    CRgbaColor color(0.0f, 0.0f, 0.0f);
    try {
        string str = reg.GetString("Table", key, def_value);
        color = CRgbaColor::GetColor(str);
    }
    catch (std::exception&) {
    }
    return color;
}


END_NCBI_SCOPE
