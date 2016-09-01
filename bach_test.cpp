
#define HAVE_STRUCT_TIMESPEC
#define _CRT_SECURE_NO_DEPRECATE
#include <iostream> 
#include <ctime> 
#include <time.h> 
#include <pthread.h> 
#include <cstdio>
#include <cstdlib>
#include <limits.h>


#ifdef _WIN32
#define SEC 1000
#define PRO 1

#endif

#ifdef linux
#define SEC 1000000
#define PRO 1000
#endif




#define BYTE 1
#define KILO 1024
#define MEG 1024*1024
#define BROJ_IT 100000






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Disk
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////																

double start, stop;
double rTime, vrijeme;
char c;

void* disk_test(void *) {

	char *buf = new char[BYTE];

	FILE *f = fopen("datoteka.txt", "w+");

	start = clock();
	for (int i = 0;i<BROJ_IT;i++) {
		fwrite(buf, BYTE, 1, f);
	}
	stop = clock();

	vrijeme = (double)(stop - start) / CLOCKS_PER_SEC;

	std::cout << "\n Sekvencijonalno byte pisanje--> latencija: " << (vrijeme / BROJ_IT) * 1000 << " ms";
	std::cout << "\n Sekvencijonalno byte pisanje--> brzina: " << (double)(((BROJ_IT*BYTE) / (1024 * 1024 * vrijeme))) << " MB/s";

	start = clock();
	for (int i = 0;i<BROJ_IT;i++) {
		fread(buf, BYTE, 1, f); //citanje podataka
	}
	stop = clock();

	double rTime = (double)(stop - start) / CLOCKS_PER_SEC;

	std::cout << "\n Sekvencijonalno byte citanje--> latencija: " << (rTime / BROJ_IT) * 1000 << " msec";
	std::cout << "\n Sekvencijonalno byte citanje--> brzina: " << (double)(BROJ_IT*BYTE / (1024 * 1024 * rTime)) << " MB/s \n";

	start = clock();
	for (int i = 0;i<BROJ_IT;i++) {
		double r = rand() % (BYTE);
		fseek(f, r, SEEK_SET);
		fwrite(buf, BYTE, 1, f);//pisanje podataka na disk
	}
	stop = clock();

	vrijeme = (double)(stop - start) / CLOCKS_PER_SEC;

	std::cout << "\n Random byte pisanje--> latencija: " << ((vrijeme / BROJ_IT) * 1000) << " ms";
	std::cout << "\n Random Byte pisanje --> brzina: " << (double)((BROJ_IT*BYTE / (1024 * 1024 * vrijeme))) << " MB/s";

	start = clock();
	for (int i = 0;i<BROJ_IT;i++) {
		double r = rand() % (BYTE);
		fseek(f, r, SEEK_SET);
		fread(buf, BYTE, 1, f);
	}
	stop = clock();

	rTime = (double)(stop - start) / CLOCKS_PER_SEC;

	std::cout << "\n Random byte citanje--> latencija: " << ((rTime / BROJ_IT)) * 1000 << " ms";
	std::cout << "\n Random Byte citanje --> brzina: " << (double)((BROJ_IT*BYTE / (1024 * 1024 * rTime))) << " MB/s \n\n";

	fclose(f);
	pthread_exit(NULL);
	return NULL;


}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Cpu
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


float getRandomVal(float pom) {
	return static_cast<float>(rand() / (static_cast<float>(RAND_MAX / pom)));
}

void flops(void *threadId) {

	volatile long i;
	volatile float x, y;

	long id_dretva = (long)threadId;
	float pom = 1e6;

	double p_vrijeme, vrijeme;

	start = clock();

	for (i = 0; i < INT_MAX; i++) {}
	stop = clock();
	double fin = stop - start;

	p_vrijeme = fin / CLOCKS_PER_SEC;

	x = getRandomVal(pom);
	y = getRandomVal(pom);

	start = clock();

	for (i = 0; i < INT_MAX; i++) {
		x += y;
	}
	stop = clock();
	fin = stop - start;

	vrijeme = fin / CLOCKS_PER_SEC;
	vrijeme -= p_vrijeme;

	std::cout << "Vrijeme prolaska petlje (FLOPS): dretva #" << id_dretva << ":  " << vrijeme << "s \n\n";

	float flops = INT_MAX / vrijeme;
	std::cout << "GFLOPS u dretvi # dretva #" << id_dretva << ":   " << flops / 1e9 << "\n\n";
}

void gIops(void *threadId) {

	volatile long i;
	volatile int x, y;

	long id_dretva = (long)threadId;
	double p_vrijeme, vrijeme;

	start = clock();

	for (i = 0; i < INT_MAX; i++) {}
	stop = clock();
	double fin = stop - start;
	p_vrijeme = fin / CLOCKS_PER_SEC;

	x = rand();
	y = rand();

	start = clock();
	for (i = 0; i < INT_MAX; i++) {
		x += y;
	}
	stop = clock();
	fin = stop - start;

	vrijeme = fin / CLOCKS_PER_SEC;
	vrijeme -= p_vrijeme;

	std::cout << "Vrijeme prolaska petlje (IOPS): dretva #" << id_dretva << " IOPS:  " << vrijeme << "s  \n\n";
	float iops = INT_MAX / vrijeme;
	std::cout << "GIOPS u dretvi # dretva #" << id_dretva << ":   " << iops / 1e9 << "\n\n";
}

void *benchmark(void *threadId) {
	flops(threadId);
	gIops(threadId);
	pthread_exit(NULL);
	return NULL;
}


int main() {
	int k = 0;
	while (k <= 3)
	{
		std::cout << "Upisi broj ispred test za pokrenuti test. \n \n 1 --> Disk test \n 2 --> Cpu test \n 3 --> Test funkcije \n";
		int k;
		std::cin >> k;
		if (k == 1)
		{
			pthread_t *thread = new pthread_t[10];
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

			std::cout << "\n \n Test---> pisanje i citanje s diska \n";
			for (int i = 0; i<1; i++) {
				pthread_create(&thread[i], &attr, disk_test, NULL);
			}

			std::cout << "\n \n \n";
		}
		else if (k == 2)
		{
			std::cout << "\n \n Test---> Cpu test\n";
			std::cout << "\n 2 Dretve \n";

			pthread_t thread[1];

			for (int i = 0; i < 1; i++) {
				pthread_create(&thread[i], NULL, benchmark, (void*)&i);
			}


		}

		else if (k == 3)
		{
			std::cout << "\n \n Test--->Test funkcije \n";
			double sT, eT;

			sT = clock();
			long a = 0;


			for (int i = 0; i < INT_MAX; i++)
			{
				a = a + i;
			}

			eT = clock();

			std::cout << "Funkcija:\n long a; \n for (int i = 0; i<INT_MAX;i++) \n { \n a = a + i; \n } \n";

			std::cout << "Stvarno vrijeme izvrsavanja = " << (eT - sT) / SEC << "s \n \n \n";

		}


	}

}