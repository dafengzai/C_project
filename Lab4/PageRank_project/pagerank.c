#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99ûbool

# define MAX 400000 //������39��+����ҳ

/*
PageRank�㷨����
һ.�õ��ڽӾ���G
��.��G�����ϵõ�Gm:G��ÿ��Ԫ��/��Ԫ������
��.�������Gm����ֵΪ1��Ӧ����������(1*v = A*v)���ֱ���Ϊ���ϡ�������������������ֵ�⣬���ý��в���Լ������
��.�����Ӧ������������
	������A �Ľ׺ܴ��޷�ֱ�Ӽ���������ֵ����������ʱ����Ҫʹ��Power Iteration�ݵ�������
	1.�������A �͵�����ʼ����v���Լ�����e
	2.���㣺Vk+1 = AVk
	3.���|vk+1 - vk |< e>0�������PageRank ֵ��ֹͣ������ת�ڶ�����
	���x = A^k*v /sum(A^k*v)
*/


long int mem = 0;
void* bupt_malloc(size_t size) {  //size_t:used to represent the size of an object
	if (size <= 0)
		return NULL;
	mem += size;
	return malloc(size);

}

//����Ԫ��
//1.ELL
int** ell_column_indices = NULL;
float** ell_values = NULL;
int ELL_COLUMN_SIZE = 70; //��̫��Ҫ��Ȼ�ڴ�ֱ�ӷ��죬̫Сcoo����
int* ell_row_contains = NULL; //ELL��ĳ�а�����Ԫ�ظ���
//2.COO
int* coo_row = NULL;
int* coo_column = NULL;
float* coo_values = NULL;
int COO_SIZE = 200000; //COO������20���Ԫ��
int coo_contains = 0;
int row_to_coo_index[MAX + 1] = { 0 }; //��һ�п϶���0


//pagerank��������Ԫ��
int* LinkOutNum = NULL;
int* LinkInNum = NULL;
double* FeatureVector = NULL;

void BuildMatrixG(int SourseID, int DestiID);
void BuildMatrixGm();
void CalFeatureVector();

//��������Ԫ��
void GetTop10();
double top10Rank[10] = { 0 };
int top10RankIndex[10] = { 0 };



