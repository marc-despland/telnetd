#ifndef _OPTIONS_H
#define _OPTIONS_H
#include "boolean.h"

#define MDE_OPTIONS_VERSION "1.0.0"

#define DISPLAY_HELP		-1
#define DISPLAY_VERSION		-2
#define MISSING_MANDATORY	-3

// The list of supported type for option value;
typedef enum {NONE, STRING, INT, LONG, DOUBLE} optionType;

//The type to define attempt option for the software
typedef struct Option {
	char shortoption;
	char * longoption;
	char * description;
	BOOL hasvalue;
	BOOL mandatory;
	BOOL isset;
	void * target;
	optionType type;
} Option;

//The function to display version. The parameter is the command name.
typedef void (*display_version_function)(char*);
 
//Parse the options parameter
//	argc		: 	number of arguments passed on command line
//	argv		:	array of arguments passed on command lina
//	options		:	array that describe possible options
//	options_length	:	size of the options array
//	display_version	:	function call to display the version of the software
//
//Return value
// 	<0 		: 	A problem occurs 
//	>=0		:	index of the first non option argument (argv has been sorted). if > at argc: no more argument to manage
int parse_options(int argc, char **argv, Option options[],int options_length, display_version_function display_version);
#endif
