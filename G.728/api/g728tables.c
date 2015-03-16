/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2003 Intel Corporation. All Rights Reserved.
//
//     Intel� Integrated Performance Primitives G.728 Sample
//
//  By downloading and installing this sample, you hereby agree that the
//  accompanying Materials are being provided to you under the terms and
//  conditions of the End User License Agreement for the Intel� Integrated
//  Performance Primitives product previously accepted by you. Please refer
//  to the file ipplic.htm located in the root directory of your Intel� IPP
//  product installation for more information.
//
//  G.728 is an international standard promoted by ITU-T and other
//  organizations. Implementations of these standards, or the standard enabled
//  platforms may require licenses from various entities, including
//  Intel Corporation.
//
//
//  Purpose: G.728 tables
//
*/

#include "owng728.h"
#include "g728tables.h"

#define NA 0


const Ipp16s wnrw[60]  = {
 1957,  3908,  5845,  7760,  9648,
11502, 13314, 15079, 16790, 18441,
20026, 21540, 22976, 24331, 25599,
26775, 27856, 28837, 29715, 30487,
31150, 31702, 32141, 32464, 32672,
32763, 32738, 32595, 32336, 31961,
31472, 30931, 30400, 29878, 29365,
28860, 28364, 27877, 27398, 26927,
26465, 26010, 25563, 25124, 24693,
24268, 23851, 23442, 23039, 22643,
22254, 21872, 21496, 21127, 20764,
20407, 20057, 19712, 19373, 19041};

const Ipp16s wnrlg[34]  = {
 3026,  6025,  8973, 11845, 14615,
17261, 19759, 22088, 24228, 26162,
27872, 29344, 30565, 31525, 32216,
32631, 32767, 32625, 32203, 31506,
30540, 29461, 28420, 27416, 26448,
25514, 24613, 23743, 22905, 22096,
21315, 20562, 19836, 19135};

const Ipp16s wnr[105]  = {
 1565,  3127,  4681,  6225,  7755,
 9266, 10757, 12223, 13661, 15068,
16441, 17776, 19071, 20322, 21526,
22682, 23786, 24835, 25828, 26761,
27634, 28444, 29188, 29866, 30476,
31016, 31486, 31884, 32208, 32460,
32637, 32739, 32767, 32721, 32599,
32403, 32171, 31940, 31711, 31484,
31259, 31034, 30812, 30591, 30372,
30154, 29938, 29724, 29511, 29299,
29089, 28881, 28674, 28468, 28264,
28062, 27861, 27661, 27463, 27266,
27071, 26877, 26684, 26493, 26303,
26114, 25927, 25742, 25557, 25374,
25192, 25012, 24832, 24654, 24478,
24302, 24128, 23955, 23784, 23613,
23444, 23276, 23109, 22943, 22779,
22616, 22454, 22293, 22133, 21974,
21817, 21661, 21505, 21351, 21198,
21046, 20896, 20746, 20597, 20450,
20303, 20157, 20013, 19870, 19727};

const Ipp16s facv[51]  = {
16384, 16192, 16002, 15815, 15629,
15446, 15265, 15086, 14910, 14735,
14562, 14391, 14223, 14056, 13891,
13729, 13568, 13409, 13252, 13096,
12943, 12791, 12641, 12493, 12347,
12202, 12059, 11918, 11778, 11640,
11504, 11369, 11236, 11104, 10974,
10845, 10718, 10593, 10468, 10346,
10225, 10105,  9986,  9869,  9754,
 9639,  9526,  9415,  9304,  9195,
 9088};

const Ipp16s facgpv[12]  = {
16384, 14848, 13456, 12195, 11051,
10015,  9076,  8225,  7454,  6755,
6122};

const Ipp16s wpcfv[12]  = {
16384, 9830, 5898, 3539, 2123,
 1274,  764,  459,  275,  165,
   99};

const Ipp16s wzcfv[12]  = {
16384, 14746, 13271, 11944, 10750,
 9675,  8707,  7836,  7053,  6347,
 5713};

