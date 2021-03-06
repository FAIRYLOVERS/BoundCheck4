#include "clang/AST/Type.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include<fstream>

#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Lexer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/AST/ParentMap.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"

using namespace clang;
using namespace std;

//行列号信息
struct Array_location_info
{
   int line;
   int column;
};

//实际插桩函数个数
int insert_total;
//可插桩点个数
int insert_total2;
// 是否在循环体中
bool m_inForLine=false;

//记录重复的指针访问
SourceRange Re_SR;

//记录是否是多维数组访问
SourceLocation Pre_Loc;

//指针定义时前一步的位置  
  SourceLocation previous; 

SourceLocation previous_end;

typedef std::vector<std::pair<ValueDecl *,ValueDecl *> >Pointer_vector;

// RecursiveASTVisitor is is the big-kahuna visitor that traverses
// everything in the AST.
class MyRecursiveASTVisitor: public ASTConsumer,public RecursiveASTVisitor<MyRecursiveASTVisitor>
{
public:
	MyRecursiveASTVisitor(Rewriter &R) : Rewrite(R) 
	{
		p1=0;
		issame="";
		isConstantArray=false;
		insert_total=0;
		insert_total2=0;
		str_count=0;
		memset(global,'\0',sizeof(global));
		Re_SR=SourceRange();
		Pre_Loc=SourceLocation();
		previous=SourceLocation();
		previous_end=SourceLocation();
	}
  bool VisitVarDecl(VarDecl *D);
  void getlineandcolumn(SourceLocation Loc);
  std::string lookforlength(std::string ss,int len,int p11);
  bool declaresSameEntity(const ValueDecl *D1, const ValueDecl *D2);
  
  int lookfordimension(std::string ss);
  int lookforlastdimension(std::string ss,int dimen,int sum_dimen);
  
  bool isSameSourceLocation(SourceLocation SL1,SourceLocation SL2);
  bool VisitBinAssign(BinaryOperator *Bo);
  bool VisitUnaryDeref(UnaryOperator *Uo);
  Expr* lookinBinaryOperator(BinaryOperator *Bo);
  bool VisitFunctionDecl(FunctionDecl *f);
  
  bool VisitCallExpr(CallExpr *CE);

  bool TraverseForStmt(ForStmt *s);
  
  bool VisitDeclStmt(DeclStmt *E);
  bool VisitImplicitCastExpr(ImplicitCastExpr *E);
  bool VisitBinComma(BinaryOperator *stmt);
  bool VisitArraySubscriptExpr(ArraySubscriptExpr *AS);
  bool SR_include_judge(SourceRange SR);
  std::string Get_N_1_Expr(std::string Org);
  std::string Get_I_Value(std::string ss,int i);
  bool isNestedinArray(SourceLocation loc);
  
  Rewriter &Rewrite;
//  CompilerInstance compiler;

private:
  Array_location_info Loc_info;
//得到固定数组的长度信息要用到的参数
  int p1;   
  SourceLocation SC2;
  SourceLocation SC3;
  SourceLocation SC1;
//首先遍历得到变量的缓冲区
  
  char fc4[256];
  char fc2[256];
 //插桩缓存区
  char fcd1[256];  
//用来存放全局变量的初始化
  char global[6144]; 

//字符串初始化时的父节点位置
  SourceLocation Parent_Loc;
//字符串计数变量
  int str_count;
//变量定义的范围  
  SourceRange SR;
  ASTContext *Context; 
//issame用来判断是不是处理到下一个数组了
  std::string issame;
  std::string fname;
//开始访问的是第几维
  int dimension;
  
//判断传递过去的是不是固定数组 
  bool isConstantArray;   
  ValueDecl *look_pointer;
//把函数声明传递过来,判断以后的插桩的是不是参数
  FunctionDecl *global_FD;  
//形成指针序列
  std::vector<std::pair<ValueDecl *,ValueDecl *> >P_vector; 
 

};

bool MyRecursiveASTVisitor::declaresSameEntity(const ValueDecl *D1, const ValueDecl *D2) {
  if (!D1 || !D2)
    return false;
  
  if (D1 == D2)
    return true;
  
  return D1->getCanonicalDecl() == D2->getCanonicalDecl();
}

bool MyRecursiveASTVisitor::isSameSourceLocation(SourceLocation SL1,SourceLocation SL2)
{
	SourceLocation SpellingLoc1 = Context->getSourceManager().getSpellingLoc(SL1);
	PresumedLoc PLoc1 = Context->getSourceManager().getPresumedLoc(SpellingLoc1);
	if (!PLoc1.isValid())
		return false;

	SourceLocation SpellingLoc2 = Context->getSourceManager().getSpellingLoc(SL2);
	PresumedLoc PLoc2 = Context->getSourceManager().getPresumedLoc(SpellingLoc2);
	if (!PLoc2.isValid())
		return false;

	if(PLoc1.getLine()==PLoc2.getLine()&&PLoc1.getColumn()==PLoc2.getColumn())
		return true;
	else 
		return false;
}

//ss表示类型值，len表示类型值的长度，p11从后往前访问的第一个[的位置
std::string MyRecursiveASTVisitor::lookforlength(std::string ss,int len,int p11)
{
   int p3,p4;
   if(p11==0)
      p3=len;
   else p3=p11;
   p3=ss.rfind(']',p3);
//   llvm::errs()<<p3<<"  2h\n";
   p4=ss.rfind('[',p3);
//   llvm::errs()<<p4<<"  1h\n"; 
   p1=p4;
   return ss.assign(ss,p4+1,p3-p4-1);
}


//求总共有多少维
int MyRecursiveASTVisitor::lookfordimension(std::string ss)
{
   int i,s=0;
   for(i=0;i<ss.length();i++)
   {
      if(ss.at(i)=='[')
      s++;
   }
   return s;
}



//得到数组访问时的维数位置,dimen要逃过的维数,sum_dimen总维数
int MyRecursiveASTVisitor::lookforlastdimension(std::string ss,int dimen,int sum_dimen)
{
   int locate,i;
   locate=ss.length();
   i=sum_dimen-dimen-1;
   while(i>0)
   {
      locate=ss.rfind(']',locate)-1;
      i--;
   }
   return locate;   
}


//Mark the father node when StringLiteral is exit
bool MyRecursiveASTVisitor::VisitDeclStmt(DeclStmt *E)
{
	Parent_Loc=E->getLocStart();
	getlineandcolumn(Parent_Loc);
	llvm::errs()<<" the parent\n";	
	return true;
}

//Rewrite the StringLiteral and Store it
bool MyRecursiveASTVisitor::VisitImplicitCastExpr(ImplicitCastExpr *E)
{
	char fc1[8192];
	char fc2[1024];
	SourceLocation Begin;
	if(ImplicitCastExpr *Str=dyn_cast<ImplicitCastExpr>(E->getSubExpr()))
		return true;

	Expr *sub_expr=E->getSubExpr()->IgnoreImpCasts();        
	if(StringLiteral *Str=dyn_cast<StringLiteral>(sub_expr)) 
	{
		Begin=Str->getLocStart();
		getlineandcolumn(Begin);
		llvm::errs()<<Str->getString()<<"  "<<Str->getLength()<<" this is StringLiteral\n";
		
		
		sprintf(fc1,"char *__tmp_string_%d=\"%s\";\n",str_count,Str->getString().str().data());
                sprintf(fc1,"%s __boundcheck_metadata_store((void *)(__tmp_string_%d),(void *)(__tmp_string_%d+%d));\n",fc1,str_count,str_count,Str->getLength());
		sprintf(fc2,"__tmp_string_%d",str_count);
                if(Parent_Loc.isValid())
                {
			llvm::errs()<<"  IIIIIIIIII\n";
			Rewrite.InsertTextBefore(Parent_Loc,fc1);
			Rewrite.ReplaceText(Begin,Str->getLength()+2,fc2);
			str_count++;
		}
	}
	return true;
}

