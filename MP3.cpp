#include "MP3.h"

#include "ZAssert.h"
#include "Common.h"
#include "CommonMath.h"
#include "PlatformMemory.h"

#include <cstring>

/*

NOTE: This is not meant to be an all encompassing MP3 handler.
  It is meant to only decode MP3 (MPEG 1 Layer 3).
  We do not care about supporting MPEG 2 audio.
  Right now it just decodes an entire file and gives you back a 16-bit PCM signal which can be pushed to an audio device.

  References below were essential in getting this working properly.
  The research papers had good information about the theory and file layout.
  Heavily borrowed from minimp3 so credit to those folks, this is not meant to be a general replacement that competes in terms of performance/conformance/etc.

References: 
    http://www.mp3-tech.org/programmer/docs/mp3_theory.pdf
    https://www.diva-portal.org/smash/get/diva2:830195/FULLTEXT01.pdf
    https://github.com/lieff/minimp3
*/

namespace ZSharp {

/*
Lookup tables used for decoding scalefactors and huffman trees.
*/
static const uint8 LongScaleFactors[8][23] = {
  { 6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54,0 },
  { 12,12,12,12,12,12,16,20,24,28,32,40,48,56,64,76,90,2,2,2,2,2,0 },
  { 6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54,0 },
  { 6,6,6,6,6,6,8,10,12,14,16,18,22,26,32,38,46,54,62,70,76,36,0 },
  { 6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54,0 },
  { 4,4,4,4,4,4,6,6,8,8,10,12,16,20,24,28,34,42,50,54,76,158,0 },
  { 4,4,4,4,4,4,6,6,6,8,10,12,16,18,22,28,34,40,46,54,54,192,0 },
  { 4,4,4,4,4,4,6,6,8,10,12,16,20,24,30,38,46,56,68,84,102,26,0 }
};

static const uint8 ShortScaleFactors[8][40] = {
  { 4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0 },
  { 8,8,8,8,8,8,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0 },
  { 4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0 },
  { 4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0 },
  { 4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0 },
  { 4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0 },
  { 4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0 },
  { 4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0 }
};

static const uint8 MixedScaleFactors[8][40] = {
  { 6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0 },
  { 12,12,12,4,4,4,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0 },
  { 6,6,6,6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0 },
  { 6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0 },
  { 6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0 },
  { 4,4,4,4,4,4,6,6,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0 },
  { 4,4,4,4,4,4,6,6,4,4,4,6,6,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0 },
  { 4,4,4,4,4,4,6,6,4,4,4,6,6,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0 }
};

static const uint8 ScaleFactorPartitions[3][28] = {
  { 6,5,5, 5,6,5,5,5,6,5, 7,3,11,10,0,0, 7, 7, 7,0, 6, 6,6,3, 8, 8,5,0 },
  { 8,9,6,12,6,9,9,9,6,9,12,6,15,18,0,0, 6,15,12,0, 6,12,9,6, 6,18,9,0 },
  { 9,9,6,12,9,9,9,9,9,9,12,6,18,18,0,0,12,12,12,0,12, 9,9,6,15,12,9,0 }
};

static const int16 HuffmanTables[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  785,785,785,785,784,784,784,784,513,513,513,513,513,513,513,513,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,
  -255,1313,1298,1282,785,785,785,785,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,290,288,
  -255,1313,1298,1282,769,769,769,769,529,529,529,529,529,529,529,529,528,528,528,528,528,528,528,528,512,512,512,512,512,512,512,512,290,288,
  -253,-318,-351,-367,785,785,785,785,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,819,818,547,547,275,275,275,275,561,560,515,546,289,274,288,258,
  -254,-287,1329,1299,1314,1312,1057,1057,1042,1042,1026,1026,784,784,784,784,529,529,529,529,529,529,529,529,769,769,769,769,768,768,768,768,563,560,306,306,291,259,
  -252,-413,-477,-542,1298,-575,1041,1041,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-383,-399,1107,1092,1106,1061,849,849,789,789,1104,1091,773,773,1076,1075,341,340,325,309,834,804,577,577,532,532,516,516,832,818,803,816,561,561,531,531,515,546,289,289,288,258,
  -252,-429,-493,-559,1057,1057,1042,1042,529,529,529,529,529,529,529,529,784,784,784,784,769,769,769,769,512,512,512,512,512,512,512,512,-382,1077,-415,1106,1061,1104,849,849,789,789,1091,1076,1029,1075,834,834,597,581,340,340,339,324,804,833,532,532,832,772,818,803,817,787,816,771,290,290,290,290,288,258,
  -253,-349,-414,-447,-463,1329,1299,-479,1314,1312,1057,1057,1042,1042,1026,1026,785,785,785,785,784,784,784,784,769,769,769,769,768,768,768,768,-319,851,821,-335,836,850,805,849,341,340,325,336,533,533,579,579,564,564,773,832,578,548,563,516,321,276,306,291,304,259,
  -251,-572,-733,-830,-863,-879,1041,1041,784,784,784,784,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-511,-527,-543,1396,1351,1381,1366,1395,1335,1380,-559,1334,1138,1138,1063,1063,1350,1392,1031,1031,1062,1062,1364,1363,1120,1120,1333,1348,881,881,881,881,375,374,359,373,343,358,341,325,791,791,1123,1122,-703,1105,1045,-719,865,865,790,790,774,774,1104,1029,338,293,323,308,-799,-815,833,788,772,818,803,816,322,292,307,320,561,531,515,546,289,274,288,258,
  -251,-525,-605,-685,-765,-831,-846,1298,1057,1057,1312,1282,785,785,785,785,784,784,784,784,769,769,769,769,512,512,512,512,512,512,512,512,1399,1398,1383,1367,1382,1396,1351,-511,1381,1366,1139,1139,1079,1079,1124,1124,1364,1349,1363,1333,882,882,882,882,807,807,807,807,1094,1094,1136,1136,373,341,535,535,881,775,867,822,774,-591,324,338,-671,849,550,550,866,864,609,609,293,336,534,534,789,835,773,-751,834,804,308,307,833,788,832,772,562,562,547,547,305,275,560,515,290,290,
  -252,-397,-477,-557,-622,-653,-719,-735,-750,1329,1299,1314,1057,1057,1042,1042,1312,1282,1024,1024,785,785,785,785,784,784,784,784,769,769,769,769,-383,1127,1141,1111,1126,1140,1095,1110,869,869,883,883,1079,1109,882,882,375,374,807,868,838,881,791,-463,867,822,368,263,852,837,836,-543,610,610,550,550,352,336,534,534,865,774,851,821,850,805,593,533,579,564,773,832,578,578,548,548,577,577,307,276,306,291,516,560,259,259,
  -250,-2107,-2507,-2764,-2909,-2974,-3007,-3023,1041,1041,1040,1040,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-767,-1052,-1213,-1277,-1358,-1405,-1469,-1535,-1550,-1582,-1614,-1647,-1662,-1694,-1726,-1759,-1774,-1807,-1822,-1854,-1886,1565,-1919,-1935,-1951,-1967,1731,1730,1580,1717,-1983,1729,1564,-1999,1548,-2015,-2031,1715,1595,-2047,1714,-2063,1610,-2079,1609,-2095,1323,1323,1457,1457,1307,1307,1712,1547,1641,1700,1699,1594,1685,1625,1442,1442,1322,1322,-780,-973,-910,1279,1278,1277,1262,1276,1261,1275,1215,1260,1229,-959,974,974,989,989,-943,735,478,478,495,463,506,414,-1039,1003,958,1017,927,942,987,957,431,476,1272,1167,1228,-1183,1256,-1199,895,895,941,941,1242,1227,1212,1135,1014,1014,490,489,503,487,910,1013,985,925,863,894,970,955,1012,847,-1343,831,755,755,984,909,428,366,754,559,-1391,752,486,457,924,997,698,698,983,893,740,740,908,877,739,739,667,667,953,938,497,287,271,271,683,606,590,712,726,574,302,302,738,736,481,286,526,725,605,711,636,724,696,651,589,681,666,710,364,467,573,695,466,466,301,465,379,379,709,604,665,679,316,316,634,633,436,436,464,269,424,394,452,332,438,363,347,408,393,448,331,422,362,407,392,421,346,406,391,376,375,359,1441,1306,-2367,1290,-2383,1337,-2399,-2415,1426,1321,-2431,1411,1336,-2447,-2463,-2479,1169,1169,1049,1049,1424,1289,1412,1352,1319,-2495,1154,1154,1064,1064,1153,1153,416,390,360,404,403,389,344,374,373,343,358,372,327,357,342,311,356,326,1395,1394,1137,1137,1047,1047,1365,1392,1287,1379,1334,1364,1349,1378,1318,1363,792,792,792,792,1152,1152,1032,1032,1121,1121,1046,1046,1120,1120,1030,1030,-2895,1106,1061,1104,849,849,789,789,1091,1076,1029,1090,1060,1075,833,833,309,324,532,532,832,772,818,803,561,561,531,560,515,546,289,274,288,258,
  -250,-1179,-1579,-1836,-1996,-2124,-2253,-2333,-2413,-2477,-2542,-2574,-2607,-2622,-2655,1314,1313,1298,1312,1282,785,785,785,785,1040,1040,1025,1025,768,768,768,768,-766,-798,-830,-862,-895,-911,-927,-943,-959,-975,-991,-1007,-1023,-1039,-1055,-1070,1724,1647,-1103,-1119,1631,1767,1662,1738,1708,1723,-1135,1780,1615,1779,1599,1677,1646,1778,1583,-1151,1777,1567,1737,1692,1765,1722,1707,1630,1751,1661,1764,1614,1736,1676,1763,1750,1645,1598,1721,1691,1762,1706,1582,1761,1566,-1167,1749,1629,767,766,751,765,494,494,735,764,719,749,734,763,447,447,748,718,477,506,431,491,446,476,461,505,415,430,475,445,504,399,460,489,414,503,383,474,429,459,502,502,746,752,488,398,501,473,413,472,486,271,480,270,-1439,-1455,1357,-1471,-1487,-1503,1341,1325,-1519,1489,1463,1403,1309,-1535,1372,1448,1418,1476,1356,1462,1387,-1551,1475,1340,1447,1402,1386,-1567,1068,1068,1474,1461,455,380,468,440,395,425,410,454,364,467,466,464,453,269,409,448,268,432,1371,1473,1432,1417,1308,1460,1355,1446,1459,1431,1083,1083,1401,1416,1458,1445,1067,1067,1370,1457,1051,1051,1291,1430,1385,1444,1354,1415,1400,1443,1082,1082,1173,1113,1186,1066,1185,1050,-1967,1158,1128,1172,1097,1171,1081,-1983,1157,1112,416,266,375,400,1170,1142,1127,1065,793,793,1169,1033,1156,1096,1141,1111,1155,1080,1126,1140,898,898,808,808,897,897,792,792,1095,1152,1032,1125,1110,1139,1079,1124,882,807,838,881,853,791,-2319,867,368,263,822,852,837,866,806,865,-2399,851,352,262,534,534,821,836,594,594,549,549,593,593,533,533,848,773,579,579,564,578,548,563,276,276,577,576,306,291,516,560,305,305,275,259,
  -251,-892,-2058,-2620,-2828,-2957,-3023,-3039,1041,1041,1040,1040,769,769,769,769,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,-511,-527,-543,-559,1530,-575,-591,1528,1527,1407,1526,1391,1023,1023,1023,1023,1525,1375,1268,1268,1103,1103,1087,1087,1039,1039,1523,-604,815,815,815,815,510,495,509,479,508,463,507,447,431,505,415,399,-734,-782,1262,-815,1259,1244,-831,1258,1228,-847,-863,1196,-879,1253,987,987,748,-767,493,493,462,477,414,414,686,669,478,446,461,445,474,429,487,458,412,471,1266,1264,1009,1009,799,799,-1019,-1276,-1452,-1581,-1677,-1757,-1821,-1886,-1933,-1997,1257,1257,1483,1468,1512,1422,1497,1406,1467,1496,1421,1510,1134,1134,1225,1225,1466,1451,1374,1405,1252,1252,1358,1480,1164,1164,1251,1251,1238,1238,1389,1465,-1407,1054,1101,-1423,1207,-1439,830,830,1248,1038,1237,1117,1223,1148,1236,1208,411,426,395,410,379,269,1193,1222,1132,1235,1221,1116,976,976,1192,1162,1177,1220,1131,1191,963,963,-1647,961,780,-1663,558,558,994,993,437,408,393,407,829,978,813,797,947,-1743,721,721,377,392,844,950,828,890,706,706,812,859,796,960,948,843,934,874,571,571,-1919,690,555,689,421,346,539,539,944,779,918,873,932,842,903,888,570,570,931,917,674,674,-2575,1562,-2591,1609,-2607,1654,1322,1322,1441,1441,1696,1546,1683,1593,1669,1624,1426,1426,1321,1321,1639,1680,1425,1425,1305,1305,1545,1668,1608,1623,1667,1592,1638,1666,1320,1320,1652,1607,1409,1409,1304,1304,1288,1288,1664,1637,1395,1395,1335,1335,1622,1636,1394,1394,1319,1319,1606,1621,1392,1392,1137,1137,1137,1137,345,390,360,375,404,373,1047,-2751,-2767,-2783,1062,1121,1046,-2799,1077,-2815,1106,1061,789,789,1105,1104,263,355,310,340,325,354,352,262,339,324,1091,1076,1029,1090,1060,1075,833,833,788,788,1088,1028,818,818,803,803,561,561,531,531,816,771,546,546,289,274,288,258,
  -253,-317,-381,-446,-478,-509,1279,1279,-811,-1179,-1451,-1756,-1900,-2028,-2189,-2253,-2333,-2414,-2445,-2511,-2526,1313,1298,-2559,1041,1041,1040,1040,1025,1025,1024,1024,1022,1007,1021,991,1020,975,1019,959,687,687,1018,1017,671,671,655,655,1016,1015,639,639,758,758,623,623,757,607,756,591,755,575,754,559,543,543,1009,783,-575,-621,-685,-749,496,-590,750,749,734,748,974,989,1003,958,988,973,1002,942,987,957,972,1001,926,986,941,971,956,1000,910,985,925,999,894,970,-1071,-1087,-1102,1390,-1135,1436,1509,1451,1374,-1151,1405,1358,1480,1420,-1167,1507,1494,1389,1342,1465,1435,1450,1326,1505,1310,1493,1373,1479,1404,1492,1464,1419,428,443,472,397,736,526,464,464,486,457,442,471,484,482,1357,1449,1434,1478,1388,1491,1341,1490,1325,1489,1463,1403,1309,1477,1372,1448,1418,1433,1476,1356,1462,1387,-1439,1475,1340,1447,1402,1474,1324,1461,1371,1473,269,448,1432,1417,1308,1460,-1711,1459,-1727,1441,1099,1099,1446,1386,1431,1401,-1743,1289,1083,1083,1160,1160,1458,1445,1067,1067,1370,1457,1307,1430,1129,1129,1098,1098,268,432,267,416,266,400,-1887,1144,1187,1082,1173,1113,1186,1066,1050,1158,1128,1143,1172,1097,1171,1081,420,391,1157,1112,1170,1142,1127,1065,1169,1049,1156,1096,1141,1111,1155,1080,1126,1154,1064,1153,1140,1095,1048,-2159,1125,1110,1137,-2175,823,823,1139,1138,807,807,384,264,368,263,868,838,853,791,867,822,852,837,866,806,865,790,-2319,851,821,836,352,262,850,805,849,-2399,533,533,835,820,336,261,578,548,563,577,532,532,832,772,562,562,547,547,305,275,560,515,290,290,288,258 };
static const uint8 HuffmanTable32[] = { 
  130,162,193,209,44,28,76,140,9,9,9,9,9,9,9,9,190,254,222,238,126,94,157,157,109,61,173,205 
};
static const uint8 HuffmanTable33[] = { 
  252,236,220,204,188,172,156,140,124,108,92,76,60,44,28,12 
};
static const int16 HuffmanTableIndex[2 * 16] = { 
  0,32,64,98,0,132,180,218,292,364,426,538,648,746,0,1126,1460,1460,1460,1460,1460,1460,1460,1460,1842,1842,1842,1842,1842,1842,1842,1842 
};
static const uint8 LineBits[] = { 
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,6,8,10,13,4,5,6,7,8,9,11,13 
};

static const float Power43Table[145] = {
  0,-1,-2.519842f,-4.326749f,-6.349604f,-8.549880f,-10.902724f,-13.390518f,-16.000000f,-18.720754f,-21.544347f,-24.463781f,-27.473142f,-30.567351f,-33.741992f,-36.993181f,
  0,1,2.519842f,4.326749f,6.349604f,8.549880f,10.902724f,13.390518f,16.000000f,18.720754f,21.544347f,24.463781f,27.473142f,30.567351f,33.741992f,36.993181f,40.317474f,43.711787f,47.173345f,50.699631f,54.288352f,57.937408f,61.644865f,65.408941f,69.227979f,73.100443f,77.024898f,81.000000f,85.024491f,89.097188f,93.216975f,97.382800f,101.593667f,105.848633f,110.146801f,114.487321f,118.869381f,123.292209f,127.755065f,132.257246f,136.798076f,141.376907f,145.993119f,150.646117f,155.335327f,160.060199f,164.820202f,169.614826f,174.443577f,179.305980f,184.201575f,189.129918f,194.090580f,199.083145f,204.107210f,209.162385f,214.248292f,219.364564f,224.510845f,229.686789f,234.892058f,240.126328f,245.389280f,250.680604f,256.000000f,261.347174f,266.721841f,272.123723f,277.552547f,283.008049f,288.489971f,293.998060f,299.532071f,305.091761f,310.676898f,316.287249f,321.922592f,327.582707f,333.267377f,338.976394f,344.709550f,350.466646f,356.247482f,362.051866f,367.879608f,373.730522f,379.604427f,385.501143f,391.420496f,397.362314f,403.326427f,409.312672f,415.320884f,421.350905f,427.402579f,433.475750f,439.570269f,445.685987f,451.822757f,457.980436f,464.158883f,470.357960f,476.577530f,482.817459f,489.077615f,495.357868f,501.658090f,507.978156f,514.317941f,520.677324f,527.056184f,533.454404f,539.871867f,546.308458f,552.764065f,559.238575f,565.731879f,572.243870f,578.774440f,585.323483f,591.890898f,598.476581f,605.080431f,611.702349f,618.342238f,625.000000f,631.675540f,638.368763f,645.079578f
};

/*
Compressed data can straddle frames.
We must copy remaining unprocessed data to a temporary buffer before we can decode subsequent frames properly.
We set aside 5 frames of space which should be enough.

Put another way, we have a known amount of frame data available after decoding header + side information.
Rather than skipping the remaining data post-decode and the start of next frame, we save it to a temporary buffer.
Prior to decoding the next frame, we copy back the leftover data to a buffer and append the current frame data to it.
*/
uint8 MainDataCache[511] = {};
uint8 MainDataResevoir[2815] = {};
size_t MainDataCacheLength = 0;

MP3::MP3(const FileString& path) : mReader(path) {
  if (mReader.IsOpen()) {
    mDataStream = (uint8*)mReader.GetBuffer();
    mStreamSize = mReader.GetSize();
    bool foundFirstFrame = ReadFileHeader();
    ZAssert(foundFirstFrame);
    (void)foundFirstFrame; // To stop release builds triggering a warning.
  }
}

/*
    Each frame has two granules, similar to how the side information is laid out.
    Stereo will have two channels for each granule, mono has one.
    Repeat the following steps for each granule below.

    1) Decode scalefactors
    2) Decode Huffman data (hard coded tables defined in the standard)
    3) Requantize spectrum
    4) Reorder spectrum
    5) Channel processing
    6) IMDCT
    7) Frequency inversion
    8) Synthesize polyphase filter banks
*/
MP3::PCMAudio MP3::Decompress() {
  if (mDataStream == nullptr) {
    PCMAudio pcmAudio;
    return pcmAudio;
  }

  int16* decompressedSamples = nullptr;

  size_t guessedSize = 0;
  size_t numSamples = 0;

  float* intermediateValues = (float*)PlatformCalloc(2 * 576 * sizeof(float));
  float* overlapValues = (float*)PlatformCalloc(2 * 288 * sizeof(float));
  float* qmfState = (float*)PlatformCalloc(960 * sizeof(float));

  PCMAudio pcmAudio;
  size_t sampleRateBits = 0;
  size_t numChannels = 0;

  while (((mBitOffset / 8) + 4) < mStreamSize) {
    const FrameHeader header(ReadFrameHeader());

    if ((mBitOffset / 8) + header.frameBytes >= mStreamSize) {
      break;
    }

    SideInfomation sideInfo(ReadSideInformation(header.sampleRateBits, header.mode, header.modeExt));

    numChannels = sideInfo.numChannels;
    sampleRateBits = header.sampleRateBits;

    // We may have some remaining data lingering from previous frames, copy it over.
    const size_t headerAndSideInfoSize = sideInfo.byteLength + 4;
    const size_t frameSize = header.frameBytes - headerAndSideInfoSize;

    const size_t nextFrameIndex = mBitOffset + (frameSize * 8);

    size_t cacheOffset = Min(MainDataCacheLength, sideInfo.mainData);
    size_t reverseIndex = (sideInfo.mainData > MainDataCacheLength) ? 0 : MainDataCacheLength - sideInfo.mainData;

    memcpy(MainDataResevoir, MainDataCache + reverseIndex, cacheOffset);
    memcpy(MainDataResevoir + cacheOffset, mDataStream + (mBitOffset / 8), frameSize);

    if (decompressedSamples == nullptr) {
      // Make a guess as to how big the buffer is going to be for storing an entire audio track.
      const size_t numFrames = (mStreamSize / header.frameBytes) - 1;
      const size_t totalSize = numFrames * 1152 * (numChannels / 2) * sizeof(int16);
      decompressedSamples = (int16*)PlatformMalloc(totalSize);
      memset(decompressedSamples, 0, totalSize);
      guessedSize = totalSize;
    }

    const size_t frameSampleIncrement = 1152 * (sideInfo.numChannels / 2) * sizeof(int16);
    numSamples += frameSampleIncrement;

    if (guessedSize + frameSampleIncrement < numSamples) {
      decompressedSamples = (int16*)PlatformReAlloc(decompressedSamples, numSamples + frameSampleIncrement);
      memset(((uint8*)decompressedSamples) + numSamples, 0, frameSampleIncrement);
    }

    size_t resevoirBitsRead = 0;
    DecodeMainData(MainDataResevoir, 
      resevoirBitsRead, 
      sideInfo, 
      header.modeExt, 
      header.sampleRateBits, 
      intermediateValues, 
      overlapValues, 
      decompressedSamples + (numSamples / 4),
      qmfState);

    MainDataCacheLength = (cacheOffset + frameSize) - (resevoirBitsRead / 8);
    if (MainDataCacheLength > 0) {
      size_t slack = 0;
      if ((resevoirBitsRead % 8) > 0) {
        --MainDataCacheLength;
        slack = 1;
      }

      memset(MainDataCache, 0, sizeof(MainDataCache));
      memcpy(MainDataCache, MainDataResevoir + ((resevoirBitsRead / 8) + slack), MainDataCacheLength);
    }

    memset(MainDataResevoir, 0, sizeof(MainDataResevoir));

    mBitOffset = nextFrameIndex;
  }

  PlatformFree(intermediateValues);
  PlatformFree(overlapValues);
  PlatformFree(qmfState);

  pcmAudio.data = decompressedSamples;
  pcmAudio.length = numSamples;
  pcmAudio.channels = numChannels / 2;

  switch (sampleRateBits) {
    case 0x00:
      pcmAudio.samplesPerSecond = 44100;
      break;
    case 0x01:
      pcmAudio.samplesPerSecond = 48000;
      break;
    case 0x02:
      pcmAudio.samplesPerSecond = 32000;
      break;
    default:
      pcmAudio.samplesPerSecond = 0;
      break;
  }

  return pcmAudio;
}

MP3::PCMAudioFloat MP3::DecompressFloat() {
  if (mDataStream == nullptr) {
    PCMAudioFloat pcmAudio;
    return pcmAudio;
  }

  float* decompressedSamples = nullptr;

  size_t guessedSize = 0;
  size_t numSamples = 0;

  float* intermediateValues = (float*)PlatformCalloc(2 * 576 * sizeof(float));
  float* overlapValues = (float*)PlatformCalloc(2 * 288 * sizeof(float));
  float* qmfState = (float*)PlatformCalloc(960 * sizeof(float));

  PCMAudioFloat pcmAudio;
  size_t sampleRateBits = 0;
  size_t numChannels = 0;

  while (((mBitOffset / 8) + 4) < mStreamSize) {
    const FrameHeader header(ReadFrameHeader());

    if ((mBitOffset / 8) + header.frameBytes >= mStreamSize) {
      break;
    }

    SideInfomation sideInfo(ReadSideInformation(header.sampleRateBits, header.mode, header.modeExt));

    numChannels = sideInfo.numChannels;
    sampleRateBits = header.sampleRateBits;

    // We may have some remaining data lingering from previous frames, copy it over.
    const size_t headerAndSideInfoSize = sideInfo.byteLength + 4;
    const size_t frameSize = header.frameBytes - headerAndSideInfoSize;

    const size_t nextFrameIndex = mBitOffset + (frameSize * 8);

    size_t cacheOffset = Min(MainDataCacheLength, sideInfo.mainData);
    size_t reverseIndex = (sideInfo.mainData > MainDataCacheLength) ? 0 : MainDataCacheLength - sideInfo.mainData;

    memcpy(MainDataResevoir, MainDataCache + reverseIndex, cacheOffset);
    memcpy(MainDataResevoir + cacheOffset, mDataStream + (mBitOffset / 8), frameSize);

    size_t resevoirBitsRead = 0;

    if (decompressedSamples == nullptr) {
      // Make a guess as to how big the buffer is going to be for storing an entire audio track.
      const size_t numFrames = (mStreamSize / header.frameBytes) - 1;
      const size_t totalSize = numFrames * 1152 * (numChannels / 2) * sizeof(float);
      decompressedSamples = (float*)PlatformCalloc(totalSize);
      guessedSize = totalSize;

      DecodeMainDataFloat(MainDataResevoir,
        resevoirBitsRead,
        sideInfo,
        header.modeExt,
        header.sampleRateBits,
        intermediateValues,
        overlapValues,
        decompressedSamples,
        qmfState);
    }
    else {
      const size_t frameSampleIncrement = 1152 * (numChannels / 2) * sizeof(float);
      numSamples += frameSampleIncrement;

      if (guessedSize + frameSampleIncrement < numSamples) {
        decompressedSamples = (float*)PlatformReAlloc(decompressedSamples, numSamples + frameSampleIncrement);
        memset(((uint8*)decompressedSamples) + numSamples, 0, frameSampleIncrement);
      }

      DecodeMainDataFloat(MainDataResevoir,
        resevoirBitsRead,
        sideInfo,
        header.modeExt,
        header.sampleRateBits,
        intermediateValues,
        overlapValues,
        decompressedSamples + (numSamples / 8),
        qmfState);
    }

    MainDataCacheLength = (cacheOffset + frameSize) - (resevoirBitsRead / 8);
    if (MainDataCacheLength > 0) {
      size_t slack = 0;
      if ((resevoirBitsRead % 8) > 0) {
        --MainDataCacheLength;
        slack = 1;
      }

      memset(MainDataCache, 0, sizeof(MainDataCache));
      memcpy(MainDataCache, MainDataResevoir + ((resevoirBitsRead / 8) + slack), MainDataCacheLength);
    }

    memset(MainDataResevoir, 0, sizeof(MainDataResevoir));

    mBitOffset = nextFrameIndex;
  }

  PlatformFree(intermediateValues);
  PlatformFree(overlapValues);
  PlatformFree(qmfState);

  pcmAudio.data = decompressedSamples;
  pcmAudio.length = numSamples;
  pcmAudio.channels = numChannels / 2;

  switch (sampleRateBits) {
    case 0x00:
      pcmAudio.samplesPerSecond = 44100;
      break;
    case 0x01:
      pcmAudio.samplesPerSecond = 48000;
      break;
    case 0x02:
      pcmAudio.samplesPerSecond = 32000;
      break;
    default:
      pcmAudio.samplesPerSecond = 0;
      break;
  }

  return pcmAudio;
}

uint32 MP3::ReadBits(uint8* buffer, size_t& bitOffset, size_t count, Endian endian) {
  if (count == 0 || count > 32) {
    return 0;
  }

  const uint8* data = (buffer + (bitOffset / 8));
  const size_t bitsReadInByte = (bitOffset % 8);

  // Read the first byte, drop unneeded bits.
  // We may need to read an entire 32bit word so we must have some extra space for carring bits.
  uint64 bytesRead = 0;
  uint64 ret = data[bytesRead];

  for (size_t i = (8 - bitsReadInByte); i < count; i += 8) {
    ++bytesRead;
    if (endian == Big) {
      ret |= (((uint64)data[bytesRead]) << (bytesRead * 8));
    }
    else {
      ret <<= 8;
      ret |= (uint64)data[bytesRead];
    }
  }

  if (endian == Big) {
    ret >>= bitsReadInByte;
  }
  else {
    ret >>= (((bytesRead + 1) * 8) - bitsReadInByte - count);
  }

  ret &= ((1ULL << count) - 1ULL);

  bitOffset += count;
  return (uint32)ret;
}

bool MP3::ReadFileHeader() {
  if (mDataStream == nullptr) {
    return false;
  }

  // Check for ID3vX header. This is an informal spec but widely used to store metadata.
  // Not to be confused with "optional side information" from the MP3 spec.
  // ID3v2 header is 10 bytes.

  // Main site seems to be down for some time, wayback machine has a cached version.
  // https://web.archive.org/web/20130116040619/http://id3.org/id3v2.4.0-structure
  uint32 id3Hdr = ReadBits(mDataStream, mBitOffset, 24, Big);
  if (memcmp(&id3Hdr, "ID3", 3)) {
    return false; // TODO: Skip directly to the first frame if file doesn't begin with this header?
  }

  uint32 majorVersion = ReadBits(mDataStream, mBitOffset, 8, Big);
  uint32 revisionNumber = ReadBits(mDataStream, mBitOffset, 8, Big);
  uint32 unsync = ReadBits(mDataStream, mBitOffset, 1, Big);
  uint32 extendedHeader = ReadBits(mDataStream, mBitOffset, 1, Big);
  uint32 experimental = ReadBits(mDataStream, mBitOffset, 1, Big);

  mBitOffset += 5; // Seek to next byte

  // Ignoring the flags for now.
  (void)majorVersion;
  (void)revisionNumber;
  (void)unsync;
  (void)extendedHeader;
  (void)experimental;

  uint32 size = ReadBits(mDataStream, mBitOffset, 32, Little);
  // Skip remaining ID3 header data (assuming no extended header), seek directly to first MP3 frame.
  mBitOffset += (size * 8);
  
  uint32 sync = ReadBits(mDataStream, mBitOffset, 12, Little);
  mBitOffset -= 12;
  return (sync & 0xFFF) == 0xFFF;
}

MP3::FrameHeader MP3::ReadFrameHeader() {
  // Frame header is 4 bytes
  
  // First 12 bits should be "frame sync"
  uint32 sync = ReadBits(mDataStream, mBitOffset, 12, Little);
  if ((sync & 0xFFF) != 0xFFF) {
    FrameHeader emptyHeader;
    return emptyHeader;
  }

  uint32 mpegVersion = ReadBits(mDataStream, mBitOffset, 1, Little);
  uint32 layer = ReadBits(mDataStream, mBitOffset, 2, Little);
  uint32 usesCrc = ReadBits(mDataStream, mBitOffset, 1, Little);
  uint32 bitrate = ReadBits(mDataStream, mBitOffset, 4, Little);
  uint32 frequency = ReadBits(mDataStream, mBitOffset, 2, Little);
  uint32 padding = ReadBits(mDataStream, mBitOffset, 1, Little);
  uint32 privateBit = ReadBits(mDataStream, mBitOffset, 1, Little);

  (void)mpegVersion;
  (void)layer;
  (void)padding;
  (void)privateBit;

  uint32 mode = ReadBits(mDataStream, mBitOffset, 2, Little);
  uint32 modeExt = ReadBits(mDataStream, mBitOffset, 2, Little);
  uint32 copy = ReadBits(mDataStream, mBitOffset, 1, Little);
  uint32 original = ReadBits(mDataStream, mBitOffset, 1, Little);
  uint32 emphasis = ReadBits(mDataStream, mBitOffset, 2, Little);

  (void)modeExt;
  (void)copy;
  (void)original;
  (void)emphasis;

  ChannelMode channelMode = ChannelFromMode(mode);
  ChannelModeExt channelModeExt = ChannelExtFromModeExt(modeExt);

  // Must do floating point arithmetic here because sample rate can return a rational number.
  // The fractional part is dropped (floored) when we convert back from float to int.
  size_t bitrateFromFlags = BitrateFromFlagsL3(bitrate);
  double bitrateInFrame = (double)bitrateFromFlags;
  double sampleRate = SampleRateFromFrequencyL3(frequency);
  size_t frameLengthBytes = (size_t)(144. * (bitrateInFrame / sampleRate));
  if (padding > 0) {
    ++frameLengthBytes;
  }

  // Can't find any official info about CRC size but it seems to be 16 bits, so skip two bytes if it's set.
  if (usesCrc == 0) {
    mBitOffset += (2 * 8);
  }

  FrameHeader header;
  header.mode = channelMode;
  header.modeExt = channelModeExt;
  header.bitrate = (uint32)bitrateFromFlags;
  header.sampleRate = sampleRate;
  header.sampleRateBits = frequency;
  header.frameBytes = frameLengthBytes;
  return header;
}

MP3::SideInfomation MP3::ReadSideInformation(uint32 sampleRateBits, ChannelMode mode, ChannelModeExt ext) {
  /*
  [main_data_begin, private_bits, scale_factor_selection, [GR1], [GR2]]

  This is organized in a confusing way. Side info only appears once.
  Side info has two forms, one for mono (single channel) and another for stereo (dual channel)

  In mono mode, GR1 & GR2 can be viewed as two instances of a granule block.
  This means in stereo we actually have 2 * 2 instances = 4 granules.
  Each granule is read in sequence. So 2 iterations for mono, 4 for stereo.
  The granule blocks begin immediately after the scalefactor select (which can vary in bits depending on mono/stereo).

  Granule order is GR1, GR2. Each granule has it's own channels.
  Mono: GR1 (single channel), GR2 (single channel)
  Stereo: GR1 (left channel) (right channel), GR2 (left channel) (right channel)
  */

  SideInfomation sideInfo;
  size_t numGranules = 2;
  Granule* granules[4] = {};

  uint32 mainData = ReadBits(mDataStream, mBitOffset, 9, Little);
  uint32 scaleFactorSelect = 0;

  bool isMono = false;

  if (mode == ChannelMode::SingleChannel || ext == ChannelModeExt::IntensityStereo) {
    // Mono = 17 bytes
    scaleFactorSelect = ReadBits(mDataStream, mBitOffset, 9, Little);
    sideInfo.byteLength = 17;
    granules[0] = sideInfo.gr0;
    granules[1] = sideInfo.gr1;
    isMono = true;
  }
  else {
    // Stereo = 32 bytes
    scaleFactorSelect = ReadBits(mDataStream, mBitOffset, 11, Little);
    numGranules += 2;
    sideInfo.byteLength = 32;
    sideInfo.numChannels = 4;
    granules[0] = sideInfo.gr0;
    granules[1] = sideInfo.gr0 + 1;
    granules[2] = sideInfo.gr1;
    granules[3] = sideInfo.gr1 + 1;
  }

  sideInfo.mainData = mainData;

  uint32 sampleRateIndex = 6 + sampleRateBits;
  if (sampleRateIndex > 0) {
    --sampleRateIndex;
  }

  for (size_t i = 0; i < numGranules; ++i) {
    if (isMono) {
      scaleFactorSelect <<= 4;
    }

    uint32 part2_3_length = ReadBits(mDataStream, mBitOffset, 12, Little);
    uint32 bigValues = ReadBits(mDataStream, mBitOffset, 9, Little);
    uint32 globalGain = ReadBits(mDataStream, mBitOffset, 8, Little);
    uint32 scaleCompress = ReadBits(mDataStream, mBitOffset, 4, Little);
    uint32 windowSwitch = ReadBits(mDataStream, mBitOffset, 1, Little);

    granules[i]->part2_3_length = part2_3_length;
    granules[i]->bigValues = bigValues;
    granules[i]->globalGain = globalGain;
    granules[i]->scaleFactorCompress = scaleCompress;
    granules[i]->scaleFactorTable = LongScaleFactors[sampleRateIndex];
    granules[i]->shortScaleBand = 0;
    granules[i]->longScaleBand = 22;
    granules[i]->windowSwitch = windowSwitch == 1;

    uint32 tableSelect = 0;
    if (windowSwitch == 1) {
      uint32 blockType = ReadBits(mDataStream, mBitOffset, 2, Little);
      uint32 mixedBlock = ReadBits(mDataStream, mBitOffset, 1, Little);
      tableSelect = ReadBits(mDataStream, mBitOffset, 10, Little);
      tableSelect <<= 5;
      granules[i]->regions[0] = 7;
      granules[i]->regions[1] = 255;
      granules[i]->subBlockGain[0] = ReadBits(mDataStream, mBitOffset, 3, Little);
      granules[i]->subBlockGain[1] = ReadBits(mDataStream, mBitOffset, 3, Little);
      granules[i]->subBlockGain[2] = ReadBits(mDataStream, mBitOffset, 3, Little);

      switch (blockType) {
        case 0x01:
          granules[i]->blockType = SideBlockType::StartBlock;
          break;
        case 0x02:
          granules[i]->blockType = SideBlockType::ShortWindows;
          break;
        case 0x03:
          granules[i]->blockType = SideBlockType::End;
          break;
        default:
          granules[i]->blockType = SideBlockType::Undefined;
          break;
      }

      granules[i]->mixedBlocks = mixedBlock == 1;

      if (granules[i]->blockType == SideBlockType::ShortWindows) {
        scaleFactorSelect &= 0x0F0F;

        if (granules[i]->mixedBlocks) {
          granules[i]->shortScaleBand = 30;
          granules[i]->longScaleBand = 8;
          granules[i]->scaleFactorTable = MixedScaleFactors[sampleRateIndex];
        }
        else {
          granules[i]->regions[0] = 8;
          granules[i]->shortScaleBand = 39;
          granules[i]->longScaleBand = 0;
          granules[i]->scaleFactorTable = ShortScaleFactors[sampleRateIndex];
        }
      }
    }
    else {
      tableSelect = ReadBits(mDataStream, mBitOffset, 15, Little);
      granules[i]->regions[0] = ReadBits(mDataStream, mBitOffset, 4, Little);
      granules[i]->regions[1] = ReadBits(mDataStream, mBitOffset, 3, Little);
      granules[i]->regions[2] = 255;
    }

    granules[i]->huffmanTables[0] = tableSelect >> 10;
    granules[i]->huffmanTables[1] = (tableSelect >> 5) & 31;
    granules[i]->huffmanTables[2] = tableSelect & 31;
    granules[i]->preflag = ReadBits(mDataStream, mBitOffset, 1, Little);
    granules[i]->scalefactorScale = ReadBits(mDataStream, mBitOffset, 1, Little);
    granules[i]->countTableSelect = ReadBits(mDataStream, mBitOffset, 1, Little);
    granules[i]->scaleFactorSelect = (scaleFactorSelect >> 12) & 15;
    scaleFactorSelect <<= 4;
  }

  return sideInfo;
}

void MP3::DecodeMainData(uint8* resevoir, size_t& resevoirBitsRead, SideInfomation& sideInfo, ChannelModeExt ext, size_t sampleRateBits, float* intermediateValues, float* overlapValues, int16* outData, float* qmfState) {
  Granule* granules[] = {sideInfo.gr0, sideInfo.gr1};
  uint8 sharedScaleFactors[2][39] = {};
  float scaleFactors[40] = {};
  const size_t numChannelsForGranule = sideInfo.numChannels / 2;

  for (size_t i = 0; i < 2; ++i) {
    Granule* granule = granules[i];
    memset(intermediateValues, 0, 2 * 576 * sizeof(float));

    float synthesis[2112] = {};
    int16* currOutData = outData + (i * (576 * numChannelsForGranule));

    for (size_t channel = 0; channel < numChannelsForGranule; ++channel) {
      // Each granule can have 1-2 channels.
      Granule* currentGranule = granule + channel;
      size_t frameBitEnd = resevoirBitsRead + currentGranule->part2_3_length;
      DecodeScaleFactors(resevoir, resevoirBitsRead, *currentGranule, currentGranule->scaleFactorSelect, ext == ChannelModeExt::MidStereo, scaleFactors, sharedScaleFactors[channel]);
      DecodeHuffman(resevoir, resevoirBitsRead, *currentGranule, scaleFactors, intermediateValues + (channel * 576), frameBitEnd);
    }

    if (ext == ChannelModeExt::IntensityStereo) {
      IntensityStereoProcess(intermediateValues, sharedScaleFactors[1], *granule);
    }
    else if (ext == ChannelModeExt::MidStereo) {
      MidStereoProcess(intermediateValues, 576);
    }
    
    for (size_t channel = 0; channel < numChannelsForGranule; ++channel) {
      Granule* currentGranule = granule + channel;

      uint32 sampleRateIndex = (uint32)(6 + sampleRateBits);
      if (sampleRateIndex > 0) {
        --sampleRateIndex;
      }

      int32 antiAliasBands = 31;
      int32 longBands = (currentGranule->mixedBlocks) ? 2 : 0;
      if (sampleRateIndex == 2) {
        longBands <<= 1;
      }

      float* granuleBuffer = (intermediateValues + (channel * 576));

      if (currentGranule->shortScaleBand > 0) {
        antiAliasBands = longBands - 1;
        Reorder(granuleBuffer + (longBands * 18),
          synthesis,
          currentGranule->scaleFactorTable + currentGranule->longScaleBand);
      }

      float* overlap = (overlapValues + (channel * 288));
      AntiAlias(granuleBuffer, antiAliasBands);
      InverseMDCTGranule(granuleBuffer, overlap, currentGranule->blockType, longBands);
      FlipSigns(granuleBuffer);
    }

    const int32 bands = 18;
    for (size_t channel = 0; channel < numChannelsForGranule; ++channel) {
      DCT2(intermediateValues + 576 * channel, bands);
    }

    memcpy(synthesis, qmfState, 960 * sizeof(float));

    for (size_t j = 0; j < bands; j += 2) {
      Synthesize(intermediateValues + j, currOutData + (32 * numChannelsForGranule * j), numChannelsForGranule, synthesis + j * 64);
    }

    memcpy(qmfState, synthesis + (bands * 64), 960 * sizeof(float));
  }
}

void MP3::DecodeMainDataFloat(uint8* resevoir, size_t& resevoirBitsRead, SideInfomation& sideInfo, ChannelModeExt ext, size_t sampleRateBits, float* intermediateValues, float* overlapValues, float* outData, float* qmfState) {
  Granule* granules[] = { sideInfo.gr0, sideInfo.gr1 };
  uint8 sharedScaleFactors[2][39] = {};
  float scaleFactors[40] = {};
  const size_t numChannelsForGranule = sideInfo.numChannels / 2;

  for (size_t i = 0; i < 2; ++i) {
    Granule* granule = granules[i];
    memset(intermediateValues, 0, 2 * 576 * sizeof(float));

    float synthesis[2112] = {};
    float* currOutData = outData + (i * (576 * numChannelsForGranule));

    for (size_t channel = 0; channel < numChannelsForGranule; ++channel) {
      // Each granule can have 1-2 channels.
      Granule* currentGranule = granule + channel;
      size_t frameBitEnd = resevoirBitsRead + currentGranule->part2_3_length;
      DecodeScaleFactors(resevoir, resevoirBitsRead, *currentGranule, currentGranule->scaleFactorSelect, ext == ChannelModeExt::MidStereo, scaleFactors, sharedScaleFactors[channel]);
      DecodeHuffman(resevoir, resevoirBitsRead, *currentGranule, scaleFactors, intermediateValues + (channel * 576), frameBitEnd);
    }

    if (ext == ChannelModeExt::IntensityStereo) {
      IntensityStereoProcess(intermediateValues, sharedScaleFactors[1], *granule);
    }
    else if (ext == ChannelModeExt::MidStereo) {
      MidStereoProcess(intermediateValues, 576);
    }

    for (size_t channel = 0; channel < numChannelsForGranule; ++channel) {
      Granule* currentGranule = granule + channel;

      uint32 sampleRateIndex = (uint32)(6 + sampleRateBits);
      if (sampleRateIndex > 0) {
        --sampleRateIndex;
      }

      int32 antiAliasBands = 31;
      int32 longBands = (currentGranule->mixedBlocks) ? 2 : 0;
      if (sampleRateIndex == 2) {
        longBands <<= 1;
      }

      float* granuleBuffer = (intermediateValues + (channel * 576));

      if (currentGranule->shortScaleBand > 0) {
        antiAliasBands = longBands - 1;
        Reorder(granuleBuffer + (longBands * 18),
          synthesis,
          currentGranule->scaleFactorTable + currentGranule->longScaleBand);
      }

      float* overlap = (overlapValues + (channel * 288));
      AntiAlias(granuleBuffer, antiAliasBands);
      InverseMDCTGranule(granuleBuffer, overlap, currentGranule->blockType, longBands);
      FlipSigns(granuleBuffer);
    }

    const int32 bands = 18;
    for (size_t channel = 0; channel < numChannelsForGranule; ++channel) {
      DCT2(intermediateValues + 576 * channel, bands);
    }

    memcpy(synthesis, qmfState, 960 * sizeof(float));

    for (size_t j = 0; j < bands; j += 2) {
      SynthesizeFloat(intermediateValues + j, currOutData + (32 * numChannelsForGranule * j), numChannelsForGranule, synthesis + j * 64);
    }

    memcpy(qmfState, synthesis + (bands * 64), 960 * sizeof(float));
  }
}

void MP3::DecodeScaleFactors(uint8* resevoir, size_t& resevoirBitsRead, Granule& granule, size_t scaleFactorSelect, bool isMidStereo, float* scaleFactors, uint8* sharedScaleFactors) {
  const uint8* scaleFactorPartition = ScaleFactorPartitions[!!granule.shortScaleBand + !granule.longScaleBand];
  uint8 scaleFactorSize[4] = {0,0,0,0};
  uint8 scaleFactorBuffer[40] = {};

  static const uint8 lut[16] = { 0,1,2,3, 12,5,6,7, 9,10,11,13, 14,15,18,19 };

  int32 part = (int32)lut[granule.scaleFactorCompress];
  scaleFactorSize[1] = scaleFactorSize[0] = (uint8)(part >> 2);
  scaleFactorSize[3] = scaleFactorSize[2] = (uint8)(part & 0x3);

  // Read scale factors
  uint32 selectedScaleFactor = (uint32)scaleFactorSelect;

  uint8* scfBuffIndex = scaleFactorBuffer;
  uint8* sharedScfIndex = sharedScaleFactors;
  for (size_t i = 0; i < 4 && scaleFactorPartition[i]; ++i, selectedScaleFactor *= 2) {
    uint8 count = scaleFactorPartition[i];
    if (selectedScaleFactor & 0x8) {
      memcpy(scfBuffIndex, sharedScfIndex, count);
    }
    else {
      uint8 scaleFactorBits = scaleFactorSize[i];
      if (scaleFactorBits == 0) {
        memset(scfBuffIndex, 0, count);
        memset(sharedScfIndex, 0, count);
      }
      else {
        for (uint8 k = 0; k < count; ++k) {
          uint32 scalefactor = ReadBits(resevoir, resevoirBitsRead, scaleFactorBits, Little);
          sharedScfIndex[k] = (uint8)scalefactor;
          scfBuffIndex[k] = (uint8)scalefactor;
        }
      }
    }

    scfBuffIndex += count;
    sharedScfIndex += count;
  }

  scfBuffIndex[0] = 0;
  scfBuffIndex[1] = 0;
  scfBuffIndex[2] = 0;

  int32 scaleFactorShift = granule.scalefactorScale + 1;
  if (granule.shortScaleBand > 0) {
    int32 shift = 3 - scaleFactorShift;
    for (size_t i = 0; i < granule.shortScaleBand; i += 3) {
      scaleFactorBuffer[granule.longScaleBand + i + 0] += (uint8)(granule.subBlockGain[0] << shift);
      scaleFactorBuffer[granule.longScaleBand + i + 1] += (uint8)(granule.subBlockGain[1] << shift);
      scaleFactorBuffer[granule.longScaleBand + i + 2] += (uint8)(granule.subBlockGain[2] << shift);
    }
  }
  else if(granule.preflag > 0) {
    const uint8 preAmp[10] = { 1,1,1,1,2,2,3,3,3,2 };
    for (size_t i = 0; i < 10; ++i) {
      scaleFactorBuffer[11 + i] += preAmp[i];
    }
  }

  int32 gainExponent = (int32)granule.globalGain;
  gainExponent -= 214;
  if (isMidStereo) {
    gainExponent -= 2;
  }

  float gain = DecodeExponent(2048, 44 - gainExponent);
  for (size_t i = 0; i < granule.shortScaleBand + granule.longScaleBand; ++i) {
    scaleFactors[i] = DecodeExponent(gain, scaleFactorBuffer[i] << scaleFactorShift);
  }
}

void MP3::DecodeHuffman(uint8* resevoir, size_t& resevoirBitsRead, Granule& granule, float* scaleFactors, float* intermediateValues, size_t frameEnd) {
  int32 bigValueCount = (int32)granule.bigValues;
  size_t tableIndex = 0;
  const uint8* scaleFactorTable = granule.scaleFactorTable;
  float one = 0.f;
  float* currScaleFactor = scaleFactors;
  const uint8* resevoirOffset = resevoir + (resevoirBitsRead / 8);
  uint32 cachedStream = (((resevoirOffset[0] * 256U + resevoirOffset[1]) * 256U + resevoirOffset[2]) * 256U + resevoirOffset[3]) << (resevoirBitsRead & 0x7);
  int32 bitPosition = (resevoirBitsRead & 0x7) - 8;
  resevoirOffset += 4;

  while (bigValueCount > 0) {
    int32 tableNumber = (int32)granule.huffmanTables[tableIndex];
    int32 scaleFactorBandNumber = (int32)granule.regions[tableIndex];
    ++tableIndex;

    const int16* huffman = HuffmanTables + HuffmanTableIndex[tableNumber];
    uint32 lineBits = LineBits[tableNumber];
    if (lineBits > 0) {
      int32 numPairs = 0;
      for (bool decodingBigValues = true; decodingBigValues; decodingBigValues = (((bigValueCount -= numPairs) > 0) && ((--scaleFactorBandNumber) >= 0))) {
        numPairs = (*scaleFactorTable) / 2;
        ++scaleFactorTable;
        int32 pairToDecode = Min(bigValueCount, numPairs);
        one = *currScaleFactor;
        ++currScaleFactor;

        for (bool decodingPairs = true; decodingPairs; decodingPairs = ((--pairToDecode) > 0)) {
          int32 w = 5;
          int32 leaf = huffman[cachedStream >> (32 - w)];
          while (leaf < 0) {
            cachedStream <<= w;
            bitPosition += w;
            w = leaf & 0x7;
            leaf = huffman[(cachedStream >> (32 - w)) - (leaf >> 3)];
          }

          cachedStream <<= (leaf >> 8);
          bitPosition += (leaf >> 8);

          for (size_t j = 0; j < 2; ++j, ++intermediateValues, leaf >>= 4) {
            int32 lsb = leaf & 0xF;
            if (lsb == 0xF) {
              lsb += (cachedStream >> (32 - lineBits));
              cachedStream <<= lineBits;
              bitPosition += lineBits;

              while (bitPosition >= 0) {
                cachedStream |= ((uint32)(*resevoirOffset) << bitPosition);
                ++resevoirOffset;
                bitPosition -= 8;
              }

              (*intermediateValues) = one * Power43(lsb);
              if (((int32)cachedStream) < 0) {
                (*intermediateValues) = -(*intermediateValues);
              }
            }
            else {
              (*intermediateValues) = Power43Table[16 + lsb - (16 * (cachedStream >> 31))] * one;
            }

            if (lsb != 0) {
              cachedStream <<= 1;
              bitPosition += 1;
            }
          }

          while (bitPosition >= 0) {
            cachedStream |= ((uint32)(*resevoirOffset) << bitPosition);
            ++resevoirOffset;
            bitPosition -= 8;
          }
        }
      }
    }
    else {
      int32 numPairs = 0;
      for (bool decodingBigValues = true; decodingBigValues; decodingBigValues = (((bigValueCount -= numPairs) > 0) && ((--scaleFactorBandNumber) >= 0))) {
        numPairs = (*scaleFactorTable) / 2;
        ++scaleFactorTable;
        int32 pairToDecode = Min(bigValueCount, numPairs);
        one = *currScaleFactor;
        ++currScaleFactor;

        for (bool decodingPairs = true; decodingPairs; decodingPairs = ((--pairToDecode) > 0)) {
          int32 w = 5;
          int32 leaf = huffman[(cachedStream >> (32 - w))];
          while (leaf < 0) {
            cachedStream <<= w;
            bitPosition += w;
            w = leaf & 0x7;
            leaf = huffman[(cachedStream >> (32 - w)) - (leaf >> 3)];
          }

          cachedStream <<= (leaf >> 8);
          bitPosition += (leaf >> 8);

          for (size_t j = 0; j < 2; ++j, ++intermediateValues, leaf >>= 4) {
            int32 lsb = leaf & 0xF;
            (*intermediateValues) = Power43Table[16 + lsb - (16 * (cachedStream >> 31))] * one;

            if (lsb != 0) {
              cachedStream <<= 1;
              bitPosition += 1;
            }
          }

          while (bitPosition >= 0) {
            cachedStream |= ((uint32)(*resevoirOffset) << bitPosition);
            ++resevoirOffset;
            bitPosition -= 8;
          }
        }
      }
    }
  }

  for (int32 numPairs = 1 - bigValueCount;; intermediateValues += 4) {
    const uint8* huffman = (granule.countTableSelect) ? HuffmanTable33 : HuffmanTable32;
    int32 leaf = huffman[(cachedStream >> (32 - 4))];
    if (!(leaf & 0x8)) {
      leaf = huffman[(leaf >> 3) + (cachedStream << 4 >> (32 - (leaf & 0x3)))];
    }

    cachedStream <<= (leaf & 0x7);
    bitPosition += (leaf & 0x7);

    if ((((resevoirOffset - resevoir) * 8) - 24 + bitPosition) > (int32)frameEnd) {
      break;
    }

    for (size_t i = 0; i < 4; i += 2) {
      if (!(--numPairs)) {
        numPairs = (*scaleFactorTable) / 2;
        ++scaleFactorTable;

        if (!numPairs) {
          break;
        }
        else {
          one = *currScaleFactor;
          ++currScaleFactor;
        }
      }

      if (leaf & (128 >> i)) {
        if (((int32)cachedStream) < 0) {
          intermediateValues[i] = -one;
        }
        else {
          intermediateValues[i] = one;
        }

        cachedStream <<= 1;
        bitPosition += 1;
      }

      if (leaf & (128 >> (i + 1))) {
        if (((int32)cachedStream) < 0) {
          intermediateValues[i + 1] = -one;
        }
        else {
          intermediateValues[i + 1] = one;
        }

        cachedStream <<= 1;
        bitPosition += 1;
      }
    }

    while (bitPosition >= 0) {
      cachedStream |= ((uint32)(*resevoirOffset) << bitPosition);
      ++resevoirOffset;
      bitPosition -= 8;
    }
  }

  resevoirBitsRead = frameEnd;
}

float MP3::DecodeExponent(float y, int32 exponent) {
  const float exponentFractions[4] = { 9.31322575e-10f,7.83145814e-10f,6.58544508e-10f,5.53767716e-10f };
  int32 e;
  int32 lMin = 30 * 4;
  do
  {
    e = Min(lMin, exponent);
    y *= exponentFractions[e & 3] * (1 << 30 >> (e >> 2));
  } 
  while ((exponent -= e) > 0);

  return y;
}

MP3::ChannelMode MP3::ChannelFromMode(size_t flags) {
  switch (flags) {
    case 0x00:
      return ChannelMode::Stereo;
    case 0x01:
      return ChannelMode::JointStero;
    case 0x02:
      return ChannelMode::DualChannel;
    case 0x03:
      return ChannelMode::SingleChannel;
    default:
      return ChannelMode::Undefined;
  }
}

MP3::ChannelModeExt MP3::ChannelExtFromModeExt(size_t flags) {
  switch (flags) {
    case 0x00:
      return ChannelModeExt::Undefined;
    case 0x01:
      return ChannelModeExt::IntensityStereo;
    case 0x02:
      return ChannelModeExt::MidStereo;
    case 0x03:
      return ChannelModeExt::Both;
    default:
      return ChannelModeExt::Undefined;
  }
}

size_t MP3::BitrateFromFlagsL3(size_t flags) {
  switch (flags) {
    case 0x01:
      return 32;
    case 0x02:
      return 40;
    case 0x03:
      return 48;
    case 0x04:
      return 56;
    case 0x05:
      return 64;
    case 0x06:
      return 80;
    case 0x07:
      return 96;
    case 0x08:
      return 112;
    case 0x09:
      return 128;
    case 0x0A:
      return 160;
    case 0x0B:
      return 192;
    case 0x0C:
      return 224;
    case 0x0D:
      return 256;
    case 0x0E:
      return 320;
    default:
      return 0;
  }
}

double MP3::SampleRateFromFrequencyL3(size_t frequency) {
  switch (frequency) {
    case 0x00:
      return 44.1;
    case 0x01:
      return 48.;
    case 0x02:
      return 32.;
    default:
      return 0.;
  }
}

float MP3::Power43(int32 x) {
  float fraction = 0.f;
  int32 sign = 0;
  int32 multiple = 256;

  if (x < 129) {
    return Power43Table[16 + x];
  }

  if (x < 1024) {
    multiple = 16;
    x <<= 3;
  }

  sign = (2 * x) & 64;
  fraction = ((float)((x & 63) - sign)) / (float)(((x & ~63) + sign));

  const float powerIndex = Power43Table[16 + ((x + sign) >> 6)];
  const float fractionalValue = 1.f + (fraction * ((4.f / 3.f) + (fraction * (2.f / 9.f))));

  return powerIndex * fractionalValue * multiple;
}

void MP3::MidStereoProcess(float* left, size_t length) {
  float* right = left + 576;
  for (int32 i = 0; i < length; ++i) {
    float a = left[i];
    float b = right[i];
    left[i] = a + b;
    right[i] = a - b;
  }
}

void MP3::IntensityStereoProcess(float* left, uint8* sharedScaleFactors, Granule& granule) {
  int32 max_band[3] = {};
  int32 n_sfb = (int32)granule.longScaleBand + (int32)granule.shortScaleBand;
  int32 max_blocks = granule.shortScaleBand ? 3 : 1;

  StereoTopBand(left + 576, granule.scaleFactorTable, n_sfb, max_band);
  if (granule.longScaleBand) {
    max_band[0] = max_band[1] = max_band[2] = Max(Max(max_band[0], max_band[1]), max_band[2]);
  }

  for (int32 i = 0; i < max_blocks; ++i) {
    uint8 default_pos = 3;
    int32 itop = n_sfb - max_blocks + i;
    int32 prev = itop - max_blocks;
    sharedScaleFactors[itop] = max_band[i] >= prev ? default_pos : sharedScaleFactors[prev];
  }

  StereoProcess(left, sharedScaleFactors, granule.scaleFactorTable, max_band);
}

void MP3::StereoTopBand(const float* right, const uint8* sharedScaleFactors, int32 numBands, int* maxBands) {
  maxBands[0] = maxBands[1] = maxBands[2] = -1;

  for (int32 i = 0; i < numBands; ++i) {
    for (int32 k = 0; k < sharedScaleFactors[i]; k += 2) {
      if (right[k] != 0 || right[k + 1] != 0) {
        maxBands[i % 3] = i;
        break;
      }
    }

    right += sharedScaleFactors[i];
  }
}

void MP3::StereoProcess(float* left, const uint8* sharedScaleFactors, const uint8* scaleFactorBands, int* maxBands) {
  static const float lut[7 * 2] = { 
    0,1,0.21132487f,0.78867513f,0.36602540f,0.63397460f,0.5f,0.5f,0.63397460f,0.36602540f,0.78867513f,0.21132487f,1,0 
  };

  const uint32 max_pos = 7;
  for (int32 i = 0; scaleFactorBands[i]; ++i) {
    uint32 ipos = sharedScaleFactors[i];
    if (i > maxBands[i % 3] && ipos < max_pos) {
      float kl;
      float kr;
      float s = 1.f;

      kl = lut[2 * ipos];
      kr = lut[2 * ipos + 1];
      
      for (int32 k = 0; k < scaleFactorBands[i]; ++k) {
        left[i + 576] = left[i] * (kr * s);
        left[i] = left[i] * (kl * s);
      }
    }

    left += scaleFactorBands[i];
  }
}

void MP3::Reorder(float* buffer, float* scratch, const uint8* scalefactors) {
  int32 len = 0;
  float* src = buffer;
  float* dst = scratch;

  for (; 0 != (len = (int32)(*scalefactors)); scalefactors += 3, src += (2 * len)) {
    for (size_t i = 0; i < len; ++i, ++src) {
      *dst++ = src[0 * len];
      *dst++ = src[1 * len];
      *dst++ = src[2 * len];
    }
  }

  memcpy(buffer, scratch, (dst - scratch) * sizeof(float));
}

void MP3::AntiAlias(float* buffer, int32 bands) {
  static const float lut[2][8] = {
    {0.85749293f,0.88174200f,0.94962865f,0.98331459f,0.99551782f,0.99916056f,0.99989920f,0.99999316f},
    {0.51449576f,0.47173197f,0.31337745f,0.18191320f,0.09457419f,0.04096558f,0.01419856f,0.00369997f}
  };

  for (; bands > 0; --bands, buffer += 18) {
    for (size_t i = 0; i < 8; ++i) {
      float u = buffer[18 + i];
      float d = buffer[17 - i];
      buffer[18 + i] = u * lut[0][i] - d * lut[1][i];
      buffer[17 - i] = u * lut[1][i] + d * lut[0][i];
    }
  }
}

void MP3::InverseMDCTGranule(float* buffer, float* overlap, SideBlockType blockType, int32 bands) {
  static const float lut[2][18] = {
    { 0.99904822f,0.99144486f,0.97629601f,0.95371695f,0.92387953f,0.88701083f,0.84339145f,0.79335334f,0.73727734f,0.04361938f,0.13052619f,0.21643961f,0.30070580f,0.38268343f,0.46174861f,0.53729961f,0.60876143f,0.67559021f },
    { 1,1,1,1,1,1,0.99144486f,0.92387953f,0.79335334f,0,0,0,0,0,0,0.13052619f,0.38268343f,0.60876143f }
  };

  if (bands > 0) {
    InverseMDCT36(buffer, overlap, lut[0], bands);
    buffer += 18 * bands;
    overlap += 9 * bands;
  }

  if (blockType == SideBlockType::ShortWindows) {
    InverseMDCTShort(buffer, overlap, 32 - bands);
  }
  else {
    size_t blockIndex = (blockType == SideBlockType::End) ? 1 : 0;
    InverseMDCT36(buffer, overlap, lut[blockIndex], 32 - bands);
  }
}

void MP3::InverseMDCT36(float* buffer, float* overlap, const float* window, int32 bands) {
  static const float lut[18] = {
    0.73727734f,0.79335334f,0.84339145f,0.88701083f,0.92387953f,0.95371695f,0.97629601f,0.99144486f,0.99904822f,0.67559021f,0.60876143f,0.53729961f,0.46174861f,0.38268343f,0.30070580f,0.21643961f,0.13052619f,0.04361938f
  };

  for (int32 j = 0; j < bands; j++, buffer += 18, overlap += 9) {
    float cos[9] = {};
    float sin[9] = {};

    cos[0] = -buffer[0];
    sin[0] = buffer[17];

    for (int32 i = 0; i < 4; ++i) {
      sin[8 - 2 * i] = buffer[4 * i + 1] - buffer[4 * i + 2];
      cos[1 + 2 * i] = buffer[4 * i + 1] + buffer[4 * i + 2];
      sin[7 - 2 * i] = buffer[4 * i + 4] - buffer[4 * i + 3];
      cos[2 + 2 * i] = -(buffer[4 * i + 3] + buffer[4 * i + 4]);
    }

    DCT39(cos);
    DCT39(sin);

    sin[1] = -sin[1];
    sin[3] = -sin[3];
    sin[5] = -sin[5];
    sin[7] = -sin[7];

    for (int32 i = 0; i < 9; ++i) {
      float ovl = overlap[i];
      float sum = cos[i] * lut[9 + i] + sin[i] * lut[0 + i];
      overlap[i] = cos[i] * lut[0 + i] - sin[i] * lut[9 + i];
      buffer[i] = ovl * window[0 + i] - sum * window[9 + i];
      buffer[17 - i] = ovl * window[9 + i] + sum * window[0 + i];
    }
  }
}

void MP3::InverseMDCTShort(float* buffer, float* overlap, int32 bands) {
  for (; bands > 0; --bands, overlap += 9, buffer += 18) {
    float tmp[18] = {};
    memcpy(tmp, buffer, sizeof(tmp));
    memcpy(buffer, overlap, 24);
    InverseMDCT12(tmp, buffer + 6, overlap + 6);
    InverseMDCT12(tmp + 1, buffer + 12, overlap + 6);
    InverseMDCT12(tmp + 2, overlap, overlap + 6);
  }
}

void MP3::InverseMDCT12(float* x, float* dest, float* overlap) {
  static const float lut[6] = { 0.79335334f,0.92387953f,0.99144486f, 0.60876143f,0.38268343f,0.13052619f };
  float cos[3] = {};
  float sin[3] = {};

  InverseMDCT3(-x[0], x[6] + x[3], x[12] + x[9], cos);
  InverseMDCT3(x[15], x[12] - x[9], x[6] - x[3], sin);
  sin[1] = -sin[1];

  for (int32 i = 0; i < 3; ++i) {
    float ovl = overlap[i];
    float sum = cos[i] * lut[3 + i] + sin[i] * lut[0 + i];
    overlap[i] = cos[i] * lut[0 + i] - sin[i] * lut[3 + i];
    dest[i] = ovl * lut[2 - i] - sum * lut[5 - i];
    dest[5 - i] = ovl * lut[5 - i] + sum * lut[2 - i];
  }
}

void MP3::InverseMDCT3(float x0, float x1, float x2, float* dest) {
  float m1 = x1 * 0.86602540f;
  float a1 = x0 - x2 * 0.5f;
  dest[0] = a1 + m1;
  dest[1] = x0 + x2;
  dest[2] = a1 - m1;
}

void MP3::DCT2(float* buffer, int32 n) {
  static const float lut[24] = {
    10.19000816f,0.50060302f,0.50241929f,3.40760851f,0.50547093f,0.52249861f,2.05778098f,0.51544732f,0.56694406f,1.48416460f,0.53104258f,0.64682180f,1.16943991f,0.55310392f,0.78815460f,0.97256821f,0.58293498f,1.06067765f,0.83934963f,0.62250412f,1.72244716f,0.74453628f,0.67480832f,5.10114861f
  };

  for (int32 k = 0; k < n; ++k) {
    float t[4][8] = {};
    float* y = buffer + k;

    int32 i = 0;
    for (float* x = t[0]; i < 8; ++i, ++x) {
      float x0 = y[i * 18];
      float x1 = y[(15 - i) * 18];
      float x2 = y[(16 + i) * 18];
      float x3 = y[(31 - i) * 18];
      float t0 = x0 + x3;
      float t1 = x1 + x2;
      float t2 = (x1 - x2) * lut[3 * i + 0];
      float t3 = (x0 - x3) * lut[3 * i + 1];
      x[0] = t0 + t1;
      x[8] = (t0 - t1) * lut[3 * i + 2];
      x[16] = t3 + t2;
      x[24] = (t3 - t2) * lut[3 * i + 2];
    }

    i = 0;
    for (float* x = t[0]; i < 4; ++i, x += 8) {
      float x0 = x[0];
      float x1 = x[1];
      float x2 = x[2];
      float x3 = x[3];
      float x4 = x[4];
      float x5 = x[5];
      float x6 = x[6];
      float x7 = x[7];
      float xt = x0 - x7;

      x0 += x7;
      x7 = x1 - x6; 
      x1 += x6;
      x6 = x2 - x5; 
      x2 += x5;
      x5 = x3 - x4;
      x3 += x4;
      x4 = x0 - x3; 
      x0 += x3;
      x3 = x1 - x2; 
      x1 += x2;
      x[0] = x0 + x1;
      x[4] = (x0 - x1) * 0.70710677f;
      x5 = x5 + x6;
      x6 = (x6 + x7) * 0.70710677f;
      x7 = x7 + xt;
      x3 = (x3 + x4) * 0.70710677f;
      x5 -= x7 * 0.198912367f;
      x7 += x5 * 0.382683432f;
      x5 -= x7 * 0.198912367f;
      x0 = xt - x6; xt += x6;
      x[1] = (xt + x7) * 0.50979561f;
      x[2] = (x4 + x3) * 0.54119611f;
      x[3] = (x0 - x5) * 0.60134488f;
      x[5] = (x0 + x5) * 0.89997619f;
      x[6] = (x4 - x3) * 1.30656302f;
      x[7] = (xt - x7) * 2.56291556f;
    }

    i = 0;
    for (; i < 7; ++i, y += 4 * 18) {
      y[0 * 18] = t[0][i];
      y[1 * 18] = t[2][i] + t[3][i] + t[3][i + 1];
      y[2 * 18] = t[1][i] + t[1][i + 1];
      y[3 * 18] = t[2][i + 1] + t[3][i] + t[3][i + 1];
    }

    y[0 * 18] = t[0][7];
    y[1 * 18] = t[2][7] + t[3][7];
    y[2 * 18] = t[1][7];
    y[3 * 18] = t[3][7];
  }
}

void MP3::DCT39(float* y) {
  float s0 = y[0];
  float s2 = y[2];
  float s4 = y[4];
  float s6 = y[6];
  float s8 = y[8];

  float t0 = s0 + s6 * 0.5f;

  s0 -= s6;

  float t4 = (s4 + s2) * 0.93969262f;
  float t2 = (s8 + s2) * 0.76604444f;
  s6 = (s4 - s8) * 0.17364818f;
  s4 += (s8 - s2);

  s2 = s0 - s4 * 0.5f;
  y[4] = s4 + s0;
  s8 = t0 - t2 + s6;
  s0 = t0 - t4 + t2;
  s4 = t0 + t4 - s6;

  float s1 = y[1];
  float s3 = y[3];
  float s5 = y[5];
  float s7 = y[7];

  s3 *= 0.86602540f;
  t0 = (s5 + s1) * 0.98480775f;
  t4 = (s5 - s7) * 0.34202014f;
  t2 = (s1 + s7) * 0.64278761f;
  s1 = (s1 - s5 - s7) * 0.86602540f;

  s5 = t0 - s3 - t2;
  s7 = t4 - s3 - t0;
  s3 = t4 + s3 - t2;

  y[0] = s4 - s7;
  y[1] = s2 + s1;
  y[2] = s0 - s3;
  y[3] = s8 + s5;
  y[5] = s8 - s5;
  y[6] = s0 + s3;
  y[7] = s2 - s1;
  y[8] = s4 + s7;
}

void MP3::FlipSigns(float* buffer) {
  buffer += 18;
  for (int32 b = 0; b < 32; b += 2, buffer += 36) {
    for (int32 i = 1; i < 18; i += 2) {
      buffer[i] = -buffer[i];
    }
  }
}

void MP3::SynthesizeGranule(float* buffer, int32 bands, size_t channels, int16* outData, float* lines) {
  for (int32 i = 0; i < channels; i++) {
    DCT2(buffer + 576 * i, bands);
  }

  for (int32 i = 0; i < bands; i += 2) {
    Synthesize(buffer + i, outData + (32 * channels * i), channels, lines + i * 64);
  }
}

void MP3::SynthesizeGranuleFloat(float* buffer, int32 bands, size_t channels, float* outData, float* lines) {
  for (int32 i = 0; i < channels; i++) {
    DCT2(buffer + 576 * i, bands);
  }

  for (int32 i = 0; i < bands; i += 2) {
    SynthesizeFloat(buffer + i, outData + (32 * channels * i), channels, lines + i * 64);
  }
}

void MP3::SynthesizePair(int16* outData, size_t channels, const float* z) {
  float a = (z[14 * 64] - z[0]) * 29;
  a += (z[1 * 64] + z[13 * 64]) * 213;
  a += (z[12 * 64] - z[2 * 64]) * 459;
  a += (z[3 * 64] + z[11 * 64]) * 2037;
  a += (z[10 * 64] - z[4 * 64]) * 5153;
  a += (z[5 * 64] + z[9 * 64]) * 6574;
  a += (z[8 * 64] - z[6 * 64]) * 37489;
  a += z[7 * 64] * 75038;
  outData[0] = ScalePCM(a);

  z += 2;
  a = z[14 * 64] * 104;
  a += z[12 * 64] * 1567;
  a += z[10 * 64] * 9727;
  a += z[8 * 64] * 64019;
  a += z[6 * 64] * -9975;
  a += z[4 * 64] * -45;
  a += z[2 * 64] * 146;
  a += z[0 * 64] * -5;
  outData[16 * channels] = ScalePCM(a);
}

void MP3::SynthesizePairFloat(float* outData, size_t channels, const float* z) {
  float a = (z[14 * 64] - z[0]) * 29;
  a += (z[1 * 64] + z[13 * 64]) * 213;
  a += (z[12 * 64] - z[2 * 64]) * 459;
  a += (z[3 * 64] + z[11 * 64]) * 2037;
  a += (z[10 * 64] - z[4 * 64]) * 5153;
  a += (z[5 * 64] + z[9 * 64]) * 6574;
  a += (z[8 * 64] - z[6 * 64]) * 37489;
  a += z[7 * 64] * 75038;
  outData[0] = ScalePCMFloat(a);

  z += 2;
  a = z[14 * 64] * 104;
  a += z[12 * 64] * 1567;
  a += z[10 * 64] * 9727;
  a += z[8 * 64] * 64019;
  a += z[6 * 64] * -9975;
  a += z[4 * 64] * -45;
  a += z[2 * 64] * 146;
  a += z[0 * 64] * -5;
  outData[16 * channels] = ScalePCMFloat(a);
}

void MP3::Synthesize(float* x1, int16* outData, size_t channels, float* lines) {
  float* xr = x1 + 576 * (channels - 1);
  int16* dstr = outData + (channels - 1);
  float* zlin = lines + 15 * 64;

  zlin[4 * 15] = x1[18 * 16];
  zlin[4 * 15 + 1] = xr[18 * 16];
  zlin[4 * 15 + 2] = x1[0];
  zlin[4 * 15 + 3] = xr[0];

  zlin[4 * 31] = x1[1 + 18 * 16];
  zlin[4 * 31 + 1] = xr[1 + 18 * 16];
  zlin[4 * 31 + 2] = x1[1];
  zlin[4 * 31 + 3] = xr[1];

  SynthesizePair(dstr, channels, lines + 4 * 15 + 1);
  SynthesizePair(dstr + 32 * channels, channels, lines + 4 * 15 + 64 + 1);
  SynthesizePair(outData, channels, lines + 4 * 15);
  SynthesizePair(outData + 32 * channels, channels, lines + 4 * 15 + 64);

  static const float lut[] = {
    -1,26,-31,208,218,401,-519,2063,2000,4788,-5517,7134,5959,35640,-39336,74992,
    -1,24,-35,202,222,347,-581,2080,1952,4425,-5879,7640,5288,33791,-41176,74856,
    -1,21,-38,196,225,294,-645,2087,1893,4063,-6237,8092,4561,31947,-43006,74630,
    -1,19,-41,190,227,244,-711,2085,1822,3705,-6589,8492,3776,30112,-44821,74313,
    -1,17,-45,183,228,197,-779,2075,1739,3351,-6935,8840,2935,28289,-46617,73908,
    -1,16,-49,176,228,153,-848,2057,1644,3004,-7271,9139,2037,26482,-48390,73415,
    -2,14,-53,169,227,111,-919,2032,1535,2663,-7597,9389,1082,24694,-50137,72835,
    -2,13,-58,161,224,72,-991,2001,1414,2330,-7910,9592,70,22929,-51853,72169,
    -2,11,-63,154,221,36,-1064,1962,1280,2006,-8209,9750,-998,21189,-53534,71420,
    -2,10,-68,147,215,2,-1137,1919,1131,1692,-8491,9863,-2122,19478,-55178,70590,
    -3,9,-73,139,208,-29,-1210,1870,970,1388,-8755,9935,-3300,17799,-56778,69679,
    -3,8,-79,132,200,-57,-1283,1817,794,1095,-8998,9966,-4533,16155,-58333,68692,
    -4,7,-85,125,189,-83,-1356,1759,605,814,-9219,9959,-5818,14548,-59838,67629,
    -4,7,-91,117,177,-106,-1428,1698,402,545,-9416,9916,-7154,12980,-61289,66494,
    -5,6,-97,111,163,-127,-1498,1634,185,288,-9585,9838,-8540,11455,-62684,65290
  };

  const float* w = lut;
  for (int32 i = 14; i >= 0; --i) {
    float a[4] = {};
    float b[4] = {};

    zlin[4 * i] = x1[18 * (31 - i)];
    zlin[4 * i + 1] = xr[18 * (31 - i)];
    zlin[4 * i + 2] = x1[1 + 18 * (31 - i)];
    zlin[4 * i + 3] = xr[1 + 18 * (31 - i)];
    zlin[4 * (i + 16)] = x1[1 + 18 * (1 + i)];
    zlin[4 * (i + 16) + 1] = xr[1 + 18 * (1 + i)];
    zlin[4 * (i - 16) + 2] = x1[18 * (1 + i)];
    zlin[4 * (i - 16) + 3] = xr[18 * (1 + i)];

    // S0(0), S2(1), S1(2), S2(3), S1(4), S2(5), S1(6), S2(7)
    size_t k = 0;
    { // S0(0)
      float w0 = *w++; 
      float w1 = *w++; 
      float* vz = &zlin[4 * i - k * 64]; 
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (int32 j = 0; j < 4; j++) {
        b[j] = vz[j] * w1 + vy[j] * w0;
        a[j] = vz[j] * w0 - vy[j] * w1;
      }
    }

    k = 1;
    { // S2(1)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) { 
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vy[j] * w1 - vz[j] * w0;
      }
    }

    k = 2;
    { // S1(2)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) { 
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vz[j] * w0 - vy[j] * w1;
      }
    }

