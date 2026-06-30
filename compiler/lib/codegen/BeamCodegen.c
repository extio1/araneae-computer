#include "araneae/codegen/BeamCodegen.h"
#include "araneae/aux/Aux.h"
#include "araneae/aux/CodeBuffer.h"
#include "araneae/cfg/Cfg.h"
#include "araneae/cfg/Class.h"
#include "araneae/cfg/Dag.h"
#include "araneae/cfg/Function.h"
#include "araneae/cfg/Module.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── limits ─────────────────────────────────────────────────────────────── */
#define MAX_VARS   64
#define MAX_BB_ID  8192
#define MAX_FUNCS  256

/* ── context ─────────────────────────────────────────────────────────────── */
typedef struct {
    CodeBuffer *Code;

    /* global label counter (1-based) */
    int  NextLabel;
    int  TotalLabels; /* filled after pre-pass */

    const char   *ModName;
    struct Module *Mod;

    /* per-function variable → y-register table */
    char *VarNames[MAX_VARS];
    int   VarCount;
    int   NumYRegs;
    int   NumArgs;

    /* BB id → beam label (filled during pre-pass) */
    int BBLabel[MAX_BB_ID];

    /* per-function entry/func-info labels, indexed by function index */
    int FuncInfoLabel[MAX_FUNCS];
    int FuncEntryLabel[MAX_FUNCS];
    int FuncCount;

    /* for DFS traversal */
    struct basic_block_l *VisitedBBs;
} BeamCtx;

/* ── helpers ──────────────────────────────────────────────────────────────── */

static BeamCtx *beamCtxCreate(const char *ModName, struct Module *Mod) {
    BeamCtx *C = calloc(1, sizeof(BeamCtx));
    C->Code    = codeBufferCreate();
    C->NextLabel = 1;
    C->ModName   = ModName;
    C->Mod       = Mod;
    memset(C->BBLabel, -1, sizeof(C->BBLabel));
    return C;
}

static void beamCtxFree(BeamCtx *C) {
    for (int i = 0; i < C->VarCount; i++) free(C->VarNames[i]);
    if (C->VisitedBBs) bbl_free(C->VisitedBBs);
    codeBufferFree(C->Code);
    free(C);
}

static int allocLabel(BeamCtx *C) { return C->NextLabel++; }

/* ── variable table ──────────────────────────────────────────────────────── */

static int getVarY(BeamCtx *C, const char *Name) {
    for (int i = 0; i < C->VarCount; i++)
        if (strcmp(C->VarNames[i], Name) == 0) return i;
    return -1;
}

static int addVar(BeamCtx *C, const char *Name) {
    int i = getVarY(C, Name);
    if (i >= 0) return i;
    assert(C->VarCount < MAX_VARS && "too many variables");
    C->VarNames[C->VarCount] = strdup(Name);
    return C->VarCount++;
}

static void resetFuncState(BeamCtx *C) {
    for (int i = 0; i < C->VarCount; i++) { free(C->VarNames[i]); C->VarNames[i] = NULL; }
    C->VarCount = C->NumYRegs = C->NumArgs = 0;
    if (C->VisitedBBs) { bbl_free(C->VisitedBBs); C->VisitedBBs = NULL; }
}

/* ── DFS BB collection ───────────────────────────────────────────────────── */

static struct basic_block_l *getAllBBs(struct BasicBlock *Entry) {
    struct basic_block_l *All     = bbl_create(16);
    struct basic_block_l *Stack   = bbl_create(16);
    struct basic_block_l *Visited = bbl_create(16);
    bbl_push_back(Stack, Entry);
    while (!bbl_empty(Stack)) {
        struct BasicBlock *BB = bbl_back(Stack);
        bbl_pop_back(Stack);
        if (bbl_contains(Visited, BB)) continue;
        bbl_push_back(Visited, BB);
        bbl_push_back(All, BB);
        for (size_t i = 0; i < bbl_count(BB->Successors); i++) {
            struct BasicBlock *S = bbl_get(BB->Successors, i);
            if (!bbl_contains(Visited, S)) bbl_push_back(Stack, S);
        }
    }
    bbl_free(Visited);
    bbl_free(Stack);
    return All;
}

