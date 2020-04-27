#include "finders.h"
#include "generator.h"
#include "math.h"
#include <time.h>

struct compactinfo_t
{
    int64_t seedStart, seedEnd;
    unsigned int range;
    BiomeFilter filter;
    int withHut, withMonument;
    int minscale;
};

long count = 0;
long last_count = 0;
float sps = 0;
time_t start_time;

#ifdef USE_PTHREAD
static void *searchCompactBiomesThread(void *data)
#else
static DWORD WINAPI searchCompactBiomesThread(LPVOID data)
#endif
{
    struct compactinfo_t info = *(struct compactinfo_t *)data;
    int ax = -info.range, az = -info.range;
    int w = 2*info.range, h = 2*info.range;
    int64_t s;

    LayerStack g = setupGenerator(MC_1_15);
    int *cache = allocCache(&g.layers[L_VORONOI_ZOOM_1], w, h);

	int ice = 140,  bamboo = 168,  desert = 2,  plains = 1,  ocean = 0,  jungle = 21,  forest = 4,  mushroom = 14, mesa = 37, flower = 132;
	float step = 8;
	float max_ocean = 25;

    for (s = info.seedStart; s != info.seedEnd; s++)
    {
		sps = ++count / (time (NULL) - start_time);
		printf("\r%li seeds scanned | %.0lf seeds per second", count, sps);
		fflush(stdout);
		if (!checkForBiomes(&g, cache, s, ax, az, w, h, info.filter, info.minscale))
			goto nope;
		applySeed(&g, s);
		int x, z;
		int r = info.range;
		Pos goodhuts[2];
		if (info.withHut)
		{
			Pos huts[10000];
			int counter = 0;
			int r = info.range / SWAMP_HUT_CONFIG.regionSize;
			for (z = -r; z < r; z++)
			{
				for (x = -r; x < r; x++)
				{
					Pos p;
					p = getStructurePos(SWAMP_HUT_CONFIG, s, x, z);
					if (isViableFeaturePos(Swamp_Hut, g, cache, p.x, p.z)) {
						if (abs(p.x) < info.range && abs(p.z) < info.range) {
							huts[counter] = p;
							counter++;
							//printf("%i\n", huts[0].x);
						}
					}
				}
			}
			
			for (int i = 0; i < counter; i++) {
				for (int j = 0; j < counter; j++) {
					if (j == i)
						continue;
					float dx, dz;
					dx = abs(huts[i].x - huts[j].x);
					dz = abs(huts[i].z - huts[j].z);
					if (sqrt((dx*dx)+(dz*dz)) <= 200) {
						//printf("%i\n", counter);
						//printf("%f\n",(sqrt((dx*dx)+(dz*dz))));
						//printf("%i, %i, %i, %i\n",huts[i].x,huts[i].z,huts[j].x,huts[j].z);
						goodhuts[0] = huts[i];
						goodhuts[1] = huts[j];
						goto L_hut_found;
					}
				}
			}
			
			goto nope;
			L_hut_found:;
		}
		if (info.withMonument)
		{
			int r = info.range / MONUMENT_CONFIG.regionSize;
			for (z = -r; z < r; z++)
			{
				for (x = -r; x < r; x++)
				{
					Pos p;
					p = getLargeStructurePos(MONUMENT_CONFIG, s, x, z);
					if (isViableOceanMonumentPos(g, cache, p.x, p.z))
						if (abs(p.x) < info.range && abs(p.z) < info.range)
							goto L_monument_found;
				}
			}
			goto nope;
			L_monument_found:;
		}
			
		float ocean_count = 0;
		int biomes[10] = {ice, bamboo, desert, plains, ocean, jungle, forest, mushroom, mesa, flower};
		for (z = -r; z < r; z+=step)
		{
			for (x = -r; x < r; x+=step)
			{
				Pos p = {x, z};
				int biome = getBiomeAtPos(g, p);
				if (isOceanic(biome))
					ocean_count++;
				for (int i = 0; i < 10; i++) {
					if (biome == biomes[i]) {
						biomes[i] = -1;
					}
				}
			}
		}
		float ocean_percent = (ocean_count * (step * step) / (w * h)) * 100;
		if (ocean_percent > max_ocean)
			goto nope;
		for (int i = 0; i < 10; i++) {
			if (biomes[i] != -1)
				goto nope;
		}
		printf("\rFound: %ld | huts at: %i,%i & %i,%i | ocean: %.2lf%% %lf\n", s, goodhuts[0].x, goodhuts[0].z, goodhuts[1].x, goodhuts[1].z, ocean_percent, ocean_count);
		fflush(stdout);
		
		nope:;
    }

    freeGenerator(g);
    free(cache);

#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}


int main(int argc, char *argv[])
{
	start_time = time (NULL) - 1;
    initBiomes();

    int64_t seedStart, seedEnd;
    unsigned int threads, t, range;
    BiomeFilter filter;
    int withHut, withMonument;
    int minscale;

    // arguments
    if (argc <= 0)
    {
        printf( "find_compactbiomes [seed_start] [seed_end] [threads] [range]\n"
                "\n"
                "  seed_start    starting seed for search [long, default=0]\n"
                "  end_start     end seed for search [long, default=-1]\n"
                "  threads       number of threads to use [uint, default=1]\n"
                "  range         search range (in blocks) [uint, default=1024]\n");
        exit(1);
    }
    if (argc <= 1 || sscanf(argv[1], "%" PRId64, &seedStart) != 1) seedStart = 0;
    if (argc <= 2 || sscanf(argv[2], "%" PRId64, &seedEnd) != 1) seedEnd = -1;
    if (argc <= 3 || sscanf(argv[3], "%u", &threads) != 1) threads = 1;
    if (argc <= 4 || sscanf(argv[4], "%u", &range) != 1) range = 1024;


	int ice = 140,  bamboo = 168,  desert = 2,  plains = 1,  ocean = 0,  jungle = 21,  forest = 4,  mushroom = 14, mesa = 37, flower = 132;

	int biomes[] = {desert, plains, ocean, jungle, forest, mushroom, mesa, flower};
    // TODO: set up a customisable biome filter
    filter = setupBiomeFilter(biomes,
                sizeof(biomes)/sizeof(int));
    minscale = 1; // terminate search at this layer scale
    // TODO: simple structure filter
    withHut = 1;
    withMonument = 1;

    printf("Starting search through seeds %" PRId64 " to %" PRId64", using %u threads.\n"
           "Search radius = %u.\n", seedStart, seedEnd, threads, range);

    thread_id_t threadID[threads];
    struct compactinfo_t info[threads];

    // store thread information
    uint64_t seedCnt = ((uint64_t)seedEnd - (uint64_t)seedStart) / threads;
    for (t = 0; t < threads; t++)
    {
        info[t].seedStart = (int64_t)(seedStart + seedCnt * t);
        info[t].seedEnd = (int64_t)(seedStart + seedCnt * (t+1));
        info[t].range = range;
        info[t].filter = filter;
        info[t].withHut = withHut;
        info[t].withMonument = withMonument;
        info[t].minscale = minscale;
    }
    info[threads-1].seedEnd = seedEnd;

    // start threads
#ifdef USE_PTHREAD

    for (t = 0; t < threads; t++)
    {
        pthread_create(&threadID[t], NULL, searchCompactBiomesThread, (void*)&info[t]);
    }

    for (t = 0; t < threads; t++)
    {
        pthread_join(threadID[t], NULL);
    }

#else

    for (t = 0; t < threads; t++)
    {
        threadID[t] = CreateThread(NULL, 0, searchCompactBiomesThread, (LPVOID)&info[t], 0, NULL);
    }

    WaitForMultipleObjects(threads, threadID, TRUE, INFINITE);

#endif

    return 0;
}
