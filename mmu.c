// ==============================================================================
/**
 * mmu.c
 */
// ==============================================================================



// ==============================================================================
// INCLUDES

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "mmu.h"
#include "vmsim.h"
// ==============================================================================



// ==============================================================================
// MACROS AND GLOBALS

/** 
 * The (real) address of the upper page table.  Initialized by a call
 * to `mmu_init()`.
 */
static vmsim_addr_t upper_pt_addr = 0;
// ==============================================================================



// ==============================================================================
void mmu_init (vmsim_addr_t new_upper_pt_addr) {

  upper_pt_addr = new_upper_pt_addr;
  
}
// ==============================================================================



// ==============================================================================
vmsim_addr_t mmu_translate (vmsim_addr_t sim_addr) {

  /** Find upper and lower page indices, as well as the offset. */
  uint32_t upper_index = sim_addr >> 22;
  uint32_t lower_index = (sim_addr >> 12) & 0x3ff;
  uint32_t offset = sim_addr & 0xfff;

  /** Find the address for the lower page table using the upper page table. */
  uint32_t lower_pt_addr;
  uint32_t correct_addr = upper_pt_addr + (upper_index * sizeof(lower_pt_addr));
  
  /** Read the page table entry at that address. */
  vmsim_read_real(&lower_pt_addr, correct_addr, sizeof(lower_pt_addr));

  /** If there's no lower page table to which the address' upper page table
   *  leads, then call vmsim_map_fault() and re-attempt the translation. */
  if (lower_pt_addr == 0) {
    vmsim_map_fault(sim_addr);
    return mmu_translate(sim_addr);
  }

  /** Find the address for the page using the lower page table. */
  uint32_t page_addr;
  correct_addr = lower_pt_addr + (lower_index * sizeof(page_addr));

  /** Read the page table entry at that address. */
  vmsim_read_real(&page_addr, correct_addr, sizeof(page_addr));

  /** If there is no real page to which the address' lower page table 
   *  leads, then call vmsim_map_fault() and re-attempt the translation.*/
  if (page_addr == 0) {
    vmsim_map_fault(sim_addr);
    return mmu_translate(sim_addr);
  }

  /** Finally, we can return the desired address, by adding the offset. */
  correct_addr = page_addr | offset;

  return (vmsim_addr_t) correct_addr;

}
// ==============================================================================