bool MyRecursiveASTVisitor::VisitBinComma(BinaryOperator *stmt)
{
	llvm::errs()<<"2lhs==rhs\n";
	Expr *lhs=stmt->getLHS()->IgnoreImpCasts();
	Expr *rhs=stmt->getRHS()->IgnoreImpCasts();
	if (lhs->getType() == rhs->getType()) 
	{
		DeclRefExpr *dec1 = dyn_cast<DeclRefExpr>(lhs);
		DeclRefExpr *dec2 = dyn_cast<DeclRefExpr>(rhs);
		if(dec1&&dec2)		
		{
			VarDecl *var1=dyn_cast<VarDecl>(dec1->getDecl());
			VarDecl *var2=dyn_cast<VarDecl>(dec2->getDecl());
			if(var1&&var2)
			{
				llvm::errs()<<"lhs==rhs\n";
			}
		}
	}
	return true;

}

//得到前N-1维的表达式
std::string MyRecursiveASTVisitor::Get_N_1_Expr(std::string Org)
{
	int first,final,tmp1,tmp2,count=0;

	final=Org.length()+1;
//	llvm::errs()<<final<<"   TTTTTTTTTTTTTT\n";
	first=0;
	tmp1=tmp2=0;
	while(first<=final)
	{
		tmp1=Org.rfind(']',final-1);
		tmp2=Org.rfind('[',final-1);
		if(tmp1>tmp2)
		{
			count++;
			final=tmp1;
		}
		else
		{
			count--;
			final=tmp2;
		}
		if(count==0)
			break;
	}
//	llvm::errs()<<final<<"   KKKKKKKKKKKK\n";
	
//	llvm::errs()<<first<<" "<<final<<"  PPPPPPPPPPP\n";
//	llvm::errs()<<Org.substr(0,first)<<"  PPPPPPPPPP\n";
//	llvm::errs()<<Org.substr(0,final)<<"  PPPPPPPPPP\n";
	return Org.substr(0,final);
/*
	if(final==first)
		return Org.substr(0,first);
	else	return Org.substr(0,final);
*/
}

//判断数组是否嵌套
bool MyRecursiveASTVisitor::isNestedinArray(SourceLocation loc)
{
	PresumedLoc PLoc1 = Context->getSourceManager().getPresumedLoc(Context->getSourceManager().getSpellingLoc(Pre_Loc));
	PresumedLoc PLoc2 = Context->getSourceManager().getPresumedLoc(Context->getSourceManager().getSpellingLoc(loc));
	if((PLoc1.getLine()==PLoc2.getLine())&&(PLoc1.getColumn()==PLoc2.getColumn()))
		return true;
	return false;
}

//查找要插桩的数组并替换
bool MyRecursiveASTVisitor::VisitArraySubscriptExpr(ArraySubscriptExpr *AS)
{
	bool isdimen=false;
	SourceLocation Begin=AS->getLocStart();
	char fc[1024];
	char fc1[256];
	char fc4[256];
	Expr *Curr=NULL;
	std::string definition;
	std::string definition1;
	int sum_dimension;
	dimension=0;

	getlineandcolumn(Begin);
	std::string Org=Rewrite.ConvertToString(AS);
	SourceRange SR=AS->getSourceRange();
	
	if(Org.find("argv")!=-1)
		return true;

//	判断是否为嵌套访问
	if(Pre_Loc.isInvalid())
		Pre_Loc=Begin;
	else
	{
		if(isNestedinArray(Begin))
			isdimen=true;
		Pre_Loc=Begin;
	}
	

//	二维数组要插入的位置,如a[5],则在5的前面
	SC2=AS->getRHS()->getLocStart();
//	下标结束的位置
	SC3=AS->getRBracketLoc();
//	向左偏移一个单位，把]符号去掉
	SourceLocation SC3_left=Context->getSourceManager().getExpansionLoc(SC3).getLocWithOffset(-1);
//	下标
	std::string xiabiao=Rewrite.getRewrittenText(SourceRange(SC2,SC3_left));
//	得到行列号
	Expr *lhs=AS->getLHS();
//	得到不同维数的数组名
	while(ImplicitCastExpr *ICE2 = dyn_cast<ImplicitCastExpr>(lhs))
	{    
		Curr=ICE2->getSubExpr();
//		找到第一维        
		if(ArraySubscriptExpr *Decl2= dyn_cast<ArraySubscriptExpr>(Curr))
		{    
			lhs=Decl2->getLHS();
			llvm::errs()<<"LOOP\n";
			dimension++;
			continue;
		}
//		如果是结构成员
		else if(MemberExpr *Decl2= dyn_cast<MemberExpr>(Curr))
		{
//			当结构体的成员变量是固定长度数组时
			if(Decl2->getType().getTypePtr()->isConstantArrayType())
			{
				std::string definition=Decl2->getType().getAsString();
				sum_dimension=lookfordimension(definition);
				std::string definition1=lookforlength(definition,lookforlastdimension(definition,dimension,sum_dimension),0);  
				sprintf(fc1,"_RV_insert_check(0,%s,%d,%d,\"%s\",",definition1.data(),Loc_info.line,Loc_info.column,fname.c_str());
				sprintf(fc4,")");
				Rewrite.InsertText(SC2,fc1,true,true);
				Rewrite.InsertText(SC3,fc4,true,true); 
				insert_total++;
			}
			break;
		}
//     		如果是单个变量
		else if(DeclRefExpr *DICE5 = dyn_cast<DeclRefExpr>(Curr)) 
		{
//			DCT表示当前局部符号表
			DeclContext *DCT=DICE5->getDecl()->getDeclContext();
			ValueDecl *ND=DICE5->getDecl();

//			变量是固定长度数组
			if(ND->getType().getTypePtr()->isConstantArrayType())
			{
//				防止出现重复数组访问出现错误  
//				DeclContext::decl_iterator I = std::find(DCT->decls_begin(), DCT->decls_end(), ND);
//				if (I != DCT->decls_end())             
				for(DeclContext::decl_iterator I=DCT->decls_begin(),E=DCT->decls_end();I!=E;I++)
				{
					if(ValueDecl *ND2= dyn_cast<ValueDecl>(*I))
					{
						if(declaresSameEntity(ND,ND2))
						{
							definition=ND2->getType().getAsString();
							if(issame=="") 
								issame=ND2->getNameAsString();
							else if((issame!=ND2->getNameAsString())||(p1==definition.find('[')))
							{    
								p1=0; 
								issame=ND2->getNameAsString();
							}
//							求出数组的总维数
							sum_dimension=lookfordimension(definition);
							llvm::errs()<<definition<<" "<<ND->getNameAsString()<<"\n";
							llvm::errs()<<p1<<"  hhhhh\n";
//							这里lookforlength()函数最后一个参数可能是0也可能是p1                         
							definition1=lookforlength(definition,lookforlastdimension(definition,dimension,sum_dimension),0);

							llvm::errs()<<definition1<<"  OOOO\n";
							llvm::errs()<<"\n\n";
						}
					}                   
				}
//				如果找到长度
				if((definition1!="")&&(definition1.find('*')==-1)&&(sum_dimension-dimension-1>=0))
				{
					llvm::errs()<<"  AAAAAAAAAA 24 \n";
//					判断返回的长度值是不是类型值，如果是的话再判断是不是参数，如果是参数再返回类型值+size
					if(definition1!=definition)
					{
						sprintf(fc1,"_RV_insert_check(0,%s,%d,%d,\"%s\",",definition1.data(),Loc_info.line,Loc_info.column,fname.c_str());
                     				sprintf(fc4,")");
						Rewrite.InsertText(SC2,fc1,true,true);
						Rewrite.InsertText(SC3,fc4,true,true); 
						insert_total++;
					}
				}
				else if(!isdimen)
				{
//					llvm::errs()<<Org<<"  OOOOOOOOOOOOOOOOO\n";
//					llvm::errs()<<AS->getType().getAsString()<<"\n";
					llvm::errs()<<"数组访问：";
					getlineandcolumn(SR.getBegin());
					getlineandcolumn(SR.getEnd());
					llvm::errs()<<"\n";
//					if((AS->getType().getAsString().find('[')!=-1)||(AS->getType().getAsString().find('*')!=-1))
//						sprintf(fc,"(__boundcheck_ptr_reference(%d,%d,\"%s\",(void *)(&%s[0]),(void *)(&%s)))",Loc_info.line,Loc_info.column,fname.c_str(),Get_N_1_Expr(Org).data(),Org.data());
//					else
					sprintf(fc,"(*(%s *)(__boundcheck_ptr_reference(%d,%d,\"%s\",(void *)(&%s[0]),(void *)(&%s))))",AS->getType().getAsString().data(),Loc_info.line,Loc_info.column,fname.c_str(),Get_N_1_Expr(Org).data(),Org.data());
					llvm::errs()<<fc<<"\n";
					Rewrite.ReplaceText(SR,fc);
				}
				break;
			}
			else if(!isdimen)
			{
				llvm::errs()<<"数组访问2：";
				sprintf(fc,"(*(%s *)(__boundcheck_ptr_reference(%d,%d,\"%s\",(void *)(&%s[0]),(void *)(&%s))))",AS->getType().getAsString().data(),Loc_info.line,Loc_info.column,fname.c_str(),Get_N_1_Expr(Org).data(),Org.data());
				Rewrite.ReplaceText(SR,fc);
				break;
			}
			break;
			
		}
		else break;
	}  
	return true;
}

