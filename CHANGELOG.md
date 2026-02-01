CHANGELOG: CopyCat Deduplicator
[1.0.0] - 2026-01-30
CopyCat Stable Release

Logic Restoration: Re-implemented the Three-Gate verification system (Size > Hash > Bit-check) to ensure 100% data integrity and resolve false-positive regressions.

UI Optimization: Standardized app workflow and optimized the Ncurses architecture for better window management.

Branding: Finalized official "CopyCat Deduplicator" naming and integrated the 2026 G. Melancon JR copyright footer across all TUI screens.

[0.9.0] - 2026-01-08
TUI & Logic Integration

Full System Test: Successfully executed a V1.0 functional test on a known dataset with confirmed duplicate deletion.

System Linkage: Connected the backend deduplicator.c engine to the Ncurses front-end actions.

Workflow Implementation: Added a dynamic scan progress view, results navigation menu, and a directory selection interface.

TUI Scaffolding: Initialized Ncurses windows and layout management.

[0.8.0] - 2026-01-01
Deduplication Engine Finalization

Verification Gate: Implemented byte-by-byte verification to handle hash collisions and ensure absolute bit-level accuracy.

Optimization: Introduced file size comparison filters to skip unnecessary hashing for unique file sizes.

Data Structures: Implemented a custom Hash Table/Index structure for fast duplicate lookup and optimized the dynamic list management for FileEntry structs.

[0.6.0] - 2025-12-28
Cryptographic Hashing Implementation

SHA-256 Integration: Integrated OpenSSL library calls for cryptographic hash generation.

Chunked Reading: Implemented a buffered file reading loop to process large files efficiently in 1MB chunks.

Validation: Verified implementation against standard SHA-256 test vectors.

[0.3.0] - 2025-12-26
Filesystem Subsystem

Recursive Traversal: Implemented deep-scan logic using opendir() and readdir() to traverse complex directory trees.

Metadata Retrieval: Integrated stat() calls to accurately retrieve file size, type, and last-modified attributes.

Architecture: Defined the core FileEntry types and initialized the repository structure.

[0.1.0] - 2025-12-25
Project Inception

Initial commit and project scaffolding.