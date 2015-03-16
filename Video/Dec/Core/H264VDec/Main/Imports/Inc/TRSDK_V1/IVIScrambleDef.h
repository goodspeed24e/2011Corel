//=============================================================================
//  THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
//  ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//  ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE
//  COMPANY.
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 2006  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _IVISCRAMBLE_GLOBALDEF_H
#define _IVISCRAMBLE_GLOBALDEF_H

#include ".\BaseTR\IVITRWinDef.h"

/////////////////////////////////////////////////////////////////////////
// random seeds
#define TR_CHECKSUM_SEED_0    0x1f78cfad
#define TR_CHECKSUM_SEED_1    0x3c869731
#define TR_CHECKSUM_SEED_2    0x18c9149d
#define TR_CHECKSUM_SEED_3    0x40f7e74b
#define TR_CHECKSUM_SEED_4    0x4b0be6db
#define TR_CHECKSUM_SEED_5    0xcab0d4cc
#define TR_CHECKSUM_SEED_6    0xf0a8b687
#define TR_CHECKSUM_SEED_7    0x0c56b872
#define TR_CHECKSUM_SEED_8    0xe4db8718
#define TR_CHECKSUM_SEED_9    0x5819fc54
#define TR_CHECKSUM_SEED_10    0xa7759560
#define TR_CHECKSUM_SEED_11    0x5c0955f9
#define TR_CHECKSUM_SEED_12    0x86ed2657
#define TR_CHECKSUM_SEED_13    0x3439ef73
#define TR_CHECKSUM_SEED_14    0x7bc95d2a
#define TR_CHECKSUM_SEED_15    0xaa17d208
#define TR_CHECKSUM_SEED_16    0xab8f5f1a
#define TR_CHECKSUM_SEED_17    0x985c8ce2
#define TR_CHECKSUM_SEED_18    0x3681939d
#define TR_CHECKSUM_SEED_19    0x7146ba43
#define TR_CHECKSUM_SEED_20    0x56935746
#define TR_CHECKSUM_SEED_21    0xd0c7cdc3
#define TR_CHECKSUM_SEED_22    0xd3215ffb
#define TR_CHECKSUM_SEED_23    0x06857e1a
#define TR_CHECKSUM_SEED_24    0xc41478ee
#define TR_CHECKSUM_SEED_25    0x5fac04ea
#define TR_CHECKSUM_SEED_26    0xf03c84df
#define TR_CHECKSUM_SEED_27    0x8aa1371a
#define TR_CHECKSUM_SEED_28    0xb5c6fd27
#define TR_CHECKSUM_SEED_29    0xa2dd0031
#define TR_CHECKSUM_SEED_30    0x48e6ac0c
#define TR_CHECKSUM_SEED_31    0x293df33e
#define TR_CHECKSUM_SEED_32    0xe7ca28d8
#define TR_CHECKSUM_SEED_33    0x314f4bc6
#define TR_CHECKSUM_SEED_34    0x2430643e
#define TR_CHECKSUM_SEED_35    0x3e1f413b
#define TR_CHECKSUM_SEED_36    0x2e08fb84
#define TR_CHECKSUM_SEED_37    0x595ff77a
#define TR_CHECKSUM_SEED_38    0x5fa370f7
#define TR_CHECKSUM_SEED_39    0xd9ab7ed3
#define TR_CHECKSUM_SEED_40    0x531bb022
#define TR_CHECKSUM_SEED_41    0xd4e2be14
#define TR_CHECKSUM_SEED_42    0x35cdec54
#define TR_CHECKSUM_SEED_43    0xf5c54924
#define TR_CHECKSUM_SEED_44    0xa5b78fe8
#define TR_CHECKSUM_SEED_45    0x9ffb0e9f
#define TR_CHECKSUM_SEED_46    0x0dd55ada
#define TR_CHECKSUM_SEED_47    0xbbe50efc
#define TR_CHECKSUM_SEED_48    0x65a3571e
#define TR_CHECKSUM_SEED_49    0xdbb977bb
#define TR_CHECKSUM_SEED_50    0x0d0d7146
#define TR_CHECKSUM_SEED_51    0xbd7a8520
#define TR_CHECKSUM_SEED_52    0x1934abea
#define TR_CHECKSUM_SEED_53    0xb58514ec
#define TR_CHECKSUM_SEED_54    0x679a0c5e
#define TR_CHECKSUM_SEED_55    0xab99dca7
#define TR_CHECKSUM_SEED_56    0xf4548727
#define TR_CHECKSUM_SEED_57    0x88493dec
#define TR_CHECKSUM_SEED_58    0xbd1a1ac1
#define TR_CHECKSUM_SEED_59    0x02ef9b41
#define TR_CHECKSUM_SEED_60    0x020e2f2b
#define TR_CHECKSUM_SEED_61    0xcc6cf7f7
#define TR_CHECKSUM_SEED_62    0x765698b6
#define TR_CHECKSUM_SEED_63    0x54fa758c
#define TR_CHECKSUM_SEED_64    0xa5a404a0
#define TR_CHECKSUM_SEED_65    0x3392c91e
#define TR_CHECKSUM_SEED_66    0x94fc7dfa
#define TR_CHECKSUM_SEED_67    0x2b8d8b60
#define TR_CHECKSUM_SEED_68    0xec2a614f
#define TR_CHECKSUM_SEED_69    0x411f4d98
#define TR_CHECKSUM_SEED_70    0x325e9d97
#define TR_CHECKSUM_SEED_71    0x139ec626
#define TR_CHECKSUM_SEED_72    0xf3afc8f3
#define TR_CHECKSUM_SEED_73    0xfc886f11
#define TR_CHECKSUM_SEED_74    0x163e77ad
#define TR_CHECKSUM_SEED_75    0xe857391f
#define TR_CHECKSUM_SEED_76    0xdce98c05
#define TR_CHECKSUM_SEED_77    0x166e3df5
#define TR_CHECKSUM_SEED_78    0xde9ddd44
#define TR_CHECKSUM_SEED_79    0xa36fa3bb
#define TR_CHECKSUM_SEED_80    0xa0675c92
#define TR_CHECKSUM_SEED_81    0x9976eccb
#define TR_CHECKSUM_SEED_82    0x8195e61e
#define TR_CHECKSUM_SEED_83    0x4bd315e7
#define TR_CHECKSUM_SEED_84    0xb0529505
#define TR_CHECKSUM_SEED_85    0x07fa0d79
#define TR_CHECKSUM_SEED_86    0x4e3d1885
#define TR_CHECKSUM_SEED_87    0xf38c965a
#define TR_CHECKSUM_SEED_88    0x93b2b0f9
#define TR_CHECKSUM_SEED_89    0xf36cb09d
#define TR_CHECKSUM_SEED_90    0x8d8b831b
#define TR_CHECKSUM_SEED_91    0xfdf58aea
#define TR_CHECKSUM_SEED_92    0xbc39780c
#define TR_CHECKSUM_SEED_93    0xee9986d8
#define TR_CHECKSUM_SEED_94    0x284e68ab
#define TR_CHECKSUM_SEED_95    0xf73602d8
#define TR_CHECKSUM_SEED_96    0xc39a5aab
#define TR_CHECKSUM_SEED_97    0x17a1a420
#define TR_CHECKSUM_SEED_98    0x53227df9
#define TR_CHECKSUM_SEED_99    0xae056323
#define TR_SCRAMBLE_SEED_0    0x616680e3
#define TR_SCRAMBLE_SEED_1    0x3ac2050e
#define TR_SCRAMBLE_SEED_2    0x8572d98f
#define TR_SCRAMBLE_SEED_3    0xe8b614d3
#define TR_SCRAMBLE_SEED_4    0xb672ab32
#define TR_SCRAMBLE_SEED_5    0x0ee31632
#define TR_SCRAMBLE_SEED_6    0x74845593
#define TR_SCRAMBLE_SEED_7    0x0688d551
#define TR_SCRAMBLE_SEED_8    0x22c2a556
#define TR_SCRAMBLE_SEED_9    0x78f03d5d
#define TR_SCRAMBLE_SEED_10    0x8f834390
#define TR_SCRAMBLE_SEED_11    0x0645a8af
#define TR_SCRAMBLE_SEED_12    0xbefeeb1f
#define TR_SCRAMBLE_SEED_13    0xa2f524f9
#define TR_SCRAMBLE_SEED_14    0x51a1124d
#define TR_SCRAMBLE_SEED_15    0x7b24adfe
#define TR_SCRAMBLE_SEED_16    0x1a755f3c
#define TR_SCRAMBLE_SEED_17    0xd40a6951
#define TR_SCRAMBLE_SEED_18    0x2b33e597
#define TR_SCRAMBLE_SEED_19    0xb0f4359b
#define TR_SCRAMBLE_SEED_20    0x3a9ab40e
#define TR_SCRAMBLE_SEED_21    0x99fea7e7
#define TR_SCRAMBLE_SEED_22    0x72de6016
#define TR_SCRAMBLE_SEED_23    0x488e597e
#define TR_SCRAMBLE_SEED_24    0x1d18c675
#define TR_SCRAMBLE_SEED_25    0x3dd0b0bf
#define TR_SCRAMBLE_SEED_26    0x5fc2df16
#define TR_SCRAMBLE_SEED_27    0xc592f78c
#define TR_SCRAMBLE_SEED_28    0x3163caa2
#define TR_SCRAMBLE_SEED_29    0xb3efd9b3
#define TR_SCRAMBLE_SEED_30    0xbdbc2861
#define TR_SCRAMBLE_SEED_31    0x4e6ccde9
#define TR_SCRAMBLE_SEED_32    0xf2d98ae7
#define TR_SCRAMBLE_SEED_33    0x2940f3c2
#define TR_SCRAMBLE_SEED_34    0x4c9dea05
#define TR_SCRAMBLE_SEED_35    0x0b9b9941
#define TR_SCRAMBLE_SEED_36    0x8c629886
#define TR_SCRAMBLE_SEED_37    0xe1ede25e
#define TR_SCRAMBLE_SEED_38    0x9f7eac2c
#define TR_SCRAMBLE_SEED_39    0x7f45f521
#define TR_SCRAMBLE_SEED_40    0x46a2ee81
#define TR_SCRAMBLE_SEED_41    0xaafe54be
#define TR_SCRAMBLE_SEED_42    0x7d0934e5
#define TR_SCRAMBLE_SEED_43    0xa6173e3e
#define TR_SCRAMBLE_SEED_44    0x99ade66b
#define TR_SCRAMBLE_SEED_45    0x63b5c12d
#define TR_SCRAMBLE_SEED_46    0x29d4ddfa
#define TR_SCRAMBLE_SEED_47    0xca6196ce
#define TR_SCRAMBLE_SEED_48    0xff447f38
#define TR_SCRAMBLE_SEED_49    0x9bbae25d
#define TR_SCRAMBLE_SEED_50    0x01c7fcbe
#define TR_SCRAMBLE_SEED_51    0x02882cd2
#define TR_SCRAMBLE_SEED_52    0x1298b60d
#define TR_SCRAMBLE_SEED_53    0x2b07d6b2
#define TR_SCRAMBLE_SEED_54    0x29805bda
#define TR_SCRAMBLE_SEED_55    0x43b40266
#define TR_SCRAMBLE_SEED_56    0xce9b910f
#define TR_SCRAMBLE_SEED_57    0x489f0196
#define TR_SCRAMBLE_SEED_58    0x30c50722
#define TR_SCRAMBLE_SEED_59    0xd9cf5813
#define TR_SCRAMBLE_SEED_60    0xcfd49c30
#define TR_SCRAMBLE_SEED_61    0x6d0e0ec5
#define TR_SCRAMBLE_SEED_62    0x2eff2960
#define TR_SCRAMBLE_SEED_63    0x15c79920
#define TR_SCRAMBLE_SEED_64    0x16bb9809
#define TR_SCRAMBLE_SEED_65    0x95b1f7a3
#define TR_SCRAMBLE_SEED_66    0xdabb9c29
#define TR_SCRAMBLE_SEED_67    0x571929de
#define TR_SCRAMBLE_SEED_68    0x8b9fba9d
#define TR_SCRAMBLE_SEED_69    0x85c6ca84
#define TR_SCRAMBLE_SEED_70    0x6132c9a7
#define TR_SCRAMBLE_SEED_71    0xc9a217fc
#define TR_SCRAMBLE_SEED_72    0x0d13c234
#define TR_SCRAMBLE_SEED_73    0x863f9405
#define TR_SCRAMBLE_SEED_74    0x7adbc071
#define TR_SCRAMBLE_SEED_75    0x00be4cd7
#define TR_SCRAMBLE_SEED_76    0xd5e55a25
#define TR_SCRAMBLE_SEED_77    0x81662a55
#define TR_SCRAMBLE_SEED_78    0x52049953
#define TR_SCRAMBLE_SEED_79    0xe8aa21c4
#define TR_SCRAMBLE_SEED_80    0x2f96d1a6
#define TR_SCRAMBLE_SEED_81    0x0f405a88
#define TR_SCRAMBLE_SEED_82    0x20732f23
#define TR_SCRAMBLE_SEED_83    0x932e766a
#define TR_SCRAMBLE_SEED_84    0x6a68a4a2
#define TR_SCRAMBLE_SEED_85    0x8bc297e8
#define TR_SCRAMBLE_SEED_86    0xe1110891
#define TR_SCRAMBLE_SEED_87    0x0893890a
#define TR_SCRAMBLE_SEED_88    0x7be47680
#define TR_SCRAMBLE_SEED_89    0x0bc3ad42
#define TR_SCRAMBLE_SEED_90    0x8a9873f3
#define TR_SCRAMBLE_SEED_91    0xa1dbb0d2
#define TR_SCRAMBLE_SEED_92    0x1cf13efb
#define TR_SCRAMBLE_SEED_93    0x51be5039
#define TR_SCRAMBLE_SEED_94    0x20521519
#define TR_SCRAMBLE_SEED_95    0x71485a2b
#define TR_SCRAMBLE_SEED_96    0x887488ea
#define TR_SCRAMBLE_SEED_97    0x1f595690
#define TR_SCRAMBLE_SEED_98    0x4dd3a519
#define TR_SCRAMBLE_SEED_99    0xc622a50a
#define TR_CHECKSUM_VALUE_0   0x23720318
#define TR_CHECKSUM_VALUE_1   0x64b16d84
#define TR_CHECKSUM_VALUE_2   0x9ac72425
#define TR_CHECKSUM_VALUE_3   0x80cfa545
#define TR_CHECKSUM_VALUE_4   0x83bc0310
#define TR_CHECKSUM_VALUE_5   0x06765313
#define TR_CHECKSUM_VALUE_6   0xdfc09d54
#define TR_CHECKSUM_VALUE_7   0xf42c04d8
#define TR_CHECKSUM_VALUE_8   0x7a2959ec
#define TR_CHECKSUM_VALUE_9   0xabc7b151
#define TR_CHECKSUM_VALUE_10   0xb1159c79
#define TR_CHECKSUM_VALUE_11   0x6d314842
#define TR_CHECKSUM_VALUE_12   0xa1525b29
#define TR_CHECKSUM_VALUE_13   0xf0dcfbb7
#define TR_CHECKSUM_VALUE_14   0x98bd15ed
#define TR_CHECKSUM_VALUE_15   0x41dbd4c9
#define TR_CHECKSUM_VALUE_16   0xc5d37571
#define TR_CHECKSUM_VALUE_17   0x3f73ae6f
#define TR_CHECKSUM_VALUE_18   0xbc39c061
#define TR_CHECKSUM_VALUE_19   0xf463b8d2
#define TR_CHECKSUM_VALUE_20   0x751e3d72
#define TR_CHECKSUM_VALUE_21   0x46068fad
#define TR_CHECKSUM_VALUE_22   0x1b8b00b3
#define TR_CHECKSUM_VALUE_23   0x7bb31631
#define TR_CHECKSUM_VALUE_24   0xdfca6aa9
#define TR_CHECKSUM_VALUE_25   0x3acdf2f6
#define TR_CHECKSUM_VALUE_26   0x062707b4
#define TR_CHECKSUM_VALUE_27   0xf22e2e72
#define TR_CHECKSUM_VALUE_28   0xc0782634
#define TR_CHECKSUM_VALUE_29   0xc787be75
#define TR_CHECKSUM_VALUE_30   0xcafc2201
#define TR_CHECKSUM_VALUE_31   0x4fbbf131
#define TR_CHECKSUM_VALUE_32   0xd8324ba1
#define TR_CHECKSUM_VALUE_33   0x2304b248
#define TR_CHECKSUM_VALUE_34   0xc87839ce
#define TR_CHECKSUM_VALUE_35   0xd4184c63
#define TR_CHECKSUM_VALUE_36   0xed5a3bbe
#define TR_CHECKSUM_VALUE_37   0x397f71d1
#define TR_CHECKSUM_VALUE_38   0x2d9b4ab7
#define TR_CHECKSUM_VALUE_39   0x9a71beac
#define TR_CHECKSUM_VALUE_40   0xd620c16a
#define TR_CHECKSUM_VALUE_41   0xfcbdaf09
#define TR_CHECKSUM_VALUE_42   0xc1102793
#define TR_CHECKSUM_VALUE_43   0xc74b0bae
#define TR_CHECKSUM_VALUE_44   0x2b80e866
#define TR_CHECKSUM_VALUE_45   0x54f7f7ce
#define TR_CHECKSUM_VALUE_46   0x5750b240
#define TR_CHECKSUM_VALUE_47   0x6da87254
#define TR_CHECKSUM_VALUE_48   0x7dc64621
#define TR_CHECKSUM_VALUE_49   0x4b85452e
#define TR_CHECKSUM_VALUE_50   0xf4c92572
#define TR_CHECKSUM_VALUE_51   0x1c7ec526
#define TR_CHECKSUM_VALUE_52   0xf798948c
#define TR_CHECKSUM_VALUE_53   0x1854ecbf
#define TR_CHECKSUM_VALUE_54   0xb90e908a
#define TR_CHECKSUM_VALUE_55   0x9574449a
#define TR_CHECKSUM_VALUE_56   0x2c937ee6
#define TR_CHECKSUM_VALUE_57   0xab2e13e9
#define TR_CHECKSUM_VALUE_58   0x7015055c
#define TR_CHECKSUM_VALUE_59   0x19dd7b61
#define TR_CHECKSUM_VALUE_60   0xd8606092
#define TR_CHECKSUM_VALUE_61   0xedb4a53c
#define TR_CHECKSUM_VALUE_62   0x8671f80b
#define TR_CHECKSUM_VALUE_63   0xcb0dfcbb
#define TR_CHECKSUM_VALUE_64   0x975eb5e2
#define TR_CHECKSUM_VALUE_65   0x6f3b07ba
#define TR_CHECKSUM_VALUE_66   0xd9a5f0ce
#define TR_CHECKSUM_VALUE_67   0x9ae964c2
#define TR_CHECKSUM_VALUE_68   0xd1d0fee9
#define TR_CHECKSUM_VALUE_69   0xfb59982b
#define TR_CHECKSUM_VALUE_70   0x356ecfc7	//DDPlusDecGMO used.
#define TR_CHECKSUM_VALUE_71   0x15c03abf	//DTSHDDecGMO used.
#define TR_CHECKSUM_VALUE_72   0x2691d44b
#define TR_CHECKSUM_VALUE_73   0x704c846b
#define TR_CHECKSUM_VALUE_74   0x071ec2cf
#define TR_CHECKSUM_VALUE_75   0xe2701575
#define TR_CHECKSUM_VALUE_76   0xee4fce0a
#define TR_CHECKSUM_VALUE_77   0xf21ae8ba
#define TR_CHECKSUM_VALUE_78   0x4ffd8d47
#define TR_CHECKSUM_VALUE_79   0x05d06174
#define TR_CHECKSUM_VALUE_80   0x20525c5b
#define TR_CHECKSUM_VALUE_81   0x7785cf0c
#define TR_CHECKSUM_VALUE_82   0x37b381ea
#define TR_CHECKSUM_VALUE_83   0xb667096a
#define TR_CHECKSUM_VALUE_84   0x9dca3272
#define TR_CHECKSUM_VALUE_85   0xe9c0f05b
#define TR_CHECKSUM_VALUE_86   0x13b98e9c
#define TR_CHECKSUM_VALUE_87   0x5161c071
#define TR_CHECKSUM_VALUE_88   0x8ca3e355
#define TR_CHECKSUM_VALUE_89   0x13f1f7f4
#define TR_CHECKSUM_VALUE_90   0x58d35a36
#define TR_CHECKSUM_VALUE_91   0xc5da8e5c
#define TR_CHECKSUM_VALUE_92   0x1ae610a8
#define TR_CHECKSUM_VALUE_93   0xe77568c9
#define TR_CHECKSUM_VALUE_94   0x9f8f145c
#define TR_CHECKSUM_VALUE_95   0x83607e0f
#define TR_CHECKSUM_VALUE_96   0x93946b82
#define TR_CHECKSUM_VALUE_97   0x25ef64c2
#define TR_CHECKSUM_VALUE_98   0x14d0374c
#define TR_CHECKSUM_VALUE_99   0x1ccfedca
#define TR_CHECKSUM_ENABLE_0  0x26228739
#define TR_CHECKSUM_ENABLE_1  0x82249608
#define TR_CHECKSUM_ENABLE_2  0x1f603ba9
#define TR_CHECKSUM_ENABLE_3  0x28684177
#define TR_CHECKSUM_ENABLE_4  0x3162de35
#define TR_CHECKSUM_ENABLE_5  0x94969a5c
#define TR_CHECKSUM_ENABLE_6  0x73879e52
#define TR_CHECKSUM_ENABLE_7  0x723316ff
#define TR_CHECKSUM_ENABLE_8  0xdb0748bd
#define TR_CHECKSUM_ENABLE_9  0xc4ebf063
#define TR_CHECKSUM_ENABLE_10  0x45aed82c
#define TR_CHECKSUM_ENABLE_11  0x25acbc95
#define TR_CHECKSUM_ENABLE_12  0xc0a5c5c9
#define TR_CHECKSUM_ENABLE_13  0x812006f5
#define TR_CHECKSUM_ENABLE_14  0x890887f6
#define TR_CHECKSUM_ENABLE_15  0xa3b9890d
#define TR_CHECKSUM_ENABLE_16  0xdb4615d3
#define TR_CHECKSUM_ENABLE_17  0xab7c4e70
#define TR_CHECKSUM_ENABLE_18  0x3f43ce16
#define TR_CHECKSUM_ENABLE_19  0xb9e1d621
#define TR_CHECKSUM_ENABLE_20  0x027727a2
#define TR_CHECKSUM_ENABLE_21  0xec528c03
#define TR_CHECKSUM_ENABLE_22  0x3e409a6b
#define TR_CHECKSUM_ENABLE_23  0x57d03ccd
#define TR_CHECKSUM_ENABLE_24  0x5fe93721
#define TR_CHECKSUM_ENABLE_25  0x9487cf08
#define TR_CHECKSUM_ENABLE_26  0x94d74c1f
#define TR_CHECKSUM_ENABLE_27  0x88249397
#define TR_CHECKSUM_ENABLE_28  0x28995e29
#define TR_CHECKSUM_ENABLE_29  0x5de3a846
#define TR_CHECKSUM_ENABLE_30  0xdf9d636f
#define TR_CHECKSUM_ENABLE_31  0x957eaee1
#define TR_CHECKSUM_ENABLE_32  0x74769755
#define TR_CHECKSUM_ENABLE_33  0x5224e34f
#define TR_CHECKSUM_ENABLE_34  0xeeb8c586
#define TR_CHECKSUM_ENABLE_35  0x0ce0a33b
#define TR_CHECKSUM_ENABLE_36  0x2d9e32d0
#define TR_CHECKSUM_ENABLE_37  0x01e321c5
#define TR_CHECKSUM_ENABLE_38  0x71b7f245
#define TR_CHECKSUM_ENABLE_39  0x7e4e29b2
#define TR_CHECKSUM_ENABLE_40  0x4c167d26
#define TR_CHECKSUM_ENABLE_41  0x2836d8b1
#define TR_CHECKSUM_ENABLE_42  0x0675cc16
#define TR_CHECKSUM_ENABLE_43  0xccbb261e
#define TR_CHECKSUM_ENABLE_44  0x3f13f814
#define TR_CHECKSUM_ENABLE_45  0xc825afd1
#define TR_CHECKSUM_ENABLE_46  0x87fed1bc
#define TR_CHECKSUM_ENABLE_47  0x66394f74
#define TR_CHECKSUM_ENABLE_48  0x9cc3e555
#define TR_CHECKSUM_ENABLE_49  0x24dac2e4
#define TR_CHECKSUM_ENABLE_50  0x7e856a20
#define TR_CHECKSUM_ENABLE_51  0x3978da14
#define TR_CHECKSUM_ENABLE_52  0x59a70376
#define TR_CHECKSUM_ENABLE_53  0x41a76500
#define TR_CHECKSUM_ENABLE_54  0x3159a227
#define TR_CHECKSUM_ENABLE_55  0x758a041e
#define TR_CHECKSUM_ENABLE_56  0xc06abca5
#define TR_CHECKSUM_ENABLE_57  0x1ac8a5e0
#define TR_CHECKSUM_ENABLE_58  0x33eb3c7b
#define TR_CHECKSUM_ENABLE_59  0xa8e703bb
#define TR_CHECKSUM_ENABLE_60  0x8f4f4781
#define TR_CHECKSUM_ENABLE_61  0x7bf81932
#define TR_CHECKSUM_ENABLE_62  0x82d35362
#define TR_CHECKSUM_ENABLE_63  0xfcc8f772
#define TR_CHECKSUM_ENABLE_64  0x1c1559e7
#define TR_CHECKSUM_ENABLE_65  0xc9c01fea
#define TR_CHECKSUM_ENABLE_66  0xcde36c88
#define TR_CHECKSUM_ENABLE_67  0x45c1d877
#define TR_CHECKSUM_ENABLE_68  0x8772edc7
#define TR_CHECKSUM_ENABLE_69  0xe69f738f
#define TR_CHECKSUM_ENABLE_70  0xe0419ed2	//DDPlusDecGMO used.
#define TR_CHECKSUM_ENABLE_71  0x1ea16efa	//DTSHDDecGMO used.
#define TR_CHECKSUM_ENABLE_72  0x551237f0
#define TR_CHECKSUM_ENABLE_73  0x9ced938e
#define TR_CHECKSUM_ENABLE_74  0x53886a2f
#define TR_CHECKSUM_ENABLE_75  0x6e9ea877
#define TR_CHECKSUM_ENABLE_76  0x4513f0e3
#define TR_CHECKSUM_ENABLE_77  0xce8a5487
#define TR_CHECKSUM_ENABLE_78  0x26ef6165
#define TR_CHECKSUM_ENABLE_79  0x17cc9008
#define TR_CHECKSUM_ENABLE_80  0xb41f6fa0
#define TR_CHECKSUM_ENABLE_81  0xaf55d19f
#define TR_CHECKSUM_ENABLE_82  0x777d65e2
#define TR_CHECKSUM_ENABLE_83  0x32d388b1
#define TR_CHECKSUM_ENABLE_84  0xaaf49678
#define TR_CHECKSUM_ENABLE_85  0x436dffd1
#define TR_CHECKSUM_ENABLE_86  0x0a40158e
#define TR_CHECKSUM_ENABLE_87  0x0cf5a9b5
#define TR_CHECKSUM_ENABLE_88  0xce8cb1dd
#define TR_CHECKSUM_ENABLE_89  0x6638633a
#define TR_CHECKSUM_ENABLE_90  0x7d912a99
#define TR_CHECKSUM_ENABLE_91  0xa55b7ce2
#define TR_CHECKSUM_ENABLE_92  0x15be6930
#define TR_CHECKSUM_ENABLE_93  0xe330e66c
#define TR_CHECKSUM_ENABLE_94  0x0a5b0d4a
#define TR_CHECKSUM_ENABLE_95  0x7aa4dee2
#define TR_CHECKSUM_ENABLE_96  0x1d696194
#define TR_CHECKSUM_ENABLE_97  0x4d7db3bf
#define TR_CHECKSUM_ENABLE_98  0x0e6cb7d3
#define TR_CHECKSUM_ENABLE_99  0xa5c5018e
#define TR_SCRAMBLE_ENABLE_0  0x482e12bd
#define TR_SCRAMBLE_ENABLE_1  0x1354909f
#define TR_SCRAMBLE_ENABLE_2  0xacd2ac1d
#define TR_SCRAMBLE_ENABLE_3  0x0726503a
#define TR_SCRAMBLE_ENABLE_4  0xd6a8cdbe
#define TR_SCRAMBLE_ENABLE_5  0xfa1afadd
#define TR_SCRAMBLE_ENABLE_6  0x69371b9b
#define TR_SCRAMBLE_ENABLE_7  0x4f14b606
#define TR_SCRAMBLE_ENABLE_8  0xf138b5dc
#define TR_SCRAMBLE_ENABLE_9  0x145ce7fd
#define TR_SCRAMBLE_ENABLE_10  0xbb4d6116
#define TR_SCRAMBLE_ENABLE_11  0x7272e62e
#define TR_SCRAMBLE_ENABLE_12  0xd84bcb59
#define TR_SCRAMBLE_ENABLE_13  0x5a7cc687
#define TR_SCRAMBLE_ENABLE_14  0xf73f4c80
#define TR_SCRAMBLE_ENABLE_15  0xe8b1865c
#define TR_SCRAMBLE_ENABLE_16  0x2434e84c
#define TR_SCRAMBLE_ENABLE_17  0xcb37e15a
#define TR_SCRAMBLE_ENABLE_18  0xd42b20c7
#define TR_SCRAMBLE_ENABLE_19  0x48169af8
#define TR_SCRAMBLE_ENABLE_20  0x25884f6c
#define TR_SCRAMBLE_ENABLE_21  0x38ec24f9
#define TR_SCRAMBLE_ENABLE_22  0xc5a2239d
#define TR_SCRAMBLE_ENABLE_23  0x11483572
#define TR_SCRAMBLE_ENABLE_24  0x27024cdc
#define TR_SCRAMBLE_ENABLE_25  0x74a5e6d8
#define TR_SCRAMBLE_ENABLE_26  0x3d315f4e
#define TR_SCRAMBLE_ENABLE_27  0x731b6499
#define TR_SCRAMBLE_ENABLE_28  0xe9f16904
#define TR_SCRAMBLE_ENABLE_29  0x1ff8eeb5
#define TR_SCRAMBLE_ENABLE_30  0x44f3b690
#define TR_SCRAMBLE_ENABLE_31  0xcbb566ea
#define TR_SCRAMBLE_ENABLE_32  0x822fed5c
#define TR_SCRAMBLE_ENABLE_33  0x829f81de
#define TR_SCRAMBLE_ENABLE_34  0x782d0a4f
#define TR_SCRAMBLE_ENABLE_35  0x23ec1edb
#define TR_SCRAMBLE_ENABLE_36  0xae9e9b39
#define TR_SCRAMBLE_ENABLE_37  0x8ce0f087
#define TR_SCRAMBLE_ENABLE_38  0xb2e7f181
#define TR_SCRAMBLE_ENABLE_39  0xbdf02931
#define TR_SCRAMBLE_ENABLE_40  0xc328f4a5
#define TR_SCRAMBLE_ENABLE_41  0xe7c1451a
#define TR_SCRAMBLE_ENABLE_42  0x1da182f6
#define TR_SCRAMBLE_ENABLE_43  0x5c42954e
#define TR_SCRAMBLE_ENABLE_44  0xb9555728
#define TR_SCRAMBLE_ENABLE_45  0x6ff8f583
#define TR_SCRAMBLE_ENABLE_46  0x9df9b123
#define TR_SCRAMBLE_ENABLE_47  0xd9edff7f
#define TR_SCRAMBLE_ENABLE_48  0xa857a89c
#define TR_SCRAMBLE_ENABLE_49  0xa4b1f685
#define TR_SCRAMBLE_ENABLE_50  0xf16c17f9
#define TR_SCRAMBLE_ENABLE_51  0x750bad4e
#define TR_SCRAMBLE_ENABLE_52  0xf9afb5fa
#define TR_SCRAMBLE_ENABLE_53  0xf3125105
#define TR_SCRAMBLE_ENABLE_54  0x3f121cae
#define TR_SCRAMBLE_ENABLE_55  0x848cebd1
#define TR_SCRAMBLE_ENABLE_56  0x2b4f642c
#define TR_SCRAMBLE_ENABLE_57  0x2389b95f
#define TR_SCRAMBLE_ENABLE_58  0x656cdf94
#define TR_SCRAMBLE_ENABLE_59  0x0e508ffc
#define TR_SCRAMBLE_ENABLE_60  0x304612dd
#define TR_SCRAMBLE_ENABLE_61  0xf5e708b2
#define TR_SCRAMBLE_ENABLE_62  0xdd339fe4
#define TR_SCRAMBLE_ENABLE_63  0x5a7bd8ee
#define TR_SCRAMBLE_ENABLE_64  0xe4f65148
#define TR_SCRAMBLE_ENABLE_65  0xb53af831
#define TR_SCRAMBLE_ENABLE_66  0x1630118e
#define TR_SCRAMBLE_ENABLE_67  0x191f9443
#define TR_SCRAMBLE_ENABLE_68  0x17d58508
#define TR_SCRAMBLE_ENABLE_69  0xc87dfe74
#define TR_SCRAMBLE_ENABLE_70  0x8f1f450c
#define TR_SCRAMBLE_ENABLE_71  0x7f22a463
#define TR_SCRAMBLE_ENABLE_72  0xdfac82ea
#define TR_SCRAMBLE_ENABLE_73  0x78fd0ec9
#define TR_SCRAMBLE_ENABLE_74  0x29972131
#define TR_SCRAMBLE_ENABLE_75  0x1b7388d3
#define TR_SCRAMBLE_ENABLE_76  0x9468d9b9
#define TR_SCRAMBLE_ENABLE_77  0xbb94da7f
#define TR_SCRAMBLE_ENABLE_78  0x5afcaef6
#define TR_SCRAMBLE_ENABLE_79  0xfd876883
#define TR_SCRAMBLE_ENABLE_80  0xf3702f12
#define TR_SCRAMBLE_ENABLE_81  0x01ad943c
#define TR_SCRAMBLE_ENABLE_82  0xbf793750
#define TR_SCRAMBLE_ENABLE_83  0xf2189e24
#define TR_SCRAMBLE_ENABLE_84  0x147e0e35
#define TR_SCRAMBLE_ENABLE_85  0xe304f54d
#define TR_SCRAMBLE_ENABLE_86  0x880072fb
#define TR_SCRAMBLE_ENABLE_87  0x9737866e
#define TR_SCRAMBLE_ENABLE_88  0x880928ce
#define TR_SCRAMBLE_ENABLE_89  0xb93c44ee
#define TR_SCRAMBLE_ENABLE_90  0x4453ec4b
#define TR_SCRAMBLE_ENABLE_91  0xcc98107a
#define TR_SCRAMBLE_ENABLE_92  0xd43049cd
#define TR_SCRAMBLE_ENABLE_93  0xea2f41a4
#define TR_SCRAMBLE_ENABLE_94  0xe21912fd
#define TR_SCRAMBLE_ENABLE_95  0x0932700a
#define TR_SCRAMBLE_ENABLE_96  0x6530fa30
#define TR_SCRAMBLE_ENABLE_97  0x8614ac85
#define TR_SCRAMBLE_ENABLE_98  0x1def8205
#define TR_SCRAMBLE_ENABLE_99  0x1320d1de