bool MyRecursiveASTVisitor::TraverseForStmt(ForStmt *s)
{
  if(!WalkUpFromForStmt(s))
     return false;
  m_inForLine=true;
  for(Stmt::child_range range=s->children();range;++range){
   if(*range==s->getBody())
      m_inForLine=false;
   TraverseStmt(*range);
  }
  return true;
}


//得到行列号
void MyRecursiveASTVisitor::getlineandcolumn(SourceLocation Loc)
{
    SourceLocation SpellingLoc = Context->getSourceManager().getSpellingLoc(Loc);
    PresumedLoc PLoc = Context->getSourceManager().getPresumedLoc(SpellingLoc);
    if (!PLoc.isValid())
        return ;
//    const char *Filename = PLoc.getFilename();
    Loc_info.line = PLoc.getLine();
    unsigned Line = PLoc.getLine();
    llvm::errs()<< Line << ':';
    Loc_info.column=PLoc.getColumn();
    llvm::errs()<< PLoc.getColumn()<<"   ";
}

//得到第i维的大小
std::string MyRecursiveASTVisitor::Get_I_Value(std::string ss,int i)
{
	int first,last,count;
	first=last=-1;	
	count=0;
	while(count<=i)
	{
		first=ss.find('[',first+1);
		last=ss.find(']',last+1);
		count++;
	}
//	llvm::errs()<<ss.substr(first+1,last-first-1)<<"  YYYYYYYYYYYYYYY\n";
	return ss.substr(first+1,last-first-1);
}

