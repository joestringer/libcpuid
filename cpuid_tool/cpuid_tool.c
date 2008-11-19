/*
 * Copyright 2008  Veselin Georgiev,
 * anrieffNOSPAM @ mgail_DOT.com (convert to gmail)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @File cpuid_tool.c
 * @Date 2008-11-19
 * @Author Veselin Georgiev
 * @Brief Command line interface to libcpuid
 *
 * This file is provides a direct interface to libcpuid. See the usage()
 * function (or just run the program with the `--help' switch) for a short
 * command line options reference.
 *
 * This file has several purposes:
 *
 * 1) When started with no arguments, the program outputs the RAW and decoded
 *    CPU data to files (`raw.txt' and `report.txt', respectively) - this is
 *    intended to be a dumb, doubleclicky tool for non-developer
 *    users, that can provide debug info about unrecognized processors to
 *    libcpuid developers.
 * 2) When operated from the terminal with the `--report' option, it is a
 *    generic CPU-info utility.
 * 3) Can be used in shell scripts, e.g. to get the name of the CPU, cache
 *    sizes, features, with query options like `--cache', `--brandstr', etc.
 * 4) Finally, it serves to self-document libcpiud itself :)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libcpuid.h"

/* Globals: */
char raw_data_file[256] = "";
char out_file[256] = "";
typedef enum {
	NEED_CPUID_PRESENT,
	NEED_VENDOR_STR,
	NEED_VENDOR_ID,
	NEED_BRAND_STRING,
	NEED_FAMILY,
	NEED_MODEL,
	NEED_STEPPING,
	NEED_EXT_FAMILY,
	NEED_EXT_MODEL,
	NEED_NUM_CORES,
	NEED_NUM_LOGICAL,
	NEED_TOTAL_CPUS,
	NEED_L1D_SIZE,
	NEED_L1I_SIZE,
	NEED_L2_SIZE,
	NEED_L3_SIZE,
	NEED_L1D_ASSOC,
	NEED_L2_ASSOC,
	NEED_L3_ASSOC,
	NEED_L1D_CACHELINE,
	NEED_L2_CACHELINE,
	NEED_L3_CACHELINE,
	NEED_CODENAME,
	NEED_FEATURES,
	NEED_CLOCK,
	NEED_CLOCK_RDTSC,
} output_data_switch;

int need_input = 0,
    need_output = 0,
    need_quiet = 0,
    need_report = 0,
    need_clockreport = 0,
    need_timed_clockreport = 0,
    need_verbose = 0,
    need_version = 0;

#define MAX_REQUESTS 32
int num_requests = 0;
output_data_switch requests[MAX_REQUESTS];

FILE *fout;


const struct { output_data_switch sw; const char* synopsis; int ident_required; }
matchtable[] = {
	{ NEED_CPUID_PRESENT, "--cpuid"        , 0},
	{ NEED_VENDOR_STR   , "--vendorstr"    , 1},
	{ NEED_VENDOR_ID    , "--vendorid"     , 1},
	{ NEED_BRAND_STRING , "--brandstr"     , 1},
	{ NEED_FAMILY       , "--family"       , 1},
	{ NEED_MODEL        , "--model"        , 1},
	{ NEED_STEPPING     , "--stepping"     , 1},
	{ NEED_EXT_FAMILY   , "--extfamily"    , 1},
	{ NEED_EXT_MODEL    , "--extmodel"     , 1},
	{ NEED_NUM_CORES    , "--cores"        , 1},
	{ NEED_NUM_LOGICAL  , "--logical"      , 1},
	{ NEED_TOTAL_CPUS   , "--total-cpus"   , 1},
	{ NEED_L1D_SIZE     , "--l1d-cache"    , 1},
	{ NEED_L1I_SIZE     , "--l1i-cache"    , 1},
	{ NEED_L2_SIZE      , "--cache"        , 1},
	{ NEED_L2_SIZE      , "--l2-cache"     , 1},
	{ NEED_L3_SIZE      , "--l3-cache"     , 1},
	{ NEED_L1D_ASSOC    , "--l1d-assoc"    , 1},
	{ NEED_L2_ASSOC     , "--l2-assoc"     , 1},
	{ NEED_L3_ASSOC     , "--l3-assoc"     , 1},
	{ NEED_L1D_CACHELINE, "--l1d-cacheline", 1},
	{ NEED_L2_CACHELINE , "--l2-cacheline" , 1},
	{ NEED_L3_CACHELINE , "--l3-cacheline" , 1},
	{ NEED_CODENAME     , "--codename"     , 1},
	{ NEED_FEATURES     , "--flags"        , 1},
	{ NEED_CLOCK        , "--clock"        , 0},
	{ NEED_CLOCK_RDTSC  , "--clock-rdtsc"  , 1},
};

