#ifndef GUI_WIDGETS_SEQ_GRAPHIC___LEGEND_ITEMD_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___LEGEND_ITEMD_GLYPH__HPP

/*  
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
 * Authors:  Andrei shkeda
 *
 * File Description:
 */

#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>

BEGIN_NCBI_SCOPE



class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT ILegendHost
{
public:
    virtual ~ILegendHost() {}
    /// notifies the host about changes of the track.
    virtual void LH_OnItemDblClick(const string& source_name) = 0;
    /// notifies the host about changes of the track.
    virtual void LH_OnItemRightClick(const string& source_name) = 0;

};

///////////////////////////////////////////////////////////////////////////////
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CLegendItemGlyph : public CSeqGlyph
{
public:
    CLegendItemGlyph(const string& source_name, const string& label, const string& id,
                     const CRgbaColor& legend_color, const CRgbaColor& text_color); 

    void SetHost(ILegendHost* host);
    void SetConfig(const CSeqGraphicConfig& config);

    /// @name CSeqGlyph virtual methods.
    virtual bool OnLeftDblClick (const TModelPoint &);
    virtual bool OnContextMenu(wxContextMenuEvent&);
    virtual void GetHTMLActiveAreas	(TAreaVector* p_ares) const;
    virtual bool IsClickable() const;
    ///@}

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();
private:
    TSeqRange m_Range;
    string m_SourceName;
    string m_Label;
    string m_Id;
    CRgbaColor m_LegendColor;
    CRgbaColor m_TextColor;
    CConstRef<CSeqGraphicConfig> m_gConfig;
    ILegendHost* m_LHost;
};

///////////////////////////////////////////////////////////////////////////////
/// CLegendItemGlyph inline methods

inline
void CLegendItemGlyph::SetHost(ILegendHost* host)
{
    m_LHost = host;
}

inline
void CLegendItemGlyph::SetConfig(const CSeqGraphicConfig& config)
{
    m_gConfig.Reset(&config);
}


inline
bool CLegendItemGlyph::IsClickable() const 
{ 
    return true; 
}

inline
void CLegendItemGlyph::x_UpdateBoundingBox()
{
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___LEGEND_ITEMD_GLYPH__HPP
