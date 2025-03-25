/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  Orderer.cpp
 *  @brief: Startup Tcl interpreter with the EventBuilder package loaded.
 *
 */



/**
 * The reason for this is to allow profiling of the
 * event builder compiled bits and pieces.  Supporiting that
 * requires a profiled main (and we'll find out what else as well).
 */


// The rest of this is just a butchered tclApppInit.c from the debian source
// pkg for tcl8.6

/*
 * tclAppInit.c --
 *
 *	Provides a default version of the main program and Tcl_AppInit
 *	procedure for tclsh and other Tcl-based applications (without Tk).
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * Copyright (c) 1994-1997 Sun Microsystems, Inc.
 * Copyright (c) 1998-1999 Scriptics Corporation.
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#undef BUILD_tcl
#undef STATIC_BUILD
#include "tcl.h"
#include <TCLApplication.h>
#include <sys/types.h>
#include <sys/gmon.h>



extern "C" {
    int Eventbuilder_Init(Tcl_Interp* pInterp);
    void _start(void);
    void etext(void);
}


#ifdef TCL_TEST
extern Tcl_PackageInitProc Tcltest_Init;
extern Tcl_PackageInitProc Tcltest_SafeInit;
#endif /* TCL_TEST */

#ifdef TCL_XT_TEST
extern void                XtToolkitInitialize(void);
extern Tcl_PackageInitProc Tclxttest_Init;
#endif /* TCL_XT_TEST */

/*
 * The following #if block allows you to change the AppInit function by using
 * a #define of TCL_LOCAL_APPINIT instead of rewriting this entire file. The
 * #if checks for that #define and uses Tcl_AppInit if it does not exist.
 */

#ifndef TCL_LOCAL_APPINIT
#define TCL_LOCAL_APPINIT Tcl_AppInit
#endif
#ifndef MODULE_SCOPE
#   define MODULE_SCOPE extern
#endif
MODULE_SCOPE int TCL_LOCAL_APPINIT(Tcl_Interp *);
MODULE_SCOPE int main(int, char **);

/*
 * The following #if block allows you to change how Tcl finds the startup
 * script, prime the library or encoding paths, fiddle with the argv, etc.,
 * without needing to rewrite Tcl_Main()
 */

#ifdef TCL_LOCAL_MAIN_HOOK
MODULE_SCOPE int TCL_LOCAL_MAIN_HOOK(int *argc, char ***argv);
#endif

/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	This is the main program for the application.
 *
 * Results:
 *	None: Tcl_Main never returns here, so this procedure never returns
 *	either.
 *
 * Side effects:
 *	Just about anything, since from here we call arbitrary Tcl code.
 *
 *----------------------------------------------------------------------
 */

