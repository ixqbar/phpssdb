/*
 * ssdb_geo.h
 *
 */

#ifndef EXT_SSDB_SSDB_GEO_H_
#define EXT_SSDB_SSDB_GEO_H_

#include "php.h"
#include "ssdb_library.h"
#include "geo/geohash.h"
#include "geo/geohash_helper.h"

typedef struct _SSDBGeoNode {
	struct _SSDBGeoNode *prev;
	struct _SSDBGeoNode *next;
	void *data;
} SSDBGeoNode;

typedef struct {
	SSDBGeoNode *head;
	SSDBGeoNode *tail;
	unsigned long num;
	void (*free)(void *ptr);
} SSDBGeoList;

typedef struct {
    double latitude;
    double longitude;
    double dist;
    char *member;
    int member_key_len;
} SSDBGeoPoint;

typedef struct {
	SSDBSock *ssdb_sock;
	char *key;
	int key_len;
	char *member_key;
	int member_key_len;
} SSDBGeoObj;

bool ssdb_geo_set(
		SSDBSock *ssdb_sock,
		char *key,
		int key_len,
		char *member_key,
		int member_key_len,
		double latitude,
		double longitude,
		INTERNAL_FUNCTION_PARAMETERS);

bool ssdb_geo_get(
		SSDBSock *ssdb_sock,
		char *key,
		int key_len,
		char *member_key,
		int member_key_len,
		INTERNAL_FUNCTION_PARAMETERS);

bool ssdb_geo_neighbours(
		SSDBSock *ssdb_sock,
		char *key,
		int key_len,
		char *member_key,
		int member_key_len,
		double radius_meters,
		long limit,
		INTERNAL_FUNCTION_PARAMETERS);

bool ssdb_geo_distance(
		SSDBSock *ssdb_sock,
		char *key,
		int key_len,
		char *member_a_key,
		int member_a_key_len,
		char *member_b_key,
		int member_b_key_len,
		INTERNAL_FUNCTION_PARAMETERS);

#endif /* EXT_SSDB_SSDB_GEO_H_ */
