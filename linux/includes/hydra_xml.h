
PHDR_VAR xml_extract_var_from_node(PHDR_VAR node,int position)
{
    PDX_LIST list = (PDX_LIST)node->obj ;
    PDXL_OBJECT obj = dx_list_go_to_pos(list,position)->object ;
    return (PHDR_VAR)obj->obj ;
}

PDXL_OBJECT xml_create_name()
{
 
 PDXL_OBJECT obj = dxl_object_create()     ;
 obj->key = dx_string_createU(NULL,"name");
 obj->obj = (void*)hdr_var_create(dx_string_createU(NULL,""),"",hvf_dynamic,NULL) ;
 ((PHDR_VAR)obj->obj)->type = hvt_simple_string ;

 return obj ; 
}


PDXL_OBJECT xml_create_value()
{
 
 PDXL_OBJECT obj = dxl_object_create()     ;
 obj->key = dx_string_createU(NULL,"value");
 obj->obj = (void*)hdr_var_create(dx_string_createU(NULL,""),"",hvf_dynamic,NULL) ;
 ((PHDR_VAR)obj->obj)->type = hvt_simple_string ;

 return obj ; 
}

PDXL_OBJECT xml_create_properties()
{
  PDX_LIST props = dx_list_create_list() ;
  PDXL_OBJECT obj = dxl_object_create()     ;
  obj->key = dx_string_createU(NULL,"properties");
  obj->obj = (void*)hdr_var_create(props,"",hvf_dynamic,NULL) ;
  ((PHDR_VAR)obj->obj)->type = hvt_list ;
 
  return obj ;
}

PHDR_VAR xml_create_property()
{
  PDX_LIST node = dx_list_create_list() ;
 
  PDXL_OBJECT obj = xml_create_name();
  dx_list_add_node_direct(node,obj) ;

  obj = xml_create_value();
  dx_list_add_node_direct(node,obj) ;

  PHDR_VAR var = (void*)hdr_var_create(node,"",hvf_dynamic,NULL) ;
  var->type = hvt_list ;

  return var ;

}

PDXL_OBJECT xml_create_tags()
{
  PDX_LIST props = dx_list_create_list() ;
  PDXL_OBJECT obj = dxl_object_create()     ;
  obj->key = dx_string_createU(NULL,"tags");
  obj->obj = (void*)hdr_var_create(props,"",hvf_dynamic,NULL) ;
  ((PHDR_VAR)obj->obj)->type = hvt_list ;
 
  return obj ;
}

PHDR_VAR xml_create_node()
{
  PDX_LIST node = dx_list_create_list() ;
 
  PDXL_OBJECT obj = xml_create_name();
  dx_list_add_node_direct(node,obj) ;

  obj = xml_create_value();
  dx_list_add_node_direct(node,obj) ;

  obj = xml_create_properties();
  dx_list_add_node_direct(node,obj) ;

  obj = xml_create_tags();
  dx_list_add_node_direct(node,obj) ;

  PHDR_VAR var = (void*)hdr_var_create(node,"",hvf_dynamic,NULL) ;
  var->type = hvt_list ;

  return var ;

}


