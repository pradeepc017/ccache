/********************************************************************
	created:	2008/01/23
	filename: 	ccache.h
	author:		Lichuang
                
	purpose:    
*********************************************************************/

#ifndef __CCACHE_CCACHE_H__
#define __CCACHE_CCACHE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "ccache_hash.h"
#include "ccache_node.h"
#include "ccache_error.h"
#include <pthread.h>

typedef struct ccache_data_t
{
    int     datasize;               /* the size of the data */
    int     keysize;                /* the size of the key */
    char   *data;                   /* the pointer of the data */
    char   *key;                    /* the pointer of the key */
}ccache_data_t;

/**
 * @brief   function pointer type used to compare data
 * @return  > 0 data1 > data2, = 0 data1 = data2, < 0 data1 < data2
 */
typedef     int (*ccache_compare_t)(const void* data1, const void* data2, int len);

/**
 * @brief   function pointer type used when deleting data
 * @return 
 */
typedef     void (*ccache_erase_t)(void* arg, const ccache_node_t *node);

/**
 * @brief   function pointer type used to update a node when node exist, the result will be saved in data
 * @param   node: original data
 * @param   data: update data and saved the result 
 * @return 
 */
typedef     void (*ccache_update_t)(const ccache_node_t *node, ccache_data_t *data);

/**
 * @brief   function pointer type used to visit all the nodes in the cache
 * @param   
 * @return 
 */
typedef     void (*ccache_visit_t)(void* arg, ccache_node_t *node);

struct ccache_t;

/* function pointers operating data */
typedef struct ccache_functor_t
{
    /*
     * find a data in the hashindex hashtable  
     */
    ccache_node_t* (*find)(struct ccache_t *cache,
							int hashindex, const ccache_data_t *data);

    /*
     * insert a data in the hashindex hashtable, if the key exist,
     * return the node and set exist 
     */
    ccache_node_t* (*insert)(struct ccache_t *cache, int hashindex, 
							const ccache_data_t *data, ccache_erase_t erase, 
                            void* arg, int *exist);

    /*
     * set a data in the hashindex hashtable,
     * if the key exist, insert the data 
     */
    ccache_node_t* (*set)(struct ccache_t *cache, int hashindex, 
							ccache_data_t *data, ccache_erase_t erase, 
                            void* arg, ccache_update_t update);

    /*
     * update a data in the hashindex hashtable, 
     * if the key is not exist, return NULL
     */
    ccache_node_t* (*update)(struct ccache_t *cache, 
							int hashindex, const ccache_data_t *data); 

    /*
     * erase a data in the hashindex hashtable and return the erased node, 
     * if the key is not exist, return NULL
     */
    ccache_node_t* (*erase)(struct ccache_t *cache, int hashindex, ccache_node_t *node);

    /*
     * visit the nodes in the hashindex hash-table
     */
    void           (*visit)(struct ccache_t *cache, int hashindex, ccache_visit_t visit, void* arg);
}ccache_functor_t;

typedef struct ccache_stat_count_t
{
    int total_num;
    int success_num;
    int fail_num;
}ccache_stat_count_t;

typedef struct ccache_stat_t
{
    ccache_stat_count_t find_stat;
    ccache_stat_count_t update_stat;
    ccache_stat_count_t set_stat;
    ccache_stat_count_t insert_stat;
    ccache_stat_count_t erase_stat;
}ccache_stat_t;

struct ccache_freearea_t;

/* data struct that operating cache */
typedef struct ccache_t
{
    int hashitemnum;                /* number of hashitem */
    int freeareanum;                /* number of freearea */
    int datasize;                   /* size of data */
    int filesize;                   /* size of share memory file */
    int freesize;                   /* the size of free data */
    char *start_free;               /* the pointer to the free data, if there is not free data, equals to NULL */

    ccache_functor_t functor;
    ccache_stat_t stat;
    pthread_rwlock_t lock;          /* thread reader-writer lock */
    struct ccache_freearea_t* freearea;    /* pointer to the freearea array */

    ccache_hash_t hashitem[0];      /* pointer to the hashitem array */
}ccache_t;

typedef struct ccache_config_t
{
    char    *path;          
    int     min_size;
    int     max_size;
    int     hashitem;
    int     datasize;
    int     prealloc_num;
    int     align_size;
    char    init;
} ccache_config_t;

/**
 * @brief   open a ccache 
 * @param   configfile: the config file path
 * @param   compare: the function pointer used when comparing keys, if NULL, use
 *          the memcmp function instead
 * @return  NULL if failed
 */
ccache_t*    ccache_open(const char *configfile, ccache_compare_t compare);

/**
 * @brief   open a ccache 
 * @param   configfile: the config file path
 * @param   compare: the function pointer used when comparing keys, if NULL, use
 *          the memcmp function instead
 * @return  NULL if failed
 */
ccache_t*    ccache_open2(ccache_config_t *config, ccache_compare_t compare);

/**
 * @brief   close a ccache 
 * @param   
 * @return  
 */
void         ccache_close(ccache_t *cache);

/**
 * @brief   insert a data into the cache
 * @param   cache: the cache pointer
 * @param   data: the data will be inserted
 * @param   erase: when there is no more space to insert data, use LRU algorithm to allocate a new node, 
 *               this function used to manage the deleted node, if NULL delete the node directly
 * @param   arg: the argument passed to the erase function
 * @return  0 if success, -1 if failed
 */
int         ccache_insert(ccache_t *cache, const ccache_data_t *data, 
                        ccache_erase_t erase, void* arg);

/**
 * @brief   find a node in the cache
 * @param   cache: the cache pointer
 * @param   data  if success, the value of the node contained in this data, so it must not be NULL
 * @return  0 if success, -1 if failed
 */
int         ccache_find(ccache_t *cache, ccache_data_t *data);

/**
 * @brief   update a node in the cache
 * @param   cache: the cache pointer
 * @param   data: the data will be update to the node,if success, the value of the node contained in this data,
 *          so it must not be NULL
 * @return  0 if success, -1 if failed
 * @NOTE    the data size and key size MUST equal to the previous
 */
int         ccache_update(ccache_t *cache, const ccache_data_t *data);

/**
 * @brief   erase a node in the cache
 * @param   cache: the cache pointer
 * @param   data:  if success, the value of the deleted node contained in this data,so it must not be NULL
 * @return  0 if success, -1 if failed
 */
int         ccache_erase(ccache_t *cache, ccache_data_t *data);

/**
 * @brief   set the key with the data, no matter if or not the key exists
 * @param   cache: the cache pointer
 * @param   data:   the data updated to the node, if success, the new value of node contained in this data,so it must not be NULL
 * @param   arg: the argument passed to the erase function
 * @param   update: the function used to update node
 * @return  0 if success, -1 if failed
 */
int         ccache_set(ccache_t *cache, ccache_data_t *data,
                        ccache_erase_t erase, void* arg, ccache_update_t update);

/**
 * @brief   use the visit function to visit all the node in the cache
 * @param   cache: the cache pointer
 * @param   visit: the function used to visit node
 * @param   arg: the argument passed to the visit function
 * @return  0 if success, -1 if failed
 */
int        ccache_visit(ccache_t *cache, ccache_visit_t visit, void* arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CCACHE_CCACHE_H__ */

