#include "mmu.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

// byte addressable memory
unsigned char RAM[RAM_SIZE];  


// OS's memory starts at the beginning of RAM.
// Store the process related info, page tables or other data structures here.
// do not use more than (OS_MEM_SIZE: 72 MB).
unsigned char* OS_MEM = RAM;  

// memory that can be used by processes.   
// 128 MB size (RAM_SIZE - OS_MEM_SIZE)
unsigned char* PS_MEM = RAM + OS_MEM_SIZE; 


// This first frame has frame number 0 and is located at start of RAM(NOT PS_MEM).
// We also include the OS_MEM even though it is not paged. This is 
// because the RAM can only be accessed through physical RAM addresses.  
// The OS should ensure that it does not map any of the frames, that correspond
// to its memory, to any process's page. 
int NUM_FRAMES = ((RAM_SIZE) / PAGE_SIZE);

// Actual number of usable frames by the processes.
int NUM_USABLE_FRAMES = ((RAM_SIZE - OS_MEM_SIZE) / PAGE_SIZE);

// To be set in case of errors. 

int error_no;

struct global_variable
{
	int id;
	int framearray[32768];
	int memsize;
	struct PCB tmp[100];
	struct PCB* process;
	
} g_v;



void os_init()
{
    // TODO student 
    // initialize your data structures.
	
	g_v.id=0;
	g_v.memsize=0;
	g_v.process=g_v.tmp;
	
	for(int i=0;i<100;i++)
	{
		g_v.process[i].pid=-1;
	}
	for(int i=0;i<32768;i++)
	{
		g_v.framearray[i]=0;
	}
	
	memcpy(&OS_MEM[g_v.memsize],g_v.framearray,sizeof(int)*32768);
	g_v.memsize+=sizeof(int)*100;
	memcpy(&OS_MEM[g_v.memsize],g_v.process,sizeof(struct PCB)*100);
	g_v.memsize+=sizeof(struct PCB)*100;
}


// ----------------------------------- Functions for managing memory --------------------------------- //

/**
 *  Process Virtual Memory layout: 
 *  ---------------------- (virt. memory start 0x00)
 *        code
 *  ----------------------  
 *     read only data 
 *  ----------------------
 *     read / write data
 *  ----------------------
 *        heap
 *  ----------------------
 *        stack  
 *  ----------------------  (virt. memory end 0x3fffff)
 * 
 * 
 *  code            : read + execute only
 *  ro_data         : read only
 *  rw_data         : read + write only
 *  stack           : read + write only
 *  heap            : (protection bits can be different for each heap page)
 * 
 *  assume:
 *  code_size, ro_data_size, rw_data_size, max_stack_size, are all in bytes
 *  code_size, ro_data_size, rw_data_size, max_stack_size, are all multiples of PAGE_SIZE
 *  code_size + ro_data_size + rw_data_size + max_stack_size < PS_VIRTUAL_MEM_SIZE
 *  
 * 
 *  The rest of memory will be used dynamically for the heap.
 * 
 *  This function should create a new process, 
 *  allocate code_size + ro_data_size + rw_data_size + max_stack_size amount of physical memory in PS_MEM,
 *  and create the page table for this process. Then it should copy the code and read only data from the
 *  given `unsigned char* code_and_ro_data` into processes' memory.
 *   
 *  It should return the pid of the new process.  
 *  
 */



int create_ps(int code_size, int ro_data_size, int rw_data_size,int max_stack_size, unsigned char* code_and_ro_data) 
{   
    // TODO student
	int frames = (code_size + ro_data_size + rw_data_size + max_stack_size)/(4096);
	for(int i=0;i<100;i++)
	{
		if(g_v.process[i].marker==0)
		{
			g_v.process[i].pid=g_v.id;
			g_v.id++;
			g_v.process[i].marker=1;
			g_v.process[i].page_table=(page_table_entry*) (&OS_MEM[g_v.memsize]);
			g_v.memsize+=sizeof(unsigned int)*1024;
			int total=0;
			for(int j=0;total<frames && j<32768;j++)
			{
				if(g_v.framearray[j]==0)
				{
					g_v.framearray[j]=1;
					if(total<code_size/4096)
					{
						memcpy(&PS_MEM[j*4096],(code_and_ro_data+total*4096),4096);
						g_v.process[i].page_table[total]=support1(g_v.process[i].page_table[total],j);
					}
					else if(total<(code_size + ro_data_size)/4096)
					{
						memcpy(&PS_MEM[j*4096],(code_and_ro_data+total*4096),4096);
						g_v.process[i].page_table[total]=support2(g_v.process[i].page_table[total],j);
					}
					else if(total<(code_size + ro_data_size + rw_data_size)/4096)
					{
					    g_v.process[i].page_table[total]=support3(g_v.process[i].page_table[total],j);
					}
					else
					{
						g_v.process[i].page_table[1024+total-frames]=support4(g_v.process[i].page_table[1024+total-frames],j);
					}
					total++;
                
				}
            }
			
			for(int j=(code_size + ro_data_size + rw_data_size)/4096;j<1024-((max_stack_size)/4096);j++)
			{
				g_v.process[i].page_table[j]=0;
			}
			if(total!=frames)
			{
				return 0;
			}
			return g_v.process[i].pid;
		}
	}
    return 0;
}