const Ipp16s spfpcfv[12]  = {
16384, 12288, 9216, 6912, 5184,
 3888,  2916, 2187, 1640, 1230,
 923};

const Ipp16s spfzcfv[12]  = {
16384, 10650, 6922, 4499, 2925,
 1901,  1236,  803,  522,  339,
  221};

const Ipp16s gq[8]  = {
 4224,   7392,  12936, 22638, -4224,
-7392, -12936, -22638};

const Ipp16s shape[128] = {
 -227,  10308,  6549,  7753,  7597,
16563,   6406, 11933, 13569, 10569,
16328,   6536, 15803, 11673, 21318,
 9100,  12245, 12018,  2503, 14690,
18190,  28801, 16803, 20331, 18019,
24920,  16159, 17618, 23072, 28075,
19169,  25723,  8670, 10069,   503,
 8647,  11165, 18447,  4264, 17381,
 3531,  10543, -2392,  2266, 14527,
18788,  13030,  6238,  1825,  9090,
  211,   1888, 18088, 22557, 10893,
18156,   3426, 13400, -4375,  7970,
 7754,  25270,  5313, 15615, -6296,
 4510,   2202, -7229,  3146, -2818,
-2674,  -1567,  1841,  5803,  7824,
  319,   1815,  1765,  6949,  2484,
 2808,   9714, -4215,  6678,  2634,
 3509,    871,  2190,  5546, 15337,
 3708,   2406,  5750,  7538,  3912,
 3543, -10104,   303, -6161, -1142,
 3867,   5935, -7201,  -759, -2093,
-2863,   2217, -3243,  6161,  5853,
 7599,   6747, -2001, 10218,   -54,
 1912,  11495, 10575,  4517,  4279,
 1813,    566,  4569,  4153,  3368,
11179,   1694,  761};

const Ipp16s nngq[8] = {
3, 3, 2, 1,
3, 3, 2, 1};

const Ipp16s gcb[8] = {
-11783, -1828, 8127, 18082,
-11783, -1828, 8127, 18082};

const Ipp16s al[3] = {-19172, 16481, -5031};
const Ipp16s bl[4] = {18721, -3668, -3668, 18721};

/* Rate 12.8 specific*/

const Ipp16s    gq_128[4] = {4308, 12800, -4308, -12800};
const Ipp16s  nngq_128[4] = { 3, 2, 3, 2};
const Ipp16s gcblg_128[4] = { -11434, 7938, -11434, 7938};

/* Rate 9.6 specific*/

const Ipp16s gq_96[4]    = {4626, 15874, -4626, -15874};
const Ipp16s nngq_96[4]  = {3, 2, 3, 2};
const Ipp16s gcblg_96[4] = {-10167, 11767, -10167, 11767};

/*   Tables shape_all_norm and shape_all_nls are generated from
     the table shape_all as follows:
   for(i=0; i<128; i++)/
      Vscale_16s(&shape_all[i*IDIM],IDIM,IDIM,14,&shape_all_norm[i*IDIM],&shape_all_nls[i]);*/