/* ── variable collection ─────────────────────────────────────────────────── */

static void collectVarsDAG(struct OpDAGNode *N, BeamCtx *C) {
    if (!N) return;
    if (N->Type == OpDAGVP)
        addVar(C, N->TerminalData.ValuePlace.PlaceName);
    for (size_t i = 0; i < dnl_count(N->Operands); i++)
        collectVarsDAG(dnl_get(N->Operands, i), C);
}

static void collectFuncVars(BeamCtx *C, const struct Function *F) {
    /* args first so they map to y0..y(n-1) */
    for (size_t i = 0; i < al_count(F->Args); i++)
        addVar(C, al_get(F->Args, i)->Name);
    C->NumArgs = (int)al_count(F->Args);

    if (!F->CFG) { C->NumYRegs = C->VarCount; return; }

    struct basic_block_l *All = getAllBBs(F->CFG->Entry);
    for (size_t bi = 0; bi < bbl_count(All); bi++) {
        struct BasicBlock *BB = bbl_get(All, bi);
        for (size_t ii = 0; ii < dl_count(BB->Instructions); ii++)
            collectVarsDAG(dl_get(BB->Instructions, ii)->Root, C);
        if (BB->Type == BBTypeCondBranch && BB->Flow.Cond.Condition)
            collectVarsDAG(BB->Flow.Cond.Condition->Root, C);
    }
    bbl_free(All);
    C->NumYRegs = C->VarCount;
}

/* ── label pre-pass ──────────────────────────────────────────────────────── */

static void preLabelAssign(BeamCtx *C) {
    int n = (int)fl_count(C->Mod->Functions);
    assert(n < MAX_FUNCS);
    C->FuncCount = n;
    for (int fi = 0; fi < n; fi++) {
        struct Function *F = fl_get(C->Mod->Functions, fi);
        C->FuncInfoLabel[fi]  = allocLabel(C);
        C->FuncEntryLabel[fi] = allocLabel(C);
        if (!F->CFG) continue; /* built-in: only 2 labels */
        struct basic_block_l *All = getAllBBs(F->CFG->Entry);
        for (size_t bi = 0; bi < bbl_count(All); bi++) {
            struct BasicBlock *BB = bbl_get(All, bi);
            assert((int)BB->Id < MAX_BB_ID);
            if (BB == F->CFG->Entry)
                C->BBLabel[BB->Id] = C->FuncEntryLabel[fi];
            else
                C->BBLabel[BB->Id] = allocLabel(C);
        }
        bbl_free(All);
    }
    C->TotalLabels = C->NextLabel - 1;
}

/* ── function label lookup ───────────────────────────────────────────────── */

static int funcEntryLabel(BeamCtx *C, const char *Name, int Arity) {
    for (int i = 0; i < C->FuncCount; i++) {
        struct Function *F = fl_get(C->Mod->Functions, i);
        if (strcmp(F->Name, Name) == 0 && (int)al_count(F->Args) == Arity)
            return C->FuncEntryLabel[i];
    }
    return -1; /* unknown → will abort at runtime */
}

/* ── source operand formatting ───────────────────────────────────────────── */

/* Is this node a simple (no-compute) source? */
static bool isSimple(struct OpDAGNode *N) {
    switch (N->Type) {
    case OpDAGVP:
    case OpDAGLiteralBool:
    case OpDAGLiteralByte:
    case OpDAGLiteralDoubleWord:
    case OpDAGLiteralQuadWord:
        return true;
    default:
        return false;
    }
}

/* Write BEAM source operand into Buf without emitting code.
 * Only valid for simple nodes. */
