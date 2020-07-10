#ifndef GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_ENUMS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_ENUMS__HPP

/*  $Id: feature_enums.hpp 34760 2016-02-08 15:54:49Z rudnev $
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
 * Authors:  Dmitry Rudnev
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

BEGIN_NCBI_SCOPE

enum ELinkedFeatDisplay {
    eLFD_Expanded,
    eLFD_Hidden,
    eLFD_Packed,
    eLFD_PackedWithParent,
    eLFD_ParentHidden,
    eLFD_Invalid,
    eLFD_Expandable,
    eLFD_Default = eLFD_Expanded
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_ENUMS__HPP
