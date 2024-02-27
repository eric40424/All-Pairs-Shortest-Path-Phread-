#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sched.h>
#include <pthread.h>
#include <omp.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define vMax 6000
#define wMax 1000
#define INF 1073741823

int vertexNum, edgeNum;
int numThread;
int graph[vMax][vMax];
int *input_bin,*output_bin;
pthread_barrier_t barrier;

void* APSP(void* args)
{
    int *data = (int*)args;
    for(int i=*data;i<vertexNum;i+=numThread){
        for(int j=0;j<vertexNum;j++){
            graph[i][j] = INF;
        }
    }

    for(int i=0;i<vertexNum;i++){
        graph[i][i] = 0;
    }

    pthread_barrier_wait(&barrier);

    for(int i=*data;i<edgeNum;i+=numThread){
        //int e[3];
        //f.read((char*)e, sizeof(e));
        //printf("%4d -> %4d, dist=%4d\n", e[0], e[1], e[2]);
        //graph[e[0]][e[1]] = e[2];
        graph[input_bin[2+i*3]][input_bin[3+i*3]] = input_bin[4+i*3];
    }

    pthread_barrier_wait(&barrier);

    pthread_barrier_wait(&barrier);
    for(int k=0;k<vertexNum;k++){
        for(int i=*data;i<vertexNum;i+=numThread){
            for(int j=0;j<vertexNum;j++){
                int temp = graph[i][k]+graph[k][j];
                if(temp<graph[i][j]){
                    graph[i][j]=temp;
                }
            }
        }
        pthread_barrier_wait(&barrier);
    }
    pthread_exit(NULL);
}

/*void* APSP(void* args)
{
    int *data = (int*)args;
    for(int k=0;k<vertexNum;k++){
        for(int i=0;i<vertexNum;i++){
            for(int j=*data;j<vertexNum;j+=numThread){
                int temp = graph[i][k]+graph[k][j];
                if(temp<graph[i][j]){
                    graph[i][j]=temp;
                }
            }
        }
        pthread_barrier_wait(&barrier);
    }
    pthread_exit(NULL);
}*/

/*void floydwarshall()
{
    for(int k=0;k<vertexNum;k++){
        #pragma omp parallel for num_threads(numThread) schedule(dynamic) collapse(2)
        for(int i=0;i<vertexNum;i++){
            for(int j=0;j<vertexNum;j++){
                int temp = graph[i][k]+graph[k][j];
                if(temp<graph[i][j]){
                    graph[i][j]=temp;
                }
            }
        }
    }
}*/

/*void APSP_CPP(int k,int index)
{
    for(int i=index;i<vertexNum;i+=numThread){
        for(int j=0;j<vertexNum;j++){
            int temp = graph[i][k]+graph[k][j];
            if(temp<graph[i][j]){
                graph[i][j]=temp;
            }
        }
    }
}*/

int main(int argc,char** argv)
{
    cpu_set_t cpu_set;
    sched_getaffinity(0, sizeof(cpu_set), &cpu_set);
    printf("%d cpus available\n", CPU_COUNT(&cpu_set));
    numThread = CPU_COUNT(&cpu_set);
    //char *inputFile = argv[1];
    //char *outputFile = argv[2];
    //std::ifstream f(argv[1]);
    std::ofstream fout(argv[2], std::ios::out | std::ios::binary); 

    //int vertexNum, edgeNum;
    /*f.seekg (0, std::ios_base::beg);
    f.read((char*)&vertexNum,sizeof(vertexNum));
    f.read((char*)&edgeNum,sizeof(edgeNum));*/
    //printf("(%d,%d)\n",vertexNum,edgeNum);

    int fd = open(argv[1], O_RDONLY);
    input_bin = (int*)mmap(NULL, 2 * sizeof(int), PROT_READ, MAP_SHARED, fd, 0);
    vertexNum = input_bin[0];
    edgeNum = input_bin[1];
    input_bin = (int*)mmap(NULL, (edgeNum * 3 + 2) * sizeof(int), PROT_READ, MAP_SHARED, fd, 0);

    //#pragma omp parallel for num_threads(numThread) collapse(2)
    /*for(int i=0;i<vertexNum;i++){
        for(int j=0;j<vertexNum;j++){
            graph[i][j] = INF;
        }
    }*/
    //#pragma omp parallel for num_threads(numThread)
    /*for(int i=0;i<vertexNum;i++){
        graph[i][i] = 0;
    }*/

    /*for(int i=0;i<edgeNum;i++){
        //int e[3];
        //f.read((char*)e, sizeof(e));
        //printf("%4d -> %4d, dist=%4d\n", e[0], e[1], e[2]);
        //graph[e[0]][e[1]] = e[2];
        graph[input_bin[2+i*3]][input_bin[3+i*3]] = input_bin[4+i*3];
    }*/

    pthread_barrier_init(&barrier,NULL,numThread);
    pthread_t threads[numThread];
    int tids[numThread];
    for(int i=0;i<numThread;i++){
        tids[i] = i;
        pthread_create(&threads[i],NULL,APSP,(void*)&tids[i]);
    }

    for(int i=0;i<numThread;i++){
        pthread_join(threads[i],NULL);
    }
    //floydwarshall();
    //std::thread threads[numThread];
    /*std::vector<std::thread> threads; 
    for(int k=0;k<vertexNum;k++){
        for(int i=0;i<numThread;i++){
            //threads[i](APSP_CPP,k,i);
            threads.push_back(std::thread(APSP_CPP,k,i));
        }
        for(int j=0;j<numThread;j++){
            threads[j].join();
        }
    }*/
    //munmap(input_bin, (edgeNum * 3 + 2) * sizeof(int));
    /*int fd2 = open(argv[2],O_RDWR|O_CREAT|O_TRUNC);
    int result = lseek(fd2, sizeof(int)*vertexNum*vertexNum-1, SEEK_SET);
    result = write(fd2, "", 1);
    output_bin = (int*)mmap(0, sizeof(int)*vertexNum*vertexNum, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);
    for(int i=0;i<vertexNum;i++){
        for(int j=0;j<vertexNum;j++){
            output_bin[i*vertexNum+j] = graph[i][j];
        }
    }*/
    //close(fd2);

    /*for(int i=0;i<vertexNum;i++){
        for(int j=0;j<vertexNum;j++){
            fout.write((char*)&graph[i][j],sizeof(int));
        }
    }*/

    for(int i=0;i<vertexNum;i++){
        fout.write((char*)&graph[i],sizeof(int)*vertexNum);
    }

    //f.close();
    //fout.close();
}