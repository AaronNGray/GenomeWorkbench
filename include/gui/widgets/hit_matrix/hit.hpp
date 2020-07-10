#ifndef GUI_WIDGETS_HIT_MATRIX___HIT__HPP
#define GUI_WIDGETS_HIT_MATRIX___HIT__HPP

/*  $Id: hit.hpp 15168 2007-10-15 19:11:34Z dicuccio $
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

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <objmgr/scope.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objtools/alnmgr/aln_user_options.hpp>

BEGIN_NCBI_SCOPE

class IHit;

///////////////////////////////////////////////////////////////////////////////
/// IHitElement
class NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT IHitElement
{
public:
    typedef size_t  TDim;

    virtual ~IHitElement()  {}

    virtual const IHit&  GetHit() const = 0;
    virtual TSignedSeqPos  GetQueryStart() const = 0;
    virtual TSignedSeqPos  GetSubjectStart() const = 0;
    virtual TSeqPos  GetQueryLength() const = 0;
    virtual TSeqPos  GetSubjectLength() const = 0;
    virtual objects::ENa_strand     GetQueryStrand() const = 0;
    virtual objects::ENa_strand     GetSubjectStrand() const = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// IHit
class NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT IHit
{
public:
    typedef IHitElement::TDim  TDim;
    typedef CAlnUserOptions::EDirection TDirection;

    virtual ~IHit() {};

    virtual TDim   GetElemsCount() const = 0;
    virtual const IHitElement& GetElem(TDim elem_index) const = 0;

    virtual double   GetScoreValue(const string& score_name) const = 0;
    virtual const objects::CSeq_align*   GetSeqAlign() const = 0;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___HIT__HPP
