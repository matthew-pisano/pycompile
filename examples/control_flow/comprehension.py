a = [i * 2 for i in range(3)]
print("List Comprehension:", a)

a = {i: str(i) for i in range(3)}
print("Dict Comprehension:", a)

a = {i * 2 for i in range(3)}
print("Set Comprehension:", a)
