/*
 *The structure of Priority sequence to store the seeds by which we can
 *get the largest one in the sequence each time.
 * Copyright 1997, 1999-2003 John-Mark Gurney.
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *	$Id: fibpriv.h,v 1.12 2003/01/14 10:11:30 jmg Exp $
 *	$Id: fib.c,v 1.31 2003/01/14 10:11:30 jmg Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/time.h>
#include <time.h>
#include <getopt.h>
typedef int (*voidcmp)(void *, void *);
static const int HEAP_SIZE = 20000000;
#define	fhe_destroy(x) free((x))
#define INT_BITS (sizeof(int) * 8)
#define bool unsigned char
/*#define NULL 0*/
#define TRUE 1
#define FALSE 0
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define ABS(x) ((x)>0?(x):-(x))
#define AllocVar(pt) (pt = xmalloc(sizeof(*pt)))
#define AllocArray(pt, size) (pt = xmalloc(sizeof(*pt) * (size)))
#define ReAllocArray(pt, size) (pt = xrealloc(pt, sizeof(*pt) * (size)))
#define sameString(a, b) (strcmp((a), (b))==0)
#define internalErr() errAbort("Internal error %s %d", __FILE__, __LINE__)
#define LABEL_LEN 64 
#define dsFree free
#define dsClear(pds) ((pds)->top = -1)
#define dsSize(pds) ((pds)->top + 1)
#define dsItem(pds, j) ((pds)->items[j])
#define MAXC 100000
#define swap(type, a, b)	\
	do{			\
		type c;	\
		c = a;	\
		a = b;	\
		b = c;	\
	}while(0)	\

struct fibheap
{
	int (*fh_cmp_fnct)(void *, void *);
	int fh_n;
	int fh_Dl;
	struct fibheap_el **fh_cons;
	struct fibheap_el *fh_min;
	struct fibheap_el *fh_root;
	void *fh_neginf;
	int fh_keys : 1;
};
struct fibheap_el
{
	int fhe_degree;
	int fhe_mark;
	struct fibheap_el *fhe_p;
	struct fibheap_el *fhe_child;
	struct fibheap_el *fhe_left;
	struct fibheap_el *fhe_right;
	int fhe_key;
	void *fhe_data;
};
int ceillog2(unsigned int a)
{
	int oa, i, b;
	oa = a;
	b = INT_BITS / 2;
	i = 0;
	while (b){
		i = (i << 1);
		if(a >= (1 << b)){
			a /= (1 << b);
			i = i | 1;
		}else  a &= (1 << b) - 1;
		b /= 2;
	}
	if((1 << i) == oa)  return i;
	else  return i + 1;
}
#ifdef FH_STATS
	int fh_maxn(struct fibheap *h)  {return h->fh_maxn;}
	int fh_ninserts(struct fibheap *h)  {return h->fh_ninserts;}
	int fh_nextracts(struct fibheap *h)  {return h->fh_nextracts;}
