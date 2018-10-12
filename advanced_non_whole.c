#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#define max(x,y) (x > y?x:y) 

int cmp( const void *a , const void *b);
void merge(float *arr, float *recv, int* changenum, int* arr_num, int mode, char* isSorted);
float* tmp;
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
		MPI_Comm_group(MPI_COMM_WORLD, &origin_group);		// Obtain the group of processes in the world communicator
		int ranges[][3] = { {N, size-1, 1} };                   // Remove all unnecessary ranks
		MPI_Group_range_excl(origin_group, 1, ranges, &new_group);
		MPI_Comm_create(MPI_COMM_WORLD, new_group, &custom_world);// Create a new communicator

		if (custom_world == MPI_COMM_NULL){// Bye bye cruel world
			MPI_Finalize();
			exit(0);
		}size = N;
	}
  
	int List_size_in_process, front, rest;
	int RecvPrev, RecvNext;

	rest = N%size;
	if(rest){
		if(rank<rest){
			List_size_in_process = N/size+1;
			front = rank*(List_size_in_process);
			RecvPrev = List_size_in_process;
			if(rank==rest-1)RecvNext = List_size_in_process-1;
			else RecvNext = List_size_in_process;
		}else{
			List_size_in_process = N/size;
			front = rank*List_size_in_process+rest;
			RecvNext = List_size_in_process;
			if(rank==rest)RecvPrev = List_size_in_process+1;
			else RecvPrev = List_size_in_process;

		}
	}else{
		List_size_in_process = N/size;
		front = rank*List_size_in_process;
		RecvPrev = List_size_in_process;
		RecvNext = List_size_in_process;
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
	float *arr=(float*)malloc(List_size_in_process*sizeof(MPI_FLOAT));
	MPI_File_read_at(file_in,offset,arr,List_size_in_process,MPI_FLOAT,MPI_STATUS_IGNORE);
	MPI_File_close(&file_in);
 
	char isSorted = 0;
	//int changeprev = ceil(ceil((double)N/(double)size)/2);
	
	//sort inside 
	qsort(arr, List_size_in_process, sizeof(float), cmp);
	float *recv=(float*)malloc(RecvPrev*sizeof(float));
	tmp=(float*)malloc((List_size_in_process+max(RecvPrev,RecvNext))*sizeof(float));
	while(isSorted==0){
		isSorted = 1;
		
		if(rank%2){
			if(rank > 0){ 
				//float *recv1=(float*)malloc(RecvPrev*sizeof(MPI_FLOAT));//send to / receive from prev keep large 10
				MPI_Sendrecv(arr,List_size_in_process,MPI_FLOAT,rank-1,1,recv,RecvPrev,MPI_FLOAT,rank-1,0,custom_world,MPI_STATUS_IGNORE);

				merge(arr,recv,&RecvPrev,&List_size_in_process,1, &isSorted);
				//free(recv1);
			}
			MPI_Barrier(custom_world);
			if(rank != size-1){	
				//float *recv=(float*)malloc(RecvNext*sizeof(MPI_FLOAT));//send to next keep small 01
				MPI_Sendrecv(arr,List_size_in_process,MPI_FLOAT,rank+1,0,recv,RecvNext,MPI_FLOAT,rank+1,1,custom_world,MPI_STATUS_IGNORE);
				
				merge(arr,recv,&RecvNext,&List_size_in_process,0, &isSorted);
				//free(recv);	
			}
		}else{
			if(rank != size-1){	
				MPI_Sendrecv(arr,List_size_in_process,MPI_FLOAT,rank+1,0,recv,RecvNext,MPI_FLOAT,rank+1,1,custom_world,MPI_STATUS_IGNORE);
				merge(arr,recv,&RecvNext,&List_size_in_process,0, &isSorted);
			}
			MPI_Barrier(custom_world);
			if(rank > 0){ 
				MPI_Sendrecv(arr,List_size_in_process,MPI_FLOAT,rank-1,1,recv,RecvPrev,MPI_FLOAT,rank-1,0,custom_world,MPI_STATUS_IGNORE);
				merge(arr,recv,&RecvPrev,&List_size_in_process,1, &isSorted);
			}
		}
		char tmp2=isSorted;
		MPI_Allreduce(&tmp2, &isSorted, 1,MPI_CHAR, MPI_BAND, custom_world);
	}
  
	MPI_File_open(custom_world, argv[3], MPI_MODE_CREATE|MPI_MODE_WRONLY , MPI_INFO_NULL, &file_out);
	MPI_File_write_at(file_out , offset, arr , List_size_in_process, MPI_FLOAT, MPI_STATUS_IGNORE);
	MPI_File_close(&file_out);

	free(arr);
	free(tmp);
	MPI_Barrier(custom_world);
	MPI_Finalize();
	return 0;
}

void merge(float *arr, float *recv, int* recvnum, int *arr_num, int mode, char* isSorted){ 
  if(mode){//from prev keep large
	int recv_ptr = *recvnum - 1;
	int tmp_ptr = *arr_num - 1;
    for(int arr_ptr = *arr_num-1; arr_ptr >=0; arr_ptr--){
      tmp[arr_ptr] = arr[arr_ptr]; 
	  if(recv_ptr>=0 && tmp[tmp_ptr] < recv[recv_ptr]){
        arr[arr_ptr] = recv[recv_ptr];
        --recv_ptr;
        *isSorted = 0;
        //printf("swap, mode:keep large\n");
      }else{
        arr[arr_ptr] = tmp[tmp_ptr];
        --tmp_ptr;
      }
    }
  }else{//sent to next keep small
    int recv_ptr = 0;
	int tmp_ptr = 0;
    for(int arr_ptr = 0; arr_ptr < *arr_num; arr_ptr++){
      tmp[arr_ptr] = arr[arr_ptr];
	  if(recv_ptr< *recvnum && recv[recv_ptr] < tmp[tmp_ptr]){
        arr[arr_ptr] = recv[recv_ptr];
        recv_ptr++;
        *isSorted = 0;
      }else{
        arr[arr_ptr] = tmp[tmp_ptr];
        tmp_ptr++;
      }
    }
  }


}


int cmp( const void *a , const void *b){
	if(*(const float*)a < *(const float*)b){

			return -1;
	}
    return *(const float*)a>*(const float*)b;
}