const int sz_match = (sizeof(matchtable) / sizeof(matchtable[0]));

/* functions */

static void usage(void)
{
	printf("Usage: cpuid_tool [options]\n\n");
	printf("Options:\n");
	printf("  -h,--help      - Show this help\n");
	printf("  --load=<file>  - Load RAW CPUID data from file\n");
	printf("  --save=<file>  - Aquire RAW CPUID data and write it to file\n");
	printf("  --report,--all - Report all decoded CPU info (w/o clock)\n");
	printf("  --clock        - in conjunction to --report: print CPU clock as well\n");
	printf("  --clock-rdtsc  - same as --clock, but use RDTSC for clock detection\n");
	printf("  --quiet        - disable warnings\n");
	printf("  --outfile=<file> - redirect all output to this file, instead of stdout\n");
	printf("  --verbose      - be extra verbose\n");
	printf("  --version      - print library version\n");
	printf("\n");
	printf("Query switches (generate 1 line of ouput per switch; in order of appearance):");
	
	int line_fill = 80, l, i;
	for (i = 0; i < sz_match; i++) {
		l = (int) strlen(matchtable[i].synopsis);
		if (line_fill + l > 76) {
			line_fill = 2;
			printf("\n  ");
		}
		printf("%s", matchtable[i].synopsis);
		if (i < sz_match - 1) {
			line_fill += l + 2;
			printf(", ");
		}
	}
	printf("\n\n");
	printf("If `-' is used for <file>, then stdin/stdout will be used instead of files.\n");
	printf("When no options are present, the program behaves as if it was invoked with\n");
	printf("  cpuid_tool \"--save=raw.txt --outfile=report.txt --report --verbose\"\n");
}

static int parse_cmdline(int argc, char** argv)
{
	#define xerror(msg)\
		fprintf(stderr, "Error: %s\n\n", msg); \
		fprintf(stderr, "Use -h to get a list of supported options\n"); \
		return -1;
	if (argc == 1) {
		/* Default command line options */
		need_output = 1;
		strcpy(raw_data_file, "raw.txt");
		strcpy(out_file, "report.txt");
		need_report = 1;
		need_verbose = 1;
		return 1;
	}
	int i, j, recog;
	for (i = 1; i < argc; i++) {
		char *arg = argv[i];
		recog = 0;
		if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
			usage();
			return 0;
		}
		if (!strncmp(arg, "--load=", 7)) {
			if (need_input) {
				xerror("Too many `--load' options!");
			}
			if (need_output) {
				xerror("Cannot have both `--load' and `--save' options!");
			}
			if (strlen(arg) <= 7) {
				xerror("--load: bad file specification!");
			}
			need_input = 1;
			strcpy(raw_data_file, arg + 7);
			recog = 1;
		}
		if (!strncmp(arg, "--save=", 7)) {
			if (need_output) {
				xerror("Too many `--save' options!");
			}
			if (need_input) {
				xerror("Cannot have both `--load' and `--save' options!");
			}
			if (strlen(arg) <= 7) {
				xerror("--save: bad file specification!");
			}
			need_output = 1;
			strcpy(raw_data_file, arg + 7);
			recog = 1;
		}
		if (!strncmp(arg, "--outfile=", 10)) {
			if (strlen(arg) <= 10) {
				xerror("--output: bad file specification!");
			}
			strcpy(out_file, arg + 10);
			recog = 1;
		}
		if (!strcmp(arg, "--report") || !strcmp(arg, "--all")) {
			need_report = 1;
			recog = 1;
		}
		if (!strcmp(arg, "--clock")) {
			need_clockreport = 1;
			recog = 1;
		}
		if (!strcmp(arg, "--clock-rdtsc")) {
			need_timed_clockreport = 1;
			recog = 1;
		}
		if (!strcmp(arg, "--quiet")) {
			need_quiet = 1;
			recog = 1;
		}
		if (!strcmp(arg, "--verbose")) {
			need_verbose = 1;
			recog = 1;
		}
		if (!strcmp(arg, "--version")) {
			need_version = 1;
			recog = 1;
		}
		for (j = 0; j < sz_match; j++)
			if (!strcmp(arg, matchtable[j].synopsis)) {
				if (num_requests >= MAX_REQUESTS) {
					xerror("Too many requests!");
				}
				requests[num_requests++] = matchtable[j].sw;
				recog = 1;
				break;
			}
		
		if (!recog) {
			fprintf(stderr, "Unrecognized option: `%s'\n\n", arg);
			fprintf(stderr, "Use -h to get a list of supported options\n");
			return -1;
		}
	}
	return 1;
}