#define TR_KEYA_INITIAL_XOR_SEED	0x01010101		// a simple initial scramble seed
#define TR_KEYA_ACTUAL_XOR_SEED		0x81782311		// actual one used, gets replaced run-time
#define TR_KEYB_INITIAL_XOR_SEED	0x01010101		// a simple initial scramble seed
#define TR_KEYB_ACTUAL_XOR_SEED		0xbf981981		// actual one used, gets replaced run-time

#define TR_CHECKSUM_RELOCATION	0xf9183183

#define MAX_TABLE_COUNT 4

#define iviTR_TREXE_EIP_FAR_BIT (1 << 15)

NS_TRLIB_BEGIN

void __cdecl trCall_Scramble_Entry();
void __cdecl trCall_Checksum_Entry();
void __cdecl trCall_Descramble_Entry();

DWORD __fastcall TR_RelocateCode();
extern volatile DWORD TR_RelocateBase;
extern volatile DWORD TR_Relocate_Base2;

struct trScrambleItem
{
	BYTE m_byValue;
	BYTE m_byRValue;
	int m_nFlag;
};

// extern trScrambleItem* __cdecl gfnGetIVIScrambleItem(int i, int j);
extern struct trScrambleItem *g_pScrambleTable;
extern struct trScrambleItem *g_pScrambleTable1;
extern struct trScrambleItem *g_pScramTable;

