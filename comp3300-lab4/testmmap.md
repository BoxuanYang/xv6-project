# mmap/munmap tests

Here are a brief description of each test for mmap/munmap. The provided `testmmap.c` contains sample tests for each category -- the actual assessment will use a variant of these tests, but the categories to be tested are the same. 

- Test 1: basic mmap functionality. An mmap of one page (with read/write permission) within the expected range should return successfully. The mmap-ed page can be written to and be read back correctly. 

- Test 2: mmap should return an address within the valid range. 

- Test 3: mmap bound checks. Mmap with length <= 0 should return an error. 

- Test 4: mmap hint. mmap should try its best to follow the hint. If the provided range is within the valid range, mmap should try to follow the hint, provided the address is page-aligned. A hint that is not NULL and outside the valid range should be ignored.

- Test 5: mmap-ed pages should be zero-ed. 

- Test 6: mmap permission tests. An mmap-ed address with read-only flag cannot be written to. 

- Test 7: small mmap functionality tests. Mmap a small number of pages (> 1), test writes and read back. 

- Test 8: `mmap(addr, len)` should always return  "the next available address after `addr`". Basic tests. 

- Test 9: same as 8, but with fragmented blocks (blocks with holes)

- Test 10: mmap large block. Mmap a large block of memory, test writes and read back. 

- Test 11: basic unmap functionality. An mmap of one page followed by an munmap on the returned address of `mmap` should render the address inaccessible (so a trap 14 error will be triggered when attempting to access that address). 

- Test 12: munmap bound checks. An munmap in the kernel space should return an error.

- Test 13: munmap unaligned address. This should return an error. 

- Test 14: simple map/unmap check. Mapping a small number of pages ( > 1) and unmapping them should work correctly. Further references to any of the unmapped pages will trigger a trap 14 error. 

- Test 15: munmap large blocks. Mapping a large number of pages and unmap them. 

- Test 16: munmap an unmapped page in the user address space is okay (no error returned). 

- Test 17: munmap with holes. Mapping an address range which contains an unmapped page in the middle should unmap all pages. So for example, if we have the following pages:

  ```
  addr:   a1   a2    a3   a4
           [ x ][    ][ x  ]
  ```

  where `[ x ]` denotes a mapped page, then calling munmap(a1, 3 * 4096) should unmap all three pages.

- Test 18: munmap should recycle unmapped page(s). Test for one page. 

- Test 19: same as test 18, but test for multiple pages. 

- Test 20: stress test. Run a large iterations of mmap and munmap. If it doesn't crash, then it passes the test.  

