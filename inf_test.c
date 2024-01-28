#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <zlib.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
   FILE *F;
   FILE *tmp;
   
   F=fopen("/usr/local/Hercules/service_E440/Trends/217.SHT","r");
   if (!F)
   {
    printf("cannot open *.SHT !");
    return 0;
   }    
   tmp=fopen("trend.tmp","w+");
   if (!tmp)
   {
    printf("cannot open trend.tmp !");
    return 0;
   }       
   if (inf(F,tmp)==Z_OK)
    printf("OK!");
   else    
    printf("Error!");
   fclose(F);
   fclose(tmp);

  return 0;
}
