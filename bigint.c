#include "bigint.h"
#include "command_options.h"
#include "user_interface.h"
#include "merge.h"
#include "quick.h"

int main(int argc,  char *argv[])
{
  arguments args = default_args();
  argp_parse(&argp, argc, argv, 0, 0, &args);

  FILE* cin = fopen(args.filename,"r");
  if (!cin) {
    printf("Failed to open input data file %s\n",args.filename);
    return -1;
  }

  bigint_array bigints __attribute__((cleanup (bigints_clear)))
                       = bigints_read(cin);

  //quicksort_mpz_t(bigints.data, bigints.data+bigints.size);

  if (bigints.size == 0) {
    printf("No data read from input data file %s\n",args.filename);
    return -1;
  }
  if (args.interactive)
    ui_loop(&args,bigints);

}
