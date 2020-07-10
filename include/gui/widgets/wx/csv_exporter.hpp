#ifndef GUI_WIDGETS_WX___CSV_EXPORTER__HPP
#define GUI_WIDGETS_WX___CSV_EXPORTER__HPP

/*  $Id: csv_exporter.hpp 36710 2016-10-26 16:51:44Z evgeniev $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>
#include <wx/string.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CCSVExporter
{
public:
    CCSVExporter(CNcbiOstream& os, char delimiter, char quote, bool quote_all = true) :
        m_OS(os), m_Delimiter(delimiter), m_Quote(quote), m_NewRow(true), m_QuoteAll(quote_all) {}

    void Field(const string& value);
    void Field(const wxString& value);
    void Field(long value);
    void Field(double value);
    void NewRow();

protected:
    CNcbiOstream& m_OS;
    char m_Delimiter;
    char m_Quote;
    bool m_NewRow;
    bool m_QuoteAll;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___CSV_EXPORTER__HPP
