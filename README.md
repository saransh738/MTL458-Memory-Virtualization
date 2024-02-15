# Memory Virtualization
## Implemented single-level page tables, and an API to simulate the memory.
We will be implementing single-level page tables, and an API to simulate the memory. We are given 200 MB RAM, to be used by the OS to create and manage user processes. The first portion of the RAM will be used by the OS and the remaining will be assigned to processes by OS. The OS should support the creation, stopping, and forking of processes (only the memory management part of it). Each process has 4 MB of virtual memory (The layout is given in Figure ), and the page size is set to 4KB. The OS should flexibly map the virtual pages of processes to the physical frames.

![image](https://github.com/saransh738/MTL458-Memory-Virtualization/assets/74806993/2cf4d142-6633-4fb7-961a-4c5611491c9e)

## Details
You should implement the following functions: 
* create ps(int code size, int ro data size, int rw data size, int max stack size, unsigned char* code and ro data): This function should create a new process, allocate its virtual memory initialize code, and read-only data segments.
* exit ps(int pid): This function should deallocate all the memory allocated to the process with the given pid. This pid can now be used for other new processes.
* fork ps(int pid): This function should create a new process that has identical memory with the process of a given pid.
* allocate pages(int pid, int vmem addr, int num pages, int flags): This function should allocate num pages starting at vnem addr for a process with a given pid. Set the protection bits of each page according to the flags.
* If any of the to-be-allocated pages were already allocated then this should kill the process. 
* deallocate pages(int pid, int vnem addr, int num pages ): This function should deallocate num pages starting at vnem addr for a process with a given pid. If any of the to be de-allocated pages were not allocated then this should kill the process.
* read mem(int pid, int vmem addr): Read 1 byte at the given virtual address for the process with the given pid, and return it. In case of illegal access, kill the process.
* write mem(int pid, int vmem addr, unsigned char byte): Write the byte at the given virtual address for the process with the given pid. In case of illegal access, kill the process.
