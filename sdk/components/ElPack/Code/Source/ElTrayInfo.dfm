object TrayInfoForm: TTrayInfoForm
  Left = 273
  Top = 274
  BorderStyle = bsNone
  ClientHeight = 22
  ClientWidth = 122
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clInfoText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  Scaled = False
  OnClick = ClickHandler
  OnDblClick = DblClickHandler
  OnHide = HideHandler
  OnShow = ShowHandler
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 122
    Height = 22
    Align = alClient
    BevelOuter = bvNone
    BorderStyle = bsSingle
    Color = clInfoBk
    Ctl3D = False
    ParentCtl3D = False
    TabOrder = 0
    OnClick = ClickHandler
    OnDblClick = DblClickHandler
    object Image1: TImage
      Left = 4
      Top = 2
      Width = 16
      Height = 16
      Picture.Data = {
        07544269746D6170F6000000424DF60000000000000076000000280000001000
        0000100000000100040000000000800000000000000000000000100000001000
        0000000000000000800000800000008080008000000080008000808000008080
        8000C0C0C0000000FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFF
        FF00DDDDDDDDD77DDDDDDDDDDDDD007DDDDDDDDDDDD0F07DDDDDDDDDD008F077
        DDDDDDD008FFF80077DDDD08FFFFFFF8077DD7FFFCCCCCFF807D78FFFF7CCFFF
        F8077FFFFF7CCFFFFF077FFFFF7CCFFFFF077FFFF7CCCFFFFF077FFFFFFFFFFF
        F80DD7FFFF7CCFFFF07DD7FFFFCCCFFF80DDDD77FFFFFFF00DDDDDDD7777777D
        DDDD}
      Transparent = True
      OnClick = ClickHandler
      OnDblClick = DblClickHandler
    end
    object InfoLabel: TElHTMLLabel
      Left = 28
      Top = 2
      Width = 24
      Height = 13
      Cursor = crDefault
      IsHTML = False
      WordWrap = False
      Caption = '        '
      LinkColor = clBlue
      LinkStyle = [fsUnderline]
      OnClick = ClickHandler
      OnDblClick = DblClickHandler
    end
  end
  object Timer: TTimer
    OnTimer = TimerTimer
    Left = 97
    Top = 9
  end
  object ElFormPersist1: TElFormPersist
    PersistOptions = []
    TopMost = True
    Flipped = False
    Left = 73
    Top = 9
    PropsToStore = {00000000}
  end
end