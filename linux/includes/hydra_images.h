#define STBIW_WINDOWS_UTF8 /*we need the path names as utf8*/
#define STB_IMAGE_IMPLEMENTATION
#include <thirdparty/stbimages/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <thirdparty/stbimages/stb_image_write.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <thirdparty/stbimages/stb_image_resize2.h>




bool hdr_imgConvertImage(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{

    /*
      open an image and convert it to the specified format.
      the function gets as parameters a string for the file path, a string to set the new format and
      the filename for the new file.
      supported strings (formats) : png , bmp , tga , jpg
      
    */

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,3) ;
   if(params == NULL)
   {
    printf("The system function convertImage($fileName:String,$asFormat:String[png,bmp,tga,jpg],$outFile:String):Boolean failed.\n");
    return true ;
   }
   
   bool type_error = false ;

   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

   PDX_STRING oformat = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String variable.\n");
     goto fail ;
   }

   PDX_STRING format = LowerCase(oformat) ;

   PDX_STRING outf = hdr_inter_ret_string(params->params[2],&type_error) ; 
   if(type_error == true)
   {
	 printf("The third parameter must be a String variable.\n");
     goto fail ;
   }

    bool ret = false ;
    int width, height, channels;
    unsigned char *img = stbi_load(fname->stringa, &width, &height, &channels, 0);
    if(img == NULL) 
    {
        hdr_inter_print_warning(inter,"The ")              ;
        hdr_inter_print_warning(inter,fname->stringa)      ;
        hdr_inter_print_warning(inter," was not loaded") ;
    }
    else
    { 
        /*the image was opened, try to convert it*/
        if(hdr_inter_fast_str(format, "png", 3) == true)
        {
          if (stbi_write_png(outf->stringa,width,height,channels,img,width * channels) != 0) ret = true ;
        }
        else
        if(hdr_inter_fast_str(format, "bmp", 3) == true)
        {
          if(stbi_write_bmp(outf->stringa,width,height,channels,img) != 0) ret = true ;
        }
        else
        if(hdr_inter_fast_str(format, "tga", 3) == true)
        {
          if(stbi_write_tga(outf->stringa,width,height,channels,img) != 0) ret = true ;
        }
        else
        if(hdr_inter_fast_str(format, "jpg", 3) == true)
        {
          /*we will save the jpg always in 100% quality*/ 
          if(stbi_write_jpg(outf->stringa,width,height,channels,img,100) != 0 ) ret = true;
        }
        else
        {
         hdr_inter_print_warning(inter,"The format [")  ;
         hdr_inter_print_warning(inter,format->stringa) ;
         hdr_inter_print_warning(inter,"] is not supported") ;
        }

        stbi_image_free(img);/*release the memory*/
    }

   dx_string_free(format);
   *result           = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = hdr_inter_bool_to_int(ret)      ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function convertImage($fileName:String,$asFormat:String[png,bmp,tga,jpg],$outFile:String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}



