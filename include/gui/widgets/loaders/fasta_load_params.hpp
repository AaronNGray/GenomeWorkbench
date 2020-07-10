#ifndef GUI_WIDGETS___LOADERS___FASTA_LOAD_PARAMS__HPP
#define GUI_WIDGETS___LOADERS___FASTA_LOAD_PARAMS__HPP

/*  $Id: fasta_load_params.hpp 44796 2020-03-17 22:37:42Z evgeniev $
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
 * Authors:  Roman Katargin
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/objutils/reg_settings.hpp>

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

/*!
 * CFastaLoadParams class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CFastaLoadParams: public IRegSettings
{
    friend class CFASTAFormatParamsPanel;
public:
    /// Default constructor for CFastaLoadParams
    CFastaLoadParams();

    /// Copy constructor for CFastaLoadParams
    CFastaLoadParams(const CFastaLoadParams& data);

    /// Destructor for CFastaLoadParams
    ~CFastaLoadParams();

    /// Assignment operator for CFastaLoadParams
    void operator=(const CFastaLoadParams& data);

    /// Equality operator for CFastaLoadParams
    bool operator==(const CFastaLoadParams& data) const;

    /// Copy function for CFastaLoadParams
    void Copy(const CFastaLoadParams& data);

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    ////@begin CFastaLoadParams member function declarations
    bool GetForceLocalIDs() const { return m_ForceLocalIDs ; }
    void SetForceLocalIDs(bool value) { m_ForceLocalIDs = value ; }

    bool GetIgnoreGaps() const { return m_IgnoreGaps ; }
    void SetIgnoreGaps(bool value) { m_IgnoreGaps = value ; }

    int GetLowercaseOption() const { return m_LowercaseOption ; }
    void SetLowercaseOption(int value) { m_LowercaseOption = value ; }

    bool GetMakeDelta() const { return m_MakeDelta ; }
    void SetMakeDelta(bool value) { m_MakeDelta = value ; }

    bool GetNoSplit() const { return m_NoSplit ; }
    void SetNoSplit(bool value) { m_NoSplit = value ; }

    bool GetReadFirst() const { return m_ReadFirst ; }
    void SetReadFirst(bool value) { m_ReadFirst = value ; }

    int GetSeqType() const { return m_SeqType ; }
    void SetSeqType(int value) { m_SeqType = value ; }

    bool GetSkipInvalid() const { return m_SkipInvalid ; }
    void SetSkipInvalid(bool value) { m_SkipInvalid = value ; }

////@end CFastaLoadParams member function declarations

////@begin CFastaLoadParams member variables
private:
    bool m_ForceLocalIDs;
    bool m_IgnoreGaps;
    int m_LowercaseOption;
    bool m_MakeDelta;
    bool m_NoSplit;
    bool m_ReadFirst;
    int m_SeqType;
    bool m_SkipInvalid;
////@end CFastaLoadParams member variables

    string m_RegPath;
};

/* @} */

END_NCBI_SCOPE

#endif  // GUI_WIDGETS___LOADERS___FASTA_LOAD_PARAMS__HPP
