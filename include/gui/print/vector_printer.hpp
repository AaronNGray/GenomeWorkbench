#ifndef GUI_UTILS___VECTOR_PRINTER__HPP
#define GUI_UTILS___VECTOR_PRINTER__HPP

/*  $Id: vector_printer.hpp 32423 2015-02-25 15:55:32Z falkrb $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CVectorPrinter - print vector objects
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/print/print_options.hpp>


BEGIN_NCBI_SCOPE


class CPageBuffers;


// a default color that is out of range
static const float defcolor[4] = { 123.0f, 123.0f, 123.0f, 123.0f };


struct CPrintState
{
    CPrintState()
        : m_Stipple(defcolor, 4),
          m_NonStipple(defcolor, 4)
    {
    }

    CRgbaColor m_Stipple;
    CRgbaColor m_NonStipple;
};


class NCBI_GUIPRINT_EXPORT CVectorPrinter : public CObject
{
public:
    CVectorPrinter(CNcbiOstream& ostream = NcbiCerr);
    virtual ~CVectorPrinter();

    virtual void BeginDocument(void) = 0;
    virtual void EndDocument(void) = 0;

    virtual void SetOutputStream(CNcbiOstream* ostream);

    virtual void SetOptions(const CPrintOptions& options);
    virtual CPrintOptions& GetOptions() { return m_Options; }
    virtual const CPrintOptions& GetOptions() const { return m_Options; }


protected:

    CNcbiOstream* m_Strm;
    unsigned int m_PageCount;
    CPrintOptions m_Options;
    auto_ptr<CPageBuffers> m_PageBuffers;   
};



END_NCBI_SCOPE


#endif // GUI_UTILS___VECTOR_PRINTER__HPP
