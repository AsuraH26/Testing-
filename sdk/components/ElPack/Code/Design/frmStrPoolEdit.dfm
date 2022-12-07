object StrPoolEditForm: TStrPoolEditForm
  Left = 255
  Top = 208
  Width = 584
  Height = 298
  Caption = 'Strings Editor'
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Position = poScreenCenter
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 120
  TextHeight = 16
  object ElPanel1: TElPanel
    Left = 0
    Top = 226
    Width = 576
    Height = 44
    AlphaLevel = 0
    BackgroundType = bgtColorFill
    Align = alBottom
    BevelOuter = bvNone
    MouseCapture = False
    object OkBtn: TElPopupButton
      Left = 129
      Top = 7
      Width = 93
      Height = 31
      DrawDefaultFrame = False
      Default = True
      ModalResult = 1
      NumGlyphs = 1
      ShowFocus = False
      Caption = 'OK'
      TabOrder = 0
      Color = clBtnFace
      ParentColor = False
      OnClick = OkBtnClick
    end
    object CancelBtn: TElPopupButton
      Left = 228
      Top = 7
      Width = 92
      Height = 31
      DrawDefaultFrame = False
      Cancel = True
      ModalResult = 2
      NumGlyphs = 1
      ShowFocus = False
      Caption = 'Cancel'
      TabOrder = 1
      Color = clBtnFace
      ParentColor = False
    end
  end
  object ElPanel2: TElPanel
    Left = 0
    Top = 0
    Width = 576
    Height = 226
    AlphaLevel = 0
    BackgroundType = bgtColorFill
    Align = alClient
    BevelOuter = bvNone
    MouseCapture = False
    object ElPanel3: TElPanel
      Left = 0
      Top = 0
      Width = 218
      Height = 226
      AlphaLevel = 0
      BackgroundType = bgtColorFill
      Align = alLeft
      BevelOuter = bvNone
      MouseCapture = False
      object ElPanel4: TElPanel
        Left = 0
        Top = 153
        Width = 218
        Height = 73
        AlphaLevel = 0
        BackgroundType = bgtColorFill
        Align = alBottom
        BevelOuter = bvNone
        MouseCapture = False
        object AddBtn: TElPopupButton
          Left = 12
          Top = 5
          Width = 61
          Height = 26
          DrawDefaultFrame = False
          NumGlyphs = 1
          ShowFocus = False
          ShowGlyph = False
          Caption = '&Add'
          TabOrder = 0
          Color = clBtnFace
          ParentColor = False
          OnClick = AddBtnClick
        end
        object InsertBtn: TElPopupButton
          Left = 79
          Top = 5
          Width = 60
          Height = 26
          DrawDefaultFrame = False
          NumGlyphs = 1
          ShowFocus = False
          ShowGlyph = False
          Caption = '&Insert'
          Enabled = False
          TabOrder = 1
          Color = clBtnFace
          ParentColor = False
          OnClick = InsertBtnClick
        end
        object DeleteBtn: TElPopupButton
          Left = 145
          Top = 5
          Width = 61
          Height = 26
          DrawDefaultFrame = False
          NumGlyphs = 1
          ShowFocus = False
          ShowGlyph = False
          Caption = '&Delete'
          Enabled = False
          TabOrder = 2
          Color = clBtnFace
          ParentColor = False
          OnClick = DeleteBtnClick
        end
        object UpBtn: TElPopupButton
          Left = 79
          Top = 34
          Width = 60
          Height = 26
          DrawDefaultFrame = False
          NumGlyphs = 1
          ShowFocus = False
          ShowGlyph = False
          Caption = 'Up'
          Enabled = False
          TabOrder = 3
          Color = clBtnFace
          ParentColor = False
          OnClick = UpBtnClick
        end
        object DownBtn: TElPopupButton
          Left = 145
          Top = 34
          Width = 61
          Height = 26
          DrawDefaultFrame = False
          NumGlyphs = 1
          ShowFocus = False
          ShowGlyph = False
          Caption = 'Down'
          Enabled = False
          TabOrder = 4
          Color = clBtnFace
          ParentColor = False
          OnClick = DownBtnClick
        end
        object CopyBtn: TElPopupButton
          Left = 12
          Top = 34
          Width = 61
          Height = 26
          DrawDefaultFrame = False
          NumGlyphs = 1
          ShowFocus = False
          ShowGlyph = False
          Caption = 'Copy'
          Enabled = False
          TabOrder = 5
          Color = clBtnFace
          ParentColor = False
          OnClick = CopyBtnClick
        end
      end
      object List: TElTree
        Left = 0
        Top = 0
        Width = 218
        Height = 153
        Cursor = crDefault
        LeftPosition = 0
        Align = alClient
        AutoCollapse = False
        AutoLookup = True
        DefaultSectionWidth = 120
        BorderSides = [ebsLeft, ebsRight, ebsTop, ebsBottom]
        DoInplaceEdit = False
        DragCursor = crDrag
        ExpandOnDblClick = False
        ExplorerEditMode = False
        Flat = True
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -14
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        GradientSteps = 64
        HeaderHeight = 20
        HeaderHotTrack = False
        HeaderSections.Data = {
          F5FFFFFF020000004054004100140000FFFFFFFF00000101000000411E000000
          00000000102700000000026DA060530100000000020000410000000000000120
          000000000000000000010000000000006E20627574746F6E292E207D0D0A6265
          67696E0D0A202047657441747472696275746573203A3D205B70614469616C6F
          675D3B0D0A656E643B20207B2047657441747472020000002300010000000000
          000000000000004054004101140000FFFFFFFF00000101010000418E00000000
          000000102700000000006D586153010100000001000041000000000000012000
          0000000000000000010000000000006E20627574746F6E292E207D0D0A626567
          696E0D0A202047657441747472696275746573203A3D205B70614469616C6F67
          5D3B0D0A656E643B20207B204765744174747205000000546578740001000000
          000000000000000000}
        HeaderFlat = True
        HeaderFont.Charset = DEFAULT_CHARSET
        HeaderFont.Color = clWindowText
        HeaderFont.Height = -11
        HeaderFont.Name = 'MS Sans Serif'
        HeaderFont.Style = []
        HorizontalLines = True
        HorzScrollBarStyles.ShowTrackHint = False
        HorzScrollBarStyles.Width = 15
        HorzScrollBarStyles.ButtonSize = 13
        HorzScrollBarStyles.UseSystemMetrics = False
        IgnoreEnabled = False
        IncrementalSearch = False
        KeepSelectionWithinLevel = False
        LineBorderActiveColor = clBlack
        LineBorderInactiveColor = clBlack
        LineHeight = 19
        LinesColor = clBtnShadow
        MainTreeColumn = 1
        MouseFrameSelect = True
        MultiSelect = False
        OwnerDrawMask = '~~@~~'
        PopupMenu = PopupMenu
        ScrollbarOpposite = False
        ShowButtons = False
        ShowColumns = True
        ShowImages = False
        ShowLeafButton = False
        ShowLines = False
        SortSection = 1
        SortType = stCustom
        StoragePath = 'Tree'
        TabOrder = 1
        TabStop = True
        UnderlineTracked = False
        VerticalLines = True
        VertScrollBarStyles.ShowTrackHint = True
        VertScrollBarStyles.Width = 15
        VertScrollBarStyles.ButtonSize = 13
        VertScrollBarStyles.UseSystemMetrics = False
        VirtualityLevel = vlTextAndStyles
        TextColor = clBtnText
        OnItemDeletion = ListItemDeletion
        OnItemFocused = ListItemFocused
        OnVirtualTextNeeded = ListVirtualTextNeeded
        OnVirtualHintNeeded = ListVirtualHintNeeded
      end
    end
    object ElSplitter1: TElSplitter
      Left = 218
      Top = 0
      Width = 4
      Height = 226
      Cursor = crHSplit
      MinSizeTopLeft = 185
      SnapTopLeft = True
      SnapBottomRight = True
      ControlTopLeft = ElPanel3
      AutoHide = False
      Align = alLeft
      BevelOuter = bvNone
    end
    object Memo: TElEdit
      Left = 222
      Top = 0
      Width = 354
      Height = 226
      Cursor = crIBeam
      AutoSize = False
      Alignment = taLeftJustify
      BorderSides = [ebsLeft, ebsRight, ebsTop, ebsBottom]
      RTLContent = False
      Transparent = False
      WantTabs = True
      TopMargin = 0
      BorderStyle = bsSingle
      Lines.Strings = (
        '')
      MultiLine = True
      Flat = True
      LineBorderActiveColor = clBlack
      LineBorderInactiveColor = clBlack
      WordWrap = False
      ScrollBars = ssBoth
      Align = alClient
      Ctl3D = True
      Enabled = False
      ParentColor = False
      ParentCtl3D = False
      TabOrder = 2
      TabStop = True
    end
  end
  object PopupMenu: TPopupMenu
    Left = 312
    Top = 8
    object AddItem: TMenuItem
      Caption = '&Add'
      OnClick = AddBtnClick
    end
    object InsertItem: TMenuItem
      Caption = '&Insert'
      Enabled = False
      OnClick = InsertBtnClick
    end
    object DeleteItem: TMenuItem
      Caption = '&Delete'
      Enabled = False
      OnClick = DeleteBtnClick
    end
  end
  object MainMenu: TMainMenu
    Left = 232
    Top = 88
    object Pool1: TMenuItem
      Caption = 'Pool'
      object Clear1: TMenuItem
        Caption = 'Clear'
      end
      object Open1: TMenuItem
        Caption = 'Open ...'
      end
      object Save1: TMenuItem
        Caption = 'Save ...'
      end
    end
    object Text1: TMenuItem
      Caption = 'Item'
      object Open2: TMenuItem
        Caption = 'Open ...'
      end
      object Save2: TMenuItem
        Caption = 'Save ...'
      end
    end
  end
  object ElFormPersist1: TElFormPersist
    Storage = ElIniFile1
    PersistOptions = [epoState, epoPosition, epoSize]
    TopMost = False
    Flipped = False
    Left = 96
    Top = 24
    PropsToStore = {00000000}
  end
  object ElIniFile1: TElIniFile
    BinaryMode = False
    Comment = ';'
    Delimiter = '\'
    Path = '\SOFTWARE\EldoS\ElPack\Design-time'
    RegRoot = rrtHKEY_CURRENT_USER
    UseRegistry = True
    WarningMessage = 'Automatically generated file. DO NOT MODIFY!!!'
    Left = 40
    Top = 24
  end
end