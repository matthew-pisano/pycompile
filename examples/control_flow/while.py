print("Single While loop:")
i = 0
while i < 10:
    print("Index:", i)
    i += 1

print("Nested While loop:")
i = 0
j = 0
while i < 3:
    while j < 3:
        print("Index:", i, j)
        j += 1
    i += 1