static void simpleSrc(struct OpDAGNode *N, char *Buf, BeamCtx *C) {
    switch (N->Type) {
    case OpDAGVP: {
        int y = getVarY(C, N->TerminalData.ValuePlace.PlaceName);
        assert(y >= 0);
        sprintf(Buf, "{y, %d}", y);
        break;
    }
    case OpDAGLiteralBool:
        sprintf(Buf, "{atom, %s}", N->TerminalData.Literal.Bool ? "true" : "false");
        break;
    case OpDAGLiteralByte:
        sprintf(Buf, "{integer, %d}", (unsigned char)N->TerminalData.Literal.Char);
        break;
    case OpDAGLiteralDoubleWord:
        if (N->TerminalData.Literal.Signed)
            sprintf(Buf, "{integer, %d}", N->TerminalData.Literal.Int);
        else
            sprintf(Buf, "{integer, %u}", N->TerminalData.Literal.UInt);
        break;
    case OpDAGLiteralQuadWord:
        if (N->TerminalData.Literal.Signed)
            sprintf(Buf, "{integer, %ld}", (long)N->TerminalData.Literal.Long);
        else
            sprintf(Buf, "{integer, %lu}", (unsigned long)N->TerminalData.Literal.ULong);
        break;
    default:
        assert(false && "simpleSrc called on non-simple node");
    }
}

/* ── expression emitter ──────────────────────────────────────────────────── */

/* Emit code so that the value of Node ends up in x[XDest].
 * Uses x[XDest..XDest+depth] as scratch.
 * Returns XDest. */
static int emitExprX(struct OpDAGNode *Node, int XDest, BeamCtx *C);

/* Emit a call. Args go into x0..x(N-1). Result ends up in x[XDest]. */
static int emitCallNode(struct OpDAGNode *Node, int XDest, BeamCtx *C) {
    const char *FName;
    bool IsObj = (Node->Type == OpDAGObjectCall);

    if (IsObj) {
        /* object calls not needed for calc.ae */
        assert(false && "object calls NYI in BEAM backend");
        FName = Node->TerminalData.ObjectCall.FuncName;
    } else {
        FName = Node->TerminalData.Call.FuncName;
    }

    /* Single child is always OpDAGList */
    assert(dnl_count(Node->Operands) == 1);
    struct OpDAGNode *ArgList = dnl_get(Node->Operands, 0);
    assert(ArgList->Type == OpDAGList);
    int NArgs = (int)dnl_count(ArgList->Operands);

    /* Evaluate each argument into x0..x(N-1) */
    for (int i = 0; i < NArgs; i++)
        emitExprX(dnl_get(ArgList->Operands, i), i, C);

    int Label = funcEntryLabel(C, FName, NArgs);
    if (Label >= 0) {
        codeBufferAddFormatted(C->Code, "    {call, %d, {f, %d}}.\n", NArgs, Label);
    } else {
        /* Declaration not found — should not happen for valid programs */
        codeBufferAddFormatted(C->Code, "    %% WARNING: unknown function %s/%d\n", FName, NArgs);
    }

    /* Result is in x0; move to XDest if different */
    if (XDest != 0)
        codeBufferAddFormatted(C->Code, "    {move, {x, 0}, {x, %d}}.\n", XDest);
    return XDest;
}

static const char *binopBIF(enum OpDAGType T) {
    switch (T) {
    case OpDAGAdd:    return "'+'";
    case OpDAGSub:    return "'-'";
    case OpDAGMul:    return "'*'";
    case OpDAGDiv:    return "'div'";
    case OpDAGMod:    return "'rem'";
    case OpDAGBitAnd: return "'band'";
    case OpDAGBitOr:  return "'bor'";
    case OpDAGBitXor: return "'bxor'";
    case OpDAGBitShl: return "'bsl'";
    case OpDAGBitShr: return "'bsr'";
    default:          return NULL;
    }
}

