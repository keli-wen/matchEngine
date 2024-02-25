# Understanding File I/O: Buffered Reads and Memory Mapping

## Why is Reading with a Buffer Faster?

When dealing with file input/output (I/O) in programming, performance is often a key consideration. One common optimization technique is to read data in large chunks (buffers), rather than one byte at a time. But why does this approach improve performance?

### Reduced Disk I/O Calls

Disk I/O operations are significantly slower than most CPU operations. By reading a large buffer at once, you reduce the number of disk access calls. Each disk I/O call involves a context switch between user space and kernel space, which is relatively expensive in terms of CPU cycles.

### Leveraging Operating System Optimizations

Operating systems optimize file I/O operations through mechanisms like read-ahead. When you start reading a file, the system anticipates further reads and preloads additional data into memory. Subsequent reads can then be served from this memory cache, which is much faster than disk access.

### Minimizing Function Call Overhead

Each function call, such as `read()` in a file stream, incurs overhead including parameter passing and internal processing. Reducing the number of calls by reading larger chunks decreases this overhead.

### Efficient Use of Internal Buffering

I/O systems typically employ internal buffering mechanisms. Reading larger blocks of data at a time can make more effective use of these buffers, minimizing physical disk access.

### Exploiting Data Locality

Computer programs tend to exhibit data locality. Sequentially reading larger blocks of data aligns well with this principle, improving cache utilization.

## Why is Memory-Mapped I/O (`mmap`) Often Faster?

Memory-mapped I/O (`mmap`) provides an alternative to traditional read/write calls, offering several advantages:

### Direct Memory Access

`mmap` allows a program to access file data directly in memory, bypassing the buffer copying that occurs with standard read/write calls. This direct access can lead to performance improvements, especially for random access patterns.

### Lazy Loading

Memory mapping lazily loads file data. This means that the system reads file content into memory only as it's accessed, rather than loading the entire file upfront. This approach can be particularly efficient for large files.

### Page Cache Utilization

Memory-mapped files leverage the operating system's page cache. When multiple processes access the same file, they can share this cache, reducing overall memory usage and improving access times.

### Reduced System Calls

With `mmap`, file access is treated as memory access, significantly reducing the number of system calls. This reduction is beneficial because system calls are expensive in terms of CPU time.

### Simplified File Handling

Memory mapping simplifies the code for file manipulation. The operating system automatically handles paging of file content, making the programmer's job easier, especially when dealing with large files.

## Conclusion

Both buffered reads and memory-mapped I/O (`mmap`) provide mechanisms to improve file I/O performance. The choice between them depends on specific use cases, file sizes, access patterns, and system architecture. In general, buffered reads excel in sequential access and large read operations, while `mmap` shines in scenarios requiring random access and efficient memory utilization.

## Result

```
Cost time of reading order_log : (repeat 2) 4.30164s [no buffer]
Cost time of reading order_log with concurrent: (repeat 2 thread cnt 112) 1.10344s [no buffer]
Cost time of reading order_log: (repeat 2) 2.24096s
Cost time of reading order_log with concurrent: (repeat 2 thread cnt 112) 1.05141s
Cost time of reading order_log with mmap: (repeat 2) 1.32013s

Cost time of reading alpha: (repeat 2) 0.0022458s [no buffer]
Cost time of reading alpha with concurrent: (repeat 2 thread cnt 112) 0.00616087s [no buffer]
Cost time of reading alpha: (repeat 2) 0.00144781s
Cost time of reading alpha with concurrent: (repeat 2 thread cnt 112) 0.00318303s
Cost time of reading alpha with mmap: (repeat 2) 0.000236994s
```

## 

Talk:
- Why is read buffer faster than no buffer?
- Why is mmap faster than read buffer?
- In multi-threading scenario, should we convert the data pointer to a vector?

TODO LIST:
- [ ] `io_uring` asycn io tool.