bool xml_get_properties(PDX_STRING tag,PHDR_VAR prop,PDX_STRING error)
{
    /*the function does not do any error check, maybe in the next version xD*/
    error = dx_string_createU(error,"");
    char *tag_indx = tag->stringa ; 
    PDX_LIST true_prop = (PDX_LIST)prop->obj ; 
    /*got to the first space if exists */
    dxGoForwardToCh(&tag_indx,' ') ;
    if((*tag_indx == '/')||(*tag_indx == '>')) 
    {
        return true ; /*no properties here*/
    }
    /*go to the first character after the first space or tab*/
    dxGoForwardWhileChars(&tag_indx,"\r\n\t ");
   
    if((*tag_indx=='/')||(*tag_indx=='>'))
    {
     /*no properties here*/
     return true ;
    }

    /*copy properties line*/
    char *properties = dxCopyStrToChars(&tag_indx,"/>","\"'");
    if(properties == NULL)
    {
     /*not a fatal error*/
     return true;
    }

    PDX_STRINGLIST parts = dx_stringlist_create() ;
    PDX_STRING propl = dx_string_create_bU(properties) ;
    PDX_STRING sep1 = dx_string_createU(NULL," ") ;
    PDX_STRING sep2 = dx_string_createU(NULL,"=") ;
    ExplodeEx(parts,propl,sep1) ;

    PDXL_NODE node = parts->start;
    while(node != NULL)
    {
      PDX_STRING str = node->object->key ;
      /*explode the str based to the [=] */
      PDX_STRINGLIST prop_parts = dx_stringlist_create() ;
      ExplodeEx(prop_parts,str,sep2) ;

      if(prop_parts->count == 0)
      {
        dx_stringlist_free(prop_parts) ;
        continue ;
      }

      /*check if the value is encapsulated in quotes, thats the only syntax that we check for errors for now*/
      PDX_STRING prop_str_val = NULL ;
      if(prop_parts->start->right == NULL)
      {
         hdr_inter_hlp_concatenate_and_set(error,"No value for the property. Property : [",prop_parts->start->object->key->stringa,"]") ;
         return false ;
      }
      else
      {
          prop_str_val = prop_parts->start->right->object->key ;
          
          if(dxIsTextAString(prop_str_val->stringa,'"') == false)
          {
             hdr_inter_hlp_concatenate_and_set(error,"The value of the property MUST be encapsulated in '\"'. Property : [",prop_parts->start->object->key->stringa,"]") ;
             return false ;
          }
      }

      /*
        remove the "" for storing the true value in the structure 
      */
      DXLONG64 indx = 1 ;
      PDX_STRING calbr_val = CopyIndxToChar(prop_str_val,'"',&indx);
      
      /*ok we have the property , add it to the properties */
      PHDR_VAR var = xml_create_property() ;
      PDX_LIST tprop = (PDX_LIST)var->obj ;
      PDXL_OBJECT obj = dx_list_go_to_pos(tprop,0)->object ;
      PHDR_VAR ppart = (PHDR_VAR)obj->obj ; 
      PDX_STRING pstr = (PDX_STRING)ppart->obj ;
      PDX_STRING expl_part = prop_parts->start->object->key ;
      pstr = dx_string_createU(pstr,expl_part->stringa) ;

      if(prop_parts->count > 1)
      {

        obj = dx_list_go_to_pos(tprop,1)->object ;
        ppart = (PHDR_VAR)obj->obj ; 
        pstr = (PDX_STRING)ppart->obj ;
        pstr = dx_string_createU(pstr,calbr_val->stringa) ;
        dx_string_free(calbr_val);
      }

      /*name and value is set add it to the properties*/
      obj = dxl_object_create() ;
      obj->obj = var ;
      dx_list_add_node_direct(true_prop,obj) ;

      /*clean up memory*/
      dx_stringlist_free(prop_parts) ;

      node = node->right ;
    }

    dx_string_free(sep1) ;
    dx_string_free(sep2) ;

    /*clean up memory*/
    dx_stringlist_free(parts);
    dx_string_free(propl);

  
  return true ;
}

bool xml_isCDATA(char **str_indx)
{
  char *cdata ="<![CDATA[" ;
  char *indx = *str_indx   ;
  for(int i=0;i<9;i++)
  {
    if(*indx == 0) return false ;
    char cc = *indx ;
    if(cc != *cdata) return false ;

    cdata++ ;
    indx++  ;
  }

  *str_indx = indx ;
 return true ;
}

