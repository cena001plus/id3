​
//决策树 ID3 算法代码实现

//Copyright:wxh5055

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


//天气类型
typedef enum _outlook
{
	sunny,
	overcast,
	rain
}_outlook;


//温度类型
typedef enum _temperature
{
	hot,
	mild,
	cool
}_temperature;


//湿度类型
typedef enum _humidity
{
	high,
	normal
}_humidity;


//风强度类型
typedef enum _wind
{
	weak,
	strong
}_wind;


//属性类型(不包括目标属性)
typedef enum _attributeTypeName
{
	outlook,
	temperature,
	humidity,
	wind
}_attributeTypeName;


//目标属性
typedef enum _targetAttribute
{
	yes,
	no
}_targetAttribute;


//属性结构体(不包括目标属性)
typedef struct _attribute
{
	_outlook outlook;
	_temperature temperature;
	_humidity humidity;
	_wind wind;
}_attribute,*pAttribute;


//单个样本结构体,包括属性和目标属性
typedef struct _sample
{
	_attribute attributes;//属性
	_targetAttribute targatAttribute;//目标属性
}_sample,*pSample;


//训练样例结构体,在 ID3 算法中用来表示训练样例集
typedef struct _examples
{
	int sampleNum;//训练样例样本数
	int attributeTypeNum;//训练样例包含的属性类型数目(不包括目标属性)
	_attributeTypeName *pAttributeTypeName;//训练样例集的属性类型 
	int *pTrainSample;//训练样例集
}_examples,*pExamples;


//决策树结点结构体
typedef struct _node
{
	_attributeTypeName attributeTypeName;//节点代表的属性类型名称 
	int childNum;//分支数
	int *pChildAttributeValue;//每个子节点对应的属性值 
	struct _node **pChildNode;//指向子结点
	struct _node *pParent;//指向父结点
	int lable;//叶子节点的属性值(目标属性)
}_node,*pNode;


//定义属性类型对应的字符串(大写),用来打印搜索出的决策树
char *attributeTypeString[]=\
{
	"OUTLOOK",
	"TEMPERATURE",
	"HUMIDITY",
	"WIND",
};

//定义属性值对应的字符串(小写),用来打印搜索出的决策树 
char *attributeValueString[][3]=\
{
	{"sunny","overcast","rain"},
	{"hot","mild","cool"},
	{"high","normal","error"},
	{"weak","strong","error"},
};


//定义目标属性对应的字符串(小写),用来打印搜索出的决策树
char *attributeTargeteString[]=\
{
	"yes",
	"no",
};

//定义每个属性类型和属性值之间的连接符
char *connectString="--->";

//定义用来存储决策树结果的每条规则最大字符串数
#define MAX_ID3_CHAR_LENGTH 500

//训练实例				
#define TRAIN_NUM	14//定义 14 个训练实例
#define ATTR_NUM	4//每个训练实例有 4 个属性
_sample trainSample[TRAIN_NUM]={
{{sunny	, hot , high	, weak	}, no },//1
{{sunny	, hot , high	, strong}, no },//2
{{overcast, hot , high	, weak	}, yes},//3
{{rain	, mild, high	, weak	}, yes},//4
{{rain	, cool, normal, weak	}, yes},//5
{{rain	, cool, normal, strong}, no },//6
{{overcast, cool, normal, strong}, yes},//7
{{sunny	, mild, high	, weak	}, no },//8
{{sunny	, cool, normal, weak	}, yes},//9
{{rain	, mild, normal, weak	}, yes},//10
{{sunny	, mild, normal, strong}, yes},//11
{{overcast, mild, high	, strong}, yes},//12
{{overcast, hot , normal, weak	}, yes},//13
{{rain	, mild, high	, strong}, no },//14
};


//计算训练样例集的正样本数和负样本数
//输入训练样例集
//输出训练样例集的正样本数和负样本数
void CalcPosAndNegNum(pExamples pNodeExamples,int *pPosNum,int *pNegNum)
{
	int i;
	//取出训练样例
	int *pTrainSample=pNodeExamples->pTrainSample;
	//先清 0
	*pPosNum=0;
	*pNegNum=0;
	for(i=0;i<pNodeExamples->sampleNum;i++)
	{
	//pTrainSample 为 训 练 样 例 首 地 址 ， 每 个 训 练 样 本 共 有(pNodeExamples->attributeTypeNum+1(目标属性))个属性
	//根据目标属性地址存储的 yes 或 no 值来增加 pPosNum 或 pNegNum
		if(*(pTrainSample+i*((pNodeExamples->attributeTypeNum)+1)+pNodeExamples->attributeTypeNum)==yes)
		{
			(*pPosNum)++;
		}

		if(*(pTrainSample+i*((pNodeExamples->attributeTypeNum)+1)+pNodeExamples->attributeTypeNum)==no)
		{
			(*pNegNum)++;
		}
	}

	return;
}


