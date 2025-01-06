/*
 Hydra socket support implementation file

 Nikos Mourgis deus-ex.gr 2024

 Live Long and Prosper
*/

/*
 for windows the sockets have to be initialized , the sockets will be initialized
 the first time that a socket is being created and they will be cleaned up after the 
 script's  execution.
*/

/****************************************************************/

#ifdef _WIN32
bool IS_SOCKETS_INIT = false;
WSADATA wsaData;
#endif
bool IS_WOLF_INIT = false ;/*this flag shows if the wolfssl library is initialized or not*/

void _init_socks()
{

    #ifdef _LINUX
     return  ;
    #endif
 
     #ifdef _WIN32
     if (IS_SOCKETS_INIT == true) return ;

     /*initialize the sockets for windows*/
     int res;
     res = WSAStartup(MAKEWORD(2,2), &wsaData);
     if (res != 0) 
     {
        printf("****************** GENERAL SYSTEM ERROR : WSAStartup failed: %d ******************\n", res);
        return ;
     }

     IS_SOCKETS_INIT = true ;
 
     #endif
     return ;
}

bool _init_wolf()
{
     if(IS_WOLF_INIT == true) return true ;
     if(wolfSSL_Init() != SSL_SUCCESS) return false ;
     return true ;
}



/********************* TCP CLIENT **************************************/

