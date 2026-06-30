#include "araneae/cfg/Cfg.h"
#include "araneae/cfg/Dag.h"
#include "araneae/cfg/Class.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Вспомогательная функция для получения строкового представления типа узла
static const char* getDAGNodeTypeName(enum OpDAGType type) {
    switch (type) {
        case OpDAGAssign: return "ASSIGN";
        case OpDAGVP: return "VALUE_PLACE";
        case OpDAGFieldAccess: return "FIELD_ACCESS";
        case OpDAGCall: return "CALL";
        case OpDAGObjectCall: return "OBJECT_CALL";
        case OpDAGIndexer: return "INDEXER";
        case OpDAGCreateArray: return "CREATE_ARRAY";
        // Arith
        case OpDAGAdd: return "ADD";
        case OpDAGSub: return "SUB";
        case OpDAGMul: return "MUL";
        case OpDAGDiv: return "DIV";
        case OpDAGMod: return "MOD";
        case OpDAGNeg: return "NEG";
        case OpDAGInc: return "INC";
        case OpDAGDec: return "DEC";
        // Logical
        case OpDAGEq: return "EQ";
        case OpDAGNeq: return "NEQ";
        case OpDAGLt: return "LT";
        case OpDAGLte: return "LTE";
        case OpDAGGt: return "GT";
        case OpDAGGte: return "GTE";
        case OpDAGAnd: return "AND";
        case OpDAGOr: return "OR";
        case OpDAGNot: return "NOT";
        // Binary
        case OpDAGBitAnd: return "BIT_AND";
        case OpDAGBitOr: return "BIT_OR";
        case OpDAGBitXor: return "BIT_XOR";
        case OpDAGBitNot: return "BIT_NOT";
        case OpDAGBitShl: return "BIT_SHL";
        case OpDAGBitShr: return "BIT_SHR";
        // Literals
        case OpDAGLiteralStr: return "LIT_Str";
        case OpDAGLiteralBool: return "LIT_Bool";
        case OpDAGLiteralByte: return "LIT_Byte";
        case OpDAGLiteralDoubleWord: return "LIT_DoubleWord";
        case OpDAGLiteralQuadWord: return "LIT_QuadWord";
        // Misc
        case OpDAGList: return "LIST";
        case OpDAGNoSpec: return "NO_SPEC";
        default: return "UNKNOWN";
    }
}

// Вспомогательная функция для форматирования метки узла
static void formatNodeLabel(FILE *file, struct OpDAGNode *node) {
    fprintf(file, "  node_%lu [label=\"%s", node->Id, getDAGNodeTypeName(node->Type));
    fprintf(file, "\\type: NULL");
    // Добавляем дополнительную инфу в зависимости от типа
    switch (node->Type) {
        case OpDAGCall:
            if (node->TerminalData.Call.FuncName)
                fprintf(file, "\\n%s", node->TerminalData.Call.FuncName);
            break;
        case OpDAGObjectCall:
            if (node->TerminalData.ObjectCall.ObjectName && node->TerminalData.ObjectCall.FuncName)
                fprintf(file, "\\n%s.%s", node->TerminalData.ObjectCall.ObjectName, node->TerminalData.ObjectCall.FuncName);
            break;
        case OpDAGVP:
            if (node->TerminalData.ValuePlace.PlaceName)
                fprintf(file, "\\n%s", node->TerminalData.ValuePlace.PlaceName);
            break;
        case OpDAGFieldAccess:
            if (node->TerminalData.FieldAccess.FieldName && node->TerminalData.FieldAccess.ObjectName)
                fprintf(file, "\\n%s.%s", node->TerminalData.FieldAccess.ObjectName, node->TerminalData.FieldAccess.FieldName);
            break;
        case OpDAGIndexer:
            if (node->TerminalData.Indexer.IndexedName)
                fprintf(file, "\\n%s", node->TerminalData.Indexer.IndexedName);
            break;
        case OpDAGCreateArray:
            fprintf(file, "\\n%ld", node->TerminalData.CreateArray.SizeOf);
            break;
        case OpDAGLiteralStr:
            if (node->TerminalData.Literal.Str)
                fprintf(file, "\\n%s", node->TerminalData.Literal.Str);
            break;
        case OpDAGLiteralByte:
            fprintf(file, "\\n%c", node->TerminalData.Literal.Char);
            break;
        case OpDAGLiteralBool:
            fprintf(file, "\\n%s", node->TerminalData.Literal.Bool ? "true" : "false");
            break;
        case OpDAGLiteralDoubleWord:
            if (node->TerminalData.Literal.Signed)
                fprintf(file, "\\n%u", node->TerminalData.Literal.Int);
            else
                fprintf(file, "\\n%d", node->TerminalData.Literal.UInt);
            break;
        default:
            break;
    }
    
    fprintf(file, "\"];\n");
}

