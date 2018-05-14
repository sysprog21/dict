#include<stdio.h>
#include<string.h>


int main()
{
    int wordcount=0;

    char word[50];
    char def[999];
    char buf[999];

    FILE *file;
    file = fopen("../dictionary.txt", "r");
    if (file) {

        while(fgets(buf,999,file))
            if(strlen(buf)>1) {
                ++wordcount;
                sscanf(buf,"%s\t%s",word,def);
//			printf("word:  %s,def:  %s\n",word,def);
            }
        fclose(file);
        printf("number of word:%d\n",wordcount);
    }
    return 0;
}




