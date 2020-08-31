#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99ûbool

#define MAX(a,b) (((a)>(b))?(a):(b))
//�ֱ���ƽ��������
#define BranchM 5 //m��B+��
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

//����B+���Ľڵ����ݽṹ
typedef struct bplus_node {
	int keynum; //�ýڵ�Ĺؼ��ֵĸ��������Կ���keys[]���߼��߽�
	KEY* keys; //���������max=m��,��Сmin=m/2��, �ռ�max+1���Ա����Ѳ���ʱ���м䲽��ʹ��
	/*int* data; //������ݵ����飬���max��,��Сmin��, �ռ�max+1���ڲ��ڵ�ʱ��data��NULL
	������keys�ʹ�����Ҫ��ŵ����ݣ��ʲ���Ҫdata�ֶ�*/
	struct bplus_node** child; //�ֽڵ�ָ������
	struct bplus_node* parent;
	struct bplus_node* next; //�ֵܽڵ㣬����Ҷ�ӽڵ���������
}bplus_node, * bplus_tree;

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

bplus_node* CreatBplusLeaf(); //����Ҷ�ӽڵ�
bplus_node* CreatBplusNode(); //�����м�ڵ�
int InserBplusTree(bplus_tree tree, KEY key);
int InserBplusLeaf(bplus_node* leaf, int key);
bool FindBplusTree(bplus_tree root, KEY key);

