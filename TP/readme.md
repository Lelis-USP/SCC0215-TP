# TP - Arquivos

**Daniel H. Lelis de Almeia - _12543822_**

## Build & Run

to-do

## Improvoments

- [x] Better reuse code between T1Regisry and T2Registry
  - Maybe apply some kind of generic implementation over void\* and a registry type indicator.
- [x] Global error management
- [ ] Auto resizing buffers
- (Canceled) Detect column from header definitions
- [x] Skip remaining length of RT_VAR_LEN types to handle previously removed fields
- [x] Check for missing closes and MEM leaks
- [x] Extend documentation for new features
- [x] How will removed var_len fields work with their registry_size??
- [x] Check for duplicate IDs on insertion
- [x] Add insertion sort or merge sort for partial sorting
- (Not allowed) Add partial sort tracking for faster searchs on unordered array
- (Canceled) Check for duplicates after index build
- [x] Optimize index removals
- [x] Review documentation & organization
- [x] Re-do a general improvements round
- (Duplicate) Improve TP2 features docs
- [x] Remove extra index sort on removal 