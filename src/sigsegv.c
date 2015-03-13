/**
 * This source file is used to print out a stack-trace when your program
 * segfaults.  It is relatively reliable and spot-on accurate.
 *
 * This code is in the public domain.  Use it as you see fit, some credit
 * would be appreciated, but is not a prerequisite for usage.  Feedback
 * on it's use would encourage further development and maintenance.
 *
 * Author:  Jaco Kroon <jaco@kroon.co.za>
 *
 * Copyright (C) 2005 - 2008 Jaco Kroon
 *
 * Modifications made by H. Elwood Gilliland III - 2009
 * See doc/LICENSE.TXT for more information about what license this code is
 * distributed under.
 */

#define NO_CPP_DEMANGLE

#ifndef __USE_GNU
# define __USE_GNU
#endif

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <dlfcn.h>
#include <execinfo.h>
#ifndef NO_CPP_DEMANGLE
#include <cxxabi.h>
#endif

#if defined(REG_RIP)
# define SIGSEGV_STACK_IA64
# define REGFORMAT "%016lx"
#elif defined(REG_EIP)
# define SIGSEGV_STACK_X86
# define REGFORMAT "%08x"
#else
# define SIGSEGV_STACK_GENERIC
# define REGFORMAT "%x"
#endif

#include "nimud.h"

static void signal_segv(int signum, siginfo_t* info, void*ptr) {
    static const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};

    size_t i;
    ucontext_t *ucontext = (ucontext_t*)ptr;

#if defined(SIGSEGV_STACK_X86) || defined(SIGSEGV_STACK_IA64)
    int f = 0;
    Dl_info ddlinfo;
    void **bp = 0;
    void *ip = 0;
#else
    void *bt[20];
    char **strings;
    size_t sz;
#endif

    wtf_logf( "Segmentation Fault!\n");
    wtf_logf( "info.si_signo = %d\n", signum);
    wtf_logf( "info.si_errno = %d\n", info->si_errno);
    wtf_logf( "info.si_code  = %d (%s)\n", info->si_code, si_codes[info->si_code]);
    wtf_logf( "info.si_addr  = %p\n", info->si_addr);
    for(i = 0; i < NGREG; i++)
        wtf_logf( "reg[%02d]       = 0x" REGFORMAT "\n", i, ucontext->uc_mcontext.gregs[i]);

#if defined(SIGSEGV_STACK_X86) || defined(SIGSEGV_STACK_IA64)
# if defined(SIGSEGV_STACK_IA64)
    ip = (void*)ucontext->uc_mcontext.gregs[REG_RIP];
    bp = (void**)ucontext->uc_mcontext.gregs[REG_RBP];
# elif defined(SIGSEGV_STACK_X86)
    ip = (void*)ucontext->uc_mcontext.gregs[REG_EIP];
    bp = (void**)ucontext->uc_mcontext.gregs[REG_EBP];
# endif

    wtf_logf( "Stack trace:\n");
    while(bp && ip) {
        if(!dladdr(ip, &ddlinfo))
            break;

        const char *symname = ddlinfo.dli_sname;
#ifndef NO_CPP_DEMANGLE
        int status;
        char *tmp = __cxa_demangle(symname, NULL, 0, &status);

        if(status == 0 && tmp)
            symname = tmp;
#endif

        wtf_logf( "% 2d: %p <%s+%u> (%s)\n",
                ++f,
                ip,
                symname,
                (unsigned)(ip - ddlinfo.dli_saddr),
                ddlinfo.dli_fname);

#ifndef NO_CPP_DEMANGLE
        if(tmp)
            free(tmp);
#endif

        if(ddlinfo.dli_sname && !strcmp(ddlinfo.dli_sname, "main"))
            break;

        ip = bp[1];
        bp = (void**)bp[0];
    }
#else
    wtf_logf( "Stack trace (non-dedicated):\n");
    sz = backtrace(bt, 20);
    strings = backtrace_symbols(bt, sz);

    for(i = 0; i < sz; ++i)
        wtf_logf( "%s\n", strings[i]);
#endif
    wtf_logf( "End of stack trace\n");
    error_handler( SIGSEGV );
    exit (-1);
}

int setup_sigsegv() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = signal_segv;
    action.sa_flags = SA_SIGINFO;
    if(sigaction(SIGSEGV, &action, NULL) < 0) {
        perror("sigaction");
        return 0;
    }
    return 1;
}

#ifndef SIGSEGV_NO_AUTO_INIT
static void __attribute((constructor)) init(void) {
    setup_sigsegv();
}
#endif