//计算训练样例集的熵
//输入正样本数和负样本数
//返回计算出的熵值
double CalcEntropy(int posNum,int negNum)
{
	double entropy;

	//如果输入正负样本数有一个为 0,则返回熵值为 0 
	if(posNum==0 || negNum==0)
	{
		entropy=0;
	}

	else
	{
		//把 int 型转化为 double 型
		double fPosNum=(double)posNum;
		double fNegNum=(double)negNum;
		//计算正负样本数的比例
		double pPos=fPosNum/(fPosNum+fNegNum);
		double pNeg=fNegNum/(fPosNum+fNegNum);
		//计算熵值
		entropy=-pPos*(log10(pPos)/log10((double)2))-pNeg*(log10(pNeg)/log10((double)2));

	}
	return entropy;
}


//找出信息增益最高的属性
//输入训练样例结构体,正样本数和负样本数
//返回具有最高信息增益的属性在训练样例中的偏移以及最大属性包含的属性值个数以及该它们的值
int FindMaxInfoGainAttrbute(pExamples pNodeExamples,int posNum,int negNum,int *pMaxInfoGainAttrValueNum,int *pMaxInfoGainAttrValue) 
{
	int i,j,k;
	//计算训练样例集的熵
	double entropyS=CalcEntropy(posNum,negNum);
	//在计算各个属性的信息增益时用于存储属性变量的熵 
	double entropyA=0,entropyV=0;
	//计算每个属性类型下共有多少种类型的属性变量
	int attributeVlaueTypeNum;
	//存储每个属性类型下的属性变量的值
	int *attributeValueName=malloc(pNodeExamples->sampleNum*4); 
	//存储每个属性类型下各属性变量的值的个数
	int *attributeValueNum=malloc(pNodeExamples->sampleNum*4); 
	//存储每个属性变量下目标属性的正样本数
	int *attributeTargetValuePosNum=malloc(pNodeExamples->sampleNum*4); 
	//存储每个属性变量下目标属性的负样本数
	int *attributeTargetValueNegNum=malloc(pNodeExamples->sampleNum*4);
	 

	//初始化训练样例的第一个属性为信息增益最大的属性,其信息增益为 0
	double maxInfoGain=0;
	int maxInfoGainOffset=0;
	*pMaxInfoGainAttrValueNum=1;
	*pMaxInfoGainAttrValue=*pNodeExamples->pTrainSample;
	
	//如果分配内存失败
	if(attributeValueName==NULL || attributeValueNum==NULL || attributeTargetValuePosNum==NULL || attributeTargetValueNegNum==NULL)
	{
		*pMaxInfoGainAttrValueNum=-1;
		return -1;
	}

	//计算每个属性的信息增益,同时取出信息增益最大的属性及偏移 
	for(i=0;i<pNodeExamples->attributeTypeNum;i++) 
	{
		//属性的信息增益
		double infoGain;
		//取出第一个训练样本的属性变量的属性值存储
		*attributeValueName=*(pNodeExamples->pTrainSample+i);
		//属性变量个数初始化为 1
		attributeVlaueTypeNum=1;
		//第一个属性变量的个数初始化为 1
		*attributeValueNum=1;
		//判断第一个训练样本的目标属性的正负
		if(*(pNodeExamples->pTrainSample+pNodeExamples->attributeTypeNum)==yes)
		{
			//正样本数初始化为 1
			*attributeTargetValuePosNum=1;
			//负样本数初始化为 0
			*attributeTargetValueNegNum=0;
		}
		else
		{
			//正样本数初始化为 0
			*attributeTargetValuePosNum=0;
			//负样本数初始化为 1
			*attributeTargetValueNegNum=1;
		}

		//从下标 1 开始统计各个属性变量的情况
		for(j=1;j<pNodeExamples->sampleNum;j++)
		{
			//用来判断取出的属性值是否为新出现的属性值
			int compareFlag=0;
			//取出属性变量的属性值
			int attributeValue=*(pNodeExamples->pTrainSample + j*(pNodeExamples->attributeTypeNum+1) +i);
			//把取出的属性值和之前存储的属性值做比较,看是否为新的属性值 
			for(k=0;k<attributeVlaueTypeNum;k++) 
			{
				//如果对比成功
				if(attributeValue == *(attributeValueName+k))
				{
					//对应属性变量个数加 1
					*(attributeValueNum+k)=*(attributeValueNum+k)+1;
					//判断该训练样本的目标属性的正负
					if(*(pNodeExamples->pTrainSample+pNodeExamples->attributeTypeNum+j*(pNodeExamples->attributeTypeNum+1))==yes)
					{
					//正样本数加 1
					*(attributeTargetValuePosNum+k)=*(attributeTargetValuePosNum+k)+1;
					}
					else
					{
					//负样本数加 1
					*(attributeTargetValueNegNum+k)=*(attributeTargetValueNegNum+k)+1;
					}

					//对比成功标志置 1
					compareFlag=1;
					break;
				}
			}//end for(k=0;k<attributeVlaueTypeNum;k++)

			//如果没有对比成功,说明新出现了属性值
			if(compareFlag==0)
			{
			//存储新出现的属性变量的属性值 
				*(attributeValueName+attributeVlaueTypeNum)=attributeValue; //新出现的属性变量个数初始化为 1 
				*(attributeValueNum+attributeVlaueTypeNum)=1; //判断新出现的属性值的样本的目标属性的正负
				if(*(pNodeExamples->pTrainSample+pNodeExamples->attributeTypeNum+j*(pNodeExamples->attributeTypeNum+1))==yes)
				{
					//正样本数初始化为 1
					*(attributeTargetValuePosNum+attributeVlaueTypeNum)=1;
					//负样本数初始化为 0
					*(attributeTargetValueNegNum+attributeVlaueTypeNum)=0;

				}
				else
				{
				//正样本数初始化为 0
				*(attributeTargetValuePosNum+attributeVlaueTypeNum)=0;
				//负样本数初始化为 1
				*(attributeTargetValueNegNum+attributeVlaueTypeNum)=1;
				}

				//属性变量个数加 1
				attributeVlaueTypeNum++;
			}//end if(compareFlag==0)

		}//end for(j=1;j<*(pNodeExamples->pTrainSample);j++)

		//计算属性分类训练样例后熵的期望值
		entropyA=0;//清 0
		for(j=0;j<attributeVlaueTypeNum;j++)
		{
			//计算第 j 个属性变量的熵
			entropyV=CalcEntropy(*(attributeTargetValuePosNum+j),*(attributeTargetValueNegNum+j));
			entropyA=entropyA+(double)(*(attributeValueNum+j))/(double)(pNodeExamples ->sampleNum)*entropyV;
		}

		//计算属性的信息增益
		infoGain=entropyS-entropyA;
		//寻找信息增益最大的属性
		if(maxInfoGain<infoGain)
		{
			//更新最大信息增益的信息
			maxInfoGain=infoGain;
			maxInfoGainOffset=i;
			*pMaxInfoGainAttrValueNum=attributeVlaueTypeNum;
			for(j=0;j<attributeVlaueTypeNum;j++)
			{
				*(pMaxInfoGainAttrValue+j)=*(attributeValueName+j);
			}
		}

	}//end for(i=0;i<pNodeExamples->attributeTypeNum;i++)

	//释放内存空间
	free(attributeValueName);
	free(attributeValueNum);
	free(attributeTargetValuePosNum);
	free(attributeTargetValueNegNum);
	return maxInfoGainOffset;
}