static int emitExprX(struct OpDAGNode *Node, int XDest, BeamCtx *C) {
    /* ── leaf ── */
    if (isSimple(Node)) {
        char Src[128];
        simpleSrc(Node, Src, C);
        codeBufferAddFormatted(C->Code, "    {move, %s, {x, %d}}.\n", Src, XDest);
        return XDest;
    }

    /* ── call ── */
    if (Node->Type == OpDAGCall || Node->Type == OpDAGObjectCall)
        return emitCallNode(Node, XDest, C);

    /* ── binary arithmetic / bit ops ── */
    const char *BIF = binopBIF(Node->Type);
    if (BIF) {
        assert(dnl_count(Node->Operands) == 2);
        struct OpDAGNode *L = dnl_get(Node->Operands, 0);
        struct OpDAGNode *R = dnl_get(Node->Operands, 1);
        char LS[128], RS[128];

        if (isSimple(L) && isSimple(R)) {
            simpleSrc(L, LS, C);
            simpleSrc(R, RS, C);
            codeBufferAddFormatted(C->Code,
                "    {gc_bif, %s, {f, 0}, %d, [%s, %s], {x, %d}}.\n",
                BIF, XDest, LS, RS, XDest);
        } else if (isSimple(L)) {
            emitExprX(R, XDest, C);
            simpleSrc(L, LS, C);
            codeBufferAddFormatted(C->Code,
                "    {gc_bif, %s, {f, 0}, %d, [%s, {x, %d}], {x, %d}}.\n",
                BIF, XDest + 1, LS, XDest, XDest);
        } else if (isSimple(R)) {
            emitExprX(L, XDest, C);
            simpleSrc(R, RS, C);
            codeBufferAddFormatted(C->Code,
                "    {gc_bif, %s, {f, 0}, %d, [{x, %d}, %s], {x, %d}}.\n",
                BIF, XDest + 1, XDest, RS, XDest);
        } else {
            emitExprX(L, XDest, C);
            emitExprX(R, XDest + 1, C);
            codeBufferAddFormatted(C->Code,
                "    {gc_bif, %s, {f, 0}, %d, [{x, %d}, {x, %d}], {x, %d}}.\n",
                BIF, XDest + 2, XDest, XDest + 1, XDest);
        }
        return XDest;
    }

    /* ── unary ops ── */
    if (Node->Type == OpDAGBitNot) {
        assert(dnl_count(Node->Operands) == 1);
        struct OpDAGNode *Op = dnl_get(Node->Operands, 0);
        if (isSimple(Op)) {
            char Src[128]; simpleSrc(Op, Src, C);
            codeBufferAddFormatted(C->Code,
                "    {gc_bif, 'bnot', {f, 0}, %d, [%s], {x, %d}}.\n", XDest, Src, XDest);
        } else {
            emitExprX(Op, XDest, C);
            codeBufferAddFormatted(C->Code,
                "    {gc_bif, 'bnot', {f, 0}, %d, [{x, %d}], {x, %d}}.\n",
                XDest + 1, XDest, XDest);
        }
        return XDest;
    }

    if (Node->Type == OpDAGNeg) {
        assert(dnl_count(Node->Operands) == 1);
        struct OpDAGNode *Op = dnl_get(Node->Operands, 0);
        if (isSimple(Op)) {
            char Src[128]; simpleSrc(Op, Src, C);
            codeBufferAddFormatted(C->Code,
                "    {gc_bif, '-', {f, 0}, %d, [{integer, 0}, %s], {x, %d}}.\n",
                XDest, Src, XDest);
        } else {
            emitExprX(Op, XDest, C);
            codeBufferAddFormatted(C->Code,
                "    {gc_bif, '-', {f, 0}, %d, [{integer, 0}, {x, %d}], {x, %d}}.\n",
                XDest + 1, XDest, XDest);
        }
        return XDest;
    }

    /* ── comparison ops used in non-branch context (eval to 0/1) ── */
    /* In calc.ae these only appear in condition branches, but handle defensively */
    if (Node->Type == OpDAGEq || Node->Type == OpDAGNeq ||
        Node->Type == OpDAGLt || Node->Type == OpDAGLte ||
        Node->Type == OpDAGGt || Node->Type == OpDAGGte) {
        /* Evaluate both sides, compare, yield 1 or 0 as integer */
        assert(dnl_count(Node->Operands) == 2);
        struct OpDAGNode *L = dnl_get(Node->Operands, 0);
        struct OpDAGNode *R = dnl_get(Node->Operands, 1);
        char LS[128], RS[128];
        if (isSimple(L)) simpleSrc(L, LS, C);
        else { emitExprX(L, XDest, C); sprintf(LS, "{x, %d}", XDest); }
        if (isSimple(R)) simpleSrc(R, RS, C);
        else { emitExprX(R, XDest + 1, C); sprintf(RS, "{x, %d}", XDest + 1); }
        /* Use is_eq_exact etc., result in boolean atom, then convert */
        /* For simplicity: just return atom true/false, callers treat != false as true */
        const char *BifName = NULL;
        switch (Node->Type) {
        case OpDAGEq:  BifName = "'=='";  break;
        case OpDAGNeq: BifName = "'/='";  break;
        case OpDAGLt:  BifName = "'<'";   break;
        case OpDAGLte: BifName = "'=<'";  break;
        case OpDAGGt:  BifName = "'>'";   break;
        case OpDAGGte: BifName = "'>='";  break;
        default: break;
        }
        int Live = XDest;
        if (!isSimple(L)) Live = XDest + 1;
        if (!isSimple(R) && XDest + 2 > Live) Live = XDest + 2;
        codeBufferAddFormatted(C->Code,
            "    {gc_bif, %s, {f, 0}, %d, [%s, %s], {x, %d}}.\n",
            BifName, Live, LS, RS, XDest);
        return XDest;
    }

    fprintf(stderr, "BEAM: unsupported DAG node type %d in expression\n", Node->Type);
    assert(false && "unsupported node type");
    return XDest;
}

