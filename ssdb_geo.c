/*
 * ssdb_geo.c
 *
 */

#include "ssdb_geo.h"

static bool decodeGeohash(GeoHashFix52Bits bits, double *latlong) {
	GeoHashArea area;
    GeoHashBits hash = {.bits = (uint64_t)bits, .step = GEO_STEP_MAX};
    if (!geohashDecodeWGS84(hash, &area)) {
    	return false;
    }

    latlong[0] = area.latitude.min;
    latlong[1] = area.longitude.min;

    return true;
}

static SSDBGeoList *ssdb_geo_list_create() {
	SSDBGeoList *l;

	if ((l = malloc(sizeof(SSDBGeoList))) == NULL) {
		return NULL;
	}

	l->head = NULL;
	l->tail = NULL;
	l->num  = 0;
	l->free = NULL;

	return l;
}

static void ssdb_geo_list_destory(SSDBGeoList *l) {
	if (l == NULL) return;

	unsigned long num;
	SSDBGeoNode *current, *next;

	current = l->head;
	num = l->num;
	while (num--) {
		next = current->next;
		if (l->free && current->data != NULL) l->free(current->data);
		free(current);
		current = next;
	}

	free(l);
}

static SSDBGeoList *ssdb_geo_list_add_tail_node(SSDBGeoList *l, void *value) {
	SSDBGeoNode *node;

   if ((node = malloc(sizeof(*node))) == NULL) {
	   return NULL;
   }

   node->data = value;
   if (l->num == 0) {
	   l->head = l->tail = node;
	   node->prev = node->next = NULL;
   } else {
	   node->prev = l->tail;
	   node->next = NULL;
	   l->tail->next = node;
	   l->tail = node;
   }
   l->num++;

   return l;
}

static void ssdb_geo_list_join(SSDBGeoList *join_to, SSDBGeoList *join) {
    if (join_to->num == 0) {
        join_to->head = join->head;
    } else {
        join_to->tail->next = join->head;
        join->head->prev = join_to->tail;
        join_to->tail = join->tail;
    }

    join_to->num += join->num;

    free(join);
}

static int ssdb_geo_point_sort_asc(const void *a, const void *b) {
    const SSDBGeoPoint *gpa = a, *gpb = b;
    if (gpa->dist > gpb->dist) {
        return 1;
    } else if (gpa->dist == gpb->dist) {
    	return strcmp(gpa->member, gpb->member);
	} else {
        return -1;
	}
}

static SSDBGeoList *ssdb_geo_member_box(SSDBGeoObj *ssdb_geo_obj, GeoHashBits hash) {
    GeoHashFix52Bits min, max;

    min = geohashAlign52Bits(hash);
    hash.bits++;
    max = geohashAlign52Bits(hash);

    char *score_start = NULL;
    int score_start_len = spprintf(&score_start, 0, "%lld", min);

    char *score_end = NULL;
    int score_end_len = spprintf(&score_end, 0, "%lld", max);

    char *cmd = NULL;
	int cmd_len = 0, key_free = 0;

	key_free = ssdb_key_prefix(ssdb_geo_obj->ssdb_sock, &ssdb_geo_obj->key, &ssdb_geo_obj->key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_geo_obj->ssdb_sock,
			&cmd, ZEND_STRL("zscan"),
			ssdb_geo_obj->key,
			ssdb_geo_obj->key_len,
			"",
			0,
			score_start,
			score_start_len,
			score_end,
			score_end_len,
			ssdb_geo_obj->limit_str,
			ssdb_geo_obj->limit_str_len,
			NULL);

	efree(score_start);
	efree(score_end);
	if (key_free) efree(ssdb_geo_obj->key);
	if (0 == cmd_len) return NULL;

	if (ssdb_sock_write(ssdb_geo_obj->ssdb_sock, cmd, cmd_len) < 0) {
		efree(cmd);
		return NULL;
	}
	efree(cmd);

	SSDBResponse *ssdb_response = ssdb_sock_read(ssdb_geo_obj->ssdb_sock);
	if (ssdb_response == NULL
			|| ssdb_response->status != SSDB_IS_OK
			|| ssdb_response->num % 2 != 0) {
		ssdb_response_free(ssdb_response);
		return NULL;
	}

	//创建链表
	SSDBGeoList *l = ssdb_geo_list_create();
	if (l == NULL) {
		return NULL;
	}

	l->free = free;

	double latlong[2] = {0};
	int i = 1;
	bool err = false;

	SSDBResponseBlock *ssdb_response_block = ssdb_response->block;
	while (ssdb_response_block != NULL) {
		if (0 == i % 2) {
			if (!decodeGeohash(atoll(ssdb_response_block->data), latlong)) {
				err = true;
				break;
			}

			SSDBGeoPoint *p   = malloc(sizeof (SSDBGeoPoint));
			p->member_key_len = ssdb_response_block->prev->len;
			p->member         = estrndup(ssdb_response_block->prev->data, ssdb_response_block->prev->len);
			p->dist           = 0.0;
			p->latitude       = latlong[0];
			p->longitude      = latlong[1];

			ssdb_geo_list_add_tail_node(l, p);
		}
		ssdb_response_block = ssdb_response_block->next;
		i++;
	}

	if (err || 0 == l->num) {
		ssdb_geo_list_destory(l);
		l = NULL;
	}

	ssdb_response_free(ssdb_response);
    return l;
}

