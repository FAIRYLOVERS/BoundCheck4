/* Generated by Frama-C */
struct __anonstruct___mpz_struct_1 {
   int _mp_alloc ;
   int _mp_size ;
   unsigned long *_mp_d ;
};
typedef struct __anonstruct___mpz_struct_1 __mpz_struct;
typedef __mpz_struct ( __attribute__((__FC_BUILTIN__)) mpz_t)[1];
typedef unsigned int size_t;
/*@ requires predicate ≢ 0;
    assigns \nothing; */
extern  __attribute__((__FC_BUILTIN__)) void e_acsl_assert(int predicate,
                                                           char *kind,
                                                           char *fct,
                                                           char *pred_txt,
                                                           int line);

/*@
model __mpz_struct { ℤ n };
*/
int __fc_random_counter __attribute__((__unused__));
unsigned long const __fc_rand_max = (unsigned long)32767;
/*@ ghost extern int __fc_heap_status; */

/*@
axiomatic
  dynamic_allocation {
  predicate is_allocable{L}(size_t n) 
    reads __fc_heap_status;
  
  }
 */
/*@ assigns \result \from *((char *)ptr+(0..size-1)); */
extern  __attribute__((__FC_BUILTIN__)) void *__store_block(void *ptr,
                                                            size_t size);

/*@ assigns \nothing; */
extern  __attribute__((__FC_BUILTIN__)) void __delete_block(void *ptr);

/*@ assigns \nothing; */
extern  __attribute__((__FC_BUILTIN__)) void __initialize(void *ptr,
                                                          size_t size);

/*@ assigns \nothing; */
extern  __attribute__((__FC_BUILTIN__)) void __full_init(void *ptr);

/*@ assigns \nothing; */
extern  __attribute__((__FC_BUILTIN__)) void __literal_string(void *ptr);

/*@ ensures \result ≡ 0 ∨ \result ≡ 1;
    ensures \result ≡ 1 ⇒ \valid((char *)\old(ptr)+(0..\old(size)-1));
    assigns \result \from *((char *)ptr+(0..size-1));
 */
extern  __attribute__((__FC_BUILTIN__)) int __valid(void *ptr, size_t size);

/*@ ensures \result ≡ 0 ∨ \result ≡ 1;
    ensures
      \result ≡ 1 ⇒ \valid_read((char *)\old(ptr)+(0..\old(size)-1));
    assigns \result \from *((char *)ptr+(0..size-1));
 */
extern  __attribute__((__FC_BUILTIN__)) int __valid_read(void *ptr,
                                                         size_t size);

/*@ assigns __e_acsl_internal_heap;
    assigns __e_acsl_internal_heap \from __e_acsl_internal_heap;
 */
extern  __attribute__((__FC_BUILTIN__)) void __e_acsl_memory_clean(void);

extern size_t __memory_size;

/*@
predicate diffSize{L1, L2}(ℤ i) =
  \at(__memory_size,L1)-\at(__memory_size,L2) ≡ i;
 */
/*@ assigns \at(\result,Post) \from *__format; */
extern int printf(char const * __restrict __format , ...);

/*@ assigns \at(\result,Post) \from *__s1, *__s2; */
extern  __attribute__((__nothrow__)) int strcmp(char const *__s1,
                                                char const *__s2) __attribute__((
__pure__, __nonnull__(1,2), __leaf__));