//add store command after every VarDecl
bool MyRecursiveASTVisitor::VisitVarDecl(VarDecl *VD) 
{     
	char fc[1024];
	char fc1[1024];
	char fc2[1024];
	char fc3[1024];
	char fc4[1024];
	char fc5[1024];
	char fc6[3072];
	char fc7[6144];
	int i=0;
	int j=0;
	bool isStruct=false;
	bool isGlobal=false;
	bool isConst=false;   
	
	Context=&(VD->getASTContext());
	SR=VD->getSourceRange();
	int sum_dimen=lookfordimension(VD->getType().getAsString());
  
//	当指针类型是结构体指针时做标记
	QualType typ=VD->getType();
	for(;typ->isPointerType(); typ = typ->getPointeeType());
	if(typ->isStructureType())
		isStruct=true;

	int offset = Lexer::MeasureTokenLength(Context->getSourceManager().getExpansionLoc(SR.getEnd()),Context->getSourceManager(),Context->getLangOpts());
//	end是指针定义的末尾
	SourceLocation end=Context->getSourceManager().getExpansionLoc(SR.getEnd()).getLocWithOffset(offset);
//	llvm::errs()<<VD->getNameAsString()<<"  SR ";
//	getlineandcolumn(SR.getBegin());
//	getlineandcolumn(SR.getEnd());
//	getlineandcolumn(end);

//      llvm::errs()<<VD->getType().getLocalQualifiers().getAsString()<<"  "<<VD->getNameAsString()<<"  1GGGGGG\n";
//	判断变量定义是否在For语句中
        if(!m_inForLine&&VD->getStorageClass()!=clang::SC_Register)
	{
		

//		llvm::errs()<<VD->getType().getAsString()<<"  "<<VD->getNameAsString()<<"  2GGGGGG\n";
//		getlineandcolumn(VD->getLocation());
//		getlineandcolumn(Context->getSourceManager().getExpansionLoc(VD->getLocation()).getLocWithOffset(-1));
//		对那些连续定义的变量，在前面加上类型,去除函数内参数 
		if(isSameSourceLocation(previous,VD->getLocStart())&&VD->isLocalVarDecl())
		{
//			llvm::errs()<<"YYYYYYYYYYYY\n";
			Rewrite.ReplaceText(previous_end,1,";");
//			Rewrite.RemoveText(Context->getSourceManager().getExpansionLoc(VD->getLocation()).getLocWithOffset(-1),1);
			Rewrite.InsertText(previous_end.getLocWithOffset(1),VD->getType().getAsString().substr(0,VD->getType().getAsString().find('[')>VD->getType().getAsString().find('*')?VD->getType().getAsString().find('*'):VD->getType().getAsString().find('['))+" ",true,true);
		}
		if(VD->getTypeSourceInfo()->getType().getTypePtr()->isConstantArrayType()||VD->getTypeSourceInfo()->getType()->isIncompleteArrayType())
		{
			isConst=true;
//			fc1记录新定义的计数变量
			memset(fc1,'\0',sizeof(fc1));
			while(i<sum_dimen-1)
			sprintf(fc1,"%s\nint __%s_%d;\n",fc1,VD->getNameAsString().data(),i++);

//			fc2记录for语句
			memset(fc2,'\0',sizeof(fc2));
			while(j<sum_dimen-1)
			{
				sprintf(fc2,"%s	for(__%s_%d=0;__%s_%d<%s;__%s_%d++)\n",fc2,VD->getNameAsString().data(),j,VD->getNameAsString().data(),j,Get_I_Value(VD->getType().getAsString(),j).data(),VD->getNameAsString().data(),j);
				j++;
			}

//			fc3,fc4,fc5分别记录内存块存储函数的函数名，第一个参数，第二个参数,fc6为整个内存块记录函数
			memset(fc3,'\0',sizeof(fc3));
			sprintf(fc3,"__boundcheck_metadata_store(");

//------------------------------------------------------------------------------------------------
//			fc4存放的是内存块记录函数的第一个参数				
			j=0;
			memset(fc4,'\0',sizeof(fc4));
			while(j<sum_dimen-1)
			sprintf(fc4,"%s[__%s_%d]",fc4,VD->getNameAsString().data(),j++);
//			当是一维数组时，第一个参数为:数组名[0];
			if(sum_dimen==1)
				sprintf(fc4,"[0]");
			else	sprintf(fc4,"%s[0]",fc4);

//------------------------------------------------------------------------------------------------			
//			fc5存放的是内存块记录函数的第二个参数
			j=0;
			memset(fc5,'\0',sizeof(fc5));
			while(j<sum_dimen-1)
				sprintf(fc5,"%s[__%s_%d]",fc5,VD->getNameAsString().data(),j++);
//			当是一维数组时，第二个参数是：数组名[N-1]
			if(sum_dimen==1)
				sprintf(fc5,"[%s-1]",Get_I_Value(VD->getType().getAsString(),j).data());
			else	sprintf(fc5,"%s[%s-1]",fc5,Get_I_Value(VD->getType().getAsString(),j).data());

//------------------------------------------------------------------------------------------------
//			fc6为整个内存块记录函数
			memset(fc6,'\0',sizeof(fc6));
			sprintf(fc6,"%s&%s%s,&%s%s);\n",fc3,VD->getNameAsString().data(),fc4,VD->getNameAsString().data(),fc5);

//			fc7存放多维数组要插入的所有数据,fc1存放计数变量，fc2存放for语句，fc6存放记录函数
			memset(fc7,'\0',sizeof(fc7));
			sprintf(fc7,"%s%s%s",fc1,fc2,fc6);
		}
//		局部变量，不包括函数参数
		if((VD->isLocalVarDecl()&&VD->hasLocalStorage())||VD->isStaticLocal())
		{
			if(isConst)
			{
//				Rewrite.ReplaceText(end.getLocWithOffset(1),1,"\n");
				Rewrite.InsertTextAfter(end.getLocWithOffset(1),fc7);			
			}
			else
			{
				sprintf(fc,"\n__boundcheck_metadata_store((void *)(&%s),(void *)((size_t)(&%s)+sizeof(%s)*8-1));\n",VD->getNameAsString().data(),VD->getNameAsString().data(),VD->getNameAsString().data());
//				Rewrite.RemoveText(end,1);
				Rewrite.InsertText(end.getLocWithOffset(1),fc);
			}
		}
//		是文件内的全局变量
		if(VD->hasGlobalStorage()&&!VD->hasExternalStorage()&&!VD->isStaticLocal())
		{
			if(isConst)
				sprintf(global,"%s%s",global,fc7);
			else
			{
				sprintf(fc,"\n__boundcheck_metadata_store((void *)(&%s),(void *)((size_t)(&%s)+sizeof(%s)*8-1));\n",VD->getNameAsString().data(),VD->getNameAsString().data(),VD->getNameAsString().data());
				sprintf(global,"%s%s",global,fc);
			}
		}
		previous=VD->getLocStart(); 
		previous_end=end;
  
	}	 
	return true;
}

//遍历赋值(=)表达式，上下界传递
bool MyRecursiveASTVisitor::VisitBinAssign(BinaryOperator *Bo)
{
	bool t1=false;
	ValueDecl *lNDe;
	ValueDecl *rNDe;
	char fc[256]; 
	SourceRange SR=Bo->getSourceRange();

	int offset = Lexer::MeasureTokenLength(Context->getSourceManager().getExpansionLoc(SR.getEnd()),Context->getSourceManager(),Context->getLangOpts());
//	end是赋值表达式的末尾
	SourceLocation end=Context->getSourceManager().getExpansionLoc(SR.getEnd()).getLocWithOffset(offset);

	Expr *elhs=Bo->getLHS()->IgnoreImpCasts();
	Expr *erhs=Bo->getRHS()->IgnoreImpCasts();
	std::string temp=Rewrite.ConvertToString(erhs);
//	llvm::errs()<<Rewrite.ConvertToString(erhs)<<"  LLLLLLLLLLLLLLL\n";
	if(!m_inForLine)
	if(DeclRefExpr *lDRF=dyn_cast<DeclRefExpr>(elhs))
	{
		DeclContext *DCTe=lDRF->getDecl()->getDeclContext();
		lNDe=lDRF->getDecl();
//		=左边的变量
		if(lNDe->getType().getTypePtr()->isPointerType())
		{
			llvm::errs()<<lNDe->getNameAsString()<<"  参数值1 ";
			t1=true;
		} 
//		=右边的变量
//		处理形如q=buf+i+3这种情况
		if(t1)
		{
			while(BinaryOperator *Bop=dyn_cast<BinaryOperator>(erhs))
			{
				Expr *Ex=Bop->getLHS()->IgnoreImpCasts();
				llvm::errs()<<Rewrite.ConvertToString(Ex)<<"  LLLLLLLLLLLLLLL\n";                 
				if(BinaryOperator *Bop=dyn_cast<BinaryOperator>(Ex))
				{
					erhs=Ex;
					continue;
				}
				if(DeclRefExpr *DRf1=dyn_cast<DeclRefExpr>(Ex))
				{
					DeclContext *DCTe=DRf1->getDecl()->getDeclContext();
					rNDe=DRf1->getDecl();
					if(rNDe->getType().getTypePtr()->isPointerType())
					{
						llvm::errs()<<rNDe->getNameAsString()<<"  参数值2\n";
						sprintf(fc,";\n__boundcheck_metadata_trans_check((void *)(%s),(void *)(%s),(void *)(%s));\n",lNDe->getNameAsString().data(),rNDe->getNameAsString().data(),Rewrite.ConvertToString(erhs).data());
						Rewrite.RemoveText(end,1);
						Rewrite.InsertTextAfter(end,fc);   
						break;
					} 
					else if(rNDe->getType().getTypePtr()->isConstantArrayType())
					{
						std::string definition=Ex->IgnoreImpCasts()->getType().getAsString();
						definition=lookforlength(definition,definition.length(),0);
						sprintf(fc,";\n__boundcheck_metadata_trans_check((void *)(%s),(void *)(%s),(void *)(%s));\n",lNDe->getNameAsString().data(),rNDe->getNameAsString().data(),Rewrite.ConvertToString(erhs).data());
						Rewrite.RemoveText(end,1);
						Rewrite.InsertTextAfter(end,fc);
						break;
					}
					else break; 
				}
				else break;
			}
//			处理形如q=p这种情况
			if(DeclRefExpr *rDRF=dyn_cast<DeclRefExpr>(erhs))
			{
				DeclContext *DCTe=rDRF->getDecl()->getDeclContext();
				rNDe=rDRF->getDecl();
				if(rNDe->getType().getTypePtr()->isPointerType())
				{
					llvm::errs()<<rNDe->getNameAsString()<<"  参数值2\n";
					sprintf(fc,";\n__boundcheck_metadata_trans_check((void *)(%s),(void *)(%s),(void *)(%s));\n",lNDe->getNameAsString().data(),rNDe->getNameAsString().data(),Rewrite.ConvertToString(erhs).data());
					Rewrite.RemoveText(end,1);
					Rewrite.InsertTextAfter(end,fc);   
				} 
				if(rNDe->getType().getTypePtr()->isConstantArrayType())
				{
					std::string definition=erhs->IgnoreImpCasts()->getType().getAsString();
					definition=lookforlength(definition,definition.length(),0);
					sprintf(fc,";\n__boundcheck_metadata_trans_check((void *)(%s),(void *)(%s),(void *)(%s));\n",lNDe->getNameAsString().data(),rNDe->getNameAsString().data(),Rewrite.ConvertToString(erhs).data());
					Rewrite.RemoveText(end,1);
					Rewrite.InsertTextAfter(end,fc);
				}
			}
			if(MemberExpr *rDRF=dyn_cast<MemberExpr>(erhs))
			{
				std::string definition=rDRF->getType().getAsString();
				std::string definition1=lookforlength(definition,definition.length(),0);
              		}
//			处理强制类型转换，如=(int)f  长度转换还有问题
			if(CStyleCastExpr *Imp=dyn_cast<CStyleCastExpr>(erhs))
			{
				Expr *subexpr=Imp->getSubExpr()->IgnoreImpCasts();
				if(DeclRefExpr *rDRF=dyn_cast<DeclRefExpr>(subexpr))
				{
					DeclContext *DCTe=rDRF->getDecl()->getDeclContext();
					rNDe=rDRF->getDecl();
					if(rNDe->getType().getTypePtr()->isPointerType())
					{
						llvm::errs()<<rNDe->getNameAsString()<<"  参数值2\n";
						sprintf(fc,";\n__boundcheck_metadata_trans_check((void *)(%s),(void *)(%s),(void *)(%s));\n",lNDe->getNameAsString().data(),rNDe->getNameAsString().data(),Rewrite.ConvertToString(erhs).data());
						Rewrite.RemoveText(end,1);
						Rewrite.InsertTextAfter(end,fc);
					} 
				}   
			}
			if(temp.find("&")==0)
 			{
			}
		}
	}     
	return true;
}

