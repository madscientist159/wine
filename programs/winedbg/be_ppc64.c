/*
 * Debugger Power PC 64 specific functions
 *
 * Copyright 2000-2003 Marcus Meissner
 *                2004 Eric Pouech
 *                2019 Timothy Pearson <tpearson@raptorengineering.com>
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

#include "debugger.h"

#if defined(__powerpc64__)

static BOOL be_ppc64_get_addr(HANDLE hThread, const dbg_ctx_t *ctx,
                            enum be_cpu_addr bca, ADDRESS64* addr)
{
    switch (bca)
    {
    case be_cpu_addr_pc:
        return be_cpu_build_addr(hThread, ctx, addr, 0, ctx->ctx.Iar);
    case be_cpu_addr_stack:
        return be_cpu_build_addr(hThread, ctx, addr, 0, ctx->ctx.u.s.Gpr1);
    case be_cpu_addr_frame:
        return be_cpu_build_addr(hThread, ctx, addr, 0, ctx->ctx.u.s.Fp);
    default:
        return FALSE;
    }
}

static BOOL be_ppc64_get_register_info(int regno, enum be_cpu_addr* kind)
{
    switch (regno)
    {
    case CV_PPC64_PC:      *kind = be_cpu_addr_pc;    return TRUE;
    case CV_PPC64_SP:      *kind = be_cpu_addr_stack; return TRUE;
    case CV_PPC64_FP:      *kind = be_cpu_addr_frame; return TRUE;
    }
    return FALSE;
}

static void be_ppc64_single_step(dbg_ctx_t *ctx, BOOL enable)
{
#ifndef MSR_SE
# define MSR_SE (1<<10)
#endif
    if (enable) ctx->ctx.Msr |= MSR_SE;
    else ctx->ctx.Msr &= ~MSR_SE;
}

static void be_ppc64_print_context(HANDLE hThread, const dbg_ctx_t *ctx, int all_regs)
{
    dbg_printf("Register dump:\n");

    dbg_printf(" Pc:%016lx Sp:%016lx Lr:%016lx Cr:%016lx\n",
               ctx->ctx.Iar, ctx->ctx.u.s.Gpr1, ctx->ctx.Lr, ctx->ctx.u.s.Cr);
    dbg_printf(" gpr0: %016lx gpr1: %016lx gpr2: %016lx gpr3: %016lx gpr4: %016lx\n",
               ctx->ctx.u.s.Gpr0, ctx->ctx.u.s.Gpr1, ctx->ctx.u.s.Gpr2, ctx->ctx.u.s.Gpr3, ctx->ctx.u.s.Gpr4);
    dbg_printf(" gpr5: %016lx gpr6: %016lx gpr7: %016lx gpr8: %016lx gpr9: %016lx\n",
               ctx->ctx.u.s.Gpr5, ctx->ctx.u.s.Gpr6, ctx->ctx.u.s.Gpr7, ctx->ctx.u.s.Gpr8, ctx->ctx.u.s.Gpr9);
    dbg_printf(" gpr10:%016lx gpr11:%016lx gpr12:%016lx gpr13:%016lx gpr14:%016lx\n",
               ctx->ctx.u.s.Gpr10, ctx->ctx.u.s.Gpr11, ctx->ctx.u.s.Gpr12, ctx->ctx.u.s.Gpr13, ctx->ctx.u.s.Gpr14);
    dbg_printf(" gpr15:%016lx ip0:%016lx ip1:%016lx gpr18:%016lx gpr19:%016lx\n",
               ctx->ctx.u.s.Gpr15, ctx->ctx.u.s.Gpr16, ctx->ctx.u.s.Gpr17, ctx->ctx.u.s.Gpr18, ctx->ctx.u.s.Gpr19);
    dbg_printf(" gpr20:%016lx gpr21:%016lx gpr22:%016lx gpr23:%016lx gpr24:%016lx\n",
               ctx->ctx.u.s.Gpr20, ctx->ctx.u.s.Gpr21, ctx->ctx.u.s.Gpr22, ctx->ctx.u.s.Gpr23, ctx->ctx.u.s.Gpr24);
    dbg_printf(" gpr25:%016lx gpr26:%016lx gpr27:%016lx gpr28:%016lx gpr29:%016lx\n",
               ctx->ctx.u.s.Gpr25, ctx->ctx.u.s.Gpr26, ctx->ctx.u.s.Gpr27, ctx->ctx.u.s.Gpr28, ctx->ctx.u.s.Gpr29);
    dbg_printf(" gpr30:%016lx gpr31:%016lx Fp:%016lx\n",
               ctx->ctx.u.s.Gpr30, ctx->ctx.u.s.Gpr31, ctx->ctx.u.s.Fp);

    if (all_regs) dbg_printf( "Floating point PPC64 dump not implemented\n" );
}

static void be_ppc64_print_segment_info(HANDLE hThread, const dbg_ctx_t *ctx)
{
}

static struct dbg_internal_var be_ppc64_ctx[] =
{
    {CV_PPC64_GPR0 +  0,    "gpr0",   (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr0),  dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  1,    "gpr1",   (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr1),  dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  2,    "gpr2",   (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr2),  dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  3,    "gpr3",   (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr3),  dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  4,    "gpr4",   (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr4),  dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  5,    "gpr5",   (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr5),  dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  6,    "gpr6",   (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr6),  dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  7,    "gpr7",   (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr7),  dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  8,    "gpr8",   (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr8),  dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  9,    "gpr9",   (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr9),  dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  10,   "gpr10",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr10), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  11,   "gpr11",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr11), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  12,   "gpr12",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr12), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  13,   "gpr13",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr13), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  14,   "gpr14",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr14), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  15,   "gpr15",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr15), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  16,   "gpr16",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr16), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  17,   "gpr17",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr17), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  18,   "gpr18",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr18), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  19,   "gpr19",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr19), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  20,   "gpr20",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr20), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  21,   "gpr21",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr21), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  22,   "gpr22",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr22), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  23,   "gpr23",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr23), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  24,   "gpr24",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr24), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  25,   "gpr25",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr25), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  26,   "gpr26",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr26), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  27,   "gpr27",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr27), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  28,   "gpr28",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr28), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  29,   "gpr29",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr29), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  30,   "gpr30",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr30), dbg_itype_unsigned_long_int},
    {CV_PPC64_GPR0 +  31,   "gpr31",  (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr31), dbg_itype_unsigned_long_int},
    {CV_PPC64_FP,           "fp",     (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Fp),    dbg_itype_unsigned_long_int},
    {CV_PPC64_LR,           "lr",     (DWORD_PTR*)FIELD_OFFSET(CONTEXT, Lr),        dbg_itype_unsigned_long_int},
    {CV_PPC64_SP,           "sp",     (DWORD_PTR*)FIELD_OFFSET(CONTEXT, u.s.Gpr1),  dbg_itype_unsigned_long_int},
    {CV_PPC64_PC,           "pc",     (DWORD_PTR*)FIELD_OFFSET(CONTEXT, Iar),       dbg_itype_unsigned_long_int},
    {0,                     NULL,     0,                                            dbg_itype_none}
};

static BOOL be_ppc64_is_step_over_insn(const void* insn)
{
    dbg_printf("not done\n");
    return FALSE;
}

static BOOL be_ppc64_is_function_return(const void* insn)
{
    dbg_printf("not done\n");
    return FALSE;
}

static BOOL be_ppc64_is_break_insn(const void* insn)
{
    dbg_printf("not done\n");
    return FALSE;
}

static BOOL be_ppc64_is_func_call(const void* insn, ADDRESS64* callee)
{
    return FALSE;
}

static BOOL be_ppc64_is_jump(const void* insn, ADDRESS64* jumpee)
{
    return FALSE;
}

static void be_ppc64_disasm_one_insn(ADDRESS64* addr, int display)

{
    dbg_printf("be_ppc64_disasm_one_insn: not done\n");
}

static BOOL be_ppc64_insert_Xpoint(HANDLE hProcess, const struct be_process_io* pio,
                                 dbg_ctx_t *ctx, enum be_xpoint_type type,
                                 void* addr, unsigned long* val, unsigned size)
{
    unsigned long       xbp;
    SIZE_T              sz;

    switch (type)
    {
    case be_xpoint_break:
        if (!size) return FALSE;
        if (!pio->read(hProcess, addr, val, 4, &sz) || sz != 4) return FALSE;
        xbp = 0x7d821008; /* 7d 82 10 08 ... in big endian */
        if (!pio->write(hProcess, addr, &xbp, 4, &sz) || sz != 4) return FALSE;
        break;
    default:
        dbg_printf("Unknown/unsupported bp type %c\n", type);
        return FALSE;
    }
    return TRUE;
}

