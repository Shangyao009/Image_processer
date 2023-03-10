#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "ppm.h"
#include <math.h>
#include <vector>
#include <time.h>
#define Pi 3.14159265358979323846

const char sc_img_path[] = "C:/Users/shang/Desktop/vs/cpp/test_pic.ppm";
const char dst_img_path[] = "C:/Users/shang/Desktop/vs/cpp/new_pic2.ppm";
PPM ori_pic; //Ori pic
PPM dst_pic; //destination pic
 

int sobel_x_kernel[3][3] = { //only emphasis the diference between left pixel and right pixel
	{-1 ,0 ,1 },			 //-1 and 1 can exchange position and it doesn't bother
	{-2 ,0 ,2 },
	{-1 ,0 ,1 }
};

int sobel_y_kernel[3][3] = { //only emphasis the diference between up pixel and down pixel
	{-1 ,-2 ,-1 },
	{ 0 , 0 , 0 },
	{ 1 , 2 , 1 }
};

void read_img(const char* filename, PPM& ori_pic);
void sobel_edge_detection(PPM ori_pic, PPM& dst_pic);
void edge_detection(PPM ori_pic, PPM& dst_pic, bool All_or_N = false, unsigned char standard_ = 0);
void write_img(PPM& img, const char* new_pic_path);
void avr_blur(PPM img, PPM& dst_img, int blur_rad = 1);
void RGB_to_BW(PPM img, PPM& dst_img);
void gaussian_blur(PPM img, PPM& dst_img, float SD, unsigned int blur_rd);
std::vector<std::vector<float>> get_gaussian_kernel(unsigned int radian, float SD);
float gaussian_func(float SD, int& x, int& y);

int main() 
{
	clock_t tStart = clock();
	read_img(sc_img_path,::ori_pic);

	std::cout << "Pic Format: " << ori_pic.format << "\n";
	std::cout << "Pic Width: " << ori_pic.width << "\n";
	std::cout << "Pic Height: " << ori_pic.height << "\n";
	std::cout << "Pic Size: " << ori_pic.height*ori_pic.width << "\n";

	clock_t t1 = clock() - tStart;
	std::cout << "Time taken for read img: " << ((double)(t1) / CLOCKS_PER_SEC) << "s \n";

	//avr_blur(::ori_pic, ::ori_pic, 1);
	gaussian_blur(::ori_pic, ::ori_pic, 1, 1);
	clock_t t2 = clock() - t1;
	std::cout << "Time taken for gaussian blur: " << ((double)(t2) / CLOCKS_PER_SEC) << "s \n";
	
	edge_detection(::ori_pic, ::dst_pic, true, 100);
	clock_t t3 = clock() - t1 - t2;
	std::cout << "Time taken for edge detection: " << ((double)(t3) / CLOCKS_PER_SEC) << "s \n";

	//RGB_to_BW(::dst_pic, ::dst_pic);

	write_img(::dst_pic, dst_img_path);
	clock_t t4 = clock() - t1 - t2 - t3;
	std::cout << "Time taken for write img: " << ((double)(t4) / CLOCKS_PER_SEC) << "s \n";

	std::cout << "Time taken for whole prog: " << ((double)(clock() - tStart) / CLOCKS_PER_SEC) << "\n";

	return 0;

}

void edge_detection(PPM ori_pic, PPM& dst_pic, bool All_or_N, unsigned char standard_)
{
	sobel_edge_detection(ori_pic,dst_pic);
	if (All_or_N)
	{
		unsigned char standard = standard_;
		for (int i = 0; i < dst_pic.pixels.size(); i++)
		{
			pixel ori_pixel = dst_pic.pixels[i];
			int greyscale = std::round(0.3 * ori_pixel.red + 0.59 * ori_pixel.green + 0.11 * ori_pixel.blue);
			if (greyscale >= standard) { greyscale = 255; }
			else { greyscale = 0; }
			dst_pic.pixels[i].red = greyscale;
			dst_pic.pixels[i].green = greyscale;
			dst_pic.pixels[i].blue = greyscale;
		}
	}
	else
	{
		for (int i = 0; i < dst_pic.pixels.size(); i++)
		{
			pixel ori_pixel = dst_pic.pixels[i];
			int greyscale = std::round(0.3 * ori_pixel.red + 0.59 * ori_pixel.green + 0.11 * ori_pixel.blue);

			dst_pic.pixels[i].red = greyscale;
			dst_pic.pixels[i].green = greyscale;
			dst_pic.pixels[i].blue = greyscale;
		}
	}
}