static void close_out(void)
{
	fclose(fout);
}

static int check_need_raw_data(void)
{
	int i, j;
	
	if (need_output || need_report) return 1;
	for (i = 0; i < num_requests; i++) {
		for (j = 0; j < sz_match; j++)
			if (requests[i] == matchtable[j].sw &&
			    matchtable[j].ident_required) return 1;
	}
	return 0;
}

static void print_info(output_data_switch query, struct cpu_raw_data_t* raw,
                       struct cpu_id_t* data)
{
	int i;
	switch (query) {
		case NEED_CPUID_PRESENT:
			fprintf(fout, "%d\n", cpuid_present());
			break;
		case NEED_VENDOR_STR:
			fprintf(fout, "%s\n", data->vendor_str);
			break;
		case NEED_VENDOR_ID:
			fprintf(fout, "%d\n", data->vendor);
			break;
		case NEED_BRAND_STRING:
			fprintf(fout, "%s\n", data->brand_str);
			break;
		case NEED_FAMILY:
			fprintf(fout, "%d\n", data->family);
			break;
		case NEED_MODEL:
			fprintf(fout, "%d\n", data->model);
			break;
		case NEED_STEPPING:
			fprintf(fout, "%d\n", data->stepping);
			break;
		case NEED_EXT_FAMILY:
			fprintf(fout, "%d\n", data->ext_family);
			break;
		case NEED_EXT_MODEL:
			fprintf(fout, "%d\n", data->ext_model);
			break;
		case NEED_NUM_CORES:
			fprintf(fout, "%d\n", data->num_cores);
			break;
		case NEED_NUM_LOGICAL:
			fprintf(fout, "%d\n", data->num_logical_cpus);
			break;
		case NEED_TOTAL_CPUS:
			fprintf(fout, "%d\n", data->total_cpus);
			break;
		case NEED_L1D_SIZE:
			fprintf(fout, "%d\n", data->l1_data_cache);
			break;
		case NEED_L1I_SIZE:
			fprintf(fout, "%d\n", data->l1_instruction_cache);
			break;
		case NEED_L2_SIZE:
			fprintf(fout, "%d\n", data->l2_cache);
			break;
		case NEED_L3_SIZE:
			fprintf(fout, "%d\n", data->l3_cache);
			break;
		case NEED_L1D_ASSOC:
			fprintf(fout, "%d\n", data->l1_assoc);
			break;
		case NEED_L2_ASSOC:
			fprintf(fout, "%d\n", data->l2_assoc);
			break;
		case NEED_L3_ASSOC:
			fprintf(fout, "%d\n", data->l3_assoc);
			break;
		case NEED_L1D_CACHELINE:
			fprintf(fout, "%d\n", data->l1_cacheline);
			break;
		case NEED_L2_CACHELINE:
			fprintf(fout, "%d\n", data->l2_cacheline);
			break;
		case NEED_L3_CACHELINE:
			fprintf(fout, "%d\n", data->l3_cacheline);
			break;
		case NEED_CODENAME:
			fprintf(fout, "%s\n", data->cpu_codename);
			break;
		case NEED_FEATURES:
		{
			for (i = 0; i < NUM_CPU_FEATURES; i++)
				if (data->flags[i])
					fprintf(fout, " %s", cpu_feature_str(i));
			fprintf(fout, "\n");
			break;
		}
		case NEED_CLOCK:
			fprintf(fout, "%d\n", cpu_clock());
			break;
		case NEED_CLOCK_RDTSC:
			fprintf(fout, "%d\n", cpu_clock_measure(400, 1));
			break;
		default:
			fprintf(fout, "How did you get here?!?\n");
			break;
	}
}

