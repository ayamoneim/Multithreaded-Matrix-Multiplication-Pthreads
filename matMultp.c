#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#define MAX_FILENAME_SIZE 260

/*
*@struct threadData
*@brief The thread arguments struct
*
*@var i row of matrix processed by the thread
*@var j column of matrix processed by the thread
*/
struct threadData{
    int i;
    int j;
};

int rows; // Number of rows in output matrix
int cols; // Number of columns in output matrix
int rc; // Number of columns in matrixA/rows in matrixB

double** mat1; // First Input Matrix
double** mat2; // Second Input Matrix
double** matOut; // Output Matrix

void* matrixMultiplyR(void* id);
void* matrixMultiplyE(void* data);
void allocateMatrix(double ***matPtr, int n, int m);
void displayOutput(char *fileName,char *methodId);
int getMatrix(char *filename, int matrixId);
void removeExtension(char *filename);
int rowsMethod();
int elementsMethod();



void randomGenerator(){
    time_t t;
    srand((unsigned)time(&t));
    rc = rand()%1000;

    rows = rand()%1000;
    cols = rand()%1000;
    FILE *fptr = fopen("a.txt","w");


    int i,j;
    allocateMatrix(&mat1,rows, rc);

    allocateMatrix(&mat2,rc, cols);

    fprintf(fptr,"row=%d col=%d\n",rows,rc);

    for(i = 0;i<rows;i++){
        for(j=0;j<rc;j++){

            mat1[i][j] = (double)rand()/(double)RAND_MAX;
            fprintf(fptr,"%.5lf",mat1[i][j]);
            if(j!=rc-1){
                fprintf(fptr," ");
            }
        }
        fprintf(fptr,"\n");
    }
    fclose(fptr);
    fptr = fopen("b.txt","w");
    fprintf(fptr,"row=%d col=%d\n",rc,cols);

    for(i = 0;i<rc;i++){
        for(j=0;j<cols;j++){
            mat2[i][j] = (double)rand()/(double)RAND_MAX;
            fprintf(fptr,"%.5lf",mat2[i][j]);
            if(j!=cols-1){
                fprintf(fptr," ");
            }
        }
        fprintf(fptr,"\n");
    }
    fclose(fptr);


}

