// generated by Fast Light User Interface Designer (fluid) version 1.0106

#include <ncbi_pch.hpp>

inline void CSeqTextDlg::cb_Ok_i(CReturnButton*, void*) {
  x_OnOK();
}
void CSeqTextDlg::cb_Ok(CReturnButton* o, void* v) {
  ((CSeqTextDlg*)(o->parent()->parent()->user_data()))->cb_Ok_i(o,v);
}

inline void CSeqTextDlg::cb_Cancel_i(CButton*, void*) {
  x_OnCancel();
}
void CSeqTextDlg::cb_Cancel(CButton* o, void* v) {
  ((CSeqTextDlg*)(o->parent()->parent()->user_data()))->cb_Cancel_i(o,v);
}

wxPanel* CSeqTextDlg::x_CreateWindow() {
  wxPanel* w;
  {
#if 0
      wxPanel* o = new wxPanel(380, 204, "Sequence Text View Options");
    w = o;
    o->user_data((void*)(this));
    { CDialogBackground* o = new CDialogBackground(0, 0, 380, 204);
      o->box(FL_NO_BOX);
      o->color(FL_BACKGROUND_COLOR);
      o->selection_color(FL_BACKGROUND_COLOR);
      o->labeltype(FL_NORMAL_LABEL);
      o->labelfont(0);
      o->labelsize(14);
      o->labelcolor(FL_BLACK);
      o->align(FL_ALIGN_TOP);
      o->when(FL_WHEN_RELEASE);
      { wxStaticBox* o = m_SubjectLabel = new wxStaticBox(20, 15, 100, 20, "Show feature in");
        o->labelsize(12);
        o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
      }
      { CReturnButton* o = new CReturnButton(180, 173, 75, 22, "Ok");
        o->box(FL_UP_BOX);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_BACKGROUND_COLOR);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(0);
        o->labelsize(12);
        o->labelcolor(FL_BLACK);
        o->callback((Fl_Callback*)cb_Ok);
        o->align(FL_ALIGN_CENTER);
        o->when(FL_WHEN_RELEASE);
      }
      { CButton* o = new CButton(270, 173, 75, 22, "Cancel");
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
      { CRoundButton* o = m_Upper = new CRoundButton(120, 15, 90, 22, "Upper case");
        o->type(102);
        o->box(FL_NO_BOX);
        o->down_box(FL_ROUND_DOWN_BOX);
        o->value(1);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_BLACK);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(0);
        o->labelsize(12);
        o->labelcolor(FL_BLACK);
        o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
        o->when(FL_WHEN_RELEASE);
      }
      { CRoundButton* o = m_Lower = new CRoundButton(210, 15, 90, 22, "Lower case");
        o->type(102);
        o->box(FL_NO_BOX);
        o->down_box(FL_ROUND_DOWN_BOX);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_BLACK);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(0);
        o->labelsize(12);
        o->labelcolor(FL_BLACK);
        o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
        o->when(FL_WHEN_RELEASE);
      }
      { CSeqTextFontList* o = m_FontSize = new CSeqTextFontList(75, 48, 55, 22, "Font Size");
        o->box(FL_FLAT_BOX);
        o->down_box(FL_BORDER_BOX);
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
      { CSeqTextDisplayChoiceList* o = m_FeatureColoration = new CSeqTextDisplayChoiceList(w);
        o->box(FL_FLAT_BOX);
        o->down_box(FL_BORDER_BOX);
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
      { CChoice* o = m_DisplayCoordinates = new CChoice(130, 140, 160, 22, "Display Coordinates");
        o->box(FL_FLAT_BOX);
        o->down_box(FL_BORDER_BOX);
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
    { CSeqTextDisplayChoiceList* o = m_CodonDraw = new CSeqTextDisplayChoiceList(130, 115, 100, 20, "Codon Display");
      o->box(FL_FLAT_BOX);
      o->down_box(FL_BORDER_BOX);
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
#endif
  }
  return w;
}
