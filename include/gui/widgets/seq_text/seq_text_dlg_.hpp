// generated by Fast Light User Interface Designer (fluid) version 1.0106

#ifndef seq_text_dlg__hpp
#define seq_text_dlg__hpp
/* $Id: seq_text_dlg_.hpp 15281 2007-11-01 16:22:10Z bollin $ */

class NCBI_GUIWIDGETS_SEQTEXT_EXPORT CSeqTextDlg : public CDialog {
public:
  CSeqTextDlg(CRef<CSeqTextConfig> cfg, CSeqTextDataSource* pDS);
private:
  wxPanel* x_CreateWindow();
  wxStaticBox *m_SubjectLabel;
  inline void cb_Ok_i(CReturnButton*, void*);
  static void cb_Ok(CReturnButton*, void*);
  inline void cb_Cancel_i(CButton*, void*);
  static void cb_Cancel(CButton*, void*);
public:
  CRoundButton *m_Upper;
  CRoundButton *m_Lower;
  CSeqTextFontList *m_FontSize;
  CSeqTextDisplayChoiceList *m_FeatureColoration;
  CChoice *m_DisplayCoordinates;
  CSeqTextDisplayChoiceList *m_CodonDraw;
private:
  virtual void x_OnOK();
  virtual void x_OnCancel();
  CRef<CSeqTextConfig> m_Cfg;
};
#endif
