#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99ûbool
#include<time.h>

//�ֱ���ƽ��������
#define branch 4

int mem = 0;
long long int cmpcount = 0;

void* bupt_malloc(size_t size) {  //size_t:used to represent the size of an object
	if (size <= 0)
		return NULL;
	mem += size;
	return malloc(size);
}

//qsort�õĽڵ���������
typedef struct patternNode { //���ȶ��еĽڵ�
	char* data;
	int priority; //priotity:
}patternNode;
//qsort�ñȽϺ���
int comparator(const void* a, const void* b) //�����践����ֵ��a>b��
{
	int l = ((patternNode*)a)->priority;
	int r = ((patternNode*)b)->priority;
	return r - l; //<0 The element pointed by a goes before the element pointed by b
}


//ac�Զ������ֵ����Ľṹ������Ҫ�����ݳ�Ա��ƽ�����ӣ��Լ����ҷ�֧
typedef struct trienode {
	struct trienode* chidren[branch]; //����Ϊ4����
	struct trienode* fail; //failָ��
	int end; //�Ƿ����սᣬ>0��ʾ�ýڵ�Ϊĳ���ʽ�β����Ӽ���
}trienode, * trietree;
trienode* CreatTrie();
void InsertNode(trietree root, char* str);
void PrintTrie(trietree root, char* str, patternNode* patterns, int index);
//AC�Զ������
void BuildFailPointer(trienode* root); //�ڶ���������ʧЧָ��
void ACSearch(trienode* root); //AC�Զ�������ģʽƥ��

//**�Ļ����Լ�дһ��������ʵ��BFS
//����ڵ�ṹ
typedef struct QNode
{
	trienode* data;   /*������*/
	struct QNode* next;   /*ָ����*/
}QNode, * QueuePtr;
//������нṹ
typedef struct
{
	QueuePtr front;
	QueuePtr rear;
	int size;
}LinkQueue;

LinkQueue* initQueue(trienode* root); //�����г�ʼ��
void pushQueue(LinkQueue* Q, trienode* e); //���
void popQueue(LinkQueue* Q);
bool isEmptyQueue(LinkQueue* Q);
QNode* peekQueue(LinkQueue* Q); //�鿴ͷԪ��





