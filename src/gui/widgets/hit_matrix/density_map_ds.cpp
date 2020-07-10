/*  $Id: density_map_ds.cpp 24043 2011-07-13 18:20:33Z kuznets $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/hit_matrix/density_map_ds.hpp>

#include <gui/objutils/utils.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
/// CDensityMapDS

CDensityMapDS::CDensityMapDS(CBioseq_Handle& handle, const string& type)
:   m_Bioseq(handle),
    m_Type(type),
    m_Map(0)
{
    TSeqPos len = handle.GetBioseqLength();
    Update(0, len);
}


CDensityMapDS::~CDensityMapDS()
{
    delete m_Map;
}

double CDensityMapDS::GetLimit() const
{
    return m_Bioseq.GetBioseqLength();
}


double CDensityMapDS::GetStart() const
{
    return m_Map->GetStart();
}

double CDensityMapDS::GetStop() const
{
    return m_Map->GetStop();
}


double CDensityMapDS::GetStep() const
{
    return m_Map->GetWindow();
}


size_t CDensityMapDS::GetCount() const
{
    return m_Map->GetBins();
}


double  CDensityMapDS::GetValue(size_t index)
{
    return (*m_Map)[index];
}


double CDensityMapDS::GetMaxValue()
{
    return m_Map->GetMax();
}


string CDensityMapDS::GetLabel() const
{
    return m_Type;
}


static const char* kGraphTypes[] = {
    "Genes",
    "mRNAs",
    "Coding Regions",
    "Variations",
    "Imported Features",
    "Non-standard Imported Features"
};


void CDensityMapDS::GetGraphTypes(vector<string>& types)
{
    int n = sizeof(kGraphTypes) / sizeof(char*);
    for( int i = 0; i < n; i++ )    {
        types.push_back(string(kGraphTypes[i]));
    }
}


SAnnotSelector    CDensityMapDS::x_GetAnnotSelector()
{
    SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
    sel.SetAnnotType(CSeq_annot::TData::e_Ftable);

    if(m_Type == "Genes") {
        sel.SetFeatType(CSeqFeatData::e_Gene);
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_gene);
    } else if(m_Type == "mRNAs") {
        sel.SetFeatType(CSeqFeatData::e_Rna);
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_mRNA);
    } else if(m_Type == "Coding Regions") {
        sel.SetFeatType(CSeqFeatData::e_Cdregion);
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_cdregion);
    } else if(m_Type == "Variations") {
        sel.SetFeatType(CSeqFeatData::e_Imp);
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_variation);
        sel.ResetAnnotsNames();
        //sel.SetAllNamedAnnots();
    } else if(m_Type == "Imported Features") {
        sel.SetFeatType(CSeqFeatData::e_Imp);
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_any);
    } else if(m_Type == "Non-Standard Imported Features") {
        sel.SetFeatType(CSeqFeatData::e_Imp);
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_imp);
    }

    sel.SetOverlapType(SAnnotSelector::eOverlap_TotalRange);
    sel.SetResolveMethod(SAnnotSelector::eResolve_All);
    return sel;
}


static const int kBins = 2048;

void CDensityMapDS::Update(double start, double stop)
{
    _ASSERT(start >= 0  &&  stop <= m_Bioseq.GetBioseqLength() &&  start < stop);

    TSeqPos pos_start = (TSeqPos) floor(start);
    TSeqPos pos_stop = (TSeqPos) ceil(stop);

    if(m_Map == NULL  ||  pos_start != m_Map->GetStart()  ||  pos_stop != m_Map->GetStop()) {
        delete m_Map;

        float window = float(pos_stop - start) / float(kBins);
        m_Map = new TMap(pos_start, pos_stop, window);

        SAnnotSelector sel = x_GetAnnotSelector();
        m_Map->AddFeatures(m_Bioseq, sel);
    }
}


END_NCBI_SCOPE
