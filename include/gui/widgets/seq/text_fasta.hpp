#ifndef GUI_WIDGETS_SEQ___TEXT_FASTA__HPP
#define GUI_WIDGETS_SEQ___TEXT_FASTA__HPP

/*  $Id: text_fasta.hpp 15168 2007-10-15 19:11:34Z dicuccio $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <gui/widgets/fl/text.hpp>


BEGIN_NCBI_SCOPE


class CFastaTextDS;


class NCBI_GUIWIDGETS_SEQ_EXPORT CFastaTextDisplay
    :   public CTextEditor
{
public:
    CFastaTextDisplay(int x, int y, int w, int h, const char* label = NULL);
    ~CFastaTextDisplay();

    void SetDataSource(CFastaTextDS& ds);

    // update the widget to reflect changes in the data source
    void Update();

protected:
    CFastaTextDisplay(const CFastaTextDisplay&);
    CFastaTextDisplay& operator=(const CFastaTextDisplay&);

    //TODO int x_OnCopy();

protected:
    CRef<CFastaTextDS> m_DataSource;
};



END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___TEXT_FASTA__HPP
