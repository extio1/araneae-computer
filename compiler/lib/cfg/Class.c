#include "araneae/cfg/Class.h"

#include "araneae/abi/Types.h"
#include "araneae/cfg/Function.h"
#include "araneae/aux/Aux.h"

#include "cmc/utl/futils.h"

#define V struct Field *
#define PFX fldl
#define SNAME field_l
#include "cmc/list.h"

struct field_l *fldl_create(size_t Capacity) {
  static struct field_l_fval FVal = {.cmp = fieldEq};
  struct field_l *L = fldl_new(8, &FVal);
  return L;
}

#define V struct Method *
#define PFX methl
#define SNAME method_l
#include "cmc/list.h"

struct method_l *methl_create(size_t Capacity) {
  static struct method_l_fval FVal = {.cmp = methodEq};
  struct method_l *L = methl_new(8, &FVal);
  return L;
}

#define V struct Interface *
#define PFX intfl
#define SNAME interface_l
#include "cmc/list.h"

struct interface_l *intfl_create(size_t Capacity) {
  static struct interface_l_fval FVal = {.cmp = interfaceEq};
  struct interface_l *L = intfl_new(8, &FVal);
  return L;
}

#define V struct ClassType *
#define PFX cltl
#define SNAME class_type_l
#include "cmc/list.h"

struct class_type_l *cltl_create(size_t Capacity) {
  static struct class_type_l_fval FVal = {.cmp = classTypeEq};
  struct class_type_l *L = cltl_new(8, &FVal);
  return L;
}

struct ClassType *classCreate(const char *Name) {
  struct ClassType *Class = (struct ClassType *)malloc(sizeof(struct ClassType));
  static uint8_t ClassEnumerator = TYPE_BUILD_LAST + 1; // TODO: here is overflow error is UB
  Class->Name = Name;
  Class->Id = ClassEnumerator++;
  Class->Fields = fldl_create(4);
  Class->Methods = methl_create(8);
  Class->Constructors = methl_create(1);
  Class->Interfaces = intfl_create(2);
  Class->BaseClass = NULL;
  Class->_CurOffset = 0;
  return Class;
}

void classAddBase(struct ClassType *Derived, struct ClassType *Base) {
  Derived->BaseClass = Base;
}

void classAddField(struct ClassType *Class, const char *Name) {
  struct Field *F = (struct Field *)malloc(sizeof(struct Field));
  F->Name = Name;
  F->Offset = Class->_CurOffset;
  Class->_CurOffset += 8;
  fldl_push_back(Class->Fields, F);
}
void classAddMethod(struct ClassType *Class, struct Method *Method) {
  methl_push_back(Class->Methods, Method);
}
void classAddConstructor(struct ClassType *Class, struct Function *Constructor) {
  struct Method *M = methodCreate(Constructor, MemberVisibilityPublic);
  methl_push_back(Class->Constructors, M);
}

bool typeIsSubclass(struct ClassType *Derived, struct ClassType *Base) {
  return Derived->BaseClass = Base;
}

int methodEq(struct Method *Arg1, struct Method *Arg2) {
  return functionEq(Arg1->Function, Arg2->Function);
}

struct Method *methodCreate(struct Function *F, enum MemberVisibility V) {
  struct Method *M = (struct Method *)malloc(sizeof(struct Method));
  M->Function = F;
  M->Visibility = V;
  return M;
}

int interfaceEq(struct Interface *Arg1, struct Interface *Arg2) {
  return strcmp(Arg1->Name, Arg2->Name);
}

int fieldEq(struct Field *Arg1, struct Field *Arg2) {
  return (strcmp(Arg1->Name, Arg2->Name) == 0) && 
         (Arg1->Offset == Arg2->Offset) && 
         (Arg1->Visibility == Arg2->Visibility);
}

int classTypeEq(struct ClassType *Arg1, struct ClassType *Arg2) {
  return (strcmp(Arg1->Name, Arg2->Name) == 0);
}

struct ClassType *classLookup(struct class_type_l *Classes, const char* Name) {
  for (size_t I = 0; I < cltl_count(Classes); ++I) {
    struct ClassType *Class = cltl_get(Classes, I);
    if (Class->Name, Name)
      return Class;
  }
  return NULL;
}
