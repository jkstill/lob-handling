#include <stdlib.h>
#include <stdio.h>

// int negative(char* db, int n)

char* mygetenv(const char* env)
{
        return getenv(env);
}


int main() {
   // Name of the environment variable (e.g., PATH)
   const char *name = "PATH";
   // Get the value associated with the variable
   //const char *env_p = getenv(name);
   const char *env_p = mygetenv(name);
   if(env_p){
      printf("Your %s is %s\n", name, env_p);
   }
   return 0;
}
