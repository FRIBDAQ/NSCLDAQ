/* cmdline.h */

/* File autogenerated by gengetopt version 2.6  */

#ifndef _cmdline_h
#define _cmdline_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Don't define PACKAGE and VERSION if we use automake.  */
#if defined PACKAGE
#  undef PACKAGE
#endif
#define PACKAGE "daqstart"
#if defined VERSION
#  undef VERSION
#endif
#define VERSION "1.0"

struct gengetopt_args_info {
  char * error_arg;	/* Error log sink (e.g. file:myfile.log).  */
  char * output_arg;	/* Output log sink (e.g. file:myfile.log).  */
  int notify_flag;	/* Enable exit notification logging (default=off).  */

  int help_given ;	/* Whether help was given.  */
  int version_given ;	/* Whether version was given.  */
  int error_given ;	/* Whether error was given.  */
  int output_given ;	/* Whether output was given.  */
  int notify_given ;	/* Whether notify was given.  */

  char **inputs ; /* unamed options */
  unsigned inputs_num ; /* unamed options number */
} ;

int cmdline_parser (int argc, char * const *argv, struct gengetopt_args_info *args_info);

void cmdline_parser_print_help(void);
void cmdline_parser_print_version(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _cmdline_h */
