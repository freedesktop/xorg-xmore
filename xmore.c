/*
 * $Xorg: xmore.c,v 1.1 2004/04/30 02:05:54 gisburn Exp $
 *
Copyright 2004 Roland Mainz <roland.mainz@nrubsig.org>

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 */

/* Force ANSI C prototypes from X11 headers */
#ifndef FUNCPROTO 
#define FUNCPROTO 15
#endif /* !FUNCPROTO */

#include <X11/StringDefs.h> 
#include <X11/Intrinsic.h> 
#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h> 

#include "xmore.h"
#include "printdialog.h"
#include "print.h"

#include <stdlib.h>
#include <stdio.h>

/* Turn a NULL pointer string into an empty string */
#define NULLSTR(x) (((x)!=NULL)?(x):(""))

#define Error(x) { printf x ; exit(EXIT_FAILURE); }
#define Assertion(expr, msg) { if (!(expr)) { Error msg } }
#define Log(x)   { if(True) printf x; }

/* Global vars */
XtAppContext  app;
Widget        printdialog_shell = NULL;
Widget        printdialog       = NULL;
Widget        toplevel;
const char   *ProgramName;  /* program name (from argv[0]) */
const char   *viewFileName; /* file to browse (from argv[1]) */
char          printJobNameBuffer[1024];

static void
quitXtProc(Widget w, XtPointer client_data, XtPointer callData)
{
    XtAppSetExitFlag(app);
}

static void
printOKXtProc(Widget w, XtPointer client_data, XtPointer callData)
{
    XawPrintDialogCallbackStruct *pdcs = (XawPrintDialogCallbackStruct *)callData;

    Log(("printOKXtProc: OK.\n"));
    
    /* ||printJobNameBuffer| must live as long the print job prints
     * because it is used for the job title AND the page headers... */
    sprintf(printJobNameBuffer, "XMore print job %s", viewFileName);

    DoPrint(toplevel, pdcs->pdpy, pdcs->pcontext,
            printJobNameBuffer,
            pdcs->printToFile?pdcs->printToFileName:NULL);

    XtPopdown(printdialog_shell);
}

static void
printCancelXtProc(Widget w, XtPointer client_data, XtPointer callData)
{
    Log(("printCancelXtProc: cancel.\n"));
    XtPopdown(printdialog_shell);
    
    Log(("destroying print dialog shell...\n"));
    XtDestroyWidget(printdialog_shell);
    printdialog_shell = NULL;
    printdialog       = NULL;
    Log(("... done\n"));
}

static void
printXtProc(Widget w, XtPointer client_data, XtPointer callData)
{
  Dimension   width, height;
  Position	x, y;
  Widget       parent = toplevel;
  puts("print!");
  
  if (!printdialog) {
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XtNallowShellResize, True); n++;
    printdialog_shell = XtCreatePopupShell("shell",
                                           transientShellWidgetClass,
                                           toplevel, args, n);
    n = 0;
    printdialog = XtCreateManagedWidget("printdialog", printDialogWidgetClass,
                                        printdialog_shell, args, n);
    XtAddCallback(printdialog, XawNOkCallback,     printOKXtProc,     NULL);
    XtAddCallback(printdialog, XawNCancelCallback, printCancelXtProc, NULL);

    XtRealizeWidget(printdialog_shell);
  }

  /* Center dialog */
  XtVaGetValues(printdialog_shell,
      XtNwidth,  &width,
      XtNheight, &height,
      NULL);

  x = (Position)(XWidthOfScreen( XtScreen(parent)) - width)  / 2;
  y = (Position)(XHeightOfScreen(XtScreen(parent)) - height) / 3;

  XtVaSetValues(printdialog_shell,
      XtNx, x,
      XtNy, y,
      NULL);
        
  XtPopup(printdialog_shell, XtGrabNonexclusive);
}


int main( int argc, char *argv[] )
{
  Widget       form;
  Widget       text;
  Widget       printbutton;
  Widget       quitbutton;
  int          n;
  Arg          args[8];

  ProgramName = argv[0];

  toplevel = XtAppInitialize(&app, "XMore", NULL, 0, &argc, argv, NULL, NULL, 0);

  if (argc != 2)
  {
    printf("usage: %s [ x options ] filename\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  viewFileName = argv[1];

  form = XtCreateManagedWidget("form", formWidgetClass, toplevel, NULL, 0);

  n = 0;
  XtSetArg(args[n], XtNtype,             XawAsciiFile);            n++;
  XtSetArg(args[n], XtNstring,           viewFileName);            n++;
  XtSetArg(args[n], XtNwidth,            500);                     n++;
  XtSetArg(args[n], XtNheight,           600);                     n++;
  XtSetArg(args[n], XtNscrollHorizontal, XawtextScrollWhenNeeded); n++;
  XtSetArg(args[n], XtNscrollVertical,   XawtextScrollAlways);     n++;
  text = XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, n);

  n = 0;
  XtSetArg(args[n], XtNfromHoriz,       NULL);            n++;
  XtSetArg(args[n], XtNfromVert,        text);            n++;
  XtSetArg(args[n], XtNlabel,           "Print...");      n++;
  printbutton = XtCreateManagedWidget("print", commandWidgetClass, form, args, n);
  XtAddCallback(printbutton, XtNcallback, printXtProc, 0);        

  n = 0;
  XtSetArg(args[n], XtNfromHoriz,       printbutton);            n++;
  XtSetArg(args[n], XtNfromVert,        text);                   n++;
  XtSetArg(args[n], XtNlabel,           "Quit");      n++;
  quitbutton = XtCreateManagedWidget("quit", commandWidgetClass, form, args, n);
  XtAddCallback(quitbutton, XtNcallback, quitXtProc, 0);
  
  printdialog_shell = NULL;
  printdialog       = NULL;

  XtRealizeWidget(toplevel);
  
  XtAppMainLoop(app);

  return EXIT_SUCCESS;
}
         