#endif
void *fh_min(struct fibheap *h)
{
	if(h->fh_min == NULL)  return NULL;
	return h->fh_min->fhe_data;
}
void *fh_replacekeydata(struct fibheap *h, struct fibheap_el *x, int key, void *data);
int fh_compare(struct fibheap *h, struct fibheap_el *a, struct fibheap_el *b)
{
	if(h->fh_keys){
		if(a->fhe_key < b->fhe_key)  return -1;
		if(a->fhe_key == b->fhe_key)  return 0;
		return 1;
	} else  return h->fh_cmp_fnct(a->fhe_data, b->fhe_data);
}
int fh_comparedata(struct fibheap *h, int key, void *data, struct fibheap_el *b)
{
	struct fibheap_el a;
	a.fhe_key = key;
	a.fhe_data = data;
	return fh_compare(h, &a, b);
}
void *fh_replacedata(struct fibheap *h, struct fibheap_el *x, void *data)  {return fh_replacekeydata(h, x, x->fhe_key, data);}
int fh_replacekey(struct fibheap *h, struct fibheap_el *x, int key)
{
	int ret;
	ret = x->fhe_key;
	(void)fh_replacekeydata(h, x, key, x->fhe_data);
	return ret;
}
void fhe_insertafter(struct fibheap_el *a, struct fibheap_el *b)
{
	if(a == a->fhe_right){
		a->fhe_right = b;
		a->fhe_left = b;
		b->fhe_right = a;
		b->fhe_left = a;
	}else{
		b->fhe_right = a->fhe_right;
		a->fhe_right->fhe_left = b;
		a->fhe_right = b;
		b->fhe_left = a;
	}
}
void fh_insertrootlist(struct fibheap *h, struct fibheap_el *x)
{
	if(h->fh_root == NULL){
		h->fh_root = x;
		x->fhe_left = x;
		x->fhe_right = x;
		return;
	}
	fhe_insertafter(h->fh_root, x);
}
struct fibheap_el *fhe_remove(struct fibheap_el *x)
{
	struct fibheap_el *ret;
	if(x == x->fhe_left)  ret = NULL;
	else  ret = x->fhe_left;
	if(x->fhe_p != NULL && x->fhe_p->fhe_child == x)  x->fhe_p->fhe_child = ret;
	x->fhe_right->fhe_left = x->fhe_left;
	x->fhe_left->fhe_right = x->fhe_right;
	x->fhe_p = NULL;
	x->fhe_left = x;
	x->fhe_right = x;
	return ret;
}
void fh_checkcons(struct fibheap *h)
{
	int oDl;
	if(h->fh_Dl == -1 || h->fh_n > (1 << h->fh_Dl)){
		oDl = h->fh_Dl;
		if((h->fh_Dl = ceillog2(h->fh_n) + 1) < 8)  h->fh_Dl = 8;
		if(oDl != h->fh_Dl)  h->fh_cons = (struct fibheap_el **)realloc(h->fh_cons, sizeof *h->fh_cons * (h->fh_Dl + 1));
		if(h->fh_cons == NULL) abort();
	}
}
void fhe_insertbefore(struct fibheap_el *a, struct fibheap_el *b)  {fhe_insertafter(a->fhe_left, b);}
void fh_heaplink(struct fibheap *h, struct fibheap_el *y, struct fibheap_el *x)
{
	if(x->fhe_child == NULL)  x->fhe_child = y;
	else  fhe_insertbefore(x->fhe_child, y);
	y->fhe_p = x;
	x->fhe_degree++;
	y->fhe_mark = 0;
}
void fh_removerootlist(struct fibheap *h, struct fibheap_el *x)
{
	if(x->fhe_left == x)  h->fh_root = NULL;
	else  h->fh_root = fhe_remove(x);
}
void fh_consolidate(struct fibheap *h)
{
	struct fibheap_el **a, *w, *y, *x;
	int i, d, D;
	fh_checkcons(h);
	D = h->fh_Dl + 1;
	a = h->fh_cons;
	for(i = 0; i < D; i++)  a[i] = NULL;
	while((w = h->fh_root) != NULL){
		x = w;
		fh_removerootlist(h, w);
		d = x->fhe_degree;
		while(a[d] != NULL){
			y = a[d];
			if(fh_compare(h, x, y) > 0)  swap(struct fibheap_el *, x, y);
			fh_heaplink(h, y, x);
			a[d] = NULL;
			d++;
		}
		a[d] = x;
	}
	h->fh_min = NULL;
	for(i = 0; i < D; i++)
		if(a[i] != NULL){
			fh_insertrootlist(h, a[i]);
			if(h->fh_min == NULL || fh_compare(h, a[i], h->fh_min) < 0)  h->fh_min = a[i];
		}
}
struct fibheap_el *fh_extractminel(struct fibheap *h)
{
	struct fibheap_el *ret, *x, *y, *orig;
	ret = h->fh_min;
	orig = NULL;
	for(x = ret->fhe_child; x != orig && x != NULL;){
		if(orig == NULL)  orig = x;
		y = x->fhe_right;
		x->fhe_p = NULL;
		fh_insertrootlist(h, x);
		x = y;
	}
	fh_removerootlist(h, ret);
	h->fh_n--;
	if(h->fh_n == 0)  h->fh_min = NULL;
	else {
		h->fh_min = ret->fhe_right;
		fh_consolidate(h);
	}
#ifdef FH_STATS
	h->fh_nextracts++;
#endif
	return ret;
}
void fh_deleteel(struct fibheap *h, struct fibheap_el *x)
{
	void *data;
	int key;
	data = x->fhe_data;
	key = x->fhe_key;
	if(!h->fh_keys)  fh_replacedata(h, x, h->fh_neginf);
	else  fh_replacekey(h, x, INT_MIN);
	if(fh_extractminel(h) != x)  abort();
	x->fhe_data = data;
	x->fhe_key = key;
}
void fh_insertel(struct fibheap *h, struct fibheap_el *x)
{
	fh_insertrootlist(h, x);
	if(h->fh_min == NULL || (h->fh_keys ? x->fhe_key < h->fh_min->fhe_key : h->fh_cmp_fnct(x->fhe_data, h->fh_min->fhe_data) < 0))  h->fh_min = x;
	h->fh_n++;
#ifdef FH_STATS
	if(h->fh_n > h->fh_maxn)  h->fh_maxn = h->fh_n;
	h->fh_ninserts++;
#endif
}
void fh_cut(struct fibheap *h, struct fibheap_el *x, struct fibheap_el *y)
{
	fhe_remove(x);
	y->fhe_degree--;
	fh_insertrootlist(h, x);
	x->fhe_p = NULL;
	x->fhe_mark = 0;
}
void fh_cascading_cut(struct fibheap *h, struct fibheap_el *y)
{
	struct fibheap_el *z;
	while((z = y->fhe_p) != NULL){
		if(y->fhe_mark == 0){
			y->fhe_mark = 1;
			return;
		}else{
			fh_cut(h, y, z);
			y = z;
		}
	}
}
void *fh_replacekeydata(struct fibheap *h, struct fibheap_el *x, int key, void *data)
{
	void *odata;
	int okey, r;
	struct fibheap_el *y;
	odata = x->fhe_data;
	okey = x->fhe_key;
	if((r = fh_comparedata(h, key, data, x)) > 0){
		abort();
		fh_deleteel(h, x);
		x->fhe_data = data;
		x->fhe_key = key;
		fh_insertel(h, x);
		return odata;
	}
	x->fhe_data = data;
	x->fhe_key = key;
	if(r == 0)  return odata;
	y = x->fhe_p;
	if(h->fh_keys && okey == key)  return odata;
	if(y != NULL && fh_compare(h, x, y) <= 0){
		fh_cut(h, x, y);
		fh_cascading_cut(h, y);
	}
	if(fh_compare(h, x, h->fh_min) <= 0)  h->fh_min = x;
	return odata;
}
void fh_initheap(struct fibheap *new)
{
	new->fh_cmp_fnct = NULL;
	new->fh_neginf = NULL;
	new->fh_n = 0;
	new->fh_Dl = -1;
	new->fh_cons = NULL;
	new->fh_min = NULL;
	new->fh_root = NULL;
	new->fh_keys = 0;
#ifdef FH_STATS
	new->fh_maxn = 0;
	new->fh_ninserts = 0;
	new->fh_nextracts = 0;
#endif
}
struct fibheap *fh_makekeyheap()
{
	struct fibheap *n;
	if((n = malloc(sizeof *n)) == NULL)  return NULL;
	fh_initheap(n);
	n->fh_keys = 1;
	return n;
}
struct fibheap *fh_makeheap()
{
	struct fibheap *n;
	if((n = malloc(sizeof *n)) == NULL)  return NULL;
	fh_initheap(n);
	return n;
}
void fh_destroyheap(struct fibheap *h)
{
	h->fh_cmp_fnct = NULL;
	h->fh_neginf = NULL;
	if(h->fh_cons != NULL)  free(h->fh_cons);
	h->fh_cons = NULL;
	free(h);
}
struct fibheap *fh_union(struct fibheap *ha, struct fibheap *hb)
{
	struct fibheap_el *x;
	if(ha->fh_root == NULL || hb->fh_root == NULL){
		if(ha->fh_root == NULL){
			fh_destroyheap(ha);  return hb;
		} else {
			fh_destroyheap(hb);  return ha;
		}
	}
	ha->fh_root->fhe_left->fhe_right = hb->fh_root;
	hb->fh_root->fhe_left->fhe_right = ha->fh_root;
	x = ha->fh_root->fhe_left;
	ha->fh_root->fhe_left = hb->fh_root->fhe_left;
	hb->fh_root->fhe_left = x;
	ha->fh_n += hb->fh_n;
	if(fh_compare(ha, hb->fh_min, ha->fh_min) < 0)  ha->fh_min = hb->fh_min;
	fh_destroyheap(hb);
	return ha;
}
voidcmp fh_setcmp(struct fibheap *h, voidcmp fnct)
{
	voidcmp oldfnct;
	oldfnct = h->fh_cmp_fnct;
	h->fh_cmp_fnct = fnct;
	return oldfnct;
}
void *fh_setneginf(struct fibheap *h, void *data)
{
	void *old;
	old = h->fh_neginf;
	h->fh_neginf = data;
	return old;
}
void fh_deleteheap(struct fibheap *h)
{
	while (h->fh_min != NULL)  fhe_destroy(fh_extractminel(h));
	fh_destroyheap(h);
}
void fhe_initelem(struct fibheap_el *e)
{
	e->fhe_degree = 0;
	e->fhe_mark = 0;
	e->fhe_p = NULL;
	e->fhe_child = NULL;
	e->fhe_left = e;
	e->fhe_right = e;
	e->fhe_data = NULL;
}
struct fibheap_el *fhe_newelem()
{
	struct fibheap_el *e;
	if((e = malloc(sizeof *e)) == NULL)  return NULL;
	fhe_initelem(e);
	return e;
}
struct fibheap_el *fh_insertkey(struct fibheap *h, int key, void *data)
{
	struct fibheap_el *x;
	if((x = fhe_newelem()) == NULL)  return NULL;
	x->fhe_data = data;
	x->fhe_key = key;
	fh_insertel(h, x);
	return x;
}
int fh_minkey(struct fibheap *h)
{
	if(h->fh_min == NULL)  return INT_MIN;
	return h->fh_min->fhe_key;
}
struct fibheap_el *fh_insert(struct fibheap *h, void *data)
{
	struct fibheap_el *x;
	if((x = fhe_newelem()) == NULL)  return NULL;
	x->fhe_data = data;
	fh_insertel(h, x);
	return x;
}
void *fh_extractmin(struct fibheap *h)
{
	struct fibheap_el *z;
	void *ret;
	ret = NULL;
	if(h->fh_min != NULL){
		z = fh_extractminel(h);
		ret = z->fhe_data;
#ifndef NO_FREE
		fhe_destroy(z);
#endif
	}
	return ret;
}
void *fh_delete(struct fibheap *h, struct fibheap_el *x)
{
	void *k;
	k = x->fhe_data;
	if(!h->fh_keys)  fh_replacedata(h, x, h->fh_neginf);
	else  fh_replacekey(h, x, INT_MIN);
	fh_extractmin(h);
	return k;
}
typedef float continuous;
typedef short discrete;
typedef unsigned short int bits16;
continuous **arr;
discrete **arr_c;
discrete *symbols;
discrete** another_arr_c;
bits16 **profile;
char **genes;
char **conds;
char **sub_genes;
char *atom = NULL;
char** another_genes;
char** another_conds;
char delims[] = "\t\r\n";
bool *sublist;
int rows, cols, sigma, TFindex, sub_genes_row, col_width, another_rows, another_cols, SY_GETLINE = 0, bb[USHRT_MAX];
enum {UP = 1, DOWN = 2, IGNORE = 3};
struct dyStack
{
	int top;
	int items[];
};
typedef struct Edge{
	int gene_one;
	int gene_two;
	int score;
}Edge;
Edge **edge_list;
Edge *edge_ptr;
typedef struct Block{
	struct dyStack *genes;
	struct dyStack *conds;
	int score;
	int block_rows;
	int block_cols;
	int block_rows_pre;
	int cond_low_bound;
	double significance;
	long double pvalue;
}Block;
typedef struct Prog_options{
	char FN[LABEL_LEN];
	char BN[LABEL_LEN];
	char LN[LABEL_LEN];
	bool IS_SWITCH;
	bool IS_DISCRETE;
	bool IS_TFname;
	bool IS_pvalue;
	bool IS_area;
	bool IS_cond;
	bool IS_list;
	int COL_WIDTH;
	int DIVIDED;
	int SCH_BLOCK;
	int RPT_BLOCK;
	double FILTER;
	double QUANTILE;
	double TOLERANCE;
	char TFname[LABEL_LEN];
	FILE* FP;
	FILE* FB;
	FILE* FL;
}Prog_options;
Prog_options* po;
long clock1000()
{
	struct timeval tv;
	static long origSec;
	gettimeofday(&tv, NULL);
	if(origSec == 0) origSec = tv.tv_sec;
	return (tv.tv_sec-origSec)*1000 + tv.tv_usec / 1000;
}
void progress(char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	fprintf(stdout, "\n");
	va_end(args);
}
void verboseDot()
{
	putchar('.');
	fflush(stdout);
}
void err(char *format, ...)
{
	va_list args;
	va_start(args, format);
	fprintf(stderr, "[Error] ");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
}
void errAbort(char *format, ...)
{
	va_list args;
	va_start(args, format);
	fprintf(stderr, "[Error] ");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
	exit(1);
}
void uglyTime(char *label, ...)
{
	static long lastTime = 0;
	long time = clock1000();
	va_list args;
	va_start(args, label);
	if(label != NULL)
	{
		vfprintf(stdout, label, args);
		fprintf(stdout, " [%.3f seconds elapsed]\n", (time - lastTime)/1000.);
	}
	lastTime = time;
	va_end(args);
}
void* xmalloc(int size)
{
	register void* value = malloc(size);
	if (value == NULL)  errAbort("Memory exhausted (xmalloc)");
	return value;
}
void* xrealloc (void* ptr, int size)
{
	register void* value = realloc(ptr, size);
	if(value == NULL)  errAbort("Memory exhausted (xrealloc)");
	return value;
}
struct dyStack *dsNew(int size)
{
	int stackSize = (size+1) * sizeof(int);
	struct dyStack *ds = malloc(stackSize);
	dsClear(ds);
	return ds;
}
void dsPrint(struct dyStack *ds)
{
	int i;
	printf("Stack contains %d elements\n", dsSize(ds));
	for(i=0; i<dsSize(ds); i++)  printf("%d ", ds->items[i]);
	putchar('\n');
}
void dsPush(struct dyStack *ds, int element)  {ds->items[++ds->top] = element;}
bool isInStack(struct dyStack *ds, int element)
{
	bool flag = FALSE;
	int i;
	for(i=0; i<dsSize(ds); i++)  if(ds->items[i]==element)  {flag = TRUE; break;}
	return flag;
}
int dsIntersect(struct dyStack *ds1, struct dyStack *ds2)
{
	int cnt = 0;
	int i;
	for(i=0; i<dsSize(ds1); i++)  if(isInStack(ds2, ds1->items[i]))  cnt++;
	return cnt;
}
void *addSuffix(char *head, char *suffix)
{
	char *ret = NULL;
	int size = strlen(head) + strlen(suffix) + 1;
	AllocArray(ret, size);
	snprintf(ret, size, "%s%s", head, suffix);
	return ret;
}
FILE *mustOpen(const char *fileName, char *mode)
{
	FILE *f;
	if(sameString(fileName, "stdin"))  return stdin;
	if(sameString(fileName, "stdout"))  return stdout;
	if((f = fopen(fileName, mode)) == NULL)
	{
		char *modeName = "";
		if(mode)
		{
			if(mode[0] == 'r')  modeName = " to read";
			else if(mode[0] == 'w')  modeName = " to write";
			else if(mode[0] == 'a')  modeName = " to append";
		}
		errAbort("Can't open %s%s: %s", fileName, modeName, strerror(errno));
	}
	return f;
}
static const char USAGE[] = 
"===================================================================\n\
[Usage]\n\
$ ./delta -i filename [argument list]\n\
===================================================================\n\
[Input]\n\
-i : input file must be one of two tab-delimited formats\n\
  A) continuous data (default, use pre-set discretization (see -q and -r))\n\
     -------------------------------------\n\
     o        cond1    cond2    cond3\n\
     gene1      2.4      3.5     -2.4\n\
     gene2     -2.1      0.0      1.2\n\
     -------------------------------------\n\
  B) discrete data with arbitray classes (turn on -d)\n\
     use '0' for missing or insignificant data\n\
     -------------------------------------\n\
     o        cond1    cond2    cond3\n\
     gene1        1        2        2\n\
     gene2       -1        2        0\n\
     -------------------------------------\n\
-q : use quantile discretization for continuous data\n\
     default: 0.06 (see details in Method section in paper)\n\
-r : the number of ranks as which we treat the up(down)-regulated value\n\
     when discretization\n\
     default: 1\n\
-d : discrete data, where user should send their processed data\n\
     to different value classes, see above\n\
-b : the file to expand in specific environment\n\
-T : to-be-searched TF name, just consider the seeds containing current TF\n\
     default format: B1234\n\
-P : the flag to enlarge current biclsuter by the pvalue constrain\n\
-S : the flag using area as the value of bicluster to determine when stop\n\
-C : the flag using the lower bound of condition number (5 persents of the gene number)\n\
-l : the list of genes out of the input file on which we do bicluster\n\
===================================================================\n\
[Output]\n\
-o : number of blocks to report, default: 100\n\
-f : filtering overlapping blocks,\n\
     default: 1 (do not remove any blocks)\n\
-k : minimum column width of the block,\n\
     default: 5% of columns, minimum 2 columns\n\
-c : consistency level of the block (0.5-1.0], the minimum ratio between the\n\
     number of identical valid symbols in a column and the total number \n\
     of rows in the output\n\
     default: 0.95\n\
-s : expansion flag\n\
===================================================================\n";
static void init_options ()
{
	/* default parameters */
	/* strcpy: Copies the C string pointed by source into the array pointed by destination, including the terminating null character.
	 * To avoid overflows, the size of the array pointed by destination shall be long enough to contain the same C string as source (including the terminating null character), and should not overlap in memory with source
	 */
	strcpy(po->FN, " ");
	strcpy(po->BN, " ");
	strcpy(po->LN, " ");
	strcpy(po->TFname, " ");
	po->IS_DISCRETE = FALSE;
	po->IS_TFname = FALSE;
	po->IS_pvalue = FALSE;
	po->COL_WIDTH = 2;
	po->DIVIDED = 1;
	/*.06 is set as default for its best performance for ecoli and yeast functional analysis*/
	po->QUANTILE = .06;
	po->TOLERANCE = .95;
	po->FP = NULL;
	po->FB = NULL;
	po->RPT_BLOCK = 100;
	po->SCH_BLOCK = 200;
	po->FILTER = 1;
	po->IS_SWITCH = FALSE;
	po->IS_area = FALSE;
	po->IS_cond = FALSE;
	po->IS_list = FALSE;
}
void get_options (int argc, char* argv[])
{
	int op;
	bool is_valid = TRUE;
	/*set memory for the point which is decleared in struct.h*/
	AllocVar(po);
	/*Initialize the point*/
	init_options();
	/*The getopt function gets the next option argument from the argument list specified by the argv and argc arguments. 
	 *Normally these values come directly from the arguments received by main
	 */
	/*An option character in this string can be followed by a colon (:) to indicate that it takes a required argument.
	 *If an option character is followed by two colons (::), its argument is optional
	 *if an option character is followed by no colons, it does not need argument
	 */
	while ((op = getopt(argc, argv, "i:b:q:r:dsf:k:o:c:T:PSCl:h")) >0)
	{
		switch (op)
		{
			/*optarg is set by getopt to point at the value of the option argument, for those options that accept arguments*/
			case 'i': strcpy(po->FN, optarg); break;
			case 'b': strcpy(po->BN, optarg); break;
			/*atof can convert string to double*/
			case 'q': po->QUANTILE = atof(optarg); break;
			/*atoi can convert string to integer*/
			case 'r': po->DIVIDED = atoi(optarg); break;
			case 'd': po->IS_DISCRETE = TRUE; break;
			case 's': po->IS_SWITCH = TRUE; break;
			case 'f': po->FILTER = atof(optarg); break; 
			case 'k': po->COL_WIDTH = atoi(optarg); break;
			case 'c': po->TOLERANCE = atof(optarg); break;
			case 'o':
				po->RPT_BLOCK = atoi(optarg); 
				po->SCH_BLOCK = 2*po->RPT_BLOCK;
				/* ensure enough searching space */
				/*if (po->SCH_BLOCK < 1000) po->SCH_BLOCK = 1000;*/ 
				break;
			/*puts writes the C string pointed by str to stdout and appends a newline character ('\n')*/
			/*exit(0) causes the program to exit with a successful termination
			 *break is normally used to jump to the end of the current block of code
			 *exit is normally used to shut down the current process
			 */
			case 'T': strcpy(po->TFname, optarg); po->IS_TFname = TRUE; break;
			case 'P': po->IS_pvalue = TRUE; break; 
			case 'S': po->IS_area = TRUE; break; 
			case 'C': po->IS_cond = TRUE; break; 
			case 'l': strcpy(po->LN, optarg); po->IS_list =TRUE; break;
			case 'h': puts(USAGE); exit(0); 
			/*if expression does not match any constant-expression, control is transferred to the statement(s) that follow the optional default label*/
			default : is_valid = FALSE;
		}
	}
	/* basic sanity check */
	if (is_valid && po->FN[0] == ' ') 
	{
		/*errAbort("You must specify input file (-i)");*/
		puts(USAGE); 
		exit(0);
	}
	if(is_valid)  po->FP = mustOpen(po->FN, "r");
	if(po->IS_SWITCH) 
	{	
		po->IS_DISCRETE = TRUE; 
		po->FB = mustOpen(po->BN, "r");
	}
	if (po->IS_list)  po->FL = mustOpen(po->LN, "r");
	/* option value range check */
	if((po->QUANTILE >.5) || (po->QUANTILE <= 0))
	{
		err("-q quantile discretization should be (0,.5]");
		is_valid = FALSE;
	}
	if((po->FILTER > 1) || (po->FILTER < 0))
	{
		err("-f overlapping filtering should be [0,1.]");
		is_valid = FALSE;
	}
	if((po->TOLERANCE > 1) || (po->TOLERANCE <= .5))
	{
		err("-c noise ratio should be (.5,1]");
		is_valid = FALSE;
	}
	if(po->COL_WIDTH < 2 && po->COL_WIDTH != -1)
	{
		err("-k minimum column width should be >=2");
		is_valid = FALSE;
	}
	if(po->RPT_BLOCK <= 0)
	{
		err("-n number of blocks to report should be >0");
		is_valid = FALSE;
	}
	if(!is_valid)  errAbort("Type -h to view possible options");

}
/* read_array subroutine prototypes  */
/* extern char** alloc2c (int rows, int cols); */
continuous** alloc2d(int rr, int cc)
{
	continuous** result;
	int i;
	AllocArray(result, rr);
	for (i = 0; i < rr; i++)  AllocArray(result[i], cc);
	return result;
}
discrete** alloc2c(int rr, int cc)
{
	discrete** result;
	int i;
	AllocArray(result, rr);
	for (i = 0; i < rr; i++)  AllocArray(result[i], cc);
	return result;
}
char** alloc2c_c(int rr, int cc)
{
	char** result;
	int i;
	AllocArray(result, rr);
	for (i = 0; i < rr; i++)  AllocArray(result[i], cc);
	return result;
}
void write_imported (const char* stream_nm)
{
	int row, col;
	FILE *fw;
	fw = mustOpen(stream_nm, "w"); 
	fprintf(fw, "o");
	for(col = 0; col < cols; col++)  fprintf(fw,"\t%s",conds[col]);
	fputc('\n',fw);
	for(row = 0; row < rows; row++)
	{
		fprintf(fw, "%s", genes[row]);
		for(col = 0; col < cols; col++)  fprintf(fw, "\t%d", symbols[arr_c[row][col]]);
		fputc('\n', fw);
	}
	progress("Formatted data are written to %s", stream_nm);
	fclose(fw);
}
void get_matrix_size(FILE* fp)
{
	/*size_t is the best type to use if you want to represent sizes of objects. 
	 * Using int to represent object sizes is likely to work on most modern systems, but it isn't guaranteed.
	 */
	size_t n = 0;
	char *line;
	/*getline() reads an entire line, storing the address of the buffer containing the text into *line.
	 *the buffer is null-terminated and includes the newline character, if a newline delimiter was found.
	 */
	if(getline(&line, &n, fp)>=0)
	{
		/*strtok function returns a pointer to the next token in str1, where str2 contains the delimiters that determine the token*/
		atom = strtok(line, delims);
		/*delete the first element in atom because the first element corresponding to description column*/
		atom = strtok(NULL, delims);
		while(atom != NULL)
		{
			/*if we do not set atom = strtok(NULL, delims), here is a infinite loop*/
			atom = strtok(NULL, delims);
			cols++;
		}
	}
	while(getline(&line, &n, fp)>=0)
	{
		atom = strtok(line, delims);
		rows++;
	}
	/*fseed sets the position indicator associated with the stream to a new position defined by adding offset to a reference position specified by origin*/
	fseek(fp, 0, 0);
}
void read_labels (FILE* fp)
{
	int row = 0;
	int col;
	size_t n = 0;
	char *line;
	while(getline(&line, &n, fp)>=0)
	{
		atom = strtok(line, delims);
		/*currently the first element in atom is the gene name of each row when row>=1, the 0 row corresponding to the line of condition names*/
		if(row >= 1) 
		{
			strcpy(genes[row-1], atom);
			/*check if there exist a gene name equals to TFname by -T*/
			if(strcmp (atom, po->TFname)==0)
			{
				TFindex = row-1;
				printf ("%d\n",TFindex);
			}
		}
		/*delete the first element in atom because the first element corresponding to description column*/
		atom = strtok(NULL, delims);
		col = 0;
		while(atom != NULL)
		{
			if(row == 0)  strcpy(conds[col], atom);
			atom = strtok(NULL, delims);
			if(++col == cols)  break;
		}
		if(++row == rows+1)  break;
	}
	fseek(fp, 0, 0);
}
int charset_add(discrete *ar, discrete s)
{
	/*A signed short can hold all the values between SHRT_MIN  and SHRT_MAX inclusive.SHRT_MIN is required to be -32767 or less,SHRT_MAX must be at least 32767*/
	int ps = s + SHRT_MAX;
	if(bb[ps]<0)
	{
		bb[ps] = sigma; 
		ar[sigma++] = s;
	}
	return bb[ps];
}
void init_dis()
{
	int row, col;
	/* store discretized values */
	AllocArray(symbols, USHRT_MAX);
	/* memset sets the first num bytes of the block of memory pointed by ptr to the specified value
	 * memset ( void * ptr, int value, size_t num )*/
	memset(bb, -1, USHRT_MAX*sizeof(*bb));
	/* always add an 'ignore' index so that symbols[0]==0*/
	charset_add(symbols, 0); 
	/*initialize for arr_c*/
	arr_c = alloc2c(rows,cols);
	for(row = 0; row < rows; row++)  for(col = 0; col < cols; col++)  arr_c[row][col] = 0;
}
void read_discrete(FILE* fp)
{
	int row, col, i;
	init_dis();
	/* import data */
	size_t n = 0;
	char *line;
	row = 1;
	/* Skip first line with condition labels */
	SY_GETLINE = getline(&line, &n, fp);
	/* read the discrete data from the second line */
	while(getline(&line, &n, fp)>=0)
	{
		atom = strtok(line, delims);
		/*skip the first column*/
		atom = strtok(NULL, delims);
		col = 0;
		while(atom != NULL)
		{			
			arr_c[row-1][col] = charset_add(symbols, atoi(atom));
			atom = strtok(NULL, delims);
			if(++col == cols)  break;
		}
		if(++row == rows+1)  break;
	}
	/* trim the leading spaceholder */
	printf("Discretized data contains %d classes with charset [ ", sigma);
	for(i=0;i<sigma;i++)  printf("%d ", symbols[i]); printf("]\n");
	fseek(fp, 0, 0);
}
void read_continuous(FILE* fp)
{
	int row, col;
	arr = alloc2d(rows,cols);
	/* import data */
	size_t n = 0;
	char *line;
	row = 1;
	/* ignore header line */
	SY_GETLINE = getline(&line, &n, fp);
	while(getline(&line, &n, fp)>=0)
	{
		atom = strtok(line, delims);
		/*skip the first column*/
		atom = strtok(NULL, delims);
		col = 0;
		while(atom != NULL)
		{
			/*we set all the aplha to ignore value 0*/
			/*Checks if parameter atom is either an uppercase or a lowercase alphabetic letter*/
			if(isalpha(*atom))  arr[row-1][col] = 0.0;
			else  arr[row-1][col] = atof(atom);
			atom = strtok(NULL, delims);
			if(++col == cols)  break;
		}
		if(++row == rows+1) break;
	}
	fseek(fp, 0, 0);
}
void read_list(FILE* fp)
{
	int i=0, j=0;
	sub_genes_row = 0;
	char line[MAXC];
	while(fgets(line,MAXC,fp)!=NULL)
	{
		atom = strtok(line, delims);
		strcpy(sub_genes[sub_genes_row], atom);	
		sub_genes_row++;
	}
	/*update the sub_list*/
	AllocArray(sublist,rows);
	for(i = 0; i<rows; i++)  sublist[i] = FALSE;
	for(i=0; i<sub_genes_row; i++)  for(j=0; j<rows; j++)  if(strcmp (sub_genes[i], genes[j])==0)  sublist[j] = TRUE;
}
continuous quantile_from_sorted_data(const continuous sorted_data[], size_t n, double f)
{
	/*floor function returns the largest integral value less than or equal to x*/
	int i = floor((n-1)*f);
	continuous delta = (n-1)*f-i;
	return (1-delta)*sorted_data[i]+delta*sorted_data[i+1];
}
discrete dis_value(float current, int divided, float *small, int cntl, float *big, int cntu)
{
	int i;
	float d_space = 1.0 / divided;	
	for(i=0; i < divided; i++)
	{		
		if((cntl > 0) && (current <= quantile_from_sorted_data(small, cntl, d_space * (i+1))))  return -i-1;
		if ((cntu > 0) && (current >= quantile_from_sorted_data(big, cntu, 1.0 - d_space * (i+1))))  return i+1;
	}
	return 0;
}
int compare_continuous (const void *a, const void *b)
{
	const continuous *da = a;
	const continuous *db = b;
	/*make qsort in the increasing order*/
	return (*da < *db)?-1:(*da != *db);
}
void discretize (const char* stream_nm)
{
	int row, col, i, cntu, cntl;
	continuous rowdata[cols];
	float big[cols], small[cols], f1,f2,f3, upper, lower;
	FILE *fw;
	fw = mustOpen(stream_nm, "w");
	init_dis();
	for(row = 0; row < rows; row++)
	{
		for(col = 0; col < cols; col++)  rowdata[col] = arr[row][col];
		qsort(rowdata, cols, sizeof *rowdata, compare_continuous);
		f1 = quantile_from_sorted_data(rowdata,cols,1-po->QUANTILE); 
		f2 = quantile_from_sorted_data(rowdata,cols,po->QUANTILE);
		f3 = quantile_from_sorted_data(rowdata,cols,0.5);
		if ((f1-f3)>=(f3-f2))
		{
			upper = 2*f3-f2; 
			lower = f2;
		}
		else
		{
			upper = f1; 
			lower = 2*f3-f1;
		}
		cntu = 0; cntl = 0;
		for(i = 0; i < cols; i++)
		{
			if(rowdata[i] < lower) 
			{ 
				small[cntl] = rowdata[i]; 
				cntl++; 
			}
			if(rowdata[i] > upper) 
			{ 
				big[cntu] = rowdata[i]; 
				cntu++; 
			}
		}	
		for(col = 0; col < cols; col++)  arr_c[row][col] = charset_add(symbols, dis_value(arr[row][col],po->DIVIDED, small, cntl, big, cntu));
		fprintf(fw, "row %s :low=%2.5f, up=%2.5f; %d down-regulated,%d up-regulated\n", genes[row], lower, upper,cntl, cntu);
	}
	progress("Discretization rules are written to %s", stream_nm);
	fclose(fw);
}
/*add write_block subroutine prototypes */
/*
char blockfn[LABEL_LEN];
FILE *fw;
*/
void seed_update(const discrete *s)
{
	int i;
	for(i = 0; i < cols; i++)  profile[i][s[i]]++;
}
/* scan through all columns and identify the set within threshold,
 * "fuzziness" of the block is controlled by TOLERANCE (-c)
*/
void scan_block (struct dyStack *gene_set, Block *b_ptr)
{
	int i, j;
	int block_rows, cur_rows;
	block_rows = cur_rows = dsSize(gene_set);
	int k;
	for (j = 0; j < cols; j++)  for(k=0; k<sigma; k++)  profile[j][k] = 0;
	for(j = 0; j< cur_rows ; j++)  seed_update(arr_c[dsItem(gene_set,j)]);
	int btolerance = ceil(po->TOLERANCE* block_rows);
	for(j = 0; j < cols; j++)  for(i = 1; i < sigma; i++)  if((profile[j][i] >= btolerance))  {dsPush(b_ptr->conds, j); break;}
		/* See if this column satisfies tolerance */
		/* here i start from 1 because symbols[0]=0 */
	b_ptr->block_cols = dsSize(b_ptr->conds);
}
/*************************************************************************/
/* Identified clusters are backtraced to the original data, by
 * putting the clustered vectors together, identify common column
 */
