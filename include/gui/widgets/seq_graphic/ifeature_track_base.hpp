#ifndef GUI_WIDGETS_SEQ_GRAPHIC___IFEATURE_TRACK_BASE__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___IFEATURE_TRACK_BASE__HPP

/*  $Id: ifeature_track_base.hpp 44424 2019-12-17 16:14:32Z filippov $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///   IFeatureTrackBase --
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT IFeatureTrackBase 
{
public:

    virtual ~IFeatureTrackBase() {}
    virtual void GetFeatSubtypes(set<objects::CSeqFeatData::ESubtype> &subtypes) const = 0;
    virtual string GetAnnot() const = 0;
    virtual bool IsVarTrack() const {return false;}
    virtual bool IsSnpTrack() const {return false;}
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___IFEATURE_TRACK_BASE__HPP