static BOOL be_ppc64_remove_Xpoint(HANDLE hProcess, const struct be_process_io* pio,
                                 dbg_ctx_t *ctx, enum be_xpoint_type type,
                                 void* addr, unsigned long val, unsigned size)
{
    SIZE_T              sz;

    switch (type)
    {
    case be_xpoint_break:
        if (!size) return FALSE;
        if (!pio->write(hProcess, addr, &val, 4, &sz) || sz == 4) return FALSE;
        break;
    default:
        dbg_printf("Unknown/unsupported bp type %c\n", type);
        return FALSE;
    }
    return TRUE;
}

static BOOL be_ppc64_is_watchpoint_set(const dbg_ctx_t *ctx, unsigned idx)
{
    dbg_printf("not done\n");
    return FALSE;
}

static void be_ppc64_clear_watchpoint(dbg_ctx_t *ctx, unsigned idx)
{
    dbg_printf("not done\n");
}

static int be_ppc64_adjust_pc_for_break(dbg_ctx_t *ctx, BOOL way)
{
    if (way)
    {
        ctx->ctx.Iar -= 4;
        return -4;
    }
    ctx->ctx.Iar += 4;
    return 4;
}

static BOOL be_ppc64_fetch_integer(const struct dbg_lvalue* lvalue, unsigned size,
                                 BOOL is_signed, LONGLONG* ret)
{
    if (size != 1 && size != 2 && size != 4 && size != 8) return FALSE;

    memset(ret, 0, sizeof(*ret)); /* clear unread bytes */
    /* FIXME: this assumes that debuggee and debugger use the same
     * integral representation
     */
    if (!memory_read_value(lvalue, size, ret)) return FALSE;

    /* propagate sign information */
    if (is_signed && size < 8 && (*ret >> (size * 8 - 1)) != 0)
    {
        ULONGLONG neg = -1;
        *ret |= neg << (size * 8);
    }
    return TRUE;
}