void print_bc (FILE* fw, Block* b, int num)
{	
	int i, j;
	int block_rows, block_cols;
	int num_1=0,num_2=0;	
	/* block height (genes) */
	block_rows = b->block_rows;
	/* block_width (conditions) */
	block_cols = b->block_cols;
	fprintf(fw, "BC%03d\tS=%d\tPvalue:%LG \n", num, block_rows * block_cols, b->pvalue);
	fprintf(fw, " Genes [%d]: ", block_rows);
	for (i=0; i<dsSize(b->genes); i++)
		fprintf(fw, "%s ", genes[dsItem(b->genes, i)]);
	fprintf(fw, "\n");
	fprintf(fw, " Conds [%d]: ", block_cols);
	for(i=0; i<dsSize(b->conds); i++)  fprintf(fw, "%s ", conds[dsItem(b->conds, i)]);
	fprintf(fw, "\n");	
	/* the complete block data output */
	for(i=0; i<dsSize(b->genes); i++)
	{
		fprintf(fw,"%10s:",genes[dsItem(b->genes, i)]);
		for(j=0; j<dsSize(b->conds); j++)
		{
			fprintf(fw, "\t%d", symbols[arr_c[dsItem(b->genes, i)][dsItem(b->conds, j)]]);
			if(i==0)
			{
				if(symbols[arr_c[dsItem(b->genes, i)][dsItem(b->conds, j)]] == 1) num_1++;
				if(symbols[arr_c[dsItem(b->genes, i)][dsItem(b->conds, j)]] == -1) num_2++;
			}
		}
		fputc('\n', fw);
		if(i == b->block_rows_pre -1)  fputc('\n',fw);
	}
	/*printf ("BC%03d: #of 1 and -1 are:\t%d\t%d\n",num,num_1,num_2);
	fputc('\n', fw);*/
}
/*add cluster subroutine prototypes */
/*
struct rowMatch
{
	int row_id;
	int matches;
};*/
/* Initialize seed */
int compare_int (const void *a, const void *b)
{
	const int *da = a;
	const int *db = b;
	return (*da < *db)?-1:(*da != *db);
}
void update_colcand(bool *colcand, discrete *g1, discrete *g2)
{
	int i;
	for(i=0; i< cols; i++)  if(colcand[i] && (g1[i] != g2[i]))  colcand[i] = FALSE;
}
int intersect_row(const bool *colcand, discrete *g1, discrete *g2)
/*caculate the weight of the edge with two vertices g1 and g2*/
{
	int i;
	int cnt = 0;
	for(i=0; i< cols; i++)  if(colcand[i] && (g1[i] == g2[i]) && (g1[i]!=0))  cnt++;
	return cnt;
}
int reverse_row(const bool *colcand, discrete *g1, discrete *g2)
{
/*caculate the negative correlation between g1 and g2*/	
	int i;
	int cnt = 0;
	for (i = 0; i < cols; i++)  if (colcand[i] && (symbols[g1[i]] == -symbols[g2[i]]))  cnt++;
	return cnt;
} 
void seed_current_modify (const discrete *s, bool *colcand, int* cnt, int components)
/* calculate the coverage of any row to the current consensus
 * cnt = # of valid consensus columns
 */
{
	int i, k, flag, n;
	int threshold = ceil(components * po->TOLERANCE);
	discrete ss;
	*cnt = 0;
	for(i=0; i<cols; i++)
	{
		flag = 0; ss = s[i];
		for(k=1; k<sigma; k++)
		{
			n = profile[i][k];
			if(k == ss)  n++;
			if(n >= threshold)
			{
				flag = k; 
				break;
			}
		}
		if(flag)	
		{
			(*cnt)++;
			colcand[i] = TRUE;
		}
	}
}
bool check_seed(Edge *e, Block **bb, const int block_id)
/*check whether current edge can be treat as a seed*/
{
	int profiles[rows];
	int i, b1, b2, b3;
	bool fg = FALSE;
	b1 = b2 = -1;
	for(i = 0; i < block_id; i++)  if(isInStack(bb[i]->genes,e->gene_one) && isInStack(bb[i]->genes, e->gene_two))  return FALSE; 
	for(i = 0; i < rows; i++)  profiles[i] = 0;
	fg = FALSE;	
	for( i = 0; i < block_id; i++)  if(isInStack(bb[i]->genes, e->gene_one))  {fg = TRUE;break;}
	if(fg)  b1 = i;
	fg = FALSE;	
	for( i = 0; i < block_id; i++)
		if(isInStack(bb[i]->genes, e->gene_two)) 
		{ 
			fg = TRUE; 
			break; 
		}
	if(fg)  b2 = i;
	if ( (b1 == -1)||(b2 == -1) )  return TRUE;
	else
	{
		for( i = 0; i < bb[b1]->block_rows; i++)
			profiles[dsItem(bb[b1]->genes,i)]++;
		for( i = 0; i < bb[b2]->block_rows; i++)
			profiles[dsItem(bb[b2]->genes,i)]++;
		for( i = 0; i < rows; i++)  if (profiles[i] > 1)  return FALSE;
		b3 = MAX(bb[b1]->block_cols, bb[b2]->block_cols);
		if ( e->score <b3/* (bb[b1]->block_cols + bb[b2]->block_cols) / 2*/ )  return FALSE;
		else  return TRUE;
	}
	err("never see this message\n");
	return FALSE;
}
long double get_pvalue (continuous a, int b)
{
	int i =0;
	long double one = 1, pvalue=0;
	long double poisson=one/exp(a);
	for(i=0; i<b+300; i++)
	{
		if(i>(b-1))  pvalue=pvalue+poisson;
		else  poisson=poisson*a/(i+1);
	}
	return pvalue;
}
void block_init(Edge *e, Block *b, struct dyStack *genes, struct dyStack *scores, bool *candidates, const int cand_threshold, int *components, struct dyStack *allincluster, long double *pvalues)
{
	int i,score,top;
	int cnt = 0, cnt_all=0, pid=0;
	continuous cnt_ave=0, row_all = rows;
	long double pvalue;
	int max_cnt, max_i;
	int *arr_rows, *arr_rows_b;
	AllocArray(arr_rows, rows);
	AllocArray(arr_rows_b, rows);	
	bool *colcand;
	AllocArray(colcand, cols);
	for (i=0; i< cols; i++) 
		colcand[i] = FALSE;
	discrete *g1, *g2;
	g1 = arr_c[dsItem(genes,0)];
	g2 = arr_c[dsItem(genes,1)];
	for(i=0; i< cols; i++)
		if((g1[i] == g2[i])&&(g1[i]!=0)) 
			colcand[i] = TRUE;
	for(i = 0; i < rows; i++)
	{
		arr_rows[i] = intersect_row(colcand, arr_c[dsItem(genes,0)], arr_c[i]);
		arr_rows_b[i] = arr_rows[i];
	}
	/*we just get the largest 100 rows when we initial a bicluster because we believe that 
	 * the 100 rows can characterize the structure of the bicluster 
	 * btw, it can reduce the time complexity*/
	if(rows > 100)
	{		
		qsort(arr_rows_b, rows, sizeof *arr_rows, compare_int);
		top = arr_rows_b[rows -100];
		for(i = 0; i < rows; i++)
			if(arr_rows[i] < top) 
				candidates[i] = FALSE;
	}
	/*calculate the condition low bound for current seed*/
	int cutoff = floor (0.05*rows);
	b->cond_low_bound = arr_rows_b[rows-cutoff];
	while(*components < rows)
	{
		max_cnt = -1;
		max_i = -1;
		(*components)++;
		cnt_all =0;
		cnt_ave = 0;
		/******************************************************/
		/*add a function of controling the bicluster by pvalue*/
		/******************************************************/
		for(i=0; i< rows; i++)
		{
			if(!candidates[i]) continue;
			if(po->IS_list && !sublist[i]) continue;
			cnt = intersect_row(colcand,arr_c[dsItem(genes,0)],arr_c[i]);
			cnt_all += cnt;
			if(cnt < cand_threshold) 
				candidates[i] = FALSE;
			if(cnt > max_cnt)
			{
				max_cnt = cnt;
				max_i = i;
			}
		}
		cnt_ave = cnt_all/row_all;
		pvalue = get_pvalue (cnt_ave, max_cnt);
		if(po->IS_cond)
		{
			if(max_cnt < po->COL_WIDTH || max_i < 0|| max_cnt < b->cond_low_bound) break;
		}
		else
		{
			if(max_cnt < po->COL_WIDTH || max_i < 0) break;
		}
		if(po->IS_area)  score = *components*max_cnt;
		else  score = MIN(*components, max_cnt);
		if(score > b->score)  b->score = score;
		if(pvalue < b->pvalue)  b->pvalue = pvalue;
		dsPush(genes, max_i);
		dsPush(scores,score);
		pvalues[pid++] = pvalue;
		update_colcand(colcand,arr_c[dsItem(genes,0)], arr_c[max_i]);
		candidates[max_i] = FALSE;
	}
	/*be sure to free a pointer when you finish using it*/
	free(colcand);
}
void print_params(FILE *fw)
{
	char filedesc[LABEL_LEN];
	strcpy(filedesc, "continuous");
	if(po->IS_DISCRETE)  strcpy(filedesc, "discrete");
	fprintf(fw, "# DELTA version %.1f output\n", VER);
	fprintf(fw, "# Datafile %s: %s type\n", po->FN, filedesc);
	fprintf(fw, "# Parameters: -k %d -f %.2f -c %.2f -o %d",
			po->COL_WIDTH, po->FILTER, po->TOLERANCE, po->RPT_BLOCK);
	if(!po->IS_DISCRETE)  fprintf(fw, " -q %.2f -r %d", po->QUANTILE, po->DIVIDED);
	fprintf(fw, "\n\n");
}
int block_cmpr(const void *a, const void *b)  {return ((*(Block **)b)->score - (*(Block **)a)->score);}
/* compare function for qsort, descending by score */
void sort_block_list(Block **el, int n)  {qsort(el, n, sizeof *el, block_cmpr);}
/************************************************************************/
int report_blocks(FILE* fw, Block** bb, int num)
{
	print_params(fw);
	sort_block_list(bb, num);
	int i, j,k;
	/*MIN MAX et al functions can be accessed in struct.h*/
	int n = MIN(num, po->RPT_BLOCK);
	bool flag;
	Block **output;
	AllocArray(output, n);
	Block **bb_ptr = output;
	Block *b_ptr;
	double cur_rows, cur_cols;
	double inter_rows, inter_cols;
	/*double proportion;*/
	/* the major post-processing here, filter overlapping blocks*/
	i = 0; j = 0;
	while (i < num && j < n)
	{
		b_ptr = bb[i];
		cur_rows = b_ptr->block_rows;
		cur_cols = b_ptr->block_cols;
		flag = TRUE;
		k = 0;
		while(k < j)
		{
			inter_rows = dsIntersect(output[k]->genes, b_ptr->genes);
			inter_cols = dsIntersect(output[k]->conds, b_ptr->conds);
			
			if(inter_rows*inter_cols > po->FILTER*cur_rows*cur_cols)
			{
				flag = FALSE; 
				break;
			}
		k++;
		}
		i++;
		if(flag)
		{
			print_bc(fw, b_ptr, j++);
			*bb_ptr++ = b_ptr;
		}
	}
	return j;
}
/************************************************************************/
int cluster(FILE *fw, Edge **el, int n)
{
	int block_id = 0;
	Block **bb;
	int allocated = po->SCH_BLOCK;
	AllocArray(bb, allocated);
	Edge *e;
	Block *b;
	struct dyStack *genes, *scores, *b_genes, *allincluster;
	int i, j, k, components;
	AllocArray(profile, cols);
	for (j = 0; j < cols; j++)  AllocArray(profile[j], sigma);
	genes = dsNew(rows);
	scores = dsNew(rows);
	allincluster = dsNew(rows);
	long double *pvalues;
	AllocArray(pvalues, rows);
	bool *candidates;
	AllocArray(candidates, rows);
	e = *el; 
	i = 0;
	while (i++ < n)
	{	
		e = *el++;
		/* check if both genes already enumerated in previous blocks */
		bool flag = TRUE;
		/* speed up the program if the rows bigger than 200 */
			if(rows > 250)
		{ 
			if(isInStack(allincluster,e->gene_one) && isInStack(allincluster,e->gene_two))  flag = FALSE;
			else if((po->IS_TFname)&&(e->gene_one!= TFindex)&&(e->gene_two!=TFindex))  flag = FALSE;
			else if((po->IS_list)&&(!sublist[e->gene_one] || !sublist[e->gene_two]))  flag =FALSE;
		}
		else  
		{
			flag = check_seed(e, bb, block_id);
			if((po->IS_TFname)&&(e->gene_one!= TFindex)&&(e->gene_two!=TFindex))  flag = FALSE;
			if((po->IS_list)&&(!sublist[e->gene_one] || !sublist[e->gene_two]))  flag = FALSE;
		}
		if(!flag)  continue;
		for(j = 0; j < cols; j++)  for(k = 0; k < sigma; k++)  profile[j][k] = 0;
		/*you must allocate a struct if you want to use the pointers related to it*/
		AllocVar(b);
		/*initial the b->score*/
		b->score = MIN(2, e->score);
		/*initial the b->pvalue*/
		b->pvalue = 1;
		/* initialize the stacks genes and scores */		
		int ii;		
		dsClear(genes);
		dsClear(scores);		
		for(ii = 0; ii < rows; ii ++)
		{
			dsPush(genes,-1);
			dsPush(scores,-1);
		}		
		dsClear(genes);
		dsClear(scores);
		dsPush(genes, e->gene_one);
		dsPush(genes, e->gene_two);
		dsPush(scores, 1);
		dsPush(scores, b->score);
		/* branch-and-cut condition for seed expansion */
		int cand_threshold = floor(po->COL_WIDTH * po->TOLERANCE);
		if(cand_threshold < 2)  cand_threshold = 2;
		/* maintain a candidate list to avoid looping through all rows */		
		for (j = 0; j < rows; j++)  candidates[j] = TRUE;
		candidates[e->gene_one] = candidates[e->gene_two] = FALSE;
		components = 2;
		/* expansion step, generate a bicluster without noise */
		block_init(e, b, genes, scores, candidates, cand_threshold, &components, allincluster, pvalues);
		/* track back to find the genes by which we get the best score*/
		for(k = 0; k < components; k++)
		{
			if (po->IS_pvalue)
				if ((pvalues[k] == b->pvalue) &&(k >= 2) &&(dsItem(scores,k)!=dsItem(scores,k+1))) break;
			if ((dsItem(scores,k) == b->score)&&(dsItem(scores,k+1)!= b->score)) break;
		}
		components = k + 1;
		int ki;
		for (ki=0; ki < rows; ki++)
			candidates[ki] = TRUE;
		for (ki=0; ki < components - 1 ; ki++)
		{
			seed_update(arr_c[dsItem(genes,ki)]);
			candidates[dsItem(genes,ki)] = FALSE;
		}
		candidates[dsItem(genes,k)] = FALSE;
		genes->top = k ;
		int cnt = 0;
		bool *colcand;
		AllocArray(colcand, cols);
		for(ki = 0; ki < cols; ki++)  colcand[ki] = FALSE;
		/* add columns satisfy the conservative r */ 
		seed_current_modify(arr_c[dsItem(genes,k)], colcand, &cnt, components);
		/* add some new possible genes */
		int m_cnt;
		for(ki = 0; ki < rows; ki++)
		{
			if(po->IS_list && !sublist[ki]) continue;
			m_cnt = intersect_row(colcand, arr_c[dsItem(genes,0)], arr_c[ki]);
			if(candidates[ki] && (m_cnt >= floor(cnt* po->TOLERANCE)))
			{
				dsPush(genes,ki);
				components++;
				candidates[ki] = FALSE;
			}
		}
		b->block_rows_pre = components;
		/* add genes that negative regulated to the consensus */
		for(ki = 0; ki < rows; ki++)
		{
			if(po->IS_list && !sublist[ki]) continue;
			m_cnt = reverse_row(colcand, arr_c[dsItem(genes,0)], arr_c[ki]);
			if(candidates[ki] && (m_cnt >= floor(cnt * po->TOLERANCE)))
			{
				dsPush(genes,ki);
				components++;
				candidates[ki] = FALSE;
			}
		}
		free(colcand);
		/* save the current cluster*/
		b_genes = dsNew(b->block_rows_pre);
		for (ki = 0; ki < b->block_rows_pre; ki++)
			dsPush(b_genes, dsItem(genes,ki));
		/* store gene arrays inside block */
		b->genes = dsNew(components);
		b->conds = dsNew(cols);
		scan_block(b_genes, b);
		if(b->block_cols == 0)  continue;
		b->block_rows = components;
		if(po->IS_pvalue)  b->score = -(100*log(b->pvalue));
		else  b->score = b->block_rows * b->block_cols;		
		dsClear(b->genes);
		for(ki=0; ki < components; ki++)  dsPush(b->genes,dsItem(genes,ki));
		for(ki = 0; ki < components; ki++)  if(!isInStack(allincluster, dsItem(genes,ki)))  dsPush(allincluster,dsItem(genes,ki));	
		/*save the current block b to the block list bb so that we can sort the blocks by their score*/
		bb[block_id++] = b;
		/* reaching the results number limit */
		if(block_id == po->SCH_BLOCK)  break;
		verboseDot();	
	}
	/* writes character to the current position in the standard output (stdout) and advances the internal file position indicator to the next position.
	 * It is equivalent to putc(character,stdout).*/
	putchar('\n');
	/* free-up the candidate list */
	free(candidates);
	free(allincluster);
	free (pvalues);
	return report_blocks(fw, bb, block_id);
}
/*make_graph subroutine prototypes */
void seed_deduct (const discrete *s)
/* remove a row from the profile */
{
	int i;
	discrete ss;
	for(i=0; i<cols; i++)
	{
		ss = s[i];
		profile[i][ss]--;
	}
}
int str_intersect_r(const discrete *s1, const discrete *s2)
{
	int common_cnt = 0;
	/* s1 and s2 of equal length, so we check s1 only */
	int i;
	for(i=0;i<cols;i++)
	{
		if(*s1==*s2 && (*s1!=0))  common_cnt++;
		s1++; 
		s2++;
	}
	return common_cnt;
}
int edge_cmpr(void *a, void *b)
{
	int score_a, score_b;
	score_a = ((Edge *)a)->score;
	score_b = ((Edge *)b)->score;
	if (score_a < score_b)  return -1;
	if (score_a == score_b)  return 0;
	return 1;
}
void fh_insert_fixed(struct fibheap *a, Edge *i, Edge **cur_min)
{
	if(a->fh_n < HEAP_SIZE)  fh_insert(a, (void *)i);
	else
	{
		if(edge_cmpr(cur_min, i) < 0)
		{
			/* Remove least value and renew */
			fh_extractmin(a);
			fh_insert(a, (void *)i);
			/* Keep a memory of the current min */
			*cur_min = (Edge *)fh_min(a);
		}
	}
}
void fh_dump(struct fibheap *a, Edge **res)
{
	int i;
	int n = a->fh_n;
	for (i=n-1; i>=0; i--)  res[i] = (Edge *) fh_extractmin(a);
}
void make_graph(const char* fn)
{
	FILE *fw = mustOpen(fn, "w");
	int i, j, cnt;
	int rec_num = 0;
	if (po->COL_WIDTH == 2) 
		po->COL_WIDTH = MAX(cols/20, 2);
	/* edge_ptr describe edges */
	AllocArray(edge_list, HEAP_SIZE);
	/* Allocating heap structure */
	struct fibheap *heap;
	heap = fh_makeheap();
	fh_setcmp(heap, edge_cmpr);
	/* Generating seed list and push into heap */
	progress("Generating seed list (minimum weight %d)", po->COL_WIDTH);
	Edge __cur_min = {0, 0, po->COL_WIDTH};
	Edge *_cur_min = &__cur_min;
	Edge **cur_min = & _cur_min;
	/* iterate over all genes to retrieve all edges */
	for(i = 0; i < rows; i++)
	{
		for(j = i+1; j < rows; j++)
		{
			cnt = str_intersect_r(arr_c[i], arr_c[j]);
			if(cnt < (*cur_min)->score)  continue;
			AllocVar(edge_ptr);
			edge_ptr -> gene_one = i;
			edge_ptr -> gene_two = j;
			edge_ptr -> score = cnt;	
			fh_insert_fixed(heap, edge_ptr, cur_min);
		}
	}
	rec_num = heap->fh_n;
	if(rec_num == 0)  errAbort("Not enough overlap between genes");
	/* sort the seeds */
	uglyTime("%d seeds generated", rec_num);
	ReAllocArray(edge_list, rec_num);
	fh_dump(heap, edge_list);
	/* model based deep clustering */
	int n_blocks = 0;
	progress("Clustering started");
	n_blocks = cluster(fw, edge_list, rec_num);
	uglyTime("%d clusters are written to %s", n_blocks, fn);
	/* clean up */
	for(i=0; i<rec_num; i++)  free(edge_list[i]);
	free(edge_list);
}
/* expand subroutine prototypes */
int intersect_rowE(const bool *colcand, discrete *g1, discrete *g2, const int cols)
{
	int i, cnt = 0;
	for(i=0; i< cols; i++)
		if( colcand[i] && (g1[i] == g2[i]) && g1[i] != 0 ) 
			cnt++;
	return cnt;
}
int reverse_rowE(const bool *colcand, discrete *g1, discrete *g2, const int cols)
{
	int i, cnt = 0;
	for(i=0; i< cols; i++)
		if( colcand[i] && (symbols[g1[i]] == -symbols[g2[i]])) 
			cnt++;
	return cnt;
}
void store_block(Block *b_ptr, struct dyStack *ge, struct dyStack *co)
{
	int row, col;
	row = dsSize(ge);
	col = dsSize(co);
	b_ptr->genes = ge;
	b_ptr->conds = co;
	b_ptr->block_rows = row;
	b_ptr->block_cols = col;
}
void init_expand()
{
	another_genes = genes;
	another_conds = conds;
	another_arr_c = arr_c;
	another_rows = rows;
	another_cols = cols;
}
/* Read the .block file, get components and colcand */
void read_and_solve_blocks(FILE *fb, const char *fn)
{
	init_expand();
	size_t n;
	int col;
	char *line = NULL;
	int bnumber = 0;
	struct dyStack *ge, *co;
	int i, components, m_cnt;
	bool *colcand;
	bool *candidates;
	Block *b;
	AllocVar(b);
	AllocArray(colcand, another_cols);
	AllocArray(candidates, another_rows);
	ge = dsNew(another_rows);
	co = dsNew(another_cols);
	FILE *fo = mustOpen(fn, "w");
	/* main course starts here */
	while(getline(&line, &n, fb) != -1)
	{
		/* fast forward to a line that contains BC*/
		/* strncmp compares up to num characters of the C string str1 to those of the C string str2
		 * strncmp ( const char * str1, const char * str2, size_t num )*/
		while(strncmp(line, "BC", 2)!=0) 
		{
			if(getline(&line, &n, fb)==-1) 
			{
				uglyTime("expanded biclusters are written to %s", fn);
				exit(0);
			}
		}
		components = 0;
		col = 0;
		dsClear(ge);
		dsClear(co);
		for (i=0; i< another_cols; i++)  colcand[i] = FALSE;
		for (i=0; i< another_rows; i++)  candidates[i] = TRUE;
		/* read genes from block */		
		SY_GETLINE = getline(&line, &n, fb);
		atom = strtok(line, delims);
		atom = strtok(NULL, delims);
		while((atom = strtok(NULL, delims)) != NULL)
		{
			/* look up for genes number */
			if (strlen(atom) == 0) continue;			
			for(i=0; i<another_rows; i++)
			{
				if (strcmp(atom ,another_genes[i]) == 0) break;
			}
			candidates[i] = FALSE;			
			dsPush(ge, i);
			components++;
		}
		/* read conditions from block */
		SY_GETLINE = getline(&line, &n, fb);
		atom = strtok(line, delims);
		atom = strtok(NULL, delims);
		while((atom = strtok(NULL, delims)) != NULL)
		{
			/*if (strlen(atom) < 5) break;*/			
			if (strlen(atom) == 0) continue;			
			for(i=0; i<another_cols; i++)
				if (strcmp(atom, another_conds[i]) == 0) break;
			colcand[i] = TRUE;
			dsPush(co, i);
			col++;
		}
		b->block_rows_pre = components;
		/* add some possible genes */
		for( i = 0; i < another_rows; i++)
		{
			m_cnt = intersect_rowE(colcand, another_arr_c[dsItem(ge,0)], another_arr_c[i], another_cols);
			/*printf ("%d\n",m_cnt);*/
			if( candidates[i] && (m_cnt >= (int)floor( (double)col * po->TOLERANCE)) )
			{
				dsPush(ge,i);
				components++;
				candidates[i] = FALSE;
			}
		}
		/* add genes that negative regulated to the consensus */
		for( i = 0; i < another_rows; i++)
		{
			m_cnt = reverse_rowE(colcand, another_arr_c[dsItem(ge,0)], another_arr_c[i], another_cols);
			if( candidates[i] && (m_cnt >= (int)floor( (double)col * po->TOLERANCE)) )
			{
				dsPush(ge,i);
				components++;
				candidates[i] = FALSE;
			}
		}
		if(dsSize(ge) > 1)
		{		
			store_block(b, ge, co);
			/*another_print_bc(fo, b, bnumber);*/
			print_bc(fo, b, bnumber++);
		}
	}
	printf("END Delta\n");
}