PDX_STRING xml_retrieve_cdata_value(char **str_indx)
{
   /*find the ]]> and retrieve all the chars*/
  char *str = "]]>" ;

  DXLONG64 pos = 0 ;
  PDX_STRING tstr = dx_string_create_bU(*str_indx) ;
  char * cpos = dx_binary_find_word(tstr , &pos , str);
  tstr->stringa = NULL ;
  dx_string_free(tstr);
  if(cpos == NULL) return NULL ; /*error not found the closing*/

  if(pos == 0)
  {
   /*empty value*/
   return dx_string_createU(NULL,"") ;
  }

  /*copy until the pos*/

  char *val = (char*)malloc(pos+2);/*+1 because malloc needs byte count and +1 for the terminating zero*/ 
  val[pos+1] = 0 ; /*pre terminate*/
  memcpy(val,*str_indx,pos+1) ;

  /*go to the next character*/
  (*str_indx) = (*str_indx) + (pos+1) ;
  return dx_string_create_bU(val) ;
}


bool xml_correct_closing(char **str_indx,char* tag_name)
{
     /*check if the tag is closing properly*/
    dxGoForwardWhileChars(str_indx,"\r\n\t ");
    char *strindx = *str_indx ;
    if((*strindx != '<')||(*(strindx+1) != '/'))
    {
      return false;
    }
    
    /*get the tag's closing name*/
    (*str_indx) = (*str_indx) + 2 ; 
    char *name = dxCopyStrToChar(str_indx,'>',"") ;
    if(name == NULL) return false ;

    if(*(*str_indx) != '>' ) 
    {
        free(name);
        return false ;
    }

    (*str_indx)++ ;/*to the next character*/
   
    if(dxIsStrEqual(name,tag_name) == false)
    {
      free(name);
      return false ;
    }

    free(name);
    return true ; /*all ok*/

}


