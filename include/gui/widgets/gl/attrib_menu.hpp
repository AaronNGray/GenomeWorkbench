#ifndef __GUI_WIDGETS_GL___ATTRIB_MENU__HPP
#define __GUI_WIDGETS_GL___ATTRIB_MENU__HPP

/*  $Id: attrib_menu.hpp 27503 2013-02-26 18:16:55Z falkrb $
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

#include <corelib/ncbistl.hpp>

#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>
#include <gui/utils/vect4.hpp>
#include <gui/opengl/glbitmapfont.hpp>

/**
OpenGL-based Attribute menus

Purpose:
This is a set of utility classes to make it quick and easy to add opengl-based
menus to opengl views.  These menus can be used to interactively update local
variables for the purpose of tuning paramenters for various algorithms.  This
is not functionality for end-users and should be compiled out of production 
builds (or commented out before being checked in).

Usage:

There are 5 parts to using the menus: Create, add menu items, capture keyboard
updates, render menu, and delete.  For creation of the toplevel menu it's going
to be easiest to use the singleton CAttribMenuInstance to get the global menu
instance.  This also allows you to share the menu items across several classes,
such as a widget, a pane, and a renderer.

After getting a menu pointer, you can add or delete menu items to allow you
to interactively adjust or display values in your classes. When adding a
menu entry, you will generally also include a pointer to the data element
to be adjusted.  If you do not provide a pointer to local data, then
whenever you want to update your local data, you'll have to query the
attribute menu for the items current value and update the data yourself.

Example:

class ImportantClass  : public ImportantGLBaseClass
{
public
...
protected:
    float m_DampingFactor;
    int  m_MaxIterations;
    std::string m_RunType;
    float m_UpdateTimeT;
};

ImportantClass::ImportantClass()
{
    /// Get global menu instance
    CAttribMenu& m = CAttribMenuInstance::GetInstance();

    /// Create a submenu to which to add our items (a good idea if you
    /// will have multiple menu users.  If there will be multiple
    /// instances of this view, you may will need to add a pointer
    /// as an instance identifier).  Using AddSubMenuUnique will add
    /// a menu item (with a unique number on the end) even if the name
    /// already exists in the menu.
    CAttribMenu* m1 = m.AddSubMenuUnique("Important Sub Menu", this);

    /// add a floating point value, 0..1, initial value 0.1
    m1->AddFloat("Damping", &m_DampingFactor, 0.1f, 0.0f, 1.0f, 0.01f);

    /// add an integer, 0..100, initial value 10
    m1->AddInt("Iterations", &m_MaxIterations, 10, 0, 100, 1);

    /// Add a string item with two possible values
    CAttribStringsMenuItem* item = m1->AddStrings("Run Type", &m_RunType);
    item->AddString("Normal Run");
    item->AddString("Fast Run");

    // add a read-only value to the menu (display a timer):
     m1->AddFloatReadOnly("Update Time", &m_UpdateTimeT);
}

ImportantClass::~ImportantClass()
{
    /// Get global menu instance
    CAttribMenu& m = CAttribMenuInstance::GetInstance();
    /// Deletes submenu and all its items
    m.RemoveMenuR("Important Sub Menu", this);
}

void ImportantClass::OnKeyEvent(wxKeyEvent& event)
{
    // Note that since we are using the static instance,
    // The KeyPress() and DrawMenu() functions could be in different
    // classes than the data and menu creation is.
    bool b = CAttribMenuInstance::GetInstance().KeyPress( event.GetKeyCode() );
    Refresh();
    
    event.Skip();
}

void ImportantClass::Draw()
{
.....
    CAttribMenuInstance::GetInstance().DrawMenu();
}

*/

#ifdef KeyPress
#  undef KeyPress
#endif

/// You can turn this on/off locally to enable/disable the use of attribute menus
/// in classes.  It should be commented out when checked in.
//#define ATTRIB_MENU_SUPPORT

BEGIN_NCBI_SCOPE


enum EAttribType { AttribMenuType, AttribIntType, AttribFloatType, AttribStringsType };

class CAttribMenu;

