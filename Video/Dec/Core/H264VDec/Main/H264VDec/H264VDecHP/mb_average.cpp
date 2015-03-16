/*!
***************************************************************************
* \file
*    mb_average.cpp
*
* \brief
*    C version file for two macrblocks/sub-macroblocks average
*
***************************************************************************
*/

#include "mb_average.h"

average_t *average_16;
average_t *average_8;
average_t *average_4;
average_t *average_2;


weight_t *weight_16;
weight_t *weight_8;
weight_t *weight_4;
weight_t *weight_2;

weight_b_t *weight_16_b;
weight_b_t *weight_8_b;
weight_b_t *weight_4_b;
weight_b_t *weight_2_b;

/*
merge UV
*/
weight_uv_t *weight_8_uv;
weight_uv_t *weight_4_uv;
weight_uv_t *weight_2_uv;

weight_b_uv_t *weight_8_b_uv;
weight_b_uv_t *weight_4_b_uv;
weight_b_uv_t *weight_2_b_uv;


void average_16_c(imgpel *output, 
									int stride_out,
									imgpel *input1,
									imgpel *input2,
									int stride_in,
									int height)
{
	int i,j;

	for(j=0;j<height;j++)
	{
		for(i=0;i<16;i++)
		{
			output[j*stride_out+i] = (input1[j*stride_in+i] + input2[j*stride_in+i] + 1)>>1;
		}
	}
}

void average_8_c(imgpel *output, 
								 int stride_out,
								 imgpel *input1,
								 imgpel *input2,
								 int stride_in,
								 int height)
{
	int i,j;

	for(j=0;j<height;j++)
	{
		for(i=0;i<8;i++)
		{
			output[j*stride_out+i] = (input1[j*stride_in+i] + input2[j*stride_in+i] + 1)>>1;
		}
	}
}

void average_4_c(imgpel *output, 
								 int stride_out,
								 imgpel *input1,
								 imgpel *input2,
								 int stride_in,
								 int height)
{
	int j;

	for(j=height;j>0;j--)
	{
		output[0] = (input1[0] + input2[0] + 1)>>1;
		output[1] = (input1[1] + input2[1] + 1)>>1;
		output[2] = (input1[2] + input2[2] + 1)>>1;
		output[3] = (input1[3] + input2[3] + 1)>>1;
		input1 += stride_in;
		input2 += stride_in;
		output += stride_out;
	}
}


void average_2_c(imgpel *output, 
								 int stride_out,
								 imgpel *input1,
								 imgpel *input2,
								 int stride_in,
								 int height)
{
	int j;

	for(j=height;j>0;j--)
	{
		output[0] = (input1[0] + input2[0] + 1)>>1;
		output[1] = (input1[1] + input2[1] + 1)>>1;
		input1 += stride_in;
		input2 += stride_in;
		output += stride_out;
	}
}


void weight_16_c(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height)
{
	int ii, jj;
	for(jj=0;jj<height;jj++)
		for(ii=0;ii<16;ii++)
		{
			(*input_image) = Clip1(((weight * (*input_image) + rounding_offset) >> down_shift) + final_offset);
			input_image++;
		}
}

void weight_8_c(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height)
{
	int ii, jj;
	imgpel *tmp_pel;
	for(jj=0;jj<height;jj++)
	{
		tmp_pel = input_image;
		for(ii=0;ii<8;ii++)
		{
			(*tmp_pel) = Clip1(((weight * (*tmp_pel) + rounding_offset) >> down_shift) + final_offset);
			tmp_pel++;
		}
		input_image += stride_in;
	}
}

void weight_4_c(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height)
{
	int jj;
	for(jj=0;jj<height;jj++)
	{
		input_image[0] = Clip1(((weight * input_image[0] + rounding_offset) >> down_shift) + final_offset);
		input_image[1] = Clip1(((weight * input_image[1] + rounding_offset) >> down_shift) + final_offset);
		input_image[2] = Clip1(((weight * input_image[2] + rounding_offset) >> down_shift) + final_offset);
		input_image[3] = Clip1(((weight * input_image[3] + rounding_offset) >> down_shift) + final_offset);

		input_image+= stride_in;
	}
}

void weight_2_c(imgpel *input_image, int weight, int rounding_offset, unsigned int down_shift, int final_offset, int stride_in, int height)
{	
	int jj;
	for(jj=0;jj<height;jj++)
	{
		input_image[0] = Clip1(((weight * input_image[0] + rounding_offset) >> down_shift) + final_offset);
		input_image[1] = Clip1(((weight * input_image[1] + rounding_offset) >> down_shift) + final_offset);
		input_image+=stride_in;
	}
}

