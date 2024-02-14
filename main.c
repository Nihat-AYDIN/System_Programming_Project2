/**
 * @file main.c 
 * @author adaskin
 * @brief this simulates the movement of a hunter and animal game in 2D site grid
 * @version 0.1
 * @date 2023-05-03
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t m1;

typedef enum { BEAR, BIRD, PANDA} AnimalType;

typedef enum { ALIVE, DEAD } AnimalStatus;

typedef struct {
    int x;
    int y;
} Location;

typedef enum { FEEDING, NESTING, WINTERING } SiteType;

typedef struct {
    /** animal can be DEAD or ALIVE*/
    AnimalStatus status;
    /** animal type, bear, bird, panda*/
    AnimalType type;
    /** its location in 2D site grid*/
    Location location;
} Animal;

/*example usage*/
Animal bird, bear, panda;

/** type of Hunter*/
typedef struct {
    /** points indicate the number of animals, a hunter killed*/
    int points;
    /** its location in the site grid*/
    Location location;
} Hunter;

/** type of a site (a cell in the grid)*/
typedef struct {
    /** array of pointers to the hunters located at this site*/
    Hunter **hunters;
    /** the number of hunters at this site*/
    int nhunters;
    /** array of pointers to the animals located at this site*/
    Animal **animals;
    /** the number of animals at this site*/
    int nanimals;
    /** the type of site*/
    SiteType type;
} Site;

/** 2D site grid*/
typedef struct {
    /** number of rows, length at the x-coordinate*/
    int xlength;
    /** number of columns, length at the y-coordinate*/
    int ylength;
    /** the 2d site array*/
    Site **sites;
} Grid;

/* initial grid, empty*/
Grid grid = {0, 0, NULL};

/**
 * @brief initialize grid with random site types
 * @param xlength
 * @param ylength
 * @return Grid
 */
Grid initgrid(int xlength, int ylength) {
    grid.xlength = xlength;
    grid.ylength = ylength;

    grid.sites = (Site **)malloc(sizeof(Site *) * xlength);
    for (int i = 0; i < xlength; i++) {
        grid.sites[i] = (Site *)malloc(sizeof(Site) * ylength);
        for (int j = 0; j < ylength; j++) {
            grid.sites[i][j].animals = NULL;
            grid.sites[i][j].hunters = NULL;
            grid.sites[i][j].nhunters = 0;
            grid.sites[i][j].nanimals = 0;
            double r = rand() / (double)RAND_MAX;
            SiteType st;
            if (r < 0.33)
                st = WINTERING;
            else if (r < 0.66)
                st = FEEDING;
            else
                st = NESTING;
            grid.sites[i][j].type = st;
        }
    }

    return grid;
}

/**
 * @brief
 *
 */
void deletegrid() {
    for (int i = 0; i < grid.xlength; i++) {
        free(grid.sites[i]);
    }

    free(grid.sites);

    grid.sites = NULL;
    grid.xlength = -1;
    grid.ylength = -1;
}

/**
 * @brief prints the number animals and hunters in each site
 * of a given grid
 * @param grid
 */
void printgrid() {
    for (int i = 0; i < grid.xlength; i++) {
        for (int j = 0; j < grid.ylength; j++) {
            Site *site = &grid.sites[i][j];
            int count[3] = {0}; /* do not forget to initialize*/
            for (int a = 0; a < site->nanimals; a++) {
                Animal *animal = site->animals[a];
                count[animal->type]++;
            }

            printf("|%d-{%d, %d, %d}{%d}|", site->type, count[0], count[1],
                   count[2], site->nhunters);
        }
        printf("\n");
    }
}

/**
 * @brief prints the info of a given site
 *
 */
void printsite(Site *site) {
    int count[3] = {0}; /* do not forget to initialize*/
    for (int a = 0; a < site->nanimals; a++) {
        Animal *animal = site->animals[a];
        count[animal->type]++;
    }
    printf("|%d-{%d,%d,%d}{%d}|", site->type, count[0], count[1], count[2],
           site->nhunters);
}

/*
============================================================= 
TODO: you need to complete following three functions 
DO NOT CHANGE ANY OF THE FUNCTION NAME OR TYPES
============================================================= 
*/

