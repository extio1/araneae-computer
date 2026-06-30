#define V int
#define PFX intl
#define SNAME int_list
#define CMC_EXT_STR
#include "cmc/list.h"

#include "cmc/utl/futils.h"

int main() {
  struct int_list *list = intl_new(10, &(struct int_list_fval){.str = cmc_i32_str});

  intl_push_back(list, 1);
  intl_push_back(list, 2);
  intl_push_back(list, 3);
  intl_push_back(list, 3);
  intl_push_back(list, 2);
  intl_push_back(list, 2);
  intl_push_back(list, 1);

  intl_print(list, stdout, "[", ", ", "]\n");
}