    k = 3;
    { // S2(3)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vy[j] * w1 - vz[j] * w0;
      }
    }

    k = 4;
    { // S1(4)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vz[j] * w0 - vy[j] * w1;
      }
    }

    k = 5;
    { // S2(5)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vy[j] * w1 - vz[j] * w0;
      }
    }

    k = 6;
    { // S1(6)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vz[j] * w0 - vy[j] * w1;
      }
    }

    k = 7;
    { // S2(7)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vy[j] * w1 - vz[j] * w0;
      }
    }
    
    dstr[(15 - i) * channels] = ScalePCM(a[1]);
    dstr[(17 + i) * channels] = ScalePCM(b[1]);
    outData[(15 - i) * channels] = ScalePCM(a[0]);
    outData[(17 + i) * channels] = ScalePCM(b[0]);
    dstr[(47 - i) * channels] = ScalePCM(a[3]);
    dstr[(49 + i) * channels] = ScalePCM(b[3]);
    outData[(47 - i) * channels] = ScalePCM(a[2]);
    outData[(49 + i) * channels] = ScalePCM(b[2]);
  }
}

void MP3::SynthesizeFloat(float* x1, float* outData, size_t channels, float* lines) {
  float* xr = x1 + 576 * (channels - 1);
  float* dstr = outData + (channels - 1);
  float* zlin = lines + 15 * 64;

  zlin[4 * 15] = x1[18 * 16];
  zlin[4 * 15 + 1] = xr[18 * 16];
  zlin[4 * 15 + 2] = x1[0];
  zlin[4 * 15 + 3] = xr[0];

  zlin[4 * 31] = x1[1 + 18 * 16];
  zlin[4 * 31 + 1] = xr[1 + 18 * 16];
  zlin[4 * 31 + 2] = x1[1];
  zlin[4 * 31 + 3] = xr[1];

  SynthesizePairFloat(dstr, channels, lines + 4 * 15 + 1);
  SynthesizePairFloat(dstr + 32 * channels, channels, lines + 4 * 15 + 64 + 1);
  SynthesizePairFloat(outData, channels, lines + 4 * 15);
  SynthesizePairFloat(outData + 32 * channels, channels, lines + 4 * 15 + 64);

  static const float lut[] = {
    -1,26,-31,208,218,401,-519,2063,2000,4788,-5517,7134,5959,35640,-39336,74992,
    -1,24,-35,202,222,347,-581,2080,1952,4425,-5879,7640,5288,33791,-41176,74856,
    -1,21,-38,196,225,294,-645,2087,1893,4063,-6237,8092,4561,31947,-43006,74630,
    -1,19,-41,190,227,244,-711,2085,1822,3705,-6589,8492,3776,30112,-44821,74313,
    -1,17,-45,183,228,197,-779,2075,1739,3351,-6935,8840,2935,28289,-46617,73908,
    -1,16,-49,176,228,153,-848,2057,1644,3004,-7271,9139,2037,26482,-48390,73415,
    -2,14,-53,169,227,111,-919,2032,1535,2663,-7597,9389,1082,24694,-50137,72835,
    -2,13,-58,161,224,72,-991,2001,1414,2330,-7910,9592,70,22929,-51853,72169,
    -2,11,-63,154,221,36,-1064,1962,1280,2006,-8209,9750,-998,21189,-53534,71420,
    -2,10,-68,147,215,2,-1137,1919,1131,1692,-8491,9863,-2122,19478,-55178,70590,
    -3,9,-73,139,208,-29,-1210,1870,970,1388,-8755,9935,-3300,17799,-56778,69679,
    -3,8,-79,132,200,-57,-1283,1817,794,1095,-8998,9966,-4533,16155,-58333,68692,
    -4,7,-85,125,189,-83,-1356,1759,605,814,-9219,9959,-5818,14548,-59838,67629,
    -4,7,-91,117,177,-106,-1428,1698,402,545,-9416,9916,-7154,12980,-61289,66494,
    -5,6,-97,111,163,-127,-1498,1634,185,288,-9585,9838,-8540,11455,-62684,65290
  };

  const float* w = lut;
  for (int32 i = 14; i >= 0; --i) {
    float a[4] = {};
    float b[4] = {};

    zlin[4 * i] = x1[18 * (31 - i)];
    zlin[4 * i + 1] = xr[18 * (31 - i)];
    zlin[4 * i + 2] = x1[1 + 18 * (31 - i)];
    zlin[4 * i + 3] = xr[1 + 18 * (31 - i)];
    zlin[4 * (i + 16)] = x1[1 + 18 * (1 + i)];
    zlin[4 * (i + 16) + 1] = xr[1 + 18 * (1 + i)];
    zlin[4 * (i - 16) + 2] = x1[18 * (1 + i)];
    zlin[4 * (i - 16) + 3] = xr[18 * (1 + i)];

    // S0(0), S2(1), S1(2), S2(3), S1(4), S2(5), S1(6), S2(7)
    size_t k = 0;
    { // S0(0)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (int32 j = 0; j < 4; j++) {
        b[j] = vz[j] * w1 + vy[j] * w0;
        a[j] = vz[j] * w0 - vy[j] * w1;
      }
    }

    k = 1;
    { // S2(1)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vy[j] * w1 - vz[j] * w0;
      }
    }

    k = 2;
    { // S1(2)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vz[j] * w0 - vy[j] * w1;
      }
    }

    k = 3;
    { // S2(3)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vy[j] * w1 - vz[j] * w0;
      }
    }

    k = 4;
    { // S1(4)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vz[j] * w0 - vy[j] * w1;
      }
    }

    k = 5;
    { // S2(5)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vy[j] * w1 - vz[j] * w0;
      }
    }

    k = 6;
    { // S1(6)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vz[j] * w0 - vy[j] * w1;
      }
    }

    k = 7;
    { // S2(7)
      float w0 = *w++;
      float w1 = *w++;
      float* vz = &zlin[4 * i - k * 64];
      float* vy = &zlin[4 * i - (15 - k) * 64];
      for (size_t j = 0; j < 4; j++) {
        b[j] += vz[j] * w1 + vy[j] * w0;
        a[j] += vy[j] * w1 - vz[j] * w0;
      }
    }

    dstr[(15 - i) * channels] = ScalePCMFloat(a[1]);
    dstr[(17 + i) * channels] = ScalePCMFloat(b[1]);
    outData[(15 - i) * channels] = ScalePCMFloat(a[0]);
    outData[(17 + i) * channels] = ScalePCMFloat(b[0]);
    dstr[(47 - i) * channels] = ScalePCMFloat(a[3]);
    dstr[(49 + i) * channels] = ScalePCMFloat(b[3]);
    outData[(47 - i) * channels] = ScalePCMFloat(a[2]);
    outData[(49 + i) * channels] = ScalePCMFloat(b[2]);
  }
}

int16 MP3::ScalePCM(float sample) {
  if (sample >= 32766.5f) {
    return (int16)32767;
  }

  if (sample <= -32767.5f) {
    return (int16)-32768;
  }

  int16 s = (int16)(sample + .5f);
  return (s < 0) ? s - 1 : s;
}

float MP3::ScalePCMFloat(float sample) {
  return sample * (1.f / 32768.f);
}

}