static BOOL be_ppc64_fetch_float(const struct dbg_lvalue* lvalue, unsigned size,
                               long double* ret)
{
    dbg_printf("not done\n");
    return FALSE;
}

static BOOL be_ppc64_store_integer(const struct dbg_lvalue* lvalue, unsigned size,
                                 BOOL is_signed, LONGLONG val)
{
    /* this is simple if we're on a little endian CPU */
    return memory_write_value(lvalue, size, &val);
}

static BOOL be_ppc64_get_context(HANDLE thread, dbg_ctx_t *ctx)
{
    ctx->ctx.ContextFlags = CONTEXT_ALL;
    return GetThreadContext(thread, &ctx->ctx);
}

static BOOL be_ppc64_set_context(HANDLE thread, const dbg_ctx_t *ctx)
{
    return SetThreadContext(thread, &ctx->ctx);
}

#define REG(r,gs)  {FIELD_OFFSET(CONTEXT, r), sizeof(((CONTEXT*)NULL)->r), gs}

static struct gdb_register be_ppc64_gdb_register_map[] = {
    REG(u.s.Gpr0,  8),
    REG(u.s.Gpr1,  8),
    REG(u.s.Gpr2,  8),
    REG(u.s.Gpr3,  8),
    REG(u.s.Gpr4,  8),
    REG(u.s.Gpr5,  8),
    REG(u.s.Gpr6,  8),
    REG(u.s.Gpr7,  8),
    REG(u.s.Gpr8,  8),
    REG(u.s.Gpr9,  8),
    REG(u.s.Gpr10, 8),
    REG(u.s.Gpr11, 8),
    REG(u.s.Gpr12, 8),
    REG(u.s.Gpr13, 8),
    REG(u.s.Gpr14, 8),
    REG(u.s.Gpr15, 8),
    REG(u.s.Gpr16, 8),
    REG(u.s.Gpr17, 8),
    REG(u.s.Gpr18, 8),
    REG(u.s.Gpr19, 8),
    REG(u.s.Gpr20, 8),
    REG(u.s.Gpr21, 8),
    REG(u.s.Gpr22, 8),
    REG(u.s.Gpr23, 8),
    REG(u.s.Gpr24, 8),
    REG(u.s.Gpr25, 8),
    REG(u.s.Gpr26, 8),
    REG(u.s.Gpr27, 8),
    REG(u.s.Gpr28, 8),
    REG(u.s.Gpr29, 8),
    REG(u.s.Gpr30, 8),
    REG(u.s.Gpr31, 8),
    REG(Fpr0,  4),
    REG(Fpr1,  4),
    REG(Fpr2,  4),
    REG(Fpr3,  4),
    REG(Fpr4,  4),
    REG(Fpr5,  4),
    REG(Fpr6,  4),
    REG(Fpr7,  4),
    REG(Fpr8,  4),
    REG(Fpr9,  4),
    REG(Fpr10, 4),
    REG(Fpr11, 4),
    REG(Fpr12, 4),
    REG(Fpr13, 4),
    REG(Fpr14, 4),
    REG(Fpr15, 4),
    REG(Fpr16, 4),
    REG(Fpr17, 4),
    REG(Fpr18, 4),
    REG(Fpr19, 4),
    REG(Fpr20, 4),
    REG(Fpr21, 4),
    REG(Fpr22, 4),
    REG(Fpr23, 4),
    REG(Fpr24, 4),
    REG(Fpr25, 4),
    REG(Fpr26, 4),
    REG(Fpr27, 4),
    REG(Fpr28, 4),
    REG(Fpr29, 4),
    REG(Fpr30, 4),
    REG(Fpr31, 4),