extern "C" {
int
main(
    int argc,			/* Number of command-line arguments. */
    char *argv[])		/* Values of command-line arguments. */
{
#ifdef TCL_XT_TEST
    XtToolkitInitialize();
#endif

#ifdef TCL_LOCAL_MAIN_HOOK
    TCL_LOCAL_MAIN_HOOK(&argc, &argv);
#endif

    Tcl_Main(argc, argv, TCL_LOCAL_APPINIT);
    return 0;			/* Needed only to prevent compiler warning. */
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *	This procedure performs application-specific initialization. Most
 *	applications, especially those that incorporate additional packages,
 *	will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error message in
 *	the interp's result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_AppInit(
    Tcl_Interp *interp)		/* Interpreter for application. */
{
    if ((Tcl_Init)(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

#ifdef TCL_XT_TEST
    if (Tclxttest_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
#endif

#ifdef TCL_TEST
    if (Tcltest_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "Tcltest", Tcltest_Init, Tcltest_SafeInit);
#endif /* TCL_TEST */

    // Provide the orderer:
    
    Tcl_StaticPackage(
        interp, "EventBuilder", Eventbuilder_Init, Eventbuilder_Init
    );

    if (Eventbuilder_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    

    /*
     * Call the init procedures for included packages. Each call should look
     * like this:
     *
     * if (Mod_Init(interp) == TCL_ERROR) {
     *     return TCL_ERROR;
     * }
     *
     * where "Mod" is the name of the module. (Dynamically-loadable packages
     * should have the same entry-point name.)
     */

    /*
     * Call Tcl_CreateCommand for application-specific commands, if they
     * weren't already created by the init procedures called above.
     */

    /*
     * Specify a user-specific startup file to invoke if the application is
     * run interactively. Typically the startup file is "~/.apprc" where "app"
     * is the name of the application. If this line is deleted then no
     * user-specific startup file will be run under any conditions.
     */

#ifdef DJGPP
    (Tcl_ObjSetVar2)(interp, Tcl_NewStringObj("tcl_rcFileName", -1), NULL,
	    Tcl_NewStringObj("~/tclsh.rc", -1), TCL_GLOBAL_ONLY);
#else
    (Tcl_ObjSetVar2)(interp, Tcl_NewStringObj("tcl_rcFileName", -1), NULL,
	    Tcl_NewStringObj("~/.tclshrc", -1), TCL_GLOBAL_ONLY);
#endif

    monstartup((u_long)&_start, (u_long)&etext);
    return TCL_OK;
}
}
#ifdef GPROFING
extern "C" {

// Shamelessly stolen from :
// http://sam.zoy.org/writings/programming/gprof.html

/* gprof-helper.c -- preload library to profile pthread-enabled programs 
 * 
 * Authors: Sam Hocevar <sam at zoy dot org> 
 *          Daniel Jönsson <danieljo at fagotten dot org> 
 * 
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the Do What The Fuck You Want To 
 *  Public License as published by Banlu Kemiyatorn. See 
 *  http://sam.zoy.org/projects/COPYING.WTFPL for more details. 
 * 
 * Compilation example: 
 * gcc -shared -fPIC gprof-helper.c -o gprof-helper.so -lpthread -ldl 
 * 
 * Usage example: 
 * LD_PRELOAD=./gprof-helper.so your_program 
 */

 // Apologies to politically sensitive but
 // Here's the license:
 
 /*
  *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.

*/

#ifndef _GNU_SOURCE

#define _GNU_SOURCE
#endif
#include <sys/time.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <dlfcn.h> 
#include <pthread.h> 

static void * wrapper_routine(void *); 

/* Original pthread function */ 
static int (*pthread_create_orig)(pthread_t *__restrict, 
                                  __const pthread_attr_t *__restrict, 
                                  void *(*)(void *), 
                                  void *__restrict) = NULL; 

/* Library initialization function */ 
void wooinit(void) __attribute__((constructor)); 

void wooinit(void) 
{ 
    pthread_create_orig = reinterpret_cast<int (*)(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*) >
        (dlsym(RTLD_NEXT, "pthread_create")); 
    fprintf(stderr, "pthreads: using profiling hooks for gprof\n"); 
    if(pthread_create_orig == NULL) 
    { 
        const char *error = dlerror(); 
        if(error == NULL) 
        { 
            error = "pthread_create is NULL"; 
        } 
        fprintf(stderr, "%s\n", error); 
        exit(EXIT_FAILURE); 
    } 
} 

/* Our data structure passed to the wrapper */ 
typedef struct wrapper_s 
{ 
    void * (*start_routine)(void *); 
    void * arg; 

    pthread_mutex_t lock; 
    pthread_cond_t  wait; 

    struct itimerval itimer; 

} wrapper_t; 

/* The wrapper function in charge for setting the itimer value */ 
static void * wrapper_routine(void * data) 
{ 
    /* Put user data in thread-local variables */ 
    void * (*start_routine)(void *) = ((wrapper_t*)data)->start_routine; 
    void * arg = ((wrapper_t*)data)->arg; 

    /* Set the profile timer value */ 
    setitimer(ITIMER_PROF, &((wrapper_t*)data)->itimer, NULL); 

    /* Tell the calling thread that we don't need its data anymore */ 
    pthread_mutex_lock(&((wrapper_t*)data)->lock); 
    pthread_cond_signal(&((wrapper_t*)data)->wait); 
    pthread_mutex_unlock(&((wrapper_t*)data)->lock); 

    /* Call the real function */ 
    return start_routine(arg); 
} 

/* Our wrapper function for the real pthread_create() */ 
int pthread_create(pthread_t *__restrict thread, 
                   __const pthread_attr_t *__restrict attr, 
                   void * (*start_routine)(void *), 
                   void *__restrict arg) 
{ 
    wrapper_t wrapper_data; 
    int i_return; 

    /* Initialize the wrapper structure */ 
    wrapper_data.start_routine = start_routine; 
    wrapper_data.arg = arg; 
    getitimer(ITIMER_PROF, &wrapper_data.itimer); 
    pthread_cond_init(&wrapper_data.wait, NULL); 
    pthread_mutex_init(&wrapper_data.lock, NULL); 
    pthread_mutex_lock(&wrapper_data.lock); 

    /* The real pthread_create call */ 
    i_return = pthread_create_orig(thread, 
                                   attr, 
                                   &wrapper_routine, 
                                   &wrapper_data); 

    /* If the thread was successfully spawned, wait for the data 
     * to be released */ 
    if(i_return == 0) 
    { 
        pthread_cond_wait(&wrapper_data.wait, &wrapper_data.lock); 
    } 

    pthread_mutex_unlock(&wrapper_data.lock); 
    pthread_mutex_destroy(&wrapper_data.lock); 
    pthread_cond_destroy(&wrapper_data.wait); 

    return i_return; 
} 

}
#endif
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
