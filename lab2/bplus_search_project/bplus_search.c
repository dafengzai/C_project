#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99没bool

#define MAX(a,b) (((a)>(b))?(a):(b))
//分别定义平衡因子数
#define BranchM 5 //m阶B+树
typedef unsigned int KEY;

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
char a, b;
int byte_cmp(char a, char b)
{
	cmpnum++;
	return a - b;
}

//定义B+树的节点数据结构
typedef struct bplus_node {
	int keynum; //该节点的关键字的个数，可以看作keys[]的逻辑边界
	KEY* keys; //主健，最大max=m个,最小min=m/2个, 空间max+1，以备分裂操作时的中间步骤使用
	/*int* data; //存放数据的数组，最大max个,最小min个, 空间max+1，内部节点时，data是NULL
	本例中keys就代表了要存放的数据，故不需要data字段*/
	struct bplus_node** child; //字节的指针数组
	struct bplus_node* parent;
	struct bplus_node* next; //兄弟节点，仅在叶子节点内有意义
}bplus_node, * bplus_tree;

unsigned int BKDRHash(const char* str, int length) //网上找的BKDR哈希函数
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

bplus_node* CreatBplusLeaf(); //建立叶子节点
bplus_node* CreatBplusNode(); //建立中间节点
int InserBplusTree(bplus_tree tree, KEY key);
int InserBplusLeaf(bplus_node* leaf, int key);
bool FindBplusTree(bplus_tree root, KEY key);