/* 
    Bu fonksiyon ile fonksiyona parametre olarak verilen 
    animal ya da hunter'in lokasyonlarina komsu lokasyonlar
    belirlenir ve bu lokasyonlar arasindan random bir tane 
    secilir ve dondurulur.
*/
Location get_a_random_location(Location location){
    
    srand(time(NULL));
    
    Location lc = location;
    /* Butun olasi lokasyonlar. */
    Location posibleLocations[4] = {
        {lc.x - 1, lc.y},
        {lc.x, lc.y - 1},
        {lc.x + 1, lc.y},
        {lc.x, lc.y + 1}
    };

    Location validLocations[4];
    int validCount = 0;

    for(int i = 0; i < 4; i++){
        if(0 <= posibleLocations[i].x && posibleLocations[i].x < grid.xlength && 
           0 <= posibleLocations[i].y && posibleLocations[i].y < grid.ylength){
            validLocations[validCount] = posibleLocations[i];
            validCount ++;
        }    
    }

    int random = rand() % validCount;

    return validLocations[random];
}

/**
 * @brief  it moves a given hunter or animal
 * randomly in the grid
 * @param args is an animalarray
 * @return void*
 */
void *simulateanimal(void *args) {

    srand(time(NULL));

    /* Arguman olarak gonderilen animalin degerleri alinir. */
    Animal *animal = (Animal *)args;
    int type = animal->type;
    Location firstlc = animal->location;

    printf("Animaltype => %d ilk lokasyon :: x : %d || y : %d\n", type, firstlc.x, firstlc.y);

    /* Animal olmedigi surece 5 dongu boyunca yasar .*/
    for (int i = 0; i < 5; i++) {

        Location lc = animal->location;
        Site *site = &grid.sites[lc.x][lc.y];
        int nanimals = site->nanimals;
        int random = rand() % 10;

        /* Animalin bulundugu site'in animals dizisindeki indexi bulunur. */
        int index = -1;
        for (int a = 0; a < nanimals; a++) {
            if (site->animals[a] == animal) {
                index = a;
                break;
            }
        }
        
        /* Animal yasiyormu diye kontrol edilir. */
        if(animal->status == DEAD){
            site->animals[index] = NULL;

            pthread_exit(0);
        
        }
        
        if (site->type == FEEDING){
            if(random >= 8){

                /* %20 ihtimalle yer degistirir. */
                /* Bulunan index'e gore animals dizisi guncellenir. Animal "get_a_random_location" 
                   fonksiyonundan gelen yeni lokasyona gore yeni site'a eklenir.  */
                
                if (index != -1) {
                    Animal *temp = site->animals[nanimals - 1];
                    site->animals[nanimals - 1] = animal;
                    site->animals[index] = temp;
                    Location newLocation = get_a_random_location(lc);
                    animal->location = newLocation;

                    Site *newSite = &grid.sites[newLocation.x][newLocation.y];

                    pthread_mutex_lock(&m1);

                    site->animals[nanimals - 1] = NULL;
                    site->nanimals--;

                    newSite->animals[newSite->nanimals] = animal;
                    newSite->nanimals++;

                    pthread_mutex_unlock(&m1);
                    printf("Animaltype => %d animal'in yeni lokasyonu:: x : %d || y : %d\n", animal->status, type, newLocation.x, newLocation.y);

                } else {

                    /* Animal animals dizisinde bulunamadiysa mesaj verilir. */
                    printf("index = %d animal type = %d index bulunamadi.\n", index, type);
                
                }
            }
        
        }else if (site->type == NESTING){
                    
            /* Bulunan index'e gore animals dizisi guncellenir. Animal "get_a_random_location" 
               fonksiyonundan gelen yeni lokasyona gore yeni site'a eklenir.  */
            
            if (index != -1) {                
                Animal *temp = site->animals[nanimals - 1];
                site->animals[nanimals - 1] = animal;
                site->animals[index] = temp;
                Location newLocation = get_a_random_location(lc);
                animal->location = newLocation;

                Site *newSite = &grid.sites[newLocation.x][newLocation.y];

                pthread_mutex_lock(&m1);

                site->animals[nanimals - 1] = NULL;
                site->nanimals--;

                newSite->animals[newSite->nanimals] = animal;
                newSite->nanimals++;

                pthread_mutex_unlock(&m1);
                printf("Animaltype => %d animal'in yeni lokasyonu:: x : %d || y : %d\n", animal->status, type, newLocation.x, newLocation.y);

            } else {
                /* Animal animals dizisinde bulunamadiysa mesaj verilir. */

                printf("index = %d animal type = %d index bulunamadi.\n", index, type);
            
            }
        
        }else if (site->type == WINTERING){
            if(random <  5){

                /* %50 ihtimalle yer degistirir. */
                /* Bulunan index'e gore animals dizisi guncellenir. Animal "get_a_random_location" 
                   fonksiyonundan gelen yeni lokasyona gore yeni site'a eklenir.  */
                
                if (index != -1) {
                    Animal *temp = site->animals[nanimals - 1];
                    site->animals[nanimals - 1] = animal;
                    site->animals[index] = temp;
                    Location newLocation = get_a_random_location(lc);
                    animal->location = newLocation;

                    Site *newSite = &grid.sites[newLocation.x][newLocation.y];

                    pthread_mutex_lock(&m1);

                    site->animals[nanimals - 1] = NULL;
                    site->nanimals--;

                    newSite->animals[newSite->nanimals] = animal;
                    newSite->nanimals++;

                    pthread_mutex_unlock(&m1);
                    printf("Animaltype => %d animal'in yeni lokasyonu:: x : %d || y : %d\n", animal->status, type, newLocation.x, newLocation.y);

                } else {
                    /* Animal animals dizisinde bulunamadiysa mesaj verilir. */

                    printf("index = %d animal type = %d index bulunamadi.\n", index, type);
                
                }               
            
            }else{

                /* %50 ihtimalle olur. */
                animal->status = DEAD;
                
                pthread_mutex_lock(&m1);
                site->nanimals --;
                pthread_mutex_unlock(&m1);

            }
        }

    }

    pthread_exit(0);
}

