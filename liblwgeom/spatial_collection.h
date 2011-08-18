#ifndef SPATIAL_COLLECTION_H
#define SPATIAL_COLLECTION_H

#include "liblwgeom.h"


/**
 * Declares the "type" of a collection. A "spatial only" collection is
 * composed only of geometries and has no associated value. A "spatial
 * plus value" collection associates values with a location, i.e.: a raster.
 */
enum COLLECTION_TYPE { SPATIAL_ONLY, SPATIAL_PLUS_VALUE } ;

/**
 * Contains a list of numeric values. This is used to represent the value
 * part of a spatial-plus-value collection. For a raster, the "value" of a
 * grid cell is the data in all of the bands at that cell.
 */
typedef struct value_t {
	int length ;   /* number of values in the array */
	double *data ; /* the values */
} VALUE ;

/**
 * Encapsulates a single binding of geometry to value.
 */
typedef struct geometry_value_t {
	LWGEOM *geometry ;
	VALUE  *value ;
} GEOMETRY_VALUE ;

typedef void PARAMETERS ;

struct spatial_collection_i ;
struct includes_i ;
struct evaluator_i ;

typedef struct spatial_collection_i SPATIAL_COLLECTION ;
typedef struct includes_i INCLUDES ;
typedef struct evaluator_i EVALUATOR ;


/* Includes interface */
typedef int ((*INCLUDE_FN)(INCLUDES *, LWPOINT *)) ;
struct includes_i {
	PARAMETERS *params ;
	SPATIAL_COLLECTION *collection ;
	INCLUDE_FN includes ;
	INCLUDE_FN includesIndex ;
};

/* Evaluator interface */
typedef VALUE *((*EVALUATOR_FN)(EVALUATOR *, LWPOINT *)) ;
struct evaluator_i {
	PARAMETERS *params ;
	SPATIAL_COLLECTION *collection ;
	EVALUATOR_FN evaluate ;
	EVALUATOR_FN evaluateIndex ;
	VALUE *result ; /* the single VALUE array which is overwritten on each call */
};

/* Spatial collection interface */
struct spatial_collection_i {
	COLLECTION_TYPE type ;
	int32       srid ;
	PARAMETERS *params ;
	SPATIAL_COLLECTION *input1 ;
	SPATIAL_COLLECTION *input2 ;
	INCLUDES *inclusion ;
	EVALUATOR *evaluator ;
};

SPATIAL_COLLECTION *
sc_create(COLLECTION_TYPE t,
		  int32 srid,
		  PARAMETERS *params,
		  INCLUDES *inc,
		  EVALUATOR *eval) ;

SPATIAL_COLLECTION *
sc_twoinput_create(COLLECTION_TYPE t,
		           PARAMETERS *params,
		           INCLUDES   *inc,
		           EVALUATOR  *eval,
		           SPATIAL_COLLECTION *input1,
		           SPATIAL_COLLECTION *input2) ;

/* methods on the spatial collection interface */
VALUE *sc_evaluate(SPATIAL_COLLECTION *sc, LWPOINT *point);
VALUE *sc_evaluateIndex(SPATIAL_COLLECTION *sc, LWPOINT *point);
int sc_includes(SPATIAL_COLLECTION *sc, LWPOINT *point);
int sc_includesIndex(SPATIAL_COLLECTION *sc, LWPOINT *point);
int sc_hasValue(SPATIAL_COLLECTION *sc) ;
int sc_hasTwoInputs(SPATIAL_COLLECTION *sc) ;
int32 sc_get_srid(SPATIAL_COLLECTION *sc) ;



INCLUDES *
inc_create(PARAMETERS *params,
		   INCLUDE_FN includes,
		   INCLUDE_FN includesIndex);

EVALUATOR *
eval_create(PARAMETERS *params,
		    EVALUATOR_FN evaluator,
		    EVALUATOR_FN evaluatorIndex) ;

void eval_destroy(EVALUATOR *eval);
void inc_destroy(INCLUDES *inc) ;
void sc_destroy(SPATIAL_COLLECTION *sc) ;

VALUE *val_create(int num_values);
void val_destroy(VALUE *val) ;

/* Implementations of the includes interface */
INCLUDES *sc_create_geometry_includes(LWGEOM *geom) ;
void sc_destroy_geometry_includes(INCLUDES *dead) ;

INCLUDES *sc_create_relation_includes(RELATION_FN relation);
void sc_destroy_relation_includes(INCLUDES *dead) ;

INCLUDES *sc_create_projection_includes(INCLUDES *wrapped, projPJ source, projPJ dest) ;
void sc_destroy_projection_includes(INCLUDES *dead);



/* Implementations of the evaluator interface */
EVALUATOR *sc_create_mask_evaluator(double true_val, double false_val, int index);
void sc_destroy_mask_evaluator(EVALUATOR *dead) ;

EVALUATOR *sc_create_first_value_evaluator(void);
void sc_destroy_first_value_evaluator(EVALUATOR *dead);

EVALUATOR *sc_create_projection_eval(EVALUATOR *wrapped, projPJ source, projPJ dest) ;
void sc_destroy_projection_eval(EVALUATOR *dead);



/* implementations of the spatial collection interface */
SPATIAL_COLLECTION *sc_create_geometry_wrapper(LWGEOM *geom, int32 srid, double inside, double outside);
void sc_destroy_geometry_wrapper(SPATIAL_COLLECTION *dead) ;

SPATIAL_COLLECTION *
sc_create_projection_wrapper(SPATIAL_COLLECTION *wrapped,
		                     int32 desired_srid,
		                     projPJ wrapped_proj, projPJ desired_proj );
void sc_destroy_projection_wrapper(SPATIAL_COLLECTION *dead);



#endif /* SPATIAL_COLLECTION_H */