    REG(Iar,     8),
    REG(Msr,     8),
    REG(u.s.Cr,  8),
    REG(Lr,      8),
    REG(Ctr,     8),
    REG(u.s.Xer, 8),
    /* FIXME: MQ is missing? FIELD_OFFSET(CONTEXT, Mq), */
    /* see gdb/nlm/ppc.c */
};

struct backend_cpu be_ppc64 =
{
    IMAGE_FILE_MACHINE_POWERPC64,
    8,
    be_cpu_linearize,
    be_cpu_build_addr,
    be_ppc64_get_addr,
    be_ppc64_get_register_info,
    be_ppc64_single_step,
    be_ppc64_print_context,
    be_ppc64_print_segment_info,
    be_ppc64_ctx,
    be_ppc64_is_step_over_insn,
    be_ppc64_is_function_return,
    be_ppc64_is_break_insn,
    be_ppc64_is_func_call,
    be_ppc64_is_jump,
    be_ppc64_disasm_one_insn,
    be_ppc64_insert_Xpoint,
    be_ppc64_remove_Xpoint,
    be_ppc64_is_watchpoint_set,
    be_ppc64_clear_watchpoint,
    be_ppc64_adjust_pc_for_break,
    be_ppc64_fetch_integer,
    be_ppc64_fetch_float,
    be_ppc64_store_integer,
    be_ppc64_get_context,
    be_ppc64_set_context,
    be_ppc64_gdb_register_map,
    ARRAY_SIZE(be_ppc64_gdb_register_map),
};
#endif
