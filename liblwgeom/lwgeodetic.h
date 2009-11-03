/**********************************************************************
 * $Id$
 *
 * PostGIS - Spatial Types for PostgreSQL
 * Copyright 2009 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public Licence. See the COPYING file.
 *
 **********************************************************************/

#include <math.h>
#include "libgeom.h"

extern int gbox_geocentric_slow;

/**
* Point in spherical coordinates on the world. Units of radians.
*/
typedef struct
{
	double lon;
	double lat; 
} GEOGRAPHIC_POINT;

/**
* Two-point great circle segment from a to b.
*/
typedef struct
{
	GEOGRAPHIC_POINT start;
	GEOGRAPHIC_POINT end;
} GEOGRAPHIC_EDGE;

/**
* Conversion functions
*/
#define deg2rad(d) (PI * (d) / 180.0)
#define rad2deg(r) (180.0 * (r) / PI)

/**
* Ape a java function
*/
#define signum(a) ((a) < 0 ? -1 : ((a) > 0 ? 1 : (a)))

/*
** Prototypes for internal functions.
*/
void geog2cart(GEOGRAPHIC_POINT g, POINT3D *p);
void cart2geog(POINT3D p, GEOGRAPHIC_POINT *g);
void robust_cross_product(GEOGRAPHIC_POINT p, GEOGRAPHIC_POINT q, POINT3D *a);
void x_to_z(POINT3D *p);
void y_to_z(POINT3D *p);
int edge_point_on_plane(GEOGRAPHIC_EDGE e, GEOGRAPHIC_POINT p);
int edge_point_in_cone(GEOGRAPHIC_EDGE e, GEOGRAPHIC_POINT p);
int edge_contains_coplanar_point(GEOGRAPHIC_EDGE e, GEOGRAPHIC_POINT p);
int edge_contains_point(GEOGRAPHIC_EDGE e, GEOGRAPHIC_POINT p);
double z_to_latitude(double z, int top);
int clairaut_cartesian(POINT3D start, POINT3D end, GEOGRAPHIC_POINT *g_top, GEOGRAPHIC_POINT *g_bottom);
int clairaut_geographic(GEOGRAPHIC_POINT start, GEOGRAPHIC_POINT end, GEOGRAPHIC_POINT *g_top, GEOGRAPHIC_POINT *g_bottom);
double sphere_distance(GEOGRAPHIC_POINT s, GEOGRAPHIC_POINT e);
double sphere_distance_cartesian(POINT3D s, POINT3D e);
int sphere_project(GEOGRAPHIC_POINT r, double distance, double azimuth, GEOGRAPHIC_POINT *n);
int edge_calculate_gbox(GEOGRAPHIC_EDGE e, GBOX *gbox);
int edge_calculate_gbox_slow(GEOGRAPHIC_EDGE e, GBOX *gbox);
int edge_intersection(GEOGRAPHIC_EDGE e1, GEOGRAPHIC_EDGE e2, GEOGRAPHIC_POINT *g);
double edge_distance_to_point(GEOGRAPHIC_EDGE e, GEOGRAPHIC_POINT gp, GEOGRAPHIC_POINT *closest);
double edge_distance_to_edge(GEOGRAPHIC_EDGE e1, GEOGRAPHIC_EDGE e2, GEOGRAPHIC_POINT *closest1, GEOGRAPHIC_POINT *closest2);
void edge_deg2rad(GEOGRAPHIC_EDGE *e);
void edge_rad2deg(GEOGRAPHIC_EDGE *e);
void point_deg2rad(GEOGRAPHIC_POINT *p);
void point_rad2deg(GEOGRAPHIC_POINT *p);
void geographic_point_init(double lon, double lat, GEOGRAPHIC_POINT *g);
int ptarray_point_in_ring_winding(POINTARRAY *pa, POINT2D pt_to_test);
int lwpoly_covers_point2d(const LWPOLY *poly, GBOX gbox, POINT2D pt_to_test);
int ptarray_point_in_ring(POINTARRAY *pa, POINT2D pt_outside, POINT2D pt_to_test);
double ptarray_area_sphere(POINTARRAY *pa, POINT2D pt_outside);
double latitude_degrees_normalize(double lat);
double longitude_degrees_normalize(double lon);
double ptarray_length_spheroid(POINTARRAY *pa, SPHEROID s);
int geographic_point_equals(GEOGRAPHIC_POINT g1, GEOGRAPHIC_POINT g2);
int crosses_dateline(GEOGRAPHIC_POINT s, GEOGRAPHIC_POINT e);
void point_shift(GEOGRAPHIC_POINT *p, double shift);
/*
** Prototypes for spheroid functions.
*/
double spheroid_distance(GEOGRAPHIC_POINT a, GEOGRAPHIC_POINT b, SPHEROID spheroid);
double spheroid_direction(GEOGRAPHIC_POINT r, GEOGRAPHIC_POINT s, SPHEROID spheroid);
int spheroid_project(GEOGRAPHIC_POINT r, SPHEROID spheroid, double distance, double azimuth, GEOGRAPHIC_POINT *g);
