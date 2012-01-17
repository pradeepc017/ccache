/********************************************************************
	created:	2008/01/23
	filename: 	lrulist.c
	author:		Lichuang
                
	purpose:    
*********************************************************************/

#ifndef __CCACHE_LRULIST_H__
#define __CCACHE_LRULIST_H__

#include "ccache.h"

void ccache_lrulist_advance(ccache_t *cache, ccache_node_t *node);
int  ccache_lrulist_free(ccache_t *cache, ccache_node_t *node);
int  ccache_lrulist_return(ccache_t *cache, ccache_node_t *node);

#endif /* __CCACHE_LRULIST_H__ */

