/*
 * picture_montage_class.h
 */

#ifndef PICTURE_MONTAGE_CLASS_H_
#define PICTURE_MONTAGE_CLASS_H_

#include <vector>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

class PictureMontage {
public:
	PictureMontage(int size_of_chips, int number_of_chips);
	~PictureMontage();
	bool runMontage(std::string folder_path, std::string image_path);
protected:
	void cutSize(cv::Mat &chip_image);
	bool readTheImageChips(std::string &folder_path);
	void calcChipsColor();
	void calcImageColor();
	int calcSimilarity(std::vector< std::vector<int> > &chip_color_1,
									std::vector< std::vector<int> > &chip_color_2);
	int findRightChip(std::vector< std::vector<int> > &chip_color);
	void stickChips();
	void stickOnImage(int y, int x);
private:
	PictureMontage(const PictureMontage &);
	PictureMontage& operator= (const PictureMontage &);

	cv::Mat image;
	int size_of_chips;
	int grid_number;
	std::vector<cv::Mat> vec_mat_chips;
	int number_of_chips;
	std::vector< std::vector< std::vector<int> > >  vec_chips_color;
	std::vector< std::vector< std::vector< std::vector<int> > > > vec2D_image_color;
	std::vector<int>  chips_visited;
};

#endif /* PICTURE_MONTAGE_CLASS_H_ */
