a = "123"
print("Iterate over str:", a)
it = iter(a)
print(next(it))
print(next(it))
print(next(it))

a = [1, 2, 3]
print("Iterate over list:", a)
it = iter(a)
print(next(it))
print(next(it))
print(next(it))

a = {1, 2, 3}
print("Iterate over set:", a)
it = iter(a)
print(next(it))
print(next(it))
print(next(it))

a = {1: 'one', 2: 'two', 3: 'three'}
print("Iterate over dict:", a)
it = iter(a)
print(next(it))
print(next(it))
print(next(it))

a = (1, 2, 3)
print("Iterate over tuple:", a)
it = iter(a)
print(next(it))
print(next(it))
print(next(it))

print("Iterate Exhausted:")
it = iter([1])
next(it)
next(it)
