/*  $Id: ld_filter_params.hpp 19845 2009-08-13 12:59:38Z quintosm $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Melvin Quintos
 */
#ifndef _LD_FILTER_PARAMS_H_
#define _LD_FILTER_PARAMS_H_

#include <corelib/ncbistd.hpp>
#include <wx/wx.h>

BEGIN_NCBI_SCOPE

/*!
 * CLDFilterParams class declaration
 */

class CLDFilterParams: public wxObject
{    
    friend class CLDFilterDialog;
    DECLARE_DYNAMIC_CLASS( CLDFilterParams )
public:
    /// Default constructor for CLDFilterParams
    CLDFilterParams();

    /// Copy constructor for CLDFilterParams
    CLDFilterParams(const CLDFilterParams& data);

    /// Destructor for CLDFilterParams
    ~CLDFilterParams();

    /// Assignment operator for CLDFilterParams
    void operator=(const CLDFilterParams& data);

    /// Equality operator for CLDFilterParams
    bool operator==(const CLDFilterParams& data) const;

    /// Copy function for CLDFilterParams
    void Copy(const CLDFilterParams& data);

    /// Initialises member variables
    void Init();

////@begin CLDFilterParams member function declarations
    int GetScore() const { return m_nScore ; }
    void SetScore(int value) { m_nScore = value ; }

    int GetLengthExp() const { return m_nLengthExp ; }
    void SetLengthExp(int value) { m_nLengthExp = value ; }

////@end CLDFilterParams member function declarations

////@begin CLDFilterParams member variables
private:
    int m_nScore;
    int m_nLengthExp;
////@end CLDFilterParams member variables
};

END_NCBI_SCOPE 
#endif // _LD_FILTER_PARAMS_H_