bool hdr_domTCPClientSocketCreate(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{ 
    _init_socks();
   /*create and connect a client tcp socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,3) ;
   if(params == NULL)
   {
    printf("The system function Socket.ConnectTCP($host : String,$port : Integer,$error : String):ClientTCPSocket failed.\n");
    return true ;
   }
   
   PSOCK_TCP_CLIENT csock = NULL ;

   bool type_error = false ;
   PDX_STRING host = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     return true ;
   }

   DXLONG64 port = hdr_inter_ret_integer(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be an Integer.\n");
     return true ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[2],&type_error) ; 
   error = dx_string_createU(error,"") ;
   if(type_error == true)
   {
	 printf("The third parameter must be a String.\n");
     return true ;
   }
 
   csock = hdr_sock_client_create() ; 
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_tcp_socket_client ; 
   hdr_var_set_obj(*result,csock)                ;

   /*create the socket*/
    PDX_STRING ip = dxHostNameToIp(host->stringa) ;
	int tmpres;

   if((csock->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
        printf("Error : Socket does not created. Error code : %d.\n",GetLastError()) ;
        error = dx_string_createU(error,"Socket does not created."); 
        hdr_sock_client_free(csock) ;
        hdr_var_set_obj(*result,NULL);
        dx_string_free(ip);
		goto success ;
	}

	/* Set remote->sin_addr.s_addr */
	csock->remote.sin_family = AF_INET;
  	tmpres = inet_pton(AF_INET, ip->stringa, (void *)(&(csock->remote.sin_addr.s_addr)));
    dx_string_free(ip);
  	if( tmpres < 0)
  	{
        printf("Error : Can't set remote->sin_addr.s_addr. Error code : %d.\n",GetLastError()) ;
    	error = dx_string_createU(error,"Can't set remote->sin_addr.s_addr."); 
        hdr_sock_client_free(csock) ;
        hdr_var_set_obj(*result,NULL);
        goto success ;
  	}
	else
     if(tmpres == 0)
  	 {
        printf("Error : The IP is not valid. Error code : %d.\n",GetLastError()) ;
    	error = dx_string_createU(error,"The IP is not valid."); 
        hdr_sock_client_free(csock) ;
        hdr_var_set_obj(*result,NULL);
        goto success ;
  	 }

    csock->remote.sin_port = htons(port);
    /*connect*/
    if(connect(csock->sock, (struct sockaddr *)&csock->remote, sizeof(struct sockaddr)) < 0)
    {
        printf("Error : Unable to connect. Error code : %d\n",GetLastError()) ;
        csock->sock = INVALID_SOCKET ;
        error = dx_string_createU(error,"The IP is not valid."); 
        hdr_sock_client_free(csock) ;
        hdr_var_set_obj(*result,NULL);
        goto success ;
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.ClientTCP($host : String,$port : Integer,$error : String):ClientTCPSocket failed.\n");
    hdr_sock_client_free(csock);
    hdr_sys_func_free_params(params) ;
    return true ;
}
 

bool hdr_domTCPSocketClose(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{ 
   /*close a simple tcp socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Socket.Close() failed.\n");
    return true ;
   }

   PSOCK_TCP_CLIENT tsock = (PSOCK_TCP_CLIENT)for_var->obj ;

#ifdef _WIN32
    closesocket((SOCKET)tsock->sock) ;
#endif

#ifdef _LINUX
   close((int)tsock->sock);
#endif

    tsock->sock = INVALID_SOCKET ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.Close().\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domTCPSocketIsValid(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{ 
   /*return true iof the socket is valid*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Socket.IsValid():Boolean failed.\n");
    return true ;
   }

   *result                = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_bool      ;    
   (*result)->integer     = 0             ;

    if(for_var->obj != NULL) (*result)->integer = 1  ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.IsValid():Boolean.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domTCPSocketSendUTF8(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR *result)
{ 
   /*send data to a tcp socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Socket.SendUTF8($data : String):Integer failed.\n");
    return true ;
   }

    bool type_error = false ;
    DXLONG64 len       = 0    ;
    char * buff      = NULL ;
   
    PDX_STRING str   = NULL ;
    str  = hdr_inter_ret_string(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The first parameter must be a String.\n");
      goto fail ;
    }
    
    len  = str->bcount  ;
    buff = str->stringa ; 

   /*send the data*/
   *result               = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer     ;    
   (*result)->integer     = 0               ;

    PSOCK_TCP_CLIENT tsock = for_var->obj;

    DXLONG64 ret = send((SOCKET)tsock->sock,buff,len,0) ;

    if(ret < 0) (*result)->integer = GetLastError() ;
   
    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.SendUTF8($data : String):Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domTCPSocketSendBytes(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR *result)
{ 
   /*send data to a tcp socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function Socket.Send($data : Bytes, $count : Integer):Integer failed.\n");
    return true ;
   }
   
    bool type_error = false ;
    DXLONG len       = 0    ;
    char * buff      = NULL ;
   
    PHDR_BYTES bytes   = NULL ;
    bytes  = hdr_inter_ret_bytes(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The first parameter must be a [Bytes] object.\n");
      goto fail ;
    }
    
    buff = bytes->bytes  ; 
     
    DXLONG64 cnt = hdr_inter_ret_integer(params->params[1],&type_error)  ;
    if(type_error == true)
    {
	  printf("The second parameter must be an Integer.\n");
      goto fail ;
    }
    
    len = cnt ;
    if (cnt > bytes->length)
    len  = bytes->length ;

   /*send the data*/

   *result               = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer     ;    
   (*result)->integer     = 0               ;

    PSOCK_TCP_CLIENT tsock = for_var->obj;
    DXLONG64 ret = send((SOCKET)tsock->sock,buff,len,0) ;
    if(ret < 0) (*result)->integer = GetLastError() ;
    else
        (*result)->integer = ret ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.Send($data : Bytes, $count : Integer):Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domTCPSocketRecvUTF8(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{ 
   /*receive data from a tcp socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function Socket.ReceiveUTF8($data : String,$error : Integer) failed.\n");
    return true ;
   }
   
   
    PDX_STRINGLIST list = dx_stringlist_create() ;
    if(list == NULL)
    {
     printf("PDX_STRINGLIST creation failed. Memory Error.\n");
     goto fail ;
    }
  

    bool type_error = false ;
   
    PDX_STRING str  = hdr_inter_ret_string(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The first parameter must be a String.\n");
      goto fail ;
    }
      
    PHDR_VAR error = params->params[1] ;
    if((error->type != hvt_integer)&&(error->type != hvt_float))
    {
	  printf("The second parameter must be an Integer.\n");
      goto fail ;
    }

   /*receive the data*/
   /*we will receive until the end*/
    PSOCK_TCP_CLIENT tsock = for_var->obj;
    while(true)
    {
        char buff[4096] ;
        DXLONG64 ret = recv((SOCKET)tsock->sock,buff,4095,0) ; /*4095 because we reserve the last byte for the terminating zero!*/
        if(ret == 0)
        {
            /*connection closed*/
              if(error->type == hvt_integer) error->integer = 0;
              else
                 error->real = 0;

            break ;
        }
        else
            if(ret < 0)
            {
             /*error*/
             if(error->type == hvt_integer) error->integer = GetLastError();
              else
                 error->real = GetLastError();
             
             goto success ;
            }
            else
            {
             /*add the data as a utf8 string*/
             buff[ret] = 0 ;
             PDX_STRING nstr = dx_string_createU(NULL,buff) ;
             dx_stringlist_add_string(list,nstr) ; 
            }
      
    }

    /*all has been received construct the final string and release the lists memory*/
    PDX_STRING nstr = dx_stringlist_raw_text(list) ;
    /*some alchemy to avoid some memory reallocation xD*/
    free(str->stringa) ;
    str->stringa = NULL;
    str->bcount = 0    ;
    str->len    = 0    ;
    dx_string_setU(str,nstr->stringa) ;
    nstr->stringa = NULL ;
    dx_string_free(nstr) ;

    dx_stringlist_free(list) ;
    /*all ok*/

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.ReceiveUTF8($data : String,$error : Integer) failed.\n");
    dx_stringlist_free(list) ;
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domTCPSocketRecv(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR *result)
{ 
   /*receive data from a tcp socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function Socket.Receive($data : Bytes,$error : Integer):Integer failed.\n");
    return true ;
   }
   
    bool type_error = false ;
    DXLONG64 len       = 0    ;
    char * buff      = NULL ;
   
    PHDR_BYTES bytes  = hdr_inter_ret_bytes(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The first parameter must be a [Bytes] object.\n");
      goto fail ;
    }
      
    buff = bytes->bytes  ;
    len  = bytes->length ;

    PHDR_VAR error = params->params[1] ;
    if((error->type != hvt_integer)&&(error->type != hvt_float))
    {
	  printf("The second parameter must be an Integer.\n");
      goto fail ;
    }

   /*receive the data*/

   *result                = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer     ;    
   (*result)->integer     = -1              ;

    /*we will receive the bytes*/
    PSOCK_TCP_CLIENT tsock = for_var->obj;
    DXLONG64 ret = recv((SOCKET)tsock->sock,buff,len,0) ;
    if(ret == 0)
    {
        /*connection closed*/
        if(error->type == hvt_integer) error->integer = 0;
            else
                error->real = 0;
        
       (*result)->integer = 0 ;
       goto success ;
    }
    else
        if(ret < 0)
        {
            /*error*/
            if(error->type == hvt_integer) error->integer = GetLastError();
            else
                error->real = GetLastError();
            
            (*result)->integer = -1 ;
            goto success ;
        }
            

    /*all ok*/
    (*result)->integer = ret ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.Receive($data : Bytes,$error : Integer):Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domTCPSocketFree(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{ 
   /*free the object and uninitialize the variable*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Socket.Free() failed.\n");
    return true ;
   }


    for_var->obj  = hdr_sock_client_free((PSOCK_TCP_CLIENT)for_var->obj) ;
    hdr_var_release_obj(for_var) ;
    for_var->type = hvt_undefined ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.Close() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


/****************** TCP SERVER ******************************************/

bool hdr_domTCPServerSocketCreate(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{ 
    _init_socks();
   /*create and setup a server socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,3) ;
   if(params == NULL)
   {
    printf("The system function Socket.TCPServer($port : Integer,$max_connections : Integer,$error : String):ServerTCPSocket failed.\n");
    return true ;
   }
   
   PSOCK_TCP_SERVER csock = hdr_sock_server_create() ;

   bool type_error = false ;

   DXLONG64 port = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be an Integer.\n");
     goto fail ;
   }

   DXLONG64 max_conn = hdr_inter_ret_integer(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be an Integer.\n");
     goto fail ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[2],&type_error) ; 
   error = dx_string_createU(error,"") ;
   if(type_error == true)
   {
	 printf("The third parameter must be a String.\n");
     goto fail ;
   }
   
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_tcp_socket_server ; 

   hdr_var_set_obj(*result,csock);
   /*create the socket*/

   if((csock->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
        printf("Error : Socket does not created. Error code : %d.\n",GetLastError()) ;
		goto success ;
	}

	csock->remote.sin_family      = AF_INET;
  	csock->remote.sin_addr.s_addr = htonl(INADDR_ANY) ;
    csock->remote.sin_port = htons(port);

    /*bind*/
    if(bind(csock->sock,  (struct sockaddr *)&csock->remote,sizeof(struct sockaddr)) < 0)
    {
        printf("Error : Unable to bind the socket to the port. Error code : %d.\n",GetLastError()) ;
        csock->sock = INVALID_SOCKET ;
        hdr_sock_server_free(csock) ;
        error = dx_string_createU(error,"Unable to bind the socket to the port.") ;
        hdr_var_set_obj(*result,NULL);
        goto success ;
    }

    /*listen*/
    if(listen(csock->sock, max_conn) < 0)
    {
        printf("Error : The listen() function failed. Error code : %d.\n",GetLastError()) ;
        csock->sock = INVALID_SOCKET ;
        hdr_sock_server_free(csock) ;
        error = dx_string_createU(error,"Unable to bind the socket to the port.") ;
        hdr_var_set_obj(*result,NULL);
        goto success ;
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.TCPServer($port : Integer,$max_connections : Integer,$error : String):ServerTCPSocket failed.\n");
    hdr_sock_server_free(csock) ;
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domTCPServerSocketAccept(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var , PHDR_VAR *result)
{ 
   /*accepts a connection and returns a socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Socket.Accept($error : String):TCPSocket failed.\n");
    return true ;
   }
 
   bool type_error = false ;
   PDX_STRING error = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   PSOCK_TCP_INCOMING csock = hdr_sock_client_create() ;
   
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_tcp_socket_client ;  
   hdr_var_set_obj(*result,csock) ;

   /*wait for a connection*/
   int ssize = sizeof(struct sockaddr) ;
   csock->sock = accept(((PSOCK_TCP_SERVER)for_var->obj)->sock, (struct sockaddr*)&csock->remote,&ssize);
   if(csock->sock == INVALID_SOCKET)
	{
	    PDX_STRING ln = dx_IntToStr(GetLastError());
        hdr_inter_hlp_concatenate_and_set(error, (char*)"Error : Accept failed. Error code : ",ln->stringa, (char*)".") ;
        dx_string_free(ln);
		goto success ;
	}

    /*ok we return the socket*/

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.ServerTCP($port : Integer,$max_connections : Integer,$error : String):ServerTCPSocket failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


/************** SSL Support******************************/


bool hdr_domTCPClientSSLCreate(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{ 
    _init_socks();
    if(_init_wolf() == false)
    {
      printf("The WolfSSl library did not initialized.\n");
      return true ;
    }
   /*create and connect a client SSL*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,3) ;
   if(params == NULL)
   {
    printf("The system function SSL.Connect($host : String,$port : Integer,$error : String):ClientSSL failed.\n");
    return true ;
   }

      
   PSOCK_SSL_CLIENT ssl = NULL ;

   bool type_error = false ;
   PDX_STRING host = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     return true ;
   }

   DXLONG64 port = hdr_inter_ret_integer(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be an Integer.\n");
     return true ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[2],&type_error) ; 
   error = dx_string_createU(error,"") ;
   if(type_error == true)
   {
	 printf("The third parameter must be a String.\n");
     return true ;
   }
   
   ssl = hdr_ssl_object_create() ;
   if(ssl == NULL)
   {
	 printf("MEMORY ERROR. The Client SSL object was not created.\n");
     goto fail ;
   }
   
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_ssl_client ; 
    hdr_var_set_obj(*result,ssl) ;

   /*fill the ssl object*/
    PDX_STRING ip = dxHostNameToIp(host->stringa) ;
	int tmpres;

    ssl->method = wolfSSLv23_client_method();
    if(ssl->method == NULL)
    {
      error = dx_string_createU(error, "SSL CLIENT : Could not allocate memory for the ssl->method");
      hdr_ssl_object_free((PSOCK_SSL)(*result)->obj) ;
      hdr_var_set_obj(*result,NULL) ;
      goto  success ;
    }
     /* create a new ssl context */
    if ( (ssl->ctx = wolfSSL_CTX_new(ssl->method)) == NULL)
    {
           error = dx_string_createU(error,"SSL CLIENT : Unable to create a new ssl->context structure.\n");
           hdr_ssl_object_free((PSOCK_SSL)(*result)->obj) ;
           hdr_var_set_obj(*result,NULL) ;
           goto success ;
    }

    wolfSSL_CTX_set_verify(ssl->ctx, WOLFSSL_VERIFY_NONE, NULL);
    ssl->ssl = wolfSSL_new(ssl->ctx);

    if(ssl->ssl == NULL)
    {
      error = dx_string_createU(error,"SSL CLIENT : Unable to create a new SSL object.");
      hdr_ssl_object_free((PSOCK_SSL)(*result)->obj) ;
      hdr_var_set_obj(*result,NULL) ;
      goto success ;
    }

   if((ssl->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
        printf("Error : Socket does not created. Error code : %d",GetLastError());
        error = dx_string_createU(error,"Error : Socket does not created. Error code : %d\n") ;
        dx_string_free(ip);
		hdr_ssl_object_free((PSOCK_SSL)(*result)->obj) ;
        hdr_var_set_obj(*result,NULL) ;
        goto success ;
	}

	/* Set remote->sin_addr.s_addr */
	ssl->remote.sin_family = AF_INET;
  	tmpres = inet_pton(AF_INET, ip->stringa, (void *)(&(ssl->remote.sin_addr.s_addr)));
    dx_string_free(ip);
  	if( tmpres < 0)
  	{
        printf("Error : Can't set remote->sin_addr.s_addr. Error code : %d.\n",GetLastError());
        error = dx_string_createU(error,"Error : Can't set remote->sin_addr.s_addr. Error code : %d") ;
    	hdr_ssl_object_free((PSOCK_SSL)(*result)->obj) ;
        hdr_var_set_obj(*result,NULL) ;
        goto success ;
  	}
	else
     if(tmpres == 0)
  	 {
        printf("Error : The IP is not valid. Error code : %d.\n",GetLastError());
        error = dx_string_createU(error,"Error : The IP is not valid") ;
    	hdr_ssl_object_free((PSOCK_SSL)(*result)->obj) ;
        hdr_var_set_obj(*result,NULL) ;
        goto success ;
  	 }

    ssl->remote.sin_port = htons(port);

     if(connect(ssl->sock, (struct sockaddr *) &ssl->remote, sizeof(struct sockaddr)) < 0)
     {
        printf("Error : Unable to connect. Error code : %d\n",GetLastError());
        error = dx_string_createU(error,"Error : Unable to connect") ;
        ssl->sock = INVALID_SOCKET ;
        hdr_ssl_object_free((PSOCK_SSL)(*result)->obj) ;
        hdr_var_set_obj(*result,NULL) ;
        goto success ;
     }

   
   /*set the socket to the ssl*/ 
    if (wolfSSL_set_fd(ssl->ssl, ssl->sock) != SSL_SUCCESS  ) 
    {
        error = dx_string_createU(error,"The setting of the [fd] for the socket failed.");
        hdr_ssl_object_free((PSOCK_SSL)(*result)->obj) ;
        hdr_var_set_obj(*result,NULL) ;
        goto success  ;
    }
 
    int ret = wolfSSL_connect(ssl->ssl) ;
    if ( ret != SSL_SUCCESS  )
    {
        ret = wolfSSL_get_error(ssl->ssl,ret) ;
        char errc[1024] ;
        wolfSSL_ERR_error_string(ret,errc) ;
        printf("wolfSSL_connect error : %d -> %s\n",ret,errc);
        error = dx_string_createU(error,"wolfSSL_connect error");
        hdr_ssl_object_free((PSOCK_SSL)(*result)->obj) ;
        hdr_var_set_obj(*result,NULL) ;
        goto success ;
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    error = dx_string_createU(error,"The system function SSL.Connect($host : String,$port : Integer,$error : String):ClientSSL failed.");
    hdr_ssl_object_free(ssl) ;
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domSSLClose(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{ 
   /*close a simple tcp socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function SSL.Close() failed.\n");
    return true ;
   }

   PSOCK_SSL ssl = (PSOCK_SSL)for_var->obj ;

  if(ssl->ssl != NULL) 
  {
      wolfSSL_free(ssl->ssl)       ;
      ssl->ssl = NULL ;
  }

    #ifdef _WIN32
        closesocket((SOCKET)ssl->sock) ;
    #endif

    #ifdef _LINUX
       close((int)ssl->sock);
    #endif

    ssl->sock = INVALID_SOCKET ;

  if(ssl->ctx != NULL) 
  {
      wolfSSL_CTX_free(ssl->ctx)   ;
      ssl->ctx = NULL ;
  }
  

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function SSL.Close() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domSSLFree(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{ 
   /*close a simple tcp socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function SSL.Free() failed.\n");
    return true ;
   }


    PSOCK_SSL pssl = for_var->obj ;
    hdr_ssl_object_free(pssl) ;
    for_var->type = hvt_undefined  ;
    hdr_var_release_obj(for_var)   ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function SSL.Free() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domSSLSendUTF8(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR *result)
{ 
   /*send data to a SSL connection*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function SSL.SendUTF8($data : String):Integer failed.\n");
    return true ;
   }

    bool type_error = false ;
    DXLONG64 len       = 0    ;
    char * buff      = NULL ;
   
    PDX_STRING str   = NULL ;
    str  = hdr_inter_ret_string(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The first parameter must be a String.\n");
      goto fail ;
    }
    
    len  = str->bcount  ;
    buff = str->stringa ; 

   /*send the data*/
   *result                = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer     ;    
   (*result)->integer     = -1              ;

   PSOCK_SSL_CLIENT ssl = (PSOCK_SSL_CLIENT)for_var->obj;

    /*send the buffer to the ssl socket*/
    DXLONG64 ret = wolfSSL_write(ssl->ssl,buff,len) ;
    if(ret > 0)
    {
      (*result)->integer     =  ret ;
    } 
    else
        {
            ret = wolfSSL_get_error(ssl->ssl,ret) ;
            (*result)->integer     =  ret ;
        }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.SendUTF8($data : String):Integer  failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domSSLSendBytes(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR *result)
{ 
   /*send data to a tcp socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function Socket.Send($data : Bytes,Count : Integer):Integer failed.\n");
    return true ;
   }
   
    bool type_error = false ;
    DXLONG len       = 0    ;
    char * buff      = NULL ;
   
    PHDR_BYTES bytes   = NULL ;
    bytes  = hdr_inter_ret_bytes(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The first parameter must be a [Bytes] object.\n");
      goto fail ;
    }
    
    buff = bytes->bytes  ; 
     
    DXLONG64 cnt = hdr_inter_ret_integer(params->params[1],&type_error)  ;
    if(type_error == true)
    {
	  printf("The second parameter must be an Integer.\n");
      goto fail ;
    }
    
    len = cnt ;
    if (cnt > bytes->length)
    len  = bytes->length ;

   /*send the data*/

   *result               = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer     ;    
   (*result)->integer     = 0               ;

    PSOCK_SSL_CLIENT ssl = for_var->obj;
 
     /*send the buffer to the ssl socket*/
    DXLONG64 ret = wolfSSL_write(ssl->ssl,buff,len) ;
    if(ret > 0)
    {
      (*result)->integer     =  ret ;
    } 
    else
        {
            ret = wolfSSL_get_error(ssl->ssl,ret) ;
            (*result)->integer     =  ret ;
        }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function SSL.Send($data : Bytes,Count : Integer):Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domSSLRecvUTF8(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{ 
   /*receive data from an SSL connection*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function Socket.ReceiveUTF8($data : String,$error : Integer) failed.\n");
    return true ;
   }
   
   PDX_STRINGLIST list = dx_stringlist_create() ;
   if(list == NULL)
   {
        printf("PDX_STRINGLIST creation failed. Memory Error.\n");
        goto fail ;
   }

    bool type_error = false ;
   
    PDX_STRING str  = hdr_inter_ret_string(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The first parameter must be a String.\n");
      goto fail ;
    }
      
    PHDR_VAR error = params->params[1] ;
    if((error->type != hvt_integer)&&(error->type != hvt_float))
    {
	  printf("The second parameter must be an Integer.\n");
      goto fail ;
    }

   /*receive the data*/
   /*we will receive until the end*/
    PSOCK_SSL_CLIENT ssl = (PSOCK_SSL_CLIENT)for_var->obj;
    while(true)
    {
        char buff[4096] ;
        DXLONG64 ret =  wolfSSL_read(ssl->ssl,buff,4095) ; /*4095 because we reserve the last byte for the terminating zero!*/
        if(ret == 0)
        {
            /*connection closed*/
              if(error->type == hvt_integer) error->integer = 0;
              else
                 error->real = 0;

            break ;
        }
        else
            if(ret < 0)
            {
             /*error*/
             if(error->type == hvt_integer) error->integer = wolfSSL_get_error(ssl->ssl,ret) ;
              else
                 error->real = wolfSSL_get_error(ssl->ssl,ret) ;
             
             goto success ;
            }
            else
            {
             /*add the data as a utf8 string*/
             buff[ret] = 0 ;
             PDX_STRING nstr = dx_string_createU(NULL,buff) ;
             dx_stringlist_add_string(list,nstr) ; 
            }
      
    }

    /*all has been received construct the final string and release the lists memory*/
    PDX_STRING nstr = dx_stringlist_raw_text(list) ;
    /*some alchemy to avoid some memory reallocation xD*/
    free(str->stringa) ;
    str->stringa = NULL;
    str->bcount = 0    ;
    str->len    = 0    ;
    dx_string_setU(str,nstr->stringa) ;
    nstr->stringa = NULL ;
    dx_string_free(nstr) ;

    dx_stringlist_free(list) ;
    /*all ok*/

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.ReceiveUTF8($data : String,$error : Integer) failed.\n");
    dx_stringlist_free(list) ;
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domSSLRecv(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR *result)
{ 
   /*receive data from a tcp socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function SSL.Receive($data : Bytes,$error : Integer):Integer failed.\n");
    return true ;
   }
   
    bool type_error = false ;
    DXLONG64 len       = 0    ;
    char * buff      = NULL ;
   
    PHDR_BYTES bytes  = hdr_inter_ret_bytes(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The first parameter must be a [Bytes] object.\n");
      goto fail ;
    }
      
    buff = bytes->bytes  ;
    len  = bytes->length ;

    PHDR_VAR error = params->params[1] ;
    if((error->type != hvt_integer)&&(error->type != hvt_float))
    {
	  printf("The second parameter must be an Integer.\n");
      goto fail ;
    }

   /*receive the data*/

   *result                = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer     ;    
   (*result)->integer     = -1              ;

    /*we will receive the bytes*/
    PSOCK_SSL_CLIENT ssl = for_var->obj;
    DXLONG64 ret =  wolfSSL_read(ssl->ssl,buff,len) ; 

    if(ret == 0)
    {
        /*connection closed*/
        if(error->type == hvt_integer) error->integer = 0;
            else
                error->real = 0;
        
       (*result)->integer = 0 ;
       goto success ;
    }
    else
        if(ret < 0)
        {
            /*error*/
            if(error->type == hvt_integer) error->integer = wolfSSL_get_error(ssl->ssl,ret) ;
            else
                error->real = wolfSSL_get_error(ssl->ssl,ret) ;
            
            (*result)->integer = -1 ;
            goto success ;
        }
            

    /*all ok*/
    (*result)->integer = ret ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function SSL.Receive($data : Bytes,$error : Integer):Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domSSLIsValid(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{ 
   /*return true iof the socket is valid*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function SSL.IsValid():Boolean failed.\n");
    return true ;
   }

   *result                = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_bool      ;    
   (*result)->integer     = 0             ;

    if( for_var->obj != NULL) (*result)->integer = 1  ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function SSL.IsValid():Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

/******************* SSL SERVER******************************/

bool hdr_domSSLServerCreate(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{ 
    _init_socks();
    if(_init_wolf() == false)
    {
      printf("The WolfSSL library was not initialized.\n");
      return true ;
    }

   /*create and setup a server socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,5) ;
   if(params == NULL)
   {
    printf("The system function SSL.Server($port : Integer,$max_connections : Integer,CERT_FILE : String , KEY_FILE : String,$error : String):SSLServer failed.\n");
    return true ;
   }
   
   PSOCK_SSL_SERVER ssl = hdr_ssl_object_create() ;

   bool type_error = false ;

   DXLONG64 port = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be an Integer.\n");
     goto fail ;
   }

   DXLONG64 max_conn = hdr_inter_ret_integer(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be an Integer.\n");
     goto fail ;
   }

   PDX_STRING CERT = hdr_inter_ret_string(params->params[2],&type_error) ; 
   if(type_error == true)
   {
	 printf("The third parameter must be a String.\n");
     goto fail ;
   }

   PDX_STRING KEY = hdr_inter_ret_string(params->params[3],&type_error) ; 
   if(type_error == true)
   {
	 printf("The fourth parameter must be a String.\n");
     goto fail ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[4],&type_error) ; 
   error = dx_string_createU(error,"") ;
   if(type_error == true)
   {
	 printf("The fifth parameter must be a String.\n");
     goto fail ;
   }
   
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_ssl_server        ; 
   hdr_var_set_obj(*result,ssl)                  ;

    ssl->method = wolfSSLv23_server_method();
    if(ssl->method == NULL)
    {
      error = dx_string_createU(error, "SSL SERVER : Could not allocate memory for the ssl->method");
      goto  success ;
    }
     /* create a new ssl context */
    if ( (ssl->ctx = wolfSSL_CTX_new(ssl->method)) == NULL)
    {
           printf("SSL SERVER : Unable to create a new ssl->context structure.\n");
           error = dx_string_createU(error,"Unable to create a new ssl->context structure.");
           hdr_ssl_object_free(ssl)           ;
           hdr_var_set_obj(*result,NULL)       ;
           
           goto success ;
    }

    if (wolfSSL_CTX_use_certificate_file(ssl->ctx, CERT->stringa, SSL_FILETYPE_PEM)!= SSL_SUCCESS) 
    {
        printf("SSL SERVER : Failed to load %s, please check the path of the file.\n",CERT->stringa);
        error = dx_string_createU(error,"SSL SERVER : Failed to load the SSL certifications. Please check the path of the file.");
        hdr_ssl_object_free(ssl)           ;
        hdr_var_set_obj(*result,NULL)       ;
        goto success ;
    }

     if (wolfSSL_CTX_use_PrivateKey_file(ssl->ctx, KEY->stringa, SSL_FILETYPE_PEM) != SSL_SUCCESS) 
     {
        printf("SSL SERVER : Failed to load %s, please check the path of the file.\n",KEY->stringa);
        error = dx_string_createU(error,"SSL SERVER : Failed to load the SSL KEY FILE, please check the path of the file");
        hdr_ssl_object_free(ssl)           ;
        hdr_var_set_obj(*result,NULL)       ;
        goto success ;
    }

     /*SSL Setup is done !*/

   /*create the socket*/
   if((ssl->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
        printf("Error : Socket does not created. Error code : %d.\n",GetLastError()) ;
		error = dx_string_createU(error,"Socket does not created.");
        hdr_ssl_object_free(ssl)           ;
        hdr_var_set_obj(*result,NULL)       ;
        goto success ;
	}

	ssl->remote.sin_family      = AF_INET;
  	ssl->remote.sin_addr.s_addr = htonl(INADDR_ANY) ;
    ssl->remote.sin_port = htons(port);

    /*bind*/
    if(bind(ssl->sock,  (struct sockaddr *)&ssl->remote,sizeof(struct sockaddr)) < 0)
    {
        printf("Error : Unable to bind the socket to the port. Error code : %d.\n",GetLastError()) ;
        ssl->sock = INVALID_SOCKET ;
        error = dx_string_createU(error,"Unable to bind the socket to the port.");
        hdr_ssl_object_free(ssl)           ;
        hdr_var_set_obj(*result,NULL)       ;
        goto success ;
    }

    /*listen*/
    if(listen(ssl->sock, max_conn) < 0)
    {
        printf("Error : The listen() function failed. Error code : %d.\n",GetLastError()) ;
        ssl->sock = INVALID_SOCKET ;
        error = dx_string_createU(error,"Unable to bind the socket to the port.");
        hdr_ssl_object_free(ssl)           ;
        hdr_var_set_obj(*result,NULL)       ;
        goto success ;
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function SSL.Server($port : Integer,$max_connections : Integer,CERT_FILE : String , KEY_FILE : String,$error : String):SSLServer failed.\n");
    hdr_ssl_object_free(ssl) ;
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domSSLServerAccept(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var , PHDR_VAR *result)
{ 
   /*accepts a connection and returns a socket*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function SSL.Accept($error : String):SSLClient failed.\n");
    return true ;
   }
 
   bool type_error = false ;
   PDX_STRING error = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   PSOCK_SSL_CLIENT ssl = hdr_ssl_object_create() ;
   
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_ssl_client ;  
   (*result)->obj         = (void*)ssl     ;
   hdr_var_set_obj(*result,ssl)           ;

   PSOCK_SSL_SERVER server = (PSOCK_SSL_SERVER)for_var->obj ;

   /*wait for a connection*/
   int ssize = sizeof(struct sockaddr) ;

   ssl->sock = accept(server->sock,(struct sockaddr*)&ssl->remote,&ssize);
   if(ssl->sock == INVALID_SOCKET)
	{
	    PDX_STRING ln = dx_IntToStr(GetLastError());
        hdr_inter_hlp_concatenate_and_set(error, (char*)"Error : Accept failed. Error code : ",ln->stringa, (char*)".") ;
        dx_string_free(ln);
		goto success ;
	}

   /*set the ssl*/   
   if ((ssl->ssl = wolfSSL_new(server->ctx)) == NULL) 
   {
       printf("SSL ACCEPT : failed to create WolfSSL object\n");
       goto success ;
   }

   /* Attach wolfSSL to the socket */
   wolfSSL_set_fd(ssl->ssl, ssl->sock) ;
   
   /*ok we return the socket*/

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Socket.ServerTCP($port : Integer,$max_connections : Integer,$error : String):ServerTCPSocket failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}