int main()
{
	//����Ԫ��
	//1.ELL
	ell_column_indices = (int **)bupt_malloc(sizeof(int*) * MAX);//��MAX=40����
	for (int i = 0; i < MAX; i++) { //��ʼ��ell_column_indices��ÿ��Ԫ��
		ell_column_indices[i] = (int*)bupt_malloc(sizeof(int) * ELL_COLUMN_SIZE);
		//printf("hehe %d\n", i); //��i=2447��Խ����,����ڴ�ռ��ֱ�ӳ�2G��
		memset(ell_column_indices[i], 0, sizeof(int) * ELL_COLUMN_SIZE); //�����˳�ʼ��
		for (int k = 0; k < ELL_COLUMN_SIZE; k++) {
			ell_column_indices[i][k] = -1;
		}
	}
	ell_values = (float**)bupt_malloc(sizeof(float*) * MAX);//��MAX=40����
	for (int i = 0; i < MAX; i++) { //��ʼ��ell_column_indices��ÿ��Ԫ��
		ell_values[i] = (float*)bupt_malloc(sizeof(float) * ELL_COLUMN_SIZE);
		memset(ell_values[i], 0, sizeof(float) * ELL_COLUMN_SIZE);//ֱ�Ӹ�0��Խ����ell_column_indices�ж�
	}
	ell_row_contains = (int*)bupt_malloc(sizeof(int) * MAX);//��MAX=40����
	memset(ell_row_contains, 0, sizeof(int) * MAX);

	//2.COO
	coo_row = (int*)bupt_malloc(sizeof(int) * COO_SIZE); //COO�ڴ������
	coo_column = (int*)bupt_malloc(sizeof(int) * COO_SIZE);
	coo_values = (float*)bupt_malloc(sizeof(float) * COO_SIZE);
	memset(coo_values, 0, sizeof(float) * COO_SIZE);

	//pagerank��������Ԫ��
	LinkOutNum = (int*)bupt_malloc(sizeof(int) * MAX);
	memset(LinkOutNum, 0, sizeof(int) * MAX);
	LinkInNum = (int*)bupt_malloc(sizeof(int) * MAX);
	memset(LinkInNum, 0, sizeof(int) * MAX);
	FeatureVector = (double*)bupt_malloc(sizeof(double) * MAX);
	for (int i = 0; i < MAX; i++) {
		FeatureVector[i] = 1;
	}


	/*
	��ȡurl.txt�ڵ���ҳ����
	����ҳ֮������ӹ�ϵ���һ��ϡ�����ʹ��Compressed Sparse Row (CSR)
	�����д洢����ʡ���д洢�����������ڶ������ݺ����ϡ�������㡣

	������ϡ�����Ĵ洢������ʹ��Hybrid (HYB) ELL+C00����ʽ
	�����ĳһ�кܶ�Ԫ�أ���ôELL������������ͻ���֣�,Ϊ�˽��ELL���ᵽ�ģ�
	���ĳһ���ر�࣬��������е��˷ѣ���ô����Щ�������Ԫ����COO�����洢��
	*ELL���ŵ��ǿ��٣���COO�ŵ��������߽�Ϻ��HYB��ʽ��һ�ֲ����ϡ������ʾ��ʽ
	*/

	//�������
	char** URLs = (char**)bupt_malloc(sizeof(char*) * MAX); //���ڴ洢��ҳ�ַ���

	FILE* fp = fopen("url.txt", "r");
	//char inputstr[100];
	char* inputstr = (char*)bupt_malloc(sizeof(char) * 500);  //url.txtÿһ�е��ַ���
	char* strSourceID = (char*)bupt_malloc(sizeof(char) * 7); //6λ����ĩβ'\0'
	char* strDestiID = (char*)bupt_malloc(sizeof(char) * 7);
	int SourseID = 0, DestiID = 0;
	int urlIndex = 0;
	while (!feof(fp)) {
		fgets(inputstr, 500, fp);
		//inputstr[99] = ' '; //̫����ֱ�ӽض�,���У������û������
		if (inputstr[0] == '\n') {
			continue;
		}
		if (inputstr[0] == '/') { //������url������
			int urllen = 0;
			for (urllen = 0; inputstr[urllen] != ' '; urllen++); //���и�ʽΪurl ���
			if ((URLs[urlIndex] = (char*)bupt_malloc(sizeof(char) * urllen + 1)) == NULL) {
				printf("Error in malloc urls\n");
				return -1;
			}
			strncpy(URLs[urlIndex], inputstr, urllen);
			//����һ��URLs[i]�Ľ�β
			URLs[urlIndex][urllen] = '\0';
			//test
			//printf("URLs %d is %s and his len is %d\n", urlIndex, URLs[urlIndex], urllen);

			//row_to_coo_index����
			row_to_coo_index[urlIndex + 1] = row_to_coo_index[urlIndex];
			

			urlIndex++; //�ļ��ڵ���ҳ����ǰ�˳�������ģ���indexof(URLs[i]) = i;

		}
		else { //�����������ִ�����ҳ�����ӹ�ϵ
			memset(strSourceID, 0, 6);
			memset(strDestiID, 0, 6);
			//��ȡSourseID
			int Sourselen = 0;
			for (Sourselen = 0; inputstr[Sourselen] != ' '; ++Sourselen);
			strncpy(strSourceID, inputstr, Sourselen);
			strSourceID[Sourselen] = '\0';
			SourseID = 0;
			for (int i = 0; strSourceID[i] != '\0'; i++) {
				SourseID = SourseID * 10 + (strSourceID[i] - '0');
			}
			//��ȡDestiID
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
			if (SourseID > 800000) //���һ�е�ͳ��������
				continue;

			LinkOutNum[SourseID]++;
			LinkInNum[DestiID]++;
			//һ.�����ڽӾ���G
			BuildMatrixG(SourseID, DestiID);
			//test
			//printf("hehe %d, %d\n", SourseID, DestiID);
		}
		
	}
	fclose(fp);

	//��.��G�����ϵõ�Gm:G��ÿ��Ԫ�� / ��Ԫ������
	BuildMatrixGm();

	//��.�������Gm����ֵΪ1��Ӧ����������(1 * v = A * v)
	printf("Start calculate feature vector\n");
	CalFeatureVector();

	//Ѱ��PageRankֵǰ10����ҳ
	GetTop10();

	//д���ļ�
	FILE* fpwrite = fopen("Top10.txt", "w");
	for (int i = 0; i < 10; i++) {
		fprintf(fpwrite, "%s %f\n", URLs[top10RankIndex[i]], top10Rank[i]);
		printf("Top %d is %s and page rank value is %.7f\n", (10 - i), URLs[top10RankIndex[i]], top10Rank[i]);
	}
	//д�����һ��
	fprintf(fpwrite, "%ld\n", mem / 1024); //��λ��KB
	fclose(fpwrite);

	return 0;

}

