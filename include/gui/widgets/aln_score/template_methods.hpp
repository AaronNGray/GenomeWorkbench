#ifndef GUI_WIDGETS_ALN_SCORE___TEMPLATE_METHODS__HPP
#define GUI_WIDGETS_ALN_SCORE___TEMPLATE_METHODS__HPP

/*  $Id: template_methods.hpp 36328 2016-09-13 22:19:33Z evgeniev $
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
 */

#include <util/math/matrix.hpp>
#include <gui/opengl/glcolortable.hpp>

#include <gui/widgets/wx/ui_tool.hpp>
#include <gui/widgets/aln_score/scoring_method.hpp>


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CColorTableMethod
class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CColorTableMethod
    : public CTemplateScoringMethod,
      public IRowScoringMethod,
      public IUIToolWithGUI
{
public:
    CColorTableMethod();
    virtual ~CColorTableMethod();

    virtual const CRgbaColor& GetColorForNoScore(EColorType type) const;
    virtual void SetColorForNoScore(IScoringMethod::EColorType type, const CRgbaColor& color);

    /// @name ITemplateScoringMethod implementation
    /// @{
    virtual IScoringMethod* Clone() const;

    virtual bool    Load(CNcbiRegistry& reg);

    virtual int     GetSupportedColorTypes() const;

    virtual const CRgbaColor& GetColorForScore(TScore score, EColorType type) const;
    /// @}

    /// @name IRowScoringMethod implementation
    /// @{
    virtual void    CalculateScores(IAlnExplorer::TNumrow row,
                                    const IScoringAlignment& aln_vec,
                                    TScoreColl& scores);
    /// @}

    // @name IUIToolWithGUI implementation
    /// @{
    virtual const CMenuItem*    GetMenu();
    virtual bool        HasPropertiesPanel() const;
    virtual wxWindow*   CreatePropertiesPanel(wxWindow* parent);
    /// @}

    /// @name Edit interface
    /// @{
    void            Clear();
    void            SetColor(char cbase, const CRgbaColor& color, EColorType type = fForeground);
    CRgbaColor        GetColor(char cbase, EColorType type) const;

    /// Which bases have colors specified?
    const string&   GetSetBases() const;
    void            AddSetBase(char cbase);

    /// Opposite of Load().
    bool            Save(CNcbiRegistry& reg);

    /// @}

protected:
    void    x_CreateColorTable(CNcbiRegistry& reg);

protected:
    vector<CRgbaColor>    m_Colors;       ///> background colors
    vector<CRgbaColor>    m_ForeColors;   ///> foreground or text colors
    CRgbaColor            m_DefaultBackColor; ///> Background color when nothing specified for a base.
    CRgbaColor            m_DefaultForeColor; ///> Foreground color when nothing specified for a base.
    string              m_SetBases;     ///> the bases (characters) we have colors for. unordered.
};

////////////////////////////////////////////////////////////////////////////////
/// CColumnScoringMethod
class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CColumnScoringMethod
    :   public CTemplateScoringMethod,
        public IColumnScoringMethod,