#ifdef __cplusplus

	class CtrLockerManager
	{
	public:
		static CtrLockerManager* __cdecl GetInstance();
		static void __cdecl FreeInstance();
#if (_MSC_VER >= 1400)
		void __thiscall Lock(DWORD dwRegion);
		void __thiscall Unlock(DWORD dwRegion);
#else
		void Lock(DWORD dwRegion);
		void Unlock(DWORD dwRegion);
#endif
		~CtrLockerManager();
	protected:
		static CtrLockerManager *m_pInstance;
	protected:
		CtrLockerManager();
		void *m_pvecpCSsPtr;
		CRITICAL_SECTION m_SelfCS;
	};
#endif

NS_TRLIB_END

#define g_ScrambleTable(i, j) \
	(g_pScramTable + i * 256 + j) 

#define iviTR_LIGHTWEIGHT_DESCRAMBLE(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable, scrambleEIPTableSize)	\
{																								\
	BYTE *bptr;																					\
	DWORD dwNewseed = seed;																		\
	WORD *pwEIPTable = pwScrambleEIPTable;														\
	if (pwScrambleEIPTable && scrambleEIPTableSize)												\
	{																							\
		if(pwTable && *pwTable!=0xffff && DO_ISRELOCATED)										\
		{																						\
			WORD *pwOrigTable;																	\
																								\
			DWORD dwScrambleSize = 0;															\
			DWORD dwStart = (DWORD)start_addr;													\
			dwStart += DO_GETUNDOOFFSET;														\
			switch ((dwStart >> 8) % 2)															\
			{																					\
			case 0:																				\
				g_pScramTable = g_pScrambleTable;												\
				break;																			\
			case 1:																				\
				g_pScramTable = g_pScrambleTable1;												\
				break;																			\
			}																					\
			pwOrigTable = pwTable;																\
			DO_RELOCATECODE(start_addr, pwOrigTable, TRUE);										\
			DWORD dwOffsetBase = (DWORD)start_addr;												\
			bptr = (BYTE*)dwOffsetBase;															\
			while (*pwEIPTable != 0xFFFF && (DWORD)bptr < (DWORD)end_addr)						\
			{																					\
				bool bFar = (*pwEIPTable) & iviTR_TREXE_EIP_FAR_BIT;							\
				bptr += (*pwEIPTable | iviTR_TREXE_EIP_FAR_BIT) ^ iviTR_TREXE_EIP_FAR_BIT;		\
				pwEIPTable++;																	\
				dwOffsetBase = (DWORD)bptr;														\
				int nFlag = g_ScrambleTable(0, *bptr)->m_nFlag;									\
																								\
				if ((*bptr == 0x8b && *(bptr + 1) == 0x25 && bFar)	||							\
					(*bptr == 0x00 && *(bptr + 1) == 0x90 && bFar)	||							\
					(*bptr == 0x8b && *(bptr + 1) == 0x90 && bFar)	||							\
					(*bptr == 0x8b && *(bptr + 1) == 0x0d && bFar)	||							\
					(*bptr == 0x68)	||															\
					(*bptr == 0x6A))															\
				{																				\
					if ((*bptr == 0x8b && *(bptr + 1) == 0x25)	||								\
						(*bptr == 0x8b && *(bptr + 1) == 0x90))									\
					{																			\
						*bptr = 0xff;															\
						*(bptr + 1) = 0x25;														\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if ((*bptr == 0x00 && *(bptr + 1) == 0x90)	||							\
							(*bptr == 0x8b && *(bptr + 1) == 0x0d))								\
					{																			\
						*bptr = 0xff;															\
						*(bptr + 1) = 0x15;														\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0x68 || *bptr == 0x6A)									\
						*(++bptr) ^= dwNewseed;													\
																								\
					DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);					\
					dwOffsetseed = dwOffsetseed << 24;											\
					dwNewseed = dwNewseed >> 8;													\
					dwNewseed = dwOffsetseed | dwNewseed;										\
				}																				\
				else if (nFlag >= 0)															\
				{																				\
					if (nFlag == 1 || nFlag == 3)												\
					{																			\
						bptr++;																	\
						if (g_ScrambleTable(nFlag, *bptr)->m_nFlag == 0)						\
							*bptr = g_ScrambleTable(nFlag, *bptr)->m_byRValue;					\
					}																			\
					else																		\
					{																			\
						*bptr = g_ScrambleTable(nFlag, *bptr)->m_byRValue;						\
						if ((*bptr == 0x9A || *bptr == 0xEA) ||									\
							(*bptr == 0xE8 || *bptr == 0xE9))									\
						{																		\
							*(++bptr) ^= dwNewseed;												\
							DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);			\
							dwOffsetseed = dwOffsetseed << 24;									\
							dwNewseed = dwNewseed >> 8;											\
							dwNewseed = dwOffsetseed | dwNewseed;								\
						}																		\
					}																			\
				}																				\
				dwScrambleSize++;																\
				if (dwScrambleSize >= scrambleEIPTableSize)										\
					break;																		\
				bptr = (BYTE*)dwOffsetBase;														\
			}																					\
			DO_RELOCATECODE(start_addr, pwOrigTable, FALSE);									\
		}																						\
		else																					\
		{																						\
			DWORD dwScrambleSize = 0;															\
			DWORD dwStart = (DWORD)start_addr;													\
			switch ((dwStart >> 8) % 2)															\
			{																					\
			case 0:																				\
				g_pScramTable = g_pScrambleTable;												\
				break;																			\
			case 1:																				\
				g_pScramTable = g_pScrambleTable1;												\
				break;																			\
			}																					\
			DWORD dwOffsetBase = (DWORD)start_addr;												\
			bptr = (BYTE*)dwOffsetBase;															\
			while (*pwEIPTable != 0xFFFF && (DWORD)bptr < (DWORD)end_addr)						\
			{																					\
				bool bFar = (*pwEIPTable) & iviTR_TREXE_EIP_FAR_BIT;							\
				bptr += (*pwEIPTable | iviTR_TREXE_EIP_FAR_BIT) ^ iviTR_TREXE_EIP_FAR_BIT;		\
				pwEIPTable++;																	\
				dwOffsetBase = (DWORD)bptr;														\
				int nFlag = g_ScrambleTable(0, *bptr)->m_nFlag;									\
																								\
				if ((*bptr == 0x8b && *(bptr + 1) == 0x25 && bFar)	||							\
					(*bptr == 0x00 && *(bptr + 1) == 0x90 && bFar)	||							\
					(*bptr == 0x8b && *(bptr + 1) == 0x90 && bFar)	||							\
					(*bptr == 0x8b && *(bptr + 1) == 0x0d && bFar)	||							\
					(*bptr == 0x68)	||															\
					(*bptr == 0x6A))															\
				{																				\
					if ((*bptr == 0x8b && *(bptr + 1) == 0x25)	||								\
						(*bptr == 0x8b && *(bptr + 1) == 0x90))									\
					{																			\
						*bptr = 0xff;															\
						*(bptr + 1) = 0x25;														\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if ((*bptr == 0x00 && *(bptr + 1) == 0x90)	||							\
							(*bptr == 0x8b && *(bptr + 1) == 0x0d))								\
					{																			\
						*bptr = 0xff;															\
						*(bptr + 1) = 0x15;														\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0x68 || *bptr == 0x6A)									\
						*(++bptr) ^= dwNewseed;													\
																								\
					DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);					\
					dwOffsetseed = dwOffsetseed << 24;											\
					dwNewseed = dwNewseed >> 8;													\
					dwNewseed = dwOffsetseed | dwNewseed;										\
				}																				\
				else if (nFlag >= 0)															\
				{																				\
					if (nFlag == 1 || nFlag == 3)												\
					{																			\
						bptr++;																	\
						if (g_ScrambleTable(nFlag, *bptr)->m_nFlag == 0)						\
							*bptr = g_ScrambleTable(nFlag, *bptr)->m_byRValue;					\
					}																			\
					else																		\
					{																			\
						*bptr = g_ScrambleTable(nFlag, *bptr)->m_byRValue;						\
						if ((*bptr == 0x9A || *bptr == 0xEA) ||									\
							(*bptr == 0xE8 || *bptr == 0xE9))									\
						{																		\
							*(++bptr) ^= dwNewseed;												\
							DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);			\
							dwOffsetseed = dwOffsetseed << 24;									\
							dwNewseed = dwNewseed >> 8;											\
							dwNewseed = dwOffsetseed | dwNewseed;								\
						}																		\
					}																			\
				}																				\
				dwScrambleSize++;																\
				if (dwScrambleSize >= scrambleEIPTableSize)										\
					break;																		\
				bptr = (BYTE*)dwOffsetBase;														\
			}																					\
		}																						\
	}																							\
}

