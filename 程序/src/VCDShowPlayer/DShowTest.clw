; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CDShowTestDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "dshowtest.h"
LastPage=0

ClassCount=4
Class1=CDShowTestApp
Class2=CAboutDlg
Class3=CDShowTestDlg
Class4=CInfoDlg

ResourceCount=5
Resource1=IDR_MENU1
Resource2=IDD_DSHOWTEST_DIALOG
Resource3=IDD_DISPINFO (English (U.S.))
Resource4=IDD_ABOUTBOX
Resource5=IDR_TOOLBAR1

[CLS:CDShowTestApp]
Type=0
BaseClass=CWinApp
HeaderFile=DShowTest.h
ImplementationFile=DShowTest.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=DShowTestDlg.cpp
ImplementationFile=DShowTestDlg.cpp
LastObject=CAboutDlg

[CLS:CDShowTestDlg]
Type=0
BaseClass=CDialog
HeaderFile=DShowTestDlg.h
ImplementationFile=DShowTestDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=IDM_TEST_INTERFACE

[CLS:CInfoDlg]
Type=0
BaseClass=CDialog
HeaderFile=InfoDlg.h
ImplementationFile=InfoDlg.cpp

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_DSHOWTEST_DIALOG]
Type=1
Class=CDShowTestDlg
ControlCount=1
Control1=IDM_PIC,static,1342177287

[DLG:IDD_DISPINFO]
Type=1
Class=CInfoDlg

[DLG:IDD_DISPINFO (English (U.S.))]
Type=1
Class=?
ControlCount=1
Control1=IDC_EDIT1,edit,1352732676

[MNU:IDR_MENU1]
Type=1
Class=?
Command1=IDM_CapturePreview
Command2=IDC_TestOurFilter
Command3=IDM_STOP
Command4=IDM_RUN
Command5=IDC_TRACKING_PREV_ASBKGND
Command6=IDC_TRACKING_FIRST_ASBKGND
Command7=IDC_TRACKING_DYNAMIC_BKGND
Command8=IDC_LIGHT_RECTANGLE
Command9=IDC_LIGHT_IMPROVED
Command10=IDC_LIGHT_RECTANGLE2
Command11=IDC_EMBOSS
Command12=IDC_GREY
Command13=IDC_BLUR
Command14=IDC_POSTERIZE
Command15=IDC_XOR
Command16=IDC_DARKEN
Command17=IDC_BLUE
Command18=IDC_GREEN
Command19=IDC_RED
Command20=IDM_NONE
Command21=IDM_TEST_INTERFACE
Command22=IDC_SAVE_GIF
Command23=IDC_TESTBALL
Command24=IDC_PLAYVIDEO
Command25=IDM_PLAYFILE
Command26=IDM_Test
Command27=IDC_LISTFILTERS
Command28=IDC_SNAPPIC
Command29=IDC_LISTFILTERBYGRAPH
Command30=IDM_PAUSE
CommandCount=30

[TB:IDR_TOOLBAR1]
Type=1
Class=?
Command1=ID_BUTTON32771
CommandCount=1