/**
class CAttribMenuItem

This is the base class for the menu items.  It stores the item
name and color, and defines virtual functions for incrementing and 
decrementing the items value as well as for getting the item's type.
*/
class NCBI_GUIWIDGETS_GL_EXPORT CAttribMenuItem
{
public:
    CAttribMenuItem() 
        : m_Color(0.0f)
        , m_DataColor(1.0f, 0.0f, 0.0f)
        , m_ReadOnly(false)
        , m_FontType(CGlBitmapFont::eHelvetica12) {}            
    CAttribMenuItem(const std::string& name) 
        : m_Name(name)
        , m_Color(0.0f)
        , m_DataColor(1.0f, 0.0f, 0.0f)
        , m_ReadOnly(false)
        , m_FontType(CGlBitmapFont::eHelvetica12) {}

    virtual ~CAttribMenuItem() {}

    /// Get and set the menu item name
    void SetName(const std::string& name);
    std::string GetName() const { return m_Name; }

    /// Set/get the color of the menu name text
    void SetColor(const CVect3<float>& c) { m_Color = c; }
    CVect3<float> GetColor() const { return m_Color; }

    /// Set/get the color of the menu value text
    void SetDataColor(const CVect3<float>& c) { m_DataColor = c; }
    CVect3<float> GetDataColor() const { return m_DataColor; }

    /// Set/get the font type for the menu item
    void SetFontType(CGlBitmapFont::EFont ft) { m_FontType = ft; }
    CGlBitmapFont::EFont GetFontType() const { return m_FontType; }

    /// Set/get readonly paramater that determines if value is updated or not
    void SetReadOnly(bool b) { m_ReadOnly = b; }
    bool GetReadOnly() const { return m_ReadOnly; }

    /// Draw the menu item at the specified position
    CVect2<float> Draw(CVect2<float> pos);
    /// Get width and height of the text for this menu item
    CVect2<float> GetTextDim() const;

    /// Virtual function for getting menu text (specialized for
    /// different subtypes as needed)
    virtual std::string GetText() const { return m_Name; }
    /// Virtual function for getting menu items value as a string
    virtual std::string GetTextValue() const { return ""; }

    /// Increment value is specialized for each subclass
    virtual void Inc() {};
    /// Decrement value is specialized for each subclass (except the menu type)
    virtual void Dec() {}; 

    /// Set current menu values to values of user variables, if any.
    /// Normally it is the menu that updates the variable, when the
    /// user increments or decrements.
    virtual void SynchToUserValue() = 0;
    
    /// Returns item type of subclass
    virtual EAttribType GetType() const = 0;

protected:
    /// Item name that appears on the menu item
    std::string m_Name;
    /// Color of the name text
    CVect3<float> m_Color;
    /// Color of the value text
    CVect3<float> m_DataColor;
    /// If true, item is displayed but not updated
    bool m_ReadOnly;
    /// Font used for the menu
    CGlBitmapFont::EFont m_FontType;   
};

/**
class CAttribIntMenuItem

This menu item represnets an integer value.  Hitting the increment and
decrement keys changes the value by m_StepSize.
*/
class NCBI_GUIWIDGETS_GL_EXPORT CAttribIntMenuItem : public CAttribMenuItem
{
public:
    CAttribIntMenuItem()
        : m_Value(50)
        , m_UserValue(NULL)
        , m_MinValue(0)
        , m_MaxValue(100)
        , m_StepSize(1) {}
    CAttribIntMenuItem(const std::string& name, int* v = NULL) 
        : CAttribMenuItem(name)
        , m_Value(50)
        , m_UserValue(v)
        , m_MinValue(0)
        , m_MaxValue(100)
        , m_StepSize(1) {}      

    /// Set the current menu item value
    void SetValue(int v) { m_Value = v; }
    /// Return the current menu item value
    int GetValue() const { return m_Value; }

    /// Set the user data to be updated with the menu changes
    void SetUserValue(int* v) { m_UserValue = v; }
    /// Get pointer to user value that is updated with the item (May be NULL)
    int* GetUserValue() const { return m_UserValue; }

    /// Set the range of integers for the item
    void SetRange(int from, int to);
    /// Set the increment/decrement amount
    void SetStepSize(int s) { m_StepSize = s; }

    /// Get menu text including name and value
    virtual std::string GetText() const;
    /// Get string version of current value
    virtual std::string GetTextValue() const;

    // Increment value by m_StepSize or wrap to m_MinValue
    virtual void Inc();
    // Decrement value by m_StepSize, or wrap to m_MaxValue
    virtual void Dec(); 