void bubble_sort(char **array, int size)
{
  char *__e_acsl_literal_string;
  char *temp;
  int i;
  int j;
  __store_block((void *)(& temp),4U);
  __store_block((void *)(& array),4U);
  i = 0;
  while (i < size) {
    j = 0;
    while (j < size) {
      int tmp;
      /*@ assert rte: mem_access: \valid_read(array+i); */
      {
        int __e_acsl_valid_read;
        __e_acsl_valid_read = __valid_read((void *)(array + i),
                                           sizeof(char *));
        e_acsl_assert(__e_acsl_valid_read,(char *)"Assertion",
                      (char *)"bubble_sort",
                      (char *)"rte: mem_access: \\valid_read(array+i)",18);
      }
      /*@ assert rte: mem_access: \valid_read(array+j); */
      {
        int __e_acsl_valid_read_2;
        __e_acsl_valid_read_2 = __valid_read((void *)(array + j),
                                             sizeof(char *));
        e_acsl_assert(__e_acsl_valid_read_2,(char *)"Assertion",
                      (char *)"bubble_sort",
                      (char *)"rte: mem_access: \\valid_read(array+j)",19);
      }
      tmp = strcmp((char const *)*(array + i),(char const *)*(array + j));
      if (tmp < 0) {
        /*@ assert rte: mem_access: \valid_read(array+i); */
        {
          int __e_acsl_valid_read_3;
          __e_acsl_valid_read_3 = __valid_read((void *)(array + i),
                                               sizeof(char *));
          e_acsl_assert(__e_acsl_valid_read_3,(char *)"Assertion",
                        (char *)"bubble_sort",
                        (char *)"rte: mem_access: \\valid_read(array+i)",22);
        }
        __full_init((void *)(& temp));
        temp = *(array + i);
        /*@ assert rte: mem_access: \valid(array+i); */
        {
          int __e_acsl_valid;
          __e_acsl_valid = __valid((void *)(array + i),sizeof(char *));
          e_acsl_assert(__e_acsl_valid,(char *)"Assertion",
                        (char *)"bubble_sort",
                        (char *)"rte: mem_access: \\valid(array+i)",24);
        }
        /*@ assert rte: mem_access: \valid_read(array+j); */
        {
          int __e_acsl_valid_read_4;
          __e_acsl_valid_read_4 = __valid_read((void *)(array + j),
                                               sizeof(char *));
          e_acsl_assert(__e_acsl_valid_read_4,(char *)"Assertion",
                        (char *)"bubble_sort",
                        (char *)"rte: mem_access: \\valid_read(array+j)",25);
        }
        __initialize((void *)(array + i),sizeof(char *));
        *(array + i) = *(array + j);
        /*@ assert rte: mem_access: \valid(array+j); */
        {
          int __e_acsl_valid_2;
          __e_acsl_valid_2 = __valid((void *)(array + j),sizeof(char *));
          e_acsl_assert(__e_acsl_valid_2,(char *)"Assertion",
                        (char *)"bubble_sort",
                        (char *)"rte: mem_access: \\valid(array+j)",27);
        }
        __initialize((void *)(array + j),sizeof(char *));
        *(array + j) = temp;
      }
      /*@ assert rte: signed_overflow: j+1 ≤ 2147483647; */
      e_acsl_assert((long long)j + (long long)1 <= (long long)2147483647,
                    (char *)"Assertion",(char *)"bubble_sort",
                    (char *)"rte: signed_overflow: j+1 <= 2147483647",30);
      j ++;
    }
    /*@ assert rte: signed_overflow: i+1 ≤ 2147483647; */
    e_acsl_assert((long long)i + (long long)1 <= (long long)2147483647,
                  (char *)"Assertion",(char *)"bubble_sort",
                  (char *)"rte: signed_overflow: i+1 <= 2147483647",33);
    i ++;
  }
  /*@ assert rte: mem_access: \valid_read(array+1); */
  {
    int __e_acsl_valid_read_5;
    __e_acsl_valid_read_5 = __valid_read((void *)(array + 1),sizeof(char *));
    e_acsl_assert(__e_acsl_valid_read_5,(char *)"Assertion",
                  (char *)"bubble_sort",
                  (char *)"rte: mem_access: \\valid_read(array+1)",36);
  }
  /*@ assert rte: mem_access: \valid_read(*(array+1)+2); */
  {
    int __e_acsl_valid_read_6;
    int __e_acsl_valid_read_7;
    __e_acsl_valid_read_6 = __valid_read((void *)(array + 1),sizeof(char *));
    e_acsl_assert(__e_acsl_valid_read_6,(char *)"RTE",(char *)"bubble_sort",
                  (char *)"mem_access: \\valid_read(array+1)",37);
    __e_acsl_valid_read_7 = __valid_read((void *)(*(array + 1) + 20),
                                         sizeof(char));
    e_acsl_assert(__e_acsl_valid_read_7,(char *)"Assertion",
                  (char *)"bubble_sort",
                  (char *)"rte: mem_access: \\valid_read(*(array+1)+2)",37);
  }
  __e_acsl_literal_string = "%c\n";
  __store_block((void *)__e_acsl_literal_string,sizeof("%c\n"));
  __full_init((void *)__e_acsl_literal_string);
  __literal_string((void *)__e_acsl_literal_string);
  printf(__e_acsl_literal_string,*(*(array + 1) + 20));
  __delete_block((void *)(& array));
  __delete_block((void *)(& temp));
  return;
}

