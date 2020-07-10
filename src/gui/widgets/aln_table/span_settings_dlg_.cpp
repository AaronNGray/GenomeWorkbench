// generated by Fast Light User Interface Designer (fluid) version 1.0106

/* $Id: span_settings_dlg_.cpp 14565 2007-05-18 12:32:01Z dicuccio $ */
#include <ncbi_pch.hpp>

inline void CSpanTableSettingsDialog::cb_OK_i(CReturnButton*, void*) {
  x_OnOK();
}
void CSpanTableSettingsDialog::cb_OK(CReturnButton* o, void* v) {
  ((CSpanTableSettingsDialog*)(o->parent()->user_data()))->cb_OK_i(o,v);
}

inline void CSpanTableSettingsDialog::cb_Cancel_i(CButton*, void*) {
  x_OnCancel();
}
void CSpanTableSettingsDialog::cb_Cancel(CButton* o, void* v) {
  ((CSpanTableSettingsDialog*)(o->parent()->user_data()))->cb_Cancel_i(o,v);
}

Fl_Double_Window* CSpanTableSettingsDialog::x_CreateWindow() {
  Fl_Double_Window* w;
  { Fl_Double_Window* o = new Fl_Double_Window(310, 150, "Span Table View Settings");
    w = o;
    o->user_data((void*)(this));
    { CDialogBackground* o = new CDialogBackground(0, 0, 310, 150);
      o->box(FL_NO_BOX);
      o->color(FL_BACKGROUND_COLOR);
      o->selection_color(FL_BACKGROUND_COLOR);
      o->labeltype(FL_NORMAL_LABEL);
      o->labelfont(0);
      o->labelsize(14);
      o->labelcolor(FL_BLACK);
      o->align(FL_ALIGN_TOP);
      o->when(FL_WHEN_RELEASE);
      { CFlFrame* o = new CFlFrame(20, 20, 270, 80);
        o->box(FL_ENGRAVED_FRAME);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_BACKGROUND_COLOR);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(1);
        o->labelsize(12);
        o->labelcolor(FL_BLACK);
        o->align(FL_ALIGN_TOP);
        o->when(FL_WHEN_RELEASE);
        { CInput* o = m_Threshold = new CInput(125, 28, 120, 22, "Threshold:");
          o->type(2);
          o->box(FL_DOWN_BOX);
          o->color(FL_BACKGROUND2_COLOR);
          o->selection_color(FL_SELECTION_COLOR);
          o->labeltype(FL_NORMAL_LABEL);
          o->labelfont(0);
          o->labelsize(12);
          o->labelcolor(FL_BLACK);
          o->textsize(12);
          o->align(FL_ALIGN_LEFT);
          o->when(FL_WHEN_RELEASE);
        }
        o->end();
      }
      o->end();
    }
    { CReturnButton* o = new CReturnButton(135, 108, 75, 22, "OK");
      o->box(FL_UP_BOX);
      o->color(FL_BACKGROUND_COLOR);
      o->selection_color(FL_BACKGROUND_COLOR);
      o->labeltype(FL_NORMAL_LABEL);
      o->labelfont(0);
      o->labelsize(12);
      o->labelcolor(FL_BLACK);
      o->callback((Fl_Callback*)cb_OK);
      o->align(FL_ALIGN_CENTER);
      o->when(FL_WHEN_RELEASE);
    }
    { CButton* o = new CButton(215, 108, 75, 22, "Cancel");
      o->box(FL_UP_BOX);
      o->color(FL_BACKGROUND_COLOR);
      o->selection_color(FL_BACKGROUND_COLOR);
      o->labeltype(FL_NORMAL_LABEL);
      o->labelfont(0);
      o->labelsize(12);
      o->labelcolor(FL_BLACK);
      o->callback((Fl_Callback*)cb_Cancel);
      o->align(FL_ALIGN_CENTER);
      o->when(FL_WHEN_RELEASE);
    }
    o->end();
  }
  return w;
}
