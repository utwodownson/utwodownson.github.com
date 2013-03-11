#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char input[72];
    int a[26];
    int max = 0;
    int length = 0;

    for (int i = 0; i < 26; ++i)
        a[i] = 0;    
    for (int i = 0; i < 4; ++i)
    {
        gets(input); 
        length = strlen(input);
        for (int j = 0; j < length; ++j)
        {
            if (((input[j] - 'A') >= 0) && ((input[j] - 'A') <= 26))
            {
                a[input[j] - 'A']++; 
                if (max < a[input[j] - 'A'])  
                    max = a[input[j] - 'A'];
            } 
        }
    }
    for (int i = max; i > 0; --i)
    {
        for (int j = 0; j < 26; ++j)
        {
            if (a[j] == i)
            {
                printf("*");
                if (j != 26)
                    printf(" ");      
                a[j]--;
            }    
            else 
                printf("  ");     
        }   
        printf("\n"); 
    }
    printf("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
}