    /// Set current menu value to m_UserValue, if available
    virtual void SynchToUserValue() { if (m_UserValue!=NULL) m_Value = *m_UserValue; }

    /// Get item type
    virtual EAttribType GetType() const { return AttribIntType; }

protected:
    /// Current menu item value
    int m_Value;
    /// Pointer to user integer to be automatically updated (may be NULL)
    int* m_UserValue;

    /// Minimum integer value
    int m_MinValue;
    /// Maximum integer value
    int m_MaxValue;
    /// Increment/decrement amount
    int m_StepSize;
};

/**
class CAttribFloatMenuItem

This menu item represnets a floating point value.  Hitting the increment and
decrement keys changes the value by m_StepSize.
*/
class NCBI_GUIWIDGETS_GL_EXPORT CAttribFloatMenuItem : public CAttribMenuItem
{
public:
    CAttribFloatMenuItem()
        : m_Value(0.5f)
        , m_UserValue(NULL)
        , m_MinValue(0.0f)
        , m_MaxValue(1.0f)
        , m_StepSize(0.01f) {}
    CAttribFloatMenuItem(const std::string& name, float* v = NULL) 
        : CAttribMenuItem(name)
        , m_Value(0.5f)
        , m_UserValue(v)
        , m_MinValue(0.0f)
        , m_MaxValue(1.0f)
        , m_StepSize(0.01f) {}      

    /// Set the current value of the item
    void SetValue(float v) { m_Value = v; }
    /// Return the current value of the menu item
    float GetValue() const { return m_Value; }

    /// Set a pointer to a caller-owned floating point value to update
    void SetUserValue(float* v) { m_UserValue = v; }
    /// Get the current user-owned data (may be NULL)
    float* GetUserValue() const { return m_UserValue; }

    /// Set floating point range for increment/decremnt
    void SetRange(float from, float to);
    /// Set the amount the menu item is updated with each increment/decrement
    void SetStepSize(float s) { m_StepSize = s; }

    /// Retrn the menu text (including value)
    virtual std::string GetText() const;
    /// Return the value only, in text form
    virtual std::string GetTextValue() const;

    /// Increment value by m_StepSize, or wrap to m_MinSize
    virtual void Inc();
    /// Decrement value by m_StepSize or wrap to m_MaxSize
    virtual void Dec(); 

    /// Set current menu value to m_UserValue, if available
    virtual void SynchToUserValue() { if (m_UserValue!=NULL) m_Value = *m_UserValue; }

    /// Get item type
    virtual EAttribType GetType() const { return AttribFloatType; }

protected:
    /// Current menu item value
    float m_Value;
    /// Pointer to user-owned value to be automatically updated (may be NULL)
    float* m_UserValue;

    /// Lowest floating point value
    float m_MinValue;
    /// Maximum floating point value
    float m_MaxValue;
    /// Increment/decrement amount
    float m_StepSize;
};

/**
class CAttribStringsMenuItem

This menu item represents an array of strings.  Hitting the increment and
decrement keys displays the next/previous element in the string array. Adding
strings to the menu item requires repeated calls to AddString(), or call
SetStrings with a vector of strings (or an empty vector to clear the list).
*/
class NCBI_GUIWIDGETS_GL_EXPORT CAttribStringsMenuItem : public CAttribMenuItem
{
public:
    CAttribStringsMenuItem() : m_CurrentIdx(0), m_UserValue(NULL) {}
    CAttribStringsMenuItem(const std::string& name, std::string* v = NULL) 
        : CAttribMenuItem(name)
        , m_CurrentIdx(0)
        , m_UserValue(v) {}

    /// Set the index into the string array which is the current value
    void SetValue(size_t idx) { m_CurrentIdx = idx; }
    /// Get current string value
    std::string GetValue() const { return m_Value[m_CurrentIdx]; }

    /// Set pointer to a user string which will be updated with menu changes
    void SetUserValue(std::string* v) { m_UserValue = v; }
    /// Return the users string value
    std::string* GetUserValue() const { return m_UserValue; }

    /// Set all strings to s (replaces current strings)
    void SetStrings(const std::vector<std::string>& s) { m_Value = s; }
    /// Add a string to the back of the string set
    void AddString(const std::string& s) { m_Value.push_back(s); }

