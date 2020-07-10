#ifndef GUI_WIDGETS_HIT_MATRIX___HISTOGRAM_GRAPH__HPP
#define GUI_WIDGETS_HIT_MATRIX___HISTOGRAM_GRAPH__HPP

/*  $Id: density_map_ds.hpp 30454 2014-05-21 16:48:42Z katargir $
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

#include <corelib/ncbistl.hpp>

#include <gui/widgets/gl/histogram_graph.hpp>

#include <gui/objutils/density_map.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CDensityMapDS
class CDensityMapDS : public CObject, public IHistogramGraphDS
{
public:
    static void GetGraphTypes(vector<string>& types);

    CDensityMapDS(objects::CBioseq_Handle& handle, const string& type);
    virtual ~CDensityMapDS();

    /// @name IHistogramDS implementation
    /// @{
    virtual double  GetLimit() const;

    virtual double  GetStart() const;
    virtual double  GetStop() const;
    virtual double  GetStep() const;

    virtual size_t     GetCount() const;
    virtual double  GetValue(size_t index);

    virtual double  GetMaxValue();

    virtual string  GetLabel() const;

    virtual void    Update(double start, double stop);
    /// @}

protected:
    objects::SAnnotSelector x_GetAnnotSelector();

protected:
    typedef CDensityMap<int>    TMap;

    objects::CBioseq_Handle  m_Bioseq;
    string  m_Type;
    TMap*   m_Map;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___HISTOGRAM_GRAPH__HPP
