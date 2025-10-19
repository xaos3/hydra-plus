#include <thirdparty/monocypher/monocypher.h>

/******utils*****/

#ifdef _LINUX
static int urandom_fallback(uint8_t *out, size_t len) {
    int flags = 0;
    #ifdef O_CLOEXEC
      flags |= O_CLOEXEC;
    #endif
    int fd = open("/dev/urandom", O_RDONLY | flags);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, out + off, len - off);
        if (r < 0) { if (errno == EINTR) continue; close(fd); return -1; }
        if (r == 0) { close(fd); return -1; }
        off += (size_t)r;
    }
    close(fd);
    return 0;
}

int hdr_gen_random_bytes(uint8_t *out, size_t len) {
    if (!out || len == 0) return 0; /* nothing to do */
    size_t off = 0;
    while (off < len) {
        ssize_t r = getrandom(out + off, len - off, 0);
        if (r < 0) {
            if (errno == EINTR) continue;      /* interrupted: retry */
            if (errno == ENOSYS) return urandom_fallback(out + off, len - off); /* no getrandom(): fallback */
            return -1;                          /* other error */
        }
        off += (size_t)r;
    }
    return 0;
}


#endif

#ifdef _WIN32
int hdr_gen_random_bytes(uint8_t *out, size_t len) 
{
    if (!out || len == 0) return 0; /* nothing to do */
    NTSTATUS st = BCryptGenRandom(NULL, (PUCHAR)out, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (st == 0) return 0;
    else
        return -1 ;
}

#endif




static int hexval(char c) 
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void crh_to_hex(const uint8_t *in, size_t len, char *out) 
{
    const char *hex = "0123456789abcdef";
    for (size_t i=0;i<len;i++) 
    {
        out[2*i]   = hex[(in[i] >> 4) & 0xF];
        out[2*i+1] = hex[in[i] & 0xF];
    }
    out[2*len] = '\0';
}

static int from_hex(const char *in, unsigned char *out, size_t out_len) 
{
    size_t len = strlen(in);
    if (len != out_len * 2) return -1;
    for (size_t i = 0; i < out_len; i++) 
    {
        int hi = hexval(in[2*i]);
        int lo = hexval(in[2*i + 1]);
        if (hi < 0 || lo < 0) return -1;
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    return 0;
}




/****************/

bool hdr_hashCreateSalt(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
  	if (hdr_inter_check_param_count_error(inter, token, 0) == false) return true;

	*result = hdr_var_create(NULL, "", hvf_temporary, NULL);
	(*result)->type = hvt_integer;

	uint8_t salt[16];
	int res = hdr_gen_random_bytes(salt, 16) ;
    if(res == -1)
    {
	  printf("The createSalt($string:String):String -> hdr_gen_random_bytes failed\n");
      return true ;
    }

	/*convert to hexadecimal string*/
    char * buff = (char*)malloc((16*2)+1) ;
    crh_to_hex(salt,16, buff) ;
    PDX_STRING str = dx_string_create_bU(buff);
    
	*result			= hdr_var_create(str,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;

	return false;
}


bool hdr_hashCreateRandomId(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
  /*
    Returns a string with random hexadecimal characters
  */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
     printf("The system function createRandomId($bytes : Integer):String failed.\n");
     return true ;
   }

   bool type_error = false ;
   DXLONG64 len = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be an Integer.\n");
     goto fail ;
   }

   if(len > 128)
   {
    len = 128 ;
    hdr_inter_print_warning(inter,"The length of the random id that the createRandomId() produces cannot be bigger than 128 bytes (as string is the double). The length was set to 128 bytes.");
   }

   if(len < 2)
   {
    len = 2 ;
    hdr_inter_print_warning(inter,"The length of the random id that the createRandomId() produces cannot be smaller than 2 characters. The length was set to 2 characters.");
   }


   uint8_t *salt = (uint8_t*) malloc(len);
   int res = hdr_gen_random_bytes(salt, len) ;
   if(res == -1)
   {
	  printf("The hdr_gen_random_bytes failed\n");
      return true ;
   }
   
   /*convert to hexadecimal string*/
   char * buff = (char*)malloc((len*2)+1) ;
   crh_to_hex(salt,len, buff) ;
   PDX_STRING str = dx_string_create_bU(buff);
   free(salt);
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_simple_string                  ;  
   hdr_var_set_obj(*result,str)                                ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function createRandomId($bytes : Integer):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_hashHashString(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function hashString($string:String):String failed.\n");
    return true ;
   }
   
   bool type_error = false ;

   PDX_STRING str = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

   /*create a salt*/
    uint8_t salt[16];
	int res = hdr_gen_random_bytes(salt, 16) ;
    if(res == -1)
    {
	  printf("The hashString($string:String):String ->  hdr_gen_random_bytes failed\n");
      return true ;
    }
	/*convert to hexadecimal string*/
    char * buff = (char*)malloc((16*2)+1) ;
    crh_to_hex(salt,16, buff) ;
    PDX_STRING saltstr = dx_string_create_bU(buff);
    /********************/

    /*hash the string*/
    void *work_area = malloc((size_t)16384 * 1024);
    crypto_argon2_config cfg;
    cfg.algorithm = CRYPTO_ARGON2_ID;
    cfg.nb_blocks = 16384;
    cfg.nb_passes = 3;
    cfg.nb_lanes  = 1;

    crypto_argon2_inputs in;
    in.pass      = (uint8_t*)str->stringa;
    in.pass_size = str->bcount;
    in.salt      = salt;
    in.salt_size = 16  ;

    crypto_argon2_extras ex;
    ex.key = NULL;  ex.key_size = 0;
    ex.ad  = NULL;  ex.ad_size  = 0;
    char hash[33] ;
    /* The actual call: */
    crypto_argon2(hash, sizeof hash, work_area, cfg, in, ex);
    free(work_area);
    hash[32] = 0 ;
    /**********************/

    /********** create a record ************/
    PDX_STRING rec     = dx_string_createU(NULL,"$hash=");

    char * buff2 = (char*)malloc((32*2)+1) ;
    crh_to_hex(hash,32, buff2) ;
    PDX_STRING hashstr = dx_string_create_bU(buff2);
    rec = dx_string_concat(rec,hashstr);
    PDX_STRING tmps    = dx_string_createU(NULL,"$salt=");
    rec = dx_string_concat(rec,tmps) ;
    rec = dx_string_concat(rec,saltstr);

    /*free the memory*/
    dx_string_free(saltstr);
    dx_string_free(hashstr);
    dx_string_free(tmps)   ;


	*result			= hdr_var_create(rec,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;
   
    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function hashString($string:String):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_hashHashStringWithSalt(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function hashStringWithSalt($string:String,$salt:String):String failed.\n");
    return true ;
   }
   
   bool type_error = false ;

   PDX_STRING str = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

   PDX_STRING saltstr = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String variable.\n");
     goto fail ;
   }

   if(saltstr->bcount != 32)
   {
      printf("The system function hashStringWithSalt($string:String,$salt:String):String failed. The salt MUST be exactly 16 bytes long\n");
      goto fail ;
   }

   /*create a salt from the hexadecimal*/
    uint8_t salt[16];
	int res = from_hex(saltstr->stringa, (unsigned char *)salt, 16) ;
    if(res == -1)
    {
	  printf("The hashStringWithSalt($string:String,$salt:String):String ->  from_hex failed\n");
      return true ;
    }

    /********************/

    /*hash the string*/
    void *work_area = malloc((size_t)16384 * 1024);
    crypto_argon2_config cfg;
    cfg.algorithm = CRYPTO_ARGON2_ID;
    cfg.nb_blocks = 16384;
    cfg.nb_passes = 3;
    cfg.nb_lanes  = 1;

    crypto_argon2_inputs in;
    in.pass      = (uint8_t*)str->stringa;
    in.pass_size = str->bcount;
    in.salt      = salt;
    in.salt_size = 16  ;

    crypto_argon2_extras ex;
    ex.key = NULL;  ex.key_size = 0;
    ex.ad  = NULL;  ex.ad_size  = 0;
    char hash[33] ;
    /* The actual call: */
    crypto_argon2(hash, sizeof hash, work_area, cfg, in, ex);
    free(work_area);
    hash[32] = 0 ;
    /**********************/

    /****** ************/

    char * buff = (char*)malloc((32*2)+1) ;
    crh_to_hex(hash,32, buff) ;
    PDX_STRING hashstr = dx_string_create_bU(buff);
   
	*result			= hdr_var_create(hashstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;
   
    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function hashStringWithSalt($string:String,$salt:String):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}



