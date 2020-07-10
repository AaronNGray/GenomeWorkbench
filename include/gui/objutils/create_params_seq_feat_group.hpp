#ifndef GUI_OBJUTILS___CREATE_PARAMS_SEQ_FEAT_GROUP__HPP
#define GUI_OBJUTILS___CREATE_PARAMS_SEQ_FEAT_GROUP__HPP

/*  $Id: create_params_seq_feat_group.hpp 41725 2018-09-14 21:07:12Z evgeniev $
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
* Authors: Vladislav Evgeniev
*
* File Description:
*
*/

#include <gui/gui_export.h>
#include <objects/seqfeat/Seq_feat.hpp>
#include <gui/objutils/interface_registry.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CCreateParamsSeqFeatGroup :
    public ICreateParams
{
public:
    using TSeqFeatMapInfoPair = CGuiObjectInfoSeq_feat::TSeqFeatMapInfoPair;
    using TSeqFeatVector = CGuiObjectInfoSeq_feat::TSeqFeatVector;

public:
    void Add(const CConstRef<objects::CSeq_feat> &feat, const CSeqUtils::TMappingInfo &mapping_info = CSeqUtils::TMappingInfo())
    {
        m_FeaturesGroup.push_back(std::make_pair(feat, mapping_info));
    }
    const TSeqFeatVector& GetFeaturesGroup() const
    {
        return m_FeaturesGroup;
    }
    virtual ~CCreateParamsSeqFeatGroup() = default;

private:
    TSeqFeatVector  m_FeaturesGroup;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___CREATE_PARAMS_SEQ_FEAT_GROUP__HPP
