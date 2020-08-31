#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99ûbool

//�ֱ���ƽ��������
#define branch 2 //����Ϊ����������������ڴ�����ʧ�ܣ����Ƚ�

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

//ƽ��������Ľṹ������Ҫ�����ݳ�Ա��ƽ�����ӣ��Լ����ҷ�֧
typedef struct trienode {
	struct trienode* chidren[branch]; //����Ϊ4����
	char end; //�Ƿ����ս�
}trienode, * trietree;

trienode* CreatTrie();
void InsertNode(trietree root, char* str);
bool FindNode(trietree root, char* str);


int main()
{
	unsigned int N = 1270688; //Ѫ����Ľ�ѵ
	unsigned int M = 986004;

	FILE* fp = fopen("patterns-127w.txt", "r");

	//struct btnode* root = NULL;
	trietree root = CreatTrie();

	//��ʼ��ȡ����
	for (int i = 0; i < N; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		//int len = strlen(tem);
		//��������
		InsertNode(root, tem);

		if (i % 10000 == 0)
			printf("%d\n", i);
	}
	fclose(fp);
	printf("%d %d\n", nodenum, mem);


	//��ʼƥ��
	fp = fopen("words-98w.txt", "r");
	FILE* fpwrite = fopen("result.txt", "w");
	int yescount = 0;
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		//int len = strlen(tem);

		//�����ֵ���������ƥ��
		if (FindNode(root, tem)) { //ƥ��ɹ�
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
trienode* CreatTrie() {
	trietree node = (trietree)malloc(sizeof(trienode));
	if (node == NULL) {
		printf("����ʧ��\n");
		printf("%d\n", nodenum);
		printf("%d\n", nodenum);
	}
		
	memset(node, 0, sizeof(trienode));
	//��C�����У�NULL��ʵ�ʵײ�����о���0,����Ϊ��Ŀ�ĺ���;������ʶ���ԭ��NULL����ָ��Ͷ���0������ֵ
	bupt_malloc(sizeof(trienode));
	return node;
}
void InsertNode(trietree root, char* str)
{
	if (*str == '\0')
		return;
	//һ��cha��8bit��4��֧��2bit��ʾ������8/2=4��������һ��cahr
	trienode* node = root;
	while (*str != '\0') {
		for (int k = 7; k >= 0; k--) {
			//cahr a = 1; //1�Ĳ���Ϊ00000001
			int loc = (*str & 1 << k) >> k;
			if (node->chidren[loc] == NULL) {
				trienode* tem = CreatTrie();
				node->chidren[loc] = tem;
			}
			node = node->chidren[loc];
		}
		str++;
	}
	node->end = 1;
}
bool FindNode(trietree root, char* str)
{
	char a = 'a', b = 'b';
	trietree node = root;
	while (*str != '\0') {
		for (int k = 7; k >= 0; k--) {
			//cahr a = 1; //1�Ĳ���Ϊ00000001
			int loc = (*str & 1 << k) >> k;
			int c = byte_cmp(a, b); //����Ƚϴ����ã����ø���
			if (node->chidren[loc] == NULL) { //����ʧ��
				return false;
			}
			node = node->chidren[loc];
		}
		str++;
	}
	if (node->end == 1) //���ڸõ���
		return true;
	return false;
}