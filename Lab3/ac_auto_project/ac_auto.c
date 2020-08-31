#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99没bool
#include<time.h>

//分别定义平衡因子数
#define branch 4

int mem = 0;
long long int cmpcount = 0;

void* bupt_malloc(size_t size) {  //size_t:used to represent the size of an object
	if (size <= 0)
		return NULL;
	mem += size;
	return malloc(size);
}

//qsort用的节点数据类型
typedef struct patternNode { //优先队列的节点
	char* data;
	int priority; //priotity:
}patternNode;
//qsort用比较函数
int comparator(const void* a, const void* b) //升序需返回正值（a>b）
{
	int l = ((patternNode*)a)->priority;
	int r = ((patternNode*)b)->priority;
	return r - l; //<0 The element pointed by a goes before the element pointed by b
}


//ac自动机中字典树的结构定义需要：数据成员，平衡因子，以及左右分支
typedef struct trienode {
	struct trienode* chidren[branch]; //本例为4叉树
	struct trienode* fail; //fail指针
	int end; //是否能终结，>0表示该节点为某单词结尾，外加计数
}trienode, * trietree;
trienode* CreatTrie();
void InsertNode(trietree root, char* str);
void PrintTrie(trietree root, char* str, patternNode* patterns, int index);
//AC自动机相关
void BuildFailPointer(trienode* root); //第二步：建立失效指针
void ACSearch(trienode* root); //AC自动机进行模式匹配

//**的还得自己写一个队列来实现BFS
//定义节点结构
typedef struct QNode
{
	trienode* data;   /*数据域*/
	struct QNode* next;   /*指针域*/
}QNode, * QueuePtr;
//定义队列结构
typedef struct
{
	QueuePtr front;
	QueuePtr rear;
	int size;
}LinkQueue;

LinkQueue* initQueue(trienode* root); //链队列初始化
void pushQueue(LinkQueue* Q, trienode* e); //入队
void popQueue(LinkQueue* Q);
bool isEmptyQueue(LinkQueue* Q);
QNode* peekQueue(LinkQueue* Q); //查看头元素





int main()
{
	//计时
	clock_t start_t, finish_t;
	double total_t = 0;
	start_t = clock();

	/*大致流程分三部分：
	一是读取pattern.txt。做一个大包含faill指针的字典树，并建立fail指针
	二：读取string.txt，进行AC自动机查找，并将查找结果写入Trie树中先
	三：三：遍历trie树，将比较结果输出到一个大patterns的节点数组中存储
	四：对节点数组进行快排，之后导出到result.txt
	*/

	/*
	一：读取patter.txt文件，做字典树
	*/
	FILE* fp = fopen("../../pattern.txt", "r");
	FILE* fpwrite = fopen("output.txt", "w");

	trietree root = CreatTrie();
	//测试
	int check = 0;
	//unsigned int M = 1000;
	unsigned int M = 2256690;
	for (int i = 0; i < M; i++) {
		char str[150];
		fgets(str, 150, fp);
		str[strlen(str) - 1] = '\0';
		int len = strlen(str);

		/*
		添加进字典树
		*/
		InsertNode(root, str);

	}
	printf("trie树占用内存：%dKB\n", mem  / 1024);
	BuildFailPointer(root); //建立fail指针
	

	fclose(fp);
	fclose(fpwrite);
	printf("STEP1:读取pattern.txt。做一个大包含faill指针的字典树，并建立fail指针 OVER\n");
	//计时
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//将时间转换为秒
	printf("CPU 占用的总时间：%f\n", total_t);
	start_t = clock();

	/*
	二：读取string.txt，进行AC自动机查找，并将查找结果写入Trie树中先（end值用于计数）
	*/
	ACSearch(root);
	printf("STEP2:读取string.txt，进行AC自动机查找，并将查找结果写入Trie树中先 OVER\n");
	//计时
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//将时间转换为秒
	printf("CPU 占用的总时间：%f\n", total_t);
	start_t = clock();

	/*
	三：遍历trie树，将比较结果输出到一个大patterns的节点数组中存储
	*/
	fp = fopen("../../pattern.txt", "r");
	//fpwrite = fopen("output.txt", "w");
	patternNode* patterns = (patternNode*)malloc(sizeof(patternNode) * M);
	for (int i = 0; i < M; i++) {
		//char tem[150]; //得新分配一个char给他，不然patterns内元素会指向同一个char
		char* tem = (char*)malloc(sizeof(char) * 150);
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		//int len = strlen(tem);

		//查找字典树来进行匹配
		PrintTrie(root, tem, patterns, i);
		//free(tem);放进node里了，不能free
	}

	fclose(fp);
	printf("STEP3:遍历trie树，将比较结果输出到一个大patterns的节点数组中存储 OVER\n");
	//计时
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//将时间转换为秒
	printf("CPU 占用的总时间：%f\n", total_t);
	start_t = clock();

	/*
	四：对节点数组进行快排，之后导出到result.txt
	*/
	fpwrite = fopen("result.txt", "w");

	//220万级别的数据太大了，使用自带函数qsort()
	//要使用qsort(),首先得定义一个comparator函数

	
	
	//对patterns进行排序
	/*void qsort (void* base, size_t num, size_t size,  
            int (*comparator)(const void*,const void*));*/
	qsort(patterns, M, sizeof(patternNode), comparator);


	//结果写入result.txt
	//patternNode* temPattern = patterns;
	for (int i = 0; i < M; i++) {
		//if (i % 100000 == 0)
		//	printf("%d\n", i);
		fprintf(fpwrite, "%s %d\n", (patterns + i)->data, (patterns + i)->priority);
		//temPattern++;
	}

	//写入最后一行
	fprintf(fpwrite, "%lld %d\n", cmpcount, mem / 1024); //最后一行输出两个数：字节比较次数K 内存开销（KB）
	//printf("统计结果word count; kb：%d %d\n", cmpcount, (mem * 8) / 1024);

	fclose(fpwrite);
	printf("STEP4:把output文件读取进来并排序，之后导出到result.txt OVER\n");

	//计时
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//将时间转换为秒

	printf("CPU 占用的总时间：%f\n", total_t);

	return 0;

}