int main(int args, char* argv[])
{
    rows = -1;
    cols = -1;
    //randomGenerator();


    char *mat1_filename ;
    mat1_filename = malloc(sizeof(char)*MAX_FILENAME_SIZE);
    char *mat2_filename ;
    mat2_filename =  malloc(sizeof(char)*MAX_FILENAME_SIZE);
    char *mat3_filename ;
    mat3_filename =  malloc(sizeof(char)*MAX_FILENAME_SIZE);

    // Set filenames

    if(args>1){
        strcpy(mat1_filename, argv[1]);
    }else{
        strcpy(mat1_filename , "a.txt");
    }
    if(args>2){
        strcpy(mat2_filename, argv[2]);

    }else{
        strcpy(mat2_filename, "b.txt");
    }

    if(args>3){
        strcpy(mat3_filename, argv[3]);
        removeExtension(mat3_filename);

    }else{
        strcpy(mat3_filename, "c");
    }

    //Set input Matrices
    int errorFlag = 0;
    errorFlag|=getMatrix(mat1_filename, 1);
    if(!errorFlag){
    	errorFlag|=getMatrix(mat2_filename, 2);
    }
    if(!errorFlag){

        //Allocate memory for the output matrix
        allocateMatrix(&matOut,rows,cols);

        struct timeval stop,start;

        gettimeofday(&start, NULL);
        printf("Method 0:\n");
        printf("----------\n");
        errorFlag|=rowsMethod();
        gettimeofday(&stop, NULL);
        if(!errorFlag){
            // Calculate the method execution time
            printf("Seconds taken --> %lu\n",stop.tv_sec - start.tv_sec);
            printf("(Micro)Seconds taken --> %lu\n",stop.tv_usec - start.tv_usec);
            printf("Number of Threads --> %lu\n",rows);
            displayOutput(mat3_filename, "1");
        }
        printf("\n");
        errorFlag = 0;
        gettimeofday(&start, NULL);
        printf("Method 1:\n");
        printf("----------\n");
        errorFlag|=elementsMethod();
        gettimeofday(&stop, NULL);
        if(!errorFlag){
            // Calculate the method execution time
            printf("Seconds taken --> %lu\n",stop.tv_sec - start.tv_sec);
            printf("(Micro)Seconds taken --> %lu\n",stop.tv_usec - start.tv_usec);
            printf("Number of Threads --> %lu\n",rows*cols);

            displayOutput(mat3_filename, "2");
        }
    }
    return 0;
}
/*
*@brief Create array of threads of size equal to number of rows in output array
*       Give each thread the task of generating a row in the output array
*       wait for running threads to finish their tasks
*/
int rowsMethod(){
        pthread_t thread[rows];
        int errorFlag = 0; // flag to inidicate if an error is encountered
        int i,j,index = 0;
        for(i = 0;i<rows;i++){
            if(pthread_create(&thread[i],NULL,matrixMultiplyR,(void*)i)!=0){
                perror("ERROR:Cannot Create Thread");
                errorFlag = 1;
                break;
            }
            if((i+1)%200==0){
                for(;index<i;index++){
                    pthread_join(thread[index],NULL);
                }
            }

        }
        for(j = index;j<i;j++){
            pthread_join(thread[j],NULL);
        }
        return errorFlag;

}
/*
*@brief Create array of threads of size equal to #of rows*columns in output array
*       Give each thread the task of generating an element in the output array
*       wait for running threads to finish their tasks
*/
int elementsMethod(){
        pthread_t thread[rows*cols];
        int i,j,e = 0;
        int errorFlag = 0; // flag to inidicate if an error is encountered
        int index = 0;
        for(i = 0;i<rows;i++){
            for(j = 0;j<cols;j++){
                struct threadData *data = (struct threadData *)malloc(sizeof(struct threadData));
                data->i = i;
                data->j = j;
               if(pthread_create(&thread[e],NULL,matrixMultiplyE,data)!=0){
                    perror("ERROR:Cannot Create Thread");
                    errorFlag = 1;
                    break;
                }
                e++;
                if(e%200==0){
                    int k;
                    for(index = index;index<e;index++){
                            pthread_join(thread[index],NULL);

                    }
                }
            }
            if(errorFlag){
                break;
            }
        }
        for(i = index;i<e;i++){
            pthread_join(thread[i],NULL);
        }
        return errorFlag;

}
/*
void printMatrix(){
        int i;
        for(i = 0;i<rows;i++){
            int j;
            for(j = 0;j<cols;j++){
                printf("%.5lf ",matOut[i][j]);
            }
            printf("\n");
        }
}
*/

/*
*@brief Create the output file
*@param name of the file & the id of the method
*/
void displayOutput(char *fileName,char *methodId){
    char *tempFilename;

    // set output file name filename+mathodId+".out"
    tempFilename = malloc(sizeof(char)*MAX_FILENAME_SIZE);

    strcpy(tempFilename,fileName);
    strcat(tempFilename,methodId);
    strcat(tempFilename,".out");

    //Open File
    FILE *fptr = fopen(tempFilename,"w");

    // Cannot Open File
    if(fptr==NULL){
        perror("ERROR:");
        return;
    }
    int i;
        for(i = 0;i<rows;i++){
            int j;
            for(j = 0;j<cols;j++){
                fprintf(fptr,"%.5lf",matOut[i][j]);
                if(j<cols-1){
                fprintf(fptr,"\t");

                }
            }
            fprintf(fptr,"\n");
        }
        fclose(fptr);

}
void* matrixMultiplyR(void* id){
    int i = (int)id;
    int j;
    for(j = 0;j < cols;j++){
        int k;

        matOut[i][j] = 0.0;
        for(k = 0;k<rc;k++){
            matOut[i][j]+=(double)((double)mat1[i][k]*(double)mat2[k][j]);
        }
    }
    pthread_exit(NULL);
    return 0;
}

