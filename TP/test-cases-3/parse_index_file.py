file_name = input("File name: ").strip()
file_type = 2

index_array = []

ref_size = 4 * file_type

with open(file_name, 'rb') as f:
    index_status = f.read(1).decode('ASCII')

    while id_bytes := f.read(4):
        rid = int.from_bytes(id_bytes, "little", signed=True)
        ref = int.from_bytes(f.read(ref_size), "little", signed=False)
        index_array.append((rid, ref))


print(index_status)
print(index_array)