/* ── instruction emitter ─────────────────────────────────────────────────── */

/* Emit a single top-level instruction DAG (not the condition of a cond-branch). */
static void emitInstr(struct OpDAG *DAG, BeamCtx *C) {
    struct OpDAGNode *Root = DAG->Root;
    if (!Root) return;

    if (Root->Type == OpDAGAssign) {
        assert(dnl_count(Root->Operands) == 2);
        struct OpDAGNode *LHS = dnl_get(Root->Operands, 0);
        struct OpDAGNode *RHS = dnl_get(Root->Operands, 1);
        assert(LHS->Type == OpDAGVP && "field-assign NYI in BEAM backend");
        int y = getVarY(C, LHS->TerminalData.ValuePlace.PlaceName);
        assert(y >= 0);
        emitExprX(RHS, 0, C);
        codeBufferAddFormatted(C->Code, "    {move, {x, 0}, {y, %d}}.\n", y);
        return;
    }

    /* Standalone expression (e.g. discarded function call) */
    emitExprX(Root, 0, C);
}

/* ── conditional branch emitter ──────────────────────────────────────────── */

typedef struct { const char *TestOp; bool Flip; } TestDesc;

static TestDesc cmpToTest(enum OpDAGType T) {
    switch (T) {
    case OpDAGEq:  return (TestDesc){"is_eq_exact", false};
    case OpDAGNeq: return (TestDesc){"is_ne_exact", false};
    case OpDAGLt:  return (TestDesc){"is_lt",       false};
    case OpDAGLte: return (TestDesc){"is_ge",       true};  /* A<=B ↔ B>=A */
    case OpDAGGt:  return (TestDesc){"is_lt",       true};  /* A>B  ↔ B<A  */
    case OpDAGGte: return (TestDesc){"is_ge",       false};
    default:       return (TestDesc){NULL, false};
    }
}