#define iviTR_LIGHTWEIGHT_DESCRAMBLE2(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable)	\
{																								\
	BYTE *bptr;																					\
	DWORD dwNewseed = seed;																		\
	WORD *pwEIPTable = pwScrambleEIPTable;														\
	if (pwScrambleEIPTable)																		\
	{																							\
		if(pwTable && *pwTable!=0xffff && DO_ISRELOCATED)										\
		{																						\
			WORD *pwOrigTable;																	\
																								\
			DWORD dwStart = (DWORD)start_addr;													\
			dwStart += DO_GETUNDOOFFSET;														\
			switch ((dwStart >> 8) % 2)															\
			{																					\
			case 0:																				\
				g_pScramTable = g_pScrambleTable;												\
				break;																			\
			case 1:																				\
				g_pScramTable = g_pScrambleTable1;												\
				break;																			\
			}																					\
			pwOrigTable = pwTable;																\
			DO_RELOCATECODE(start_addr, pwOrigTable, TRUE);										\
			DWORD dwOffsetBase = (DWORD)start_addr;												\
			bptr = (BYTE*)dwOffsetBase;															\
			while (*pwEIPTable != 0xFFFF && (DWORD)bptr < (DWORD)end_addr)						\
			{																					\
				bool bFar = (*pwEIPTable) & iviTR_TREXE_EIP_FAR_BIT;							\
				bptr += (*pwEIPTable | iviTR_TREXE_EIP_FAR_BIT) ^ iviTR_TREXE_EIP_FAR_BIT;		\
				pwEIPTable++;																	\
				dwOffsetBase = (DWORD)bptr;														\
				int nFlag = g_ScrambleTable(0, *bptr)->m_nFlag;									\
																								\
				if ((*bptr == 0x8b && *(bptr + 1) == 0x25 && bFar)	||							\
					(*bptr == 0x00 && *(bptr + 1) == 0x90 && bFar)	||							\
					(*bptr == 0x8b && *(bptr + 1) == 0x90 && bFar)	||							\
					(*bptr == 0x8b && *(bptr + 1) == 0x0d && bFar)	||							\
					(*bptr == 0x68)	||															\
					(*bptr == 0x6A))															\
				{																				\
					if ((*bptr == 0x8b && *(bptr + 1) == 0x25)	||								\
						(*bptr == 0x8b && *(bptr + 1) == 0x90))									\
					{																			\
						*bptr = 0xff;															\
						*(bptr + 1) = 0x25;														\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if ((*bptr == 0x00 && *(bptr + 1) == 0x90)	||							\
							(*bptr == 0x8b && *(bptr + 1) == 0x0d))								\
					{																			\
						*bptr = 0xff;															\
						*(bptr + 1) = 0x15;														\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0x68 || *bptr == 0x6A)									\
						*(++bptr) ^= dwNewseed;													\
																								\
					DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);					\
					dwOffsetseed = dwOffsetseed << 24;											\
					dwNewseed = dwNewseed >> 8;													\
					dwNewseed = dwOffsetseed | dwNewseed;										\
				}																				\
				else if (nFlag >= 0)															\
				{																				\
					if (nFlag == 1 || nFlag == 3)												\
					{																			\
						bptr++;																	\
						if (g_ScrambleTable(nFlag, *bptr)->m_nFlag == 0)						\
							*bptr = g_ScrambleTable(nFlag, *bptr)->m_byRValue;					\
					}																			\
					else																		\
					{																			\
						*bptr = g_ScrambleTable(nFlag, *bptr)->m_byRValue;						\
						if ((*bptr == 0x9A || *bptr == 0xEA) ||									\
							(*bptr == 0xE8 || *bptr == 0xE9))									\
						{																		\
							*(++bptr) ^= dwNewseed;												\
							DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);			\
							dwOffsetseed = dwOffsetseed << 24;									\
							dwNewseed = dwNewseed >> 8;											\
							dwNewseed = dwOffsetseed | dwNewseed;								\
						}																		\
					}																			\
				}																				\
				bptr = (BYTE*)dwOffsetBase;														\
			}																					\
			DO_RELOCATECODE(start_addr, pwOrigTable, FALSE);									\
		}																						\
		else																					\
		{																						\
			DWORD dwStart = (DWORD)start_addr;													\
			switch ((dwStart >> 8) % 2)															\
			{																					\
			case 0:																				\
				g_pScramTable = g_pScrambleTable;												\
				break;																			\
			case 1:																				\
				g_pScramTable = g_pScrambleTable1;												\
				break;																			\
			}																					\
			DWORD dwOffsetBase = (DWORD)start_addr;												\
			bptr = (BYTE*)dwOffsetBase;															\
			while (*pwEIPTable != 0xFFFF && (DWORD)bptr < (DWORD)end_addr)						\
			{																					\
				bool bFar = (*pwEIPTable) & iviTR_TREXE_EIP_FAR_BIT;							\
				bptr += (*pwEIPTable | iviTR_TREXE_EIP_FAR_BIT) ^ iviTR_TREXE_EIP_FAR_BIT;		\
				pwEIPTable++;																	\
				dwOffsetBase = (DWORD)bptr;														\
				int nFlag = g_ScrambleTable(0, *bptr)->m_nFlag;									\
																								\
				if ((*bptr == 0x8b && *(bptr + 1) == 0x25 && bFar)	||							\
					(*bptr == 0x00 && *(bptr + 1) == 0x90 && bFar)	||							\
					(*bptr == 0x8b && *(bptr + 1) == 0x90 && bFar)	||							\
					(*bptr == 0x8b && *(bptr + 1) == 0x0d && bFar)	||							\
					(*bptr == 0x68)	||															\
					(*bptr == 0x6A))															\
				{																				\
					if ((*bptr == 0x8b && *(bptr + 1) == 0x25)	||								\
						(*bptr == 0x8b && *(bptr + 1) == 0x90))									\
					{																			\
						*bptr = 0xff;															\
						*(bptr + 1) = 0x25;														\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if ((*bptr == 0x00 && *(bptr + 1) == 0x90)	||							\
							(*bptr == 0x8b && *(bptr + 1) == 0x0d))								\
					{																			\
						*bptr = 0xff;															\
						*(bptr + 1) = 0x15;														\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0x68 || *bptr == 0x6A)									\
						*(++bptr) ^= dwNewseed;													\
																								\
					DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);					\
					dwOffsetseed = dwOffsetseed << 24;											\
					dwNewseed = dwNewseed >> 8;													\
					dwNewseed = dwOffsetseed | dwNewseed;										\
				}																				\
				else if (nFlag >= 0)															\
				{																				\
					if (nFlag == 1 || nFlag == 3)												\
					{																			\
						bptr++;																	\
						if (g_ScrambleTable(nFlag, *bptr)->m_nFlag == 0)						\
							*bptr = g_ScrambleTable(nFlag, *bptr)->m_byRValue;					\
					}																			\
					else																		\
					{																			\
						*bptr = g_ScrambleTable(nFlag, *bptr)->m_byRValue;						\
						if ((*bptr == 0x9A || *bptr == 0xEA) ||									\
							(*bptr == 0xE8 || *bptr == 0xE9))									\
						{																		\
							*(++bptr) ^= dwNewseed;												\
							DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);			\
							dwOffsetseed = dwOffsetseed << 24;									\
							dwNewseed = dwNewseed >> 8;											\
							dwNewseed = dwOffsetseed | dwNewseed;								\
						}																		\
					}																			\
				}																				\
				bptr = (BYTE*)dwOffsetBase;														\
			}																					\
		}																						\
	}																							\
}

