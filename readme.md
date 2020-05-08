# Big Integer sort

```bash
gcc -o bigisort -g -O0 -Wall -lgmp -lncurses bigint.c
./bigisort -i -f bigints.dat 

 -f, --file=filename        Input filename.
 -h, --heapsort             Set sort algo to heapsort.
 -i, --interactive          Interactive mode with text UI.
 -m, --mergesort            Set sort algo to mergesort.
     --pthreads             Switch threading On/oFf.
 -q, --quicksort            Set sort algo to quicksort.
 -?, --help                 Give this help list
     --usage                Give a short usage message
 -V, --version              Print program version
```
