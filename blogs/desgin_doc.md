# Match Engine Design Document

## Why `robin_hood` hash table?

> 推荐阅读这篇博客分析的很仔细：[Robin Hood Hashing 源码分析](https://sf-zhou.github.io/programming/robin_hood_hashing.html)

### Considered Options
The feasible options for hash table implementations include:
- `std::unordered_map` from STL.
- `robin_hood::unordered_map`.
- Other hash table implementations such as: (omit in this doc)
  - `boost::unordered_map`
  - `google::dense_hash_map`
  - `sparsepp::sparse_hash_map`

Below is a detailed comparison between `std::unordered_map` and `robin_hood::unordered_map`.

| Feature                           | `std::unordered_map`                  | `robin_hood::unordered_map`           |
|-----------------------------------|---------------------------------------|---------------------------------------|
| **Collision Resolution**          | Chaining                              | Open addressing with Robin Hood       |
| **Memory Layout**                 | Non-contiguous                        | Contiguous                            |
| **Cache Efficiency**              | Low (due to chaining)                 | High                                  |
| **Insert Performance**            | Average O(1), worst-case O(n)         | Average O(1), worst-case O(n)         |
| **Search Performance**            | Average O(1), worst-case O(n)         | Average O(1), more stable under load  |
| **Deletion Performance**          | Efficient (chaining)                  | Complex (needs reorganization)        |
| **Memory Overhead**               | High (due to pointers in chaining)    | Lower                                 |
| **Load Factor Handling**          | Needs rehashing                       | Better handling at higher load factors|
| **Implementation Complexity**     | Simple                                | Complex                               |

### Why Choose `robin_hood::unordered_map`?

After evaluating the options, `robin_hood::unordered_map` is chosen for the following reasons:

1. **Cache Efficiency:** The contiguous memory layout significantly improves cache locality, leading to better performance, especially under high load factors.
2. **Stable Performance:** The Robin Hood strategy ensures more stable and predictable performance, reducing the risk of long probe sequences.
3. **Memory Efficiency:** Lower memory overhead compared to chained hashing, making it more suitable for scenarios with large datasets.

### 🔥 Robin Hood Strategy

Robin Hood strategy is a collision resolution method used in open addressing hash tables. The core idea is to balance the probe sequence lengths, making the probe path length for each key as uniform as possible. Specifically, when inserting a new element, if the probe path length of this element is greater than the probe path length of the existing element at the current bucket, the positions are swapped. This is akin to Robin Hood "robbing the rich to give to the poor," reallocating positions to elements with longer probe sequences to reduce the overall variance in probe path lengths.

#### Operation Steps

**Insertion:**
1. Compute the hash value of the element to be inserted and determine its initial position.
2. If the position is empty, insert the element directly.
3. If the position is occupied, compare the probe distance of the current element with the probe distance of the element already at that position. If the probe distance of the current element is greater, swap the two elements and continue processing the displaced element until an empty position is found.
4. Repeat the above process until an empty position is found.

**Lookup:**
1. Compute the hash value of the element to be looked up and determine its initial position.
2. Start from the initial position and check each bucket sequentially until the target element is found or an empty bucket is encountered (indicating the element does not exist).

**Deletion:**
1. Find the position of the element to be deleted and mark it as deleted.
2. To maintain the continuity of the hash table, it may be necessary to rearrange subsequent elements to ensure the correctness of the probe sequence.

#### Advantages
- **Balancing Probe Paths:** The Robin Hood strategy balances probe path lengths, reducing the occurrence of long probe sequences and thereby improving the performance and stability of the hash table.
- **Efficient Space Utilization:** By balancing probe path lengths, the hash table can maintain good performance even at higher load factors.
- **Cache Friendly:** Using contiguous memory storage enhances cache hit rates and improves access speed.

#### Disadvantages
- **Insertion Complexity:** The insertion operation may require multiple element swaps, leading to higher worst-case insertion complexity.
- **Implementation Complexity:** The need to manage probe paths and handle element rearrangement during insertion and deletion makes the algorithm relatively complex to implement.