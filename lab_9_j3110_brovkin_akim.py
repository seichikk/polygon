import numpy as np


def nth_index_of_max(arr, n):
    """
    Возвращает n-й индекс максимального элемента массива arr (NumPy-массив или список).
    Если такого индекса не существует (n > общего кол-ва максимумов), вернёт -1.
    """
    # Преобразуем вход в массив NumPy, если это список
    arr = np.asarray(arr)

    # Если массив пуст, сразу возвращаем -1 (нет максимумов)
    if arr.size == 0:
        return -1

    # 1. Находим максимальное значение
    max_val = np.max(arr)

    # 2. Находим все индексы, где значение = max_val
    # np.where вернёт кортеж, нам нужен первый элемент (индексы)
    max_indices = np.where(arr == max_val)[0]

    # 3. Если n превышает количество максимумов, возвращаем -1
    if n > max_indices.size:
        return -1

    # 4. Возвращаем (n-1)-й элемент (т.к. n — 1-based)
    return max_indices[n - 1]


if __name__ == "__main__":
    arr = np.array([3, 2, 3, 3, 1])
    n = 2
    result = nth_index_of_max(arr, n)
    print(result) 
