#include <iostream>
#include <cstdint>
#include <cmath>

#include "pch.h"

// define types
using arduino_int_t = int16_t;

// define global variables
namespace global {
    constexpr int arr_len{5};
}



long abs_sum(arduino_int_t *array_start, arduino_int_t *array_end) {
    /*args:
        array_start: zero index of an array
        array_end: last index of an array
      returns:
        the absolute value of the sum of the array values
      example:
        sum(
            &random_array[0],
            &random_array[sizeof(random_array)/sizeof(rpm_array[0])]
        );
        */
    long sum{0};
    for (arduino_int_t *ptr{array_start}; ptr < array_end; ptr++){
        sum += *ptr;
    }

    return abs(sum);
}

long sum_abs(arduino_int_t *array_start, arduino_int_t *array_end) {
    /*args:
        array_start: zero index of an array
        array_end: last index of an array
      returns:
        the sum of the absolute value of each value in the array
      example:
        sum(
            &random_array[0],
            &random_array[sizeof(random_array)/sizeof(rpm_array[0])]
        );
        */
    long sum{0};
    for (arduino_int_t *ptr{array_start}; ptr < array_end; ptr++){
        sum += abs(*ptr);
    }

    return sum;
}

bool is_uni_directional(arduino_int_t *array_start, arduino_int_t *array_end){
    // create pointer
    arduino_int_t *ptr{array_start};

    // initialize delta array
    arduino_int_t delta_array[global::arr_len-1]{};

    // gather the deltas
    arduino_int_t v1{}; arduino_int_t v2{};
    for (arduino_int_t i = 0; i < global::arr_len-1; i++){
        // collect two values
        v1 = *ptr; ptr++; v2 = *ptr;
        // calculate the delta
        delta_array[i] = v2 - v1;
    };

    // see if they are uni-directional
     long x1 = sum_abs(
        &delta_array[0],
        &delta_array[sizeof(delta_array)/sizeof(delta_array[0])]
    );

    long x2 = abs_sum(
        &delta_array[0],
        &delta_array[sizeof(delta_array)/sizeof(delta_array[0])]
    );

    if (x1 == x2) {
        return true;
    }
    return false;
};

float calc_std(arduino_int_t *array_start, arduino_int_t *array_end){
    /*takes the start and end of a fixed length vector and returns
    the standard deviation*/

    // calculate the sum
    long sum{0};
    for (arduino_int_t *ptr{array_start}; ptr<array_end; ptr++){
        sum += *ptr;
    }

    // calculate the average
    float mean{sum/global::arr_len};

    // calculate the deltas^2
    float sd{0};
    for (arduino_int_t *ptr{array_start}; ptr<array_end; ptr++){
        sd += pow((*ptr - mean), 2);
    }

    // return the standard deviation
    return sqrt((sd/global::arr_len));
};


int main()
{
    // non-uni-directional
    arduino_int_t rpm_array[global::arr_len]{0, 5, 3, 2, 8};
    // uni-directional
    arduino_int_t oned_array[global::arr_len]{8, 5, 4, 3, 0};

    // select array to use
    arduino_int_t array[global::arr_len]{};
    for (arduino_int_t i{0}; i < global::arr_len; i++){
        array[i] = rpm_array[i];
    }

    // check if the data set is uni-directional
    bool val = is_uni_directional(
        &array[0],
        &array[sizeof(array)/sizeof(array[0])]
        );

    // cout the standard deviation is not uni-directional
    float standard_deviation{0};
    if (!val) {
        standard_deviation = calc_std(
            &array[0],
            &array[sizeof(oned_array)/sizeof(array[0])]
        );
        std::cout << "standard deviation = " << standard_deviation << std::endl;
    }
    else {
        std::cout << "data set is uni-directional" << std::endl;
    }
}