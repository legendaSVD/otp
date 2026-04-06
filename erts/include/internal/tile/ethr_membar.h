#ifndef ETHR_TILE_MEMBAR_H__
#define ETHR_TILE_MEMBAR_H__
#define ETHR_LoadLoad	(1 << 0)
#define ETHR_LoadStore	(1 << 1)
#define ETHR_StoreLoad	(1 << 2)
#define ETHR_StoreStore	(1 << 3)
#define ETHR_MEMBAR(B) __insn_mf()
#endif