int binary_search(KEY* keys, KEY key, int left, int right, int* index);
bplus_tree root;
int main()
{
	unsigned int N = 1270688; //Ѫ����Ľ�ѵ
	unsigned int M = 986004;

	FILE* fp = fopen("../patterns-127w.txt", "r");

	root = CreatBplusLeaf();
	bplus_tree sqt = root;
	
	//��ʼ��ȡ����
	for (int i = 0; i < N; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		unsigned int result = BKDRHash(tem, len); //��ת����,������hash��,׼ȷ�Ļ�Ч��
		//��������
		InserBplusTree(root, result);

		//if (i == 1000)
		//	break;
	}
	fclose(fp);
	//int memsize = mem / 1024; //sizeof()���ֽ�Ϊ��λ�����ⵥλΪKB
	//printf("%d %d\n", nodenum, memsize);

	
	//��ʼƥ��
	fp = fopen("../words-98w.txt", "r");
	FILE* fpwrite = fopen("result.txt", "w");
	int yescount = 0;
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		unsigned int result = BKDRHash(tem, len); //��ת����,������hash��,׼ȷ�Ļ�Ч��
		//����B+�������в���ƥ��
		if (FindBplusTree(root, result)) { //ƥ��ɹ�
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


bplus_node* CreatBplusLeaf() //����Ҷ�ӽڵ�
{
	bplus_node* node = (bplus_node*)bupt_malloc(sizeof(bplus_node)); //bupt_malloc����
	memset(node, 0, sizeof(bplus_node)); //mallocֻ�Ƿ����ڴ棬�������г�ʼ��
	if (node == NULL) {
		printf("threre is a molloc error in CreatBplusLeaf\n");
		return NULL;
	}
	node->parent = NULL;
	node->next = NULL;
	node->keynum = 0;
	node->keys = (KEY*)bupt_malloc(sizeof(KEY) * (BranchM + 1)); //bupt_malloc����
	//��ʼ����keys
	memset(node->keys, 0, sizeof(node->keys));
	//
	node->child = NULL; 
	nodenum++;
	//bupt_malloc(sizeof(node));
	return node;
}
bplus_node* CreatBplusNode() //�����м�ڵ�
{
	bplus_node* node = (bplus_node*)bupt_malloc(sizeof(bplus_node)); //bupt_malloc����
	memset(node, 0, sizeof(bplus_node));
	if (node == NULL) {
		printf("threre is a molloc error in CreatBplusNode\n");
		return NULL;
	}
	node->parent = NULL;
	node->next = NULL;
	node->keynum = 0;
	node->keys = (KEY*)bupt_malloc(sizeof(KEY) * (BranchM + 1));
	//��ʼ����keys
	memset(node->keys, 0, sizeof(node->keys));
	//
	node->child = (bplus_node**)bupt_malloc(sizeof(bplus_node*) * (BranchM + 1)); //ԭ����+2 bupt_malloc����
	//��ʼ����child
	memset(node->child, 0, sizeof(node->child));
	//
	nodenum++;
	//bupt_malloc(sizeof(node));
	return node;
}
int InserBplusTree(bplus_tree tree, KEY key) {
	bplus_node* node = tree;
	//����Ҷ�ӽڵ�
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
				index++; //������node->child[index - 1]����keys[index]>keyʱ���ȼ����ټ���
				break;
			}
		}
		//if(flag==1)
		//printf("hehe:find child in index %d and key is %ud\n", index - 1, node->keys[index - 1]);
		node = node->child[index - 1];
		//�������ȸ����¸��ڵ�Ķ�Ӧ�ؼ���
		//if (node->parent) { //�ȸ����¸��ڵ�Ķ�Ӧ�ؼ���
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
	if (flag == 1) { //Ҷ�ӽڵ��Ѵ��ڸ�ֵ
		return 0;
	}

	if(node->parent) { //�ȸ����¸��ڵ�Ķ�Ӧ�ؼ���
		node->parent->keys[index-1] = MAX(key, node->keys[node->keynum - 1]);
	}
	InserBplusLeaf(node, key);
	return 1;
	
}
int InserBplusLeaf(bplus_node* leaf, int key)
{
	//����Ҷ�ӽڵ�
	int i;
	for (i = leaf->keynum; i > 0 && leaf->keys[i - 1] > key; i--) { //�ҵ��ؼ���key��keys�����еĲ���λ��
		leaf->keys[i] = leaf->keys[i - 1];
	}
	leaf->keys[i] = key;
	leaf->keynum++;

	KEY temkey;
	while (leaf->keynum > BranchM) { //�ڵ�ؼ�����������m�����з���
		bplus_node* node2;
		if (leaf->child == NULL) { //Ҷ�ӽڵ����
			//printf("creat a leaf\n");
			node2 = CreatBplusLeaf(); //Ҷ�ӽڵ���з���
		}
		else {
			//printf("creat a inter-node\n");
			node2 = CreatBplusNode(); //b�ڲ��ڵ����
		}
		/*test
		printf("hehe:tring to split:\n");
		for (int k = 0; k < leaf->keynum; k++)
			printf("%ud\n", leaf->keys[k]);
		if (leaf->parent) { //test
			printf("the leaf has parent, and his keynum is: %d\n", leaf->parent->keynum);
		}
		*/
		//��ʼ��������
		int mid = leaf->keynum / 2;
		//temkey = leaf->keys[mid];
		//
		if (leaf->child == NULL) {
			node2->keynum = leaf->keynum - mid;
			memcpy(node2->keys, leaf->keys + mid, sizeof(KEY) * (node2->keynum));
			temkey = node2->keys[node2->keynum - 1]; //���ڵ��Ӧkeys[]�ŵ���node������ֵ
			node2->next = leaf->next;
			leaf->next = node2;
		}
		else {
			node2->keynum = leaf->keynum - mid;
			memcpy(node2->keys, leaf->keys + mid, sizeof(KEY) * (node2->keynum));
			memcpy(node2->child, leaf->child + mid, sizeof(bplus_node*) * (leaf ->keynum - mid));
			temkey = node2->keys[node2->keynum - 1]; //���ڵ��Ӧkeys[]�ŵ���node������ֵ
			/* ���踸ָ�� */
			for (i = 0; i < node2->keynum; i++) { //ԭ����<=
				if (node2->child == NULL) {
					printf("NULL ERROW\N");
					printf("%d\n", i);
				}
				node2->child[i]->parent = node2;
			}
		}
		leaf->keynum = mid; //keynum��keys���߼�����������ʵ��������keys[]��

		bplus_node* parent = leaf->parent;
		//���븸�ڵ�
		if (parent == NULL) {
			//printf("creat a new parent node\n");
			parent = CreatBplusNode();
			//parent->parent = NULL; //��γ�ʼ������û�ɹ�
			parent->child[0] = leaf;
			parent->keys[0] = leaf->keys[leaf->keynum - 1]; //�Լ��ӵģ�����µĸ��ڵ�����leaf�Ĺؼ�ֵѽ
			parent->keynum = 1; //�½����ڵ��Ȱ�leaf���ȥ
			leaf->parent = parent;
			root = parent;
		}
		/* �������ݺ������� */
		for (i = parent->keynum; i > 0 && parent->keys[i - 1] > temkey; i--) { //�ҵ�node2�Ĳ���ռ�
			parent->keys[i] = parent->keys[i - 1];
			parent->child[i] = parent->child[i - 1];
		}
		//����i--��֮����ִ��for()�ڵ�����ж�
		parent->keys[i - 1] = leaf->keys[leaf->keynum - 1];//���Ѻ��leaf�ĸ��ڵ�ؼ��ָ���
		parent->keys[i] = temkey;
		parent->child[i] = node2;
		parent->keynum++;

		node2->parent = parent;
		leaf = parent; //�����ж��ܷ�����ѭ��
	}
	
	return 1;
}
bool FindBplusTree(bplus_tree root, KEY key)
{
	if (key > root->keys[root->keynum - 1]) //������ֵ�����Ҳ�����
		return false;
	bplus_node* node = root;
	int index;
	while (node->child != NULL) { //����ֱ��Ҷ�ӽڵ�
		int flag = binary_search(node->keys, key, 0, node->keynum - 1, &index);
		if (flag == 1) //keys�е�Ԫ��һ��������ĳҶ�ӽڵ㣬���ҵ��Ļ�ֱ��return true����
			return true;
		node = node->child[index];
	}
	//��Ҷ�ӽڵ��ϲ���
	int flag = binary_search(node->keys, key, 0, node->keynum - 1, &index);
	if (flag == 1)
		return true;
	return false;
	
}
int binary_search(KEY* keys, KEY key, int left, int right, int* index) {
	/*���ֲ��ң��ҵ�������ָ��key��λ�ã�������ڣ�����1, ���򷵻�0,
	����ҵ�����Ϊ��ֵλ�ã����򷵻�����ֵλ��(child��λ��)*/
	while (left <= right) {
		byte_cmp(a, b); //����Ƚϴ����ã����ø���
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