bool xml_parse(PDX_LIST parent,char **str_indx,PDX_STRING error)
{
  /*
    parse the current tag 
  */ 

  while(true)
  {
      if(*(*str_indx) == 0) return true ; /*end of string*/
      char *strindx = *str_indx ;

      if(*strindx != '<')
      {
        hdr_inter_hlp_concatenate_and_set(error,"The '<' for the tag is missing. String : ",strindx,"") ;
        return false ;
      }
      /*check if its the end of some parent tag*/
      if(*(strindx+1) == '/')
      {
        return true ;
      }

      /*copy the tag from < to >*/
      char* rtag = dxCopyStrToChar(str_indx,'>', "\"'");
      /*go to the next character*/
      if(rtag == NULL) 
      {
        hdr_inter_hlp_concatenate_and_set(error,"The tag was not returned. dxCopyStrToChar failed. String : ",strindx,"") ;
        return false ;
      }

      PDX_STRING tag = dx_string_create_bU(rtag) ; 
      /*
       create a tag node 
       0 name
       1 value
       2 properties
       3 tags
      */
      PHDR_VAR node  = xml_create_node() ;
      PHDR_VAR name  = xml_extract_var_from_node(node,0) ;
      PHDR_VAR value = xml_extract_var_from_node(node,1) ;
      PHDR_VAR prop  = xml_extract_var_from_node(node,2) ;
      PHDR_VAR tags  = xml_extract_var_from_node(node,3) ;


      if(tag->len < 2)
      {
        hdr_inter_hlp_concatenate_and_set(error,"The tag is very small. Tag : ",rtag,"") ;
        goto fail ;
      }
  
      /*check if we do find a closing [>]*/
      if(*(*str_indx) != '>')
      {
        hdr_inter_hlp_concatenate_and_set(error,"No closing [>] character was found. Tag : ",rtag,"") ;
        goto fail ;
      }

       (*str_indx)++ ;
      /*get the properties if exists*/
       if(xml_get_properties(tag,prop,error) == false)
        {
          hdr_inter_hlp_concatenate(error," -> Error in the tags properties. Tag : ",rtag,"") ;
          goto fail ;
        }

        /*add the name*/
        char *tg_indx = tag->stringa ;
        tg_indx++; /*omit the < */
        char *sname    = dxCopyStrToChars(&tg_indx," />","\"'") ;
        if(sname == NULL) 
        {
            hdr_inter_hlp_concatenate_and_set(error,"Error in the tag name extraction. dxCopyStrToChars failed. Tag : ",rtag,"") ;
            goto fail ;
        }
        dx_string_free((PDX_STRING)name->obj) ;
        name->obj = (void*)dx_string_create_bU(sname);


      /*check if the tag is self closing*/
      if(tag->stringa[tag->bcount-1] == '/')
      {
        /*nothing more to do*/
        goto success ;      
      }

      /*
        the tag has a value OR other tags. check first is a CDATA section
        <![CDATA[]]>
      */
  
      /*ignore special characters*/
      dxGoForwardWhileChars(str_indx,"\r\n\t ");

      if(xml_isCDATA(str_indx)==true)
      {
        /*copy the cdata section as is, the str_indx points in the first character to copy*/
        if(*str_indx == 0)
        {
      
            hdr_inter_hlp_concatenate_and_set(error,"The <![CDATA[]]> section is malformed.End of string. Tag : ",rtag,"") ;
            goto fail ;
    
        }

        PDX_STRING val = xml_retrieve_cdata_value(str_indx) ;
        if(val == NULL) 
        {
            hdr_inter_hlp_concatenate_and_set(error,"The <![CDATA[]]> section is malformed.The ']]>' closing was not found. Tag : ",rtag,"") ;
            goto fail ;
        }

         /*add the value*/
         value->obj = (void*)dx_string_free((PDX_STRING)value->obj);
         value->obj = (void*)val;
      
         if(xml_correct_closing(str_indx,sname)==false)
         {
            hdr_inter_hlp_concatenate_and_set(error,"The tag closing section is malformed. [</]. Tag : ",rtag,"") ;
            goto fail ;
         }


        /*nothing more to do*/
        goto success ;
      }
 
      /*check if the tag has children or a single value*/
      /*go to the next < in the text , if its followed by a / then the text is value else has children*/
      strindx = *str_indx ;
     if(dxCharExists(&strindx,'<',"") == false) 
     {
        hdr_inter_hlp_concatenate_and_set(error,"The tag does not contain any child or the closing element. Tag : ",rtag,"") ;
        goto fail ;
     }

     /*ok check if the '<' is for closing the tag*/
     if(*(strindx+1) == '/')
     {

         /*it seems its a value , copy it*/
         char * val = dxCopyStrToChar(str_indx,'<',""); 
         if(val == NULL) 
         {
                hdr_inter_hlp_concatenate_and_set(error,"The value for the tag was not retrieved. Tag : ",rtag,"") ;
                goto fail ;
         }

         /*add the value*/
         value->obj = (void*)dx_string_free((PDX_STRING)value->obj);
         value->obj = (void*)dx_string_create_bU(val);
     
         /*check for the validity of the closing tag*/
         if(xml_correct_closing(str_indx,sname)==false)
         {
          hdr_inter_hlp_concatenate_and_set(error,"The tag closing section is malformed. [</]. Tag : ",rtag,"") ;
          goto fail ;
         }

        goto success ; 

     }


     /*the tag is a parent of children*/

     PDX_LIST node_list = (PDX_LIST)tags->obj ;
     if(xml_parse(node_list,str_indx,error) == false) goto fail ;

    /*check for the validity of the closing tag*/
     if(xml_correct_closing(str_indx,sname)==false)
     {
       hdr_inter_hlp_concatenate_and_set(error,"The tag closing section is malformed. [</]. Tag : ",rtag,"") ;
       goto fail ;
     }


      success: 
      dx_string_free(tag);
      /*add the node in the parent*/
      PDXL_OBJECT obj = dxl_object_create();
      obj->key = dx_string_createU(NULL,sname) ;
      obj->obj = node ;
      dx_list_add_node_direct(parent,obj) ;
      /*ignore special characters an go to the next token*/
      dxGoForwardWhileChars(str_indx,"\r\n\t ");
      continue ;
      fail :
      dx_string_free(tag) ;
      hdr_var_free(node)  ;
      return false ;

  }
 
  return true ;

}



