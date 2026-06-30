// #ifndef ARANEAE_AUX_PTRVECTOR_H
// #define ARANEAE_AUX_PTRVECTOR_H

// #include <stdbool.h>
// #include <stddef.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>

// typedef struct PtrVector {
//   void **Data;
//   size_t Size;
//   size_t Capacity;
// } PtrVector;

// PtrVector *ptrvectorCreate();
// void ptrvectorFree(PtrVector *Vec);
// void ptrvectorFreeDeep(PtrVector *Vec);

// size_t ptrvectorSize(const PtrVector *Vec);
// size_t ptrvectorCapacity(const PtrVector *Vec);
// bool ptrvectorEmpty(const PtrVector *Vec);

// bool ptrvectorReserve(PtrVector *Vec, size_t NewCapacity);
// bool ptrvectorResize(PtrVector *Vec, size_t NewSize, void *DefaultValue);

// bool ptrvectorPushBack(PtrVector *Vec, void *Value);
// void *ptrvectorPopBack(PtrVector *Vec);

// void *ptrvectorAt(const PtrVector *Vec, size_t Index);
// void *ptrvectorFront(const PtrVector *Vec);
// void *ptrvectorBack(const PtrVector *Vec);

// bool ptrvectorInsert(PtrVector *Vec, size_t Index, void *Value);
// void *ptrvectorErase(PtrVector *Vec, size_t Index);
// void ptrvectorClear(PtrVector *Vec);
// void ptrvectorClearDeep(PtrVector *Vec);
// typedef void ClearF(void *);
// void ptrvectorClearCustom(PtrVector *Vec, ClearF F);

// ssize_t ptrvectorFind(const PtrVector *Vec, const void *Value);

// typedef void PtrVectorForeachF(void *El);
// void ptrvectorForeach(const PtrVector *Vec, PtrVectorForeachF F);

// #define PTRVECTOR_FOREACH(vec, item_type, var)                                 \
//   for (item_type *var = (vec).size > 0 ? (item_type *)(vec).data[0] : NULL,    \
//                  **_iter = (vec).data;                                         \
//        _iter < (vec).data + (vec).size;                                        \
//        ++_iter, var = _iter < (vec).data + (vec).size ? (item_type *)*_iter    \
//                                                       : NULL)

// #define PTRVECTOR_FOREACH_PTR(vec, item_type, var)                             \
//   for (item_type **var = (vec).size > 0 ? (item_type **)(vec).data : NULL;     \
//        var < (item_type **)(vec).data + (vec).size; ++var)

// #endif // ARANEAE_AUX_PTRVECTOR_H
