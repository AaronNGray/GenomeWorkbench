/*  $Id: attrib_menu.cpp 43798 2019-08-30 20:27:34Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/opengl.h>
#include <gui/opengl/glutils.hpp>

#include <gui/widgets/gl/attrib_menu.hpp>

#include <corelib/ncbistr.hpp>

#include <float.h>
#include <limits.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAttribMenuInstance
/// Returns a global static instance of the menu object so that it can be
/// used from different classes.
CAttribMenu& CAttribMenuInstance::GetInstance()
{
    static CAttribMenu instance;
    return instance;
}


///////////////////////////////////////////////////////////////////////////////
/// CAttribMenuItem
CVect2<float> CAttribMenuItem::Draw(CVect2<float> pos)
{
    CGlBitmapFont font(m_FontType);
    std::string text = GetText();

    float width = (float)font.TextWidth(text.c_str());
    float height = (float)font.TextHeight(); 

    text = m_Name + ": ";
        
    glColor3fv(m_Color.GetData());
    font.TextOut(pos.X(), pos.Y(), text.c_str());

    // To change the color for the data part, we call TextOut(x,y,char*)
    // instead of TextOut(char*) since the former calls glRasterPos which
    // is needed to update the color for character output.
    float rp[4];
    glGetFloatv(GL_CURRENT_RASTER_POSITION, rp);
    glColor3fv(m_DataColor.GetData());
    font.TextOut(rp[0], rp[1], GetTextValue().c_str());
    
    return CVect2<float>(width, height);
}

CVect2<float> CAttribMenuItem::GetTextDim() const
{
    CGlBitmapFont font(m_FontType);
    std::string text = GetText();

    float width = (float)font.TextWidth(text.c_str());
    float height = (float)font.TextHeight();
        
    return CVect2<float>(width, height);
}

///////////////////////////////////////////////////////////////////////////////
/// CAttribIntMenuItem
std::string CAttribIntMenuItem::GetText() const 
{
    /// Return the full text for the menu item.
    char buf[256];
    sprintf(buf, "%s: %d", m_Name.c_str(), m_Value);

    return std::string(buf);
}

std::string CAttribIntMenuItem::GetTextValue() const 
{
    /// Return the value part of the text for the menu item
    char buf[256];
    sprintf(buf, "%d", m_Value);

    return std::string(buf);
}


void CAttribIntMenuItem::SetRange(int from, int to) 
{ 
    m_MinValue = from; 
    m_MaxValue = to; 

    if (m_Value < m_MinValue || m_Value > m_MaxValue)
        m_Value = (m_MaxValue + m_MinValue)/2;
}

void CAttribIntMenuItem::Inc()
{
    if (m_ReadOnly)
        return;

    if (m_Value < m_MaxValue) {
        m_Value = std::min(m_MaxValue, m_Value + m_StepSize);
    }
    else {
        m_Value = m_MinValue;
    }

    if (m_UserValue != NULL)
        *m_UserValue = m_Value;
}

void CAttribIntMenuItem::Dec()
{
    if (m_ReadOnly)
        return;

    if (m_Value > m_MinValue) {
        m_Value = std::max(m_MinValue, m_Value - m_StepSize);
    }
    else {
        m_Value = m_MaxValue;
    }

    if (m_UserValue != NULL)
        *m_UserValue = m_Value;
}

///////////////////////////////////////////////////////////////////////////////
/// CAttribFloatMenuItem
std::string CAttribFloatMenuItem::GetText() const
{
    char buf[256];
    sprintf(buf, "%s: %.4f", m_Name.c_str(), m_Value);

    return std::string(buf);
}

std::string CAttribFloatMenuItem::GetTextValue() const
{
    char buf[256];
    sprintf(buf, "%.4f", m_Value);

    return std::string(buf);
}

void CAttribFloatMenuItem::SetRange(float from, float to) 
{ 
    m_MinValue = from; 
    m_MaxValue = to; 

    if (m_Value < m_MinValue || m_Value > m_MaxValue)
        m_Value = (m_MaxValue + m_MinValue)/2.0f;
}


void CAttribFloatMenuItem::Inc()
{
    if (m_ReadOnly)
        return;

    if (m_Value < m_MaxValue) {
        m_Value = std::min(m_MaxValue, m_Value + m_StepSize);
    }
    else {
        m_Value = m_MinValue;
    }

    if (m_UserValue != NULL)
        *m_UserValue = m_Value;
}

void CAttribFloatMenuItem::Dec()
{
    if (m_ReadOnly)
        return;

    if (m_Value > m_MinValue) {
        m_Value = std::max(m_MinValue, m_Value - m_StepSize);
    }
    else {
        m_Value = m_MaxValue;
    }

    if (m_UserValue != NULL)
        *m_UserValue = m_Value;
}

///////////////////////////////////////////////////////////////////////////////
/// CAttribStringsMenuItem
std::string CAttribStringsMenuItem::GetText() const
{
    std::string text = m_Name + " : ";

    if ( m_CurrentIdx < m_Value.size() )
        text += m_Value[m_CurrentIdx];

    return text;
}

std::string CAttribStringsMenuItem::GetTextValue() const
{
    std::string text;

    if ( m_CurrentIdx < m_Value.size() )
        text += m_Value[m_CurrentIdx];

    return text;
}


void CAttribStringsMenuItem::Inc()
{
    if (m_ReadOnly)
        return;

    if (m_CurrentIdx < m_Value.size()-1)
        ++m_CurrentIdx;
    else
        m_CurrentIdx = 0;

    if (m_UserValue != NULL)
        *m_UserValue = m_Value[m_CurrentIdx];
}

void CAttribStringsMenuItem::Dec()
{
    if (m_ReadOnly)
        return;

    if (m_CurrentIdx > 0)
        --m_CurrentIdx;
    else
        m_CurrentIdx = m_Value.size()-1;

    if (m_UserValue != NULL)
        *m_UserValue = m_Value[m_CurrentIdx];
}

void CAttribStringsMenuItem::SynchToUserValue()
{
    if (m_ReadOnly) {
        m_Value[0] = *m_UserValue;
        m_CurrentIdx = 0;
        return;
    }

    if (m_UserValue==NULL)
        return;

    for (size_t i=0; i<m_Value.size(); ++i) {
        if (m_Value[i] == *m_UserValue) {
            m_CurrentIdx = i;
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CAttribMenu
CAttribMenu::~CAttribMenu()
{
    size_t i;

    for (i=0; i<m_Value.size(); ++i) {
        delete m_Value[i];
    }

    m_Value.clear();
}

void CAttribMenu::DrawMenu()
{
    GLint viewport[4];				
    glGetIntegerv(GL_VIEWPORT, viewport);

    // otho view where 1.0 unit == 1 pixel:
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, viewport[2], 0.0, viewport[3], -1.0, 1.0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // Draw in upper left.  This could become user-controlled if desired.
    CVect2<float> pos(0.0f, (float)(viewport[3]-10));
    x_DrawMenu(pos);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

void CAttribMenu::x_DrawMenu(CVect2<float> pos)
{
    x_UpdateReadOnly();

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Draw open menus only:
    if (m_IsOpen) {
        CGlBitmapFont font(m_FontType);

        float width = 0.0f;
        float height = 0.0f;

        CVect2<float> quad_pos(pos);
        CVect2<float> dim;
        CVect2<float> dim_current;

        // Menu border size
        float border_space = 6.0f;

        // iterate over the menu once in advance to calculate
        // its width and height
        size_t i;
        for (i=0; i<m_Value.size(); ++i) {
            CVect2<float> dim = m_Value[i]->GetTextDim();

            // Only 1 item on a menu can be current at a time:
            if (m_CurrentItem == i)
                dim_current = dim;

            // pos is base (lower left) position for text - quad is higher
            if ( i==0 )
                quad_pos.Y() += dim.Y();

            width = std::max(width, dim.X());
            height += dim.Y() + 1;
        }
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /// draw the background color for the menu:
        glColor4fv(m_BackgroundColor.GetData());
        glBegin(GL_QUADS);        
            glVertex2f(quad_pos.X(), 
                       quad_pos.Y() + border_space);
            glVertex2f(quad_pos.X(), 
                       quad_pos.Y() - height - border_space*3.0f);
            glVertex2f(quad_pos.X() + width + border_space*2.0f, 
                       quad_pos.Y() - height - border_space*3.0f);
            glVertex2f(quad_pos.X() + width + border_space*2.0f, 
                       quad_pos.Y() + border_space);
        glEnd();

        // draw the background border for the menu
        glColor4f(0.0f, 0.0f, 0.0f, m_BackgroundColor[3]);
        glBegin(GL_LINE_LOOP);
            glVertex2f(quad_pos.X(), 
                       quad_pos.Y() + border_space);
            glVertex2f(quad_pos.X(), 
                       quad_pos.Y() - height - border_space*3.0f);
            glVertex2f(quad_pos.X() + width + border_space*2.0f, 
                       quad_pos.Y() - height - border_space*3.0f);
            glVertex2f(quad_pos.X() + width + border_space*2.0f, 
                       quad_pos.Y() + border_space);
        glEnd();

        glDisable(GL_BLEND);

        pos.X() += border_space;

        // Iterate over the menu items and draw each one
        for (i=0; i<m_Value.size(); ++i) {
            
            // If this is the active menu item, draw a highlight quad:
            if (m_CurrentItem == i) {                
                glColor3fv( (m_BackgroundColor*0.75f).GetData() );
                glBegin(GL_QUADS);
                    glVertex2f(quad_pos.X() + border_space/2, 
                               pos.Y() + dim_current.Y());
                    glVertex2f(quad_pos.X() + border_space/2, 
                               pos.Y());
                    glVertex2f(quad_pos.X() + width + border_space, 
                               pos.Y());
                    glVertex2f(quad_pos.X() + width + border_space, 
                               pos.Y() + dim_current.Y());
                glEnd();
            }

            // Draw the menu item text:
            dim = m_Value[i]->Draw(pos);

            // If this is a submenu, and it is open, draw it immediately to the
            // right of the current menu item:
            if (m_Value[i]->GetType() == AttribMenuType) {
                CAttribMenu* m = dynamic_cast<CAttribMenu*>(m_Value[i]);
                if (m != NULL && m->IsOpen()) {
                    CVect2<float> menu_pos(pos.X() + width + border_space, 
                                           pos.Y() + border_space - dim.Y());
                    m->x_DrawMenu(menu_pos);
                }
            }

            pos.Y() -= dim.Y() + 2;
        }
    }

    glPopAttrib();
}

void CAttribMenu::SetKeys(int key_up, int key_down, int increment, int decrement)
{
    m_KeyUp = key_up;
    m_KeyDown = key_down;
    m_KeyInc = increment;
    m_KeyDec = decrement;

    // propogate to submenus:
    std::vector<CAttribMenuItem*>::iterator iter;

    for (iter=m_Value.begin(); iter!=m_Value.end(); ++iter) {
        if ( (*iter)->GetType() == AttribMenuType ) {
            CAttribMenu* m = dynamic_cast<CAttribMenu*>(*iter);
            m->SetKeys(m_KeyUp, m_KeyDown, m_KeyInc, m_KeyDec);
        }
    }
}

void CAttribMenu::SetOpenCloseKeys(int open_key, int close_key)
{
    m_OpenKey = open_key;
    m_CloseKey = close_key;

    // propogate to submenus:
    std::vector<CAttribMenuItem*>::iterator iter;

    for (iter=m_Value.begin(); iter!=m_Value.end(); ++iter) {
        if ( (*iter)->GetType() == AttribMenuType ) {
            CAttribMenu* m = dynamic_cast<CAttribMenu*>(*iter);
            m->SetOpenCloseKeys(m_OpenKey, m_CloseKey);
        }
    }
}

bool CAttribMenu::x_HandleKey(int key_code)
{
    bool handled = true;

    // (388 is code for '+' on numpad)
    if (key_code == m_OpenKey || key_code == 388) {
        // This case is only for the top level menu (normally this 
        // function would not be called for a closed menu).
        if (!IsOpen()) {
            m_IsOpen = true;          
        }
        // If the cursor is on a closed menu, open it:
        else if ((m_CurrentItem < m_Value.size()) &&
                 (m_Value[m_CurrentItem]->GetType() == AttribMenuType)) {
            CAttribMenu* m = dynamic_cast<CAttribMenu*>(m_Value[m_CurrentItem]);
            if (!m->IsOpen()) {
                m->m_IsOpen = true;
            }
        }
    }
    // Close menu on m_CloseKey key or numpad '-' key
    else if (key_code == m_CloseKey || key_code == 390) {
        if (IsOpen()) {
            m_IsOpen = false;
        }
    }
    // Advance downward (if available) on numpad down-arrow key (w/numlock)
    else if (key_code == m_KeyDown) {
        if (m_CurrentItem < m_Value.size()-1) {
            ++m_CurrentItem;
        }
    }
    // Go upward (if available) on numpad down-arrow key (w/numlock)
    else if (key_code == m_KeyUp) {
        if (m_CurrentItem > 0) {
            --m_CurrentItem;
        }
    }
    // Go to next value on numpad right-arrow key (w/numlock)
    else if (key_code == m_KeyInc) {
        if (m_CurrentItem < m_Value.size()) {
            m_Value[m_CurrentItem]->Inc();
        }
    }
    // Go to previous value on numpad left-arrow key (w/numlock)
    else if (key_code == m_KeyDec) {
        m_Value[m_CurrentItem]->Dec();
    }
    else {
        handled = false;
    }

    return handled;
}
bool CAttribMenu::KeyPress(int key_code)
{
    bool handled = false;

    // Find active menu and call x_HandleKey on that (sub)menu
    if (m_IsOpen) {
        if ((m_CurrentItem < m_Value.size()) &&
            (m_Value[m_CurrentItem]->GetType() == AttribMenuType)) {
            CAttribMenu* m = dynamic_cast<CAttribMenu*>(m_Value[m_CurrentItem]);

            if (m != NULL && m->IsOpen()) {
                handled = m->KeyPress(key_code);
                return handled;
            }
        }
    }

    // This is the current menu - handle the key press
    handled = x_HandleKey(key_code);

    return handled;
}

bool CAttribMenu::AddItem(CAttribMenuItem* a)
{
    if (FindItem(a->GetName()) == NULL) {
        m_Value.push_back(a);
        return true;
    }
    
    return false;
}

CAttribMenu* CAttribMenu::AddSubMenuUnique(const std::string& name, void* user_value)
{
    std::string n = name;

    // Search for item in current menu.  Each time it is found, add
    // an incremented number to the end of the name. Repeat until not found
    // (name is unique in menu).
    int v = 0;

    while (FindItem(n) != NULL) {
        char buf[256];
        sprintf(buf, "%s %d", name.c_str(), ++v);
        n = buf;
    }

    // Add this unique name to the submenu.
    return AddSubMenu(n, user_value);
}

CAttribMenu* CAttribMenu::AddSubMenu(const std::string& name, void* user_value)
{    
    CAttribMenu* m = new CAttribMenu(name, user_value);
    
    if (!AddItem(m)) {
        delete m;
        return NULL;
    }

    // Make sure keys for submenu match this one:
    m->SetKeys(m_KeyUp, m_KeyDown, m_KeyInc, m_KeyDec);
    m->SetOpenCloseKeys(m_OpenKey, m_CloseKey);

    return m;    
}

CAttribFloatMenuItem* CAttribMenu::AddFloat(const std::string& name,
                                            float* user_value,
                                            float initial_value,
                                            float min_value,
                                            float max_value,
                                            float stepsize)
{
    CAttribFloatMenuItem* f = new CAttribFloatMenuItem(name, user_value);
    f->SetRange(min_value, max_value);
    f->SetValue(initial_value);
    f->SetStepSize(stepsize);

    if (!AddItem(f)) {
        delete f;
        return NULL;
    }

    return f;
}

CAttribIntMenuItem* CAttribMenu::AddInt(const std::string& name,
                                        int* user_value,
                                        int initial_value,
                                        int min_value,
                                        int max_value,
                                        int stepsize)
{
    CAttribIntMenuItem* i = new CAttribIntMenuItem(name, user_value);

    i->SetRange(min_value, max_value);
    i->SetValue(initial_value);
    i->SetStepSize(stepsize);

    if (!AddItem(i)) {
        delete i;
        return NULL;
    }

    return i;
}

CAttribStringsMenuItem* CAttribMenu::AddStrings(const std::string& name,
                                                std::string* user_value)
{
    CAttribStringsMenuItem* s = new CAttribStringsMenuItem(name, user_value);

    if (!AddItem(s)) {
        delete s;
        return NULL;
    }

    return s;
}

CAttribFloatMenuItem* CAttribMenu::AddFloatReadOnly(const std::string& name,
                                                    float* user_value)
{
    _ASSERT(user_value != NULL);

    CAttribFloatMenuItem* f = new CAttribFloatMenuItem(name, user_value);
    f->SetReadOnly(true);
    f->SetRange(FLT_MIN, FLT_MAX);
    f->SetValue(*user_value);
    f->SetStepSize(0.0f);

    if (!AddItem(f)) {
        delete f;
        return NULL;
    }

    return f;
}

CAttribIntMenuItem* CAttribMenu::AddIntReadOnly(const std::string& name,
                                                int* user_value)
{
    _ASSERT(user_value != NULL);

    CAttribIntMenuItem* i = new CAttribIntMenuItem(name, user_value);
    i->SetReadOnly(true);
    i->SetRange(INT_MIN, INT_MAX);
    i->SetValue(*user_value);
    i->SetStepSize(0);

    if (!AddItem(i)) {
        delete i;
        return NULL;
    }

    return i;
}

CAttribStringsMenuItem* CAttribMenu::AddStringsReadOnly(const std::string& name, 
                                                        std::string* user_value)
{
    _ASSERT(user_value != NULL);

    CAttribStringsMenuItem* s = new CAttribStringsMenuItem(name, user_value);
    s->SetReadOnly(true);
    s->AddString(*user_value);

    if (!AddItem(s)) {
        delete s;
        return NULL;
    }

    return s;
}

CAttribMenuItem *CAttribMenu::FindItem(const std::string& name)
{
    std::string n1 = name;
    
    NStr::ToLower(n1);

    size_t i;
    for (i=0; i<m_Value.size(); ++i) {
        std::string n2 = m_Value[i]->GetName();

        if (n1 == NStr::ToLower(n2))
            return m_Value[i];        
    }

    return NULL;
}
 
CAttribMenu *CAttribMenu::FindMenu(const std::string& name)
{
    CAttribMenuItem* item = FindItem(name);

    if (item != NULL)
        return dynamic_cast<CAttribMenu*>(item);

    return NULL;
}

CAttribIntMenuItem *CAttribMenu::FindInt(const std::string& name)
{
    CAttribMenuItem* item = FindItem(name);

    if (item != NULL)
        return dynamic_cast<CAttribIntMenuItem*>(item);
   
    return NULL;
}

CAttribFloatMenuItem *CAttribMenu::FindFloat(const std::string& name)
{
    CAttribMenuItem* item = FindItem(name);

    if (item != NULL)
        return dynamic_cast<CAttribFloatMenuItem*>(item);

    return NULL;   
}

CAttribStringsMenuItem *CAttribMenu::FindStrings(const std::string& name)
{
    CAttribMenuItem* item = FindItem(name);

    if (item != NULL)
        return dynamic_cast<CAttribStringsMenuItem*>(item);

    return NULL;
}

bool CAttribMenu::RemoveItem(const std::string& name)
{
    std::string n1 = name;
    
    NStr::ToLower(n1);

    std::vector<CAttribMenuItem*>::iterator iter;

    for (iter=m_Value.begin(); iter!=m_Value.end(); ++iter) {
        std::string n2 = (*iter)->GetName();

        if (n1 == NStr::ToLower(n2)) {
            delete *iter;  
            m_Value.erase(iter);            
            return true;
        }
    }

    return false;
}

bool CAttribMenu::RemoveMenuR(const std::string& name, 
                              void* user_value)
{
    std::string n1 = name;
    
    NStr::ToLower(n1);

    std::vector<CAttribMenuItem*>::iterator iter;

    // For each menu item:
    for (iter=m_Value.begin(); iter!=m_Value.end(); ++iter) {

        // If it is a submenu:
        if ( (*iter)->GetType() == AttribMenuType) {
            CAttribMenu* m = dynamic_cast<CAttribMenu*>(*iter);

            std::string n2 = m->GetName();
            NStr::ToLower(n2);

            // If the submenu has name 'name' and user_value matches menus
            // user_value or user_value is NULL:
            if (user_value != NULL) {
                if (n2.length() >= n1.length() && 
                  n2.substr(0, n1.length()) == n1 &&
                  user_value == m->m_UserValue) {
                    // delete the menu
                    delete m;
                    m_Value.erase(iter);            
                    return true;
                }
            }
            else if (n1 == n2) {
                // delete the menu
                delete m;
                m_Value.erase(iter);            
                return true;
            }

            // Otherwise search submenus recursively:
            if (m->RemoveMenuR(name, user_value) == true) {
                return true;
            }
        }
    }

    return false;
}

void CAttribMenu::Inc()
{
    if (!IsOpen())
        m_IsOpen = true;
}

void CAttribMenu::SynchToUserValue()
{
    std::vector<CAttribMenuItem*>::iterator iter;

    for (iter=m_Value.begin(); iter!=m_Value.end(); ++iter) {
        (*iter)->SynchToUserValue();
    }
}

void CAttribMenu::x_UpdateReadOnly()
{
    std::vector<CAttribMenuItem*>::iterator iter;

    for (iter=m_Value.begin(); iter!=m_Value.end(); ++iter) {
        if ( (*iter)->GetReadOnly() == true )
            (*iter)->SynchToUserValue();

        if ( (*iter)->GetType() == AttribMenuType ) {
            CAttribMenu* m = dynamic_cast<CAttribMenu*>(*iter);
            m->x_UpdateReadOnly();
        }
    }
}

END_NCBI_SCOPE
