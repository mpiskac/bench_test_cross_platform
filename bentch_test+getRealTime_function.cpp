
#define HAVE_STRUCT_TIMESPEC
#define _CRT_SECURE_NO_DEPRECATE
#include <iostream> 
#include <ctime> 
#include <time.h> 
#include <pthread.h> 
#include <cstdio>
#include <cstdlib>

#if defined(_WIN32)
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>	
#include <time.h>	
#include <sys/time.h>	

#if defined(__MACH__) && defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#else
#error "nemoguce je destopirati funkciju getRealTime() za nepoznati OS"
#endif

#define BYTE 1
#define KILO 1024
#define MEG 1024*1024
#define BROJ_IT 100000




/**

* Funkcija vraća stvarno vrijeme u sekundama, ili -1,0 ako se pojavila greska

*/
double getRealTime()
{
#if defined(_WIN32)
	FILETIME tm;
	ULONGLONG t;
#if defined(NTDDI_WIN8) && NTDDI_VERSION >= NTDDI_WIN8
	/* Windows 8, Windows Server 2012 + kasnije verzije. ---------------- */
	GetSystemTimePreciseAsFileTime(&tm);
#else
	/* Windows 2000 + kasnije verzije. ---------------------------------- */
	GetSystemTimeAsFileTime(&tm);
#endif
	t = ((ULONGLONG)tm.dwHighDateTime << 32) | (ULONGLONG)tm.dwLowDateTime;
	return (double)t / 10000000.0;

#elif (defined(__hpux) || defined(hpux)) || ((defined(__sun__) || defined(__sun) || defined(sun)) && (defined(__SVR4) || defined(__svr4__)))
	/* HP-UX, Solaris. ------------------------------------------ */
	return (double)gethrtime() / 1000000000.0;

#elif defined(__MACH__) && defined(__APPLE__)
	/* OSX. ----------------------------------------------------- */
	static double timeConvert = 0.0;
	if (timeConvert == 0.0)
	{
		mach_timebase_info_data_t timeBase;
		(void)mach_timebase_info(&timeBase);
		timeConvert = (double)timeBase.numer /
			(double)timeBase.denom /
			1000000000.0;
	}
	return (double)mach_absolute_time() * timeConvert;

#elif defined(_POSIX_VERSION)
	/* POSIX. --------------------------------------------------- */
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
	{
		struct timespec ts;
#if defined(CLOCK_MONOTONIC_PRECISE)
		/* BSD. --------------------------------------------- */
		const clockid_t id = CLOCK_MONOTONIC_PRECISE;
#elif defined(CLOCK_MONOTONIC_RAW)
		/* Linux. ------------------------------------------- */
		const clockid_t id = CLOCK_MONOTONIC_RAW;
#elif defined(CLOCK_HIGHRES)
		/* Solaris. ----------------------------------------- */
		const clockid_t id = CLOCK_HIGHRES;
#elif defined(CLOCK_MONOTONIC)
		/* AIX, BSD, Linux, POSIX, Solaris. ----------------- */
		const clockid_t id = CLOCK_MONOTONIC;
#elif defined(CLOCK_REALTIME)
		/* AIX, BSD, HP-UX, Linux, POSIX. ------------------- */
		const clockid_t id = CLOCK_REALTIME;
#else
		const clockid_t id = (clockid_t)-1;	/* Unknown. */
#endif /* CLOCK_* */
		if (id != (clockid_t)-1 && clock_gettime(id, &ts) != -1)
			return (double)ts.tv_sec +
			(double)ts.tv_nsec / 1000000000.0;

	}
#endif /* _POSIX_TIMERI */

	/* AIX, BSD, Cygwin, HP-UX, Linux, OSX, POSIX, Solaris. ----- */
	struct timeval tm;
	gettimeofday(&tm, NULL);
	return (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
#else
	return -1.0;		/* Greska. */
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Disk
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////																
double start, stop;
double rTime, vrijeme;
char c;

void* disk_test(void *) {

	char *buf = new char[BYTE];

	FILE *f = fopen("datoteka.txt", "w+");

	start = getRealTime();
	for (int i = 0;i<BROJ_IT;i++) {
		fwrite(buf, BYTE, 1, f);
	}
	stop = getRealTime();

	vrijeme = (double)(stop - start) / CLOCKS_PER_SEC;

	std::cout << "\n Sekvencijonalno byte pisanje--> latencija: " << (vrijeme / BROJ_IT) * 1000 << " ns";
	std::cout << "\n Sekvencijonalno byte pisanje--> brzina: " << (double)(((BROJ_IT*BYTE) / (1024 * 1024 * vrijeme)) / 1000) << " MB/s";

	start = getRealTime();
	for (int i = 0;i<BROJ_IT;i++) {
		fread(buf, BYTE, 1, f); //citanje podataka s diska
	}
	stop = getRealTime();

	rTime = (double)((stop - start) * 11000) / CLOCKS_PER_SEC;

	std::cout << "\n Sekvencijonalno byte citanje--> latencija: " << (rTime / BROJ_IT) * 1000 << " ns \n";
	std::cout << "Sekvencijonalno byte citanje--> brzina: " << (double)(BROJ_IT*BYTE / (1024 * 1024 * rTime)) << " MB/s \n";

	start = getRealTime();
	for (int i = 0;i<BROJ_IT;i++) {
		double r = rand() % (BYTE);
		fseek(f, r, SEEK_SET);
		fwrite(buf, BYTE, 1, f);//pisanje podataka na disk
	}
	stop = getRealTime();

	vrijeme = (double)(stop - start) / CLOCKS_PER_SEC;

	std::cout << "\n Random byte pisanje--> latencija: " << ((vrijeme / BROJ_IT) * 1000) << " ns";
	std::cout << "\n Random Byte pisanje --> brzina: " << (double)((BROJ_IT*BYTE / (1024 * 1024 * vrijeme)) / 1000) << " MB/s";

	start = getRealTime();
	for (int i = 0;i<BROJ_IT;i++) {
		double r = rand() % (BYTE);
		fseek(f, r, SEEK_SET);
		fread(buf, BYTE, 1, f);
	}
	stop = getRealTime();

	rTime = (double)(stop - start) / CLOCKS_PER_SEC;

	std::cout << "\n Random byte citanje--> latencija: " << ((rTime / BROJ_IT) * 1000) << " ns";
	std::cout << "\n Random Byte citanje --> brzina: " << (double)((BROJ_IT*BYTE / (1024 * 1024 * rTime)) / 1000) << " MB/s \n\n";

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
	std::cout << "GFLOPS u dretvi # dretva #" << id_dretva << ":   " << (flops / 1e9)/1000 << "\n\n";
}

void gIops(void *threadId) {

	volatile long i;
	volatile int x, y;

	long id_dretva = (long)threadId;
	double p_vrijeme, vrijeme;

	start = getRealTime();

	for (i = 0; i < INT_MAX; i++) {}
	stop = getRealTime();
	double fin = stop - start;
	p_vrijeme = fin / CLOCKS_PER_SEC;

	x = rand();
	y = rand();

	start = getRealTime();
	for (i = 0; i < INT_MAX; i++) {
		x += y;
	}
	stop = getRealTime();
	fin = stop - start;

	vrijeme = fin / CLOCKS_PER_SEC;
	vrijeme -= p_vrijeme;

	std::cout << "Vrijeme prolaska petlje (IOPS): dretva #" << id_dretva << " IOPS:  " << vrijeme << "s  \n\n";
	float iops = INT_MAX / vrijeme;
	std::cout << "GIOPS u dretvi # dretva #" << id_dretva << ":   " << (iops / 1e9)/1000<< "\n\n";
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
		std::cout << "Upisi broj ispred test za pokrenuti test. \n \n 1 --> Disk test \n 2 --> Cpu test \n 3 --> Test funkcije \n ";
		int k;
		std::cin >> k;
		if (k == 1)
		{
			pthread_t *thread = new pthread_t[10];
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

			std::cout << "Test---> pisanje i citanje s diska \n";
			for (int i = 0; i<1; i++) {
				pthread_create(&thread[i], &attr, disk_test, NULL);
			}
			Sleep(5000);
			std::cout << "\n \n \n";
		}
		else if (k == 2)
		{
			std::cout << "\n \n Test---> Cpu test\n";
			std::cout << "\n 2 Dretve \n";

			pthread_t thread[2];

			for (int i = 0; i < 2; i++) {
				pthread_create(&thread[i], NULL, benchmark, (void*)&i);
			}
		}

		else if (k == 3)
		{
			std::cout << "Test--->Test funkcije \n";
			double sT, eT;

			sT = getRealTime();
			long a = 0;
			for (int i = 0; i<INT_MAX;i++)
			{
				a = a + i;
			}

			eT = getRealTime();
			std::cout << "Funkcija:\n long a; \n for (int i = 0; i<INT_MAX;i++) \n { \n a = a + i; \n } \n";

			std::cout << "Stvarno vrijeme izvrsavanja = " << (eT - sT) << "s \n \n \n";
		}

		
		
	}

}
