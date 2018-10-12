
send rear
/*printf("==============merge receive  rank %d=======\n", rank);
				printf("chagenum = %d\n", changeNum);
				printf("send");
				for(int i = List_size_in_process-changeNum; i<List_size_in_process; i++){
					printf("%f ", arr[i]);
				}printf("\n");
				printf("recv");
				for(int i = 0; i<changeNum; i++){
					printf("%f ", recv[i]);
				}printf("\n");
				*/
				
			send head	
				/*printf("==============merge receive  rank %d=======\n", rank);
				printf("chagenum = %d\n", changeNum);
				printf("send");
				for(int i = 0; i<changeNum; i++){
					printf("%f ", arr[i]);
				}printf("\n");
				printf("recv");
				for(int i = 0; i<changeNum; i++){
					printf("%f ", recv1[i]);
				}printf("\n");*/
				
				
				
//sort inside                           
		/*printf("==============before=============\n");
		printf("rank %d : List_size_in_proces %d\n", rank, List_size_in_process);
		printf("%d:\n", rank);
		for(int i = 0; i<List_size_in_process; i++){
		  printf("%f ", arr[i]);
		}printf("\n");*/
		//qsort(arr, List_size_in_process, sizeof(float), cmp);
		/*printf("==============after=============\n");
		printf("rank %d : List_size_in_proces %d\n", rank, List_size_in_process);
		printf("%d:\n", rank);
		for(int i = 0; i<List_size_in_process; i++){
		  printf("%f ", arr[i]);
		}printf("\n");*/