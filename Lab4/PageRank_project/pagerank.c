#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99没bool

# define MAX 400000 //最终有39万+个网页

/*
PageRank算法流程
一.得到邻接矩阵G
二.在G基础上得到Gm:G中每列元素/列元素总数
三.计算矩阵Gm特征值为1对应的特征向量(1*v = A*v)，又本例为大的稀疏矩阵求特征向量的数值解，不用进行不可约修正了
四.计算对应特征向量过程
	当矩阵A 的阶很大，无法直接计算其特征值和特征向量时，需要使用Power Iteration幂迭代方法
	1.输入矩阵A 和迭代初始向量v，以及精度e
	2.计算：Vk+1 = AVk
	3.如果|vk+1 - vk |< e>0，则计算PageRank 值并停止。否则转第二步。
	最后x = A^k*v /sum(A^k*v)
*/


long int mem = 0;
void* bupt_malloc(size_t size) {  //size_t:used to represent the size of an object
	if (size <= 0)
		return NULL;
	mem += size;
	return malloc(size);

}

//矩阵元素
//1.ELL
int** ell_column_indices = NULL;
float** ell_values = NULL;
int ELL_COLUMN_SIZE = 70; //别太大，要不然内存直接飞天，太小coo会满
int* ell_row_contains = NULL; //ELL内某行包含的元素个数
//2.COO
int* coo_row = NULL;
int* coo_column = NULL;
float* coo_values = NULL;
int COO_SIZE = 200000; //COO内最多放20万个元素
int coo_contains = 0;
int row_to_coo_index[MAX + 1] = { 0 }; //第一行肯定是0


//pagerank计算所需元素
int* LinkOutNum = NULL;
int* LinkInNum = NULL;
double* FeatureVector = NULL;

void BuildMatrixG(int SourseID, int DestiID);
void BuildMatrixGm();
void CalFeatureVector();

//其他所需元素
void GetTop10();
double top10Rank[10] = { 0 };
int top10RankIndex[10] = { 0 };



