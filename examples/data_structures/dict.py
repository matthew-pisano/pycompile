print("Empty Dict:", dict())

print("Short Dict:", {1: 'one', 2: 'two', 3: 'three'})

print("Dict Merge:", {2: 'two', 3: 'three'} | {1: 'one'})

print("Dict Length:", len({1: 'one', 2: 'two', 3: 'three'}))

a = {1: 'one', 2: 'two', 3: 'three'}
print("Dict Get:", a.get(2))

a = {1: 'one', 2: 'two', 3: 'three'}
print("Dict Keys:", a.keys())

a = {1: 'one', 2: 'two', 3: 'three'}
print("Dict Values:", a.values())

a = {1: 'one', 2: 'two', 3: 'three'}
print("Dict Items:", a.items())

a = {2: 'two', 3: 'three'}
a.update({1: 'one'})
print("Dict Update:", a)

print("Dict Membership:", 2 in {1: 'one', 2: 'two', 3: 'three'})

print("Bool Dict:", bool(dict()), bool({1: 'one'}))
