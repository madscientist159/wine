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
   addr->Mode    = AddrModeFlat;
   addr->Segment = 0; /* don't need segment */
   switch (ca)
    {
#if defined(__powerpc64__)
    case cpu_addr_pc:     addr->Offset = ctx->Iar; return TRUE;
    case cpu_addr_stack:  addr->Offset = ctx->Gpr1; return TRUE;
    case cpu_addr_frame:  addr->Offset = ctx->Fp; return TRUE;
#endif
    default:
        addr->Mode = -1;
        return FALSE;
    }
}

#if defined(__powerpc64__)
enum st_mode {stm_start, stm_ppc64, stm_done};

/* indexes in Reserved array */
#define __CurrentModeCount      0

#define curr_mode   (frame->Reserved[__CurrentModeCount] & 0x0F)
#define curr_count  (frame->Reserved[__CurrentModeCount] >> 4)

#define set_curr_mode(m) {frame->Reserved[__CurrentModeCount] &= ~0x0F; frame->Reserved[__CurrentModeCount] |= (m & 0x0F);}
#define inc_curr_count() (frame->Reserved[__CurrentModeCount] += 0x10)

/* fetch_next_frame()
 *
 * modify (at least) context.Iar using unwind information
 * either out of debug info (dwarf), or simple Lr trace
 */
static BOOL fetch_next_frame(struct cpu_stack_walk* csw, union ctx *pcontext,
    DWORD_PTR curr_pc)
{
    DWORD64 xframe;
    CONTEXT *context = &pcontext->ctx;
    DWORD_PTR               oldReturn = context->Lr;

    if (dwarf2_virtual_unwind(csw, curr_pc, pcontext, &xframe))
    {
        context->Gpr1 = xframe;
        context->Iar = oldReturn;
        return TRUE;
    }

    if (context->Iar == context->Lr) return FALSE;
    context->Iar = oldReturn;

    return TRUE;
}

static BOOL ppc64_stack_walk(struct cpu_stack_walk *csw, STACKFRAME64 *frame,
    union ctx *context)
{
    unsigned deltapc = curr_count <= 1 ? 0 : 4;

    /* sanity check */
    if (curr_mode >= stm_done) return FALSE;

    TRACE("Enter: PC=%s Frame=%s Return=%s Stack=%s Mode=%s Count=%s\n",
          wine_dbgstr_addr(&frame->AddrPC),
          wine_dbgstr_addr(&frame->AddrFrame),
          wine_dbgstr_addr(&frame->AddrReturn),
          wine_dbgstr_addr(&frame->AddrStack),
          curr_mode == stm_start ? "start" : "PPC64",
          wine_dbgstr_longlong(curr_count));

    if (curr_mode == stm_start)
    {
        /* Init done */
        set_curr_mode(stm_ppc64);
        frame->AddrReturn.Mode = frame->AddrStack.Mode = AddrModeFlat;
        /* don't set up AddrStack on first call. Either the caller has set it up, or
         * we will get it in the next frame
         */
        memset(&frame->AddrBStore, 0, sizeof(frame->AddrBStore));
    }
    else
    {
        if (context->ctx.Gpr1 != frame->AddrStack.Offset) FIXME("inconsistent Stack Pointer\n");
        if (context->ctx.Iar != frame->AddrPC.Offset) FIXME("inconsistent Program Counter\n");

        if (frame->AddrReturn.Offset == 0) goto done_err;
        if (!fetch_next_frame(csw, context, frame->AddrPC.Offset - deltapc))
            goto done_err;
    }

    memset(&frame->Params, 0, sizeof(frame->Params));

    /* set frame information */
    frame->AddrStack.Offset = context->ctx.Gpr1;
    frame->AddrReturn.Offset = context->ctx.Lr;
    frame->AddrFrame.Offset = context->ctx.Fp;
    frame->AddrPC.Offset = context->ctx.Iar;

    frame->Far = TRUE;
    frame->Virtual = TRUE;
    inc_curr_count();

    TRACE("Leave: PC=%s Frame=%s Return=%s Stack=%s Mode=%s Count=%s FuncTable=%p\n",
          wine_dbgstr_addr(&frame->AddrPC),
          wine_dbgstr_addr(&frame->AddrFrame),
          wine_dbgstr_addr(&frame->AddrReturn),
          wine_dbgstr_addr(&frame->AddrStack),
          curr_mode == stm_start ? "start" : "PPC64",
          wine_dbgstr_longlong(curr_count),
          frame->FuncTableEntry);

    return TRUE;
done_err:
    set_curr_mode(stm_done);
    return FALSE;
}
#else
static BOOL ppc64_stack_walk(struct cpu_stack_walk* csw, STACKFRAME64 *frame,
    union ctx *ctx)
{
    return FALSE;
}
#endif

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
