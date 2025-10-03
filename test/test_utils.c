#include <criterion/criterion.h>
#include "utils/arraylist.h"

// test for init array
Test(arraylist, init) {
  
  ArrayList* test_arr = initArrayList(10);
  cr_assert(test_arr != NULL);
  cr_assert(test_arr->length == 0);
  cr_assert(test_arr->capacity == 10);
}

// test for destorying array
Test(arraylist, destory) {
  ArrayList* test_arr = initArrayList(10);
  cr_assert(test_arr != NULL);
  
  int* num1 = malloc(sizeof(int));
  int* num2 = malloc(sizeof(int));
  *num1 = 1;
  *num2 = 2;
  appendArrayList(test_arr, num1);
  appendArrayList(test_arr, num2);
  int* index1 = (int*)getArrayList(test_arr, 0);
  int* index2 = (int*)getArrayList(test_arr, 1);
  cr_assert(index1 != NULL && *index1 == 1);
  cr_assert(index2 != NULL && *index2 == 2);

  destoryArrayList(test_arr);
}