/**
 * @brief simulates the moving of a hunter
 *
 * @param args
 * @return void*
 */
void *simulatehunter(void *args) {
    
    srand(time(NULL));
    
    /* Arguman olarak gonderilen animalin degerleri alinir. */
    Hunter *hunter = (Hunter *)args;
    Location firstlc = hunter->location;

    printf("Hunter'ın ilk lokasyonu :: x : %d || y : %d\n", firstlc.x, firstlc.y);
    
    /* Hunter 5 dongu boyunca yasar .*/
    for (int i = 0; i < 5; i++) {

        /* Hunter'in bulundugu konum ve o site'daki animals sayisi alinir. */
        Location lc = hunter->location;
        Site site = grid.sites[lc.x][lc.y];

        int nanimals = grid.sites[lc.x][lc.y].nanimals;
        
        /* O site'daki animal sayisi hunter'a point olarak eklenir. */
        hunter->points += nanimals;
        printf("Site : %d - %d deki hunter %d tane animal oldurdu.\n", lc.x, lc.y, nanimals);
        
        /* O site'daki tum animallar ıcın status DEAD olarak degistirilir. 
           Ayni zamanda site'ın nanimals'ı sifirlanir. */
        for (int i = 0; i < nanimals; i++){
            pthread_mutex_lock(&m1);

            grid.sites[lc.x][lc.y].animals[i]->status = DEAD;
            grid.sites[lc.x][lc.y].nanimals --;
            
            pthread_mutex_unlock(&m1);

        }

        /* Hunter icin yeni lokasyon olusturulur. */
        Location newLocation = get_a_random_location(lc);
        printf("Yeni hunter konumu :: x : %d || y : %d\n", newLocation.x, newLocation.y);
        hunter->location = newLocation;
        Site newSite = grid.sites[newLocation.x][newLocation.y];

        pthread_mutex_lock(&m1);

        /* Onceki site ve yeni site guncellenir. */
        site.nhunters--;
        newSite.nhunters++;

        pthread_mutex_unlock(&m1);

        usleep(1000);
    }

    printf("======>>>> ....OYUN BITTI.... <<<<======\n");
    pthread_exit(0);

}

/**
 * the main function for the simulation
 */