void gaussian_blur(PPM img, PPM& dst_img, float SD, unsigned int blur_rd)
{
	int blur_rad = blur_rd;
	std::stringstream str(img.format);
	str >> dst_img.format;
	dst_img.height = img.height;
	dst_img.width = img.width;
	dst_img.max_val = img.max_val;
	dst_img.pixels.resize(img.pixels.size());

	std::vector<std::vector<float>> gaussian_kernel = get_gaussian_kernel(blur_rad,SD);
	/*float total = 0;
	for (std::vector<float> vec : gaussian_kernel)
		for (float value : vec)
		{
			total += value;
		}*/

	for (int y = 0; y < dst_img.height; y++)
	{
		for (int x = 0; x < dst_img.width; x++) //pixel position
		{
			float p_r = 0; float p_g = 0; float p_b = 0;
			float total_effect = 0;
			for (int j = -blur_rad; j <= blur_rad; j++) {
				for (int i = -blur_rad; i <= blur_rad; i++) {
					if (y + j < 0)
					{
						continue;
					}
					else if (y + j >= dst_img.height)
					{
						continue;
					}
					else if (x + i < 0)
					{
						continue;
					}
					else if (x + i >= dst_img.width)
					{
						continue;
					}

					int idx = (y + j) * dst_img.width + (x + i);     //index
					float effect = gaussian_kernel[j + blur_rad][i + blur_rad];
					total_effect += effect;
					p_r += img.pixels[idx].red * effect;
					p_g += img.pixels[idx].green * effect;
					p_b += img.pixels[idx].blue * effect;

				}
			}
			int mag_r = std::round(p_r / total_effect);
			int mag_g = std::round(p_g / total_effect);
			int mag_b = std::round(p_b / total_effect);

			mag_r = std::min(mag_r, img.max_val);
			mag_g = std::min(mag_g, img.max_val);
			mag_b = std::min(mag_b, img.max_val);

			int idx = y * img.width + x;
			dst_img.pixels[idx].red = mag_r;
			dst_img.pixels[idx].green = mag_g;
			dst_img.pixels[idx].blue = mag_b;
		}
	}
}

std::vector<std::vector<float>> get_gaussian_kernel(unsigned int radian,float SD)
{
	std::vector<std::vector<float>> gaussian_kernel;
	int kernel_width = 2 * radian + 1;
	
	std::vector<float> vec;
	//resize not function
	for (int i = 0; i < kernel_width; i++)
	{
		vec.push_back(0);
	}
	for (int i = 0; i < kernel_width; i++)
	{
		gaussian_kernel.push_back(vec);
	}

	for (std::vector<float> vec : gaussian_kernel) {vec.resize(kernel_width);}
	int pos_adjustment = radian;
	for (int a = 0; a < kernel_width; a++)
		for (int b = 0; b < kernel_width; b++)
		{
			int y = a - pos_adjustment; int x = b - pos_adjustment;
			y = (y >= 0) ? y : -1 * y;
			x = (x >= 0) ? x : -1 * x;
			float value = gaussian_func(SD, x, y);
			gaussian_kernel[a][b] = value;
		}
	return gaussian_kernel;
}

float gaussian_func(float SD,int & x, int & y)
{
	double e = 2.71828;
	double pow_e = -(x * x + y * y) / (2 * SD * SD);
	float value = (std::pow(e,pow_e)) / (2 * Pi * SD * SD);
	return value;
}

void brigthening(PPM ori_pic,PPM& dst_pic, int brigthening_value)
{
	std::stringstream str(ori_pic.format);
	str >> dst_pic.format;
	dst_pic.height = ori_pic.height;
	dst_pic.width = ori_pic.width;
	dst_pic.max_val = ori_pic.max_val;
	dst_pic.pixels.resize(ori_pic.pixels.size());

	for (int i =0;i<ori_pic.pixels.size();i++)
	{
		pixel pixel = ori_pic.pixels[i];
		int red = pixel.red + brigthening_value;
		int green = pixel.green + brigthening_value;
		int blue = pixel.blue + brigthening_value;
		if (red > 255)
		{
			red = 255;
		}
		else if (red < 0)
		{
			red = 0;
		}
		if (green > 255)
		{
			green = 255;
		}
		else if (green < 0)
		{
			green = 0;
		}if (blue > 255)
		{
			blue = 255;
		}
		else if (blue< 0)
		{
			blue = 0;
		}
		dst_pic.pixels[i].red = red;
		dst_pic.pixels[i].green = green;
		dst_pic.pixels[i].blue = blue;
	}
}

void read_img(const char* filename, PPM& ori_pic)
{
	std::ifstream pic(filename, std::ios::binary);
	if (!pic)
	{
		std::cout << "file not found! \n";
		exit(0);
	}

	pic >> ori_pic.format;
	std::string format(ori_pic.format);
	if (format != "P6")
	{
		std::cout << "unsupport pic format " << format << "\n";
		exit(0);
	}
	/*pic >> ori_pic.width >> ori_pic.height >> ori_pic.max_val;
	pic.ignore(1, '\n');*/
	char text[10];
	pic.getline(text, 10, ' '); ori_pic.width = std::stoi(text);
	pic.getline(text, 10, '\n'); ori_pic.height = std::stoi(text);
	pic.getline(text, 10, '\n'); ori_pic.max_val = std::stoi(text);
	
	int size = ori_pic.height * ori_pic.width;

	for (int i = 0; i < size; i++)
	{
		//pic.read((char*)(&ori_pic.pixels[0]), ori_pic.width * ori_pic.height * 3);
		unsigned char red = pic.get();
		unsigned char green = pic.get();
		unsigned char blue = pic.get();
		ori_pic.pixels.push_back({ red,green,blue });
	}
	
	pic.close();
}