// Рекурсивная функция для обхода DAG узлов
static void exportDAGNodeToDot(FILE *file, struct OpDAGNode *node, 
                                struct dag_node_l *visited) {
    if (!node) return;
    
    // Проверяем, не посетили ли уже этот узел
    for (size_t i = 0; i < dnl_count(visited); i++) {
        struct OpDAGNode *visited_node = dnl_get(visited, i);
        if (visited_node == node) return;
    }
    
    // Добавляем текущий узел в список посещенных
    dnl_push_back(visited, node);
    
    // Выводим узел
    formatNodeLabel(file, node);
    
    // Обрабатываем операнды (дети узла)
    if (node->Operands) {
        size_t numOperands = dnl_count(node->Operands);
        for (size_t i = 0; i < numOperands; i++) {
            struct OpDAGNode *operand = dnl_get(node->Operands, i);
            if (operand) {
                // Для бинарных операций помечаем левый и правый операнд
                const char *edge_label = "";
                switch (node->Type) {
                    case OpDAGAssign:
                        edge_label = i == 0 ? "src" : "dest";
                        break;
                    case OpDAGVP:
                        edge_label = i == 0 ? "ptr" : "val";
                        break;
                    case OpDAGCall:
                        edge_label = i == 0 ? "func" : "arg";
                        break;
                    case OpDAGIndexer:
                        edge_label = i == 0 ? "array" : "index";
                        break;
                    default:
                        edge_label = i == 0 ? "left" : "right";
                        break;
                }
                
                fprintf(file, "  node_%lu -> node_%lu [label=\"%s\"];\n", 
                        node->Id, operand->Id, edge_label);
                exportDAGNodeToDot(file, operand, visited);
            }
        }
    }
}

// Функция для экспорта одного DAG в DOT файл
void exportDAGToDot(struct OpDAG *DAG, const char *OutFile) {
    if (!DAG) return;
    FILE *file = fopen(OutFile, "w");

    if (!file) {
        fprintf(stderr, "Error: Cannot open file for writing\n");
        return;
    }
    
    // Начало DOT графа
    fprintf(file, "digraph DAG_%lu {\n", DAG->Id);
    fprintf(file, "  rankdir=TB;\n");
    fprintf(file, "  node [shape=ellipse, style=filled, fillcolor=lightyellow];\n");
    fprintf(file, "  splines=ortho;\n\n");
    
    // Создаем список для отслеживания посещенных узлов
    struct dag_node_l *visited = dnl_create(16);
    if (!visited) {
        fprintf(stderr, "Error: Cannot create visited list\n");
        fclose(file);
        return;
    }
    
    // Начинаем с корневого узла
    if (DAG->Root) {
        exportDAGNodeToDot(file, DAG->Root, visited);
    }
    
    // Также выводим все узлы из списка Nodes, если они не были посещены
    if (DAG->Nodes) {
        size_t numNodes = dnl_count(DAG->Nodes);
        for (size_t i = 0; i < numNodes; i++) {
            struct OpDAGNode *node = dnl_get(DAG->Nodes, i);
            if (node) {
                // Проверяем, не был ли уже обработан
                bool already_visited = false;
                for (size_t j = 0; j < dnl_count(visited); j++) {
                    struct OpDAGNode *visited_node = dnl_get(visited, j);
                    if (visited_node == node) {
                        already_visited = true;
                        break;
                    }
                }
                if (!already_visited) {
                    // Выводим непосещенный узел
                    formatNodeLabel(file, node);
                    
                    // Выводим связи с операндами
                    if (node->Operands) {
                        size_t numOperands = dnl_count(node->Operands);
                        for (size_t k = 0; k < numOperands; k++) {
                            struct OpDAGNode *operand = dnl_get(node->Operands, k);
                            if (operand) {
                                const char *edge_label = k == 0 ? "left" : "right";
                                fprintf(file, "  node_%lu -> node_%lu [label=\"%s\"];\n", 
                                        node->Id, operand->Id, edge_label);
                            }
                        }
                    }
                }
            }
        }
    }
    
    fprintf(file, "}\n");
    fclose(file);
}