PHDR_VAR hdrParseXmlString(PDX_STRING xml,PDX_STRING error)
{
 
 if(xml == NULL) return NULL ;
 error = dx_string_createU(error,"");
 char *str_indx = xml->stringa ; 
 
 /*
  The xml tag has the format of <tag>value</tag> or <tag><![CDATA[some stuff]]></tag> in simple form.
  In the empty form the xml string has the format of <tag\> this is empty string
  In the more advanced form the xml tag can have properties like <tag prop1="hello"> or <tag prop1='hello'>
  This function parses the xml string a simple as it gets 
 */
 
 /*trim the non printable characters*/
 dxGoForwardWhileChars(&str_indx,"\r\n\t ");

 PDX_LIST header = dx_list_create_list();

 /*check if the xml has a header to handle and do some alchemy to avoid to write more code  xD */
 if((*str_indx=='<')&&(*(str_indx+1)=='?'))
 {
   char *hdr = dxCopyStrToChar(&str_indx,'\r\n',"\"'") ;
   hdr[0] = ' ';
   hdr[1] = '<';
   int strl = strlen(hdr) ;
   hdr[strl-2] = '/' ;

   char *strindx = hdr ; 
    /*trim the non printable characters*/
   dxGoForwardWhileChars(&strindx,"\r\n\t ");
   if(xml_parse(header,&strindx,error) == false) 
   {
     PHDR_VAR var = hdr_var_create(header,"",hvf_temporary_ref,NULL);
     var->type = hvt_list ;
     return var ;
   }

 }

 /*add the header to the list as the first element in the xml list*/
 PDX_LIST root = dx_list_create_list() ;
 PHDR_VAR var = hdr_var_create(root,"",hvf_temporary_ref,NULL);
 var->type = hvt_list ;

 PDXL_OBJECT obj = dxl_object_create() ;
 obj->key = dx_string_createU(NULL,"header") ;

 PHDR_VAR tvar = (void*)hdr_var_create(header,"",hvf_dynamic,NULL) ;
 tvar->type = hvt_list ;

 obj->obj   = (void*)tvar     ;
 dx_list_add_node_direct(root,obj) ;

 dxGoForwardWhileChars(&str_indx,"\r\n\t ");
 if(xml_parse(root,&str_indx,error) == false) 
 {
   hdr_var_free(var) ;
   root = dx_list_create_list() ;
   var  = hdr_var_create(root,"",hvf_temporary_ref,NULL);
   var->type = hvt_list ;
   return var ;
 }

 return var ;
}



/*----------------- list as xml string ---------------------*/

PDX_STRING xml_str_get_name(PDX_LIST tag_node)
{
 /*the list has 4 variables. The name,value,properties and tags , we return the name*/

 PDX_LIST tag = tag_node;
 PDX_STRING name_token = dx_string_createU(NULL,"name") ; 
 PDXL_NODE node = tag->start ;
 while(node != NULL)
 {
   PDXL_OBJECT obj = node->object ;
   
   if(dx_string_native_compare(obj->key,name_token) == dx_equal) 
    {
        dx_string_free(name_token);
        PHDR_VAR var = obj->obj ; 
        if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
        {
             PDX_STRING name = (PDX_STRING)var->obj ;
             if(name == NULL)
             {
               return NULL ; 
             }
             
             if(name->len == 0) return NULL ;
              else
                  return name ;
             
        }
        else 
            return NULL ;
    }   

   node = node->right ; 
 }

 dx_string_free(name_token);
 return NULL ;
}