int main(int argc, char *argv[]) {
    
    initgrid(5, 5);
    
    srand(time(NULL));

    /* Hunter sayisi alinir. Alinan hunter sayisi kadar bir bellek ayirilir. */
    int hunter_count = atoi(argv[1]);
    Hunter* myhunters = malloc(hunter_count * sizeof(Hunter));

    /* Grid'teki butun site'lar için animals dizileri 10 animal alacak kadar alan ayrilir.*/
    for(int i = 0; i < grid.xlength; i++){
        for(int j = 0; j < grid.ylength; j++){
            Site *site = &grid.sites[i][j];

            site->animals = (Animal**)realloc(site->animals, 10 * sizeof(Animal*));
        }
    }

    /* Hunter sayisi kadar hunter olusturulur, random lokaysonlara atanir ve grid'e eklenir. */
    for (int i = 0; i < hunter_count; ++i){
        Hunter *newHunter = malloc(sizeof(Hunter));
        int randomValuex = rand() % grid.xlength;
        int randomValuey = rand() % grid.ylength;

        newHunter->points = 0;
        newHunter->location.x = randomValuex;
        newHunter->location.y = randomValuey;
        myhunters[i] = *newHunter;
        Site *site = &grid.sites[randomValuex][randomValuey];
        site->nhunters++;

    }
    
    /* Bear olusturulur ve lokasyonuna gore site'a eklenir. */
    int randomValuexBear = rand() % grid.xlength;
    int randomValueyBear = rand() % grid.ylength;
    
    bear.status = ALIVE;
    bear.type = BEAR;
    bear.location.x = randomValuexBear;
    bear.location.y = randomValueyBear;

    grid.sites[randomValuexBear][randomValueyBear].animals[grid.sites[randomValuexBear][randomValueyBear].nanimals] = &bear;
    grid.sites[randomValuexBear][randomValueyBear].nanimals ++;
    
    /* Bird olusturulur ve lokasyonuna gore site'a eklenir. */
    int randomValuexBird = rand() % grid.xlength;
    int randomValueyBird = rand() % grid.ylength;
    
    bird.status = ALIVE;
    bird.type = BIRD;
    bird.location.x = randomValuexBird;
    bird.location.y = randomValueyBird;

    grid.sites[randomValuexBird][randomValueyBird].animals[grid.sites[randomValuexBird][randomValueyBird].nanimals] = &bird;
    grid.sites[randomValuexBird][randomValueyBird].nanimals ++;

    /* Panda olusturulur ve lokasyonuna gore site'a eklenir. */
    int randomValuexPanda = rand() % grid.xlength;
    int randomValueyPanda = rand() % grid.ylength;
    
    panda.status = ALIVE;
    panda.type = PANDA;
    panda.location.x = randomValuexPanda;
    panda.location.y = randomValueyPanda;

    grid.sites[randomValuexPanda][randomValueyPanda].animals[grid.sites[randomValuexPanda][randomValueyPanda].nanimals] = &panda;
    grid.sites[randomValuexPanda][randomValueyPanda].nanimals ++;

    /* Istenen hunter sayisi kadar thread olusturulur. */
    pthread_t hunterthreads[hunter_count];

    /* Herbir animal icin thread olusturulur. */
    pthread_t bearthread;
    pthread_t birdthread;
    pthread_t pandathread;

    /* Kullanilacak mutex initialize edilir. */
    pthread_mutex_init(&m1, NULL);
    
    /* Olustuurlan tum hunterlar icin thread create edilir. */
    for (int i = 0; i < hunter_count; i++) {
        pthread_create(&hunterthreads[i], NULL, &simulatehunter, (void *)&myhunters[i]);

    }

    /* Olustuurlan tum animallar icin thread create edilir. */
    pthread_create(&bearthread, NULL, &simulateanimal, &bear);
    pthread_create(&birdthread, NULL, &simulateanimal, &bird);    
    pthread_create(&pandathread, NULL, &simulateanimal, &panda);

    for (int i = 0; i < hunter_count; i++) {
        pthread_join(hunterthreads[i], NULL);

    }

    pthread_join(bearthread, NULL);
    pthread_join(birdthread, NULL);
    pthread_join(pandathread, NULL);

    printgrid();
    deletegrid();

    return 0;
}
