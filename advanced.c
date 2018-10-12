#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int cmp( const void *a , const void *b);
void merge(int* changenum, int* arr_num, int mode, char* isSorted);
float* tmp;
float* freee;
float* arr;
float* recv;
int main(int argc, char *argv[])
{
        int rank, size;
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Offset offset;
        MPI_Comm custom_world = MPI_COMM_WORLD;
        MPI_Group origin_group, new_group;

        int N = atoi(argv[1]);

        if (N < size) {
                MPI_Comm_group(MPI_COMM_WORLD, &origin_group);          // Obtain the group of processes in the world communicator
                int ranges[3] = { N, size-1, 1 };                   // Remove all unnecessary ranks
                MPI_Group_range_excl(origin_group, 1, ranges, &new_group);
                MPI_Comm_create(MPI_COMM_WORLD, new_group, &custom_world);// Create a new communicator

                if (custom_world == MPI_COMM_NULL){// Bye bye cruel world
                        MPI_Finalize();
                        exit(0);
                }size = N;
        }

        int List_size_in_process, front, rest;
        rest = N%size;
        if(rest){
                if(rank<rest){
                        List_size_in_process = N/size+1;
                        front = rank*(List_size_in_process);
                }else{
                        List_size_in_process = N/size;
                        front = rank*List_size_in_process+rest;
                }
        }else{
                List_size_in_process = N/size;
                front = rank*List_size_in_process;
        }

        //MPI open file
        MPI_File file_in,file_out;
        int rc=MPI_File_open(custom_world, argv[2], MPI_MODE_RDONLY, MPI_INFO_NULL,&file_in);
    //check the file open state
        if (rc!=MPI_SUCCESS){
                printf("fail to open\n");
                MPI_Abort(custom_world,rc);
        }

    //MPI read file
        offset=front*sizeof(MPI_FLOAT);
        arr = (float*)malloc(List_size_in_process*sizeof(float));
        MPI_File_read_at(file_in,offset,arr,List_size_in_process,MPI_FLOAT,MPI_STATUS_IGNORE);
        MPI_File_close(&file_in);

        char tmp2, isSorted = 0;
        int changeNum = ceil(ceil((double)N/(double)size)/2);
        
        tmp = (float*)malloc(List_size_in_process*sizeof(float));
        recv = (float*)malloc(changeNum*sizeof(float));

        //sort inside 
        qsort(arr, List_size_in_process, sizeof(float), cmp);

        while(isSorted==0){
                isSorted = 1;

                if(rank%2){
                        if(rank > 0){
                                MPI_Sendrecv(arr,changeNum,MPI_FLOAT,rank-1,1,recv,changeNum,MPI_FLOAT,rank-1,0,custom_world,MPI_STATUS_IGNORE);
                                merge(&changeNum,&List_size_in_process,1, &isSorted);
                        }
                        //MPI_Barrier(custom_world);
                        if(rank != size-1){
                                MPI_Sendrecv(&arr[List_size_in_process-changeNum],changeNum,MPI_FLOAT,rank+1,0,recv,changeNum,MPI_FLOAT,rank+1,1,custom_world,MPI_STATUS_IGNORE);
                                merge(&changeNum,&List_size_in_process,0, &isSorted);
                        }
                }else{
                        if(rank != size-1){
                                MPI_Sendrecv(&arr[List_size_in_process-changeNum],changeNum,MPI_FLOAT,rank+1,0,recv,changeNum,MPI_FLOAT,rank+1,1,custom_world,MPI_STATUS_IGNORE);
                                merge(&changeNum,&List_size_in_process,0, &isSorted);
                        }
                        //MPI_Barrier(custom_world);
                        if(rank > 0){
                                MPI_Sendrecv(arr,changeNum,MPI_FLOAT,rank-1,1,recv,changeNum,MPI_FLOAT,rank-1,0,custom_world,MPI_STATUS_IGNORE);
                                merge(&changeNum,&List_size_in_process,1, &isSorted);
                        }
                }
                tmp2=isSorted;
                MPI_Allreduce(&tmp2, &isSorted, 1,MPI_CHAR, MPI_BAND, custom_world);
        }

        MPI_File_open(custom_world, argv[3], MPI_MODE_CREATE|MPI_MODE_WRONLY , MPI_INFO_NULL, &file_out);
        MPI_File_write_at(file_out , offset, arr , List_size_in_process, MPI_FLOAT, MPI_STATUS_IGNORE);
        MPI_File_close(&file_out);

        free(arr);
        free(tmp);
        free(recv);
        //MPI_Barrier(custom_world);
        MPI_Finalize();
        return 0;
}

void merge(int* changenum, int *arr_num, int mode, char* isSorted){
  if(mode){//from end keep large
    int recv_ptr = *changenum - 1;
    int arr_ptr = *arr_num - 1;
    for(int tmp_ptr = *arr_num-1; tmp_ptr >=0; tmp_ptr--){
        if(recv_ptr>=0 && arr[arr_ptr] < recv[recv_ptr]){
            tmp[tmp_ptr] = recv[recv_ptr];
            --recv_ptr;
            *isSorted = 0;
        //printf("swap, mode:keep large\n");
        }else{
            tmp[tmp_ptr] = arr[arr_ptr];
            --arr_ptr;
        }
    }
  }else{
    int recv_ptr = 0;
    int arr_ptr = 0;
    for(int tmp_ptr = 0; tmp_ptr < *arr_num; tmp_ptr++){
        if(recv_ptr< *changenum && recv[recv_ptr] < arr[arr_ptr]){
            tmp[tmp_ptr] = recv[recv_ptr];
            ++recv_ptr;
            *isSorted = 0;
      }else{
            tmp[tmp_ptr] = arr[arr_ptr];
            ++arr_ptr;
      }
    }
  }
  freee = arr;
  arr = tmp;
  tmp = freee;

}





int cmp( const void *a , const void *b){ 
        if(*(const float*)a < *(const float*)b){ 
 
                        return -1; 
        } 
    return *(const float*)a>*(const float*)b; 
} 

