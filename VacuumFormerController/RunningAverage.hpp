#pragma once

#include "Arduino.h"

class RunningAverage
{
  public:
  // protected:
    uint8_t _size;
    uint8_t _count;
    uint8_t _index;
    float    _sum;
  float*   _array = nullptr;

  public:
  RunningAverage(const uint8_t size)
  {
    _size = size;
    _array = new float[size];
    if (_array == nullptr) _size = 0;
    _count = 0;
    _index = 0;
    _sum = 0.0;
  }

  ~RunningAverage()
  {
    if (_array != nullptr) free(_array);
  }

  void addValue(const float value)
  {
    if (_array == nullptr)
    {
      return;
    }

    _sum -= _array[_index];
    _array[_index] = value;
    _sum += value;
    _index++;

    _index = _index % _size;  
    if (_count < _size - 1) _count++;
  }

  float getAverage()
  {
    _sum = 0;
    for (uint8_t i = 0; i < _count; i++)
    {
      _sum += _array[i];
    }
    return _sum / _count;  
  }

  
};