//        public IRowScoringMethod,
        public IUIToolWithGUI,
        public IWindowScoringMethod
{
public:
    friend class CColumnScoringPanel;

    CColumnScoringMethod();
    virtual ~CColumnScoringMethod();

    virtual void    CreateColorTable(int size);

    /// @name IScoringMethod implementation
    /// @{
    virtual IScoringMethod* Clone() const;

    virtual bool    Load(CNcbiRegistry& reg);

    virtual int     GetSupportedColorTypes() const;

//    virtual void    CalculateScores(TNumrow row, const objects::CAlnVec& aln_vec,
//                            TScoreColl& scores);

    virtual const CRgbaColor& GetColorForScore(TScore score, EColorType type) const;
    /// @}

    /// @name IColumnScoringMethod implementation
    /// @{
    virtual void    CalculateScores(char cons, const string& column,
                             TScore& col_score, TScoreVector& scores);

    bool            CanCalculateScores(const IScoringAlignment& aln);
    const CRgbaColor& GetColorForNoScore(EColorType type) const;
    void            SetColorForNoScore(EColorType type, const CRgbaColor& color);
    /// @}

    /// @name IWindowScoringMethod implementation
    /// @{
    size_t          GetWindowSize() const;
    /// @}

    // @name IUIToolWithGUI implementation
    /// @{
    virtual const CMenuItem*    GetMenu();
    virtual bool        HasPropertiesPanel() const;
    virtual wxWindow*   CreatePropertiesPanel(wxWindow* parent);
    /// @}

protected:
    void        x_CreateScoreTable(CNcbiRegistry& reg);
    CRgbaColor    x_GetThreeColorScore(TScore score) const;
    CRgbaColor    x_GetTwoColorScore(TScore score) const;
    void        x_CalculateConsensusScores(char cons,  const string& column,
                                           TScore& col_score, TScoreVector& scores);
    void        x_CalculateNonConsensusScores(const string& column, TScore& col_score,
                                              TScoreVector& scores);

protected:
    vector<TScore>  m_vScore; /// weight matrix
    TScore      m_MinScore; /// possible range of scores.
    TScore      m_MaxScore;
    TScore      m_MidScore;

    CRgbaColor    m_MinColor;
    CRgbaColor    m_MidColor;
    CRgbaColor    m_MaxColor;

    CRgbaGradColorTable   m_ColorTable;

    bool        m_UseConsensus; /// relative, not absolute scores.
    size_t      m_WindowSize;   /// if > 1, use running average.
};

////////////////////////////////////////////////////////////////////////////////
/// CMatrixScoringMethod

class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CMatrixScoringMethod
    :   public CTemplateScoringMethod,
        public IColumnScoringMethod,
        public IUIToolWithGUI,
        public IWindowScoringMethod
{
    friend class CMatrixScoringPanel;

public:
    CMatrixScoringMethod();
    virtual ~CMatrixScoringMethod();

    virtual void    CreateColorTable(int size);

    /// @name IScoringMethod implementation
    /// @{
    virtual IScoringMethod* Clone() const;

    virtual bool    Load(CNcbiRegistry& reg);

    virtual int     GetSupportedColorTypes() const;

    virtual const CRgbaColor& GetColorForScore(TScore score, EColorType type) const;
    /// @}

    /// @name IColumnScoringMethod implementation
    /// @{
    virtual void    CalculateScores(char cons, const string& column,
                             TScore& col_score, TScoreVector& scores);
    bool            CanCalculateScores(const IScoringAlignment& aln);
    const CRgbaColor& GetColorForNoScore(EColorType type) const;
    void            SetColorForNoScore(EColorType type, const CRgbaColor& color);

    /// @}

    /// @name IWindowScoringMethod implementation
    /// @{
    size_t          GetWindowSize() const;
    /// @}

    // @name IUIToolWithGUI implementation
    /// @{
    virtual const CMenuItem*    GetMenu();
    virtual bool        HasPropertiesPanel() const;
    virtual wxWindow*   CreatePropertiesPanel(wxWindow* parent);
    /// @}
protected:
    void        x_CreateScoreTable(CNcbiRegistry& reg);
    void        x_CalculateConsensusRange();
    void        x_CalculateNonConsensusRange();
    static int  x_BaseToIndex(char c);

protected:
    enum TRangeType
    {
        eNone           = 0,
        eConsensus      = 1,
        eNonConsesus    = 2,
        eBoth           = 3
    };
    unsigned char  m_CalculateRange;

    CNcbiMatrix<TScore> m_vScore;

    TScore      m_MinScore;
    TScore      m_MaxScore;
    TScore      m_ConsensusMinScore;
    TScore      m_ConsensusMaxScore;
    TScore      m_NonConsensusMinScore;
    TScore      m_NonConsensusMaxScore;

    CRgbaColor    m_MinColor;
    CRgbaColor    m_MidColor; //?
    CRgbaColor    m_MaxColor;
    CRgbaColor    m_UndefColor;

    bool        m_UseConsensus;
  
    CRgbaGradColorTable   m_ColorTable;
    size_t      m_WindowSize;
  };

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_SCORE___TEMPLATE_METHODS__HPP
