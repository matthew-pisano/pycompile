print("Empty Set:", set())

print("Short Set:", {1, 2, 3})

print("Long Set:", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})

print("Set Union:", {1, 2} | {2, 3})
print("Set Intersection:", {1, 2} & {2, 3})
print("Set Difference:", {1, 2} - {2, 3})
print("Set Symmetric Difference:", {1, 2} ^ {2, 3})

print("Set Length:", len({1, 2, 3, 4, 4}))

a = {1, 2}
a.add(3)
print("Set Add:", a)

a = {1, 2}
a.update({3})
print("Set Update:", a)

print("Set Membership:", 0 in {1, 2})

print("Set of Strings:", set("Hello"))

print("Bool Set:", bool(set()), bool({1}))