/**
 * This function should deallocate all the resources for this process. 
 * 
 */
void exit_ps(int pid) 
{
   // TODO student
	for(int i=0;i<100;i++)
	{
		if(g_v.process[i].pid==pid)
		{
			g_v.process[i].marker=0;
			g_v.process[i].pid=-1;
			for(int j=0;j<1024;j++)
			{
				int frame = pte_to_frame_num(g_v.process[i].page_table[j]);
				g_v.framearray[frame] =0;
			}
			g_v.process[i].page_table=NULL;
			break;
		}
	}
}



/**
 * Create a new process that is identical to the process with given pid. 
 * 
 */
int fork_ps(int pid) 
{

    // TODO student:
    int c=0;
	for(int i=0;i<100;i++)
	{
		if(g_v.process[i].pid==pid)
		{
			c=i;
			for(int j=0;j<100;j++)
			{
				if(g_v.process[j].marker==0 && c!=j)
				{
					g_v.process[j].pid=g_v.id;
					g_v.id++;
					g_v.process[j].marker=1;
					page_table_entry pt[1024];
					for(int j=0;j<1024;j++)
					{
						pt[j]=g_v.process[i].page_table[j];
					}
					g_v.process[j].page_table=pt;				
					return g_v.process[j].pid; 
					break;
				}
		    }
		}
	}
    return 0;
}



// dynamic heap allocation
//
// Allocate num_pages amount of pages for process pid, starting at vmem_addr.
// Assume vmem_addr points to a page boundary.  
// Assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE
//
//
// Use flags to set the protection bits of the pages.
// Ex: flags = O_READ | O_WRITE => page should be read & writeable.
//
// If any of the pages was already allocated then kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
void allocate_pages(int pid, int vmem_addr, int num_pages, int flags) 
{
   // TODO student
   int starting = vmem_addr/4096;
   int ending = starting+num_pages;
   for(int i=0;i<100;i++)
   {
	   if(g_v.process[i].pid==pid)
	   {
		   if(((vmem_addr/4096)+num_pages)>=1024)
		   {
			   error_no= ERR_SEG_FAULT;
			   exit_ps(pid);
			   return;
		   }
		   else
		   {
			   for(int j=0;j<32768 && starting<ending;j++)
			   {
				   if(g_v.framearray[j]==0)
				   {
					   int bit = is_present(g_v.process[i].page_table[starting]);
					   if(bit==0)
					   {
						   g_v.process[i].page_table[starting] = j*4096 | (1<<30) | flags<<27;
						   g_v.framearray[j]=1;
						   starting++;
					   }
					   else
					   {
						   error_no= ERR_SEG_FAULT;
						   exit_ps(pid);
						   return;
					   }
				   }
			   }
		   }
	   }
	}
}



// dynamic heap deallocation
//
// Deallocate num_pages amount of pages for process pid, starting at vmem_addr.
// Assume vmem_addr points to a page boundary
// Assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE

// If any of the pages was not already allocated then kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
void deallocate_pages(int pid, int vmem_addr, int num_pages) 
{
   // TODO student
   int starting = vmem_addr/4096;
   int ending = starting+num_pages;
   for(int i=0;i<100;i++)
   {
	   if(g_v.process[i].pid==pid)
	   {
		   if(ending>=1024)
		   {
			   error_no= ERR_SEG_FAULT;
			   exit_ps(pid);
			   return;
		   }
		   else
		   {
			   for(int j=starting;j<ending;j++)
			   {
				       int bit = is_present(g_v.process[i].page_table[j]);
					   if(bit!=0)
					   {
						   int frame = pte_to_frame_num(g_v.process[i].page_table[j]);
						   g_v.framearray[frame]=0;
						   g_v.process[i].page_table[j] = (g_v.process[i].page_table[j] & ~(1<<30));
					   }
					   else
					   {
						   error_no= ERR_SEG_FAULT;
						   exit_ps(pid);
						   return;
					   }
			   }
			   
		   }
	   }
   }
}

// Read the byte at `vmem_addr` virtual address of the process
// In case of illegal memory access kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
// 
// assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE
unsigned char read_mem(int pid, int vmem_addr) 
{
    // TODO: student
	for(int j=0;j<100;j++)
	{
		if(g_v.process[j].pid==pid)
		{
			page_table_entry bit = g_v.process[j].page_table[vmem_addr/4096];
			if(is_present(bit)==0 || is_readable(bit)==0)
			{
				error_no= ERR_SEG_FAULT;
				exit_ps(pid);
				return 0;
			}
			if((vmem_addr/4096)>=1024)
			{
				error_no= ERR_SEG_FAULT;
				exit_ps(pid);
				return 0;
			}
			unsigned int frame = support(bit)+(vmem_addr%4096);
			return PS_MEM[frame];
		}
	}
	return 0;
}

