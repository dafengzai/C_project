#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99ûbool

//�ֱ���ƽ��������
#define LH +1
#define EH  0
#define RH -1

int mem = 0;
int nodenum = 0;
int cmpnum = 0;

void* bupt_malloc(size_t size) {  //size_t:used to represent the size of an object
	if (size <= 0)
		return NULL;
	mem += size;
	nodenum++;
	return malloc(size);
}
int byte_cmp(char a, char b)
{
	cmpnum++;
	return a - b;
}
unsigned int BKDRHash(const char* str, int length) //�����ҵ�BKDR��ϣ����
{
	unsigned int seed = 131; /* 31 131 1313 13131 131313 etc.. */
	unsigned int hash = 0;
	unsigned int i = 0;

	for (i = 0; i < length; ++str, ++i)
	{
		hash = (hash * seed) + (*str);
	}

	return hash;
}

//ƽ��������Ľṹ������Ҫ�����ݳ�Ա��ƽ�����ӣ��Լ����ҷ�֧
typedef struct btnode {
	unsigned int val; //���ַ���ת��Ϊ���ֺ��ٱȽ�
	int bf; //ƽ������balance flag
	struct btnode* left, * right;
}btnode, *btree;
int InsertAVL(btree* root, unsigned int value, bool* taller);
bool FindNode(btree root, unsigned int value);

void LeftBalance(btree* root);
void RightBalance(btree* root);
//ƽ���������ʧ�������Ҫ��ͨ����ת��Сʧ��������ʵ�ֵġ�������ת�ķ��������ִ���ʽ:���������� ��[http://data.biancheng.net/view/59.html]
void L_Rotate(btree* root);//1.�ڵ���Һ�������˽ڵ�λ�� 2.�Һ��ӵ���������Ϊ�ýڵ��������,3.�ڵ㱾���Ϊ�Һ��ӵ�������
void R_Rotate(btree* root);//1.�ڵ�����Ӵ���˽ڵ�,2.�ڵ�����ӵ���������Ϊ�ڵ��������,3.���˽ڵ���Ϊ���ӽڵ����������

int main()
{
	unsigned int N = 1270688; //Ѫ����Ľ�ѵ
	unsigned int M = 986004;
	
	FILE* fp = fopen("patterns-127w.txt", "r");

	//struct btnode* root = NULL;
	btree root = NULL; 
	//��ʼ��ȡ����
	for (int i = 0; i < N; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		//��stringת��Ϊ���֣�ʹ��c99�Դ�����strtolû���ã����ǵ�Ӳ��
		int len = strlen(tem);
		int result = BKDRHash(tem, len); //��ת���֣�������λ���ϴ�120+�����֣��������û���ã�������hash��
		//for (int j = 0; j < len; j++) {
		//	result = result * 10 + (tem[j] - '0');
		//}
		//��������ƽ����
		bool taller;
		InsertAVL(&root, result, &taller); //�����ַ�ĵ�ַ���Դ��õ�ַ�ڵĽڵ�Ԫ�ظı䣨��ַ�ں����ڲ���䣬����ַ��Ԫ�ؿ��Ըı丳ֵ�ɽڵ㣩
		/*
		˵�����ڲ����µĽ���ʱ������ʹ��һ��taller�ı�������¼���ĸ߶��Ƿ�仯��Ĭ����Ϊ���ĸ߶��������ӵġ�
		�����ڲ����µĽ��������ж����ĸ߶��Ƿ������ˣ��������ĸ߶�û�б仯�����ؽ�����β�����
		�����ĸ߶�����ʱ�����ǾͿ����Ƿ���Ҫ�����Ľ���ƽ�������
		*/
		//if (i == 1000)
		//	break;
	}
	fclose(fp);

	//��ʼƥ��
	fp = fopen("words-98w.txt", "r");
	FILE* fpwrite = fopen("result.txt", "w");
	int yescount = 0;
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		//��stringת��Ϊ���֣�ʹ��c99�Դ�����strtolû���ã����ǵ�Ӳ��
		int len = strlen(tem);
		unsigned int result = BKDRHash(tem, len);
		//for (int j = 0; j < len; j++) {
		//	result = result * 10 + (tem[j] - '0');
		//}

		//����ƽ�������������ƥ��
		if(FindNode(root, result)){ //ƥ��ɹ�
			fprintf(fpwrite, "%s %s", tem, "yes\n");
			yescount++;
		}
		else {
			fprintf(fpwrite, "%s %s", tem, "no\n");
		}
	}

	//д�����һ������
	int memsize = mem / 1024; //sizeof()���ֽ�Ϊ��λ�����ⵥλΪKB
	cmpnum = cmpnum / 1000;
	fprintf(fpwrite, "%d %d %d %d %d", nodenum, memsize, cmpnum, M, yescount);
	fclose(fpwrite);

	fclose(fp);
	fclose(fpwrite);
	
	return 0;

}