void   weight_16_b_c(imgpel *output, 
										 int stride_out,
										 imgpel *input1, imgpel *input2, 
										 int weight1, int weight2, 
										 int rounding_offset, unsigned int down_shift, int final_offset, 
										 int stride_in, int height)
{
	int ii, jj;
	for(jj=0;jj<height;jj++)
		for(ii=0;ii<16;ii++)
		{
			(*output) = Clip1((( weight1 * (*input1) + weight2* (*input2) +rounding_offset) >> down_shift) + final_offset);
			output++;
			input1++;
			input2++;
		}
}

void   weight_8_b_c(imgpel *output, 
										int stride_out,
										imgpel *input1, imgpel *input2, 
										int weight1, int weight2, 
										int rounding_offset, unsigned int down_shift, int final_offset, 
										int stride_in, int height)
{
	int ii, jj;
	imgpel *tmp_pel, *tmp_pel2;	
	imgpel *tmp_out;
	for(jj=0;jj<height;jj++)
	{
		tmp_out = output;
		tmp_pel = input1;
		tmp_pel2 = input2;
		for(ii=0;ii<8;ii++)
		{
			(*tmp_out) = Clip1((( weight1 * (*tmp_pel) + weight2* (*tmp_pel2) +rounding_offset) >> down_shift) + final_offset);
			tmp_out++;
			tmp_pel++;
			tmp_pel2++;
		}
		output+=stride_out;
		input1+=stride_in;
		input2+=stride_in;
	}
}

void   weight_4_b_c(imgpel *output, 
										int stride_out,
										imgpel *input1, imgpel *input2, 
										int weight1, int weight2, 
										int rounding_offset, unsigned int down_shift, int final_offset, 
										int stride_in, int height)
{

	int jj;
	for(jj=0;jj<height;jj++)
	{
		output[0] = Clip1(((weight1 * input1[0] + weight2 * input2[0] + rounding_offset) >> down_shift) + final_offset);
		output[1] = Clip1(((weight1 * input1[1] + weight2 * input2[1] + rounding_offset) >> down_shift) + final_offset);
		output[2] = Clip1(((weight1 * input1[2] + weight2 * input2[2] + rounding_offset) >> down_shift) + final_offset);
		output[3] = Clip1(((weight1 * input1[3] + weight2 * input2[3] + rounding_offset) >> down_shift) + final_offset);
		output+=stride_out;
		input1+= stride_in;
		input2+=stride_in;
	}
}

void   weight_2_b_c(imgpel *output, 
										int stride_out,
										imgpel *input1, imgpel *input2, 
										int weight1, int weight2, 
										int rounding_offset, unsigned int down_shift, int final_offset, 
										int stride_in, int height)
{
	int jj;
	for(jj=0;jj<height;jj++)
	{
		output[0] = Clip1(((weight1 * input1[0] + weight2 * input2[0] + rounding_offset) >> down_shift) + final_offset);
		output[1] = Clip1(((weight1 * input1[1] + weight2 * input2[1] + rounding_offset) >> down_shift) + final_offset);
		output+=stride_out;
		input1+=stride_in;
		input2+=stride_in;
	}
}

/*
//merge UV code
*/
void weight_8_uv_c(imgpel *input_image, int weight_u, int weight_v, int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v, int stride_in, int height)
{
	int ii, jj;
	imgpel *tmp_pel;
	for(jj=0;jj<height;jj++)
	{
		tmp_pel = input_image;
		for(ii=0;ii<8;ii++)
		{
			(*tmp_pel) = Clip1(((weight_u * (*tmp_pel) + rounding_offset) >> down_shift) + final_offset_u);
			tmp_pel++;
			(*tmp_pel) = Clip1(((weight_v * (*tmp_pel) + rounding_offset) >> down_shift) + final_offset_v);
			tmp_pel++;
		}
		input_image += stride_in;
	}
}

void weight_4_uv_c(imgpel *input_image, int weight_u, int weight_v, int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v, int stride_in, int height)
{
	int jj;
	for(jj=0;jj<height;jj++)
	{
		input_image[0] = Clip1(((weight_u * input_image[0] + rounding_offset) >> down_shift) + final_offset_u);
		input_image[1] = Clip1(((weight_v * input_image[1] + rounding_offset) >> down_shift) + final_offset_v);
		input_image[2] = Clip1(((weight_u * input_image[2] + rounding_offset) >> down_shift) + final_offset_u);
		input_image[3] = Clip1(((weight_v * input_image[3] + rounding_offset) >> down_shift) + final_offset_v);
		input_image[4] = Clip1(((weight_u * input_image[4] + rounding_offset) >> down_shift) + final_offset_u);
		input_image[5] = Clip1(((weight_v * input_image[5] + rounding_offset) >> down_shift) + final_offset_v);
		input_image[6] = Clip1(((weight_u * input_image[6] + rounding_offset) >> down_shift) + final_offset_u);
		input_image[7] = Clip1(((weight_v * input_image[7] + rounding_offset) >> down_shift) + final_offset_v);

		input_image+= stride_in;
	}
}

