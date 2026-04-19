#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#define MAX 4
struct stud_data{
   char name[30];
   int rollno;
   float marks;
   char grade;
};
int main(){
   int my_rank,comm_sz;
   struct stud_data s[4];
   struct stud_data local_student;
   MPI_Init(NULL,NULL);
   MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
   MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
   MPI_Datatype MPI_Student;
   int i=0;
   int arr_of_blocklen[4]={30,1,1,1};
   MPI_Datatype arr_of_types[4]={MPI_CHAR, MPI_INT, MPI_FLOAT, MPI_CHAR};
   MPI_Aint arr_of_disp[4]={0};
   arr_of_disp[0]=offsetof(struct stud_data,name);
   arr_of_disp[1]=offsetof(struct stud_data,rollno);
   arr_of_disp[2]=offsetof(struct stud_data,marks);
   arr_of_disp[3]=offsetof(struct stud_data,grade);
   MPI_Type_create_struct(4,arr_of_blocklen,arr_of_disp,arr_of_types,&MPI_Student);
   MPI_Type_commit(&MPI_Student);
   if(my_rank==0){

      FILE* fin = fopen("Student.txt", "r");
    if (fin == NULL) {
        printf("Error: Could not open input.txt\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for(i = 0; i < MAX; i++) {
        // Reads name, rollno, and marks from file
        if (fscanf(fin, "%s %d %f", s[i].name, &s[i].rollno, &s[i].marks) != 3) {
            printf("Error reading data for student %d\n", i);
            break;
        }
    }
    fclose(fin);

   }
   MPI_Scatter(s,1,MPI_Student,&local_student,1,MPI_Student,0,MPI_COMM_WORLD);
   if(local_student.marks>=90)
      local_student.grade='A';
   else if(local_student.marks>=80)
      local_student.grade='B';
   else if(local_student.marks>=70)
      local_student.grade='C';
   else if(local_student.marks>=60)
      local_student.grade='D';
   else
      local_student.grade='F';
   MPI_Gather(&local_student,1,MPI_Student,s,1,MPI_Student,0,MPI_COMM_WORLD);
   if(my_rank==0){
      FILE* fp=fopen("grade.txt","w");
      for(int i=0;i<MAX;i++){
         fprintf(fp,"%s\t%d\t%2f\t%c\n",s[i].name,s[i].rollno,s[i].marks,s[i].grade);
      }
      fclose(fp);
   }
   MPI_Type_free(&MPI_Student);
   MPI_Finalize();
   return 0;
}
/*Alice 101 85.5
Bob 102 92.0
Charlie 103 67.5
David 104 74.0
[23bcs012@mepcolinux ex4]$cat grade.txt
Alice   101     85.500000       B
Bob     102     92.000000       A
Charlie 103     67.500000       D
David   104     74.000000       C*/
