def fibonacci(n):
    """Returns the nth Fibonacci number."""
    if n <= 0:
        return 0
    elif n == 1:
        return 1
    else:
        return fibonacci(n - 1) + fibonacci(n - 2)


fib = 10
print(f"The {fib}th Fibonacci number is: {fibonacci(fib)}")
