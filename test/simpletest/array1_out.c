#include"self2.h"
#include<string.h>
#include<stdio.h>
int t[10];
int u[8+3];
int v[16][17];

typedef struct _s {
  int t[15]; 
  struct {
    int u[12];
  }s;
  struct _s *next;
} ts;

ts s;

unsigned int c[10];

int main() 
{
  int i,j;
  unsigned int k;
  i=j=k=1;
  t[_RV_insert_check(0,10,23,5,"main",_RV_insert_check(0,10,23,5,"main",0))] = 0;
  u[_RV_insert_check(0,11,24,5,"main",_RV_insert_check(0,11,24,5,"main",1))] = 0;
  v[_RV_insert_check(0,16,25,5,"main",_RV_insert_check(0,16,25,5,"main",2))][_RV_insert_check(0,17,25,43,"main",_RV_insert_check(0,17,25,8,"main",3))] = 0;
  s.t[_RV_insert_check(0,15,26,7,"main",_RV_insert_check(0,15,26,7,"main",1))] = 0;
  s.s.u[_RV_insert_check(0,12,27,9,"main",_RV_insert_check(0,12,27,9,"main",2))] = 0;
  s.next->t[_RV_insert_check(0,15,28,13,"main",_RV_insert_check(0,15,28,13,"main",4))] = 0;

  t[_RV_insert_check(0,10,30,5,"main",_RV_insert_check(0,10,30,5,"main",i))] = 0;
  u[_RV_insert_check(0,11,31,5,"main",_RV_insert_check(0,11,31,5,"main",i))] = 0;
  v[_RV_insert_check(0,16,32,5,"main",_RV_insert_check(0,16,32,5,"main",i))][_RV_insert_check(0,17,32,43,"main",_RV_insert_check(0,17,32,8,"main",j))] = 0;
  s.t[_RV_insert_check(0,15,33,7,"main",_RV_insert_check(0,15,33,7,"main",i))] = 0;
  s.s.u[_RV_insert_check(0,12,34,9,"main",_RV_insert_check(0,12,34,9,"main",i))] = 0;
  s.next->t[_RV_insert_check(0,15,35,13,"main",_RV_insert_check(0,15,35,13,"main",j))] = 0;

  t[_RV_insert_check(0,10,37,5,"main",_RV_insert_check(0,10,37,5,"main",k))] = 0;
  u[_RV_insert_check(0,11,38,5,"main",_RV_insert_check(0,11,38,5,"main",k))] = 0;
  v[_RV_insert_check(0,16,39,5,"main",_RV_insert_check(0,16,39,5,"main",k))][_RV_insert_check(0,17,39,43,"main",_RV_insert_check(0,17,39,8,"main",c[_RV_insert_check(0,10,39,79,"main",_RV_insert_check(0,10,39,10,"main",k))]))] = 0;
  s.t[_RV_insert_check(0,15,40,7,"main",_RV_insert_check(0,15,40,7,"main",k))] = 0;
  s.s.u[_RV_insert_check(0,12,41,9,"main",_RV_insert_check(0,12,41,9,"main",k))] = 0;
  s.next->t[_RV_insert_check(0,15,42,13,"main",_RV_insert_check(0,15,42,13,"main",c[_RV_insert_check(0,10,42,50,"main",_RV_insert_check(0,10,42,15,"main",k))]))] = 0;
  c[_RV_insert_check(0,10,43,5,"main",_RV_insert_check(0,10,43,5,"main",k))]=0;
  return 0;
}