bool MyRecursiveASTVisitor::SR_include_judge(SourceRange SR)
{
	SourceLocation loc_B1=Re_SR.getBegin();
	SourceLocation loc_E1=Re_SR.getEnd();
	SourceLocation loc_B2=SR.getBegin();
	SourceLocation loc_E2=SR.getEnd();
	PresumedLoc PLoc_B1 = Context->getSourceManager().getPresumedLoc(Context->getSourceManager().getSpellingLoc(loc_B1));
	PresumedLoc PLoc_E1 = Context->getSourceManager().getPresumedLoc(Context->getSourceManager().getSpellingLoc(loc_E1));
	PresumedLoc PLoc_B2 = Context->getSourceManager().getPresumedLoc(Context->getSourceManager().getSpellingLoc(loc_B2));
	PresumedLoc PLoc_E2 = Context->getSourceManager().getPresumedLoc(Context->getSourceManager().getSpellingLoc(loc_E2));
	if((PLoc_B1.getLine()==PLoc_B2.getLine())&&(PLoc_B1.getColumn()<=PLoc_B2.getColumn())&&(PLoc_E1.getColumn()>=PLoc_E2.getColumn()))
		return true;
	return false;
}

//访问指针表达式
bool MyRecursiveASTVisitor::VisitUnaryDeref(UnaryOperator *Uo)
{ 
	
	UnaryOperator::Opcode Opc=Uo->getOpcode();
	StringRef Str=Uo->getOpcodeStr(Opc);
	SourceRange SR=Uo->getSourceRange();
	Expr *Ex2;
	char fc1[256];
	char fc2[256];
	bool isrepeat=false;

	std::string Org=Rewrite.ConvertToString(Uo);
	if(Org.find("argv")!=-1)
		return true;

//	判断是否为嵌套访问
	if(Re_SR.isInvalid())
		Re_SR=SR;
	else
	{
		if(SR_include_judge(SR))
			isrepeat=true;
		Re_SR=SR;
	}
//	得到形如*（P+4）最后要插入括号的地方
	SourceLocation SL_end=SR.getEnd();

	Expr *Ex1=Uo->getSubExpr()->IgnoreImpCasts(); 
	SourceLocation SL_begin=Uo->getOperatorLoc();
	int offset1 = Lexer::MeasureTokenLength(SL_begin,
                                           Context->getSourceManager(),
                                           Context->getLangOpts());
	int offset2 = Lexer::MeasureTokenLength(SL_end,
                                           Context->getSourceManager(),
                                           Context->getLangOpts());
   
//	得到要插入桩函数的地方
	SourceLocation SL_new_begin= SL_begin.getLocWithOffset(offset1);
//	得到形如*p，*--p,*p++最后要插入括号的地方
	SourceLocation SL_new_end=SL_end.getLocWithOffset(offset2);
	llvm::errs()<<"DSASSSSSSSS\n";	
	getlineandcolumn(SR.getBegin());
	getlineandcolumn(SR.getEnd());
    
	insert_total2++;

//	处理形如：*（p+4） *(4+p)这种情况没考虑
	Ex1=Ex1->IgnoreParens()->IgnoreImpCasts();
	
    	if(!isrepeat&&Str=="*")
	{
		if(BinaryOperator *Bop=dyn_cast<BinaryOperator>(Ex1))
		{
			Expr *l_expr=Bop->getLHS();
			Expr *r_expr=Bop->getRHS();
			sprintf(fc1,"(%s)(__boundcheck_ptr_reference(%d,%d,\"%s\",(void *)(%s),(void *)",Ex1->getType().getAsString().data(),Loc_info.line,Loc_info.column,fname.c_str(),Rewrite.ConvertToString(l_expr).data());
			Rewrite.InsertText(SL_new_begin,fc1,true,true);
			sprintf(fc2,"))");
			Rewrite.InsertText(SL_new_end,fc2,true,true);
			return true;   
		}
		if(UnaryOperator *Bop=dyn_cast<UnaryOperator>(Ex1))
		{
			if(Bop->isPostfix()||Bop->isPrefix())
			{
				sprintf(fc1,"(%s)(__boundcheck_ptr_reference(%d,%d,\"%s\",(void *)(%s),(void *)(",Ex1->getType().getAsString().data(),Loc_info.line,Loc_info.column,fname.c_str(),Rewrite.ConvertToString(Bop->getSubExpr()).data());
   				Rewrite.InsertText(SL_new_begin,fc1,true,true);
   				sprintf(fc2,")))");
   				Rewrite.InsertText(SL_new_end,fc2,true,true);
			}
			else
			{
				sprintf(fc1,"(%s)(__boundcheck_ptr_reference(%d,%d,\"%s\",(void *)(%s),(void *)",Ex1->getType().getAsString().data(),Loc_info.line,Loc_info.column,fname.c_str(),Rewrite.ConvertToString(Ex1).data());
				Rewrite.InsertText(SL_new_begin,fc1,true,true);
				sprintf(fc2,"))");
				Rewrite.InsertText(SL_new_end,fc2,true,true);
			}	
		}                           
//		处理形如：*p
		else if(DeclRefExpr *DRf1=dyn_cast<DeclRefExpr>(Ex1))
		{
			ValueDecl *NDe=DRf1->getDecl();
			sprintf(fc1,"(%s)(__boundcheck_ptr_reference(%d,%d,\"%s\",(void *)(%s),(void *)(",NDe->getType().getAsString().data(),Loc_info.line,Loc_info.column,fname.c_str(),NDe->getNameAsString().data());
   			Rewrite.InsertText(SL_new_begin,fc1,true,true);
   			sprintf(fc2,")))");
   			Rewrite.InsertText(SL_new_end,fc2,true,true);
			return true;
		}
		else
		{
			sprintf(fc1,"(%s)(__boundcheck_ptr_reference(%d,%d,\"%s\",(void *)(%s),(void *)",Ex1->getType().getAsString().data(),Loc_info.line,Loc_info.column,fname.c_str(),Rewrite.ConvertToString(Ex1).data());
			Rewrite.InsertText(SL_new_begin,fc1,true,true);
			sprintf(fc2,"))");
			Rewrite.InsertText(SL_new_end,fc2,true,true);
			return true;
		}
	}
	return true;
}

