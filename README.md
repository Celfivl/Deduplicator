CopyCat Deduplicator (v1.0.0)
CopyCat Deduplicator is a high-performance utility designed for total data integrity. Unlike standard deduplicators that rely solely on hashing, CopyCat uses a strict Three-Gate verification pipeline to ensure no two files are ever flagged as identical unless they are bit-for-bit clones.

The Three-Gate Pipeline
To achieve 100% reliability, CopyCat processes files through a hierarchical validation system:

1. Size Isolation Files are initially sorted into buckets by exact byte count. Any file with a unique size is instantly discarded from the search space, significantly reducing processing overhead.

2. Cryptographic Hashing SHA-256 digests are computed for files within the same size bucket to identify potential candidates.

3. Byte-for-Byte Verification The final gate performs a raw bit-level comparison using fgetc. This eliminates the risk of hash collisions and prefix matching errors.

Features
Ncurses TUI: A fast, interactive terminal interface for directory navigation and result management.

Recursive Scanning: Deep traversal of directory trees with robust error handling.

Efficient Memory Management: Custom Hash Table implementation using bucket-based collision resolution.

Explicit User Control: Interactive deletion screen where users must manually mark files before execution.

Installation Prerequisites

GCC (C99 or later)

Ncurses (Library for TUI)

OpenSSL 3.x (Required for SHA-256)

Building from Source To compile the executable, run the command: make

Usage
Launch the program and select your target directory using the interactive selector: ./copycat

Copyright 2026 G. Melancon JR.