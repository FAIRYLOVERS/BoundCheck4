#include<stdio.h>
#include<stdbool.h>
typedef unsigned int size_t;
extern  void *__store_block(void *ptr,size_t size);

extern  void __delete_block(void *ptr);

extern  void __initialize(void *ptr,size_t size);

extern  void __full_init(void *ptr);

extern  void __literal_string(void *ptr);

extern  int __valid(void *ptr, size_t size);

extern  int __valid_read(void *ptr,size_t size);

_Bool conver(int *d, int *b)
{
  int i;
  int c[5];
  __store_block((void *)(& d),4U);
  __store_block((void *)(& b),4U);
  __store_block((void *)c,20U);
  for(i=0;i<5;i++)
    {
      *((int *)__valid((void *)(d + i),sizeof(int)))=*((int *)__valid((void *)(b + i),sizeof(int)));
      *((int *)__valid((void *)(c + i),sizeof(int)))=*((int *)__valid((void *)(b + i),sizeof(int)));
     }
   
   return 1;
}

int main(void)
{
  int i;
  static int a[10];
  __store_block((void *)(a),40U);
  int num[5]={1,2,3,4,5};
  __store_block((void *)(num),20U);
  
  if(conver(a,num))
    for (i = 0; i <5; i++)
      printf("%d  ",*((int *)__valid((void *)(a + i),sizeof(int))));
  return 1;
}