//从复杂表达式的右边往左边依次遍历，当遇到指针或数组变量时终止，并返回该变量
Expr* MyRecursiveASTVisitor::lookinBinaryOperator(BinaryOperator *Bo)
{
	BinaryOperator *Bo_expr;
	Expr *Elx,*Erx;
	Bo_expr=Bo;
	while(isa<BinaryOperator>(Bo_expr))
	{
		llvm::errs()<<Rewrite.ConvertToString(Bo_expr)<<"  LLLLLLLLLLLLLLL1\n";
		Elx=Bo_expr->getLHS()->IgnoreImpCasts();
		Erx=Bo_expr->getRHS()->IgnoreImpCasts();
		llvm::errs()<<Rewrite.ConvertToString(Elx)<<"  LLLLLLLLLLLLLLL2\n";
       
		if(DeclRefExpr *DRf1=dyn_cast<DeclRefExpr>(Erx))
		{
//			llvm::errs()<<"  LLLLLLLLLLLLLLL3\n";
			DeclContext *DCTe=DRf1->getDecl()->getDeclContext();
			ValueDecl *rNDe=DRf1->getDecl();
 			if(rNDe->getType().getTypePtr()->isPointerType()||rNDe->getType().getTypePtr()->isConstantArrayType())
			{
				llvm::errs()<<rNDe->getNameAsString()<<"  参数值2\n";
				return DRf1;  
			}
		}                 
		if(BinaryOperator *Bop=dyn_cast<BinaryOperator>(Elx))
		{
			llvm::errs()<<Rewrite.ConvertToString(Bop)<<"  LLLLLLLLLLLLLLL3\n";
			Bo_expr=Bop;
			continue;
		}
		if(DeclRefExpr *DRf1=dyn_cast<DeclRefExpr>(Elx))
		{
			llvm::errs()<<"  LLLLLLLLLLLLLLL3\n";
			DeclContext *DCTe=DRf1->getDecl()->getDeclContext();
			ValueDecl *rNDe=DRf1->getDecl();
			if(rNDe->getType().getTypePtr()->isPointerType()||rNDe->getType().getTypePtr()->isConstantArrayType())
			{
				llvm::errs()<<rNDe->getNameAsString()<<"  参数值2\n";
				return DRf1;  
			}
			break;  
		}
		return 	Elx;
	}
	return Bo;
}

//Insert a function for dealing with global variables before Main()
bool MyRecursiveASTVisitor::VisitFunctionDecl(FunctionDecl *f)
{ 
	Parent_Loc=SourceLocation();
	global_FD=f;
	char fc1[10000],fc2[256];
	char fc3[1024];
	SourceLocation body_begin;
	SourceLocation new_body_begin;
	DeclarationNameInfo dni = f->getNameInfo();
	DeclarationName dn = dni.getName();
	fname = dn.getAsString();
  
	SourceLocation begin=f->getLocStart();
	

	if (f->hasBody()&&f->isMain()&&(strcmp(global,"")!=0))
	{
		body_begin=f->getBody()->getLocStart();
	        new_body_begin= body_begin.getLocWithOffset(1);
		sprintf(fc1,"void __global_variables_init(){\n%s}\n",global);
		Rewrite.InsertTextBefore(begin,fc1);
		sprintf(fc2,"\n	__global_variables_init();\n");
		Rewrite.InsertTextBefore(new_body_begin,fc2);
		memset(global,'\0',sizeof(global));
	}
/*
	if(f->hasBody()&&f->isMain())
	{
		body_begin=f->getBody()->getLocStart();
	        new_body_begin= body_begin.getLocWithOffset(1);
		sprintf(fc3,"\n__boundcheck_metadata_store((void *)&argv[0],(void *)&argv[argc-1]);\n");
		Rewrite.InsertTextBefore(new_body_begin,fc3);
	}
*/
	return true;
}

//函数调用时改写
bool MyRecursiveASTVisitor::VisitCallExpr(CallExpr *CE)
{
	Parent_Loc=SourceLocation();
	char fc[256];
	if(FunctionDecl *FD=CE->getDirectCallee())   
		if(FD->getLocStart().isValid())  
		{  
//			Parent_Loc=CE->getLocStart();
//			llvm::errs()<<FD->getNameInfo().getAsString()<<" PPPPPPPPPPPP\n";
//			strcpy(char *dest,char *src) 复制src到dest
			if(FD->getNameInfo().getAsString()=="strcpy")
			{
				sprintf(fc,"__boundcheck_");
				Rewrite.InsertTextBefore(CE->getLocStart(),fc);
			}
			if(FD->getNameInfo().getAsString()=="strncpy")
			{
				sprintf(fc,"__boundcheck_");
				Rewrite.InsertTextBefore(CE->getLocStart(),fc);
			}

//			strcat(char *dest,char *src) 将src复制在dest末尾
			if(FD->getNameInfo().getAsString()=="strcat")
			{
				sprintf(fc,"__boundcheck_");
				Rewrite.InsertTextBefore(CE->getLocStart(),fc);
			}
			if(FD->getNameInfo().getAsString()=="strncat")
			{
				sprintf(fc,"__boundcheck_");
				Rewrite.InsertTextBefore(CE->getLocStart(),fc);
			}
//			如果是malloc函数，则改写函数调用名为__boundcheck_malloc
			if(FD->getNameInfo().getAsString()=="malloc")
			{
				sprintf(fc,"__boundcheck_");
				Rewrite.InsertTextBefore(CE->getLocStart(),fc); 
			}
			if(FD->getNameInfo().getAsString()=="free")
			{
				sprintf(fc,"__boundcheck_");
				Rewrite.InsertTextBefore(CE->getLocStart(),fc); 
			}
			if(FD->getNameInfo().getAsString()=="memcpy")
			{
				sprintf(fc,"__boundcheck_");
				Rewrite.InsertTextBefore(CE->getLocStart(),fc); 
			}
			if(FD->getNameInfo().getAsString()=="memmove")
			{
				sprintf(fc,"__boundcheck_");
				Rewrite.InsertTextBefore(CE->getLocStart(),fc); 
			}

//	非字符串拷贝函数
//     memcpy(char *dest,char *src,size_t count) 从src复制count个字符到dest
//     memmove(char *dest,char *src,size_t count) 从src复制count个字符串到dest中
/* 
       if(FD->getNameInfo().getAsString()=="memcpy")
       {
         sprintf(fc,"__boundcheck_");
         Rewrite.InsertTextBefore(CE->getLocStart(),fc);
       }
*/
		}
    return true;
} 
 