int binary_search(KEY* keys, KEY key, int left, int right, int* index);
bplus_tree root;
int main()
{
	unsigned int N = 1270688; //血与泪的教训
	unsigned int M = 986004;

	FILE* fp = fopen("../patterns-127w.txt", "r");

	root = CreatBplusLeaf();
	bplus_tree sqt = root;
	
	//开始读取数据
	for (int i = 0; i < N; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		unsigned int result = BKDRHash(tem, len); //先转数字,还是用hash吧,准确的换效率
		//插入数据
		InserBplusTree(root, result);

		//if (i == 1000)
		//	break;
	}
	fclose(fp);
	//int memsize = mem / 1024; //sizeof()以字节为单位，本题单位为KB
	//printf("%d %d\n", nodenum, memsize);

	
	//开始匹配
	fp = fopen("../words-98w.txt", "r");
	FILE* fpwrite = fopen("result.txt", "w");
	int yescount = 0;
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		unsigned int result = BKDRHash(tem, len); //先转数字,还是用hash吧,准确的换效率
		//查找B+树来进行查找匹配
		if (FindBplusTree(root, result)) { //匹配成功
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


bplus_node* CreatBplusLeaf() //建立叶子节点
{
	bplus_node* node = (bplus_node*)bupt_malloc(sizeof(bplus_node)); //bupt_malloc更改
	memset(node, 0, sizeof(bplus_node)); //malloc只是分配内存，并不进行初始化
	if (node == NULL) {
		printf("threre is a molloc error in CreatBplusLeaf\n");
		return NULL;
	}
	node->parent = NULL;
	node->next = NULL;
	node->keynum = 0;
	node->keys = (KEY*)bupt_malloc(sizeof(KEY) * (BranchM + 1)); //bupt_malloc更改
	//初始化下keys
	memset(node->keys, 0, sizeof(node->keys));
	//
	node->child = NULL; 
	nodenum++;
	//bupt_malloc(sizeof(node));
	return node;
}
bplus_node* CreatBplusNode() //建立中间节点
{
	bplus_node* node = (bplus_node*)bupt_malloc(sizeof(bplus_node)); //bupt_malloc更改
	memset(node, 0, sizeof(bplus_node));
	if (node == NULL) {
		printf("threre is a molloc error in CreatBplusNode\n");
		return NULL;
	}
	node->parent = NULL;
	node->next = NULL;
	node->keynum = 0;
	node->keys = (KEY*)bupt_malloc(sizeof(KEY) * (BranchM + 1));
	//初始化下keys
	memset(node->keys, 0, sizeof(node->keys));
	//
	node->child = (bplus_node**)bupt_malloc(sizeof(bplus_node*) * (BranchM + 1)); //原版是+2 bupt_malloc更改
	//初始化下child
	memset(node->child, 0, sizeof(node->child));
	//
	nodenum++;
	//bupt_malloc(sizeof(node));
	return node;
}
int InserBplusTree(bplus_tree tree, KEY key) {
	bplus_node* node = tree;
	//查找叶子节点
	int index = 0;
	//printf("hehe:there's %d key in tree. they are:\n", node->keynum);
	/* test
	if (node->keynum > 0) { 
		for (int k = 0; k < node->keynum; k++)
			printf("%ud\n", node->keys[k]);
	}
	*/
	while (node->child != NULL) {
		//printf("hehe:finding keys\n");
		for (index = 0; index < node->keynum; index++) {
			//printf("hehe:I find %ud.\n", node->keys[index]);
			if (node->keys[index] > key) {
				index++; //下面是node->child[index - 1]，当keys[index]>key时，先加下再减。
				break;
			}
		}
		//if(flag==1)
		//printf("hehe:find child in index %d and key is %ud\n", index - 1, node->keys[index - 1]);
		node = node->child[index - 1];
		//待定：先更新下父节点的对应关键字
		//if (node->parent) { //先更新下父节点的对应关键字
		node->parent->keys[index - 1] = MAX(key,node->keys[node->keynum-1]);
		//}
	}
	int flag = 0;
	for (int k = 0; k < node->keynum; k++) {
		if (node->keys[k] == key) {
			flag = 1;
			break;
		}
			
	}
	if (flag == 1) { //叶子节点已存在该值
		return 0;
	}

	if(node->parent) { //先更新下父节点的对应关键字
		node->parent->keys[index-1] = MAX(key, node->keys[node->keynum - 1]);
	}
	InserBplusLeaf(node, key);
	return 1;
	
}
int InserBplusLeaf(bplus_node* leaf, int key)
{
	//插入叶子节点
	int i;
	for (i = leaf->keynum; i > 0 && leaf->keys[i - 1] > key; i--) { //找到关键字key在keys数组中的插入位置
		leaf->keys[i] = leaf->keys[i - 1];
	}
	leaf->keys[i] = key;
	leaf->keynum++;

	KEY temkey;
	while (leaf->keynum > BranchM) { //节点关键字数超过了m，进行分裂
		bplus_node* node2;
		if (leaf->child == NULL) { //叶子节点分裂
			//printf("creat a leaf\n");
			node2 = CreatBplusLeaf(); //叶子节点进行分裂
		}
		else {
			//printf("creat a inter-node\n");
			node2 = CreatBplusNode(); //b内部节点分裂
		}
		/*test
		printf("hehe:tring to split:\n");
		for (int k = 0; k < leaf->keynum; k++)
			printf("%ud\n", leaf->keys[k]);
		if (leaf->parent) { //test
			printf("the leaf has parent, and his keynum is: %d\n", leaf->parent->keynum);
		}
		*/
		//开始拷贝数据
		int mid = leaf->keynum / 2;
		//temkey = leaf->keys[mid];
		//
		if (leaf->child == NULL) {
			node2->keynum = leaf->keynum - mid;
			memcpy(node2->keys, leaf->keys + mid, sizeof(KEY) * (node2->keynum));
			temkey = node2->keys[node2->keynum - 1]; //父节点对应keys[]放的是node中最大的值
			node2->next = leaf->next;
			leaf->next = node2;
		}
		else {
			node2->keynum = leaf->keynum - mid;
			memcpy(node2->keys, leaf->keys + mid, sizeof(KEY) * (node2->keynum));
			memcpy(node2->child, leaf->child + mid, sizeof(bplus_node*) * (leaf ->keynum - mid));
			temkey = node2->keys[node2->keynum - 1]; //父节点对应keys[]放的是node中最大的值
			/* 重设父指针 */
			for (i = 0; i < node2->keynum; i++) { //原来是<=
				if (node2->child == NULL) {
					printf("NULL ERROW\N");
					printf("%d\n", i);
				}
				node2->child[i]->parent = node2;
			}
		}
		leaf->keynum = mid; //keynum是keys的逻辑长，数据其实还存在于keys[]内

		bplus_node* parent = leaf->parent;
		//插入父节点
		if (parent == NULL) {
			//printf("creat a new parent node\n");
			parent = CreatBplusNode();
			//parent->parent = NULL; //这段初始化好像没成功
			parent->child[0] = leaf;
			parent->keys[0] = leaf->keys[leaf->keynum - 1]; //自己加的，妈的新的父节点必须加leaf的关键值呀
			parent->keynum = 1; //新建父节点先把leaf算进去
			leaf->parent = parent;
			root = parent;
		}
		/* 增加数据和右子树 */
		for (i = parent->keynum; i > 0 && parent->keys[i - 1] > temkey; i--) { //找到node2的插入空间
			parent->keys[i] = parent->keys[i - 1];
			parent->child[i] = parent->child[i - 1];
		}
		//是先i--，之后再执行for()内的真假判断
		parent->keys[i - 1] = leaf->keys[leaf->keynum - 1];//分裂后的leaf的父节点关键字更新
		parent->keys[i] = temkey;
		parent->child[i] = node2;
		parent->keynum++;

		node2->parent = parent;
		leaf = parent; //用于判断能否跳出循环
	}
	
	return 1;
}
bool FindBplusTree(bplus_tree root, KEY key)
{
	if (key > root->keys[root->keynum - 1]) //比最大的值还大，找不到的
		return false;
	bplus_node* node = root;
	int index;
	while (node->child != NULL) { //遍历直到叶子节点
		int flag = binary_search(node->keys, key, 0, node->keynum - 1, &index);
		if (flag == 1) //keys中的元素一定存在于某叶子节点，查找到的话直接return true即可
			return true;
		node = node->child[index];
	}
	//在叶子节点上查找
	int flag = binary_search(node->keys, key, 0, node->keynum - 1, &index);
	if (flag == 1)
		return true;
	return false;
	
}
int binary_search(KEY* keys, KEY key, int left, int right, int* index) {
	/*二分查找，找到数组中指定key的位置，如果存在，返回1, 否则返回0,
	如果找到索引为该值位置，否则返回右邻值位置(child的位置)*/
	while (left <= right) {
		byte_cmp(a, b); //计算比较次数用，懒得改了
		int mid = (left + right) / 2;
		if (keys[mid] == key) {
			*index = mid;
			return 1;
		}
		else if (keys[mid] < key) {
			left = mid+1;
		}
		else {
			right = mid-1;
		}
	}
	*index = left; //keys[left]>key>keys[left-1]
	return -1;
}