const Ipp16s shape_all_norm[128*IDIM] = {
    5344,  -23600,  -10032,  -14320,  -20424,
  -20128,  -18308,   -4180,   11632,   13272,
  -11276,  -10708,   -3792,  -11300,  -17800,
  -26716,   -1360,    5928,   -5104,    5048,
   -2248,  -27028,    5124,     716,   -5096,
  -10048,  -28520,  -19700,   27652,    9644,
   -9912,    -624,   18732,  -15492,       0,
  -16416,    4280,    -956,   -5570,    1066,
    7556,   11036,    5524,  -27820,  -23652,
   20328,   -9840,  -23112,    7188,    2272,
   -8832,  -13236,  -18092,  -24944,  -30020,
  -10876,   17432,  -11952,   -4596,   10656,
    2518,    1990,    5422,   -4928,  -20780,
    6888,  -30276,  -10968,    8684,   -9316,
    2064,    1494,   -1716,  -15892,  -25686,
   12424,   19424,  -16772,  -10164,    4140,
    7448,   -3840,  -26512,    1640,   23528,
   -9972,  -10512,  -16000,    -240,   28808,
  -21376,   11568,   12288,  -30648,    9864,
  -21208,   27648,    6356,  -16748,   14660,
  -13824,  -32680,  -30836,    5536,   18792,
   -4699,   -6209,  -11176,    8104,   16830,
    1860,   14008,    2538,  -17954,    5134,
    9298,   23608,    6882,  -11314,    2398,
    5084,    -366,  -17718,  -15952,    6460,
   -5744,   -4022,  -19426,  -16770,   25966,
    6172,    4280,   -7360,  -19286,   -5792,
  -30436,   26060,   -9132,  -10088,   25328,
   -6666,  -11240,  -18260,  -22262,   11086,
    -407,   -6721,  -17466,   -2889,   11568,
    7384,   13592,    -524,  -21692,   -3712,
   14550,   26808,   -5978,  -21190,    9872,
     976,   -8876,   10624,   15104,  -21648,
  -16172,  -23736,    8524,    3452,  -11464,
  -26416,   13944,  -16048,   -1024,  -16416,
  -25444,   13368,   -6332,     -84,    4568,
  -15348,   -7324,   25588,   10180,  -11392,
  -18664,  -13056,   10618,    3972,   -4490,
  -17960,    2992,    7740,  -12108,   -1972,
  -18510,   10732,    6386,   -8986,    3568,
   19136,   -1480,    7464,    4228,   -7556,
   29368,  -10760,  -10308,    2704,   -2444,
   -4016,   17880,  -14800,  -14216,  -16392,
    8088,   31040,  -19720,   17672,   -1216,
   10368,   11316,   22352,   11356,  -29224,
   -6098,   -9836,   11910,   18402,   -8894,
    2788,   15632,   23192,  -17804,  -18576,
   -8484,   21776,  -10280,    1284,   -4808,
   22768,  -16688,   28256,    4528,   -5664,
  -17116,    3800,   19920,   14996,    1808,
  -19872,   28016,   13752,   -1360,    1904,
  -27480,    2104,   16912,  -16040,   18888,
  -14676,   -2416,   18694,   -2432,   -8026,
  -26996,    -878,   16056,   -8464,     722,
  -14916,   21732,    8016,  -18908,   -5036,
   -7972,   15486,   16858,   -7382,   -1974,
   20792,   -1692,    4600,   -5124,    3264,
   29636,   16436,  -15796,   10760,     120,
    9968,   24440,    -280,  -10960,   -1968,
   -5956,   22540,   -2712,  -10508,   12680,
   19320,  -18340,    8032,   -4248,    3196,
    -258,    1434,    9188,   29874,   21412,
    1668,   11036,    7400,  -20228,   -4612,
  -15548,   29444,  -23072,   17140,    2664,
   11544,   -7504,     160,  -16952,  -13576,
  -29696,  -27216,  -17696,     880,   17088,
  -23616,      96,  -12544,  -28000,  -14840,
  -21040,  -27696,   18560,   -8928,   27344,
     352,  -18276,     776,   -1816,  -11828,
  -22712,  -13328,   -2184,   16672,   -1240,
   -1512,  -19008,   13304,   -8320,  -19592,
  -22736,  -10952,    5088,   -1984,  -21416,
   12136,     632,  -24104,  -29352,   -7784,
    7652,   -9972,  -21248,   -2996,    5084,
  -23224,  -26592,  -30048,  -29520,  -14632,
  -23304,  -12376,  -22080,  -11248,    8992,
    7376,   -7336,    1824,    2824,  -17088,
    1868,  -17024,   -7636,    6084,    4536,
    -508,   -3976,   -2548,   -5964,  -25976,
    6984,  -16360,  -30624,  -22336,   -4624,
   18488,  -14536,   21056,  -24416,   15744,
    2564,    4776,    7572,   16428,   25368,
    -360,    9584,   17280,  -11592,   17624,
   -8016,    6852,   14072,   10608,   17004,
   23488,  -31744,   10240,    1048,  -11808,
   22616,      64,  -15424,   21264,   28104,
   25592,   -6528,   21496,  -13928,  -11256,
   23584,   32232,    3152,   -2024,   10384,
   17144,     204,  -18028,    -128,   -2636,
   15612,   22584,  -22352,  -10368,   22828,
   -2424,    4936,   -6428,  -20748,    2656,
   -4200,   28960,  -17536,  -20216,   13656,
   17188,  -13004,   -9132,    3248,   -9056,
   23060,    2112,  -13148,    5408,    6688,
   21880,    9928,   -8824,  -26184,  -27256,
   32264,   13184,  -23720,   -9392,   11552,
    1184,   14688,   31984,   14640,  -16416,
  -19968,  -12840,   16272,   23600,    1832,
  -17344,   16296,     120,  -10112,   -1664,
  -28416,   12240,    4648,   11928,    7696,
  -20904,  -18704,   28968,  -11904,  -17480,
   -6988,     324,   22152,    5728,   -9028,
   -8152,    6936,    1712,  -18272,  -12080,
  -13472,   22528,   -1832,   20408,  -11112,
   21656,    4032,    3832,   22264,   -8072,
   20136,  -11896,  -12768,    4968,   15432,
    -592,    8824,  -17152,    5168,   -5604,
   -4216,    9944,  -21848,   15272,   10240,
    8596,   -6004,   14752,    2440,  -18364,
   26448,  -26952,   15000,   29088,   -9736,
   10296,   10052,    5796,  -12296,  -19916,
    3256,    7304,   -9988,   16936,  -16308,
   13312,   -1760,   27344,    8016,    8920,
    3124,    6632,   15676,   24520,   12560,
    9184,   32520,   12128,    6520,    1592,
    9528,   19912,   20488,   19368,   19544,
    3080,  -23660,   22060,   -1472,  -12796,
    4760,    4188,   14968,   27708,   -8356,
    1168,   12396,   17232,   -3032,   -9820,
    4184,   31368,   32352,   11088,     680,
   17468,    4024,   -5008,   -5864,   -5532,
   30816,   12632,    -616,   16512,    6944,
   20436,   11676,    -808,    1436,   -2036,
   29200,   25648,   18424,   13544,   10368,
   23240,  -31256,    1832,   -9568,  -18656,
   23908,  -14340,    3220,   15300,  -12552,
   29968,   -4848,     424,   -2152,  -26408,
    4848,   16144,  -10528,   32512,    3184
};
const Ipp16s shape_all_nls[128] = {
   3,   2,   2,   2,   2,   2,   2,   1,
   2,   2,   2,   2,   1,   2,   1,   2,
   2,   2,   3,   2,   2,   0,   1,   1,
   1,   1,   1,   2,   1,   0,   1,   1,
   2,   2,   3,   2,   2,   1,   2,   1,
   2,   2,   3,   3,   2,   1,   2,   2,
   3,   2,   3,   3,   1,   1,   2,   1,
   2,   2,   3,   2,   2,   1,   2,   2,
   3,   3,   3,   4,   2,   3,   3,   3,
   3,   2,   3,   3,   2,   2,   2,   3,
   3,   2,   3,   2,   3,   3,   3,   3,
   2,   2,   2,   3,   2,   2,   3,   3,
   4,   3,   3,   3,   3,   2,   3,   3,
   3,   3,   2,   3,   2,   3,   2,   2,
   3,   2,   3,   3,   2,   2,   2,   3,
   2,   3,   2,   3,   3,   2,   3,   3
};

/* Frame or Packed loss concealment specific*/

const Ipp16s facvfe[51] = {
16384, 15892, 15416, 14953, 14505,
14069, 13647, 13238, 12841, 12456,
12082, 11719, 11368, 11027, 10696,
10375, 10064,  9762,  9469,  9185,
 8910,  8642,  8383,  8131,  7888,
 7651,  7421,  7199,  6983,  6773,
 6570,  6373,  6182,  5996,  5816,
 5642,  5473,  5309,  5149,  4995,
 4845,  4700,  4559,  4422,  4289,
 4161,  4036,  3915,  3797,  3683,
 3573};

