## Corrupted Cases

### Case 21

Case 21 expected result is wrong by the definition of the primary index (which should reject duplicate IDs), the test
case is actually inserting the elements with duplicate IDs both on the registry and the index, which shouldn't be 
allowed.
