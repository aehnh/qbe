#include "all.h"
#include "config.h"
#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>

Target T;

extern Target T_amd64_sysv;
extern Target T_arm64;

static struct TMap {
	char *name;
	Target *T;
} tmap[] = {
	{ "amd64_sysv", &T_amd64_sysv },
	{ "arm64", &T_arm64 },
	{ 0, 0 }
};

enum Asm {
	Gasmacho,
	Gaself,
};

char debug['Z'+1] = {
	['P'] = 0, /* parsing */
	['A'] = 0, /* abi lowering */
	['I'] = 0, /* instruction selection */
	['L'] = 0, /* liveness */
	['M'] = 0, /* memory optimization */
	['N'] = 0, /* ssa construction */
	['C'] = 0, /* copy elimination */
	['F'] = 0, /* constant folding */
	['S'] = 0, /* spilling */
	['R'] = 0, /* reg. allocation */
};

static FILE *outf;
static int dbg;

typedef struct {
	char *buffer;
	size_t size;
} Buffer;

typedef struct {
	Buffer *buffers;
	size_t size;
	size_t capacity;
} BufferArray;

BufferArray *arr;

void buffer_array_init() {
	arr = emalloc(sizeof(BufferArray));
	arr->buffers = emalloc(10 * sizeof(Buffer));
	arr->size = 0;
	arr->capacity = 10;
}

void buffer_array_add(char *buffer, size_t size) {
	if (arr->size >= arr->capacity) {
		arr->capacity *= 2;
		arr->buffers = realloc(arr->buffers, arr->capacity * sizeof(Buffer));
		if (!arr->buffers)
			die("realloc, out of memory");
	}
	arr->buffers[arr->size].buffer = buffer;
	arr->buffers[arr->size].size = size;
	arr->size++;
}

static void
data(Dat *d)
{
	if (dbg)
		return;
	if (d->type == DEnd) {
		fputs("/* end data */\n\n", outf);
		freeall();
	}
	gasemitdat(d, outf);
}

static void
func(Fn *fn)
{
	uint n;
	char *buffer;
	size_t size;
	FILE *temp;
	int i;

	if (dbg)
		fprintf(stderr, "**** Function %s ****", fn->name);
	if (debug['P']) {
		fprintf(stderr, "\n> After parsing:\n");
		printfn(fn, stderr);
	}
	fillrpo(fn);
	fillpreds(fn);
	filluse(fn);
	memopt(fn);
	ssa(fn);
	filluse(fn);
	ssacheck(fn);
	fillloop(fn);
	fillalias(fn);
	loadopt(fn);
	filluse(fn);
	ssacheck(fn);
	copy(fn);
	filluse(fn);
	fold(fn);
	T.abi(fn);
	fillpreds(fn);
	filluse(fn);
	T.isel(fn);
	fillrpo(fn);
	filllive(fn);
	fillcost(fn);
	spill(fn);
	rega(fn);
	fillrpo(fn);
	simpljmp(fn);
	fillpreds(fn);
	fillrpo(fn);
	assert(fn->rpo[0] == fn->start);
	for (n=0;; n++)
		if (n == fn->nblk-1) {
			fn->rpo[n]->link = 0;
			break;
		} else
			fn->rpo[n]->link = fn->rpo[n+1];
	if (!dbg) {
		temp = tmpfile();
		if (!temp) {
			die("tmpfile failed");
		}
		T.emitfn(fn, temp);
		fprintf(temp, "/* end function %s */\n\n", fn->name);

		rewind(temp);
        fseek(temp, 0, SEEK_END);
        size_t temp_size = ftell(temp);
        rewind(temp);
        char *buffer = emalloc(temp_size);
        size_t read_size = fread(buffer, 1, temp_size, temp);
        if (read_size != temp_size) {
			die("fread failed");
		}
		/*fwrite(buffer, 1, temp_size, outf);*/
		fclose(temp);
		buffer_array_add(buffer, temp_size);
	} else
		fprintf(stderr, "\n");
	freeall();
}

int
main(int ac, char *av[])
{
	struct TMap *tm;
	FILE *inf, *hf;
	char *f, *sep;
	int c, asm;
	int i, j, t, idx;

	srand(time(NULL));

	asm = Defasm;
	T = Deftgt;
	outf = stdout;
	while ((c = getopt(ac, av, "hd:o:G:t:")) != -1)
		switch (c) {
		case 'd':
			for (; *optarg; optarg++)
				if (isalpha(*optarg)) {
					debug[toupper(*optarg)] = 1;
					dbg = 1;
				}
			break;
		case 'o':
			if (strcmp(optarg, "-") != 0)
				outf = fopen(optarg, "w");
			break;
		case 't':
			for (tm=tmap;; tm++) {
				if (!tm->name) {
					fprintf(stderr, "unknown target '%s'\n", optarg);
					exit(1);
				}
				if (strcmp(optarg, tm->name) == 0) {
					T = *tm->T;
					break;
				}
			}
			break;
		case 'G':
			if (strcmp(optarg, "e") == 0)
				asm = Gaself;
			else if (strcmp(optarg, "m") == 0)
				asm = Gasmacho;
			else {
				fprintf(stderr, "unknown gas flavor '%s'\n", optarg);
				exit(1);
			}
			break;
		case 'h':
		default:
			hf = c != 'h' ? stderr : stdout;
			fprintf(hf, "%s [OPTIONS] {file.ssa, -}\n", av[0]);
			fprintf(hf, "\t%-11s prints this help\n", "-h");
			fprintf(hf, "\t%-11s output to file\n", "-o file");
			fprintf(hf, "\t%-11s generate for a target among:\n", "-t <target>");
			fprintf(hf, "\t%-11s ", "");
			for (tm=tmap, sep=""; tm->name; tm++, sep=", ")
				fprintf(hf, "%s%s", sep, tm->name);
			fprintf(hf, "\n");
			fprintf(hf, "\t%-11s generate gas (e) or osx (m) asm\n", "-G {e,m}");
			fprintf(hf, "\t%-11s dump debug information\n", "-d <flags>");
			exit(c != 'h');
		}

	switch (asm) {
	case Gaself:
		gasloc = ".L";
		gassym = "";
		break;
	case Gasmacho:
		gasloc = "L";
		gassym = "_";
		break;
	}

	buffer_array_init();

	do {
		f = av[optind];
		if (!f || strcmp(f, "-") == 0) {
			inf = stdin;
			f = "-";
		} else {
			inf = fopen(f, "r");
			if (!inf) {
				fprintf(stderr, "cannot open '%s'\n", f);
				exit(1);
			}
		}
		parse(inf, f, data, func);
	} while (++optind < ac);

	int *indices = emalloc(arr->size * sizeof(int));
	for (i = 0; i < arr->size; i++) {
        indices[i] = i;
    }
	if (arr->size > 1) {
        for (i = 0; i < arr->size - 1; i++) {
            j = i + rand() / (RAND_MAX / (arr->size - i) + 1);
            t = indices[j];
            indices[j] = indices[i];
            indices[i] = t;
        }
    }

	for (i = 0; i < arr->size; i++) {
		idx = indices[i];
		fwrite(arr->buffers[idx].buffer, sizeof(char), arr->buffers[idx].size, outf);
		free(arr->buffers[idx].buffer);
	}
	free(arr->buffers);
	free(arr);

	if (!dbg)
		gasemitfin(outf);

	exit(0);
}
