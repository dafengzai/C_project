#include<stdio.h>
#include<string.h>
#include<malloc.h>

int mem = 0;
int cmpnum = 0;

void *bupt_malloc(size_t size) {  //size_t:used to represent the size of an object
	if (size <= 0)
		return NULL;
	mem += size;
	return malloc(size);
}

int byte_cmp(char a, char b)
{
	cmpnum++;
	return a - b;
}

struct linklist {
	unsigned int val;
	struct linklist *next;
};

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

void creatchain(struct linklist *chain, int len) //��ʼ������
{
	for (int i = 0; i < len; i++) {
		chain[i].val = i;
		chain[i].next = NULL;
		bupt_malloc(sizeof(chain[i]));
	}
}

void addchain(struct linklist *chain, int loc, unsigned int hashnum) //�������в���Ԫ��
{
	struct linklist *p = chain+loc;
	while (p->next != NULL) { //����������β
		p = p->next;
	}
	struct linklist *q= (struct linklist*)malloc(sizeof(struct linklist)); //���뿪���ڴ�ռ䣬��ֹ�����������ջ��ڴ�ռ�
	q->val = hashnum;
	q->next = NULL;
	bupt_malloc(sizeof(*q));
	p->next = q;
}

int findchain(struct linklist *chain, int loc, unsigned int hashnum) //����hashnum�Ƿ���������
{
	struct linklist *p = (chain + loc)->next; //p��Ϊ��һ��Ԫ��
	int flag = 0;//Ĭ��ƥ��ʧ��
	char a = 'a', b = 'b';
	while (p != NULL) {
		int c = byte_cmp(a, b);
		if (p->val == hashnum) {
			flag = 1;
			break;
		}
		p = p->next;
	}
	return flag;
}

int main()
{
	//bkdr
	unsigned int N = 1270688; //Ѫ����Ľ�ѵ
	unsigned int M = 986004;
	FILE *fp = fopen("patterns-127w.txt", "r");
	int primenum = 122777;
	//����ģһ���������ķ�ʽʹ��������
	struct linklist *ChainHash = (struct linklist*)malloc(primenum * sizeof(struct linklist));
	//��ʼ��������Ĭ�ϵ�struct��.next����NULL
	creatchain(ChainHash, primenum);

	
	for (int i = 0; i < N; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		//��ȡ�ַ�������ʹ��bkdr��ϣ��������ת����һ������
		unsigned int bkdrhash = BKDRHash(tem, len);
		int loc = bkdrhash%primenum; //loc��ΧΪ��0-primenum-1
		//ʹ��������
		addchain(ChainHash, loc, bkdrhash);
		//if (i % 10000==0)
			//printf("checking on num %d\n", i);
	}
	fclose(fp);
	int memsize = (mem * 8) / 1024;//sizeof()���ֽ�Ϊ��λ��1byte=8bit
	
	//��ʼƥ��
	fp = fopen("words-98w.txt", "r");
	FILE *fpwrite = fopen("result.txt", "w");
	int yescount = 0;
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';;
		int len = strlen(tem);
		//��ȡ�ַ�������ʹ��bkdr��ϣ��������ת����һ������
		unsigned int bkdrhash = BKDRHash(tem, len);
		int loc = bkdrhash%primenum; //loc��ΧΪ��0-primenum-1
		//ʹ��������������ƥ��
		int flag = findchain(ChainHash, loc, bkdrhash); //1Ϊƥ��ɹ���0��ʾƥ��ʧ��
		if (flag == 1) { //ƥ��ɹ�
			fprintf(fpwrite, "%s %s", tem, "yes\n");
			yescount++;
		}
		else {
			fprintf(fpwrite, "%s %s", tem, "no\n");
		}
	}

	//д�����һ������

	fprintf(fpwrite, "%d %d %d %d", memsize, cmpnum, M, yescount);
	fclose(fpwrite);

	fclose(fp);
	fclose(fpwrite);

	
	//�ͷ��ڴ棨��������ʵ��freeҲ�У������������Զ��ѷ�����ڴ���ա�
	free(ChainHash);
	return 0;
}