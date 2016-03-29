/*
 * ssdb_wgs.h
 *
 */

#ifndef EXT_PHPSSDB_SSDB_WGS_H_
#define EXT_PHPSSDB_SSDB_WGS_H_

void wgs2gcj(double wgsLat, double wgsLng, double *gcjLat, double *gcjLng);
void gcj2wgs(double gcjLat, double gcjLng, double *wgsLat, double *wgsLng);
void gcj2wgs_exact(double gcjLat, double gcjLng, double *wgsLat, double *wgsLng);

#endif /* EXT_PHPSSDB_SSDB_WGS_H_ */
