DIMITRIOU NIKOLAOS      03847   ndimitrio@uth.gr
PAPADOPOULOS ANASTASIOS 03858   tasospapado@uth.gr
EFTHYMIOU ANASTASIA     03988   anastefthymiou@uth.gr

user files:
    wset_monitor.c
    page_info.h
    syscall_wrapper.c
    syscall_wrapper.h
    run_tests.sh

HOW TO RUN:
    compile:
        gcc -c syscall_wrapper.c -o syscall_wrapper.o
        ar rcs lib_page_info.a syscall_wrapper.o
        gcc -Wall -g wset_monitor.c -O0 -L. -l_page_info -o wset_monitor

    wset_monitor.c takes as arguments 
        1). the load percentage. (the percentage of total pages that are going to be accessed)
        2). the total number of pages
        3) ./wset_monitor <load_percentage> <num_of_pages>
    
    run_tests.sh runs the wset_monitor many times executing different run_tests
        1) the executable must be named wset_monitor
        2) chmod +x run_tests.sh
        3) ./run_tests