class MyASTConsumer : public ASTConsumer
{
 public:

  MyASTConsumer(Rewriter &Rewrite){ }
  virtual void HandleTranslationUnit(ASTContext &Ctx);

  private:
  Rewriter Rewrite;
};


void MyASTConsumer::HandleTranslationUnit(ASTContext &Ctx) 
{
  Rewrite.setSourceMgr(Ctx.getSourceManager(), Ctx.getLangOpts());
  MyRecursiveASTVisitor Visitor(Rewrite);
  Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
 // output(Rewrite);
  Rewrite.overwriteChangedFiles();
  
  for(Rewriter::buffer_iterator b=Rewrite.buffer_begin(),e=Rewrite.buffer_end();b!=e;b++)
  {
//     llvm::errs()<<Ctx.getSourceManager().getFileEntryForID(b->first)->getName()<<"\n";
   }

  
}

// function for checking option arguments
void checkOptionArgument(int argc, char *argv[], set<string> &includeDirSet,
                         bool &isBoundCheck) {
  int opt = 0;
  const char *optString = ":I:hb";
  string includeDir;
  while((opt = getopt(argc, argv, optString)) != -1) {
    switch(opt) {
      case 'I':
        includeDir.assign(optarg);
//        includeDir = "-I"+ includeDir;
        //no need to check include options and no back up
        includeDirSet.insert(includeDir);
        break;
      case 'h':
        llvm::errs() << "This program should be used like this.\n";
        llvm::errs() << "option -I to set the include directory.\n";
        llvm::errs() << "option -b to invoke the boundcheck.\n";
        llvm::errs() << "option -h to get help.\n";
        exit(1);
        break;
      case 'b':
        isBoundCheck = true;
        break;
      case ':':
        llvm::errs() << "The option: " << (char)optopt << "need a value.\n";
        exit(1);
        break;
      case '?':
        llvm::errs() << "Unknown option: " << (char)optopt << "\n";
        exit(1);
        break;
      default:
        break;
    }
  }
}

// funtion for checking dir or file argument provided
int checkDirFile(string dirFilePath) {
  //check whether the arguments are dirs and exist
  int checkResult = 0;
  struct stat statBuf;

  if(stat(dirFilePath.c_str(), &statBuf) < 0) {
    checkResult = 0;
  } else if(S_ISDIR(statBuf.st_mode)) {
    checkResult = 1;
  } else if(S_ISREG(statBuf.st_mode)) {
    checkResult = 2;
  } else {
    checkResult = 0;
  }
  
  return checkResult;
}

