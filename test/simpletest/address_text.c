#include<stdio.h>
#include<string.h>
void count_char(void *q_base,void *q_bound,char *q)
{
   printf("%p,%p,%p\n",q_base,q_bound,q);
}
void bubble_sort(void *_array_base,void *_array_bound,char *array[])
{
   printf("%p,%p,%p,%p\n",_array_base,_array_bound,array,(void *)array+1);
   printf("%p,%p,%p,%p\n",_array_base,_array_bound,array,(void *)(array+1));
   if((void *)array+1<_array_bound)
     printf("KKKKK\n");

}
void count_int(void *q_base,void *q_bound,int *q)
{
   printf("%p,%p,%p\n",q_base,q_bound,q);
}

int main()
{
   char a[5]={1,2,3,4,5};
   char string1[3][3]={{'a','d','f'},{'d','f','t'},{'e','t','g'}};
   char *string2[3]={"as","sdfsdsdfdsijh","asdasd"};
   char *values[] = {"AAA", "CCCs", "BBBsd", "EEEdfds", "DDDwewf"};
   int abc[3][3]={{1,2,3},{1,2,3},{1,2,3}};
   char *p;
   p=a;

   printf("%s\n",string1[1]);
   printf("strlen(string1[1])=%d\n",strlen(string1[1]));
   printf("strlen(string2[1])=%d\n",strlen(string2[1]));
   printf("%p,%p\n",&a,p);
   count_char(&a[0],&a[4],a);
   count_char(&string1[1][0],&string1[1][2],string1[1]);
   bubble_sort(&string2[0],&string2[2],string2);
   
//   count_char(&string2[1][0],&string2[1][0]+strlen(string2[1])*sizeof(char),string2[1]);
//   count_int(&abc[1][0],&abc[1][2],abc[1]);
//   printf("%d\n",sizeof(const char));
   return 1;
}
