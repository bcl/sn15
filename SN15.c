/* -------------------------------------------------------------------------
   AR-15 Serial Numbers v2.0.0

   Copyright (c) 2001 Brian C. Lane <bcl@brianlane.com>
   All rights reserved.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

   ==========================[ HISTORY ]====================================
   04/21/2001   I found the source for the old version, luckily! Converting
                it to use the current pilot development enviornment under
                Linux.

		All text and menus are stored in the ar15.rcp file.
		A nice new feature would be the ability to embed pictures
		in the information in the .rsc file.

		Looks like I never had the scroll bar working, currently only
		the keys are used to scroll the information page and there is
		no indication of position. Need to add this.

		Scrollbars are now working, thanks to examples from the
		O'Reilly Palm OS programming book. I'll save pictures for the
		next version. I still need to update the Olympic arms info.

		Added new Olympic Arms info. 

		Trying to add an AR-15 logo to the startup screen. Something
		new to learn! Pretty simple if you use FORMBITMAP in the main
		form rsc description. Looks ugly, but it show up.


   03/07/2001	Started this program again.
   
   ------------------------------------------------------------------------- */

#include <Pilot.h>
#include "callback.h"
#include "SN15Rsc.h"


/*
  Index for the manufacturer selection list
  If new manufacturers or sections are added don't forget to update this
  list.

  NUM_MANUF is defined in SN15Rsc.h where the ids of these entries are
  also defined.
*/
int map[NUM_MANUF] = { 
  info_laws,
  info_AA,
  info_ASA,
  info_AE,
  info_Bohica,
  info_Bush,
  info_CA,
  info_Colt,
  info_Dalphon,
  info_DPMS,
  info_Eagle,
  info_EA,
  info_FA,
  info_HA,
  info_JLM,
  info_KAC,
  info_MM,
  info_Nesard,
  info_OA,
  info_PO,
  info_PWA,
  info_RMA,
  info_Sendra
};

/* ==========================================
   Global Variables
   ========================================== */
Handle            TextHandle,
                  resH;
BitmapType        *bitmap;
FieldPtr          InfoFieldP = NULL;


/* Update the scroll bar's position */
static void UpdateScrollbar(void)
{
   FormPtr        frm = FrmGetActiveForm();
   ScrollBarPtr   scroll;
   Word           currentPosition;
   Word           textHeight;
   Word           fieldHeight;
   Word           maxValue;
  
   FldGetScrollValues(InfoFieldP, &currentPosition, &textHeight, &fieldHeight);

   // if the field is 3 lines, and the text height is 4 lines
   // then we can scroll so that the first line is at the top
   // (scroll position 0) or so the second line is at the top
   // (scroll postion 1). These two values are enough to see
   // the entire text.
   if (textHeight > fieldHeight)
      maxValue = textHeight - fieldHeight;
   else if (currentPosition)
      maxValue = currentPosition;
   else
      maxValue = 0;
     
   scroll = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, scroll_info));
  
   // on a page scroll, want to overlap by one line (to provide context)
   SclSetScrollBar(scroll, currentPosition, 0, maxValue, fieldHeight - 1);
}


static void ScrollLines(int numLinesToScroll, Boolean redraw)
{
   FormPtr        frm = FrmGetActiveForm();
  
   if (numLinesToScroll < 0)
      FldScrollField(InfoFieldP, -numLinesToScroll, up);
   else
      FldScrollField(InfoFieldP, numLinesToScroll, down);
     
   // if there are blank lines at the end and we scroll up, FldScrollField
   // makes the blank lines disappear. Therefore, we've got to update
   // the scrollbar
   if ((FldGetNumberOfBlankLines(InfoFieldP) && numLinesToScroll < 0) || redraw)
      UpdateScrollbar();
}


static void PageScroll(DirectionType direction)
{
   FormPtr        frm = FrmGetActiveForm();

   if (FldScrollable(InfoFieldP, direction)) {
      int linesToScroll = FldGetVisibleLines(InfoFieldP) - 1;
     
      if (direction == up)
         linesToScroll = -linesToScroll;
      ScrollLines(linesToScroll, true);
   }
}