int main()
{
	//矩阵元素
	//1.ELL
	ell_column_indices = (int **)bupt_malloc(sizeof(int*) * MAX);//有MAX=40万行
	for (int i = 0; i < MAX; i++) { //初始化ell_column_indices的每行元素
		ell_column_indices[i] = (int*)bupt_malloc(sizeof(int) * ELL_COLUMN_SIZE);
		//printf("hehe %d\n", i); //在i=2447处越界了,妈的内存占用直接超2G了
		memset(ell_column_indices[i], 0, sizeof(int) * ELL_COLUMN_SIZE); //别忘了初始化
		for (int k = 0; k < ELL_COLUMN_SIZE; k++) {
			ell_column_indices[i][k] = -1;
		}
	}
	ell_values = (float**)bupt_malloc(sizeof(float*) * MAX);//有MAX=40万行
	for (int i = 0; i < MAX; i++) { //初始化ell_column_indices的每行元素
		ell_values[i] = (float*)bupt_malloc(sizeof(float) * ELL_COLUMN_SIZE);
		memset(ell_values[i], 0, sizeof(float) * ELL_COLUMN_SIZE);//直接赋0，越界由ell_column_indices判断
	}
	ell_row_contains = (int*)bupt_malloc(sizeof(int) * MAX);//有MAX=40万行
	memset(ell_row_contains, 0, sizeof(int) * MAX);

	//2.COO
	coo_row = (int*)bupt_malloc(sizeof(int) * COO_SIZE); //COO内存的数据
	coo_column = (int*)bupt_malloc(sizeof(int) * COO_SIZE);
	coo_values = (float*)bupt_malloc(sizeof(float) * COO_SIZE);
	memset(coo_values, 0, sizeof(float) * COO_SIZE);

	//pagerank计算所需元素
	LinkOutNum = (int*)bupt_malloc(sizeof(int) * MAX);
	memset(LinkOutNum, 0, sizeof(int) * MAX);
	LinkInNum = (int*)bupt_malloc(sizeof(int) * MAX);
	memset(LinkInNum, 0, sizeof(int) * MAX);
	FeatureVector = (double*)bupt_malloc(sizeof(double) * MAX);
	for (int i = 0; i < MAX; i++) {
		FeatureVector[i] = 1;
	}


	/*
	读取url.txt内的网页数据
	该网页之间的链接光系组成一个稀疏矩阵，使用Compressed Sparse Row (CSR)
	来进行存储，节省了行存储开销，常用于读入数据后进行稀疏矩阵计算。

	而至于稀疏矩阵的存储，本例使用Hybrid (HYB) ELL+C00的形式
	即如果某一行很多元素，那么ELL后面两个矩阵就会很胖，,为了解决ELL中提到的，
	如果某一行特别多，造成其他行的浪费，那么把这些多出来的元素用COO单独存储。
	*ELL的优点是快速，而COO优点是灵活，二者结合后的HYB格式是一种不错的稀疏矩阵表示格式
	*/

	//所需变量
	char** URLs = (char**)bupt_malloc(sizeof(char*) * MAX); //用于存储网页字符串

	FILE* fp = fopen("url.txt", "r");
	//char inputstr[100];
	char* inputstr = (char*)bupt_malloc(sizeof(char) * 500);  //url.txt每一行的字符串
	char* strSourceID = (char*)bupt_malloc(sizeof(char) * 7); //6位数加末尾'\0'
	char* strDestiID = (char*)bupt_malloc(sizeof(char) * 7);
	int SourseID = 0, DestiID = 0;
	int urlIndex = 0;
	while (!feof(fp)) {
		fgets(inputstr, 500, fp);
		//inputstr[99] = ' '; //太长的直接截断,不行，后面的没读进来
		if (inputstr[0] == '\n') {
			continue;
		}
		if (inputstr[0] == '/') { //该行是url与其编号
			int urllen = 0;
			for (urllen = 0; inputstr[urllen] != ' '; urllen++); //该行格式为url 编号
			if ((URLs[urlIndex] = (char*)bupt_malloc(sizeof(char) * urllen + 1)) == NULL) {
				printf("Error in malloc urls\n");
				return -1;
			}
			strncpy(URLs[urlIndex], inputstr, urllen);
			//处理一下URLs[i]的结尾
			URLs[urlIndex][urllen] = '\0';
			//test
			//printf("URLs %d is %s and his len is %d\n", urlIndex, URLs[urlIndex], urllen);

			//row_to_coo_index处理
			row_to_coo_index[urlIndex + 1] = row_to_coo_index[urlIndex];
			

			urlIndex++; //文件内的网页编号是按顺序增长的，即indexof(URLs[i]) = i;

		}
		else { //该行两个数字代表网页的链接关系
			memset(strSourceID, 0, 6);
			memset(strDestiID, 0, 6);
			//获取SourseID
			int Sourselen = 0;
			for (Sourselen = 0; inputstr[Sourselen] != ' '; ++Sourselen);
			strncpy(strSourceID, inputstr, Sourselen);
			strSourceID[Sourselen] = '\0';
			SourseID = 0;
			for (int i = 0; strSourceID[i] != '\0'; i++) {
				SourseID = SourseID * 10 + (strSourceID[i] - '0');
			}
			//获取DestiID
			int Destilen = 0;
			for (Destilen = 0; inputstr[Sourselen + 1 + Destilen] != '\n'; ++Destilen); 
			strncpy(strDestiID, inputstr + Sourselen + 1, Destilen);
			strDestiID[Destilen] = '\0';
			DestiID = 0;
			for (int i = 0; strDestiID[i] != '\0'; i++) {
				DestiID = DestiID * 10 + (strDestiID[i] - '0');
			}
			//test
			//printf("Link relationship is %d to %d\n", SourseID, DestiID);
			if (SourseID > 800000) //最后一行的统计用数据
				continue;

			LinkOutNum[SourseID]++;
			LinkInNum[DestiID]++;
			//一.建立邻接矩阵G
			BuildMatrixG(SourseID, DestiID);
			//test
			//printf("hehe %d, %d\n", SourseID, DestiID);
		}
		
	}
	fclose(fp);

	//二.在G基础上得到Gm:G中每列元素 / 列元素总数
	BuildMatrixGm();

	//三.计算矩阵Gm特征值为1对应的特征向量(1 * v = A * v)
	printf("Start calculate feature vector\n");
	CalFeatureVector();

	//寻找PageRank值前10的网页
	GetTop10();

	//写入文件
	FILE* fpwrite = fopen("Top10.txt", "w");
	for (int i = 0; i < 10; i++) {
		fprintf(fpwrite, "%s %f\n", URLs[top10RankIndex[i]], top10Rank[i]);
		printf("Top %d is %s and page rank value is %.7f\n", (10 - i), URLs[top10RankIndex[i]], top10Rank[i]);
	}
	//写入最后一行
	fprintf(fpwrite, "%ld\n", mem / 1024); //单位是KB
	fclose(fpwrite);

	return 0;

}

