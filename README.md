# Interview Practice

Hardware / RTL / DV / emulation interview practice. Every file is **one self-contained problem**: question at the top, solution, then self-tests.

**Run:** paste any file into [OnlineGDB](https://www.onlinegdb.com/online_c++_compiler) or [Godbolt](https://godbolt.org) (C++17), or locally `g++ -std=c++17 file.cpp && ./a.out`. It prints `All tests passed`.

**Practice tip:** read only the header comment, write your own solution, then compare with the `SOLUTION` block.

## C++

- [Bit Manipulation](#bit-manipulation) (10)
- [Pointers & Memory](#pointers--memory) (6)
- [Embedded / Memory-Mapped I/O](#embedded--memory-mapped-io) (6)
- [Hardware Data Structures](#hardware-data-structures) (7)
- [C++ Language (OOP & Modern C++)](#c-language-oop--modern-c) (11)
- [Verification Modeling in C++](#verification-modeling-in-c) (11)
- [DPI-C & Co-Simulation](#dpi-c--co-simulation) (3)
- [Parsing & Automation](#parsing--automation) (1)

### Bit Manipulation

| # | Question | Difficulty |
|---|---|---|
| 01 | [Count set bits (popcount)](cpp/01_bit_manipulation/01_count_set_bits.cpp) | Easy |
| 02 | [Power of two & address alignment](cpp/01_bit_manipulation/02_power_of_two_alignment.cpp) | Easy |
| 03 | [Reverse bits](cpp/01_bit_manipulation/03_reverse_bits.cpp) | Easy/Medium |
| 04 | [Set / clear / toggle / extract / insert fields](cpp/01_bit_manipulation/04_bit_field_ops.cpp) | Easy |
| 05 | [Endianness](cpp/01_bit_manipulation/05_endianness.cpp) | Easy |
| 06 | [Parity & Gray code](cpp/01_bit_manipulation/06_parity_gray_code.cpp) | Medium |
| 07 | [Lowest/highest set bit, one-hot](cpp/01_bit_manipulation/07_lsb_msb_onehot.cpp) | Medium |
| 08 | [Rotate, sign-extend, overflow detect](cpp/01_bit_manipulation/08_rotate_sign_extend_overflow.cpp) | Medium |
| 09 | [XOR tricks](cpp/01_bit_manipulation/09_xor_tricks.cpp) | Easy/Medium |
| 10 | [Bit interleave (Morton / Z-order)](cpp/01_bit_manipulation/10_morton_interleave.cpp) | Hard |

### Pointers & Memory

| # | Question | Difficulty |
|---|---|---|
| 01 | [Pointer basics](cpp/02_pointers_memory/01_pointer_basics.cpp) | Easy |
| 02 | [Implement memcpy / memmove / memset / memcmp](cpp/02_pointers_memory/02_memcpy_memmove.cpp) | Medium |
| 03 | [Linked list basics](cpp/02_pointers_memory/03_reverse_linked_list.cpp) | Easy |
| 04 | [Cycle detection & merge sorted lists](cpp/02_pointers_memory/04_list_cycle_merge.cpp) | Medium |
| 05 | [aligned_malloc / aligned_free](cpp/02_pointers_memory/05_aligned_malloc.cpp) | Hard |
| 06 | [C-string handling in place](cpp/02_pointers_memory/06_c_strings_in_place.cpp) | Medium |

### Embedded / Memory-Mapped I/O

| # | Question | Difficulty |
|---|---|---|
| 01 | [Memory-mapped register access with volatile](cpp/03_embedded_mmio/01_volatile_mmio.cpp) | Easy |
| 02 | [Register bit-fields vs shift/mask](cpp/03_embedded_mmio/02_bitfield_vs_mask.cpp) | Medium |
| 03 | [Struct padding, alignment, packing](cpp/03_embedded_mmio/03_struct_packing.cpp) | Medium |
| 04 | [Driver: start DMA, poll with timeout, W1C clear](cpp/03_embedded_mmio/04_poll_with_timeout.cpp) | Medium |
| 05 | [Lock-free SPSC ring buffer (ISR -> main loop)](cpp/03_embedded_mmio/05_isr_ring_buffer.cpp) | Medium |
| 06 | [Mini register model (UVM-RAL style) in C++](cpp/03_embedded_mmio/06_register_model_ral.cpp) | Hard |

### Hardware Data Structures

| # | Question | Difficulty |
|---|---|---|
| 01 | [Circular FIFO (template)](cpp/04_hw_data_structures/01_circular_fifo.cpp) | Easy/Medium |
| 02 | [Cycle model of an ASYNC FIFO](cpp/04_hw_data_structures/02_async_fifo_model.cpp) | Hard |
| 03 | [LRU cache (LeetCode 146)](cpp/04_hw_data_structures/03_lru_cache.cpp) | Medium |
| 04 | [Set-associative cache model](cpp/04_hw_data_structures/04_set_assoc_cache_model.cpp) | Hard |
| 05 | [Fixed-priority & round-robin arbiters](cpp/04_hw_data_structures/05_arbiters.cpp) | Medium |
| 06 | [Bitmap allocator & ID pool](cpp/04_hw_data_structures/06_bitmap_id_allocator.cpp) | Medium |
| 07 | [Sparse memory model with byte strobes](cpp/04_hw_data_structures/07_sparse_memory_model.cpp) | Medium |

### C++ Language (OOP & Modern C++)

| # | Question | Difficulty |
|---|---|---|
| 01 | [Rule of three (deep copy)](cpp/05_cpp_language/01_rule_of_three.cpp) | Easy/Medium |
| 02 | [Move semantics & rule of five](cpp/05_cpp_language/02_move_semantics.cpp) | Medium |
| 03 | [Virtual functions, polymorphism, slicing](cpp/05_cpp_language/03_virtual_polymorphism.cpp) | Easy |
| 04 | [Templates](cpp/05_cpp_language/04_templates.cpp) | Medium |
| 05 | [Smart pointers & RAII](cpp/05_cpp_language/05_smart_pointers.cpp) | Medium |
| 06 | [Operator overloading - a Verilog-like bit vector](cpp/05_cpp_language/06_operator_overloading.cpp) | Medium |
| 07 | [STL essentials through DV tasks](cpp/05_cpp_language/07_stl_essentials.cpp) | Easy/Medium |
| 08 | [static, const, constexpr, singleton](cpp/05_cpp_language/08_static_const_constexpr.cpp) | Medium |
| 09 | [Factory with type overrides (UVM-factory style)](cpp/05_cpp_language/09_factory_pattern.cpp) | Medium |
| 10 | [Callbacks & observer (analysis-port style)](cpp/05_cpp_language/10_callbacks_observer.cpp) | Medium |
| 11 | [Implement a simple shared_ptr](cpp/05_cpp_language/11_implement_shared_ptr.cpp) | Hard |

### Verification Modeling in C++

| # | Question | Difficulty |
|---|---|---|
| 01 | [Transaction class (sequence-item style)](cpp/06_verification_modeling/01_transaction_class.cpp) | Easy |
| 02 | [Constrained random in plain C++](cpp/06_verification_modeling/02_constrained_random.cpp) | Medium |
| 03 | [In-order scoreboard](cpp/06_verification_modeling/03_inorder_scoreboard.cpp) | Medium |
| 04 | [Out-of-order scoreboard by AXI ID](cpp/06_verification_modeling/04_ooo_scoreboard_axi_id.cpp) | Hard |
| 05 | [ALU reference model with flags](cpp/06_verification_modeling/05_reference_model_alu.cpp) | Easy/Medium |
| 06 | [Functional coverage model](cpp/06_verification_modeling/06_functional_coverage.cpp) | Medium |
| 07 | [CRC-32 and CRC-16](cpp/06_verification_modeling/07_crc.cpp) | Medium |
| 08 | [LFSR / PRBS generator and checker](cpp/06_verification_modeling/08_lfsr_prbs.cpp) | Medium |
| 09 | [Hamming SECDED ECC (13,8)](cpp/06_verification_modeling/09_hamming_secded.cpp) | Hard |
| 10 | [Cycle-based model of a valid/ready pipeline](cpp/06_verification_modeling/10_cycle_based_pipeline.cpp) | Hard |
| 11 | [AXI burst address & strobe calculator](cpp/06_verification_modeling/11_axi_burst_addresses.cpp) | Medium |

### DPI-C & Co-Simulation

| # | Question | Difficulty |
|---|---|---|
| 01 | [DPI-C import - C reference model called from SV](cpp/07_dpi_c_cosim/01_dpi_import_ref_model.cpp) | Medium |
| 02 | [DPI-C export - a C test drives the SV testbench](cpp/07_dpi_c_cosim/02_dpi_export_c_test.cpp) | Medium |
| 03 | [Why emulation is slow - batching host<->emulator calls](cpp/07_dpi_c_cosim/03_transaction_batching.cpp) | Hard |

### Parsing & Automation

| # | Question | Difficulty |
|---|---|---|
| 01 | [Parse a UVM simulation/emulation log](cpp/08_parsing_automation/01_log_first_error.cpp) | Medium |

## Coming later

- RTL design (SystemVerilog)
- Design verification (UVM, SVA, coverage)
- Python / shell automation