void BuildMatrixG(int SourseID, int DestiID)
{
	//printf("hehe %d, %d\n", SourseID, DestiID);
	/*
	����ϡ�����Ĵ洢������ʹ��Hybrid (HYB) ELL+C00����ʽ
	�����ĳһ�кܶ�Ԫ�أ���ôELL������������ͻ���֣�,Ϊ�˽��ELL���ᵽ�ģ�
	���ĳһ���ر�࣬��������е��˷ѣ���ô����Щ�������Ԫ����COO�����洢��
	*/

	//1.�ȳ��Է���ELL����
	for (int i = 0; i < ell_row_contains[SourseID]; i++) {
		if (ell_column_indices[SourseID][i] == DestiID) { //��DestiID��Ӧ���Ѿ��������
			ell_values[SourseID][i] ++;
			//printf("ell1:%d %d\n", ell_column_indices[SourseID][i], ell_values[SourseID][i]);
			return;
		}
	}
	if (ell_row_contains[SourseID] < ELL_COLUMN_SIZE) { //ellû��
		//��һ�η����Ԫ��
		ell_column_indices[SourseID][ell_row_contains[SourseID]] = DestiID;
		ell_values[SourseID][ell_row_contains[SourseID]] ++;
		//ell_values[SourseID][ell_row_contains[SourseID]] = ell_values[SourseID][ell_row_contains[SourseID]] + 1; û��Ҫ
		//printf("ell:%d %d\n", ell_column_indices[SourseID][ell_row_contains[SourseID]], ell_values[SourseID][ell_row_contains[SourseID]]);
		ell_row_contains[SourseID]++; //��һ���Ž����Ĳ���Ҫcontains++
	}

	//2.ell_row_contains[SourseID]�����Է�����ʣ�µ�Ԫ�ط���COO��
	else {
		//printf("Adding an element to COO\n");
		//printf("hehe %d, %d\n", SourseID, DestiID);
		if (coo_contains >= COO_SIZE) {
			printf("ERROR: the COO is full\n");
			printf("hehe1 %d, %d\n", SourseID, DestiID);
			return -1;
		}
		for (int i = row_to_coo_index[SourseID]; i < row_to_coo_index[SourseID + 1]; i++) {
			if (coo_row[i]== SourseID && coo_column[i] == DestiID) { //��DestiID��Ӧ���Ѿ��������
				coo_values[i]++;
				//printf("coo1:%d %d\n", coo_column[i], coo_values[i]);
				row_to_coo_index[SourseID + 1]++;
				return;
			}
		}
		//��һ�η����Ԫ��
		coo_row[coo_contains] = SourseID;
		coo_column[coo_contains] = DestiID;
		coo_values[coo_contains]++;
		//printf("coo:%d %d %d\n", SourseID, coo_column[coo_contains], coo_values[coo_contains]);
		coo_contains++;
		//��ʾ������
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
	
	//test �鿴�¾�����Ԫ��
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
	��G�����ϵõ�Gm:G��ÿ��Ԫ��/��Ԫ������(���е�LinkInNum)
	*/

	//1.����ELL������Ԫ��
	for (int i = 0; i < MAX; i++) { //ÿ�ж�Ҫ����
		for (int k = 0; k < ell_row_contains[i]; k++) {
			ell_values[i][k] = ell_values[i][k] / LinkInNum[ell_column_indices[i][k]];
		}
	}

	//2.����COO������Ԫ��
	for (int i = 0; i < coo_contains; i++) {
		coo_values[i] = coo_values[i] / LinkInNum[coo_column[i]];
	}
}

void CalFeatureVector()
{
	/*
	�����Ӧ������������
	������A �Ľ׺ܴ��޷�ֱ�Ӽ���������ֵ����������ʱ����Ҫʹ��Power Iteration�ݵ�������
		1.�������A �͵�����ʼ����v���Լ�����e
		2.���㣺Vk+1 = AVk
		3.���|vk+1 - vk |< e>0�������PageRank ֵ��ֹͣ������ת�ڶ�����
	���x = A^k*v /sum(A^k*v)
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
			//�����˹�һ��,��һ�����ٲ�������
			normaliVector[i] = LastVector[i] / maxElement;
			FeatureVector[i] = 0;
		}
		
		
		

		maxElement = 0;
		//���ѵ�һ���֣�ϡ�����ĳ˷�Vk+1 = A*Vk
		for (int i = 0; i < MAX; i++) { //iΪ����
			//printf("counting at round %d\n", i);
			//����˷�ʱ��̫���ˣ�����coo����

			float calValue = 0;
			int k = 0; //����k�Ǵ���ell�ڵ�������
			//Vk+1[i] = ��(A[i][j]*VK[j][i]) j=1��2...MAX
			while (k < ell_row_contains[i]) {
				//if (ell_column_indices[i][k]==-1) { //���涼��0�������ˣ�ֱ�Ӷϵ�ȥ��һ��
				//	break;
				//}
				int colIndex = ell_column_indices[i][k]; //colIndex����ʹֵ��Ϊ0��j
				float ellValue = ell_values[i][k];
				calValue += ellValue * normaliVector[colIndex];
				//printf("ell_column_indices[i][k] is %d")
				k++;
			}
			if (k >= ell_row_contains[i]) { //ell�����ˣ�COO�ڻ���
				for (int p = row_to_coo_index[i]; p < row_to_coo_index[i+1]; p++) {
					if (coo_row[p] == i) { //coo�ڸ�Ԫ���ھ��������
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

		

		//��� | vk+1 - vk | < e>0�������PageRank ֵ��ֹͣ
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
		if (FeatureVector[i] > top10Rank[0]) { //top10Rank��pagerankֵ���������У�top10Rank[0]��С
			//printf("top10hehe1 %f\n", FeatureVector[i]);
			int tempIndex = 0;
			for (int k = 1; k <10 ; k++) { //�Ѿ�����top10Rank[0]��
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