//ID3 搜索算法
//输入训练样例集,包括训练样例样本数、属性数目、属性类型和训练样例,以及调用该算法的父节点
//返回计算出的根节点
pNode ID3(pExamples pNodeExamples)
{
	int i,j,k;
	//定义一个指针用于临时分配内存
	int *pPointTemp;
	//存储信息增益最高的属性的所有属性值
	int *pMaxInfoGainAttrValue;
	//存储信息增益最高的属性在训练样例中的列数
	int maxInfoGainOffset;
	//存储信息增益最高的属性包含的属性值个数(分支数) 
	int maxInfoGainAttrValueNum;
	//定义一个决策树结点
	pNode pTreeNode;
	//存储训练样例目标属性为 yes 和 no 的样本数
	int posNum,negNum;
	//如果训练样例的样本数小于 1，说明输入参数有误，直接返回 NULL 
	if(pNodeExamples->sampleNum<1)
	{
		return NULL;
	}

	//计算训练样例集的正样本数和负样本数 
	CalcPosAndNegNum(pNodeExamples,&posNum,&negNum); 
	//如果正样本数和负样本数都为 0，则出错 
	if(posNum==0 && negNum==0)
	{
		return NULL;
	}

	//为决策树节点分配内存空间
	pTreeNode=malloc(sizeof(_node));
	//如果分配失败 ，返回错误
	if(pTreeNode==NULL)
	{
	return NULL;
	}

	//标识该节点为非叶子节点
	pTreeNode->lable=0xff;
	//如果正样本数为 0，标识该节点目标属性值为 no 表示该节点为叶子节点
	if(posNum==0)
	{
		pTreeNode->lable=no;
		return pTreeNode;
	}
	//如果负样本数为 0，标识该节点目标属性值为 yes 表示该节点为叶子节点
	if(negNum==0)
	{
		pTreeNode->lable=yes;
		return pTreeNode;
	}

	//如果属性数目小于 1，标识该节点叶子节点，且目标属性值为 yes 或 no 中比较多的一个
	//如果两者数目一样多，则目标属性值为 no
	if(pNodeExamples->attributeTypeNum<1)
	{
		if(posNum>negNum)
		{
			pTreeNode->lable=yes;
		}
		else
		{
			pTreeNode->lable=no;
		}
		return pTreeNode;
	}

	//先给存储最大增益属性的属性值的指针分配和训练样例样本个数一样大小的临时空间
	pPointTemp=malloc(pNodeExamples->sampleNum*4);
	if(pPointTemp==NULL)
	{
		free(pTreeNode);
		return NULL;
	}

	//找出信息增益最高的属性
	maxInfoGainOffset=FindMaxInfoGainAttrbute(pNodeExamples,posNum,negNum,&maxInfoGainAttrValueNum,pPointTemp);
	//如果计算信息增益出错,则返回错误
	if(maxInfoGainOffset==-1)
	{
		//释放节点占用的内存空间
		free(pPointTemp);
		free(pTreeNode);
		return NULL;
	}

	//给存储最大增益属性的属性值的指针根据统计出的 maxInfoGainAttrValueNum 大小分配空间
	pMaxInfoGainAttrValue=malloc(maxInfoGainAttrValueNum*4);
	if(pMaxInfoGainAttrValue==NULL)
	{
		free(pPointTemp);
		free(pTreeNode);
		return NULL;
	}

	//把最大增益属性的属性值存储到 pMaxInfoGainAttrValue 中 
	for(i=0;i<maxInfoGainAttrValueNum;i++) 
	{
		*(pMaxInfoGainAttrValue+i)=*(pPointTemp+i);
	}

	//释放临时变量 pPointTemp 占用的内存空间
	free(pPointTemp);
	//把信息增益最高的属性名称存储到节点中
	pTreeNode->attributeTypeName=*(pNodeExamples->pAttributeTypeName+maxInfoGainOffset);
	//存储节点的分支数即信息增益最高的属性的属性值种类数
	pTreeNode->childNum=maxInfoGainAttrValueNum;
	//存储每个子节点对应的属性值
	pTreeNode->pChildAttributeValue=pMaxInfoGainAttrValue;
	//先分配 maxInfoGainAttrValueNum 个指针赋给节点的子节点
	pTreeNode->pChildNode=(struct _node **)malloc(sizeof(struct _node **)*maxInfoGainAttrValueNum);

	if(pTreeNode->pChildNode==NULL)
	{
		free(pTreeNode);
		return NULL;
	}

	//清除子节点指向的指针变量
	for(i=0;i<maxInfoGainAttrValueNum;i++)
	{
		*(pTreeNode->pChildNode+i)=NULL;
	}
	 

	//以下代码根据信息增益最高的属性类型下的每个属性值\
	//提取出与每个属性值相对应的训练样本数据,递归条用 ID3 算法
	//分配与训练样例集一样大小的临时空间来存储按每个属性值提取出的样本数据 
	pPointTemp=malloc(pNodeExamples->sampleNum*(pNodeExamples->attributeTypeNum+1)*4);

	//按每个属性值依次提取样本数据
	for(i=0;i<maxInfoGainAttrValueNum;i++)
	{
		//取出第 i 个属性值
		int attrValue=*(pMaxInfoGainAttrValue+i);
		//统计整个训练样例集里具有最大信息增益的属性中与第 i 个属性值相同的样本数
		int attrValueNum=0;
		//从整个训练样例集中提取数据
		for(j=0;j<pNodeExamples->sampleNum;j++)
		{
			//判断第 j 个训练样例的第 maxInfoGainOffset 个属性值是否和 attrValue 相等
			if(attrValue==*(pNodeExamples->pTrainSample+j*(pNodeExamples->attributeTypeNum+1)+ maxInfoGainOffset))
			{
			//把匹配的训练样本存储到临时内存 pPointTemp 中 
				for(k=0;k<(pNodeExamples->attributeTypeNum+1);k++) 
				{
					*(pPointTemp+attrValueNum*(pNodeExamples->attributeTypeNum+1)+k)=\
					*(pNodeExamples->pTrainSample+j*(pNodeExamples->attributeTypeNum+1)+k);
				}

			//匹配样本数加 1
			attrValueNum++;
			}

		}

		//如果满足第 i 个属性的样本数为空
		if(attrValueNum==0)
		{
			//创建一个子节点
			pNode pTreeNodeI=malloc(sizeof(_node));
			if(pTreeNodeI==NULL)
			{
				free(pPointTemp);
				free(pTreeNode);
				return NULL;
			}

			//标识该节点叶子节点，且目标属性值为 yes 或 no 中比较多的一个
			if(posNum>negNum)
			{
				pTreeNodeI->lable=yes;
			}
			else
			{
				pTreeNodeI->lable=no;
			}

			//把该节点赋给根节点的第 i 个子节点
			*(pTreeNode->pChildNode+i)=pTreeNodeI;
		}//end if(attrValueNum==0)

		else	//if(attrValueNum==0)
		{
			//从训练样例集中移出第 i 个属性值,重新组合成新的训练样例集 
			int l;
			//定义一个节点的训练样例结构体 examples
			pExamples pNodeExamplesTemp;
			//为新的训练样例集分配内存空间
			int *pTrainSample=malloc(pNodeExamples->attributeTypeNum*attrValueNum*4);
			if(pTrainSample==NULL)
			{
				free(pPointTemp);
				free(pTreeNode);
				return NULL;
			}

			//为训练样例结构体 examples 分配内存空间 
			pNodeExamplesTemp=(pExamples)malloc(sizeof(_examples)); 
			if(pNodeExamplesTemp==NULL)
			{
				free(pPointTemp);
				free(pTreeNode);
				return NULL;
			}

			//把数据转移到新的训练样例集 pTrainSample 中
			l=0;
			for(j=0;j<attrValueNum;j++)
			{
				for(k=0;k<(pNodeExamples->attributeTypeNum+1);k++)
				{
					//	把除去最大信息增益的属性之外的所有训练样例存储到pTrainSample 中
					if(k!=maxInfoGainOffset)
					{
						*(pTrainSample+l)=*(pPointTemp+j*(pNodeExamples->attributeTypeNum+1)+k);
						l++;
					}
				}

			}

			//存储训练样例
			pNodeExamplesTemp->pTrainSample=pTrainSample;
			//为 pNodeExampls 的属性类型分配空间
			k=(pNodeExamples->attributeTypeNum-1)*4;
			pNodeExamplesTemp->pAttributeTypeName=malloc((pNodeExamples->attributeTypeNum-1)*4);
			if(pNodeExamplesTemp->pAttributeTypeName==NULL)
			{
				free(pPointTemp);
				free(pTreeNode);
				return NULL;
			}

			//存储除了第 maxInfoGainOffset 之外的其他属性类型
			k=0;
			for(j=0;j<(pNodeExamples->attributeTypeNum);j++)
			{
				if(j!=maxInfoGainOffset)
				{
					*(pNodeExamplesTemp->pAttributeTypeName+k)=*(pNodeExamples->pAttributeTypeName+j);
					k++;
				}
			}

			//存储属性类型数量
			pNodeExamplesTemp->attributeTypeNum=pNodeExamples->attributeTypeNum-1;
			//存储训练样例样本数
			pNodeExamplesTemp->sampleNum=attrValueNum;
			//以 pNodeExamplesTemp 为训练样例集,以 pTreeNode 为父节点递归调用 ID3算法
			*(pTreeNode->pChildNode+i)=ID3(pNodeExamplesTemp);
			if(*(pTreeNode->pChildNode+i)==NULL)
			{
				free(pPointTemp);
				free(pTreeNode);
				return NULL;
			}

			//释放给 pNodeExamplesTemp 分配的内存
			free(pNodeExamplesTemp->pAttributeTypeName);
			free(pNodeExamplesTemp->pTrainSample);
			free(pNodeExamplesTemp);

		}//end else	//if(attrValueNum==0)

	}//end for(i=0;i<maxInfoGainAttrValueNum;i++)

	//释放存储按每个属性值提取出的样本数据的临时空间
	free(pPointTemp);
	//返回生成的节点
	return pTreeNode;

}



