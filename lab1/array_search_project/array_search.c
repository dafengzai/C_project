#include<stdio.h>
#include<string.h>
#include<malloc.h>

int mem = 0;
int cmpnum = 0;
//��ʽת�� GB2312
//�ڴ������Կ���һ����ά�����Σ�ÿһ��stringʹ��malloc(size_of(s.length))���������Ԫ��ʱʹ��relloc����

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

int main()
{
	unsigned int N = 1270688; //Ѫ����Ľ�ѵ
	FILE *fp = fopen("patterns-127w.txt", "r");

	//ע�⣬ʹ�ö�άָ��char **��ָ��ָ�����һ��ָ�룬�ٸ�ָ���ָ�뿪�ٿռ���ַ���
	char **p=NULL; //��ַ�ĵ�ַ
	p = (char **)malloc(N * sizeof(char*)); //����άָ�룬����p+1ֱ�ӾͶ���һ���ַ���  
	//printf("%s\n", p[0]); //������printf�������ʱ��ֻҪ���ӡ��ʽ��%s��ϵͳ֪������Ҫ����ַ���������ϵͳҲ����ȷ���ַ����ĳ��ȣ����Ե�����%s��ӡ�ַ�ָ��p��ʱ��ͻ�����ַ���

	//��ʼ��ȡ����
	int *strslen1 = (int *)malloc(N * sizeof(int));
	for (int i = 0; i < N;i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		strslen1[i] = len; //���ڼ�¼���ַ�������
		//׷���ڴ�ռ�
		p[i] = (char *)malloc(len * sizeof(char));
		//printf("%d\n", sizeof(tem));
		strcpy(p[i], tem);
		bupt_malloc(sizeof(p[i]));
		

		//if (i == 1000)
		//	break;
	}

	fclose(fp);

	//��ȡ�ȶ�������
	unsigned int M = 986004;
	//int strslen[M];
	int *strslen2 = (int *)malloc(M * sizeof(int));
	fp = fopen("words-98w.txt", "r");
	char **q = NULL;
	q = (char **)malloc(M * sizeof(char*));
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		strslen2[i] = len; //���ڼ�¼���ַ�������
		//׷���ڴ�ռ�
		q[i] = (char *)malloc(len * sizeof(char));
		strcpy(q[i], tem);
		bupt_malloc(sizeof(q[i]));

		//char *p1 = (char *)malloc(sizeof(tem));
		//if (i == 1000)
		//	break;
	}
	fclose(fp);
	//for (int i = 0; i < 1000; i++)
	//	printf("%s\n", q[i]);

	//printf("%c\n", q[1][1]); //�Ǹ��ʺţ����ı��벻һ��
	//
	FILE *fpwrite = fopen("result.txt", "w");

	int memsize = (mem * 8) / 1024;//sizeof()���ֽ�Ϊ��λ��1byte=8bit

	

	//��ʼ�Ƚ�
	//ά��һ��string��������Ž��
	//char *results = (char *)malloc(M * sizeof(char));
	int yescount = 0;
	for (int i = 0; i < M; i++) {

		int flag = 1;
		int border = strslen2[i];
		for (int j = 0; j < N; j++) {
			//��ʼ�ȶ������ַ���
			int judge = 0;
			int k = 0;
			//printf("%d\n", strslen1[j]);
			int plen = strslen1[j];
			if (plen !=  border) //��һ�������϶���
				continue;
			for (int k = 0; k < border; k++) {
				int a = byte_cmp(q[i][k], p[j][k]);
				if (a != 0) { //�ȶ�ʧ��
					judge = 1;
					break;
				}
			}
			if (judge == 0) { //�ȶԳɹ�
				flag = 0;
				break;
			}
		}
		if (flag == 0) { //�ȶԳɹ�
			//fprintf(fpwrite, "%s", "yes\n");
			fprintf(fpwrite, "%s %s", q[i], "yes\n");
			yescount++;
		}
		else {
			//fprintf(fpwrite, "%s", "no\n");
			fprintf(fpwrite, "%s %s", q[i], "no\n");
		}

		if (i % 10000 == 0)
			printf("checking on num %d\n", i);
	}
	//д������
	fprintf(fpwrite, "%d %d %d %d", memsize, cmpnum, M, yescount);
	fclose(fpwrite);
	//fclose(fp);
	//�ͷ��ڴ�
	free(p);
	free(q);
	free(strslen1);
	free(strslen2);
	return 0;
}