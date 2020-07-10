#ifndef GUI_UTILS__PRINT_UTILS_HPP
#define GUI_UTILS__PRINT_UTILS_HPP

/*  $Id: print_utils.hpp 15930 2008-02-08 13:38:34Z dicuccio $
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
 *   print-related utilities
 *
 */


#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>


BEGIN_NCBI_SCOPE



class NCBI_GUIPRINT_EXPORT CUnit
{
public:
    enum EUnit
    {
        eInch,
        eMillimeter,
        ePdfUnit
    };

    typedef enum EUnit TUnit;

    typedef float TInch;
    typedef float TMillimeter;
    typedef float TPdfUnit;


    static string UnitToString(TUnit unit)
    {
        switch (unit) {
        case eInch:
            return "inch";
        case eMillimeter:
            return "millimeter";
        case ePdfUnit:
            return "point";
        }

        NCBI_THROW(CException, eUnknown, "CUnit::UnitToString: unknown unit");
    }

    static string UnitToSymbol(TUnit unit)
    {
        switch (unit) {
        case eInch:
            return "\"";
        case eMillimeter:
            return "mm";
        case ePdfUnit:
            return "pt";
        }

        return "";
    }
};


inline CUnit::TPdfUnit InchesToPdfUnits(CUnit::TInch x)
{
    return x * 72;
}

inline CUnit::TPdfUnit MmToPdfUnits(CUnit::TMillimeter x)
{
    return InchesToPdfUnits(x / 25.4f);
}

inline CUnit::TInch PdfUnitsToInches(CUnit::TPdfUnit x)
{
    return x / 72;
}

inline CUnit::TMillimeter PdfUnitsToMm(CUnit::TPdfUnit x)
{
    return PdfUnitsToInches(x) * 25.4f;
}



END_NCBI_SCOPE

#endif  // GUI_UTILS__PRINT_UTILS_HPP
