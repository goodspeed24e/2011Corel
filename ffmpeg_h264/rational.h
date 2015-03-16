
/**
 * @file rational.h
 * Rational numbers.
 */

#ifndef RATIONAL_H
#define RATIONAL_H

typedef struct AVRational{
    int num; 
    int den;
} AVRational;

static inline int av_cmp_q(AVRational a, AVRational b){
//static int av_cmp_q(AVRational a, AVRational b){
    const int64_t tmp= a.num * (int64_t)b.den - b.num * (int64_t)a.den;
    if     (tmp <  0) return -1;
    else if(tmp == 0) return  0;
    else              return  1;
}
//static inline double av_q2d(AVRational a){
static double av_q2d(AVRational a){
    return a.num / (double) a.den;
}

AVRational av_mul_q(AVRational b, AVRational c);
AVRational av_div_q(AVRational b, AVRational c);
AVRational av_add_q(AVRational b, AVRational c);
AVRational av_sub_q(AVRational b, AVRational c);
AVRational av_d2q(double d, int max);

#endif // RATIONAL_H
