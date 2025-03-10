#include "rccrsv.h"

const char*
rn(size_t n, size_t size)
{
  if (size > 4) {
    switch (n) {
      case 0:
        return "x0";
      case 1:
        return "x1";
      case 2:
        return "x2";
      case 3:
        return "x3";
      case 4:
        return "x4";
      case 5:
        return "x5";
      case 6:
        return "x6";
      case 7:
        return "x7";
      case 8:
        return "x8";
      case 9:
        return "x9";
      case 10:
        return "x10";
      case 11:
        return "x11";
      case 12:
        return "x12";
      case 13:
        return "x13";
      case 14:
        return "x14";
      case 15:
        return "x15";
      case 16:
        return "x16";
      case 17:
        return "x17";
      case 18:
        return "x18";
      case 19:
        return "x19";
      case 20:
        return "x20";
      case 21:
        return "x21";
      case 22:
        return "x22";
      case 23:
        return "x23";
      case 24:
        return "x24";
      case 25:
        return "x25";
      case 26:
        return "x26";
      case 27:
        return "x27";
      case 28:
        return "x28";
      case 29:
        return "x29";
      case 30:
        return "x30";
    }
  } else {
    switch (n) {
      case 0:
        return "w0";
      case 1:
        return "w1";
      case 2:
        return "w2";
      case 3:
        return "w3";
      case 4:
        return "w4";
      case 5:
        return "w5";
      case 6:
        return "w6";
      case 7:
        return "w7";
      case 8:
        return "w8";
      case 9:
        return "w9";
      case 10:
        return "w10";
      case 11:
        return "w11";
      case 12:
        return "w12";
      case 13:
        return "w13";
      case 14:
        return "w14";
      case 15:
        return "w15";
      case 16:
        return "w16";
      case 17:
        return "w17";
      case 18:
        return "w18";
      case 19:
        return "w19";
      case 20:
        return "w20";
      case 21:
        return "w21";
      case 22:
        return "w22";
      case 23:
        return "w23";
      case 24:
        return "w24";
      case 25:
        return "w25";
      case 26:
        return "w26";
      case 27:
        return "w27";
      case 28:
        return "w28";
      case 29:
        return "w29";
      case 30:
        return "w30";
    }
  }

  fprintf(stderr, "Register is not supported %zu\n", n);
  exit(1);
}
