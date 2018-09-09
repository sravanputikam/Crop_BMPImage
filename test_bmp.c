#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
int main( int argc, char* argv[]){
	
	char* filename = "6x6_24bit.bmp";
	FILE* fp = fopen(filename,"r");
	if(fp == NULL) {
		return EXIT_FAILURE;
	}
	char* error = NULL;
	BMPImage* image = read_bmp(fp,&error);
	if(error != NULL) {
	
		fprintf(stderr,"Error: %s\n",error);
		free(error);
		fclose(fp);
		return EXIT_FAILURE;
	}
	fclose(fp);
	fp = fopen("image.bmp","w");
	write_bmp(fp, image,&error);
	if(error != NULL) {
	
		fprintf(stderr,"Error: %s\n",error);
		free(error);
		fclose(fp);
		free_bmp(image);
		return EXIT_FAILURE;
	
	}
	BMPImage* crop = crop_bmp(image,0,0 ,5 ,5 , &error);
	if(error != NULL) {
	
		fprintf(stderr,"Error: %s\n",error);
		free(error);
		fclose(fp);
		free_bmp(image);		
		return EXIT_FAILURE;
	}
	fclose(fp);
	fp = fopen("crop.bmp","w");
	write_bmp(fp,crop, &error);
	free_bmp(image);
	free_bmp(crop);
	fclose(fp);
	printf("Everything is successful");
	return EXIT_SUCCESS;
}