    /// Get display text for menu
    virtual std::string GetText() const;
    /// Get string value to be displayed on menu
    virtual std::string GetTextValue() const;

    /// Increment to next string in array (or wrap to start)
    virtual void Inc();
    /// Decrement to previous string in array (or wrap to end)
    virtual void Dec(); 

    /// Set current menu value to m_UserValue, if available.  If the
    /// current string in m_UserValue is not in the m_Value array, the
    /// current value is not changed.
    virtual void SynchToUserValue();

    /// Get item type
    virtual EAttribType GetType() const { return AttribStringsType; }
protected:
    /// Array of string values to select from
    std::vector<std::string> m_Value;
    /// Index of current value in string array
    size_t m_CurrentIdx;
    /// Pointer to user string value to update (may be null)
    std::string* m_UserValue;
};

/**
class CAttribMenuItem

This is the primary class for displaying attribute menus, adding items to
menus and getting values from menus.  A program wanting to use attribute
menus will:

get an instance to a base menu
will add items to that menu: Add[Int/Float/Strings/SubMenu]()
will route key events to the menus KeyPress() function
will draw the menu from their drawing update: DrawMenu()

If you share a menu instance across multiple windows, e.g. via
the singleton 'CAttribMenuInstance' class, then be careful to delete menu
items that are not longer needed in class destructors.  Also if multiple
instances of a view might be open, then create a separate submenu
for each instance of the class, and delete those menu instances in
the view destructors.  This is most important if you are passing your
variables (user values) to the menu.
*/
class NCBI_GUIWIDGETS_GL_EXPORT CAttribMenu : public CAttribMenuItem
{
public:
    CAttribMenu() 
        : m_UserValue(NULL)
        , m_BackgroundColor(0.9f, 0.9f, 0.9f, 0.8f)
        , m_CurrentItem(0)
        , m_IsOpen(false)
        , m_KeyUp('W')
        , m_KeyDown('S')
        , m_KeyInc('D')
        , m_KeyDec('A') 
        , m_OpenKey('+') 
        , m_CloseKey('-') {}
    CAttribMenu(const std::string& name, void* user_value = NULL) 
        : CAttribMenuItem(name)
        , m_UserValue(user_value)
        , m_BackgroundColor(0.9f, 0.9f, 0.9f, 0.8f)
        , m_CurrentItem(0)
        , m_IsOpen(false)
        , m_KeyUp('W')
        , m_KeyDown('S')
        , m_KeyInc('D')
        , m_KeyDec('A') 
        , m_OpenKey('+') 
        , m_CloseKey('-') {}

    ~CAttribMenu();

    // Draw menu in corner of screen
    void DrawMenu();

    // Returns true if menus responded to keypress in some way
    bool KeyPress(int key_code);

    /// Returns true on success (if name is unique in this menu)
    bool AddItem(CAttribMenuItem* a);

    /// Convienance function to add a submenu to this menu.  If the menu name
    /// is not unique, a number will be added to the name to make it unique.
    CAttribMenu* AddSubMenuUnique(const std::string& name, void* user_value = NULL);

    /// Convienance funcions to add new menu elements or submenus.
    /// Only one element with a given name can be added to the same menu,
    /// even if the variable (user_value) is different.
    CAttribMenu* AddSubMenu(const std::string& name, void* user_value = NULL);   
    CAttribFloatMenuItem* AddFloat(const std::string& name,
                                   float* user_value = NULL,
                                   float initial_value = 0.5f,
                                   float min_value = 0.0f,
                                   float max_value = 1.0f,
                                   float stepsize = 0.01f);
    CAttribIntMenuItem* AddInt(const std::string& name,
                               int* user_value = NULL,
                               int initial_value = 50,
                               int min_value = 0,
                               int max_value = 100,
                               int stepsize = 1);
    CAttribStringsMenuItem* AddStrings(const std::string& name, 
                                       std::string* user_value = NULL);

    /// Add entries to the menu which display the users value but
    /// do not update it.
    CAttribFloatMenuItem* AddFloatReadOnly(const std::string& name,
                                           float* user_value);
    CAttribIntMenuItem* AddIntReadOnly(const std::string& name,
                                      int* user_value);
    CAttribStringsMenuItem* AddStringsReadOnly(const std::string& name, 
                                               std::string* user_value);    
    
