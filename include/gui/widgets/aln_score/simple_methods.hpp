#ifndef GUI_WIDGETS_ALN_SCORE___SIMPLE_METHODS__HPP
#define GUI_WIDGETS_ALN_SCORE___SIMPLE_METHODS__HPP


/*  $Id: simple_methods.hpp 43438 2019-06-28 16:30:56Z katargir $
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
#include <gui/utils/menu_item.hpp>

#include <gui/widgets/wx/ui_tool.hpp>

#include <gui/widgets/aln_score/scoring_method.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CSimpleScoringMethod - trivial implementation of IScoringMethod.
///
/// Scores are calculated as frequencies of symbols normalized to fit in range
/// [0, 1]. CreateColorTable() must be called once before any GetColorForScore()
/// calls.
class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CSimpleScoringMethod
    : public CObject
    , public IScoringMethod
    , public IColumnScoringMethod
    , public IUIToolWithGUI
{
public:
    friend class CSimpleScoringPanel;

    enum    EOptions    {
        fIgnoreEmptySpace   = 0x01,
        fIgnoreGaps         = 0x02
    };

    CSimpleScoringMethod();
    CSimpleScoringMethod(int colors); // initializes color table
    CSimpleScoringMethod(const CSimpleScoringMethod& orig);
    virtual ~CSimpleScoringMethod();

    void    SetOptions(int options); // takes as argument combination of EOptions flags
    int     GetOptions();
    void    CreateColorTable(int size);

    /// @name IUITool implementation
    /// @{
    virtual IUITool* Clone() const;
    virtual string  GetName() const;
    virtual string  GetDescription() const;
    /// @}

    /// @name IScoringMethod implementation
    /// @{
    virtual int   GetType() const;
    virtual bool  IsAverageable() const;
    virtual int   GetSupportedColorTypes() const;
    virtual const CRgbaColor& GetColorForScore(TScore score, EColorType type) const;
    virtual const CRgbaColor& GetColorForNoScore(EColorType type) const;
    virtual void  SetColorForNoScore(EColorType type, const CRgbaColor& color);
    /// @}

    /// @name IColumnScoringMethod implementation
    /// @{
    virtual void    CalculateScores(char cons, const string& column,
                                    TScore& col_score, TScoreVector& scores);

    /// Do we have what it takes to calculate scores? e.g. a master row selected?
    virtual bool    CanCalculateScores(const IScoringAlignment& aln);
    /// @}

    /// @name IUIToolWithGUI implementation
    /// @{
    virtual const CMenuItem* GetMenu();
    virtual bool HasPropertiesPanel() const;
    virtual wxWindow* CreatePropertiesPanel(wxWindow* parent);
    /// @}

private:
    void x_Init(int colors);

protected:
    int m_Options; /// combination of EOptions flags
    CRgbaColor    m_Worst;
    CRgbaColor    m_Best;
    CRgbaGradColorTable   m_ColorTable;

    vector<int>     m_vCharCounts; /// histogramm of characters
    char    m_Space;
    char    m_Gap;

    auto_ptr<CMenuItem> m_Menu;
};

///////////////////////////////////////////////////////////////////////////////
/// CSNPScoringMethod
///
class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CSNPScoringMethod
    : public CSimpleScoringMethod
{
public:
    friend class CSNPScoringPanel;

    CSNPScoringMethod();
    CSNPScoringMethod(const CSNPScoringMethod& orig);

    /// @name IScoringMethod implementation
    /// @{
    virtual IUITool* Clone() const;

    virtual string  GetName() const;
    virtual string  GetDescription() const;
    virtual int     GetSupportedColorTypes() const;

    virtual const CRgbaColor& GetColorForScore(TScore score, EColorType type) const;
    /// @}

    virtual void    CalculateScores(char cons, const string& column,
                                    TScore& col_score, TScoreVector& scores);
    /// Do we have what it takes to calculate scores? e.g. a master row selected?
    virtual bool    CanCalculateScores(const IScoringAlignment& aln);

    /// Call for display colors when CanCalculateScores returns false.
    virtual const CRgbaColor& GetColorForNoScore(EColorType type) const;
    virtual void            SetColorForNoScore(EColorType type, const CRgbaColor& color);

    /// @name IUIToolWithGUI implementation
    /// @{
    virtual const CMenuItem* GetMenu();
    virtual bool HasPropertiesPanel() const;
    virtual wxWindow* CreatePropertiesPanel(wxWindow* parent);
    /// @}
protected:
    CRgbaColor    m_UndefColor;
    bool        m_NoScores;

};


///////////////////////////////////////////////////////////////////////////////
/// CTestSeqBasedMethod
///
class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CTestSeqBasedMethod
    :   public CObject,
        public IScoringMethod,
        public IRowScoringMethod
{
public:
    CTestSeqBasedMethod ();

    virtual IUITool* Clone() const;

    void    CreateColorTable();

    virtual string  GetName() const;
    virtual string  GetDescription() const;

    int             GetType() const;
    virtual bool    IsAverageable() const;
    virtual int     GetSupportedColorTypes() const;

    virtual void    CalculateScores(IAlnExplorer::TNumrow row,
                                    const IScoringAlignment& aln,
                                    TScoreColl& scores);

    virtual const CRgbaColor& GetColorForScore(TScore score, EColorType type) const;

    virtual const CRgbaColor& GetColorForNoScore(IScoringMethod::EColorType type) const;
    virtual void            SetColorForNoScore(IScoringMethod::EColorType type, const CRgbaColor& color);

protected:
    vector<CRgbaColor>    m_vColors;
    CRgbaColor            m_NoScoreColor;
};
////////////////////////////////////////////////////////////////////////////////
/// CNonSynonymousScoringMethod - implementation of IScoringMethod.
///
/// Highlights triplets with non-synonymous mismtaches 
/// Can be used only with 'CDS' alignments - nucleotide alignments of CDS regions 
/// 

class NCBI_GUIWIDGETS_ALNSCORE_EXPORT CNonSynonymousScoringMethod
    : public CObject,
      public IScoringMethod,
      public IColumnScoringMethod
{
public:
    CNonSynonymousScoringMethod();
    CNonSynonymousScoringMethod(const CNonSynonymousScoringMethod& orig);
    virtual ~CNonSynonymousScoringMethod();

    /// @name IUITool implementation
    /// @{
    virtual IUITool* Clone() const override;
    virtual string  GetName() const override;
    virtual string  GetDescription() const override;
    /// @}

    /// @name IScoringMethod implementation
    /// @{
    virtual int   GetType() const override;
    virtual int   GetSupportedColorTypes() const override;
    virtual const CRgbaColor& GetColorForScore(TScore score, EColorType type) const override;
    virtual bool  IsAverageable() const override {
        return true;
    }
    virtual const CRgbaColor& GetColorForNoScore(EColorType type) const override;
    virtual void  SetColorForNoScore(EColorType type, const CRgbaColor& color) override;
    /// @}
    /// @}

    /// @name IColumnScoringMethod implementation
    /// @{
    virtual void    CalculateScores(char cons, const string& column,
        TScore& col_score, TScoreVector& scores) override;

    virtual bool    CanCalculateScores(const IScoringAlignment& aln) override {
        return true;
    }
    virtual bool    NeedsTranslation() const override {
        return true;
    }
    /// @}

    /// @name IUIToolWithGUI implementation
    /// @{
    //    virtual wxWindow* CreatePropertiesPanel(wxWindow* parent) override {
    //  return NULL;
    //    }
    /// @}

    // true if coloration is requested for sequence level scale
    void SetSequenceLevel(bool seq_level = true);

private:
    void x_Init();
    int m_ColumnCount = 0;
    bool m_SequenceLevel = false;
    enum EColors {
        //        eNoScore = 0,
        //        eText,
        eMismatch,
        eMatch,
        eMatchLight,
        eLast
    };
    CRgbaColor m_Colors[EColors::eLast];
    CRgbaGradColorTable   m_ColorTable;
};


inline
void CNonSynonymousScoringMethod::SetSequenceLevel(bool seq_level)
{
    m_SequenceLevel = seq_level;
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_SCORE___SIMPLE_METHODS__HPP