void sobel_edge_detection(PPM ori_pic, PPM& dst_pic) 
{
	std::stringstream str(ori_pic.format);
	str >> dst_pic.format;
	dst_pic.height = ori_pic.height;
	dst_pic.width = ori_pic.width;
	dst_pic.max_val = ori_pic.max_val;
	dst_pic.pixels.resize(ori_pic.pixels.size()); //reserve space for the pixels

	//not detect the edge of the picture as they do not fulfill the requirement of being edge
	
	for (int y = 1; y < ori_pic.height - 1; y++)
	{
		for (int x = 1; x < ori_pic.width - 1; x++) //pixel position
		{
			int px_r = 0, px_g = 0, px_b = 0; //pixel color
			int py_r = 0, py_g = 0, py_b = 0;
			for (int i = -1; i <= 1; i++){
				for (int j = -1; j <= 1; j++) {
					unsigned char srd_pixel_r, srd_pixel_g, srd_pixel_b;
					int idx = (y + j) * ori_pic.width + (x + i); ;     //index
					int effect_x = sobel_x_kernel[j + 1][i + 1];
					int effect_y = sobel_y_kernel[j + 1][i + 1];

					px_r += ori_pic.pixels[idx].red * effect_x;
					px_g += ori_pic.pixels[idx].green * effect_x;
					px_b += ori_pic.pixels[idx].blue * effect_x;

					py_r += ori_pic.pixels[idx].red * effect_y;
					py_g += ori_pic.pixels[idx].green * effect_y;
					py_b += ori_pic.pixels[idx].blue * effect_y;

				}
			}
			int mag_r = std::sqrt(px_r * px_r + py_r * py_r); 
			int mag_g = std::sqrt(px_g * px_g + py_g * py_g);
			int mag_b = std::sqrt(px_b * px_b + py_b * py_b);

			mag_r = std::min(mag_r, ori_pic.max_val);
			mag_g = std::min(mag_g, ori_pic.max_val);
			mag_b = std::min(mag_b, ori_pic.max_val);

			int idx = y * ori_pic.width + x;
			dst_pic.pixels[idx].red = mag_r;
			dst_pic.pixels[idx].green = mag_g;
			dst_pic.pixels[idx].blue = mag_b;
		}
	}
}

void avr_blur(PPM img, PPM& dst_img, int blur_rad)
{
	if (blur_rad < 0) {blur_rad *= -1;}
	std::stringstream str(img.format);
	str >> dst_img.format;
	dst_img.height = img.height;
	dst_img.width = img.width;
	dst_img.max_val = img.max_val;
	dst_img.pixels.resize(img.pixels.size());

	for (int y = 0; y < dst_img.height; y++) {
		for (int x = 0; x < dst_img.width; x++) {
			int p_r = 0; int p_g = 0; int p_b = 0;
			int T_eff = 0;
			for (int j = -1 * blur_rad; j <= blur_rad; j++){
				for (int i = -1 * blur_rad; i <= blur_rad; i++) {
					
					if (y + j < 0)
					{
						continue;
					}
					else if (y + j >= dst_img.height)
					{
						continue;
					}
					else if (x + i < 0)
					{
						continue;
					}
					else if (x + i >= dst_img.width)
					{
						continue;
					}

					int index = (y + j) * dst_img.width + (x + i);
					p_r += img.pixels[index].red;
					p_g += img.pixels[index].green;
					p_b += img.pixels[index].blue;
					T_eff += 1;
				}
			}
			int idx = y * dst_img.width + x;
			dst_img.pixels[idx].red = std::round(p_r / (T_eff));
			dst_img.pixels[idx].green = std::round(p_g / T_eff);
			dst_img.pixels[idx].blue = std::round(p_b / T_eff);
		}
	}
}

void write_img(PPM& img, const char* new_pic_path)
{
	std::ofstream new_pic(new_pic_path, std::ios::binary);
    if (!new_pic) {
        std::cerr << "Error: could not open file " << new_pic_path << std::endl;
        exit(1);
    }
	//new_pic.write()
	new_pic << img.format << '\n';
	new_pic << img.width << ' ' << img.height << '\n';
	new_pic << img.max_val << '\n';
	for (pixel pixel : img.pixels)
	{
		new_pic << pixel.red << pixel.green << pixel.blue;
	}
	new_pic.close();
}

void RGB_to_BW(PPM img, PPM& dst_img)
{
	std::stringstream str(img.format);
	str >> dst_img.format;
	dst_img.height = img.height;
	dst_img.width = img.width;
	dst_img.max_val = img.max_val;
	dst_img.pixels.resize(img.pixels.size());

	for (int i = 0; i < dst_img.pixels.size(); i++)
	{
		pixel ori_pixel = img.pixels[i];
		int greyscale = std::round(0.3 * ori_pixel.red + 0.59 * ori_pixel.green + 0.11 * ori_pixel.blue);
		dst_img.pixels[i].red = greyscale;
		dst_img.pixels[i].green = greyscale;
		dst_img.pixels[i].blue = greyscale;
		
	}
}