void BuildMatrixG(int SourseID, int DestiID)
{
	//printf("hehe %d, %d\n", SourseID, DestiID);
	/*
	至于稀疏矩阵的存储，本例使用Hybrid (HYB) ELL+C00的形式
	即如果某一行很多元素，那么ELL后面两个矩阵就会很胖，,为了解决ELL中提到的，
	如果某一行特别多，造成其他行的浪费，那么把这些多出来的元素用COO单独存储。
	*/

	//1.先尝试放入ELL矩阵
	for (int i = 0; i < ell_row_contains[SourseID]; i++) {
		if (ell_column_indices[SourseID][i] == DestiID) { //该DestiID对应列已经放入过了
			ell_values[SourseID][i] ++;
			//printf("ell1:%d %d\n", ell_column_indices[SourseID][i], ell_values[SourseID][i]);
			return;
		}
	}
	if (ell_row_contains[SourseID] < ELL_COLUMN_SIZE) { //ell没满
		//第一次放入的元素
		ell_column_indices[SourseID][ell_row_contains[SourseID]] = DestiID;
		ell_values[SourseID][ell_row_contains[SourseID]] ++;
		//ell_values[SourseID][ell_row_contains[SourseID]] = ell_values[SourseID][ell_row_contains[SourseID]] + 1; 没必要
		//printf("ell:%d %d\n", ell_column_indices[SourseID][ell_row_contains[SourseID]], ell_values[SourseID][ell_row_contains[SourseID]]);
		ell_row_contains[SourseID]++; //第一个放进来的才需要contains++
	}

	//2.ell_row_contains[SourseID]该行以放慢，剩下的元素放入COO中
	else {
		//printf("Adding an element to COO\n");
		//printf("hehe %d, %d\n", SourseID, DestiID);
		if (coo_contains >= COO_SIZE) {
			printf("ERROR: the COO is full\n");
			printf("hehe1 %d, %d\n", SourseID, DestiID);
			return -1;
		}
		for (int i = row_to_coo_index[SourseID]; i < row_to_coo_index[SourseID + 1]; i++) {
			if (coo_row[i]== SourseID && coo_column[i] == DestiID) { //该DestiID对应列已经放入过了
				coo_values[i]++;
				//printf("coo1:%d %d\n", coo_column[i], coo_values[i]);
				row_to_coo_index[SourseID + 1]++;
				return;
			}
		}
		//第一次放入的元素
		coo_row[coo_contains] = SourseID;
		coo_column[coo_contains] = DestiID;
		coo_values[coo_contains]++;
		//printf("coo:%d %d %d\n", SourseID, coo_column[coo_contains], coo_values[coo_contains]);
		coo_contains++;
		//表示行数的
		row_to_coo_index[SourseID + 1]++;
		//printf("row_to_coo_index[%d] is %d\n", SourseID + 1, row_to_coo_index[SourseID + 1]);
	}
}

void BuildMatrixGm()
{
	/*
	int a = 255058;
	printf("row%d num in coo is %d\n", a, row_to_coo_index[a]);
	for (int i = 0; i < 10; i++) {
		a = i;
		printf("row%d num in coo is %d\n", a, row_to_coo_index[a]);
	}
	
	//test 查看下矩阵内元素
	int temp = 48064;
	printf("Checking row %d\n", temp);
	for (int k = 0; k < ell_row_contains[temp]; k++) {
		printf("%d:%d ", ell_column_indices[temp][k], ell_values[temp][k]);
	}
	for (int k = row_to_coo_index[temp]; k < row_to_coo_index[temp+1]; k++) {
		if (coo_row[k] == temp)
			printf("%d:%d ", coo_column[k], coo_values[k]);
	}
	printf("\n");
	*/

	/*
	在G基础上得到Gm:G中每列元素/列元素总数(该列的LinkInNum)
	*/

	//1.处理ELL矩阵内元素
	for (int i = 0; i < MAX; i++) { //每行都要处理
		for (int k = 0; k < ell_row_contains[i]; k++) {
			ell_values[i][k] = ell_values[i][k] / LinkInNum[ell_column_indices[i][k]];
		}
	}

	//2.处理COO矩阵内元素
	for (int i = 0; i < coo_contains; i++) {
		coo_values[i] = coo_values[i] / LinkInNum[coo_column[i]];
	}
}