PDX_STRING xml_str_get_value(PDX_LIST tag_node)
{
 /*the list has 4 variables. The name,value,properties and tags , we return the value*/

 PDX_LIST tag = tag_node;
 PDX_STRING name_token = dx_string_createU(NULL,"value") ; 
 PDXL_NODE node = tag->start ;
 while(node != NULL)
 {
   PDXL_OBJECT obj = node->object ;
   
   if(dx_string_native_compare(obj->key,name_token) == dx_equal) 
    {
        dx_string_free(name_token);
        PHDR_VAR var = obj->obj ; 
        if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
        {
             PDX_STRING name = (PDX_STRING)var->obj ;
             if(name == NULL)
             {
               return NULL ; 
             }
             
             if(name->len == 0) return NULL ;
              else
                  return name ;
             
        }
        else 
            return NULL ;
    }   

   node = node->right ; 
 }

 dx_string_free(name_token);
 return NULL ;
}


void xml_str_add_prop(PDX_STRING tag_name,PDX_LIST prop) 
{
  PDXL_NODE node = prop->start ;
  while(node != NULL)
  {
    PDXL_OBJECT obj  = node->object ;
    PHDR_VAR    var  = (PHDR_VAR)obj->obj ;
    if(var->type != hvt_list)
    {
      /*ommit this is not valid*/
      node = node->right ;
      continue ;
    }

    PDX_LIST    prop = var->obj ;
    /*extract the name and value string , we can use the same functions that we use for the tags */
    PDX_STRING name  = xml_str_get_name(prop)  ;
    PDX_STRING value = xml_str_get_value(prop) ;

    if(name == NULL)
    {
      /*ommit this is not valid*/
      node = node->right ;
      continue ;
    }

    if(name->len == 0)
    {
      node = node->right ;
      continue ;
    }

    /* add the name */
    hdr_inter_hlp_concatenate(tag_name," ",name->stringa,"=\"") ;
    /* add the value */
    if(value == NULL)
    hdr_inter_hlp_concatenate(tag_name,"\"","","") ;
    else
        hdr_inter_hlp_concatenate(tag_name,value->stringa,"\"","") ;

    node =node->right ;
  }

}


void xml_str_add_tag_name(PDX_STRINGLIST xmlstr, PDX_STRING name,PDX_LIST prop)
{
 
  PDX_STRING temp_str = dx_string_createU(NULL,"<")          ;
  PDX_STRING tag_name = dx_string_concat(temp_str,name)     ;

  /*
    check if there is properties
  */

  if(prop != NULL)
  {
    if(prop->count > 0)
    {
      /*add a space*/
      hdr_inter_hlp_concatenate(tag_name," ","","") ;
      /*add the properties*/
      xml_str_add_prop(tag_name,prop) ;
    }
  }

  hdr_inter_hlp_concatenate(tag_name,">","","") ;
  dx_stringlist_add_string(xmlstr,tag_name) ;
  dx_string_free(temp_str) ;
}

void xml_str_add_tag_closing(PDX_STRINGLIST xmlstr, PDX_STRING name)
{
 
  PDX_STRING temp_str = dx_string_createU(NULL,"</")          ;
  PDX_STRING tag_name = dx_string_concat(temp_str,name)     ;
  hdr_inter_hlp_concatenate(tag_name,">\r\n","","") ;
  dx_stringlist_add_string(xmlstr,tag_name) ;
  dx_string_free(temp_str) ;
}

PDX_LIST xml_str_get_properties(PDX_LIST tag_node)
{
 /*the list has 4 variables. The name,value,properties and tags , we return the properties*/

 PDX_LIST tag = tag_node;
 PDX_STRING name_token = dx_string_createU(NULL,"properties") ; 
 PDXL_NODE node = tag->start ;
 while(node != NULL)
 {
   PDXL_OBJECT obj = node->object ;
   if(dx_string_native_compare(obj->key,name_token) == dx_equal) 
   {
        dx_string_free(name_token);
        PHDR_VAR var = obj->obj ; 
        if(var->type == hvt_list) return (PDX_LIST)var->obj ;
        else
            return NULL ;
   }  
  
   node = node->right ; 
 }

 dx_string_free(name_token);
 return NULL ;

}


