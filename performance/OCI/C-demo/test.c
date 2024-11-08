#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include <oci.h>
//#include <ociextp.h>

// int negative(char* db, int n)
int negative(int n)
{
        return -1*n;
}

char* mygetenv(const char* env)
{
        return getenv(env);
}


float mybench(int n_iterations, int i) {
    float result;

    // Calculate the equivalent of r = mod(n_iterations, i) + sqrt(i) * log(i, n_iterations);
    result = fmod(n_iterations, i) + sqrt(i) * (log(i) / log(n_iterations));

    return result;
}

void log_env_vars(const char* env_var) {
    FILE *fp = fopen("/tmp/extproc_env.log", "w");
    if (fp != NULL) {
        const char *value = getenv(env_var);
        if (value != NULL) {
            fprintf(fp, "Value of %s: %s\n", env_var, value);
        } else {
            fprintf(fp, "%s not found\n", env_var);
        }
        fclose(fp);
    }
}

