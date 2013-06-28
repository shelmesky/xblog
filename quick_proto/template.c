#include <stdio.h>
#include <stdlib.h>
#include <ctemplate.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    TMPL_varlist *vl, *mainlist;
    TMPL_loop *loop;
    
    char *ptr;
    size_t size;
    FILE *out;
    
    loop = NULL;
    
    vl = TMPL_add_var(0, "row", "one", "user", "Bill", 0);
    loop = TMPL_add_varlist(loop, vl);
    
    vl = TMPL_add_var(0, "row", "two", "user", "Susan", 0);
    loop = TMPL_add_varlist(loop, vl);
    
    TMPL_add_varlist(loop, TMPL_add_var(0, "row", "three", "user", "Jane", 0));
    
    mainlist = TMPL_add_loop(0, "myloop", loop);
    
    
    out = open_memstream(&ptr, &size);
    if(out == NULL){
        perror("open_memstream");
    }
    
    
    TMPL_write("index.html", 0, 0, mainlist, out, stderr);
    
    char buf[4096]; 
    
    TMPL_free_varlist(mainlist);
    fclose(out);
    free(ptr);
    
    return 0;
    
}