    /// Return a copy of the items in this menu
    std::vector<CAttribMenuItem*> GetValue() const { return m_Value; }

    /// Find a menu item in the current menu (do not search recursively)
    CAttribMenuItem *FindItem(const std::string& name);
    CAttribMenu *FindMenu(const std::string& name);
    CAttribFloatMenuItem *FindFloat(const std::string& name);
    CAttribIntMenuItem *FindInt(const std::string& name);
    CAttribStringsMenuItem *FindStrings(const std::string& name);

    /// Remove an item from the menu with the specified name
    bool RemoveItem(const std::string& name);
    /// Search the menu(s) recursively for menu 'name' with the specified user 
    /// data 'user_value'.  If 'user_value' is NULL, delete first 'name' found.
    /// If user_value is not NULL, name only has to match the root of the
    /// menu name (to support AddSubMenuUnique)
    bool RemoveMenuR(const std::string& name, void* user_value = NULL);

    //CAttribMenuItem *FindItemR(const std::string& name);
    //bool RemoveItemR(CAttribMenuItem *item);

    /// Set keys for menu up/down and previous/next value. Default is numpad
    /// arrow keys with capslock but you may want letters since laptops don't
    /// always have numpad.  
    void SetKeys(int key_up, int key_down, int increment, int decrement);
    /// Opening and closing menu will always work with numpad +/-. By default
    /// char '+', '-' also work but you can use different chars (laptops
    /// don't support numpad)
    void SetOpenCloseKeys(int open_key, int close_key);

    /// Add a '>' to name of menu for simple indicator of submenu
    virtual std::string GetText() const { return m_Name + " >"; }
    /// The value is the part of the menu after the name, so here it's " >"
    virtual std::string GetTextValue() const { return " >"; }

    /// Menus are either open or closed, based on user navigation
    bool IsOpen() const { return m_IsOpen; }

    /// Set/get menu background color
    void SetBackgroundColor(const CVect4<float>& c) { m_BackgroundColor = c; }
    CVect4<float> GetBackgroundColor() const { return m_BackgroundColor; }

    // Hitting the increment key for menus opens a submenu
    virtual void Inc();
    // Decrement key doesn't have meaning for menus since incement puts you
    // in the submenu, you can't decrement from there.
    //virtual void Dec(); 

    /// Recursively calls each menu item to set its menu value to the
    /// current user value (if not NULL).
    virtual void SynchToUserValue();

    /// Return the subtype.
    virtual EAttribType GetType() const { return AttribMenuType; }

protected:
    /// respond to key value
    bool x_HandleKey(int key_code);

    /// Draw submenu (called by toplevel draw)
    void x_DrawMenu(CVect2<float> pos);

    /// Set read-only menu values to their variable value.
    void x_UpdateReadOnly();

    /// array of menuitems for this menu 
    std::vector<CAttribMenuItem*> m_Value;
    /// Pointer used Only to identify this menu (to look it up later,
    /// for example in the dtor of an object so that it can be deleted
    void* m_UserValue;

    /// menu (background) color
    CVect4<float> m_BackgroundColor;
    /// Current active (focused) item in this submenu
    size_t m_CurrentItem;
    /// True if menu is open (displayed).
    bool m_IsOpen;

    /// Up to previous menu item
    int m_KeyUp;
    /// Down to next menu item
    int m_KeyDown;
    /// Increment value of current menu item
    int m_KeyInc;
    /// Decrement value of current menu item
    int m_KeyDec;

    /// key for opening a menu
    int m_OpenKey;
    /// key for closing a menu
    int m_CloseKey;
};

/**
Simple singleton class to allow multiple classes
to share the same menu. Since this singleton class
is separate from the menu class itself, users that want
local menus can create them.
*/
class NCBI_GUIWIDGETS_GL_EXPORT CAttribMenuInstance 
{
public:
    /// Return a static instance of CAttribMenu
    static CAttribMenu& GetInstance();

private:
    /// Defined privite to only allow the one instance
    CAttribMenuInstance() {}
    ~CAttribMenuInstance() {}

    /// Intentionally undefined
    CAttribMenuInstance(const CAttribMenuInstance&);
    CAttribMenuInstance& operator=(const CAttribMenuInstance&);
};



END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_GL___ATTRIB_MENU__HPP