void weight_2_uv_c(imgpel *input_image, int weight_u, int weight_v, int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v, int stride_in, int height)
{	
	int jj;
	for(jj=0;jj<height;jj++)
	{
		input_image[0] = Clip1(((weight_u * input_image[0] + rounding_offset) >> down_shift) + final_offset_u);
		input_image[1] = Clip1(((weight_v * input_image[1] + rounding_offset) >> down_shift) + final_offset_v);
		input_image[2] = Clip1(((weight_u * input_image[2] + rounding_offset) >> down_shift) + final_offset_u);
		input_image[3] = Clip1(((weight_v * input_image[3] + rounding_offset) >> down_shift) + final_offset_v);
		input_image+=stride_in;
	}
}


void   weight_8_b_uv_c(imgpel *output, 
											 int stride_out,
											 imgpel *input1, imgpel *input2, 
											 int weight1_u, int weight2_u, 
											 int weight1_v, int weight2_v, 
											 int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v,
											 int stride_in, int height)
{
	int ii, jj;
	imgpel *tmp_pel, *tmp_pel2;	
	imgpel *tmp_out;
	for(jj=0;jj<height;jj++)
	{
		tmp_out = output;
		tmp_pel = input1;
		tmp_pel2 = input2;
		for(ii=0;ii<8;ii++)
		{
			(*tmp_out) = Clip1((( weight1_u * (*tmp_pel) + weight2_u* (*tmp_pel2) +rounding_offset) >> down_shift) + final_offset_u);
			tmp_out++;
			tmp_pel++;
			tmp_pel2++;
			(*tmp_out) = Clip1((( weight1_v * (*tmp_pel) + weight2_v* (*tmp_pel2) +rounding_offset) >> down_shift) + final_offset_v);
			tmp_out++;
			tmp_pel++;
			tmp_pel2++;
		}
		output+=stride_out;
		input1+=stride_in;
		input2+=stride_in;
	}
}

void   weight_4_b_uv_c(imgpel *output, 
											 int stride_out,
											 imgpel *input1, imgpel *input2, 
											 int weight1_u, int weight2_u, 
											 int weight1_v, int weight2_v, 
											 int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v,
											 int stride_in, int height)
{

	int jj;
	for(jj=0;jj<height;jj++)
	{
		output[0] = Clip1(((weight1_u * input1[0] + weight2_u * input2[0] + rounding_offset) >> down_shift) + final_offset_u);
		output[1] = Clip1(((weight1_v * input1[1] + weight2_v * input2[1] + rounding_offset) >> down_shift) + final_offset_v);
		output[2] = Clip1(((weight1_u * input1[2] + weight2_u * input2[2] + rounding_offset) >> down_shift) + final_offset_u);
		output[3] = Clip1(((weight1_v * input1[3] + weight2_v * input2[3] + rounding_offset) >> down_shift) + final_offset_v);
		output[4] = Clip1(((weight1_u * input1[4] + weight2_u * input2[4] + rounding_offset) >> down_shift) + final_offset_u);
		output[5] = Clip1(((weight1_v * input1[5] + weight2_v * input2[5] + rounding_offset) >> down_shift) + final_offset_v);
		output[6] = Clip1(((weight1_u * input1[6] + weight2_u * input2[6] + rounding_offset) >> down_shift) + final_offset_u);
		output[7] = Clip1(((weight1_v * input1[7] + weight2_v * input2[7] + rounding_offset) >> down_shift) + final_offset_v);
		output+=stride_out;
		input1+= stride_in;
		input2+=stride_in;
	}
}

void   weight_2_b_uv_c(imgpel *output, 
											 int stride_out,
											 imgpel *input1, imgpel *input2, 
											 int weight1_u, int weight2_u, 
											 int weight1_v, int weight2_v, 
											 int rounding_offset, unsigned int down_shift, int final_offset_u, int final_offset_v,
											 int stride_in, int height)
{
	int jj;
	for(jj=0;jj<height;jj++)
	{
		output[0] = Clip1(((weight1_u * input1[0] + weight2_u * input2[0] + rounding_offset) >> down_shift) + final_offset_u);
		output[1] = Clip1(((weight1_v * input1[1] + weight2_v * input2[1] + rounding_offset) >> down_shift) + final_offset_v);
		output[2] = Clip1(((weight1_u * input1[2] + weight2_u * input2[2] + rounding_offset) >> down_shift) + final_offset_u);
		output[3] = Clip1(((weight1_v * input1[3] + weight2_v * input2[3] + rounding_offset) >> down_shift) + final_offset_v);
		output+=stride_out;
		input1+=stride_in;
		input2+=stride_in;
	}
}