int main(int argc, char** argv)
{
	int parseres = parse_cmdline(argc, argv);
	int i, readres, writeres;
	int only_clock_queries;
	if (parseres != 1)
		return parseres;
	struct cpu_raw_data_t raw;
	struct cpu_id_t data;
	
	/* In quiet mode, disable libcpuid warning messages: */
	if (need_quiet)
		cpuid_set_warn_function(NULL);

	/* Redirect output, if necessary: */
	if (strcmp(out_file, "") && strcmp(out_file, "-")) {
		fout = fopen(out_file, "wt");
		if (!fout) {
			if (!need_quiet)
				fprintf(stderr, "Cannot open `%s' for writing!\n", out_file);
			return -1;
		}
		atexit(close_out);
	} else {
		fout = stdout;
	}
	
	/* If requested, print library version: */
	if (need_version)
		fprintf(fout, "%s\n", cpuid_lib_version());
	
	if (need_input) {
		/* We have a request to input raw CPUID data from file: */
		if (!strcmp(raw_data_file, "-"))
			/* Input from stdin */
			readres = cpuid_deserialize_raw_data(&raw, "");
		else
			/* Input from file */
			readres = cpuid_deserialize_raw_data(&raw, raw_data_file);
		if (readres < 0) {
			if (!need_quiet) {
				fprintf(stderr, "Cannot deserialize raw data from ");
				if (!strcmp(raw_data_file, "-"))
					fprintf(stderr, "stdin\n");
				else
					fprintf(stderr, "file `%s'\n", raw_data_file);
				/* Print the error message */
				fprintf(stderr, "Error: %s\n", cpuid_error());
			}
			return -1;
		}
	} else {
		if (check_need_raw_data()) {
			/* Try to obtain raw CPUID data from the CPU: */
			readres = cpuid_get_raw_data(&raw);
			if (readres < 0) {
				if (!need_quiet) {
					fprintf(stderr, "Cannot obtain raw CPU data!\n");
					fprintf(stderr, "Error: %s\n", cpuid_error());
				}
				return -1;
			}
		}
	}
	
	/* Need to dump raw CPUID data to file: */
	if (need_output) {
		if (need_verbose)
			printf("Writing raw CPUID dump to `%s'\n", raw_data_file);
		if (!strcmp(raw_data_file, "-"))
			/* Serialize to stdout */
			writeres = cpuid_serialize_raw_data(&raw, "");
		else
			/* Serialize to file */
			writeres = cpuid_serialize_raw_data(&raw, raw_data_file);
		if (writeres < 0) {
			if (!need_quiet) {
				fprintf(stderr, "Cannot serialize raw data to ");
				if (!strcmp(raw_data_file, "-"))
					fprintf(stderr, "stdout\n");
				else
					fprintf(stderr, "file `%s'\n", raw_data_file);
				/* Print the error message */
				fprintf(stderr, "Error: %s\n", cpuid_error());
			}
			return -1;
		}
	}
	if (need_report) {
		if (need_verbose) {
			printf("Writing decoded CPU report to `%s'\n", out_file);
		}
		/* Write a thorough report of cpu_id_t structure to output (usually stdout) */
		fprintf(fout, "CPUID is present\n");
		/*
		 * Try CPU identification
		 * (this fill the `data' structure with decoded CPU features)
		 */
		if (cpu_identify(&raw, &data) < 0)
			fprintf(fout, "Error identifying the CPU: %s\n", cpuid_error());
		
		/* OK, now write what we have in `data'...: */
		fprintf(fout, "CPU Info:\n------------------\n");
		fprintf(fout, "  vendor_str : `%s'\n", data.vendor_str);
		fprintf(fout, "  vendor id  : %d\n", (int) data.vendor);
		fprintf(fout, "  brand_str  : `%s'\n", data.brand_str);
		fprintf(fout, "  family     : %d\n", data.family);
		fprintf(fout, "  model      : %d\n", data.model);
		fprintf(fout, "  stepping   : %d\n", data.stepping);
		fprintf(fout, "  ext_family : %d\n", data.ext_family);
		fprintf(fout, "  ext_model  : %d\n", data.ext_model);
		fprintf(fout, "  num_cores  : %d\n", data.num_cores);
		fprintf(fout, "  num_logical: %d\n", data.num_logical_cpus);
		fprintf(fout, "  total_cpus : %d\n", data.total_cpus);
		fprintf(fout, "  L1 D cache : %d KB\n", data.l1_data_cache);
		fprintf(fout, "  L1 I cache : %d KB\n", data.l1_instruction_cache);
		fprintf(fout, "  L2 cache   : %d KB\n", data.l2_cache);
		fprintf(fout, "  L3 cache   : %d KB\n", data.l3_cache);
		fprintf(fout, "  L1D assoc. : %d-way\n", data.l1_assoc);
		fprintf(fout, "  L2 assoc.  : %d-way\n", data.l2_assoc);
		fprintf(fout, "  L3 assoc.  : %d-way\n", data.l3_assoc);
		fprintf(fout, "  L1D line sz: %d bytes\n", data.l1_cacheline);
		fprintf(fout, "  L2 line sz : %d bytes\n", data.l2_cacheline);
		fprintf(fout, "  L3 line sz : %d bytes\n", data.l3_cacheline);
		fprintf(fout, "  code name  : `%s'\n", data.cpu_codename);
		fprintf(fout, "  features   :");
		/*
		 * Here we enumerate all CPU feature bits, and when a feature
		 * is present output its name:
		 */
		for (i = 0; i < NUM_CPU_FEATURES; i++)
			if (data.flags[i])
				fprintf(fout, " %s", cpu_feature_str(i));
		fprintf(fout, "\n");
		
		/* Is CPU clock info requested? */
		if (need_clockreport) {
			if (need_timed_clockreport) {
				/* Here we use the RDTSC-based routine */
				fprintf(fout, "  cpu clock  : %d MHz\n",
				        cpu_clock_measure(400, 1));
			} else {
				/* Here we use the OS-provided info */
				fprintf(fout, "  cpu clock  : %d MHz\n",
				        cpu_clock());
			}
		}
	}
	/*
	 * Check if we have any queries to process.
	 * We have to handle the case when `--clock' or `--clock-rdtsc' options
	 * are present.
	 * If in report mode, this will generate spurious output after the
	 * report, if not handled explicitly.
	 */
	only_clock_queries = 1;
	for (i = 0; i < num_requests; i++)
		if (requests[i] != NEED_CLOCK && requests[i] != NEED_CLOCK_RDTSC) {
			only_clock_queries = 0;
			break;
		}
	/* OK, process all queries. */
	if ((!need_report || !only_clock_queries) && num_requests > 0) {
		/* Identify the CPU. Make it do cpuid_get_raw_data() itself */
		if (check_need_raw_data() && cpu_identify(NULL, &data) < 0) {
			if (!need_quiet)
				fprintf(stderr,
				        "Error identifying the CPU: %s\n",
				        cpuid_error());
			return -1;
		}
		for (i = 0; i < num_requests; i++)
			print_info(requests[i], &raw, &data);
	}
	
	return 0;
}
