#pragma once
#include <iostream>
#include <vector>

struct pixel
{
	unsigned char red, green, blue;  //unsigned char --> 当成数字处理 (0-255) if signed (0-127) 
};

struct PPM
{
	std::vector<pixel> pixels;
	char format[3];
	int width;
	int height;
	int max_val; //extra 1 index for the ending '/0' 'NULL'
};