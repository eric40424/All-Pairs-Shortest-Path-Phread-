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
#include <iostream>
#include <chrono>

#define vMax 6000
#define wMax 1000
#define INF 1073741823

int vertexNum, edgeNum;
int numThread;
int graph[vMax][vMax];
int output1[2];
int output2[6000][6000];
int *input_bin,*output_bin;
pthread_barrier_t barrier;

void* APSP(void* args)
{
    int *data = (int*)args;
    //int vertex_temp = (vertexNum/4) * 4;
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
    for(int k=0;k<vertexNum;k++){
        for(int i=*data;i<vertexNum;i+=numThread){
            /*for(int j=0;j<vertex_temp;j+=4){
                __m128i as = _mm_lddqu_si128((__m128i const*)&graph[k][j]);
                __m128i bs = _mm_set1_epi32(graph[i][k]);
                __m128i cs = _mm_lddqu_si128((__m128i const*)&graph[i][j]);
                __m128i temp_vec = _mm_add_epi32(bs,as);
                cs = _mm_min_epi32(temp_vec,cs);
                _mm_store_si128((__m128i*)&graph[i][j],cs);
                //int temp = graph[i][k]+graph[k][j];
                //if(temp<graph[i][j]){
                //    graph[i][j]=temp;
                //}
            }*/
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

/*void Floydwarshall()
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
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    cpu_set_t cpu_set;
    sched_getaffinity(0, sizeof(cpu_set), &cpu_set);
    printf("%d cpus available\n", CPU_COUNT(&cpu_set));
    numThread = CPU_COUNT(&cpu_set);

    std::ofstream fout("testcase.in", std::ios::out | std::ios::binary); 
    output1[0] = 6000;
    output1[1] = 5999*6000;

    for(int i=0;i<6000;i++){
        for(int j=0;j<6000;j++){
            if(i!=j){
                output2[i][j] = 1000 - 1000 * i /6000;
            }
        }
    }

    fout.write((char*)&output1[0],sizeof(int));
    fout.write((char*)&output1[1],sizeof(int));

    for(int i=0;i<6000;i++){
        for(int j=0;j<6000;j++){
            if(i!=j){
                fout.write((char*)new int(i),sizeof(int));
                fout.write((char*)new int(j),sizeof(int));
                fout.write((char*)&output2[i][j],sizeof(int));
            }
        }
    }

    // Cal time for report
    /*std::chrono::steady_clock::time_point tout1 = std::chrono::steady_clock::now();
    for(int i=0;i<vertexNum;i++){
        fout.write((char*)&graph[i],sizeof(int)*vertexNum);
    }
    std::chrono::steady_clock::time_point tout2 = std::chrono::steady_clock::now();

    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    std::cout << "ALL time: "
    << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
    << "ms.\n";

    std::cout << "In time: "
    << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() - std::chrono::duration_cast<std::chrono::milliseconds>(tcmp2 - tcmp1).count() - std::chrono::duration_cast<std::chrono::milliseconds>(tout2 - tout1).count()
    << "ms.\n";

    std::cout << "cmp time: "
    << std::chrono::duration_cast<std::chrono::milliseconds>(tcmp2 - tcmp1).count()
    << "ms.\n";

    std::cout << "Out time: "
    << std::chrono::duration_cast<std::chrono::milliseconds>(tout2 - tout1).count()
    << "ms.\n";*/

    f.close();
    fout.close();
}