#define iviTR_LIGHTWEIGHT_SCRAMBLE(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable, scrambleEIPTableSize)							\
{																								\
	BYTE *bptr;																					\
	DWORD dwNewseed = seed;																		\
	WORD *pwEIPTable = pwScrambleEIPTable;														\
	if (pwScrambleEIPTable && scrambleEIPTableSize)												\
	{																							\
		if(pwTable && *pwTable!=0xffff && DO_ISRELOCATED)										\
		{																						\
			WORD *pwOrigTable;																	\
																								\
			DWORD dwScrambleSize = 0;	\
			DWORD dwStart = (DWORD)start_addr;													\
			dwStart += DO_GETUNDOOFFSET;														\
			switch ((dwStart >> 8) % 2)															\
			{																					\
			case 0:																				\
				g_pScramTable = g_pScrambleTable;												\
				break;																			\
			case 1:																				\
				g_pScramTable = g_pScrambleTable1;												\
				break;																			\
			}																					\
			pwOrigTable = pwTable;																\
			DO_RELOCATECODE(start_addr, pwOrigTable, TRUE);										\
			DWORD dwOffsetBase = (DWORD)start_addr;												\
			bptr = (BYTE*)dwOffsetBase;															\
			while (*pwEIPTable != 0xFFFF && (DWORD)bptr < (DWORD)end_addr)						\
			{																					\
				bool bFar = (*pwEIPTable) & iviTR_TREXE_EIP_FAR_BIT;							\
				bptr += (*pwEIPTable | iviTR_TREXE_EIP_FAR_BIT) ^ iviTR_TREXE_EIP_FAR_BIT;		\
				pwEIPTable++;																	\
				dwOffsetBase = (DWORD)bptr;														\
				int nFlag = g_ScrambleTable(0, *bptr)->m_nFlag;									\
																								\
				if ((*bptr == 0xff && *(bptr + 1) == 0x15) ||									\
					(*bptr == 0xff && *(bptr + 1) == 0x25)	||									\
					(*bptr == 0x68)	||															\
					(*bptr == 0x6A))															\
				{																				\
					if (*bptr == 0xff && *(bptr + 1) == 0x25)									\
					{																			\
						switch ((DWORD)bptr % 2)												\
						{																		\
						case 0:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x25;													\
							break;																\
						case 1:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x90;													\
							break;																\
						}																		\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0xff && *(bptr + 1) == 0x15)								\
					{																			\
						switch ((DWORD)bptr % 2)												\
						{																		\
						case 0:																	\
							*bptr = 0x00;														\
							*(bptr + 1) = 0x90;													\
							break;																\
						case 1:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x0d;													\
							break;																\
						}																		\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0x68 || *bptr == 0x6A)									\
						*(++bptr) ^= dwNewseed;													\
																								\
					DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);					\
					dwOffsetseed = dwOffsetseed << 24;											\
					dwNewseed = dwNewseed >> 8;													\
					dwNewseed = dwOffsetseed | dwNewseed;										\
				}																				\
				else if (nFlag >= 0)															\
				{																				\
					if (nFlag == 1 || nFlag == 3)												\
					{																			\
						bptr++;																	\
						if (g_ScrambleTable(nFlag, *bptr)->m_nFlag == 0)						\
							*bptr = g_ScrambleTable(nFlag, *bptr)->m_byValue;					\
					}																			\
					else																		\
					{																			\
						*bptr = g_ScrambleTable(nFlag, *bptr)->m_byValue;						\
						if ((*bptr == 0x9A || *bptr == 0xEA) ||									\
							(*bptr == 0xE8 || *bptr == 0xE9))									\
						{																		\
							*(++bptr) ^= dwNewseed;												\
							DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);			\
							dwOffsetseed = dwOffsetseed << 24;									\
							dwNewseed = dwNewseed >> 8;											\
							dwNewseed = dwOffsetseed | dwNewseed;								\
						}																		\
					}																			\
				}																				\
				dwScrambleSize++;																\
				if (dwScrambleSize >= scrambleEIPTableSize)										\
					break;																		\
				bptr = (BYTE*)dwOffsetBase;														\
			}																					\
			DO_RELOCATECODE(start_addr, pwOrigTable, FALSE);									\
		}																						\
		else																					\
		{																						\
			DWORD dwScrambleSize = 0;	\
			DWORD dwStart = (DWORD)start_addr;													\
			switch ((dwStart >> 8) % 2)															\
			{																					\
			case 0:																				\
				g_pScramTable = g_pScrambleTable;												\
				break;																			\
			case 1:																				\
				g_pScramTable = g_pScrambleTable1;												\
				break;																			\
			}																					\
			DWORD dwOffsetBase = (DWORD)start_addr;												\
			bptr = (BYTE*)dwOffsetBase;															\
			while (*pwEIPTable != 0xFFFF && (DWORD)bptr < (DWORD)end_addr)						\
			{																					\
				bool bFar = (*pwEIPTable) & iviTR_TREXE_EIP_FAR_BIT;							\
				bptr += (*pwEIPTable | iviTR_TREXE_EIP_FAR_BIT) ^ iviTR_TREXE_EIP_FAR_BIT;		\
				pwEIPTable++;																	\
				dwOffsetBase = (DWORD)bptr;														\
				int nFlag = g_ScrambleTable(0, *bptr)->m_nFlag;									\
																								\
				if ((*bptr == 0xff && *(bptr + 1) == 0x15) ||									\
					(*bptr == 0xff && *(bptr + 1) == 0x25)	||									\
					(*bptr == 0x68)	||															\
					(*bptr == 0x6A))															\
				{																				\
					if (*bptr == 0xff && *(bptr + 1) == 0x25)									\
					{																			\
						switch ((DWORD)bptr % 2)												\
						{																		\
						case 0:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x25;													\
							break;																\
						case 1:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x90;													\
							break;																\
						}																		\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0xff && *(bptr + 1) == 0x15)								\
					{																			\
						switch ((DWORD)bptr % 2)												\
						{																		\
						case 0:																	\
							*bptr = 0x00;														\
							*(bptr + 1) = 0x90;													\
							break;																\
						case 1:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x0d;													\
							break;																\
						}																		\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0x68 || *bptr == 0x6A)									\
						*(++bptr) ^= dwNewseed;													\
																								\
					DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);					\
					dwOffsetseed = dwOffsetseed << 24;											\
					dwNewseed = dwNewseed >> 8;													\
					dwNewseed = dwOffsetseed | dwNewseed;										\
				}																				\
				else if (nFlag >= 0)															\
				{																				\
					if (nFlag == 1 || nFlag == 3)												\
					{																			\
						bptr++;																	\
						if (g_ScrambleTable(nFlag, *bptr)->m_nFlag == 0)						\
							*bptr = g_ScrambleTable(nFlag, *bptr)->m_byValue;					\
					}																			\
					else																		\
					{																			\
						*bptr = g_ScrambleTable(nFlag, *bptr)->m_byValue;						\
						if ((*bptr == 0x9A || *bptr == 0xEA) ||									\
							(*bptr == 0xE8 || *bptr == 0xE9))									\
						{																		\
							*(++bptr) ^= dwNewseed;												\
							DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);			\
							dwOffsetseed = dwOffsetseed << 24;									\
							dwNewseed = dwNewseed >> 8;											\
							dwNewseed = dwOffsetseed | dwNewseed;								\
						}																		\
					}																			\
				}																				\
				dwScrambleSize++;																\
				if (dwScrambleSize >= scrambleEIPTableSize)										\
					break;																		\
				bptr = (BYTE*)dwOffsetBase;														\
			}																					\
		}																						\
	}																							\
}