static SSDBGeoList *ssdb_geo_members(SSDBGeoObj *ssdb_geo_obj, GeoHashRadius n, double latitude, double longitude, double radius_meters) {
	GeoHashBits neighbors[9];

	neighbors[0] = n.hash;
	neighbors[1] = n.neighbors.north;
	neighbors[2] = n.neighbors.south;
	neighbors[3] = n.neighbors.east;
	neighbors[4] = n.neighbors.west;
	neighbors[5] = n.neighbors.north_east;
	neighbors[6] = n.neighbors.north_west;
	neighbors[7] = n.neighbors.south_east;
	neighbors[8] = n.neighbors.south_west;

	SSDBGeoList *l = NULL;
        int i;
	for (i = 0; i < sizeof(neighbors) / sizeof(*neighbors); i++) {
		if (HASHISZERO(neighbors[i])) {
			continue;
		}

		SSDBGeoList *tl = ssdb_geo_member_box(ssdb_geo_obj, neighbors[i]);
		if (tl == NULL) {
			continue;
		}

		if (!l) {
			l = tl;
		} else {
			ssdb_geo_list_join(l, tl);
		}
	}

	if (l) {
		SSDBGeoNode *c,*n = l->head;
		SSDBGeoPoint *p;
		while (n != NULL) {
			c = n;
			p = (SSDBGeoPoint *)c->data;
			n = n->next;

			if ((ssdb_geo_obj->member_key_len == p->member_key_len
					&& 0 == strncmp(ssdb_geo_obj->member_key, p->member, p->member_key_len))
					|| !geohashGetDistanceIfInRadiusWGS84(longitude, latitude, p->longitude, p->latitude, radius_meters, &p->dist)) {
				//不在范围内删除
				l->num--;
				//说明当前为head
				if (c->prev == NULL) {
					l->head = c->next;
				}
				//说明当前为tail
				if (c->next == NULL) {
					l->tail = c->prev;
				}
				//重整
				c->prev->next = c->next;
				c->next->prev = c->prev;
				//释放
				efree(p->member);
				free(p);
				free(c);
			}
		}
	}

	if (0 == l->num) {
		free(l);
		l = NULL;
	}

	return l;
}

static bool ssdb_geo_member(
		SSDBSock *ssdb_sock,
		char *key,
		int key_len,
		char *member_key,
		int member_key_len,
		double *latlong) {
	char *cmd = NULL;
	int cmd_len = 0, key_free = 0;

	key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, ZEND_STRL("zget"), key, key_len, member_key, member_key_len, NULL);

	if (key_free) efree(key);
	if (0 == cmd_len) return false;

	if (ssdb_sock_write(ssdb_sock, cmd, cmd_len) < 0) {
		efree(cmd);
		return false;
	}
	efree(cmd);

	SSDBResponse *ssdb_response = ssdb_sock_read(ssdb_sock);
	if (ssdb_response == NULL || ssdb_response->status != SSDB_IS_OK) {
		ssdb_response_free(ssdb_response);
		return false;
	}

	if (!decodeGeohash(atoll(ssdb_response->block->data), latlong)) {
		ssdb_response_free(ssdb_response);
		return false;
	}

	ssdb_response_free(ssdb_response);

	return true;
}

bool ssdb_geo_set(
		SSDBSock *ssdb_sock,
		char *key,
		int key_len,
		char *member_key,
		int member_key_len,
		double latitude,
		double longitude,
		INTERNAL_FUNCTION_PARAMETERS) {
	GeoHashBits hash;
	if (!geohashEncodeWGS84(latitude, longitude, GEO_STEP_MAX, &hash)) {
		return false;
	}

	char *member_score_str = NULL, *cmd = NULL;
	GeoHashFix52Bits bits = geohashAlign52Bits(hash);
	int member_score_str_len = spprintf(&member_score_str, 0, "%lld", bits);
	int key_free = ssdb_key_prefix(ssdb_sock, &key, &key_len);
	int cmd_len = ssdb_cmd_format_by_str(ssdb_sock, &cmd, ZEND_STRL("zset"), key, key_len, member_key, member_key_len, member_score_str, member_score_str_len, NULL);

	efree(member_score_str);
	if (key_free) efree(key);
	if (0 == cmd_len) return false;

	if (ssdb_sock_write(ssdb_sock, cmd, cmd_len) < 0) {
		efree(cmd);
		return false;
	}
	efree(cmd);

	SSDBResponse *ssdb_response = ssdb_sock_read(ssdb_sock);
	if (ssdb_response == NULL || ssdb_response->status != SSDB_IS_OK) {
		ssdb_response_free(ssdb_response);
		return false;
	}

	ssdb_response_free(ssdb_response);
	RETVAL_LONG(bits);

	return true;
}

