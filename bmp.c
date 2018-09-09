#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

char*  _error(const char* s){
	
	char *error = malloc(sizeof(*error)* (strlen(s) + 1));
	strcpy(error, s);
	return error;

}

bool check_bmp_header(BMPHeader* bmp_header, FILE* fp) {

	if(bmp_header->type != 0x4d42){
		return false;
	}
	fseek(fp,0,SEEK_END);
	if(bmp_header->size != ftell(fp)){
		return false;
	}
	if(bmp_header->offset != 0x36){
		return false;
	}
	if(bmp_header->dib_header_size != 0x28){
		return false;
	}
	if(bmp_header->num_planes != 0x01){
		return false;
	}
	if(bmp_header->compression != 0){
		return false;
	}
	if(bmp_header->important_colors != 0 || bmp_header->num_colors != 0){
		return false;
	}
	if(bmp_header->bits_per_pixel != 0x0018){
		return false;
	}
	
	if(bmp_header->image_size_bytes != (bmp_header->width_px * 3 + (bmp_header->width_px%4))*bmp_header->height_px) {
		return false;
	}
	
	return true;
}

BMPImage* read_bmp(FILE* fp, char** error) {

	bool ok = true;
	BMPImage* image = malloc(sizeof(*image));
	if(image == NULL){
		*error = _error("Not malloced properly");
		return NULL;
	}		
    if(fread(&(image->header),sizeof(image->header),1,fp) != 1){	
		*error =_error("Could not read the file properly");
		free(image);
		return NULL;
	}
	fseek(fp,0,SEEK_END);
	int image_size = ftell(fp) - sizeof(image->header);
	image->data = malloc(sizeof(*(image->data))*image_size);
	if(image->data == NULL) {
		*error = _error("Could not allocate data on heap");
		free(image);
		return NULL;
	}	
	fseek(fp,sizeof(image->header),SEEK_SET);
	fread((image->data),image_size,1,fp);
	ok = check_bmp_header(&(image->header),fp);
	if(ok == false)	{
		
		*error = _error("Header does not match");
		free_bmp(image);
		return NULL;
	}
	return image;
}

bool write_bmp(FILE* fp, BMPImage* image, char** error) {
	
	if(fwrite(&(image->header),sizeof(image->header),1,fp) != 1){
		*error = _error("Could not write properly");
		return false;
	}
	int padding = (image->header.width_px)%4;
	int image_size = (image->header.width_px * 3 + padding)*image->header.height_px;
		
	for(int i = 0; i< image_size; i++) {
		if(fwrite(&(image->data[i]),sizeof(image->data[i]),1,fp) != 1){
		
			*error = _error("Could not write properly");
			return false;
		}
	}
	return true;
}

void free_bmp(BMPImage* image){

	if(image != NULL){
	
		free(image->data);
		free(image);
	}
}
BMPImage* crop_bmp(BMPImage* image, int x, int y, int w, int h, char** error){
	
	int padding_old = (image->header.width_px)%4;
	if(x > image->header.width_px || x < 0 || y > image->header.height_px || y < 0) {
		
		*error = _error("x or y coordinates out of bounds");
		return NULL;
	}
	if(h + y  > image->header.height_px || h + y < 0 || w + x > image->header.width_px || w+x < 0 || h <=0 || w<= 0 ) {	
		*error = _error("width or height go out of bounds");
		return NULL;
	}
	BMPImage* crop = malloc(sizeof(*crop));
	if(crop == NULL){
		
		*error = _error("Could not malloc properly");
		return NULL;
	}
	crop->header = image->header;
	crop->header.width_px = w;
	crop->header.height_px = h;
	int image_size = (w*3 + w%4)*h;
	crop->header.image_size_bytes = image_size;
	crop->header.size = image_size + sizeof(crop->header);
	crop->data = malloc(sizeof(*(crop->data))*image_size);
	if(crop->data == NULL){
		*error = _error("could not malloc properly");
		free(crop);
		return NULL;
	}
	int index = 0;
	int pos = ((image->header.height_px-1) - (h+y-1)) * ((image->header.width_px)*3 + padding_old) + x * 3;
	for(int i = 0; i < h ; i++) {		
		for(int j = 0; j < (w*3 + (w%4) ); j++) {
			
			if(j>=w*3){
				crop->data[index] = 0;
			}
			else {
				crop->data[index] = image->data[pos+j];
			}
			index++;
		}
		pos += (image->header.width_px*3+padding_old) ;
	}
	return crop;
}