PDX_LIST xml_str_get_tags(PDX_LIST tag_node)
{
     /*the list has 4 variables. The name,value,properties and tags , we return the tags*/
     PDX_LIST tag = tag_node;
     PDX_STRING name_token = dx_string_createU(NULL,"tags") ; 
     PDXL_NODE node = tag->start ;
     while(node != NULL)
     {
       PDXL_OBJECT obj = node->object ;
       if(dx_string_native_compare(obj->key,name_token) == dx_equal) 
       {
            dx_string_free(name_token);
            PHDR_VAR var = obj->obj ; 
            if(var->type == hvt_list) return (PDX_LIST)var->obj ;
            else
                return NULL ;
       }  
  
       node = node->right ; 
     }

     dx_string_free(name_token);
     return NULL ;

}


bool xml_convert_to_str(PDX_STRINGLIST xmlstr, PDX_LIST root, PDX_STRING error)
{
  /*add to the string list the appropriate string tags and values*/
  /*get the name*/

  PDX_STRING name = xml_str_get_name(root) ;
  if(name == NULL)
  {
    hdr_inter_hlp_concatenate_and_set(error,"An xml tag does not have a name.  : ","","") ; 
    return false ;
  }

   /* get the properties list */
  PDX_LIST prop = xml_str_get_properties(root);
  xml_str_add_tag_name(xmlstr,name,prop) ;

  /*set the value*/
  PDX_STRING val = xml_str_get_value(root) ;
  if(val != NULL)
  {
    if(val->len > 0)
    {
      dx_stringlist_add_raw_string(xmlstr, val->stringa) ; /*pass a copy of the string so in the destruction of the list the memory will not be invalidated*/
    }
   
  }
  /*
    get the tags
  */

  PDX_LIST tags = xml_str_get_tags(root) ;
  if(tags != NULL)
  {
    if(tags->count > 0)
    {
      /*add a line to the stringlist for better printing (beautifying)*/
      dx_stringlist_add_raw_string(xmlstr,"") ;
      PDXL_NODE node = tags->start ; 
      while(node != NULL)
      {
        PDXL_OBJECT obj = node->object ;
        PHDR_VAR    var = obj->obj     ;
        if(var->type != hvt_list)
        {
          node = node->right ;
          continue ;
        }
        PDX_LIST    tag = (PDX_LIST)var->obj ;

        if (xml_convert_to_str(xmlstr, tag, error) == false) return false ;   
        node =node->right ;
      }
    }
   
  }

  xml_str_add_tag_closing(xmlstr,name) ;
  return true ;
}


PHDR_VAR hdrListToXmlStr(PDX_LIST root,PDX_STRING error)
{
    /*
      create an xml formated string from a properly prepared list
      the list is the root. Every node have 
      as PHDR_VAR the following elements 
      name  : string 
      value : string
      prop  : list of lists with name,value
      tags  : list of list with all this nodes
    */

   
    PHDR_VAR var = hdr_var_create(dx_string_createU(NULL,""),"",hvf_temporary_ref,NULL);
    var->type = hvt_simple_string ;

    if(root == NULL)        return var ;
    if(root->start == NULL) return var ;

    PDX_STRINGLIST xmlstr = dx_stringlist_create() ;
    if(root->start == NULL)
    {
      error = dx_string_createU(error,"The list that was supplied is empty.") ;
      return var ;
    }

    PHDR_VAR tvar = NULL ;
    /*check if there is a header*/
    if(root->start->right != NULL)
    {
      tvar  = root->start->right->object->obj ;
    }
    else
    {
      tvar  = root->start->object->obj ;
    }
    PDX_LIST tag  = (PDX_LIST)tvar->obj       ;
    if(xml_convert_to_str(xmlstr,tag,error) == false) 
    {
      dx_stringlist_free(xmlstr) ;
      return var ;
    }

    /*all ok*/
    hdr_var_free(var) ;
    PDX_STRING str = dx_stringlist_raw_text(xmlstr) ;
    var  = hdr_var_create(str,"",hvf_temporary_ref,NULL);
    var->type = hvt_simple_string ;
    dx_stringlist_free(xmlstr) ;

    return var ;

}

















