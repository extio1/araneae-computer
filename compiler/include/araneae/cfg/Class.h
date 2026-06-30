#ifndef ARANEAE_CFG_CLASS_H
#define ARANEAE_CFG_CLASS_H

#include "Function.h"
#include <stdbool.h>
#include <stddef.h>

struct ClassType;
struct ArrayType;
struct Field;
struct Method;
struct InheritanceInfo;

// Member visibility.
enum MemberVisibility {
  MemberVisibilityPrivate,
  MemberVisibilityPublic
};

// Field member.
struct Field {
  const char *Name;
  enum MemberVisibility Visibility;
  // Offset in object layout
  size_t Offset;
};
struct field_l;
struct field_l *fldl_create(size_t Capacity);
void fldl_free(struct field_l *L);
bool fldl_push_back(struct field_l *L, struct Field *V);
size_t fldl_count(struct field_l *L);
struct Field *fldl_get(struct field_l *L, size_t I);
bool fldl_pop_back(struct field_l *);
struct Field *fldl_back(struct field_l *);
bool fldl_empty(struct field_l *);
bool fldl_contains(struct field_l *, struct Field *);

int fieldEq(struct Field *Arg1, struct Field *Arg2);

// Method member.
struct Method {
  struct Function *Function;
  enum MemberVisibility Visibility;
};
struct method_l;
struct method_l *methl_create(size_t Capacity);
void methl_free(struct method_l *L);
bool methl_push_back(struct method_l *L, struct Method *V);
size_t methl_count(struct method_l *L);
struct Method *methl_get(struct method_l *L, size_t I);
bool methl_pop_back(struct method_l *);
struct Method *methl_back(struct method_l *);
bool methl_empty(struct method_l *);
bool methl_contains(struct method_l *, struct Method *);

int methodEq(struct Method *Arg1, struct Method *Arg2);
struct Method *methodCreate(struct Function *F, enum MemberVisibility V);

// Inheritance relationship.
typedef struct InheritanceInfo {
  struct ClassType *BaseClass;
  bool IsVirtual;
} InheritanceInfo;

// Interface class.
typedef struct Interface {
  char *Name;
  // Must have all function only signatures.
  struct method_l *Methods;
} Interface;
struct interface_l;
struct interface_l *intfl_create(size_t Capacity);
void intfl_free(struct interface_l *L);
bool intfl_push_back(struct interface_l *L, struct Interface *V);
size_t intfl_count(struct interface_l *L);
struct Interface *intfl_get(struct interface_l *L, size_t I);
bool intfl_pop_back(struct interface_l *);
struct Interface *intfl_back(struct interface_l *);
bool intfl_empty(struct interface_l *);
bool intfl_contains(struct interface_l *, struct Interface *);

int interfaceEq(struct Interface *Arg1, struct Interface *Arg2);

// Class type struct (only one base class, several interfaces).
struct ClassType {
  uint8_t Id;
  const char *Name;
  
  struct ClassType *BaseClass;
  struct interface_l *Interfaces;
  
  struct method_l *Methods;
  struct method_l *Constructors;

  struct field_l *Fields;
  size_t _CurOffset;
};
struct class_type_l;
struct class_type_l *cltl_create(size_t Capacity);
void cltl_free(struct class_type_l *L);
bool cltl_push_back(struct class_type_l *L, struct ClassType *V);
size_t cltl_count(struct class_type_l *L);
struct ClassType *cltl_get(struct class_type_l *L, size_t I);
bool cltl_pop_back(struct class_type_l *);
struct ClassType *cltl_back(struct class_type_l *);
bool cltl_empty(struct class_type_l *);
bool cltl_contains(struct class_type_l *, struct ClassType *);
struct ClassType *classCreate(const char *Name);
void classAddBase(struct ClassType *Derived, struct ClassType *Base);
void classAddField(struct ClassType *Class, const char *Name);
void classAddMethod(struct ClassType *Class, struct Method *Method);
void classAddConstructor(struct ClassType *Class, struct Function *Constructor);
int classTypeEq(struct ClassType *Arg1, struct ClassType *Arg2);
struct ClassType *classLookup(struct class_type_l *Classes, const char* Name);

#endif