static void emitCondBranch(struct BasicBlock *BB, BeamCtx *C) {
    struct OpDAGNode *Cond = BB->Flow.Cond.Condition->Root;
    int TrueLabel  = C->BBLabel[BB->Flow.Cond.IfTrue->Id];
    int FalseLabel = C->BBLabel[BB->Flow.Cond.IfFalse->Id];

    /* ── always-true / always-false literal ── */
    if (Cond->Type == OpDAGLiteralBool) {
        int Target = Cond->TerminalData.Literal.Bool ? TrueLabel : FalseLabel;
        codeBufferAddFormatted(C->Code, "    {jump, {f, %d}}.\n", Target);
        return;
    }

    /* ── direct comparison ── */
    TestDesc TD = cmpToTest(Cond->Type);
    if (TD.TestOp) {
        assert(dnl_count(Cond->Operands) == 2);
        struct OpDAGNode *L = dnl_get(Cond->Operands, 0);
        struct OpDAGNode *R = dnl_get(Cond->Operands, 1);
        if (TD.Flip) { struct OpDAGNode *Tmp = L; L = R; R = Tmp; }

        char LS[128], RS[128];
        if (isSimple(L)) simpleSrc(L, LS, C);
        else { emitExprX(L, 0, C); strcpy(LS, "{x, 0}"); }
        if (isSimple(R)) simpleSrc(R, RS, C);
        else { emitExprX(R, 1, C); strcpy(RS, "{x, 1}"); }

        /* BEAM test: jump to FalseLabel when test FAILS (condition is false) */
        codeBufferAddFormatted(C->Code,
            "    {test, %s, {f, %d}, [%s, %s]}.\n",
            TD.TestOp, FalseLabel, LS, RS);
        codeBufferAddFormatted(C->Code, "    {jump, {f, %d}}.\n", TrueLabel);
        return;
    }

    /* ── general boolean ── */
    emitExprX(Cond, 0, C);
    codeBufferAddFormatted(C->Code,
        "    {test, is_ne_exact, {f, %d}, [{x, 0}, {atom, false}]}.\n", FalseLabel);
    codeBufferAddFormatted(C->Code, "    {jump, {f, %d}}.\n", TrueLabel);
}

/* ── basic block emitter ─────────────────────────────────────────────────── */

static void emitBB(struct BasicBlock *BB, const struct Function *F, BeamCtx *C);

static void emitBBSuccessors(struct BasicBlock *BB, const struct Function *F, BeamCtx *C) {
    for (size_t i = 0; i < bbl_count(BB->Successors); i++) {
        struct BasicBlock *S = bbl_get(BB->Successors, i);
        if (!bbl_contains(C->VisitedBBs, S))
            emitBB(S, F, C);
    }
}

static void emitBB(struct BasicBlock *BB, const struct Function *F, BeamCtx *C) {
    if (bbl_contains(C->VisitedBBs, BB)) return;
    bbl_push_back(C->VisitedBBs, BB);

    int Label = C->BBLabel[BB->Id];
    codeBufferAddFormatted(C->Code, "  {label, %d}.\n", Label);

    size_t NInstr = dl_count(BB->Instructions);

    switch (BB->Type) {
    case BBTypeUncondBranch: {
        /* Emit all instructions in this block */
        for (size_t i = 0; i < NInstr; i++)
            emitInstr(dl_get(BB->Instructions, i), C);
        int Target = C->BBLabel[BB->Flow.Uncond.Target->Id];
        codeBufferAddFormatted(C->Code, "    {jump, {f, %d}}.\n", Target);
        break;
    }

    case BBTypeCondBranch: {
        /* Condition is the last instruction; emit the rest normally */
        size_t condIdx = NInstr > 0 ? NInstr - 1 : 0;
        for (size_t i = 0; i < condIdx; i++)
            emitInstr(dl_get(BB->Instructions, i), C);
        emitCondBranch(BB, C);
        break;
    }

    case BBTypeRet: {
        /* All instructions except the last are regular statements */
        size_t retIdx = NInstr > 0 ? NInstr - 1 : 0;
        for (size_t i = 0; i < retIdx; i++)
            emitInstr(dl_get(BB->Instructions, i), C);

        /* Evaluate return value to x0 */
        if (BB->Flow.Ret.RetVal && BB->Flow.Ret.RetVal->Root) {
            emitExprX(BB->Flow.Ret.RetVal->Root, 0, C);
        } else {
            codeBufferAddFormatted(C->Code, "    {move, {integer, 0}, {x, 0}}.\n");
        }
        codeBufferAddFormatted(C->Code, "    {deallocate, %d}.\n", C->NumYRegs);
        codeBufferAddFormatted(C->Code, "    return.\n");
        return; /* don't visit successors from a ret block */
    }
    }

    emitBBSuccessors(BB, F, C);
}

