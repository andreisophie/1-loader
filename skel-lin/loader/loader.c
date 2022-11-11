/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "exec_parser.h"
#include "debug.h"

static so_exec_t *exec;
static struct sigaction old_action;
static int page_size;
static int fd;

// handler
void handler(int signal, siginfo_t *info, void *context)
{
	// if signal is not segfault, use old handler
	if (signal != SIGSEGV) {
		old_action.sa_sigaction(signal, info, context);
		return;
	}

	// address that generated error
	uintptr_t addr = (uintptr_t)info->si_addr;

	unsigned int page_index;
	int *seg_data;
	so_seg_t *seg;

	// search for address in each segment from exec
	for (int i = 0; i < exec->segments_no; i++) {
		// current segment
		seg = &(exec->segments[i]);
		// boolean vector that remembers whether segment was loaded in memory
		seg_data = exec->segments[i].data;

		// check if address that threw segfault is inside current segment
		if (seg->vaddr <= addr && addr < seg->vaddr + seg->mem_size) {
			// get page index
			page_index = (addr - seg->vaddr) / page_size;

			// check if page is not allocated
			if (seg_data[page_index] == 0) {
				unsigned int page_addr = page_index * page_size;
				// allocate virtual memory for accessed page
				mmap((void *)(seg->vaddr + page_addr), page_size,
					 PROT_WRITE, MAP_SHARED | MAP_FIXED_NOREPLACE | MAP_ANONYMOUS, 0, 0);

				/*
				* Copy data from exec to the new page
				*/
				if (seg->file_size > page_addr) {
					lseek(fd, seg->offset + page_addr, SEEK_SET);
					if (seg->file_size < page_addr + page_size) {
						read(fd, (void *)(seg->vaddr + page_addr), seg->file_size - page_addr);
					} else {
						read(fd, (void *)(seg->vaddr + page_addr), page_size);
					}
				}


				mprotect((void *)(seg->vaddr + page_addr), page_size, seg->perm);

				// mark page as allocated
				seg_data[page_index] = 1;

			} else {
				// if page is already allocated, use default handler
				old_action.sa_sigaction(signal, info, context);
			}

			return;
		}
	}

	//if address in not from this program, execute default handler
	old_action.sa_sigaction(signal, info, context);
}

int so_init_loader(void)
{

	// sigaction struct that contains new handler
	struct sigaction new_sigaction;

	// set flags to trigger routine on SIGSEGV
	sigemptyset(&new_sigaction.sa_mask);
	sigaddset(&new_sigaction.sa_mask, SIGSEGV);

	// set new handler for segfault
	new_sigaction.sa_sigaction = handler;

	// set flag so that sa_sigaction is used instead of sa_handler
	new_sigaction.sa_flags = SA_SIGINFO;

	// syscall to set new routine for segfault exception
	// and get the old routine
	sigaction(SIGSEGV, &new_sigaction, &old_action);
	return 0;
}

int so_execute(char *path, char *argv[])
{
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	// get page size
	page_size = getpagesize();

	// open exec file from disk to read data
	fd = open(path, O_RDONLY);

	// allocate exec->segments[i].data as "boolean" vectors that show whether
	// each page was allocated
	int page_nr;
	for (int i = 0; i < exec->segments_no; i++) {
		page_nr = exec->segments[i].mem_size / page_size;
		// use calloc to initialize with 0
		exec->segments[i].data = calloc(page_nr, sizeof(int));
	}

	so_start_exec(exec, argv);

	return 1;
}