//将决策树结果以规则集合的形式打印出来
//输入决策树根节点和要打印的字符串
//无输出
void DisPlayID3Result(pNode pRootNode,char *printString)
{
	int i;
	//定义一个新的字符串数组来存储到该节点之前的打印信息
	char prePrintString[MAX_ID3_CHAR_LENGTH];
	//先判断该节点是否为叶子节点
	if(pRootNode->lable!=0xff)
	{
		//如果是叶子节点,则把叶子节点的属性值(目标属性)添加到 printString 中并打印出来
		//strcat(printString,connectString);
		strcat(printString,attributeTargeteString[pRootNode->lable]);
		printf("%s\n",printString);
		return;
	}

	//把非叶子节点的属性类型添加到 printString 中
	strcat(printString,attributeTypeString[pRootNode->attributeTypeName]);
	//添加属性类型到属性值的连接符
	strcat(printString,connectString);
	//根据该节点的子节点数来把每个子节点属性值添加到 prePrintString 中
	for(i=0;i<pRootNode->childNum;i++)
	{
		//把 printString 里的字符串复制到 prePrintString 中
		strcpy(prePrintString,printString);
		//把每个子节点的属性值添加到 prePrintString 中
		strcat(prePrintString,attributeValueString[pRootNode->attributeTypeName][*(pRootNode-> pChildAttributeValue+i)]);
		//添加属性值到下一个属性类型(或目标属性)的连接符到 prePrintString 中
		strcat(prePrintString,connectString);
		//递归条用 DisPlayID3Result 来打印每一个子节点的属性类型和属性值
		DisPlayID3Result(*(pRootNode->pChildNode+i),prePrintString);
	}

	return;
}