void CalFeatureVector()
{
	/*
	计算对应特征向量过程
	当矩阵A 的阶很大，无法直接计算其特征值和特征向量时，需要使用Power Iteration幂迭代方法
		1.输入矩阵A 和迭代初始向量v，以及精度e
		2.计算：Vk+1 = AVk
		3.如果|vk+1 - vk |< e>0，则计算PageRank 值并停止。否则转第二步。
	最后x = A^k*v /sum(A^k*v)
	*/


	double episilon = 0.0001;//0.0001;
	double DiffValue = 100; //DiffValue = |vk+1 - vk |^2
	double* LastVector = (double*)bupt_malloc(sizeof(double) * MAX);
	double* normaliVector = (double*)bupt_malloc(sizeof(double) * MAX);
	int roundCount = 1;
	double maxElement = 1;
	while (DiffValue >= episilon * episilon) {
		printf("Calculating in round %d\n", roundCount);
		roundCount++;

		//V=FeatureVector
		for (int i = 0; i < MAX; i++) {
			LastVector[i] = FeatureVector[i];
			//别忘了归一化,归一化后再参与运算
			normaliVector[i] = LastVector[i] / maxElement;
			FeatureVector[i] = 0;
		}
		
		
		

		maxElement = 0;
		//最难的一部分：稀疏矩阵的乘法Vk+1 = A*Vk
		for (int i = 0; i < MAX; i++) { //i为行数
			//printf("counting at round %d\n", i);
			//这块浪费时间太多了，改下coo部分

			float calValue = 0;
			int k = 0; //这里k是代表ell内的列索引
			//Vk+1[i] = Σ(A[i][j]*VK[j][i]) j=1、2...MAX
			while (k < ell_row_contains[i]) {
				//if (ell_column_indices[i][k]==-1) { //后面都是0，别整了，直接断掉去下一行
				//	break;
				//}
				int colIndex = ell_column_indices[i][k]; //colIndex就是使值不为0的j
				float ellValue = ell_values[i][k];
				calValue += ellValue * normaliVector[colIndex];
				//printf("ell_column_indices[i][k] is %d")
				k++;
			}
			if (k >= ell_row_contains[i]) { //ell放满了，COO内还有
				for (int p = row_to_coo_index[i]; p < row_to_coo_index[i+1]; p++) {
					if (coo_row[p] == i) { //coo内该元素在矩阵该行中
						int colIndex = coo_column[p];
						float cooValue = coo_values[p];
						calValue += cooValue * normaliVector[colIndex];
					}
				}
			}
			FeatureVector[i] = calValue;

			
			if (FeatureVector[i] > maxElement)
				maxElement = FeatureVector[i];

		}

		

		//如果 | vk+1 - vk | < e>0，则计算PageRank 值并停止
		DiffValue = 0;
		for (int i = 0; i < MAX; i++) {
			DiffValue += (FeatureVector[i] - LastVector[i]) * (FeatureVector[i] - LastVector[i]);
		}

		//test
		/*
		double a = 0;
		for (int i = 0; i < MAX; i++) {
			a += FeatureVector[i];
			if (i % 100 == 0)
				printf("hehe %d: %f %f\n", i, LastVector[i], FeatureVector[i]);
		}
		*/

		printf("maxElement is %f\n", maxElement);
		//printf("hehe1 %f\n", a);
		printf("DiffValue^2 is %.8f\n", DiffValue);

	}
}

void GetTop10()
{
	//test
	double a = 0;
	for (int i = 0; i < MAX; i++) {
		a += FeatureVector[i];
	}
	printf("top10hehe %f\n", a);

	//int topNum = 10;
	for (int i = 0; i < MAX; i++) {
		if (FeatureVector[i] > top10Rank[0]) { //top10Rank内pagerank值按倒叙排列，top10Rank[0]最小
			//printf("top10hehe1 %f\n", FeatureVector[i]);
			int tempIndex = 0;
			for (int k = 1; k <10 ; k++) { //已经大于top10Rank[0]了
				//if(FeatureVector[i] > top10Rank[0])
				if (FeatureVector[i] > top10Rank[k])
					tempIndex++;
				else
					break;
			}
			top10Rank[tempIndex] = FeatureVector[i];
			top10RankIndex[tempIndex] = i;
		}
	}
}