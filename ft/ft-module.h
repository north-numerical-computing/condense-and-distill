#pragma once 

#include <span>

extern "C" 
{
    extern void ft_acc_sum(int* n, double* data, double* sum);
    extern void ft_Demmel_Hida_Sum(int* n, double* data, double* sum);
    extern void ft_exact_sum(int* n, double* data, double* sum);
    extern void ft_fast_acc_sum(int* n, double* data, double* sum);
    extern void ft_prec_sum(int* n, double* data, double* sum);
}

double accSum(std::span<double> sum) {
    double out;
    int n = sum.size();
    ft_acc_sum(&n, sum.data(), &out);
    return out;
}

double demmelHidaSum(std::span<double> sum) {
    double out;
    int n = sum.size();
    ft_Demmel_Hida_Sum(&n, sum.data(), &out);
    return out;
}

double exactSum(std::span<double> sum) {
    double out;
    int n = sum.size();
    ft_exact_sum(&n, sum.data(), &out);
    return out;
}

double fastAccSum(std::span<double> sum) {
    double out;
    int n = sum.size();
    ft_fast_acc_sum(&n, sum.data(), &out);
    return out;
}

double precSum(std::span<double> sum) {
    double out;
    int n = sum.size();
    ft_prec_sum(&n, sum.data(), &out);
    return out;
}

