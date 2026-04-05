print("Empty List:", list())

print("Short List:", [1, 2, 3])

print("Long List:", [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1])

print("List Addition:", [1, 2] + [3])

print("List Length:", len([1, 2, 3, 4]))

a = [1, 2]
a.append(3)
print("List Append:", a)

a = [1, 2]
a.extend([3])
print("List Extend:", a)

print("List Index:", [1, 2][0])

print("List Membership:", 0 in [1, 2])

print("List of Strings:", list("Hello"))

print("Bool Lists:", bool([]), bool([1]))