/* ── built-in implementations ────────────────────────────────────────────── */

static void emitBuiltinReadChar(BeamCtx *C, int FILabel, int EntryLabel) {
    /* readChar() -> integer ASCII value of the next stdin character */
    codeBufferAddFormatted(C->Code,
        "\n{function, 'readChar', 0, %d}.\n"
        "  {label, %d}.\n"
        "    {func_info, {atom, '%s'}, {atom, 'readChar'}, 0}.\n"
        "  {label, %d}.\n"
        "    {allocate, 0, 0}.\n"
        "    {move, {atom, ''}, {x, 0}}.\n"
        "    {move, {integer, 1}, {x, 1}}.\n"
        "    {call_ext, 2, {extfunc, io, get_chars, 2}}.\n"
        "    {bif, hd, {f, 0}, [{x, 0}], {x, 0}}.\n"
        "    {deallocate, 0}.\n"
        "    return.\n",
        EntryLabel, FILabel, C->ModName, EntryLabel);
}

static void emitBuiltinWriteChar(BeamCtx *C, int FILabel, int EntryLabel) {
    /* writeChar(C) -> write character with ASCII value C to stdout */
    codeBufferAddFormatted(C->Code,
        "\n{function, 'writeChar', 1, %d}.\n"
        "  {label, %d}.\n"
        "    {func_info, {atom, '%s'}, {atom, 'writeChar'}, 1}.\n"
        "  {label, %d}.\n"
        "    {allocate_heap, 0, 2, 1}.\n"
        "    {put_list, {x, 0}, nil, {x, 0}}.\n"
        "    {call_ext_last, 1, {extfunc, io, put_chars, 1}, 0}.\n",
        EntryLabel, FILabel, C->ModName, EntryLabel);
}

/* ── function emitter ────────────────────────────────────────────────────── */

