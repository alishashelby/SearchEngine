# Search Engine

## Overview
Implementation of full-text search with inverted index and BM25 ranking.

## Features

- **Inverted Index** with optimized storage
- **BM25 Ranking** using TF-IDF
- Query Support:
  - AND/OR with precedence
  - Parenthesis grouping
  - Automatic error handling
- **Case-insensitive search**
- Multi-threaded indexing
- Results with line numbers where the term was met
- Optimized Data Structures:
  - Trie for term lookup
  - Memory-mapped files for large indices

### Query syntax

The following queries are considered valid

 - "for"
 - "vector OR list"
 - "vector AND list"
 - "(for)"
 - "(vector OR list)"
 - "(vector AND list)"
 - "(while OR for) and vector"
 - "for AND and"

Invalid requests are considered
 - "for AND"
 - "vector list"
 - "for AND OR list"
- "vector Or list"

## Usage

### Indexing Files

```bash
./index /path/to/data
```

### Searching

```bash
./search k
```
where k is is the top-k results after ranking the found documents according to BM25.

## Testing

All specified requirements are verified through comprehensive test coverage using the [Google Test](https://github.com/google/googletest) framework.
