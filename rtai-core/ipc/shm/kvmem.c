/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <linux/slab.h>

#include <rtai_shm.h>

/* allocate user space mmapable block of memory in the kernel space */
void *rvmalloc(unsigned long size)
{
	void *mem;
	unsigned long adr;
        
	if ((mem = vmalloc(size))) {
	        adr = (unsigned long)mem;
		while (size > 0) {
			mem_map_reserve(virt_to_page(__va(kvirt_to_pa(adr))));
			adr  += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}
	return mem;
}

void rvfree(void *mem, unsigned long size)
{
        unsigned long adr;
        
	if ((adr = (unsigned long)mem)) {
		while (size > 0) {
			mem_map_unreserve(virt_to_page(__va(kvirt_to_pa(adr))));
			adr  += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
		vfree(mem);
	}
}

/* this function will map (fragment of) rvmalloc'ed memory area to user space */
int rvmmap(void *mem, unsigned long memsize, struct vm_area_struct *vma) {
	unsigned long pos, size, offset;
	unsigned long start  = vma->vm_start;

	/* this is not time critical code, so we check the arguments */
	/* vma->vm_offset HAS to be checked (and is checked)*/
	if (vma->vm_pgoff > (0x7FFFFFFF >> PAGE_SHIFT)) {
		return -EFAULT;
	}
	offset = vma->vm_pgoff << PAGE_SHIFT;
	size = vma->vm_end - start;
	if ((size + offset) > memsize) {
		return -EFAULT;
	}
	pos = (unsigned long)mem + offset;
	if (pos%PAGE_SIZE || start%PAGE_SIZE || size%PAGE_SIZE) {
		return -EFAULT;
	}
	while (size > 0) {
		if (remap_page_range(start, kvirt_to_pa(pos), PAGE_SIZE, PAGE_SHARED)) {
			return -EAGAIN;
		}
		start += PAGE_SIZE;
		pos   += PAGE_SIZE;
		size  -= PAGE_SIZE;
	}
	return 0;
}

/* allocate user space mmapable block of memory in the kernel space */
void *rkmalloc(int *memsize, int suprt)
{
	unsigned long mem, adr, size;
        
	if ((mem = (unsigned long)kmalloc(*memsize, suprt))) {
		adr  = PAGE_ALIGN(mem);
		size = *memsize -= (adr - mem);
		while (size > 0) {
			mem_map_reserve(virt_to_page(adr));
			adr  += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}
	return (void *)mem;
}

void rkfree(void *mem, unsigned long size)
{
        unsigned long adr;
        
	if ((adr = (unsigned long)mem)) {
		adr  = PAGE_ALIGN((unsigned long)mem);
		while (size > 0) {
			mem_map_unreserve(virt_to_page(adr));
			adr  += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
		kfree(mem);
	}
}

/* this function will map an rkmalloc'ed memory area to user space */
int rkmmap(void *mem, unsigned long memsize, struct vm_area_struct *vma) {
	unsigned long pos, size, offset;
	unsigned long start  = vma->vm_start;

	/* this is not time critical code, so we check the arguments */
	/* vma->vm_offset HAS to be checked (and is checked)*/
	if (vma->vm_pgoff > (0x7FFFFFFF >> PAGE_SHIFT)) {
		return -EFAULT;
	}
	offset = vma->vm_pgoff << PAGE_SHIFT;
	size = vma->vm_end - start;
	if ((size + offset) > memsize) {
		return -EFAULT;
	}
	pos = (unsigned long)mem + offset;
	if (pos%PAGE_SIZE || start%PAGE_SIZE || size%PAGE_SIZE) {
		return -EFAULT;
	}
	if (remap_page_range(start, virt_to_phys((void *)pos), size, PAGE_SHARED)) {
		return -EAGAIN;
	}
	return 0;
}
