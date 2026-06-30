// #include "araneae/aux/PtrVector.h"

// PtrVector *ptrvectorCreate() {
//   PtrVector *Vec = (PtrVector *)malloc(sizeof(PtrVector));
//   Vec->Data = NULL;
//   Vec->Size = 0;
//   Vec->Capacity = 0;
// }

// void ptrvectorFree(PtrVector *Vec) {
//   free(Vec->Data);
//   Vec->Data = NULL;
//   Vec->Size = 0;
//   Vec->Capacity = 0;
// }

// void ptrvectorFreeDeep(PtrVector *Vec) {
//   for (size_t i = 0; i < Vec->Size; i++) {
//     free(Vec->Data[i]);
//   }
//   free(Vec->Data);
//   Vec->Data = NULL;
//   Vec->Size = 0;
//   Vec->Capacity = 0;
// }

// void ptrvectorForeach(const PtrVector *Vec, PtrVectorForeachF F) {
//   for (size_t I = 0; I < ptrvectorSize(Vec); ++I)
//     F(ptrvectorAt(Vec, I));
// }

// size_t ptrvectorSize(const PtrVector *Vec) { return Vec->Size; }

// size_t ptrvectorCapacity(const PtrVector *Vec) { return Vec->Capacity; }

// bool ptrvectorEmpty(const PtrVector *Vec) { return Vec->Size == 0; }

// bool ptrvectorReserve(PtrVector *Vec, size_t NewCapacity) {
//   if (NewCapacity <= Vec->Capacity)
//     return true;

//   size_t Capacity = Vec->Capacity;
//   if (Capacity == 0)
//     Capacity = 8;
//   while (Capacity < NewCapacity)
//     Capacity *= 2;

//   void **new_data = realloc(Vec->Data, Capacity * sizeof(void *));
//   if (!new_data)
//     return false;

//   Vec->Data = new_data;
//   Vec->Capacity = Capacity;
//   return true;
// }

// bool ptrvectorResize(PtrVector *Vec, size_t NewSize, void *DefaultValue) {
//   if (!ptrvectorReserve(Vec, NewSize))
//     return false;

//   if (NewSize > Vec->Size) {
//     for (size_t i = Vec->Size; i < NewSize; i++) {
//       Vec->Data[i] = DefaultValue;
//     }
//   }

//   Vec->Size = NewSize;
//   return true;
// }

// bool ptrvectorPushBack(PtrVector *Vec, void *Value) {
//   if (Vec->Size >= Vec->Capacity) {
//     size_t NewCapacity = Vec->Capacity ? Vec->Capacity * 2 : 8;
//     if (!ptrvectorReserve(Vec, NewCapacity))
//       return false;
//   }

//   Vec->Data[Vec->Size++] = Value;
//   return true;
// }

// void *ptrvectorPopBack(PtrVector *Vec) {
//   if (Vec->Size == 0)
//     return NULL;
//   return Vec->Data[--Vec->Size];
// }

// void *ptrvectorAt(const PtrVector *Vec, size_t Index) {
//   return (Index < Vec->Size) ? Vec->Data[Index] : NULL;
// }

// void *ptrvectorFront(const PtrVector *Vec) {
//   return (Vec->Size > 0) ? Vec->Data[0] : NULL;
// }

// void *ptrvectorBack(const PtrVector *Vec) {
//   return (Vec->Size > 0) ? Vec->Data[Vec->Size - 1] : NULL;
// }

// bool ptrvectorInsert(PtrVector *Vec, size_t Index, void *Value) {
//   if (Index > Vec->Size)
//     return false;

//   if (Vec->Size >= Vec->Capacity) {
//     if (!ptrvectorReserve(Vec, Vec->Size + 1))
//       return false;
//   }

//   memmove(&Vec->Data[Index + 1], &Vec->Data[Index],
//           (Vec->Size - Index) * sizeof(void *));

//   Vec->Data[Index] = Value;
//   Vec->Size++;
//   return true;
// }

// void *ptrvectorErase(PtrVector *Vec, size_t Index) {
//   if (Index >= Vec->Size)
//     return NULL;

//   void *removed = Vec->Data[Index];

//   if (Index < Vec->Size - 1) {
//     memmove(&Vec->Data[Index], &Vec->Data[Index + 1],
//             (Vec->Size - Index - 1) * sizeof(void *));
//   }

//   Vec->Size--;
//   return removed;
// }

// void ptrvectorClear(PtrVector *Vec) { Vec->Size = 0; }

// void ptrvectorClearCustom(PtrVector *Vec, ClearF F) {
//   for (size_t i = 0; i < Vec->Size; i++) {
//     F(Vec->Data[i]);
//     free(Vec->Data[i]);
//   }
//   Vec->Size = 0;
// }

// void ptrvectorClearDeep(PtrVector *Vec) {
//   for (size_t i = 0; i < Vec->Size; i++) {
//     free(Vec->Data[i]);
//   }
//   Vec->Size = 0;
// }

// ssize_t ptrvectorFind(const PtrVector *Vec, const void *Value) {
//   for (size_t i = 0; i < Vec->Size; i++) {
//     if (Vec->Data[i] == Value)
//       return (ssize_t)i;
//   }
//   return -1;
// }
