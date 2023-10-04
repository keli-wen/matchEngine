## `mmap` Basic Introduction

Firstly, in which header file is `mmap` located, and which systems is it applicable to?

> A: The header file is: `#include <sys/mman.h>`. `mmap`, `munmap` - map or unmap files or devices into memory, primarily for Unix and Unix-like systems such as Linux and macOS.

Secondly, what is the specific function signature? What do the optional parameters mean?

> ```c++
> void *mmap(void addr[.length], size_t length, int prot, int flags,
>            int fd, off_t offset);
> int munmap(void addr[.length], size_t length);
> ```

Next, I will describe the parameters in detail:

The contents of a file mapping (as opposed to an anonymous mapping; see `MAP_ANONYMOUS` in the [url](https://man7.org/linux/man-pages/man2/mmap.2.html)), are initialized using `length` bytes starting at `offset` offset in the file (or other object) referred to by the file descriptor `fd`.  offset must be a multiple of the page size as returned by `sysconf(_SC_PAGE_SIZE)`.

### `prot`

The prot argument describes the desired memory **prot**ection of the mapping (and must not conflict with the open mode of the file). It is either PROT_NONE or the bitwise OR of one or more of the following flags, such as (`PROT_READ | PROT_WRITE`)

- `PROT_EXEC`: Pages may be executed.
- `PROT_READ`: Pages may be read.
- `PROT_WRITE`: Pages may be written.
- `PROT_NONE`: Pages may not be accessed.

Each of these flags contains many **subtleties**.

`PROT_NONE` is used to configure a memory region that any attempts to read, write, or excute operations within that region will trigger a segmentation fault. However, this raises the question: “Why do we need `PROT_NONE`?”

> 给出一些建设性的回答：
>
> 1. **保留地址空间：** 使用 `PROT_NONE` 可以预先为进程保留一块地址空间，但阻止任何实际访问，直到进程确实需要这些内存为止。然后，你可以使用 `mprotect()` 系统调用来更改这块内存区域的权限。
> 2. **内存保护：** 如果你想在某个时间点“冻结”内存的一部分以防止其被修改（或者读取或执行），你可以临时地设置其权限为 `PROT_NONE`。
> 3. **调试和故障排除：** 在开发阶段，你可能想更容易地捕获对某些内存区域的非法访问，以便进行调试。
> 4. **堆栈保护：** 在许多现代操作系统中，紧邻堆栈的内存页面常常被设置为 `PROT_NONE`，这样任何堆栈溢出的尝试都会立即导致段错误，而不是默默地覆盖相邻的内存。
>
> 同时可以参考 [stackoverflow](https://stackoverflow.com/questions/12916603/what-s-the-purpose-of-mmap-memory-protection-prot-none) 的相关回答:
>
> `PROT_NONE` can be used to implement [guard pages](https://www.us-cert.gov/bsi/articles/knowledge/coding-practices/guard-pages), Microsoft has the same concept ([MSDN](http://msdn.microsoft.com/en-us/library/windows/desktop/aa366549(v=vs.85).aspx)).
>
> To quote the first link:
>
> *... allocation of additional inaccessible memory during memory allocation operations is a technique for mitigating against exploitation of heap buffer overflows. These guard pages are unmapped pages placed between all memory allocations of one page or larger. The guard page causes a segmentation fault upon any access.*
>
> Thus useful in implementing protection for areas such as network interfacing, virtual machines, and interpreters. An example usage: [pthread_attr_setguardsize, pthread_attr_getguardsize](http://www.kernel.org/doc/man-pages/online/pages/man3/pthread_attr_getguardsize.3.html).
>
> 因此，虽然 `PROT_NONE` 在第一眼看起来似乎没什么用，但它在系统编程和内存管理方面确实有一系列有效的应用场景。

`PROT_WRITE` is utilized to specify whether write operations can be performed on the corresponding memory. When invoking `mmap()` with the `PROT_WRITE` flag for memory mapping, whether the original content of the file will be altered depends on how we configure other parameters within `mmap()`. The impact on the original content of the file with respect to the `flags` parameter will be discussed later.

The beginner like me may can’t figure out the difference between `PROT_EXEC` and `PROT_READ`.  For `PROT_EXEC`, we can directly execute the code in the memory. A more specific example is as follows:

```cpp
// You can execute code with PROT_EXEC, but you cannot read or write
char *ptr = mmap(..., PROT_EXEC, ...);
void (*func)() = (void (*)())ptr;
func();  // This is valid.
char byte = ptr[0];  // This will result in a segment fault (unless PROT_READ is also set)
```

If you use `PROT_READ`, you can read the contents of this memory area just like you would read an ordinary array or buffer. You can view the bytes in it, but you can't **execute** them as code. If you try to jump to this memory area to execute code, the operating system will give a segmentation fault. A more specific example is as followings:

```cpp
// You can read with PROT_READ, but you can't execute it.
char *ptr = mmap(..., PROT_READ, ...);
char byte = ptr[0];  // This is valid.
void (*func)() = (void (*)())ptr;
func();  // This will result in a segment fault (unless PROT_READ is also set)
```

### `flags`

The flags argument determines whether updates to the mapping are visible to other processes mapping the same region, and whether updates are carried through to the underlying file.  This behavior is determined by including exactly one of the following values in flags:

There are several optioanl values, i will primaryly to introduce the following three values:

`MAP_SHARED`: When you update the mapped region, those changes are visible to other processes that also mapped the same area. Changes are also saved to the underlying file if it’s a filed-backed mapping (to precisely control when updates are carried through to the underlying file requires the use of `msync(2)`). (⚠️ **When using `MAP_SHARED`, enusre that the memory access permission of the mapping area are compatible with the file open mode.)**

`MAP_SHARED_VALIDATE`: Similar to `MAP_SHARED`, but it fails the mappling if any unkown flags are passed. This is more strict and ensures you’re only using known flags. It’s Linux-specific feature available since Linux 4.15.

`MAP_PRIVATE`: This gives you a private copy-on-write mappin. Any updates you make won’t be visible to other processes and won’t be saved to the underlying file.

## Example#1: Demonstrate the usage of `mmap`.

```cpp
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

int
main(int argc, char *argv[])
{
   int          fd;
   char         *addr;
   off_t        offset, pa_offset;
   size_t       length;
   ssize_t      s;
   struct stat  sb;

   if (argc < 3 || argc > 4) {
       fprintf(stderr, "%s file offset [length]\n", argv[0]);
       exit(EXIT_FAILURE);
   }

   fd = open(argv[1], O_RDONLY);
   if (fd == -1)
       handle_error("open");

   if (fstat(fd, &sb) == -1)           /* To obtain file size */
       handle_error("fstat");

   offset = atoi(argv[2]);
   pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
   /* offset for mmap() must be page aligned */

   if (offset >= sb.st_size) {
       fprintf(stderr, "offset is past end of file\n");
       exit(EXIT_FAILURE);
   }

   if (argc == 4) {
       length = atoi(argv[3]);
       if (offset + length > sb.st_size)
           length = sb.st_size - offset;
               /* Can't display bytes past end of file */

   } else {    /* No length arg ==> display to end of file */
       length = sb.st_size - offset;
   }

   addr = (char*) mmap(NULL, length + offset - pa_offset, PROT_READ,
                       MAP_PRIVATE, fd, pa_offset);
   if (addr == MAP_FAILED)
       handle_error("mmap");

   s = write(STDOUT_FILENO, addr + offset - pa_offset, length);
   if (s != length) {
       if (s == -1)
           handle_error("write");

       fprintf(stderr, "partial write");
       exit(EXIT_FAILURE);
   }

   munmap(addr, length + offset - pa_offset);
   close(fd);

   exit(EXIT_SUCCESS);
}
```

1.**Why do wee need to define `#define handle_error(msg)` the macro?** 

The `handle_error` macro is used for error handling, providing a convenient way to print an error message and then exit the program. This is often used in code examples or shorter programs to avoid complicating the main logic with error-handling details.

2.**Why `do { ... } while (0)`?**

The `do { ... } while (0)` construct is a common idiom for writing macros that can be used safely as a single statement in C/C++ programs. This ensures that the macro behaves correctly when used in conditionals.

A macro is useful here because it allows you to reuse this snippet of code wherever you need it, without having to write it out each time. 

## Example#2: How to use it in practice (in my `MatchEngine`)



## Reference

- [mmap(2) - Linux manual page](https://man7.org/linux/man-pages/man2/mmap.2.html)