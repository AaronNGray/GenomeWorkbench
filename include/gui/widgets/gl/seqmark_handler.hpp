#ifndef __GUI_WIDGETS_GL___SEQKMARK_HANDLER__HPP
#define __GUI_WIDGETS_GL___SEQKMARK_HANDLER__HPP

/*  $Id: seqmark_handler.hpp 43798 2019-08-30 20:27:34Z katargir $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/widgets/gl/ievent_handler.hpp>


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class ISeqMarkHandlerHost

class ISeqMarkHandlerHost
{
public:
    virtual ~ISeqMarkHandlerHost() { }

    virtual TModelUnit  SMHH_GetModelByWindow(int z, EOrientation orient) = 0;

    /// If the host needs to dictate/store the marker position,
    /// then it needs to return a non (TSeqPos)-1 value. Otherwise,
    /// the internal marker position will be used.
    virtual TSeqPos SMHH_GetSeqMark() const = 0;

    /// The derived class may choose do something differently when
    /// marker position is changed.
    virtual void    SMHH_SetSeqMark(TSeqPos mark) = 0;

    /// The derived class may need to do something differently when
    /// a marker gets reset, such as remove the marker.
    virtual void    SMHH_OnReset(const string& marker_id) = 0;
};


////////////////////////////////////////////////////////////////////////////////
/// Class CSeqMarkHandler represents in GUI a single "hairpin" on a sequence bar.
///
/// CSeqMarkHandler implements IGlEventHandler what makes it compatible with
/// IGlEventHandler-supproting widgets.
class NCBI_GUIWIDGETS_GL_EXPORT CSeqMarkHandler :
    public wxEvtHandler,
    public IGlEventHandler
{
    DECLARE_EVENT_TABLE()
public:
    enum ERenderingOption
    {
        eActiveState,
        ePassiveState
    };

    enum EMarkerType
    {
        ePoint,
        eRange
    };

    CSeqMarkHandler();
    virtual ~CSeqMarkHandler();

    void    SetHost(ISeqMarkHandlerHost* pHost);
    IGenericHandlerHost*    GetGenericHost();
    void    SetMarkerType(EMarkerType t);
    EMarkerType GetMarkerType() const { return m_MarkerType; }
    void    Render(CGlPane& Pane, ERenderingOption option);
    void  SetLabel(const string& label);
    const string& GetLabel() const;
    void  SetId(const string& id);
    const string& GetId() const;
    void  SetPos(TModelUnit pos);
    TSeqPos  GetPos() const;
    void  SetExtendedPos(TModelUnit pos);
    TSeqPos  GetExtendedPos() const;
    // updates both pos and extended pos
    void  SetRange(TSeqRange range);
    
    /// Return positions based on whether sequence is flipped
    TModelUnit  GetPosLeft() const;
    TModelUnit  GetPosRight() const;

    void  SetColor(const CRgbaColor& color);
    const CRgbaColor& GetColor() const;
    bool  HitMe() const;
    void  SetShowCoord(bool f);
    void  SetFlipped(bool flipped);
    bool  GetFlipped() const { return m_Flipped; }

    /// @name IGlEventHandler implementaion
    /// @{
    virtual void    SetPane(CGlPane* pane);
    virtual wxEvtHandler*    GetEvtHandler();
    /// @}

    void    OnLeftDown(wxMouseEvent& event);
    void    OnLeftDoubleClick(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& evt);

protected:
    TModelRect x_GetMarkRect(CGlPane& pane) const;
    TSeqPos    x_SeqDragPos() const;
    void       x_DrawMarker(TModelUnit x_c, TModelUnit y_c, int half) const;
    void       x_DrawLabel(TModelUnit x, TModelUnit y, int half) const;
    virtual string x_GetText(TSeqPos pos);
private:
    void        x_OnEndAction();

private:
    CGlPane* m_Pane;
    /// label shown beside the marker (optional).
    string   m_Label;
    /// unique id for manager multiple markers (optional).
    string   m_Id; 
    ISeqMarkHandlerHost* m_Host;

    EMarkerType m_MarkerType; // point or range

    /// For draging the mark
    bool       m_MarkDrag;

    /// show coordinate
    bool       m_ShowCoord;

    /// marker position
    TModelUnit m_DragPos;
    /// Second position, for range markers
    TModelUnit m_ExtendedPos;
    /// offset between mouse position and marker position
    TModelUnit m_PosOff;

    // font for sequence position (while dragging)
    CGlTextureFont m_Font_Helv10;
    CGlTextureFont m_LabelFont;
    CRgbaColor    m_Color;
    bool          m_HitMe;

    /// True if strands are flipped 
    bool m_Flipped;
};


// inline method implementation
inline
void CSeqMarkHandler::SetLabel(const string& label)
{ m_Label = label; }

inline
const string& CSeqMarkHandler::GetLabel() const
{ return m_Label; }

inline
void CSeqMarkHandler::SetId(const string& id)
{ m_Id = id; }

inline
const string& CSeqMarkHandler::GetId() const
{ return m_Id; }

inline
TSeqPos CSeqMarkHandler::GetPos() const
{ return x_SeqDragPos(); }

inline
TSeqPos CSeqMarkHandler::GetExtendedPos() const
{ return m_ExtendedPos; }

inline
void CSeqMarkHandler::SetColor(const CRgbaColor& color)
{ m_Color = color; }

inline
const CRgbaColor& CSeqMarkHandler::GetColor() const
{ return m_Color; }

inline
bool CSeqMarkHandler::HitMe() const
{ return m_HitMe; }

inline
void CSeqMarkHandler::SetShowCoord(bool f)
{ m_ShowCoord = f; }

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_GL___SEQKMARK_HANDLER__HPP
