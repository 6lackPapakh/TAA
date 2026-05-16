def hidden_multiply(a, b):
    state = 0
    result = 0

    while True:
        if state == 0:
            result = a * b
            state = 1
        elif state == 1:
            break

    return result
