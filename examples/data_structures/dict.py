print("Empty Dict:", dict())

print("Short Dict:", {1: '1', 2: '2', 3: '3'})

print("Dict Merge:", {1: '1', 2: '2'} | {3: '3'})

print("Dict Length:", len({1: '1', 2: '2', 3: '3'}))

a = {1: '1', 2: '2', 3: '3'}
print("Dict Get:", a.get(2))

a = {1: '1', 2: '2', 3: '3'}
print("Dict Get:", a.keys())

a = {1: '1', 2: '2', 3: '3'}
print("Dict Get:", a.values())

a = {1: '1', 2: '2', 3: '3'}
print("Dict Get:", a.items())

a = {1: '1', 2: '2'}
a.update({3: '3'})
print("Dict Update:", a)

print("Dict Membership:", 2 in {1: '1', 2: '2', 3: '3'})

print("Bool Dict:", bool(dict()), bool({1: '1'}))
