#pragma once

#include <span>

double kahanSum(std::span<double> data) {
    double sum = 0.0;
    double c = 0.0;

    for (double x : data) {
        double y = x - c;
        volatile double t = sum + y;
        volatile double z = t - sum;
        c = z - y;

        sum = t;
    }
    return sum;
}