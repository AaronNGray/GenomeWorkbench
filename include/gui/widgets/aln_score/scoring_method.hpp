#ifndef GUI_WIDGETS_ALN_SCORE___SCORING_METHOD__HPP
#define GUI_WIDGETS_ALN_SCORE___SCORING_METHOD__HPP

/*  $Id: scoring_method.hpp 44842 2020-03-25 17:20:30Z shkeda $
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
 *  File Description: Defines base interfaces and classes for Alignment Scoring Methods.
 *
 */

#include <corelib/ncbiobj.hpp>

#include <objtools/alnmgr/aln_explorer.hpp>

#include <gui/widgets/wx/ui_tool.hpp>

#include <gui/utils/attr_range_coll.hpp>
#include <gui/utils/rgba_color.hpp>


BEGIN_NCBI_SCOPE

class CDialog;

////////////////////////////////////////////////////////////////////////////////
/// IScoringMethod represents an abstract algorithm for calculating alignment
/// scores and assigning colors to residues.
///
/// IScoringMethod has a unique Name that identifies the method and Description
/// that can contain detailed information about the method. Method Type
/// indicates whether it can be applied to DNA or Protein alignments (or both).
/// Method is defined as "Averageable" is scores produced by the method can be
/// calculated for the interval not only for individual residues.
class NCBI_GUIWIDGETS_ALNSCORE_EXPORT IScoringMethod
    : public IUITool
{
public:
    typedef float   TScore;
    typedef vector<TScore>  TScoreVector;
    typedef CAttrRangeCollection<TScore, TSeqPos>   TScoreColl;

    enum    EColorType  {
        fBackground = 0x1,
        fForeground = 0x2
    };

    /// default number of colors in gradient
    static const int    sm_DefGradientSize = 32;

    virtual ~IScoringMethod()   {};

    /// returns combination of EAlignType flags defining types of alignments
    /// that can be colored with this method
    virtual int     GetType() const = 0;

    /// return "true" if scores could be averaged
    virtual bool    IsAverageable() const = 0;

    /// returns a combination of EColorType flags
    virtual int     GetSupportedColorTypes() const = 0;

    /// returns a color corresponding to a given score value. It  is
    /// recommended that this method be implemented using color table,
    /// on-the-fly creation of colors is expensive.
    virtual const CRgbaColor&   GetColorForScore(TScore score,
                                        EColorType type) const = 0;

    /// Call for display colors when CanCalculateScores returns false.
    virtual const CRgbaColor& GetColorForNoScore(IScoringMethod::EColorType type) const = 0;
    virtual void  SetColorForNoScore(IScoringMethod::EColorType type, const CRgbaColor& color) = 0;

    /// returns symbolic label for the given alignment type, should be
    static const char*  GetAlignmentTagByType(IAlnExplorer::EAlignType type);
    static IAlnExplorer::EAlignType     GetAlignmentTypeByTag(const string& tag);
};


////////////////////////////////////////////////////////////////////////////////
///  IScoringAlignment
class   NCBI_GUIWIDGETS_ALNSCORE_EXPORT IScoringAlignment
{
public:
    virtual ~IScoringAlignment() {}
    virtual IAlnExplorer::TNumrow GetNumRows() const = 0;
    virtual IAlnExplorer::EAlignType  GetAlignType() const = 0;
    virtual TSeqPos GetAlnStart() const = 0;
    virtual TSeqPos GetAlnStop() const = 0;
    virtual IAlnExplorer::TNumrow GetAnchor() const = 0;
    virtual string& GetAlnSeqString(IAlnExplorer::TNumrow row, string &buffer,
                                    const IAlnExplorer::TSignedRange& aln_range) const = 0;
    virtual IAlnSegmentIterator* CreateSegmentIterator(IAlnExplorer::TNumrow /*row*/,
                                                       const IAlnExplorer::TSignedRange& /*range*/, IAlnSegmentIterator::EFlags /*flags*/) const = 0;
    virtual TSeqPos GetBaseWidth(IAlnExplorer::TNumrow /*row*/) const = 0;

//    virtual TSignedSeqPos GetSeqPosFromAlnPos(IAlnExplorer::TNumrow /*row*/,
  //                                            TSeqPos seq_pos) const { return seq_pos; }