// Функция для экспорта CFG в DOT файл
void exportCFGToDot(struct CFG *CFG, const char *OutFile) {
    if (!CFG) return;

    FILE *file = fopen(OutFile, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file for writing\n");
        return;
    }
    
    // Начало DOT графа
    fprintf(file, "digraph CFG {\n");
    fprintf(file, "  rankdir=TB;\n");
    fprintf(file, "  node [shape=box, style=rounded];\n");
    fprintf(file, "  splines=ortho;\n\n");
    
    // Проходим по всем базовым блокам и выводим их
    size_t numBlocks = bbl_count(CFG->Blocks);
    for (size_t i = 0; i < numBlocks; i++) {
        struct BasicBlock *bb = bbl_get(CFG->Blocks, i);
        if (!bb) continue;
        
        // Формируем метку для узла
        fprintf(file, "  bb_%lu [label=\"BB_%lu\\n%s", bb->Id, bb->Id, 
                bb->Label ? bb->Label : "unknown");
        
        // Добавляем тип блока
        switch (bb->Type) {
            case BBTypeUncondBranch:
                fprintf(file, "\\n[Uncond Branch]");
                break;
            case BBTypeCondBranch:
                fprintf(file, "\\n[Cond Branch]");
                break;
            case BBTypeRet:
                fprintf(file, "\\n[Return]");
                break;
        }
        
        // Добавляем количество инструкций
        if (bb->Instructions) {
            fprintf(file, "\\nInstructions: %zu", dl_count(bb->Instructions));
        }
        
        // Экспортируем каждый DAG инструкции в отдельный файл
        for (size_t j = 0; j < dl_count(bb->Instructions); ++j) {
            struct OpDAG *dag = dl_get(bb->Instructions, j);
            fprintf(file, "\\nInstruction # %ld", dag->Id);
            if (dag) {
                char OutFBuff[256];
                snprintf(OutFBuff, sizeof(OutFBuff), "%s_dag_%lu.dot", OutFile, dag->Id);
                exportDAGToDot(dag, OutFBuff);
            }
        }
        
        // Добавляем информацию о преемниках/предшественниках
        fprintf(file, "\\nPred: %zu, Succ: %zu", 
                bb->Predecessors ? bbl_count(bb->Predecessors) : 0,
                bb->Successors ? bbl_count(bb->Successors) : 0);
        
        fprintf(file, "\"];\n");
    }
    
    fprintf(file, "\n");
    
    // Проходим по всем базовым блокам и выводим связи
    for (size_t i = 0; i < numBlocks; i++) {
        struct BasicBlock *bb = bbl_get(CFG->Blocks, i);
        if (!bb) continue;
        
        // В зависимости от типа блока выводим разные связи
        switch (bb->Type) {
            case BBTypeUncondBranch:
                if (bb->Flow.Uncond.Target) {
                    fprintf(file, "  bb_%lu -> bb_%lu [label=\"jump\", color=blue];\n", 
                            bb->Id, bb->Flow.Uncond.Target->Id);
                }
                break;
                
            case BBTypeCondBranch:
                if (bb->Flow.Cond.IfTrue) {
                    fprintf(file, "  bb_%lu -> bb_%lu [label=\"true\", color=green];\n", 
                            bb->Id, bb->Flow.Cond.IfTrue->Id);
                }
                if (bb->Flow.Cond.IfFalse) {
                    fprintf(file, "  bb_%lu -> bb_%lu [label=\"false\", color=red];\n", 
                            bb->Id, bb->Flow.Cond.IfFalse->Id);
                }
                break;
                
            case BBTypeRet:
                // Терминальный блок - выделяем его
                fprintf(file, "  bb_%lu [shape=doublecircle, color=darkgreen];\n", bb->Id);
                break;
        }
    }
    
    // Отмечаем entry блок
    if (CFG->Entry) {
        fprintf(file, "\n  entry [shape=point, width=0.1];\n");
        fprintf(file, "  entry -> bb_%lu [label=\"start\", color=purple, penwidth=2];\n", 
                CFG->Entry->Id);
    }
    
    fprintf(file, "}\n");
    fclose(file);
}