int main(void)
{
  char *__e_acsl_literal_string_6;
  char *__e_acsl_literal_string_5;
  char *__e_acsl_literal_string_4;
  char *__e_acsl_literal_string_3;
  char *__e_acsl_literal_string_2;
  char *__e_acsl_literal_string;
  int __retres;
  char *values[5];
  int i;
  __store_block((void *)(values),20U);
  __e_acsl_literal_string = "AAA";
  __store_block((void *)__e_acsl_literal_string,sizeof("AAA"));
  __full_init((void *)__e_acsl_literal_string);
  __literal_string((void *)__e_acsl_literal_string);
  __initialize((void *)(values),sizeof(char *));
  values[0] = (char *)__e_acsl_literal_string;
  __e_acsl_literal_string_2 = "CCCD";
  __store_block((void *)__e_acsl_literal_string_2,sizeof("CCCD"));
  __full_init((void *)__e_acsl_literal_string_2);
  __literal_string((void *)__e_acsl_literal_string_2);
  __initialize((void *)(& values[1]),sizeof(char *));
  values[1] = (char *)__e_acsl_literal_string_2;
  __e_acsl_literal_string_3 = "BBBDE";
  __store_block((void *)__e_acsl_literal_string_3,sizeof("BBBDE"));
  __full_init((void *)__e_acsl_literal_string_3);
  __literal_string((void *)__e_acsl_literal_string_3);
  __initialize((void *)(& values[2]),sizeof(char *));
  values[2] = (char *)__e_acsl_literal_string_3;
  __e_acsl_literal_string_4 = "EEEDEF";
  __store_block((void *)__e_acsl_literal_string_4,sizeof("EEEDEF"));
  __full_init((void *)__e_acsl_literal_string_4);
  __literal_string((void *)__e_acsl_literal_string_4);
  __initialize((void *)(& values[3]),sizeof(char *));
  values[3] = (char *)__e_acsl_literal_string_4;
  __e_acsl_literal_string_5 = "DDDDEFG";
  __store_block((void *)__e_acsl_literal_string_5,sizeof("DDDDEFG"));
  __full_init((void *)__e_acsl_literal_string_5);
  __literal_string((void *)__e_acsl_literal_string_5);
  __initialize((void *)(& values[4]),sizeof(char *));
  values[4] = (char *)__e_acsl_literal_string_5;
  bubble_sort(values,5);
  i = 0;
  while (i < 5) {
    /*@ assert rte: index_bound: 0 ≤ i; */
    e_acsl_assert(0 <= i,(char *)"Assertion",(char *)"main",
                  (char *)"rte: index_bound: 0 <= i",55);
    /*@ assert rte: index_bound: i < 5; */
    e_acsl_assert(i < 5,(char *)"Assertion",(char *)"main",
                  (char *)"rte: index_bound: i < 5",56);
    __e_acsl_literal_string_6 = "%s ";
    __store_block((void *)__e_acsl_literal_string_6,sizeof("%s "));
    __full_init((void *)__e_acsl_literal_string_6);
    __literal_string((void *)__e_acsl_literal_string_6);
    printf(__e_acsl_literal_string_6,values[i]);
    /*@ assert rte: signed_overflow: i+1 ≤ 2147483647; */
    e_acsl_assert((long long)i + (long long)1 <= (long long)2147483647,
                  (char *)"Assertion",(char *)"main",
                  (char *)"rte: signed_overflow: i+1 <= 2147483647",58);
    i ++;
  }
  __retres = 1;
  __delete_block((void *)(values));
  __e_acsl_memory_clean();
  return __retres;
}