//AC自动机函数
void BuildFailPointer(trienode* root)
{
	root->fail = root; //中断判断

	LinkQueue* Q = initQueue(root);
	while (!isEmptyQueue(Q)) {
		trienode* parent = peekQueue(Q)->data;
		popQueue(Q);
		for (int i = 0; i < branch; i++) { //对每一个子节点设置FailedPointer
			if (parent->chidren[i]) {
				trienode* child = parent->chidren[i];
				pushQueue(Q, child);
				trienode* candidate = parent->fail; //从father->FailedPointer开始游走的指针
				while (1) {
					if (parent == root) {
						candidate = root;
						break;
					}
					if (candidate->chidren[i]) {//有与child相同的子节点
						candidate = candidate->chidren[i];
						break;
					}
					else {
						if (candidate == root)
							break;
						candidate = candidate->fail;//以上两句顺序不能交换，因为在root仍可以做一次匹配
					}
				}
				child->fail = candidate;
			}
		}
		
	}
}

void ACSearch(trienode* root) //读的是大文件
{
	FILE* fpstr = fopen("../../string.txt", "r");
	//FILE* fpcount = fopen("wordcount.txt", "w");
	trienode* p = root;
	char c;
	//test
	//int test = 0;
	while ((c = (char)fgetc(fpstr)) != EOF) {
		if (c != '\0') {
			for (int k = 3; k >= 0;) {
				//cahr a = 3; //3的补码为00000011
				int loc = (c & 3 << (k * 2)) >> (k * 2);
				cmpcount++; //统计比较次数
				if (p->chidren[loc]) { //查找成功
					trienode* temp = p->chidren[loc]->fail;
					while (temp != root && temp != NULL) {//在匹配的情况下，仍然沿FailedPointer搜索，可检索出所有模式。
						if (temp->end != 0) {//该节点是单词尾节点。
							//fprintf(fpcount, "%s %d\n", p->data, 1);
							p->end++;
						}
						temp = temp->fail;
					}

					p = p->chidren[loc];
					if (p->end != 0) { //该节点是单词尾节点。
						//fprintf(fpcount, "%s %d\n", p->data, 1);
						p->end++;
					}
					k--;
				}
				else { //失配，跳转
					if (p == root)
						k--;
					else {
						p = p->fail;
					}
				}
				
			}
		}
		/*
		test++;
		if (test > 10000000)
			break;//*/
	}
	fclose(fpstr);
	//fclose(fpcount);
}

