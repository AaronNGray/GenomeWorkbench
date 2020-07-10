#ifndef GUI_WIDGETS_ALN_SCORE___QUALITY_SCORING_METHODS__HPP
#define GUI_WIDGETS_ALN_SCORE___QUALITY_SCORING_METHODS__HPP


/*  $Id: quality_methods.hpp 40237 2018-01-12 17:40:24Z katargir $
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
 * Authors:  Andrey Yazhuk
 */

#include <corelib/ncbistd.hpp>

#include <gui/opengl/glcolortable.hpp>
#include <util/tables/raw_scoremat.h>

#include <gui/widgets/wx/ui_tool.hpp>

#include <gui/widgets/aln_score/scoring_method.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CQualityScoringMethod -
/// Like CSimpleScoringMethod, in that it calculate the quality of the alignment
/// column and residue, based on how different each residue is from the columns
/// consensus.

class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CQualityScoringMethod
    :   public CObject,
        public IScoringMethod,
        public IColumnScoringMethod,
        public IUIToolWithGUI
{
public:
    enum    EOptions    {
        fIgnoreEmptySpace   = 0x01, ///< Do not count unaligned regions in score
        fIgnoreGaps         = 0x02, ///< Do not count gaps in the alignment in the score
        fScoreWholeColumn   = 0x04  ///< The column's total score is also the score for each residue.
    };

    friend class CQualityScoringPanel;

    CQualityScoringMethod();
    CQualityScoringMethod(int colors); // intializes color table
    CQualityScoringMethod(const CQualityScoringMethod& orig);
    virtual ~CQualityScoringMethod();

    void    CreateColorTable(int size);

    virtual TNCBIScore  GetSubstitutionScore(int aa1, int aa2) const = 0;
    virtual string      GetAllowedSymbols() const = 0;
    bool        IsScoreWholeColumn() const;
    void        SetOptions(int options); // takes as argument combination of EOptions flags
    int         GetOptions() const;

    /// @name IScoringMethod implementation
    /// @{
    virtual string  GetDescription() const;

    virtual bool    IsAverageable() const;

    virtual int     GetSupportedColorTypes() const;

    virtual const CRgbaColor& GetColorForScore(TScore score, EColorType type) const;
    /// @}

    /// @name IColumnScoringMethod implementaion
    /// @{
    virtual void    CalculateScores(char cons, const string& column,
                                    TScore& col_score, TScoreVector& scores);

    /// Do we have what it takes to calculate scores? e.g. a master row selected?
    virtual bool    CanCalculateScores(const IScoringAlignment& aln);

    /// Call for display colors when CanCalculateScores returns false.
    virtual const CRgbaColor& GetColorForNoScore(EColorType type) const;
    virtual void     SetColorForNoScore(EColorType type, const CRgbaColor& color);
    /// @}

    /// @name IUIToolWithGUI implementation
    /// @{
    virtual const CMenuItem*    GetMenu();
    virtual bool        HasPropertiesPanel() const;
    virtual wxWindow* CreatePropertiesPanel(wxWindow* parent);
    /// @}

private:
    void x_Init(int colors);

protected:
    CRgbaColor    m_Worst;
    CRgbaColor    m_Best;
    CRgbaGradColorTable   m_ColorTable;

    int     m_Options;
    char    m_Space;
    char    m_Gap;
};


////////////////////////////////////////////////////////////////////////////////
/// CQualityScoringMethodNA -
/// Quality scoring method for Nucleic Acids (DNA alignments).
/// Use a fixed substition table.
class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CQualityScoringMethodNA : public CQualityScoringMethod
{
public:
    CQualityScoringMethodNA();
    CQualityScoringMethodNA(int colors); // intializes color table
    virtual ~CQualityScoringMethodNA();

    virtual string  GetName() const;
    virtual IUITool* Clone() const;

    virtual int         GetType() const;
    virtual string      GetAllowedSymbols() const;
    virtual TNCBIScore  GetSubstitutionScore(int aa1, int aa2) const;

};


////////////////////////////////////////////////////////////////////////////////
/// CQualityScoringMethodAA -
/// Quality scoring method for Amino Acids (Protein alignments).
/// Use a user specified substition table.
class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CQualityScoringMethodAA : public CQualityScoringMethod
{
public:
    CQualityScoringMethodAA();
    CQualityScoringMethodAA(int colors); // intializes color table
    CQualityScoringMethodAA(const CQualityScoringMethodAA& orig);
    virtual ~CQualityScoringMethodAA();

    virtual string  GetName() const;
    virtual IUITool* Clone() const;

    virtual int         GetType() const;
    virtual TNCBIScore  GetSubstitutionScore(int aa1, int aa2) const;
    virtual string      GetAllowedSymbols() const;

    void                            SetSubstitutionMatrix(const SNCBIPackedScoreMatrix* m);
    const SNCBIPackedScoreMatrix*   GetSubstitutionMatrix();

protected:
    const SNCBIPackedScoreMatrix* m_SubstMatrix;
    SNCBIFullScoreMatrix    m_FullSubstMatrix;
};


////////////////////////////////////////////////////////////////////////////////
/// CConservationScoringMethod - implementation of IScoringMethod.
///
/// Scores are calculated as frequencies of symbols normalized to fit in range
/// [0, 1]. CreateColorTable() must be called once before any GetColorForScore()
/// calls.
class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CConservationScoringMethod
    : public CQualityScoringMethod
{
public:

    friend class CConservationScoringPanel;

    enum EOptions
    {
        eOneBit,
        eTwoBits,
        eThreeBits,
        eFourBits,
        eIdentity
    };

    CConservationScoringMethod();
    CConservationScoringMethod(int colors); // initializes color table
    CConservationScoringMethod(const CConservationScoringMethod& orig);
    virtual ~CConservationScoringMethod();

    /// @name IUITool implementation
    /// @{
    virtual IUITool* Clone() const;
    virtual string  GetName() const;
    virtual string  GetDescription() const;
    /// @}

    /// @name IScoringMethod implementation
    /// @{
    virtual int   GetType() const;
    virtual int   GetSupportedColorTypes() const;
    virtual const CRgbaColor& GetColorForScore(TScore score, EColorType type) const;
    /// @}

    /// @name IColumnScoringMethod implementation
    /// @{
    virtual void    CalculateScores(char cons, const string& column,
                                    TScore& col_score, TScoreVector& scores);
    /// @}

    /// @name CQualityScoringMethod implementation
    /// @{
    virtual TNCBIScore GetSubstitutionScore(int /*aa1*/, int /*aa2*/) const
    {
        return 0;
    }
    virtual string GetAllowedSymbols() const
    {
        return NcbiEmptyString;
    }
    /// @}

    /// @name IUIToolWithGUI implementation
    /// @{
    virtual wxWindow* CreatePropertiesPanel(wxWindow* parent);
    /// @}

    void SetConservedOption(const string& name);

private:
    void x_Init(int colors);

protected:
    bool x_IsConserved(const vector<int>& counts,
                       int num_rows,
                       const double* std_prob) const;

protected:
    EOptions m_ConservedOption = eTwoBits;
    CRgbaColor m_NotScored;
    double* m_StdProb = 0; // standard aa probabilities
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_SCORE___QUALITY_SCORING_METHODS__HPP
