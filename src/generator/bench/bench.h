#ifndef BENCH_H
#define BENCH_H

void bench_co_ints_toby(int n);
void bench_co_ints_gor(int n);
#ifdef HAS_EXPERIMENTAL_GENERATOR
void bench_co_ints_exp(int n);
#endif
void bench_co_ints_atomic(int n);
void bench_handrolled_ints(int n);
void bench_ranges_ints(int n);

#endif // BENCH_H
