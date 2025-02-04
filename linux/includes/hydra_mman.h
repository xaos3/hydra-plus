/*
 Well the name is some what missleading as Hydra+ does not have a memory
 manager. 
 But what it have is a serious problem when terminating with multiple references 
 in the variables for the same object.
 To mitigate this that is actually a problem only when the [Hydra+] is terminating,
 (Or if the script tries to access a variable when another variable has disposed the referenced object, but this 
 is a programmer problem xD)
 A small finalization code will run , that it will make a full list with the non managed pointers that are disposed in the 
field->obj in the variables
 (remember simple strings are managed from [Hydra+] so we ommit them)
 and then it will call the destructors only once per object 
 even if there is multiple variables that are referenced the object.
 I know bad original design! Or is it ...??

 ---- Expand the memory manager ----
 ---- 29-01-2025 -------------------

 A very serious problem is the memory leaks. Its very easy to forgot to .Free 
 a complex type and in a loop to create alot of megabytes of memory lost.
 So I will create a special state of the Hydra+ that will log all the pointers that are being created 
 within the variables and it will remove them when the variable is being destroy this pointers.
 This state will introduce some delay , so it will be selectable by a special command in the start of the script.



 Nikos Mourgis deus-ex.gr 2024
 Live Long and Prosper

*/

#define HDR_MEM_HASH_BUCKETS 500 


typedef	PDX_HASH_TABLE PHDR_POINTERS ;
typedef	PDX_HASH_TABLE PHDR_POINTERS_LOGGER ;

/*the struct with the variable name and the object addres*/
typedef struct hdr_pointer_info
{
	PDX_STRING var_name ;
	void       *obj     ;   
}*PHDR_POINTER_INFO;


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

/*---------------------------------------------------------------------------*/

PHDR_POINTER_INFO hdr_mem_logger_create_info(char *var_name,void *object)
{
  PHDR_POINTER_INFO info = (PHDR_POINTER_INFO)malloc(sizeof(struct hdr_pointer_info)) ;
  info->var_name = dx_string_createU(NULL,var_name);
  info->obj      = object ;
  return info ;
}

void* hdr_mem_logger_free_info(PHDR_POINTER_INFO info)
{
	dx_string_free(info->var_name) ;
	free(info);
	return NULL ;
}

void* hdr_mem_logger_free_info_in_obj(PDXL_OBJECT obj)
{
	PHDR_POINTER_INFO info = (PHDR_POINTER_INFO)obj->obj ;
	dx_string_free(info->var_name) ;
	free(info);

}

PHDR_POINTERS_LOGGER hdr_mem_pointers_Logger_create()
{
  return dx_HashCreateTable(HDR_MEM_HASH_BUCKETS) ;
}


PHDR_POINTERS_LOGGER hdr_mem_pointers_logger_free(PHDR_POINTERS_LOGGER mem_logger)
{
	dx_HashDestroyTable(mem_logger,hdr_mem_logger_free_info_in_obj) ;
	return NULL ;
}

/*add directly an object info*/
void hdr_mem_pointers_logger_add(PHDR_POINTERS_LOGGER mem_logger,char *var_name,void *object)
{

  PDX_STRING strobj = dx_UintToStr((uint64_t)object) ;
  PDXL_OBJECT obj = dx_HashReturnItem(mem_logger,strobj,true) ;
  dx_string_free(strobj);/*we are done with it~!*/
  if(obj != NULL) return ; /*allready in!*/

  obj = dxl_object_create() ;
  obj->key = dx_UintToStr((uint64_t)object) ;
  obj->obj = hdr_mem_logger_create_info(var_name,object) ;
  dx_HashInsertObject(mem_logger,obj) ;
}

void hdr_mem_pointers_logger_remove(PHDR_POINTERS_LOGGER mem_logger,void *object)
{
 PDX_STRING strobj = dx_UintToStr((uint64_t)object) ;
 PDXL_OBJECT obj = dx_HashRemoveItem(mem_logger ,strobj);
 dx_string_free(strobj);

 if(obj == NULL) return ;

 PHDR_POINTER_INFO info = obj->obj ;
 dx_string_free(obj->key) ;

 hdr_mem_logger_free_info(info) ;
 free(obj);
}

void hdr_mem_pointers_logger_print(PHDR_POINTERS_LOGGER mem_logger)
{
	if(mem_logger->count == 0) return ;

	printf("----------------------- Undisposed Variables -----------------------\n");
	PDX_LIST *buckets = mem_logger->buckets ;
	DXLONG64 cindx = 0 ;
	for(DXLONG64 i = 0 ; i < mem_logger->length;i++)
	{
	 PDX_LIST bucket = buckets[i] ;
	 PDXL_NODE node = bucket->start ;
	 while(node!=NULL)
	 {
	     PDXL_OBJECT obj        = node->object ;
		 PHDR_POINTER_INFO info = (PHDR_POINTER_INFO)obj->obj ;
		 printf("Variable -> %s\n",info->var_name->stringa)   ;
		 node                   = node->right                 ;
	 }
	
	}
	return NULL ; // no luck !

}