int main()
{
	//��ʱ
	clock_t start_t, finish_t;
	double total_t = 0;
	start_t = clock();

	/*�������̷������֣�
	һ�Ƕ�ȡpattern.txt����һ�������faillָ����ֵ�����������failָ��
	������ȡstring.txt������AC�Զ������ң��������ҽ��д��Trie������
	������������trie�������ȽϽ�������һ����patterns�Ľڵ������д洢
	�ģ��Խڵ�������п��ţ�֮�󵼳���result.txt
	*/

	/*
	һ����ȡpatter.txt�ļ������ֵ���
	*/
	FILE* fp = fopen("../../pattern.txt", "r");
	FILE* fpwrite = fopen("output.txt", "w");

	trietree root = CreatTrie();
	//����
	int check = 0;
	//unsigned int M = 1000;
	unsigned int M = 2256690;
	for (int i = 0; i < M; i++) {
		char str[150];
		fgets(str, 150, fp);
		str[strlen(str) - 1] = '\0';
		int len = strlen(str);

		/*
		��ӽ��ֵ���
		*/
		InsertNode(root, str);

	}
	printf("trie��ռ���ڴ棺%dKB\n", mem  / 1024);
	BuildFailPointer(root); //����failָ��
	

	fclose(fp);
	fclose(fpwrite);
	printf("STEP1:��ȡpattern.txt����һ�������faillָ����ֵ�����������failָ�� OVER\n");
	//��ʱ
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//��ʱ��ת��Ϊ��
	printf("CPU ռ�õ���ʱ�䣺%f\n", total_t);
	start_t = clock();

	/*
	������ȡstring.txt������AC�Զ������ң��������ҽ��д��Trie�����ȣ�endֵ���ڼ�����
	*/
	ACSearch(root);
	printf("STEP2:��ȡstring.txt������AC�Զ������ң��������ҽ��д��Trie������ OVER\n");
	//��ʱ
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//��ʱ��ת��Ϊ��
	printf("CPU ռ�õ���ʱ�䣺%f\n", total_t);
	start_t = clock();

	/*
	��������trie�������ȽϽ�������һ����patterns�Ľڵ������д洢
	*/
	fp = fopen("../../pattern.txt", "r");
	//fpwrite = fopen("output.txt", "w");
	patternNode* patterns = (patternNode*)malloc(sizeof(patternNode) * M);
	for (int i = 0; i < M; i++) {
		//char tem[150]; //���·���һ��char��������Ȼpatterns��Ԫ�ػ�ָ��ͬһ��char
		char* tem = (char*)malloc(sizeof(char) * 150);
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		//int len = strlen(tem);

		//�����ֵ���������ƥ��
		PrintTrie(root, tem, patterns, i);
		//free(tem);�Ž�node���ˣ�����free
	}

	fclose(fp);
	printf("STEP3:����trie�������ȽϽ�������һ����patterns�Ľڵ������д洢 OVER\n");
	//��ʱ
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//��ʱ��ת��Ϊ��
	printf("CPU ռ�õ���ʱ�䣺%f\n", total_t);
	start_t = clock();

	/*
	�ģ��Խڵ�������п��ţ�֮�󵼳���result.txt
	*/
	fpwrite = fopen("result.txt", "w");

	//220�򼶱������̫���ˣ�ʹ���Դ�����qsort()
	//Ҫʹ��qsort(),���ȵö���һ��comparator����

	
	
	//��patterns��������
	/*void qsort (void* base, size_t num, size_t size,  
            int (*comparator)(const void*,const void*));*/
	qsort(patterns, M, sizeof(patternNode), comparator);


	//���д��result.txt
	//patternNode* temPattern = patterns;
	for (int i = 0; i < M; i++) {
		//if (i % 100000 == 0)
		//	printf("%d\n", i);
		fprintf(fpwrite, "%s %d\n", (patterns + i)->data, (patterns + i)->priority);
		//temPattern++;
	}

	//д�����һ��
	fprintf(fpwrite, "%lld %d\n", cmpcount, mem / 1024); //���һ��������������ֽڱȽϴ���K �ڴ濪����KB��
	//printf("ͳ�ƽ��word count; kb��%d %d\n", cmpcount, (mem * 8) / 1024);

	fclose(fpwrite);
	printf("STEP4:��output�ļ���ȡ����������֮�󵼳���result.txt OVER\n");

	//��ʱ
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//��ʱ��ת��Ϊ��

	printf("CPU ռ�õ���ʱ�䣺%f\n", total_t);

	return 0;

}


//AC�Զ�������
void BuildFailPointer(trienode* root)
{
	root->fail = root; //�ж��ж�

	LinkQueue* Q = initQueue(root);
	while (!isEmptyQueue(Q)) {
		trienode* parent = peekQueue(Q)->data;
		popQueue(Q);
		for (int i = 0; i < branch; i++) { //��ÿһ���ӽڵ�����FailedPointer
			if (parent->chidren[i]) {
				trienode* child = parent->chidren[i];
				pushQueue(Q, child);
				trienode* candidate = parent->fail; //��father->FailedPointer��ʼ���ߵ�ָ��
				while (1) {
					if (parent == root) {
						candidate = root;
						break;
					}
					if (candidate->chidren[i]) {//����child��ͬ���ӽڵ�
						candidate = candidate->chidren[i];
						break;
					}
					else {
						if (candidate == root)
							break;
						candidate = candidate->fail;//��������˳���ܽ�������Ϊ��root�Կ�����һ��ƥ��
					}
				}
				child->fail = candidate;
			}
		}
		
	}
}

