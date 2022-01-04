
main() {

    int A[7];
    int j;
    int flag;                             /* set flag to true to begin first pass */
    int temp;                          /* holding variable */

    A[0]=5;
    A[1]=4;
    A[2]=3;
    A[3]=2;
    A[4]=1;
    A[5]=0;

    flag = 1;
    while( flag  == 1 )
     {
        flag = 0;                   /* set flag to false awaiting a possible swap */
        for( j=0;  j < 6;  j++ )
        {
             if (A[ j ] > A[j+1] )   /*  test for ascending sort */
             {   
                 temp = A[ j ];      /* swap elements */
                 A[ j ] = A[ j+1 ];
                 A[ j+1 ] = temp;
                 flag = 1;               /* shows a swap occurred */
              }
        }
    }
} 
