#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99没bool

//分别定义平衡因子数
#define branch 2 //本例为二叉进制树，由于内存分配会失败，得先将

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

//平衡二叉树的结构定义需要：数据成员，平衡因子，以及左右分支
typedef struct trienode {
	struct trienode* chidren[branch]; //本例为4叉树
	char end; //是否能终结
}trienode, * trietree;

trienode* CreatTrie();
void InsertNode(trietree root, char* str);
bool FindNode(trietree root, char* str);


int main()
{
	unsigned int N = 1270688; //血与泪的教训
	unsigned int M = 986004;

	FILE* fp = fopen("patterns-127w.txt", "r");

	//struct btnode* root = NULL;
	trietree root = CreatTrie();

	//开始读取数据
	for (int i = 0; i < N; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		//int len = strlen(tem);
		//插入数据
		InsertNode(root, tem);

		if (i % 10000 == 0)
			printf("%d\n", i);
	}
	fclose(fp);
	printf("%d %d\n", nodenum, mem);


	//开始匹配
	fp = fopen("words-98w.txt", "r");
	FILE* fpwrite = fopen("result.txt", "w");
	int yescount = 0;
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		//int len = strlen(tem);

		//查找字典树来进行匹配
		if (FindNode(root, tem)) { //匹配成功
			fprintf(fpwrite, "%s %s", tem, "yes\n");
			yescount++;
		}
		else {
			fprintf(fpwrite, "%s %s", tem, "no\n");
		}
	}

	//写入最后一行数据
	int memsize = mem / 1024; //sizeof()以字节为单位，本题单位为KB
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
		printf("申请失败\n");
		printf("%d\n", nodenum);
		printf("%d\n", nodenum);
	}
		
	memset(node, 0, sizeof(trienode));
	//在C语言中，NULL在实际底层调用中就是0,但是为了目的和用途及容易识别的原因，NULL用于指针和对象，0用于数值
	bupt_malloc(sizeof(trienode));
	return node;
}
void InsertNode(trietree root, char* str)
{
	if (*str == '\0')
		return;
	//一个cha有8bit，4分支需2bit表示，共需8/2=4步处理完一个cahr
	trienode* node = root;
	while (*str != '\0') {
		for (int k = 7; k >= 0; k--) {
			//cahr a = 1; //1的补码为00000001
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
			//cahr a = 1; //1的补码为00000001
			int loc = (*str & 1 << k) >> k;
			int c = byte_cmp(a, b); //计算比较次数用，懒得改了
			if (node->chidren[loc] == NULL) { //查找失败
				return false;
			}
			node = node->chidren[loc];
		}
		str++;
	}
	if (node->end == 1) //存在该单词
		return true;
	return false;
}