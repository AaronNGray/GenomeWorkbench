#ifndef GUI_WIDGETS_SNP___SURVEY_BROWSER__HPP
#define GUI_WIDGETS_SNP___SURVEY_BROWSER__HPP


/*  $Id: survey_browser.hpp 17710 2008-09-04 20:07:49Z dicuccio $
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
 * Authors:  Melvin Quintos
 *
 * File Description:
 *  ListBox containing three (3) checkboxes.  Intended use is for items with
 *  three (3) unique states.
 *    e.g.  ['yes','no','maybe'] or ["True","False", "Don't care"]
 *
 */

#include <gui/widgets/fl/browser.hpp>

BEGIN_NCBI_SCOPE

class CSurveyBrowser : public CBrowser
{
public:
    CSurveyBrowser();
    CSurveyBrowser(int x, int y, int w, int h, const char* label = NULL);

    /// @name FLTK overrides
    /// @{
    int handle(int event);
    /// @}

    /// @name Extended FLTK API
    /// @{

    /// Add() appends an item to the list and returns the index to that item
    /// Remember: indexes are 1-based!
    virtual int Add(const char* text,   int value = 0, void* data = NULL);
    virtual int Add(const string& text, int value = 0, void* data = NULL);
    virtual void Remove(int row);
    virtual void Insert(int row_before, const char* text, int value = 0, void* data = NULL);
    virtual void Clear();

    void  Set(int row, int value);
    int   Get(int row) const;

    void SetAll(int value);

    /// return the index (1 based) of the last line changed on
    /// the last click.  0 if the last click did not change a check box.
    int GetLastChecked() const;

    /// @}

protected:
    virtual int item_height(void *) const;
    void    item_draw(void* item, int x, int y, int w, int h) const;
    void    x_DrawCheckbox(int x, int y, int w, int h, int idx) const;
    int     x_HitTestItemBox(void *);
private:
    /// @name Deprecated FLTK interface
    /// @{

    void remove(int);
    void add(const char*, void* = 0);
    void insert(int, const char*, void* = 0);
    void move(int to, int from);
    int  load(const char* filename);
    void swap(int a, int b);
    void clear();

    /// @}

private:
    vector<int>    m_CheckState;
    int m_LastChecked;

};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SNP___SNP_OPTIONS_DLG__HPP

