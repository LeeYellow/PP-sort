#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float* tmp;
float* freee;
float* arr;
float* recv;

int List_size_in_process, front, rest;
int RecvPrev, RecvNext; 

void merge_L(){ 
  int recv_ptr = RecvPrev - 1;
  int arr_ptr = List_size_in_process - 1;
  for(int tmp_ptr = List_size_in_process-1; tmp_ptr >=0; tmp_ptr--){ 
    if(recv_ptr>=0 && arr[arr_ptr] < recv[recv_ptr]){
      tmp[tmp_ptr] = recv[recv_ptr];
      --recv_ptr;
    }else{
      tmp[tmp_ptr] = arr[arr_ptr];
      --arr_ptr;
    }
  }
 
  freee = arr;
  arr = tmp;
  tmp = freee;

}

void merge_R(){ 
  int recv_ptr = 0;
  int arr_ptr = 0;
  for(int tmp_ptr = 0; tmp_ptr < List_size_in_process; tmp_ptr++){
    if(recv_ptr< RecvNext && recv[recv_ptr] < arr[arr_ptr]){
          tmp[tmp_ptr] = recv[recv_ptr];
          ++recv_ptr;
    }else{
      tmp[tmp_ptr] = arr[arr_ptr];
      ++arr_ptr;
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
		int ranges[3] = { N, size-1, 1 };                   // Remove all unnecessary ranks
		MPI_Group_range_excl(origin_group, 1, ranges, &new_group);
		MPI_Comm_create(MPI_COMM_WORLD, new_group, &custom_world);// Create a new communicator

		if (custom_world == MPI_COMM_NULL){// Bye bye cruel world
			MPI_Finalize();
			exit(0);
		}size = N;
	}
  
	rest = N%size;
		
  List_size_in_process = N/size;
  if(rank<rest) {++List_size_in_process;
  front = rank*(List_size_in_process);}
  else front = rank*(List_size_in_process) + rest;
  RecvPrev = List_size_in_process;
	RecvNext = List_size_in_process;
  if(rank == rest && rest!=0)RecvPrev=List_size_in_process+1;
  if((rank+1) == rest)RecvNext=List_size_in_process-1;
  
	//MPI open file
	MPI_File file_in,file_out;
	int rc=MPI_File_open(custom_world, argv[2], MPI_MODE_RDONLY, MPI_INFO_NULL,&file_in);
    //check the file open state
	if (rc!=MPI_SUCCESS){
		printf("fail to open\n");
		MPI_Abort(custom_world,rc);
	}
  
  arr=(float*)malloc(List_size_in_process*sizeof(float));
  recv=(float*)malloc(RecvPrev*sizeof(float));
	tmp=(float*)malloc((List_size_in_process)*sizeof(float));
    
  //MPI read file
	offset=front*sizeof(MPI_FLOAT);
	MPI_File_read_at(file_in,offset,arr,List_size_in_process,MPI_FLOAT,MPI_STATUS_IGNORE);
	MPI_File_close(&file_in);
 

	//sort inside 
	qsort(arr, List_size_in_process, sizeof(float), cmp);
	
	//char tmp2, isSorted = 0;
	//while(isSorted==0){
  for(int j = 0; j < size; j++){
		//isSorted = 1;
		if(rank%2){
			if(rank > 0){ 
				MPI_Sendrecv(arr,List_size_in_process,MPI_FLOAT,rank-1,1,recv,RecvPrev,MPI_FLOAT,rank-1,0,custom_world,MPI_STATUS_IGNORE);
				merge_L();
			}

			if(rank != size-1){	
				MPI_Sendrecv(arr,List_size_in_process,MPI_FLOAT,rank+1,0,recv,RecvNext,MPI_FLOAT,rank+1,1,custom_world,MPI_STATUS_IGNORE);
				merge_R();
			}
		}else{
			if(rank != size-1){	
				MPI_Sendrecv(arr,List_size_in_process,MPI_FLOAT,rank+1,0,recv,RecvNext,MPI_FLOAT,rank+1,1,custom_world,MPI_STATUS_IGNORE);			
  			merge_R();
			}
			if(rank > 0){ 
				MPI_Sendrecv(arr,List_size_in_process,MPI_FLOAT,rank-1,1,recv,RecvPrev,MPI_FLOAT,rank-1,0,custom_world,MPI_STATUS_IGNORE);
				merge_L();
			}
		}
    //tmp2=isSorted;
    //MPI_Allreduce(&tmp2, &isSorted, 1,MPI_CHAR, MPI_BAND, custom_world);
	}
  
	MPI_File_open(custom_world, argv[3], MPI_MODE_CREATE|MPI_MODE_WRONLY , MPI_INFO_NULL, &file_out);
	MPI_File_write_at(file_out , offset, arr , List_size_in_process, MPI_FLOAT, MPI_STATUS_IGNORE);
	MPI_File_close(&file_out);

	free(arr);
	free(tmp);
  free(recv);
	MPI_Finalize();
	return 0;
}