bool hdr_imgImageInfo(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,3) ;
   if(params == NULL)
   {
    printf("The system function imageInfo($filename:String,out->$width:Integer,out->$height:Integer):Boolean failed.\n");
    return true ;
   }
   
   bool type_error = false ;

   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

   DXLONG64 width = hdr_inter_ret_integer(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be an Integer variable.\n");
     goto fail ;
   }

   DXLONG64 height = hdr_inter_ret_integer(params->params[2],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be an Integer variable.\n");
     goto fail ;
   }

    bool ret = false ;  
    DXLONG64 ch = 0 ;
    width  = 0 ;
    height = 0 ;
    if(stbi_info(fname->stringa, &width, &height, &ch) != 0 ) ret = true ;

    /*set the parameters with the right values*/
    if(params->params[1]->type == hvt_integer) params->params[1]->integer = width ;
    else
        params->params[1]->real = (double)width ;

    if(params->params[2]->type == hvt_integer) params->params[2]->integer = height ;
    else
        params->params[2]->real = (double)height ;
   


    *result           = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = hdr_inter_bool_to_int(ret)      ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function imageInfo($filename:String,out->$width:Integer,out->$height:Integer):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_imgResizeImage(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{

    /*
      open an image and resize it to the specified dimensions.
      the function gets as parameters a string for the file path, a new width, a new height and
      the filename for the new file.  

      we support resizing only for jpg,png,tga,bmp

    */

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,4) ;
   if(params == NULL)
   {
    printf("The system function resizeImage($fileName:String,$width:Integer,$height:Integer,$outFile:String):Boolean failed.\n");
    return true ;
   }
   
   bool type_error = false ;

   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

   DXLONG64 nwidth = hdr_inter_ret_integer(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be an Integer variable.\n");
     goto fail ;
   }

   DXLONG64 nheight = hdr_inter_ret_integer(params->params[2],&type_error) ; 
   if(type_error == true)
   {
	 printf("The third parameter must be an Integer variable.\n");
     goto fail ;
   }

   PDX_STRING outf = hdr_inter_ret_string(params->params[3],&type_error) ; 
   if(type_error == true)
   {
	 printf("The fourth parameter must be a String variable.\n");
     goto fail ;
   }


   /*get the extension of the file*/
   char *src_indx = fname->stringa ; 
   char* ext = dxCopyStrToCharReverse(&src_indx,'.',"")  ; 
   PDX_STRING otype = dx_string_create_bU(ext) ;
   PDX_STRING itype = LowerCase(otype);

    bool ret = false ;
    int width, height, channels;
    unsigned char *img = stbi_load(fname->stringa, &width, &height, &channels, 0);
    if(img == NULL) 
    {
        hdr_inter_print_warning(inter,"The ")              ;
        hdr_inter_print_warning(inter,fname->stringa)      ;
        hdr_inter_print_warning(inter," was not loaded") ;
    }
    else
    { 
        /*the image was opened, try to resize it*/
        int res_im_buff = nwidth * nheight * channels;
        char *img_buffer = malloc(res_im_buff);
        if(img_buffer == NULL)
        {
          printf("MALLOC() failed. See the following error to pinpoint where ->\n");
          stbi_image_free(img);/*release the memory*/
          dx_string_free(itype);
          dx_string_free(otype);
          goto fail ;
        }

        /*fill the new buffer*/
        int ret_s = stbir_resize_uint8_linear(img,width,height, 0, img_buffer, nwidth, nheight, 0,channels);

        if(ret_s != 0)
        {
            if(hdr_inter_fast_str(itype, "png", 3) == true)
            {
              if (stbi_write_png(outf->stringa,nwidth,nheight,channels,img_buffer,nwidth * channels) != 0) ret = true ;
            }
            else
            if(hdr_inter_fast_str(itype, "bmp", 3) == true)
            {
              if(stbi_write_bmp(outf->stringa,nwidth,nheight,channels,img_buffer) != 0) ret = true ;
            }
            else
            if(hdr_inter_fast_str(itype, "tga", 3) == true)
            {
              if(stbi_write_tga(outf->stringa,nwidth,nheight,channels,img_buffer) != 0) ret = true ;
            }
            else
            if((hdr_inter_fast_str(itype, "jpg", 3) == true)||(hdr_inter_fast_str(itype, "jpeg", 4) == true))
            {
              /*we will save the jpg always in 100% quality*/ 
              if(stbi_write_jpg(outf->stringa,nwidth,nheight,channels,img_buffer,100) != 0 ) ret = true;
            }
            else
            {
             hdr_inter_print_warning(inter,"The image type [")  ;
             hdr_inter_print_warning(inter,ext) ;
             hdr_inter_print_warning(inter,"] is not supported") ;
            }
        }


        free(img_buffer);
        stbi_image_free(img);/*release the memory*/
    }

   dx_string_free(itype);
   dx_string_free(otype);
   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = hdr_inter_bool_to_int(ret)      ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function resizeImage($fileName:String,$width:Integer,$height:Integer,$outFile:String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}