// Write the given `byte` at `vmem_addr` virtual address of the process
// In case of illegal memory access kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
// 
// assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE
void write_mem(int pid, int vmem_addr, unsigned char byte) 
{
    // TODO: student
	//searching in processay which pid matches with the given pid
	for(int j=0;j<100;j++)
	{
		if(g_v.process[j].pid==pid)
		{
			page_table_entry bit = g_v.process[j].page_table[vmem_addr/4096];
			if(is_present(bit)==0 || is_writeable(bit)==0)
			{
				error_no = ERR_SEG_FAULT;
				exit_ps(pid);
				return ;
			}
			if((vmem_addr/4096)>=1024)
			{
				error_no= ERR_SEG_FAULT;
				exit_ps(pid);
				return ;
			}
			unsigned int frame = support(bit)+(vmem_addr%4096);
			PS_MEM[frame]=byte;
			break;
		}
	}
}


// ---------------------- Helper functions for Page table entries ------------------ // 

// return the frame number from the pte

page_table_entry support1(page_table_entry pte,int j)
{
	pte=j*4096 | 1<<30 | 1<<27 | 1<<29;
	return pte;
}

page_table_entry support2(page_table_entry pte,int j)
{
	pte=j*4096 | 1<<30 | 1<<27 ;
	return pte;
}

page_table_entry support3(page_table_entry pte,int j)
{
	pte=j*4096 | 1<<30 | 1<<27 | 1<<28;
	return pte;
}

page_table_entry support4(page_table_entry pte,int j)
{
	pte=j*4096 | 1<<30 | 1<<27 | 1<<28;
	return pte;
}

int support(page_table_entry pte) 
{
    // TODO: student
	int a[32];
	int frame_no=0;
	for(int i=0;i<32;i++)
	{
		a[i]=pte%2;
		pte/=2;
		if(i<27)
		{
			int c = pow(2,i);
			frame_no+=a[i]*c;
		}
	}
    return (frame_no);
}

int pte_to_frame_num(page_table_entry pte) 
{
    // TODO: student
	//returning the frame number from pte
	int a[32];
	int frame_no=0;
	for(int i=0;i<32;i++)
	{
		a[i]=pte%2;
		pte/=2;
		if(i<27)
		{
			int c = pow(2,i);
			frame_no+=a[i]*c;
		}
	}
    return (frame_no)/PAGE_SIZE;
}


// return 1 if read bit is set in the pte
// 0 otherwise
int is_readable(page_table_entry pte)
{
    // TODO: student
	//setting 27th bit as read bit and checking if it is set or not
	if(((pte)&(1<<27))!=0)
	{
		return 1;
	}
    return 0;
}

// return 1 if write bit is set in the pte
// 0 otherwise
int is_writeable(page_table_entry pte)
{
    // TODO: student
	//setting 28th bit as write bit and checking if it is set or not
    if(((pte)&(1<<28))!=0)
	{
		return 1;
	}
    return 0;
}

// return 1 if executable bit is set in the pte
// 0 otherwise
int is_executable(page_table_entry pte) 
{
    // TODO: student
	//setting 29th bit as executable bit and checking if it is set or not
    if(((pte)&(1<<29))!=0)
	{
		return 1;
	}
    return 0;
}


// return 1 if present bit is set in the pte
// 0 otherwise
int is_present(page_table_entry pte) 
{
    // TODO: student
	//setting 30th bit as present bit and checking if it is set or not
    if(((pte)&(1<<30))!=0)
	{
		return 1;
	}
    return 0;
}

// -------------------  functions to print the state  --------------------------------------------- //

void print_page_table(int pid) 
{
	//finding in the processay which process has the PID equal to pid
	int c;
	for(c=0;c<100;c++)
	{
		if(g_v.process[c].pid==pid)
		{
			break;
		}	
		
	}
    
    page_table_entry* page_table_start = g_v.process[c].page_table; // TODO student: start of page table of process pid
    int num_page_table_entries = 1024;           // TODO student: num of page table entries
    
	
    // Do not change anything below
    puts("------ Printing page table-------");
    for (int i = 0; i < num_page_table_entries; i++) 
    {
        page_table_entry pte = page_table_start[i];
        printf("Page num: %d, frame num: %d, R:%d, W:%d, X:%d, P%d\n", 
                i, 
                pte_to_frame_num(pte),
                is_readable(pte),
                is_writeable(pte),
                is_executable(pte),
                is_present(pte)
                );
		
    }
}

