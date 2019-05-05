#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>
#include <stdint.h>
#include "sys/types.h"
#include "sys/sysinfo.h"

#define STRING_SIZE 8000

int NUM_THREADS;
char** substring_array;
int file_line_count;

typedef struct {
        uint32_t virtualMem;
        uint32_t physicalMem;
} processMem_t;

int parseLine(char *line) {
        // This assumes that a digit will be found and the line ends in " Kb".
        int i = strlen(line);
        const char *p = line;
        while (*p < '0' || *p > '9') p++;
        line[i - 3] = '\0';
        i = atoi(p);
        return i;
}

void GetProcessMemory(processMem_t* processMem) {
        FILE *file = fopen("/proc/self/status", "r");
        char line[128];

        while (fgets(line, 128, file) != NULL) {
                //printf("%s", line);
                if (strncmp(line, "VmSize:", 7) == 0) {
                        processMem->virtualMem = parseLine(line);
                }

				if (strncmp(line, "VmRSS:", 6) == 0) {
                        processMem->physicalMem = parseLine(line);
                }
        }
        fclose(file);
}

void free_array(int **array, int length){
	int i;
	for(i = 0; i < length;i++){
		free(array[i]);
	}
	free(array);
}
int count_lines(char* filename){
	FILE *fp;
	char c;
	int count = 0;
	fp = fopen(filename, "r");
	if(fp == NULL){
		printf("Error open %s\n",filename);
		return -1;
	}
	while(!feof(fp)){
		c = fgetc(fp);
		if(c == '\n')
			count++;
	}
	fclose(fp);
	return count;
}

char* substr(char *src, int m, int n){
	int len = n - m;
	char *dest = (char*)malloc(sizeof(char) * (len + 1));
	for(int i = m; i < n && (*src != '\0');i++){
		*dest = *(src+i);
		dest++;
	}
	*dest = '\0';
	return dest - len;
}

void longest_common_substring(char *first_string, char *second_string){
	int strlen_first = strlen(first_string);
	int strlen_second = strlen(second_string);
	int result = 0;
	int end;
	int **len = malloc((strlen_second+1) * sizeof(int *));
	for(int k = 0; k < (strlen_second+1);k++)
		len[k] = (int*)malloc((strlen_first+1)*sizeof(int));
	for(int i = 0; i <= strlen_second;i++){
		for(int j = 0; j <= strlen_first;j++){
			if(i == 0  || j == 0)
				len[i][j] = 0; 
			else if(second_string[i-1] == first_string[j-1]){
				len[i][j] = len[i-1][j-1]+1;
				if(len[i][j]> result){
					result = len[i][j];
					end = i-1;
				}
			}
			else
				len[i][j]=0;
		}
	}
	if(result == 0){
		const char no_substring[19] = "No substring found";
		memcpy(first_string,no_substring,19);
		first_string[19] = '\0';
	}
	else{
		char* dest = substr(second_string,end-result+1,end+1);
		memcpy(first_string, dest,result);
		first_string[result] = '\0';
		free(dest);
		free_array(len, strlen_second);
	}
}

void longest_common_substring_setup(int rank){
	int startPos = rank * ceil((double)file_line_count / NUM_THREADS);
	int endPos = startPos + ceil((double)file_line_count / NUM_THREADS);
	int i;
	for(i = startPos; (i < endPos) && (i+1 < file_line_count); i++){
		longest_common_substring(substring_array[i], substring_array[i+1]);
	}
}

void trimTrailing(char * str){
	int index, i;
	index = -1;
	i=0;
	while(str[i] != '\0'){
		if(str[i] != ' ' && str[i] != '\n')
			index = i;
		i++;
	}
	str[index+1] = '\0';
}

void print_results(int counter){
	int i;
	for(i=0;i<counter-1;i++){
		printf("%d-%d: %s\n",i,i+1,substring_array[i]);
	}
}

int main(int argc, char *argv[]) {
	FILE *fp;
	struct timeval t1, t2;
	double elapsedTime;
	int myVersion = 1;
	int problem_size, i;
	char* filename;
	processMem_t myMem;	

	gettimeofday(&t1, NULL);
	printf("DEBUG: starting loop on %s\n", getenv("HOSTNAME"));
	if((argc == 2)){
		filename = argv[1];
	}
	else if(argc == 3){
		filename = argv[1];
		problem_size = strtol(argv[2], NULL, 10);
	}
	else{
		printf("Error: Program needs a problem size\n");
		return 1;
	}
	
	file_line_count = count_lines(filename);

	if((argc > 2) && (problem_size < file_line_count)){
		file_line_count = problem_size;
	}
	substring_array = malloc(sizeof(char*) * file_line_count);
	
	fp = fopen(filename,"r");
	if(fp == NULL){
		printf("%s failed to open\n", filename);
		return 1;
	}

	for(i = 0; i < file_line_count; i++){
		if(ferror(fp) || feof(fp))
			break;
		substring_array[i] = malloc(sizeof(char) * STRING_SIZE);
		fgets(substring_array[i],STRING_SIZE,fp);
		trimTrailing(substring_array[i]);
	}

	fclose(fp);
	int rc, numtasks, rank;
	
	MPI_Status Status;
	rc = MPI_Init(&argc,&argv);
	if(rc != MPI_SUCCESS){
		printf("Error starting MPI program.");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
	
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	NUM_THREADS = numtasks;
	
	longest_common_substring_setup(rank);
	
	MPI_Finalize();
	if(rank == 0){
		print_results(file_line_count);
		GetProcessMemory(&myMem);
		gettimeofday(&t2, NULL);
		elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
		elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
		printf("DATA, %d, %s, %s, %f\n", myVersion, getenv("SLURM_JOB_NUM_NODES"),getenv("SLURM_JOB_CPUS_PER_NODE"), elapsedTime);
		printf("Memory: vMem %u KB, pMem %u KB\n", myMem.virtualMem, myMem.physicalMem);
	}
	free_array((int **) substring_array, file_line_count);
	return 0;
}
