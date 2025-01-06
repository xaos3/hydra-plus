#define DXDATATYPES

/*
 In this module some basic datatypes and structs will be declared
*/

enum dx_data_types {dxt_string,dxt_ustring,dxt_astring,dxt_float,dxt_int,dxt_pointer,dxt_char,dxt_byte,dxt_bytes,dxt_sock,dxt_ssl_sock,dxt_file,
                    dxt_database,dxt_dataset,dxt_xml,dxt_json,dxt_list};
/*#
 The supporting native types of the hydra+
#*/

enum dx_database_type  {dxd_SQLServer,dxd_MySQL,dxd_SQLite,dxd_CSV};
/*#
 The supported databases in the hydra+
 The dxd_CSV type is emulated as a select query.
#*/

typedef struct dxl_object
/*#
 This object is the object that the lists are store in the node object.
 is designed with the key of a hash table in mind , so to be
 compatible with simple lists , sorted list and dictionaries alike.
#*/
{
 PDX_STRING key        ;
 DXLONG64   int_key    ;
 double     float_key  ;
 void * obj		       ;
 uint32_t flags        ;

} *PDXL_OBJECT          ;

PDXL_OBJECT dxl_object_create()
{
  PDXL_OBJECT obj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object)) ;
  if(obj == NULL) return NULL ;
  obj->flags     = 0 ;
  obj->float_key = 0 ;
  obj->int_key   = 0 ; 
  obj->key       = NULL ;
  obj->obj		 = NULL ;
  return obj ;
}

/*Helper functions*/

static inline enum dx_compare_res dx_float_compare(double a, double b)
{
    if (fabs(a - b) < DBL_EPSILON) return dx_equal;
	 else
	   if (a > b) return dx_left_bigger;
		else return dx_right_bigger ;
}

static inline enum dx_compare_res dx_int_compare(DXLONG64 i1 , DXLONG64 i2)
{
   if (i1 == i2) return dx_equal;
	 else
	   if (i1 > i2) return dx_left_bigger;
		else return dx_right_bigger ;

}









