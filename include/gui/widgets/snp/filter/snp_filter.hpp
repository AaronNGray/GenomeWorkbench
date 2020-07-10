#ifndef GUI_WIDGETS_SNP___SNP_FILTER_HPP
#define GUI_WIDGETS_SNP___SNP_FILTER_HPP

/*  $Id: snp_filter.hpp 36010 2016-07-27 18:52:00Z rudnev $
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
 * Authors:  Melvin Quintos
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>
#include <objtools/snputil/snp_bitfield.hpp>

BEGIN_NCBI_SCOPE

struct NCBI_GUIWIDGETS_SNP_EXPORT SSnpFilter
    : public CObject
{
    SSnpFilter();
    SSnpFilter(const string &str);
    
    string  name;

    bool    checkLinks;
    bool    checkGeneFxn;
    bool    checkMapping;
    bool    checkWeight;
    bool    checkAlleleFreqList;
    bool    checkAlleleFreqChoice;
    bool    checkGaP_Hapmap;
    bool    checkVarClass;
    bool    checkQualityCheck;

    int     choiceWeight;
    int     choiceAlleleFreq;
    int     choiceVarClass;

    typedef list<int> TList;
    TList   listLinks;
    TList   listGeneFxns;
    TList   listMappings;
    TList   listAlleleFreqs;
    TList   listGaP_Hapmaps;
    TList   listQualityChecks;

    void Clear();
    void SerializeTo( string &output ) const;
    void SerializeFrom( string input );

    // Inherited from CFilterT<CSnpBitfield>
    virtual bool Passes(const CSnpBitfield &b) const;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SNP___SNP_FILTER_HPP

