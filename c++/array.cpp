#include <stdexcept>
#include <memory>
#include "array.h"

using namespace std;

Array* arrayCreate() {
    return new Array;
}

void arrayDestroy(Array* array) {
    delete[] array->data;
    delete array;
}

int arraySize(const Array* array) {
    return array->size;
}

void arrayPush(Array* array, const string& value) {
    arrayInsert(array, array->size, value);
}

// При нехватке места увеличивает массив и вставляет значение по индексу.
void arrayInsert(Array* array, int index, const string& value) {
    if (index < 0 || index > array->size) throw runtime_error("index out of range");
    const string inserted = value;
    if (array->size == array->capacity) {
        int newCapacity = array->capacity == 0 ? 4 : array->capacity * 2;
        auto newData = make_unique<string[]>(newCapacity);
        for (int i = 0; i < array->size; i++) newData[i] = array->data[i];
        delete[] array->data;
        array->data = newData.release();
        array->capacity = newCapacity;
    }
    for (int i = array->size; i > index; i--) array->data[i] = array->data[i - 1];
    array->data[index] = inserted;
    array->size++;
}

string arrayGet(const Array* array, int index) {
    if (index < 0 || index >= array->size) throw runtime_error("index out of range");
    return array->data[index];
}

void arraySet(Array* array, int index, const string& value) {
    if (index < 0 || index >= array->size) throw runtime_error("index out of range");
    array->data[index] = value;
}

// Удаляет элемент и сдвигает оставшиеся значения влево.
string arrayDelete(Array* array, int index) {
    string value = arrayGet(array, index);
    for (int i = index; i + 1 < array->size; i++) array->data[i] = array->data[i + 1];
    array->data[--array->size].clear();
    return value;
}

vector<string> arrayRead(const Array* array) {
    vector<string> result;
    result.reserve(array->size);
    for (int i = 0; i < array->size; i++) result.push_back(array->data[i]);
    return result;
}