int InsertAVL(btree* root,unsigned int value, bool* taller) {
	if (*root == NULL) {  //��һ���ڵ㸳ֵ�ˣ�����Ҫ�ȶ����
		//printf("hehe\n");
		*root = (btree)malloc(sizeof(btnode));
		bupt_malloc(sizeof(btnode));
		(*root)->left = NULL;
		(*root)->right = NULL;
		(*root)->val = value;
		(*root)->bf = EH;
		*taller = true;
	}
	//printf("heihei\n");
	else if (value == (*root)->val) { //�����ַ�������һ�����ò��������
		*taller = false;
		return 0;
	}
	//���valueС��root��Ԫ�أ�������������
	else if (value < (*root)->val) {
		int flag = InsertAVL(&(*root)->left, value, taller);
		if (flag == 0) //���벻��Ӱ������ƽ��
			return 0;
		//�жϲ�������Ƿ�ᵼ�������+1
		if (*taller) {
			/*
			���߶����Ӻ����жϸ��ڵ��ƽ�����ӣ���������������������½��Ĺ����е���ʧȥƽ�⣬
			���Ե�������ԭƽ�����ӱ���Ϊ 1 ʱ�������½ڵ��+1=2�ˣ�����Ҫ������������ƽ�⴦������������и�����ƽ��������
			*/
			switch ((*root)->bf)
			{
			case LH:
				LeftBalance(root);
				*taller = false; //��ƽ��󸸽ڵ�����Ķ���
				break;
			case EH:
				(*root)->bf = LH;
				*taller = true; //ԭ��ΪEH���Ըýڵ���Ӱ�죬����Ϊtrue�ø��ڵ�����ж�
				break;
			case RH:
				(*root)->bf = EH;
				*taller = false; //��Ӻ�ڵ�ƽ���ˣ������ø��ڵ�����ж�
				break;
			}
		}
	}
	else if (value > (*root)->val) { //ͬ��
		int flag = InsertAVL(&(*root)->right, value, taller);
		if (flag == 0) //�ڵ����Ϊ�ı�����ƽ��
			return 0;
		if (*taller) {
			switch ((*root)->bf)
			{
			case LH:
				(*root)->bf = EH;
				*taller = false; //��ƽ������߻�ָ�
				break;
			case EH:
				(*root)->bf = RH;
				*taller = true;
				break;
			case RH:
				RightBalance(root);
				*taller = false; //����Ľڵ㲻����ƽ�⺯����taller��false��������ж�
				break;
			}
		}
	}
	return 1; //����ڵ�Ĭ���ǻ��ƽ�����Ӱ��ģ����ɸ��ڵ��ж�
}
void LeftBalance(btree* root) {
	btree lc, lrc;
	lc = (*root)->left;
	//�鿴�� T ��������Ϊ������������ʧȥƽ���ԭ����� bf ֵΪ 1 ����˵�������������Ϊ�������������У���Ҫ���������������
	//��֮����� bf ֵΪ -1��˵���������������Ϊ�������������У���Ҫ����˫���������������Ĵ���
	switch (lc->bf)
	{
	//��Ȼroot��ƽ���ˣ�+2�������������϶�bf�㲻����Ϊ0
	case LH:
		(*root)->bf = lc->bf = EH;
		R_Rotate(root);
		break;
	case RH: //root�����ӵ�����������ڵ㣬ʹ�����������ת
		lrc = lc->right;
		switch (lrc->bf) //���½ڵ�bf,����[http://data.biancheng.net/view/59.html]��ͼ7
		{
		case LH:
			(*root)->bf = RH; lc->bf = EH; break;
		case RH:
			(*root)->bf = EH; lc->bf = LH; break;
		case EH:
			(*root)->bf = EH; lc->bf = EH; break;
		}
		lrc->bf = EH;
		L_Rotate(&(*root)->left);
		R_Rotate(root);
		break;
	}
}
void RightBalance(btree* root) {
	btree rc, rlc;
	rc = (*root)->right;
	//�鿴�� T ��������Ϊ������������ʧȥƽ���ԭ����� bf ֵΪ -1 ����Ҫ���������������
	//��֮����� bf ֵΪ 1�����󣩣�˵���������������Ϊ�������������У���Ҫ����˫���������������Ĵ���
	switch (rc->bf)
	{
		//��Ȼroot��ƽ���ˣ�+2�������������϶�bf�㲻����Ϊ0
	case RH:
		(*root)->bf = rc->bf = EH;
		L_Rotate(root);
		break;
	case LH: //root�����ӵ�����������ڵ㣬ʹ�����������ת
		rlc = rc->left;
		switch (rlc->bf) //���½ڵ�bf,����[http://data.biancheng.net/view/59.html]��ͼ8
		{
		case LH:
			(*root)->bf = EH; rc->bf = RH; break;
		case RH:
			(*root)->bf = LH; rc->bf = EH; break;
		case EH:
			(*root)->bf = EH; rc->bf = EH; break;
		}
		rlc->bf = EH;
		R_Rotate(&(*root)->right); //����ַ�ĵ�ַ����������
		
		L_Rotate(root);
		break;
	}
}
void L_Rotate(btree* root) {
	//1.�ڵ���Һ�������˽ڵ�λ�� 2.�Һ��ӵ���������Ϊ�ýڵ��������,3.�ڵ㱾���Ϊ�Һ��ӵ�������
	btree rc = (*root)->right;
	(*root)->right = rc->left;
	rc->left = *root;
	*root = rc;
}
void R_Rotate(btree* root) {
	//1.�ڵ�����Ӵ���˽ڵ�,2.�ڵ�����ӵ���������Ϊ�ڵ��������,2.���˽ڵ���Ϊ���ӽڵ����������
	btree lc = (*root)->left;
	(*root)->left = lc->right;
	lc->right = *root;
	*root = lc;
}
bool FindNode(btree root, unsigned int value) {
	//�����Ͼ��Ƕ������Ĳ���
	btree tem = root;
	char a = 'a', b = 'b';
	while (tem) {
		int c = byte_cmp(a, b); //����Ƚϴ����ã����ø���
		if (value == tem->val) { //���ҳɹ�
			return true;
		}
		else if (value < tem->val) {
			tem = tem->left;
		}
		else {
			tem = tem->right;
		}
	}
	return false; //���������ҵ�ѭ����û�ҵ�
}
