# Virtual Memory Management Unit Simulator

## Project Overview

This project involves the development of a Virtual Memory Management Unit (MMU) Simulator. It's an advanced tool designed for the simulation of logical to physical address translation in a virtual memory system. The simulator effectively handles address translation using a Translation Lookaside Buffer (TLB) and a page table, incorporating concepts like demand paging, TLB management, and page-replacement algorithms.

## Features

- **Address Translation:** Simulates the process of converting logical addresses to physical addresses using TLB and page table.
- **Demand Paging:** Implements demand paging to resolve page faults, simulating real-world virtual memory management.
- **Page Table Management:** Efficient handling of a page table with 28 entries.
- **TLB Implementation:** Manages a TLB with 16 entries, employing a FIFO replacement strategy.
- **Page Replacement Algorithm:** Incorporates Least Recently Used (LRU) algorithm for page replacement in limited physical memory scenarios.
- **Memory Size Simulation:** Supports virtual address space of 65,536 bytes, with physical memory configurations of both 256 and 128 frames.

## How It Works

- The simulator reads 32-bit integer numbers representing logical addresses, focusing on the lower 16 bits divided into an 8-bit page number and an 8-bit page offset.
- It then translates these logical addresses into physical addresses, outputting the value stored at the corresponding physical address.
- The process involves checking the TLB and the page table, handling TLB misses, and resolving page faults by fetching data from a simulated backing store.

## Technical Specifications

- **Page and Frame Size:** 256 bytes
- **Physical Memory:** Configurable to simulate different sizes (256 and 128 frames).
- **Input File Format:** Reads logical addresses from `addresses.txt`.
- **Output Format:** Generates a CSV file detailing logical addresses, physical addresses, and the byte values at those addresses.
## Project Phases

### Phase 1: Basic Address Translation
- Focuses on translating logical addresses to physical addresses using a page table and TLB.
- Handles TLB misses and page faults without the need for page replacement.
- Assumes physical memory size equal to the virtual address space (65,536 bytes).

### Phase 2: Implementing Page Replacement
- Simulates a more constrained physical memory scenario with 128 page frames instead of 256.
- Implements the Least Recently Used (LRU) page-replacement algorithm to manage limited physical memory.
- Involves modifying the program to track free page frames and manage page faults when memory is full.
## Usage

- To compile: `make mmu`
- To run phase 1: `./mmu 256 BACKING_STORE.bin addresses.txt`
- To run phase 2: `./mmu 128 BACKING_STORE.bin addresses.txt`

## Evaluation Metrics

- **Page-fault rate:** Percentage of address references resulting in page faults.
- **TLB hit rate:** Percentage of address references resolved in the TLB.

## Getting Started

Please refer to the `BACKING_STORE.bin`, `addresses.txt`, and the `test.sh` script included in the repository to understand the implementation and testing process.

## Deliverables

The project includes:

- `Makefile`
- C source and header files
- `BACKING_STORE.bin` (Simulated backing store)
- `addresses.txt` (Test input)
- `test.sh` (Test script)
- Sample output files: `correct128.csv`, `correct256.csv`