#define iviTR_LIGHTWEIGHT_SCRAMBLE2(start_addr, end_addr, seed, pwTable, pwScrambleEIPTable)	\
{																								\
	BYTE *bptr;																					\
	DWORD dwNewseed = seed;																		\
	WORD *pwEIPTable = pwScrambleEIPTable;														\
	if (pwScrambleEIPTable)																		\
	{																							\
		if(pwTable && *pwTable!=0xffff && DO_ISRELOCATED)										\
		{																						\
			WORD *pwOrigTable;																	\
																								\
			DWORD dwStart = (DWORD)start_addr;													\
			dwStart += DO_GETUNDOOFFSET;														\
			switch ((dwStart >> 8) % 2)															\
			{																					\
			case 0:																				\
				g_pScramTable = g_pScrambleTable;												\
				break;																			\
			case 1:																				\
				g_pScramTable = g_pScrambleTable1;												\
				break;																			\
			}																					\
			pwOrigTable = pwTable;																\
			DO_RELOCATECODE(start_addr, pwOrigTable, TRUE);										\
			DWORD dwOffsetBase = (DWORD)start_addr;												\
			bptr = (BYTE*)dwOffsetBase;															\
			while (*pwEIPTable != 0xFFFF && (DWORD)bptr < (DWORD)end_addr)						\
			{																					\
				bool bFar = (*pwEIPTable) & iviTR_TREXE_EIP_FAR_BIT;							\
				bptr += (*pwEIPTable | iviTR_TREXE_EIP_FAR_BIT) ^ iviTR_TREXE_EIP_FAR_BIT;		\
				pwEIPTable++;																	\
				dwOffsetBase = (DWORD)bptr;														\
				int nFlag = g_ScrambleTable(0, *bptr)->m_nFlag;									\
																								\
				if ((*bptr == 0xff && *(bptr + 1) == 0x15) ||									\
					(*bptr == 0xff && *(bptr + 1) == 0x25)	||									\
					(*bptr == 0x68)	||															\
					(*bptr == 0x6A))															\
				{																				\
					if (*bptr == 0xff && *(bptr + 1) == 0x25)									\
					{																			\
						switch ((DWORD)bptr % 2)												\
						{																		\
						case 0:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x25;													\
							break;																\
						case 1:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x90;													\
							break;																\
						}																		\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0xff && *(bptr + 1) == 0x15)								\
					{																			\
						switch ((DWORD)bptr % 2)												\
						{																		\
						case 0:																	\
							*bptr = 0x00;														\
							*(bptr + 1) = 0x90;													\
							break;																\
						case 1:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x0d;													\
							break;																\
						}																		\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0x68 || *bptr == 0x6A)									\
						*(++bptr) ^= dwNewseed;													\
																								\
					DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);					\
					dwOffsetseed = dwOffsetseed << 24;											\
					dwNewseed = dwNewseed >> 8;													\
					dwNewseed = dwOffsetseed | dwNewseed;										\
				}																				\
				else if (nFlag >= 0)															\
				{																				\
					if (nFlag == 1 || nFlag == 3)												\
					{																			\
						bptr++;																	\
						if (g_ScrambleTable(nFlag, *bptr)->m_nFlag == 0)						\
							*bptr = g_ScrambleTable(nFlag, *bptr)->m_byValue;					\
					}																			\
					else																		\
					{																			\
						*bptr = g_ScrambleTable(nFlag, *bptr)->m_byValue;						\
						if ((*bptr == 0x9A || *bptr == 0xEA) ||									\
							(*bptr == 0xE8 || *bptr == 0xE9))									\
						{																		\
							*(++bptr) ^= dwNewseed;												\
							DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);			\
							dwOffsetseed = dwOffsetseed << 24;									\
							dwNewseed = dwNewseed >> 8;											\
							dwNewseed = dwOffsetseed | dwNewseed;								\
						}																		\
					}																			\
				}																				\
				bptr = (BYTE*)dwOffsetBase;														\
			}																					\
			DO_RELOCATECODE(start_addr, pwOrigTable, FALSE);									\
		}																						\
		else																					\
		{																						\
			DWORD dwStart = (DWORD)start_addr;													\
			switch ((dwStart >> 8) % 2)															\
			{																					\
			case 0:																				\
				g_pScramTable = g_pScrambleTable;												\
				break;																			\
			case 1:																				\
				g_pScramTable = g_pScrambleTable1;												\
				break;																			\
			}																					\
			DWORD dwOffsetBase = (DWORD)start_addr;												\
			bptr = (BYTE*)dwOffsetBase;															\
			while (*pwEIPTable != 0xFFFF && (DWORD)bptr < (DWORD)end_addr)						\
			{																					\
				bool bFar = (*pwEIPTable) & iviTR_TREXE_EIP_FAR_BIT;							\
				bptr += (*pwEIPTable | iviTR_TREXE_EIP_FAR_BIT) ^ iviTR_TREXE_EIP_FAR_BIT;		\
				pwEIPTable++;																	\
				dwOffsetBase = (DWORD)bptr;														\
				int nFlag = g_ScrambleTable(0, *bptr)->m_nFlag;									\
																								\
				if ((*bptr == 0xff && *(bptr + 1) == 0x15) ||									\
					(*bptr == 0xff && *(bptr + 1) == 0x25)	||									\
					(*bptr == 0x68)	||															\
					(*bptr == 0x6A))															\
				{																				\
					if (*bptr == 0xff && *(bptr + 1) == 0x25)									\
					{																			\
						switch ((DWORD)bptr % 2)												\
						{																		\
						case 0:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x25;													\
							break;																\
						case 1:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x90;													\
							break;																\
						}																		\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0xff && *(bptr + 1) == 0x15)								\
					{																			\
						switch ((DWORD)bptr % 2)												\
						{																		\
						case 0:																	\
							*bptr = 0x00;														\
							*(bptr + 1) = 0x90;													\
							break;																\
						case 1:																	\
							*bptr = 0x8B;														\
							*(bptr + 1) = 0x0d;													\
							break;																\
						}																		\
						bptr++;																	\
						*(++bptr) ^= dwNewseed;													\
						*(++bptr) ^= dwNewseed >> 8;											\
						*(++bptr) ^= dwNewseed >> 16;											\
						*(++bptr) ^= dwNewseed >> 24;											\
					}																			\
					else if (*bptr == 0x68 || *bptr == 0x6A)									\
						*(++bptr) ^= dwNewseed;													\
																								\
					DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);					\
					dwOffsetseed = dwOffsetseed << 24;											\
					dwNewseed = dwNewseed >> 8;													\
					dwNewseed = dwOffsetseed | dwNewseed;										\
				}																				\
				else if (nFlag >= 0)															\
				{																				\
					if (nFlag == 1 || nFlag == 3)												\
					{																			\
						bptr++;																	\
						if (g_ScrambleTable(nFlag, *bptr)->m_nFlag == 0)						\
							*bptr = g_ScrambleTable(nFlag, *bptr)->m_byValue;					\
					}																			\
					else																		\
					{																			\
						*bptr = g_ScrambleTable(nFlag, *bptr)->m_byValue;						\
						if ((*bptr == 0x9A || *bptr == 0xEA) ||									\
							(*bptr == 0xE8 || *bptr == 0xE9))									\
						{																		\
							*(++bptr) ^= dwNewseed;												\
							DWORD dwOffsetseed = dwNewseed - ((dwNewseed >> 8) << 8);			\
							dwOffsetseed = dwOffsetseed << 24;									\
							dwNewseed = dwNewseed >> 8;											\
							dwNewseed = dwOffsetseed | dwNewseed;								\
						}																		\
					}																			\
				}																				\
				bptr = (BYTE*)dwOffsetBase;														\
			}																					\
		}																						\
	}																							\
}

#endif // _IVISCRAMBLE_GLOBALDEF_H