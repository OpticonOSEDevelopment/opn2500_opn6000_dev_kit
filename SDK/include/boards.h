#ifndef BOARDS_H
#define BOARDS_H

#if defined(BOARD_OPN2500_V4_nRF52840)
  #include "opn2500_v4_NRF52840.h"
#elif defined(BOARD_OPN6000_nRF52840)
  #include "opn6000_NRF52840.h"
#else
#error "Board is not defined"

#endif

#endif