void copyFile(string filePath, string desPath, set<string> &filePathSet,
                set<string> &allPathSet) {
  allPathSet.insert(desPath);
  if(desPath.rfind(".c") == (desPath.size() - 2)||desPath.rfind(".h") == (desPath.size() - 2)) {
    filePathSet.insert(desPath);
    llvm::errs()<<desPath<<"  filePathSet\n";
  }
  int sfd = 0, dfd = 0;
  int len = 0;
  char buff[512] = {0};
  sfd = open(filePath.c_str(), O_RDONLY);
  if(sfd < 0) {
    llvm::errs() << "open file " << filePath << " failed.\n";
    exit(1);
  }
  dfd = open(desPath.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
  if(dfd < 0) {
    llvm::errs() << "open file " << desPath << " failed.\n";
    exit(1);
  }
  do {
    len = read(sfd, buff, sizeof(buff));
    if(len > 0) {
      len = write(dfd, buff, len);
    }
  } while(len > 0);
  close(sfd);
  close(dfd);
}

// copy directories to /tmp
void copyDir(string dirPath, string desPath, set<string> &includeDirSet,
               set<string> &filePathSet, set<string> &allPathSet) {
  size_t posFoundLow = desPath.rfind("/include");
  size_t posFoundUp = desPath.rfind("/INCLUDE");
//  if(posFoundLow == desPath.size() - 8 || posFoundUp == desPath.size() - 8) {
//    string opIncludeDir = "-I" + desPath;
//    includeDirSet.insert(opIncludeDir);
//  }
  includeDirSet.insert(desPath);
  struct dirent *pEnt = NULL;
  DIR *pSrcDir = opendir(dirPath.c_str());
  if(pSrcDir == NULL) {
    llvm::errs() << "open directory " << dirPath << " failed.\n";
    exit(1);
  }
  if(!opendir(desPath.c_str())) { 
    if(mkdir(desPath.c_str(), S_IRWXU) != 0) {
      llvm::errs() << "error  mkdir " << desPath << " .\n";
      exit(1);
    }
  }
  string dirName, nDirPath, nDesPath;
  while((pEnt = readdir(pSrcDir)) != NULL) {
    switch(pEnt->d_type) {
      case DT_REG:
        nDirPath = dirPath + "/" + pEnt->d_name;
        nDesPath = desPath + "/" + pEnt->d_name;
        copyFile(nDirPath, nDesPath, filePathSet, allPathSet);
        break;
      case DT_DIR:
        dirName = pEnt->d_name;
        if(dirName.compare(".") != 0 && dirName.compare("..") != 0) {
          nDirPath = dirPath + "/" + pEnt->d_name;
          nDesPath = desPath + "/" + pEnt->d_name;
          copyDir(nDirPath, nDesPath, includeDirSet, filePathSet, allPathSet);
        }
        break;
      default:
        break;
    }
  }
  closedir(pSrcDir);
}

void rmDir(string dirPath) { 
  struct dirent *pEnt = NULL;
  DIR *pSrcDir = opendir(dirPath.c_str());
  if(pSrcDir == NULL) {
    return;
  }
  string nDirPath;
  while((pEnt = readdir(pSrcDir)) != NULL) {
    switch(pEnt->d_type) {
      case DT_DIR:
        nDirPath = pEnt->d_name;
        if(nDirPath.compare(".") != 0 && nDirPath.compare("..") != 0) {
          nDirPath = dirPath + "/" + nDirPath;
          rmDir(nDirPath);
        }
        break;
      default:
        nDirPath = dirPath + "/" + pEnt->d_name;
        if(remove(nDirPath.c_str()) != 0) {
          llvm::errs() << "delete error.\n";
        }
        break;
    }
  }
  closedir(pSrcDir);
  if(remove(dirPath.c_str()) != 0) {
    llvm::errs() << "delete error.\n";
  }
}

// function for checking file arguments
void checkFileDirArgument(int argc, char *argv[],
                          set<string> &includeDirSet, set<string> &filePathSet,
                          set<string> &allPathSet) {
  for(;optind < argc; ++optind) {
    string dirFilePath = argv[optind];
//    llvm::errs()<<dirFilePath<<"  KKKKKKKK\n";
    if(checkDirFile(dirFilePath) == 0) {
      llvm::errs() << "Please check your arguments, file or diectory not exist.\n";
      exit(1);
    }
    size_t ext = dirFilePath.rfind("/");
    if(ext == dirFilePath.size() - 1 && ext != 0) {
      dirFilePath.erase(ext, 1);
      ext = dirFilePath.rfind("/");
    }
//    llvm::errs()<<dirFilePath<<"  2KKKKKKKK\n";
    string dirPart, filePart;// 目录部分，文件部分
    if(ext == string::npos) {
      dirPart = dirFilePath;
    } else {
      dirPart = dirFilePath.substr(0, ext);
      filePart = dirFilePath.substr(ext, dirFilePath.size() - ext);
    }

    //first get the actual path of dirFilePath
    char curDir[512] = {0};
    getcwd(curDir, sizeof(curDir));
    string curDirPath = curDir;
//    llvm::errs()<<curDirPath<<"  curDirPath\n";
    chdir(dirPart.c_str());
    getcwd(curDir, sizeof(curDir));
//    llvm::errs()<<curDir<<"  curDir\n";
    chdir(curDirPath.c_str());
    dirPart = curDir;
    dirFilePath = dirPart + filePart;
//    llvm::errs()<<dirFilePath<<"  dirFilePath\n";
    //end get the actual path of dirFilePath

/*  
    string desPath = dirFilePath;
    ext = desPath.rfind("/");
    if(ext != std::string::npos) {
      desPath.erase(0, ext + 1);
    }
    llvm::errs()<<desPath<<"  desPath\n";
    desPath = "/tmp/" +desPath;
*/
    string desPath=dirFilePath+"_out";
//    llvm::errs()<<desPath<<"  LLLL\n";
//    desPath.append("_out");
    switch(checkDirFile(dirFilePath)) {
      // directory
      case 1:
        rmDir(desPath);
        copyDir(dirFilePath, desPath, includeDirSet, filePathSet, allPathSet);
        break;
      // regular file
      case 2:
        remove(desPath.c_str());
        copyFile(dirFilePath, desPath, filePathSet, allPathSet);
        break;
      default:
        break;
    }
  }
}


void backupMacroedFiles(set<string> &allPathSet) {
  for(set<string>::iterator itPath = allPathSet.begin();
        itPath != allPathSet.end(); ++itPath) {
    string filePath = *itPath;
    string desPath = filePath + ".tmp";
    set<string> temp1,temp2;
    copyFile(filePath, desPath, temp1, temp2);
  }
}

void boundcheck(set<string> includeDirSet,set<string> filePathSet)
{
  struct stat sb;  
  set<string>::iterator vp=filePathSet.begin();
  

  while(vp!=filePathSet.end())
  {
     set<string>::iterator vp_inclu=includeDirSet.begin();
     llvm::errs()<<"\nPrecessing "<<*vp<<"\n";
     // Get filename
     std::string fileName(*vp);

     CompilerInstance compiler;
     DiagnosticOptions diagnosticOptions;
     compiler.createDiagnostics();

     // Set default target triple
     llvm::IntrusiveRefCntPtr<TargetOptions> pto( new TargetOptions());
     pto->Triple = llvm::sys::getDefaultTargetTriple();
     llvm::IntrusiveRefCntPtr<TargetInfo>
     pti(TargetInfo::CreateTargetInfo(compiler.getDiagnostics(),
                                      pto.getPtr()));
     compiler.setTarget(pti.getPtr());

     compiler.createFileManager();
     compiler.createSourceManager(compiler.getFileManager());

     HeaderSearchOptions &headerSearchOptions = compiler.getHeaderSearchOpts();

  // <Warning!!> -- Platform Specific Code lives here
  // This depends on A) that you're running linux and
  // B) that you have the same GCC LIBs installed that
  // I do.
  // Search through Clang itself for something like this,
  // go on, you won't find it. The reason why is Clang
  // has its own versions of std* which are installed under
  // /usr/local/lib/clang/<version>/include/
  // See somewhere around Driver.cpp:77 to see Clang adding
  // its version of the headers to its include path.
  // To see what include paths need to be here, try
  // clang -v -c test.c
  // or clang++ for C++ paths as used below:
  
  
  // </Warning!!> -- End of Platform Specific Code
     headerSearchOptions.AddPath("/usr/include",
          clang::frontend::System,
          false,
          false);
     headerSearchOptions.AddPath("/usr/include/i386-linux-gnu/",
          clang::frontend::System,
          false,
          false);
     headerSearchOptions.AddPath("/usr/lib/gcc/i686-linux-gnu/4.6/include",
          clang::frontend::System,
          false,
          false);
     headerSearchOptions.AddPath("/usr/local/ssl/include",
          clang::frontend::System,
          false,
          false);
     headerSearchOptions.AddPath("/home/administrator/openssl-1.0.1f_out",
          clang::frontend::System,
          false,
          false);
     while(vp_inclu!=includeDirSet.end())
     {
	llvm::errs()<<"Include "<<" UUUUUUUUUUUUUU"<<*vp_inclu<<"\n";
        headerSearchOptions.AddPath(*vp_inclu,clang::frontend::System,false,false);
	vp_inclu++;
     }
  

     compiler.createPreprocessor();
     compiler.getPreprocessorOpts().UsePredefines = false;

     compiler.createASTContext();

      // Initialize rewriter
     Rewriter Rewrite;
     SourceManager& SM=compiler.getSourceManager();
     Rewrite.setSourceMgr(SM, compiler.getLangOpts());

     const FileEntry *pFile = compiler.getFileManager().getFile(fileName);
     compiler.getSourceManager().createMainFileID(pFile);
     compiler.getDiagnosticClient().BeginSourceFile(compiler.getLangOpts(),
                                                &compiler.getPreprocessor());

     MyASTConsumer astConsumer(Rewrite);

     // Parse the AST
     ParseAST(compiler.getPreprocessor(), &astConsumer, compiler.getASTContext());
     compiler.getDiagnosticClient().EndSourceFile();
     vp++;
   }
}



int main(int argc, char **argv)
{
  

  bool isBoundCheck = true;
  
  // initalize the vector for files and includeDir
  std::set<std::string> includeDirSet;
  std::set<std::string> filePathSet;
  std::set<std::string> allPathSet;
  
  // check the option arguments
  checkOptionArgument(argc, argv, includeDirSet,
                      isBoundCheck);

  // check the file arguments
  checkFileDirArgument(argc, argv, includeDirSet, filePathSet, allPathSet);

  if(isBoundCheck)
  {
     llvm::errs()<<"BoundCheck is begining.\n";
     boundcheck(includeDirSet,filePathSet);
  }

//  macroRewritePlus(includeDirSet, filePathSet);

//  backupMacroedFiles(allPathSet);

  return 0;
}
