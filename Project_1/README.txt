ΑΝΑΣΤΑΣΙΟΣ  ΠΑΠΑΔΟΠΟΥΛΟΣ    03858   tasospapado@uth.gr
ΔΗΜΗΤΡΙΟΥ   ΝΙΚΟΛΑΟΣ        03847   ndimtrio@uth.gr
ΑΝΑΣΤΑΣΙΑ   ΕΥΘΥΜΙΟΥ        03988   anastefthymiou@uth.gr

for the compilation of find_roots_lib and modules use:
            find_roots
    >>  gcc -w -c roots.c -o roots.o
        ar rcs libroots.a roots.o
        gcc -w find_roots_lib.c -L. -lroots -o find_roots

            modules
    >> sudo make && sudo insmod <module name>  (module name = project1-kyber.ko / sysfs_module.ko)