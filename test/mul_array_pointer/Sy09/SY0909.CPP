/*****************************************************
 实验九  第 9 题
  题目：字符串由小到大排序
  要求： 用动态存储分配技术实现字符串的输入与排序　
  
  注意： 
******************************************************/
#include <iostream.h>
#include <string.h>
void  Input(char **p, int n); // n 字符串个数
void  Sort(char *p[ ],int n);/*选择法排序*/
void  Print(char **p, int n);
void  Free(char *p[ ], int n);
void main(void)
{ 
	char **p;
	int n;
	cout<<"请输入字符串个数: ";
	cin>>n;
	p=new char *[n]; //分配 n 个 char 型指针空间，p 指向该指针数组
	Input(p,n);      //输入
	Sort(p,n);       //排序
	cout<<endl;
	Print(p,n);      //输出
	Free(p,n);       //释放空间
}
void  Input(char **p, int n) // n 字符串个数
{ 
	char  tempstr[100]; 
	int i;
	for( i=0 ;i<n;i++)
	{
		cin>>tempstr;	
	    p[i]=new char[strlen(tempstr)+1];//分配一个串空间		
		strcpy(p[i],tempstr);
	}
}

void Sort(char *p[ ],int n)/*选择法排序*/
{ 
	char *temp; int i,j,k;
	for(i=0;i<n-1;i++)
	{
		k=i;
		for(j=i+1;j<n;j++)
			if( strcmp(p[k],p[j])>0 ) k=j;
		if(k!=i)
		{
			temp=p[i];p[i]=p[k];p[k]=temp;
		}
	} 
}

void  Print(char **p, int n)
{ 
	for(int i=0; i<n; i++)
		cout <<p[i] << "\n"; 
}

void  Free(char *p[ ], int n)
{ 
	for(int i=0; i<n; i++) //释放 n 个串空间
		delete [] p[i]; 
	delete[]p; //释放指针数组空间
}

