#include <stdio.h>
#include <stdlib.h>
#include <ctemplate.h>
#include <unistd.h>
#include <string.h>


typedef struct template_result_s {
	char * result;
	int size;
} template_result_t;


int run(void)
{
    TMPL_varlist *vl, *mainlist;
    TMPL_loop *loop;

    
    char *ptr;
    size_t size;
    FILE *out;
    
    loop = NULL;
    
	// add data to loop
    vl = TMPL_add_var(0, "row", "one", "user", "Bill", 0);
    loop = TMPL_add_varlist(loop, vl);
    
    vl = TMPL_add_var(0, "row", "two", "user", "Susan", 0);
    loop = TMPL_add_varlist(loop, vl);
    
    TMPL_add_varlist(loop, TMPL_add_var(0, "row", "three", "user", "Jane", 0));

	// add data to vl1
	mainlist = TMPL_add_var(0, "var1", "value1", 0);
    
	// add data to mainlist
    mainlist = TMPL_add_loop(mainlist, "myloop", loop);
    
    
    out = open_memstream(&ptr, &size);
    if(out == NULL){
        perror("open_memstream");
    }
    
    
    TMPL_write("default.html", 0, 0, mainlist, out, stderr);
    
    char buf[4096]; 
	int read_ret;
	read_ret = fread(buf, 4096, 1, out);
	fprintf(stderr, "length: %u\n%s", (unsigned int)strlen(buf), buf);
    
    TMPL_free_varlist(mainlist);
    fclose(out);
    free(ptr);
    
    return 0;
}
