#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    FILE *fp = fopen("cpy.txt", "r");
    FILE *output = fopen("output.txt", "w");

    if (!fp||!output) {
        printf("ERROR opening input file orig.txt\n");
        exit(0);
    }
    double sum = 0;
    double temp;

    while(fscanf(fp, "%lf\n",&temp)!=EOF)
        sum+=temp;

    FILE *fp2 = fopen("ref.txt", "r");
    if (!fp2||!output) {
        printf("ERROR opening input file orig.txt\n");
        exit(0);
    }
    double sum2 = 0;

    while(fscanf(fp2, "%lf",&temp)!=EOF)
        sum2+=temp;

    printf("insert %lf %lf\n",sum,sum2);
    sum/=100;
    sum2/=100;

    fprintf(output,"insert %lf %lf\n",sum,sum2);
    fclose(fp);
    fclose(fp2);


}
