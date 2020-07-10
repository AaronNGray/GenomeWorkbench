#ifndef GUI_WIDGET_SEQ_GRAPHICS___ALIGNMENT_SORTER_FACTORY__HPP
#define GUI_WIDGET_SEQ_GRAPHICS___ALIGNMENT_SORTER_FACTORY__HPP

/*  $Id:
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
 * Authors:  Vladislav Evgeniev
 *
 * File Description: Factory for alignment sorting classes
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/seq_graphic/feature_sorter.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CChoice;
END_SCOPE(objects)

class  NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAlignSorterFactory
{
public:

    static IAlignSorter* CreateAlignSorter(std::string const& sort_by, std::string const& sort_str);
    static void SetTrackSetting(objects::CChoice& choice);

    static std::string const& GetHaplotypeID();

protected:
    CAlignSorterFactory() = default;
    CAlignSorterFactory(const CAlignSorterFactory&) = default;
};

END_NCBI_SCOPE


#endif  // GUI_WIDGET_SEQ_GRAPHICS___ALIGNMENT_SORTER_FACTORY__HPP
