#ifndef GUI_UTILS___USER_TYPE__HPP
#define GUI_UTILS___USER_TYPE__HPP

/*  $Id: user_type.hpp 26143 2012-07-25 19:53:48Z falkrb $
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

#include <corelib/ncbistr.hpp>

#include <gui/gui_export.h>

/** @addtogroup GUI_UTILS
 *
 * @{
 */

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CGUIUserType
class NCBI_GUIOBJUTILS_EXPORT CGUIUserType
{
public:
    // Type definitions
    static const string sm_Tp_Undefined;

    static const string sm_Tp_Alignment;
    static const string sm_Tp_Alignment_Set;
    static const string sm_Tp_Annotation;
    static const string sm_Tp_EntrezGeneRecord;
    static const string sm_Tp_EntrezRecord;
    static const string sm_Tp_Feature;
    static const string sm_Tp_Location;
    static const string sm_Tp_PhylogeneticTree;
    static const string sm_Tp_TaxId_Set;
    static const string sm_Tp_Sequence;
    static const string sm_Tp_Sequence_ID;
    static const string sm_Tp_Sequence_Set;
    static const string sm_Tp_Submission;
    static const string sm_Tp_TaxPlotData;
    static const string sm_Tp_ValidationResult;
    static const string sm_Tp_SeqTable;
    static const string sm_Tp_Variation;

    static const string sm_Tp_Project;
    static const string sm_Tp_ProjectItem;
    static const string sm_Tp_Workspace;

    // User subtypes - sorted alphabetically, grouped by type
    static const string sm_SbTp_Undefined;

    // Sequence subtypes
    static const string sm_SbTp_DNA;
    static const string sm_SbTp_Protein;

    // Annotation subtypes
    static const string sm_SbTp_Alignments;
    static const string sm_SbTp_Features;
    static const string sm_SbTp_Graphs;
    static const string sm_SbTp_Sequence_IDs;
    static const string sm_SbTp_Locations;
    static const string sm_SbTp_Table;

    // Bioseq Set subtypes
    static const string sm_SbTp_NucProt;
    static const string sm_SbTp_SegSeq;
    static const string sm_SbTp_GenProdSet;

    static const string sm_SbTp_PopSet;
    static const string sm_SbTp_PhySet;
    static const string sm_SbTp_EcoSet;
    static const string sm_SbTp_MutSet;

    static const string sm_SbTp_PopSet_Aln;
    static const string sm_SbTp_PhySet_Aln;
    static const string sm_SbTp_EcoSet_Aln;
    static const string sm_SbTp_MutSet_Aln;

    // Project subtypes
    static const string sm_SbTp_ProjectVer1;
    static const string sm_SbTp_ProjectVer2;

    CGUIUserType();
    CGUIUserType(const string& type, const string& subtype = sm_SbTp_Undefined);
    bool operator<(const CGUIUserType& other) const;

    string m_Type;
    string m_Subtype;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___USER_TYPE__HPP