void* matrixMultiplyE(void* data){

    struct threadData *element = data;
    int k;
    matOut[element->i][element->j] = 0.0;
    for(k = 0;k<rc;k++){
        matOut[element->i][element->j]+=(double)mat1[element->i][k]*(double)mat2[k][element->j];
    }
    pthread_exit(NULL);

    return 0;
}

/*
*@brief Read Matrix of given from file
*@param name of the file & the id of the matrix
*/
int getMatrix(char *filename, int matrixId){
    FILE* ptr ;
    ptr = fopen(filename,"r");
    if(ptr == NULL){
        perror("Error");
        return 1;
    }
    int n,m;
    char *line = (char*)malloc(MAX_FILENAME_SIZE * sizeof(char));;
    if(fgets(line, MAX_FILENAME_SIZE, ptr)!=NULL){
    	sscanf(line,"row=%d col=%d",&n,&m);
    }
    if(matrixId == 1){
        allocateMatrix(&mat1, n, m);
    }else{
        allocateMatrix(&mat2, n, m);
    }
    int i = 0;
    int j = 0;

    int errorFlag = 0;
    while(fgets(line, MAX_FILENAME_SIZE, ptr)!=NULL){
        if(i>=rows){
            errorFlag = 1;
            break;
        }
        char *token = strtok(line, "\t");
        j = 0;
        while(token!=NULL){

            if(j>=cols){
                errorFlag=1;
                break;
            }
            if(matrixId == 1){

                mat1[i][j] = atof(token);
             }else{

                mat2[i][j] = atof(token);

             }
            token = strtok(NULL, "\t");
            j++;
        }
        if(j!=cols){
            errorFlag = 1;
            break;
        }
        i++;

    }
    if(i!=rows){
        errorFlag = 1;
    }
    if(errorFlag){
        perror("Error:Input Format error");
        return 1;
    }
    if(rows==-1){
        rows = n;
        rc = m;
        if(rows==0){
            perror("Error:Zero Dimension Size");
            return 1;
        }
    }else if(cols==-1){
        if(rc!=n){
            perror("Error:Cannot multiply Matrices ");
            perror("[Number of columns in First Matrix != Number of rows in Second Matrix]");
            return 1;
        }
        cols = m;
        if(cols==0){
            perror("Error:Zero Dimension Size");
            return 1;
        }
    }
    return 0;
}
void allocateMatrix(double***ptr,int n,int m){
    *ptr = (double**)malloc(n*sizeof(double*));
    int i;
    for(i = 0;i<n;i++){
        (*ptr)[i] = malloc(m*sizeof(double));
    }

}
/*
*@brief helper methof to remove extensions from filename
*@param name of the file
*/
void removeExtension(char *filename){
    int i = 0;
    int lastDotIndex = -1;
    int len = 0;
    for(i = 0;filename[i]!=NULL;i++){
        if(filename[i]=='.'){
            lastDotIndex = i;
        }
        len++;
    }
    char *txtExtention = ".txt";
    char *outExtention = ".out";

    int extension = 0;
    int index = 0;
    for(i = lastDotIndex;i>0 && i<len && index<4;i++){
        if(txtExtention[index]==filename[i]){
            if(i==len-1 && index==3){
                extension = 1;
            }
        }
        index++;

    }
    if(!extension){
        index = 0;
        for(i = lastDotIndex;i>0 && i<len && index<4;i++){
            if(outExtention[index]==filename[i]){
                if(i==len-1 && index==3){
                    extension = 1;
                }
            }
            index++;

        }
    }
    if(extension){
        filename[lastDotIndex]= '\0';
    }

}