//队列相关函数
LinkQueue* initQueue(trienode* root) //链队列初始化
{
	LinkQueue* Q = (LinkQueue*)bupt_malloc(sizeof(LinkQueue));
	//Q->front = Q->rear = (QueuePtr)bupt_malloc(sizeof(QNode));
	QNode* temp = (QNode*)bupt_malloc(sizeof(QNode));
	temp->data = root;
	temp->next = NULL;
	//if (!Q->front) exit(OVERFLOW);
	Q->front = Q->rear = temp;
	Q->size = 1;
	return Q;
}
void pushQueue(LinkQueue* Q, trienode* e) //入队
{
	if (isEmptyQueue(Q)) {
		//printf("push the first element in queue\n");
		//Q = initQueue(e); 传入函数的参数不能修改，赋另一个Q的地址给他是不行的
		QNode* temp = (QNode*)bupt_malloc(sizeof(QNode));
		temp->data = e;
		temp->next = NULL;
		Q->front = Q->rear = temp;
		Q->size = 1;
		return;
	}
	QueuePtr p = (QueuePtr)bupt_malloc(sizeof(QNode));
	//if (!p) exit(OVERFLOW);
	//插入数据
	p->data = e;
	p->next = NULL;
	//Q.rear一直指向队尾
	Q->rear->next = p;
	Q->rear = p;
	Q->size++;
}
void popQueue(LinkQueue* Q)
{
	if (isEmptyQueue(Q)) printf("ERROR");
	QueuePtr p = Q->front;
	Q->front = p->next;   //队头元素p出队
	if (Q->rear == p)   //如果队中只有一个元素p, 则p出队后成为空队
		Q->rear = Q->front;     //给队尾指针赋值
	free(p);   //释放存储空间
	Q->size--;
}
bool isEmptyQueue(LinkQueue* Q)
{
	return (Q->front == NULL);
}
QNode* peekQueue(LinkQueue* Q) //查看头元素
{
	return Q->front;
}

//ac自动机中trie树相关
trienode* CreatTrie() {
	trienode* node = (trienode*)bupt_malloc(sizeof(trienode));
	memset(node, 0, sizeof(trienode));
	node->fail = NULL;
	//for (int i = 0; i < branch; i++) {
	//	node->chidren[i] = NULL;
	//}
	//在C语言中，NULL在实际底层调用中就是0,但是为了目的和用途及容易识别的原因，NULL用于指针和对象，0用于数值
	//node->chidren = (trienode*)bupt_malloc(sizeof(trienode) * branch); 我children用数组存的
	return node;
}

void InsertNode(trietree root, char* str)
{
	if (*str == '\0')
		return;
	char* temp = str;
	//一个cha有8bit，4分支需2bit表示，共需8/2=4步处理完一个cahr
	trienode* node = root;
	while (*temp != '\0') {
		for (int k = 3; k >= 0; k--) {
			//cahr a = 3; //3的补码为00000011
			int loc = (*temp & 3 << (k * 2)) >> (k * 2);
			if (node->chidren[loc] == NULL) {
				trienode* tem = CreatTrie();
				node->chidren[loc] = tem;
			}
			node = node->chidren[loc];
		}
		temp++;
	}
	node->end = 1;
	//node->data = str;
	//printf("Inser a node: %s\n", node->data);
}

void PrintTrie(trietree root, char* str, patternNode* patterns, int index)
{
	//test
	//FILE* fpwrite = fopen("output.txt", "w"); //放进output看下中间过程
	char a = 'a', b = 'b';
	trietree node = root;
	char* temp = str;
	while (*temp != '\0') {
		for (int k = 3; k >= 0; k--) {
			//cahr a = 3; //3的补码为00000011
			int loc = (*temp & 3 << (k * 2)) >> (k * 2);
			//int c = byte_cmp(a, b); //计算比较次数用，懒得改了
			if (node->chidren[loc] == NULL) { //查找失败
				return false;
			}
			node = node->chidren[loc];
		}
		temp++;
	}
	if (node->end != 0) {//存在该单词，即命中了
		(patterns + index)->data = str;
		(patterns + index)->priority = node->end - 1;
		return;
	}
	(patterns + index)->data = str;
	(patterns + index)->priority = 0;
	//printf("%s %d\n", str, node->end - 1);
}