    virtual TSignedSeqPos  GetSeqPosFromAlnPos(IAlnExplorer::TNumrow for_row,
                                               TSeqPos aln_pos,
                                               IAlnExplorer::ESearchDirection dir = IAlnExplorer::eNone,
                                               bool try_reverse_dir = true) const = 0;

    virtual const objects::CBioseq_Handle& GetBioseqHandle(IAlnExplorer::TNumrow row) const = 0;

    virtual int GetGenCode(IAlnExplorer::TNumrow row) const;

    /// New interfaces to handle alignemnt with mixed type (protein-to-genomic)
    /// To make them backward compatible, they have default implementation.
    virtual bool IsMixedAlign() const
    {
        return GetAlignType() == IAlnExplorer::fMixed;
    }

private:
    mutable unique_ptr<vector<int>> m_GenCodeCache;
};


////////////////////////////////////////////////////////////////////////////////
/// IRowScoringMethod - row-based method
class   NCBI_GUIWIDGETS_ALNSCORE_EXPORT IRowScoringMethod
{
public:
    virtual ~IRowScoringMethod() {}

    virtual void CalculateScores(IAlnExplorer::TNumrow row,
                                 const IScoringAlignment& aln,
                                 IScoringMethod::TScoreColl& scores) = 0;

};


////////////////////////////////////////////////////////////////////////////////
/// IColumnScoringMethod - column-based method
class   NCBI_GUIWIDGETS_ALNSCORE_EXPORT IColumnScoringMethod
{
public:
    static const int kAmbiguousResidueScore = 1000;

    virtual ~IColumnScoringMethod() {}
    /// Do we have what it takes to calculate scores? e.g. a master row selected?
    virtual bool    CanCalculateScores(const IScoringAlignment& aln) = 0;


    virtual void    CalculateScores(char cons, const string& column,
                                    IScoringMethod::TScore& col_score,
                                    IScoringMethod::TScoreVector& scores) = 0;

    virtual bool    NeedsTranslation() const {
        return false;
    }

    /// Set alignment specifi settings
    virtual void    UpdateSettings(const IScoringAlignment& aln);

protected:
    /// Ambiguous Residue, typically 'N' for nucleotides, 'X' for proteins 
    char m_AmbiguousResidue = ' ';

};


class CNcbiRegistry;

////////////////////////////////////////////////////////////////////////////////
/// CTemplateMethod - describes a class of similar methods.
/// Specific information for a particular incarnation is loaded from
/// CNcbiRegistry.
class   NCBI_GUIWIDGETS_ALNSCORE_EXPORT CTemplateScoringMethod
    :   public  CObject,
        public  IScoringMethod,
        public  ITemplateUITool
{
public:
    enum    EColorGradType  {
        eTwoColorGradient,
        eThreeColorGradient
    };

    CTemplateScoringMethod();
    virtual ~CTemplateScoringMethod();

    /// @name IScoringMethod / IUITool implementation
    /// @{
    virtual string  GetName() const;
    virtual string  GetDescription() const;

    virtual int     GetType() const;
    virtual bool    IsAverageable() const;

    virtual int     GetSupportedColorTypes() const;
    /// @}

    /// loads only descriptive information
    virtual bool    LoadInfo(CNcbiRegistry& reg);

    // loads all information
    virtual bool    Load(CNcbiRegistry& reg);

    /// @name Edit interface
    /// @{
    void            SetName(const string& s);
    void            SetDescription(const string& s);

    /// saves only descriptive information
    virtual bool    SaveInfo(CNcbiRegistry& reg);

    // saves all information
    virtual bool    Save(CNcbiRegistry& reg);
    /// @}

protected:
    CRgbaColor    x_GetColor(CNcbiRegistry& reg,
                           const string& key, const string def_value);

protected:
    string  m_Name;
    string  m_Descr;
    int     m_Type; // combination of EAlignType flags
    bool    m_Averageable;

    EColorGradType  m_ColorGradType;
};


////////////////////////////////////////////////////////////////////////////////
/// IWindowScoringMethod - Method that can be windowed,
/// i.e. averaged over a sliding window.
class   NCBI_GUIWIDGETS_ALNSCORE_EXPORT IWindowScoringMethod
{
public:
    virtual ~IWindowScoringMethod() {}
    virtual size_t  GetWindowSize() const = 0;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_SCORE___SCORING_METHOD__HPP