//释放决策树的每个节点占用的内存空间
//输入决策树根节点
//无输出
void freeNode(pNode pRootNode)
{
	int i;
	//如果是叶子节点,则释放该节点占用的内存空间
	if(pRootNode->lable!=0xff)
	{
		free(pRootNode);
		return;
	}

	//如果不是叶子节点
	for(i=0;i<pRootNode->childNum;i++)
	{
		//先递归调用 freeNode 释放每个子节点占用的内存空间 
		freeNode(*(pRootNode->pChildNode+i));
	}
			
	//释放指向该非叶子节点的每个子节点的属性值占用的内存空间; 
	free(pRootNode->pChildAttributeValue); 
	//释放本节点占用的内存空间
	free(pRootNode);
	return;
}



//决策树算法程序
void DecisionTree(void)
{
	//定义决策树的根节点
	pNode pRootNode=NULL;
	//定义根节点的训练样例集的属性类型
	_attributeTypeName pRootAttributeName[4]={outlook,temperature,humidity,wind}; 
	//用原始训练实例初始化根节点的训练样例结构体 examples
	pExamples pRootExampls=(pExamples)malloc(sizeof(_examples));
	if(pRootExampls==NULL)
	{
		printf("根节点训练样例结构体内存分配失败!");
		return;
	}
 

	//存储根节点的训练样例集的属性类型
	pRootExampls->pAttributeTypeName=pRootAttributeName;
	//计算根节点地训练样例的样本数
	pRootExampls->sampleNum =sizeof(trainSample)/sizeof(_sample);
	//存储根节点的训练样例包含的属性类型数
	pRootExampls->attributeTypeNum=ATTR_NUM;
	//储根节点的训练样例
	pRootExampls->pTrainSample=(int *)trainSample;

	//ID3 算法
	pRootNode=ID3(pRootExampls);
	//打印决策树信息
	if(pRootNode)
	{
		//将决策树结果以规则集合的形式打印出来
		char ID3_Result[MAX_ID3_CHAR_LENGTH]="Rule:";
		DisPlayID3Result(pRootNode,ID3_Result);
	}
	else
	{
		printf("ID3 搜索决策树失败!");
	}

	//内存释放
	free(pRootExampls);
	if(pRootNode)
	{
		freeNode(pRootNode);
	}
	return;
}



int main(void)
{
	//调用决策树算法
	DecisionTree();
	return 1;
}

​