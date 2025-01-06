/*
 Well the name is some what missleading as Hydra+ does not have a memory
 manager. 
 But what it have is a serious problem when terminating with multiple references 
 in the variables for the same object.
 To mitigate this that is actually a problem only when the [Hydra+] is terminating,
 (Or if the script tries to access a variable when another variable has disposed the referenced object, but this 
 is a programmer problem xD)
 A small finalization code will run , that it will make a full list with the non managed pointers that are disposed in the 
field ->obj in the variables
 (remember simple strings are managed from [Hydra+] so we ommit them)
 and then it will call the destructors only once per object 
 even if there is multiple variables that are referenced the object.
 I know bad original design! Or is it ...??

 Nikos Mourgis deus-ex.gr 2024
 Live Long and Prosper

*/

#define HDR_MEM_HASH_BUCKETS 500 


typedef	PDX_HASH_TABLE PHDR_POINTERS ;


PHDR_POINTERS hdr_mem_pointers_create()
{
  return dx_HashCreateTable(HDR_MEM_HASH_BUCKETS) ;
}


bool hdr_mem_pointers_is_freed(PHDR_POINTERS pointers,uint64_t pointer_as_uint)
{
  /*if the function returns false then the pointer must be destroyed , if return true then then pointer has been already destroyed*/
  PDX_STRING  str = dx_UintToStr(pointer_as_uint) ;
  PDXL_OBJECT obj = dx_HashReturnItem(pointers,str,true) ;
  dx_string_free(str) ;
  if(obj == NULL) /*add the object as the variable will destroy the pointer*/
  {
	 PDXL_OBJECT obj = dxl_object_create() ;
	 obj->key = dx_UintToStr(pointer_as_uint) ;
	 dx_HashInsertObject(pointers,obj) ;
	  return false;
  }
  else
	  return true ;
}

PHDR_POINTERS hdr_mem_pointers_free(PHDR_POINTERS pointers)
{
	/*the obj->key is automatically freed so no need for special function*/
	dx_HashDestroyTable(pointers,NULL) ;
	return NULL ;
}