/* Handle events on the MainForm */
static Boolean MainFormHandleEvent (EventPtr e)
{
    Boolean handled = false;
    FormPtr frm;
    
    CALLBACK_PROLOGUE

    switch (e->eType)
    {
      /* Use the up/down keys to scroll the text info */
      case keyDownEvent:
	if (e->data.keyDown.chr == pageUpChr)
	  {
	    if ( InfoFieldP != NULL ){
	      PageScroll(up);
	      //	      FldScrollField(InfoFieldP,1,up);
	    }
	    handled = true;
	  }
	else if (e->data.keyDown.chr == pageDownChr)
	  {
	    if ( InfoFieldP != NULL )
	    {
	      PageScroll(down);
	      //	      FldScrollField(InfoFieldP,1,down);
	    }
	    handled = true;
	  }
	break;

      /* The info field has changed, update the scroll bars */
      case fldChangedEvent:
	{
	  UpdateScrollbar();
	  handled = true;
	}
	break;


      /* This is called while the scroll bar is being dragged */
      case sclRepeatEvent:
        {
	  ScrollLines(e->data.sclRepeat.newValue - e->data.sclRepeat.value, true);
        }
	break;


      case ctlSelectEvent:
	switch( e->data.ctlEnter.controlID )
	{
	  case HelloButton:
	  {
            FormPtr    currentForm = FrmGetFormPtr(FrmGetActiveFormID());
            Word       btnIndex    = FrmGetObjectIndex(currentForm, HelloButton);
            ControlPtr btn         =
                (ControlPtr)FrmGetObjectPtr(currentForm, btnIndex);

            FrmAlert(ClickMeAlert);
            CtlSetValue(btn, 0);
            handled = true;
	  }
	  break;
	}
	break;

      /* Handle selections from the popup menu of items, copy the info
	 Into the main form's text box
       */
      case popSelectEvent:
	switch( e->data.ctlEnter.controlID )
	{
          case MainPopup:
	    {
	      FormPtr currentForm = FrmGetFormPtr(FrmGetActiveFormID());
	      ListPtr lstp = FrmGetObjectPtr(currentForm,FrmGetObjectIndex(currentForm,MainList));

              /* Fill the field with the select text */
	      if( LstGetSelection(lstp) < NUM_MANUF )
	      {
//		WinDrawChars((CharPtr) "AB",2,1,140);
	  
		TextHandle = DmGetResource( strRsc, map[LstGetSelection(lstp)]);
		MemHandleLock( TextHandle );
		FldSetUsable(InfoFieldP,true);
		FldSetTextHandle( InfoFieldP, TextHandle);
		FldDrawField(InfoFieldP);
		MemHandleUnlock( TextHandle );
		DmReleaseResource( TextHandle );
		UpdateScrollbar();
	      }
	      handled = true;
	    }
	    break;
	}
	break;


    case frmOpenEvent:
        /* Get a pointer to the info field where we show the info text */
	InfoFieldP = FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),field_info));
        UpdateScrollbar();
        FrmDrawForm(FrmGetActiveForm());
//	WinDrawChars((CharPtr) "Try #02",7,1,120);

	//	resH = DmGetResource (bitmapRsc, AR15LOGO);
	//	bitmap = MemHandleLock (resH);
	//	WinPaintBitmap(bitmap, 2, 15);
	//	DmReleaseResource( resH );
        handled = true;
        break;

    case menuEvent:
        switch (e->data.menu.itemID)
        {
	/* Show the About box */
        case mit_About:
            FrmAlert(AboutAlert);
            handled = true;
        }
	break;

    default:

      break;
    }

    CALLBACK_EPILOGUE

    return handled;
}

static Boolean ApplicationHandleEvent(EventPtr e)
{
    FormPtr frm;
    Word    formId;
    Boolean handled = false;

    /* Setup the From's Handler when it is loaded */
    if (e->eType == frmLoadEvent) {
	formId = e->data.frmLoad.formID;
	frm = FrmInitForm(formId);
	FrmSetActiveForm(frm);

	switch(formId) {
	case MainForm:
	    FrmSetEventHandler(frm, MainFormHandleEvent);
	    break;
	}
	handled = true;
    }

    return handled;
}

/* Get preferences, open (or create) app database */
static Word StartApplication(void)
{
    FrmGotoForm(MainForm);
    return 0;
}

/* Save preferences, close forms, close app database */
static void StopApplication(void)
{
  /*    FrmSaveAllForms(); */
  /*    FrmCloseAllForms(); */
}

/* The main event loop */
static void EventLoop(void)
{
    Word err;
    EventType e;

    do {
	EvtGetEvent(&e, evtWaitForever);
	if (! SysHandleEvent (&e))
	    if (! MenuHandleEvent (NULL, &e, &err))
		if (! ApplicationHandleEvent (&e))
		    FrmDispatchEvent (&e);
    } while (e.eType != appStopEvent);
}

/* Main entry point; it is unlikely you will need to change this except to
   handle other launch command codes */
DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags)
{
    Word err;

    if (cmd == sysAppLaunchCmdNormalLaunch) {

	err = StartApplication();
	if (err) return err;

	EventLoop();
	StopApplication();

    } else {
	return sysErrParamErr;
    }

    return 0;
}
