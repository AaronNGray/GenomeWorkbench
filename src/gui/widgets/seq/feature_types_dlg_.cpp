// generated by Fast Light User Interface Designer (fluid) version 1.0106

/* $Id: feature_types_dlg_.cpp 14565 2007-05-18 12:32:01Z dicuccio $ */
#include <ncbi_pch.hpp>

inline void CFeatureTypesDlg::cb_m_Ok_i(CReturnButton*, void*) {
  x_OnOK();
}
void CFeatureTypesDlg::cb_m_Ok(CReturnButton* o, void* v) {
  ((CFeatureTypesDlg*)(o->parent()->parent()->user_data()))->cb_m_Ok_i(o,v);
}

inline void CFeatureTypesDlg::cb_m_Cancel_i(CButton*, void*) {
  x_OnCancel();
}
void CFeatureTypesDlg::cb_m_Cancel(CButton* o, void* v) {
  ((CFeatureTypesDlg*)(o->parent()->parent()->user_data()))->cb_m_Cancel_i(o,v);
}

inline void CFeatureTypesDlg::cb_m_ClearAllBtn_i(CButton*, void*) {
  x_OnClearAll();
}
void CFeatureTypesDlg::cb_m_ClearAllBtn(CButton* o, void* v) {
  ((CFeatureTypesDlg*)(o->parent()->parent()->user_data()))->cb_m_ClearAllBtn_i(o,v);
}

inline void CFeatureTypesDlg::cb_m_CheckAllBtn_i(CButton*, void*) {
  x_OnCheckAll();
}
void CFeatureTypesDlg::cb_m_CheckAllBtn(CButton* o, void* v) {
  ((CFeatureTypesDlg*)(o->parent()->parent()->user_data()))->cb_m_CheckAllBtn_i(o,v);
}

Fl_Double_Window* CFeatureTypesDlg::x_CreateWindow() {
  Fl_Double_Window* w;
  { Fl_Double_Window* o = new Fl_Double_Window(374, 315);
    w = o;
    o->labelsize(12);
    o->user_data((void*)(this));
    { CDialogBackground* o = new CDialogBackground(0, 0, 375, 315);
      o->box(FL_NO_BOX);
      o->color(FL_BACKGROUND_COLOR);
      o->selection_color(FL_BACKGROUND_COLOR);
      o->labeltype(FL_NORMAL_LABEL);
      o->labelfont(0);
      o->labelsize(14);
      o->labelcolor(FL_BLACK);
      o->align(FL_ALIGN_TOP);
      o->when(FL_WHEN_RELEASE);
      { CReturnButton* o = m_Ok = new CReturnButton(280, 243, 80, 22, "Ok");
        o->box(FL_UP_BOX);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_BACKGROUND_COLOR);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(0);
        o->labelsize(12);
        o->labelcolor(FL_BLACK);
        o->callback((Fl_Callback*)cb_m_Ok);
        o->align(FL_ALIGN_CENTER);
        o->when(FL_WHEN_RELEASE);
      }
      { CButton* o = m_Cancel = new CButton(280, 273, 80, 22, "Cancel");
        o->box(FL_UP_BOX);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_BACKGROUND_COLOR);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(0);
        o->labelsize(12);
        o->labelcolor(FL_BLACK);
        o->callback((Fl_Callback*)cb_m_Cancel);
        o->align(FL_ALIGN_CENTER);
        o->when(FL_WHEN_RELEASE);
      }
      { CFeatureTypesBrowser* o = m_Browser = new CFeatureTypesBrowser(10, 10, 260, 285);
        o->box(FL_NO_BOX);
        o->color(FL_BACKGROUND2_COLOR);
        o->selection_color(FL_SELECTION_COLOR);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(0);
        o->labelsize(12);
        o->labelcolor(FL_BLACK);
        o->textsize(12);
        o->align(FL_ALIGN_LEFT);
        o->when(FL_WHEN_CHANGED);
        Fl_Group::current()->resizable(o);
      }
      { CButton* o = m_ClearAllBtn = new CButton(280, 43, 80, 22, "Clear All");
        o->box(FL_UP_BOX);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_BACKGROUND_COLOR);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(0);
        o->labelsize(12);
        o->labelcolor(FL_BLACK);
        o->callback((Fl_Callback*)cb_m_ClearAllBtn);
        o->align(FL_ALIGN_CENTER);
        o->when(FL_WHEN_RELEASE);
      }
      { CButton* o = m_CheckAllBtn = new CButton(280, 13, 80, 22, "Check All");
        o->box(FL_UP_BOX);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_BACKGROUND_COLOR);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(0);
        o->labelsize(12);
        o->labelcolor(FL_BLACK);
        o->callback((Fl_Callback*)cb_m_CheckAllBtn);
        o->align(FL_ALIGN_CENTER);
        o->when(FL_WHEN_RELEASE);
      }
      o->end();
    }
    o->end();
  }
  return w;
}