bool ssdb_geo_get(
		SSDBSock *ssdb_sock,
		char *key,
		int key_len,
		char *member_key,
		int member_key_len,
		INTERNAL_FUNCTION_PARAMETERS) {
	double latlong[2] = {0};
	if (!ssdb_geo_member(ssdb_sock, key, key_len, member_key, member_key_len, (double *)latlong)) {
		return false;
	}

	array_init(return_value);
	add_assoc_double_ex(return_value, ZEND_STRS("latitude"),  latlong[0]);
	add_assoc_double_ex(return_value, ZEND_STRS("longitude"), latlong[1]);

	return true;
}

bool ssdb_geo_neighbours(
		SSDBSock *ssdb_sock, char *key,
		int key_len,
		char *member_key,
		int member_key_len,
		double radius_meters,
		long return_limit,
		long zscan_limit,
		INTERNAL_FUNCTION_PARAMETERS) {
	double latlong[2] = {0};
	if (!ssdb_geo_member(ssdb_sock, key, key_len, member_key, member_key_len, (double *)latlong)) {
		return false;
	}

	SSDBGeoObj *ssdb_geo_obj = emalloc(sizeof(SSDBGeoObj));

	ssdb_geo_obj->ssdb_sock      = ssdb_sock;
	ssdb_geo_obj->key            = key;
	ssdb_geo_obj->key_len        = key_len;
	ssdb_geo_obj->member_key     = member_key;
	ssdb_geo_obj->member_key_len = member_key_len;
	ssdb_geo_obj->limit_str      = NULL;
	ssdb_geo_obj->limit_str_len  = spprintf(&ssdb_geo_obj->limit_str, 0, "%ld", zscan_limit);

	GeoHashRadius georadius = geohashGetAreasByRadiusWGS84(latlong[0], latlong[1], radius_meters);
	SSDBGeoList *l = ssdb_geo_members(ssdb_geo_obj, georadius, latlong[0], latlong[1], radius_meters);

	efree(ssdb_geo_obj->limit_str);
	efree(ssdb_geo_obj);
	if (l == NULL) {
		return false;
	}

	return_limit = return_limit <= 0 ? l->num : return_limit;

	int i = 0;
	SSDBGeoPoint *p_l = malloc(sizeof(SSDBGeoPoint) * l->num);
	SSDBGeoPoint *p;
	SSDBGeoNode *n = l->head;
	while (n != NULL) {
		p = (SSDBGeoPoint *)n->data;

		p_l[i].member    = p->member;
		p_l[i].latitude  = p->latitude;
		p_l[i].longitude = p->longitude;
		p_l[i].dist      = p->dist;

		n = n->next;
		i++;
	}

	qsort(p_l, l->num, sizeof(SSDBGeoPoint), ssdb_geo_point_sort_asc);

	zval *temp;
	array_init(return_value);
	for (i = 0; i < l->num; i++) {
		if (i < return_limit) {
			MAKE_STD_ZVAL(temp);
			array_init_size(temp, 3);
			add_assoc_double_ex(temp, ZEND_STRS("latitude"),  p_l[i].latitude);
			add_assoc_double_ex(temp, ZEND_STRS("longitude"), p_l[i].longitude);
			add_assoc_double_ex(temp, ZEND_STRS("distance"),  p_l[i].dist);
			add_assoc_zval(return_value, p_l[i].member, temp);
		}
		efree(p_l[i].member);
	}

	free(p_l);
	ssdb_geo_list_destory(l);

	return true;
}

bool ssdb_geo_distance(
		SSDBSock *ssdb_sock,
		char *key,
		int key_len,
		char *member_a_key,
		int member_a_key_len,
		char *member_b_key,
		int member_b_key_len,
		INTERNAL_FUNCTION_PARAMETERS) {
	double a_latlong[2] = {0};
	if (!ssdb_geo_member(ssdb_sock, key, key_len, member_a_key, member_a_key_len, (double *)a_latlong)) {
		return false;
	}

	double b_latlong[2] = {0};
	if (!ssdb_geo_member(ssdb_sock, key, key_len, member_b_key, member_b_key_len, (double *)b_latlong)) {
		return false;
	}

	RETVAL_DOUBLE(geohashDistanceEarth(a_latlong[0], a_latlong[1], b_latlong[0], b_latlong[1]));

	return true;
}