static void emitBeamFunction(BeamCtx *C, int FuncIdx, const struct Function *F) {
    resetFuncState(C);
    collectFuncVars(C, F);

    int FILabel    = C->FuncInfoLabel[FuncIdx];
    int EntryLabel = C->FuncEntryLabel[FuncIdx];
    int Arity      = (int)al_count(F->Args);

    /* Emit {function,...} + func_info guard (no entry label yet) */
    codeBufferAddFormatted(C->Code,
        "\n{function, '%s', %d, %d}.\n"
        "  {label, %d}.\n"
        "    {func_info, {atom, '%s'}, {atom, '%s'}, %d}.\n",
        F->Name, Arity, EntryLabel,
        FILabel, C->ModName, F->Name, Arity);

    /* Emit entry label + prologue inline.
     * The entry BB (CFG->Entry) is always an unconditional branch with zero
     * instructions: it was created by the CFG builder before ASTStatementBlock
     * adds the real body.  We handle it here so emitBB never sees it again. */
    struct BasicBlock *EntryBB = F->CFG->Entry;
    codeBufferAddFormatted(C->Code, "  {label, %d}.\n", EntryLabel);
    codeBufferAddFormatted(C->Code, "    {allocate_zero, %d, %d}.\n", C->NumYRegs, Arity);
    for (int i = 0; i < C->NumArgs; i++)
        codeBufferAddFormatted(C->Code, "    {move, {x, %d}, {y, %d}}.\n", i, i);

    /* Emit entry BB's instructions (normally empty) and its terminal */
    for (size_t i = 0; i < dl_count(EntryBB->Instructions); i++)
        emitInstr(dl_get(EntryBB->Instructions, i), C);

    switch (EntryBB->Type) {
    case BBTypeUncondBranch:
        codeBufferAddFormatted(C->Code, "    {jump, {f, %d}}.\n",
            C->BBLabel[EntryBB->Flow.Uncond.Target->Id]);
        break;
    case BBTypeCondBranch:
        emitCondBranch(EntryBB, C);
        break;
    case BBTypeRet:
        if (EntryBB->Flow.Ret.RetVal && EntryBB->Flow.Ret.RetVal->Root)
            emitExprX(EntryBB->Flow.Ret.RetVal->Root, 0, C);
        else
            codeBufferAddFormatted(C->Code, "    {move, {integer, 0}, {x, 0}}.\n");
        codeBufferAddFormatted(C->Code, "    {deallocate, %d}.\n", C->NumYRegs);
        codeBufferAddFormatted(C->Code, "    return.\n");
        return;
    }

    /* Continue emitting all other BBs; entry BB is pre-marked visited */
    C->VisitedBBs = bbl_create(16);
    bbl_push_back(C->VisitedBBs, EntryBB);
    emitBBSuccessors(EntryBB, F, C);
}

/* ── module header ───────────────────────────────────────────────────────── */

static void emitHeader(BeamCtx *C) {
    codeBufferAddFormatted(C->Code, "{module, '%s'}.\n", C->ModName);

    /* Only export main/0 */
    codeBufferAddFormatted(C->Code, "{exports, [{main, 0}]}.\n");
    codeBufferAddFormatted(C->Code, "{attributes, []}.\n");
    codeBufferAddFormatted(C->Code, "{labels, %d}.\n", C->TotalLabels);
}

/* ── public entry point ──────────────────────────────────────────────────── */

int generateBeam(struct Module *M, const char *ModuleName, const char *OutputFile) {
    BeamCtx *C = beamCtxCreate(ModuleName, M);

    /* Assign all labels in one pre-pass */
    preLabelAssign(C);

    /* Emit header (uses TotalLabels computed by pre-pass) */
    emitHeader(C);

    /* Emit each function */
    for (int fi = 0; fi < C->FuncCount; fi++) {
        struct Function *F = fl_get(M->Functions, fi);
        int FILabel    = C->FuncInfoLabel[fi];
        int EntryLabel = C->FuncEntryLabel[fi];

        if (!F->CFG) {
            /* Declaration: generate built-in */
            if (strcmp(F->Name, "readChar") == 0 && al_count(F->Args) == 0)
                emitBuiltinReadChar(C, FILabel, EntryLabel);
            else if (strcmp(F->Name, "writeChar") == 0 && al_count(F->Args) == 1)
                emitBuiltinWriteChar(C, FILabel, EntryLabel);
            else
                fprintf(stderr, "BEAM: no built-in for declared function '%s'\n", F->Name);
        } else {
            /* Definition: translate from CFG */
            emitBeamFunction(C, fi, F);
        }
    }

    /* Write to file */
    FILE *Out = fopen(OutputFile, "w");
    if (!Out) {
        perror(OutputFile);
        beamCtxFree(C);
        return 1;
    }
    codeBufferDumpToFile(C->Code, Out);
    fclose(Out);

    printf("BEAM assembly written to %s (%d labels)\n", OutputFile, C->TotalLabels);
    beamCtxFree(C);
    return 0;
}