void ACSearch(trienode* root) //�����Ǵ��ļ�
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
				//cahr a = 3; //3�Ĳ���Ϊ00000011
				int loc = (c & 3 << (k * 2)) >> (k * 2);
				cmpcount++; //ͳ�ƱȽϴ���
				if (p->chidren[loc]) { //���ҳɹ�
					trienode* temp = p->chidren[loc]->fail;
					while (temp != root && temp != NULL) {//��ƥ�������£���Ȼ��FailedPointer�������ɼ���������ģʽ��
						if (temp->end != 0) {//�ýڵ��ǵ���β�ڵ㡣
							//fprintf(fpcount, "%s %d\n", p->data, 1);
							p->end++;
						}
						temp = temp->fail;
					}

					p = p->chidren[loc];
					if (p->end != 0) { //�ýڵ��ǵ���β�ڵ㡣
						//fprintf(fpcount, "%s %d\n", p->data, 1);
						p->end++;
					}
					k--;
				}
				else { //ʧ�䣬��ת
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

//������غ���
LinkQueue* initQueue(trienode* root) //�����г�ʼ��
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
void pushQueue(LinkQueue* Q, trienode* e) //���
{
	if (isEmptyQueue(Q)) {
		//printf("push the first element in queue\n");
		//Q = initQueue(e); ���뺯���Ĳ��������޸ģ�����һ��Q�ĵ�ַ�����ǲ��е�
		QNode* temp = (QNode*)bupt_malloc(sizeof(QNode));
		temp->data = e;
		temp->next = NULL;
		Q->front = Q->rear = temp;
		Q->size = 1;
		return;
	}
	QueuePtr p = (QueuePtr)bupt_malloc(sizeof(QNode));
	//if (!p) exit(OVERFLOW);
	//��������
	p->data = e;
	p->next = NULL;
	//Q.rearһֱָ���β
	Q->rear->next = p;
	Q->rear = p;
	Q->size++;
}
void popQueue(LinkQueue* Q)
{
	if (isEmptyQueue(Q)) printf("ERROR");
	QueuePtr p = Q->front;
	Q->front = p->next;   //��ͷԪ��p����
	if (Q->rear == p)   //�������ֻ��һ��Ԫ��p, ��p���Ӻ��Ϊ�ն�
		Q->rear = Q->front;     //����βָ�븳ֵ
	free(p);   //�ͷŴ洢�ռ�
	Q->size--;
}
bool isEmptyQueue(LinkQueue* Q)
{
	return (Q->front == NULL);
}
QNode* peekQueue(LinkQueue* Q) //�鿴ͷԪ��
{
	return Q->front;
}

//ac�Զ�����trie�����
trienode* CreatTrie() {
	trienode* node = (trienode*)bupt_malloc(sizeof(trienode));
	memset(node, 0, sizeof(trienode));
	node->fail = NULL;
	//for (int i = 0; i < branch; i++) {
	//	node->chidren[i] = NULL;
	//}
	//��C�����У�NULL��ʵ�ʵײ�����о���0,����Ϊ��Ŀ�ĺ���;������ʶ���ԭ��NULL����ָ��Ͷ���0������ֵ
	//node->chidren = (trienode*)bupt_malloc(sizeof(trienode) * branch); ��children��������
	return node;
}

void InsertNode(trietree root, char* str)
{
	if (*str == '\0')
		return;
	char* temp = str;
	//һ��cha��8bit��4��֧��2bit��ʾ������8/2=4��������һ��cahr
	trienode* node = root;
	while (*temp != '\0') {
		for (int k = 3; k >= 0; k--) {
			//cahr a = 3; //3�Ĳ���Ϊ00000011
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
	//FILE* fpwrite = fopen("output.txt", "w"); //�Ž�output�����м����
	char a = 'a', b = 'b';
	trietree node = root;
	char* temp = str;
	while (*temp != '\0') {
		for (int k = 3; k >= 0; k--) {
			//cahr a = 3; //3�Ĳ���Ϊ00000011
			int loc = (*temp & 3 << (k * 2)) >> (k * 2);
			//int c = byte_cmp(a, b); //����Ƚϴ����ã����ø���
			if (node->chidren[loc] == NULL) { //����ʧ��
				return false;
			}
			node = node->chidren[loc];
		}
		temp++;
	}
	if (node->end != 0) {//���ڸõ��ʣ���������
		(patterns + index)->data = str;
		(patterns + index)->priority = node->end - 1;
		return;
	}
	(patterns + index)->data = str;
	(patterns + index)->priority = 0;
	//printf("%s %d\n", str, node->end - 1);
}


