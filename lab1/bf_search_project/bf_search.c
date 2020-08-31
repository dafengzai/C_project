#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>

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

unsigned int MurmurHash2(const void * key, int len, unsigned int seed) //���ã�ûë��
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value

	unsigned int h = seed ^ len;

	// Mix 4 bytes at a time into the hash

	const unsigned char * data = (const unsigned char *)key;

	while (len >= 4)
	{
		unsigned int k = *(unsigned int *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array

	switch (len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

//���б��ز����õĺ���
void SetArrayBit(char *bf, unsigned int loc) {
	char tem = 1 << (7 - loc % 8);
	bf[loc / 8] = bf[loc / 8] | tem; //�����
}
int GetArrayBit(char *bf, unsigned int loc) {  //����1Ϊƥ��ɹ�
	char tem = bf[loc / 8];
	int a= (tem & 1 << (7 - loc % 8)) >> (7 - loc % 8); //����1��������bf��Ԫ�ؽ����������1���ƺ�����λΪ0�������Ӱ�죬�����ƻ�ȥĩβΪ1�ͷ���1��ĩβΪ0����0
	//printf("%d\n", a);
	return a;
}

int main()
{
	unsigned int N = 1270688; //Ѫ����Ľ�ѵ
	unsigned int M = 986004;
	int k = 0; //hash�����ĸ���
	long int m = 0;//�迪�ٵĴ洢�ռ����
	//��ע����false positive rate f:һ�����ڼ���N�е��ַ���k��hash���Ӧλ��Ϊ1�ĸ���
	//�������q:������ĳһλΪ0�ĸ���Ϊ0.5��������m��һ��Ԫ��Ϊ��ʱf��С
	//���幫ʽΪ������f��,����Կɼ����
	
	double f = 0.000001; //�趨������
	m = N*1.44*(log(1/f) / log(2));  //m=3.6kw
	k = 0.693*(m / N); //k=19
	//printf("%d\n", m);
	//printf("%d\n", k);

	//����bf�õ����飬��char����ʾһ��8bit���ж�
	int size = m / 8;
	//int size = m; //��ʵmû����INT_MAX
	char *bfarray = (char *)malloc((size) * sizeof(char)); //charռһ���ֽڣ�intռ4��
	mem = (size) * sizeof(char);
	//��ʼ��bfarray
	for (int i = 0; i < size; i++) {
		bfarray[i] = '0';
	}
	
	//����k��murmur hash�����õ�seed
	int *seed = (int *)malloc(k * sizeof(int));
	for (int i = 0; i < k; i++)
	{
		//seed[i] = 66 * (i + 1);
		//seed[i] = pow(13, i + 1) % 10000;
		seed[i] = (13 ^ (i + 1)) % 10000;
	}

	FILE *fp = fopen("patterns-127w.txt", "r");
	//���ļ����ַ�����hash����bfarray��


	
	for (int i = 0; i < N; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		for (int j = 0; j < k; j++) {
			//��ȡ�ַ�������ʹ��murmur��ϣ��������ת����һ������
			unsigned int murmurindex = MurmurHash2(tem, len, seed[j]);
			murmurindex = murmurindex%m;
			SetArrayBit(bfarray, murmurindex);
			//bfarray[murmurindex] = '1';
		}
	}
	fclose(fp);
	unsigned int memsize = (mem * 8) / 1024;//sizeof()���ֽ�Ϊ��λ��1byte=8bit
	
	//��ʼƥ��
	fp = fopen("words-98w.txt", "r");
	FILE *fpwrite = fopen("result.txt", "w");
	int yescount = 0;
	char a = 'a', b = 'b';
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';;
		int len = strlen(tem);
		//��ʼ����bloom filter
		int flag = 1;//Ĭ�ϳɹ�
		for (int j = 0; j < k; j++) {
			//��ȡ�ַ�������ʹ��murmur��ϣ��������ת����һ������
			unsigned int murmurindex = MurmurHash2(tem, len, seed[j]);
			murmurindex = murmurindex%m;
			//�ж�
			int c = byte_cmp(a, b); //����Ƚϴ����ã����ø���
			//printf("%d\n", cmpnum);
			int result = GetArrayBit(bfarray, murmurindex); //����1��ʾƥ��ɹ�
			//char result = bfarray[murmurindex];
			if (result != 1) { //ƥ��ʧ��
				flag = 0;
				break;
			}
		}
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
	//free(bfarray); ��bug����Ҳ��֪��Ϊɶ
	//free(seed);
	return 0;

}