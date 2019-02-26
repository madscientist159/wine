/*
 * File cpu_ppc64.c
 *
 * Copyright (C) 2009-2009, Eric Pouech.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <assert.h>

#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "dbghelp_private.h"
#include "winternl.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(dbghelp);

static BOOL ppc64_get_addr(HANDLE hThread, const CONTEXT* ctx,
                         enum cpu_addr ca, ADDRESS64* addr)
{
   switch (ca)
    {
#if defined(__powerpc64__)
    case cpu_addr_pc:
        addr->Mode    = AddrModeFlat;
        addr->Segment = 0; /* don't need segment */
        addr->Offset  = ctx->Iar;
        return TRUE;
#endif
    default:
    case cpu_addr_stack:
    case cpu_addr_frame:
        FIXME("not done\n");
    }
    return FALSE;
}

static BOOL ppc64_stack_walk(struct cpu_stack_walk* csw, STACKFRAME64 *frame,
    union ctx *ctx)
{
    FIXME("not done\n");
    return FALSE;
}

static unsigned ppc64_map_dwarf_register(unsigned regno, BOOL eh_frame)
{
    FIXME("not done\n");
    return 0;
}

static void *ppc64_fetch_context_reg(union ctx *ctx, unsigned regno, unsigned *size)
{
    FIXME("NIY\n");
    return NULL;
}

static const char* ppc64_fetch_regname(unsigned regno)
{
    FIXME("Unknown register %x\n", regno);
    return NULL;
}

static BOOL ppc64_fetch_minidump_thread(struct dump_context* dc, unsigned index, unsigned flags, const CONTEXT* ctx)
{
    FIXME("NIY\n");
    return FALSE;
}

static BOOL ppc64_fetch_minidump_module(struct dump_context* dc, unsigned index, unsigned flags)
{
    FIXME("NIY\n");
    return FALSE;
}

DECLSPEC_HIDDEN struct cpu cpu_ppc64 = {
    IMAGE_FILE_MACHINE_POWERPC64,
    8,
    CV_REG_NONE, /* FIXME */
    ppc64_get_addr,
    ppc64_stack_walk,
    NULL,
    ppc64_map_dwarf_register,
    ppc64_fetch_context_reg,
    ppc64_fetch_regname,
    ppc64_fetch_minidump_thread,
    ppc64_fetch_minidump_module,
};
