#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include "options.h"

int parse_options(int argc, char **argv, Option options[],int options_length, display_version_function display_version) {
	char * shortlist=(char *) malloc(sizeof(char)*((options_length+2)*2));
	struct option *longlist= (struct option *) malloc(sizeof(struct option)*(options_length+2));
	int index=0;
	shortlist[0]=0;
	int result=1;
	int i;
	int opt;
	int count;
	bzero(shortlist, sizeof(char)*((options_length+2)*2));	

	for (i=0; i<options_length; i++) {
		options[i].isset=FALSE;
		shortlist[strlen(shortlist)]=options[i].shortoption;
		longlist[i].name=strdup(options[i].longoption);
		if (options[i].hasvalue) {
			strcat(shortlist,":");
			longlist[i].has_arg=required_argument;
		} else {
			longlist[i].has_arg=no_argument;
		}
		longlist[i].flag=0;
		longlist[i].val=options[i].shortoption;
	}
	strcat(shortlist,"hv");
	longlist[options_length].name="help";
	longlist[options_length].has_arg=no_argument;
	longlist[options_length].flag=0;
	longlist[options_length].val='h';
	longlist[options_length+1].name="version";
	longlist[options_length+1].has_arg=no_argument;
	longlist[options_length+1].flag=0;
	longlist[options_length+1].val='v';
	
	while ((opt=getopt_long(argc, argv, shortlist,longlist, &index))>0) {
		//printf("Read option:%c index=%d\n",opt,index);
		switch(opt) {
			case 'h':
				//Display help message
				count=2;
				printf(" usage : %s	[-h|--help] [-v|--version] ",argv[0]);
				for (i=0; i<options_length; i++) {
					if (options[i].mandatory) {
						if (options[i].hasvalue) {
							printf("-%c|--%s <value> ",options[i].shortoption, options[i].longoption);
						} else {
							printf("-%c|--%s ",options[i].shortoption, options[i].longoption);
						}
					} else {
						if (options[i].hasvalue) {
							printf("[-%c|--%s <value>] ",options[i].shortoption, options[i].longoption);
						} else {
							printf("[-%c|--%s] ",options[i].shortoption, options[i].longoption);
						}
					}
					count++;
					if (count>3) {
						printf("\n			");
						count=0;
					}
				}
				printf("\n\nDescription:\n");
				printf("-h --help	: Display this help message\n");
				printf("-v --version	: Display %s version\n", argv[0]);
				for (i=0; i<options_length; i++) {
					printf("-%c --%s	: %s\n",options[i].shortoption, options[i].longoption, options[i].description);
				}
				return(DISPLAY_HELP);
			break;
			case 'v':
				//Display the version
				display_version(argv[0]);
				return(DISPLAY_VERSION);
			break;
			case '?':
				//@printf("Unknown option\n");
			break;
			default:
				index=options_length;
				for (i=0;i<options_length;i++) if (options[i].shortoption==opt) index=i;
				if (index<options_length) {
					if (options[index].hasvalue) {
						if (optarg==0) {
							result=-1;
							printf("Error, missing value for -%c --%s : %s\n",options[index].shortoption, options[index].longoption, options[index].description);
						} else {
							options[index].isset=TRUE;
							switch (options[index].type) {
								case STRING:
									*((char **) options[index].target)=strdup(optarg);
								break;
								case INT :
									*((int *) options[index].target)=atoi(optarg);
								break;
								case LONG :
									*((long *) options[index].target)=atol(optarg);
								break;
								case DOUBLE:
									*((double *)options[index].target)=atof(optarg);
								break;
								case NONE:
								break;
							}
						}
					} else {
						options[index].isset=TRUE;
					}
				}
			break;
		}
	} //End while loop on getopt
	result=optind;
	//Now check that every required option is set.
	for (i=0; i<options_length; i++) {
		if (options[i].mandatory) {
			if (!options[i].isset) {
				result=MISSING_MANDATORY;
				printf("Missing required parameter -%c --%s : %s\n",options[i].shortoption, options[i].longoption, options[i].description);
			}
		}
	}
	return result;
} 

