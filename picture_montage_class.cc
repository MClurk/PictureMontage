/*
 * picture_montage_class.cc
 */
#include <errno.h>
#include <dirent.h>
#include <iostream>
#include <queue>
#include "picture_montage_class.h"
using namespace cv;
using namespace std;

PictureMontage::PictureMontage(int size = 20,
							   int grid = 2):number_of_chips(0) {
	if (size <= 0) {
		size_of_chips = 20;
	}
	else {
		size_of_chips = size;
	}

	if (size_of_chips % grid != 0) {
		size_of_chips = 20;
		grid_number = 2;
	}  //reset
	else {
		grid_number = grid;
	}
}

PictureMontage::~PictureMontage() {}

void PictureMontage::cutSize(Mat &chip_image) {
	int rows = chip_image.rows;
	int cols = chip_image.cols;
	int length = min(rows, cols);
	  //first: cut it to a square
	Mat square_image(chip_image,
					 Range(rows / 2 - length / 2, rows / 2 + (length - length / 2)),
					 Range(cols / 2 - length / 2, cols / 2 + (length - length / 2)) );
	resize(square_image, chip_image, Size(size_of_chips, size_of_chips));
}

bool PictureMontage::readTheImageChips(string &folder_path) {
	if (folder_path.empty()) {
		cout << "can't find the image folder!" << endl;
		return false;
	}
	vec_mat_chips.clear();
	DIR* ptr_dir = opendir(folder_path.c_str());
	if (!ptr_dir) {
		cout << "open fail!" <<endl;
		return false;
	}
	dirent* ptr_imagefile;
	errno = 0;
	while( (ptr_imagefile = readdir(ptr_dir)) ) {
		string filename(ptr_imagefile->d_name);
		if ( filename != string("..") && filename != string(".") ) {
			Mat chip_image = imread(folder_path +"/"+ filename);
			if (chip_image.empty()) {
				cout << "read image wrong!";
				return false;
			}
			cutSize(chip_image);
			vec_mat_chips.push_back(chip_image);
			++ number_of_chips;
		}
	}
	if(errno){
		cout << "file wrong!" << endl;
		return false;
	}
	closedir(ptr_dir);
	return true;
}

void PictureMontage::calcChipsColor() {
	if(vec_mat_chips.empty() || number_of_chips <= 0)
		return;

	const int grid_size = size_of_chips / grid_number;
	for(int i = 0; i < number_of_chips; ++i) {
		const Mat &this_chip = vec_mat_chips[i];
		for(int y = 0; y < size_of_chips; ++y) {
			int grid_y = y / grid_size;
			const unsigned char* ptr_chip_data = this_chip.ptr(y);
			for (int x = 0; x < size_of_chips; ++x) {
				int grid_x = x / grid_size;
				int grid_index = grid_y * grid_number + grid_x;
				for (int c = 0; c < 3; ++c) {
					vec_chips_color[i][grid_index][c] += ptr_chip_data[3 * x + c];
				}
			}
		}
	}  //for every chip image
	return;
}

void PictureMontage::calcImageColor() {
	if(image.empty())
		return;
	const int grid_height = image.rows / size_of_chips;
	const int grid_width = image.cols / size_of_chips;
	const int height = grid_height * size_of_chips;
	const int width = grid_width * size_of_chips;
	const int grid_size = size_of_chips / grid_number;
	  //focus on the roi_image
	Mat ori_image(image, Range(0, height), Range(0, width));
	image = ori_image;

	vector< vector<int> > chips_color(grid_number * grid_number, vector<int>(3, 0));
	vec2D_image_color.assign(grid_height, vector< vector< vector<int> > >(grid_width, chips_color));
	for (int y = 0; y < grid_height; ++y) {
		for (int x = 0; x < grid_width; ++x) {
			Mat roi_image_chip(image,
							   Range(y * size_of_chips, (y + 1) * size_of_chips),
							   Range(x * size_of_chips, (x + 1) * size_of_chips));
			for (int roi_y = 0; roi_y < size_of_chips; ++roi_y) {
				const unsigned char* ptr_data = roi_image_chip.ptr(roi_y);
				int grid_y = roi_y / grid_size;
				for (int roi_x = 0; roi_x < size_of_chips; ++roi_x) {
					int grid_x = roi_x / grid_size;
					int index = grid_y * grid_number + grid_x;
					for(int c = 0; c < 3; ++c) {
						vec2D_image_color[y][x][index][c] += ptr_data[3 * x + c];
					}
				}
			}
		}
	}
}

int PictureMontage::calcSimilarity(vector< vector<int> > &chip_color_1,
								   vector< vector<int> > &chip_color_2) {
	int similarity = 0;
	for (int i = 0; i < grid_number * grid_number; ++i) {
		for (int c = 0; c < 3; ++c) {
			similarity += abs(chip_color_1[i][c] - chip_color_2[i][c]);
		}
	}
	return similarity;
}

int PictureMontage::findRightChip(vector< vector<int> > &chip_color) {
	int most_similarity = 255 * 3 * size_of_chips * size_of_chips;
	int index_chip = -1;  //attention!
	for (int i = 0; i < number_of_chips; ++i) {
		if (chips_visited[i] > 0)
			continue;
		int similarity = calcSimilarity(chip_color, vec_chips_color[i]);
		if (similarity < most_similarity) {
			most_similarity = similarity;
			index_chip = i;
		}
	}
	return index_chip;
}

void PictureMontage::stickOnImage(int y, int x) {
	Mat roi_image_chip(image,
					   Range(y * size_of_chips, (y + 1) * size_of_chips),
					   Range(x * size_of_chips, (x + 1) * size_of_chips));
	int right_chip = findRightChip(vec2D_image_color[y][x]);
	if (right_chip >= 0) {  //right_chip == -1 means no image left, then do nothing! or ...
		vec_mat_chips[right_chip].copyTo(roi_image_chip);
		chips_visited[right_chip] ++;
	}
	else {
		roi_image_chip = Scalar_<unsigned char>::all(255);
	}
	imshow(">v<", image);
}

void PictureMontage::stickChips() {
	int image_chip_height = vec2D_image_color.size();
	int image_chip_width = vec2D_image_color[0].size();
	vector< vector<bool> > visited(image_chip_height, vector<bool>(image_chip_width, false));
	int y = image_chip_height / 2;
	int x = image_chip_width / 2;
	queue< pair<int, int> > queue_image_stickers;
	queue_image_stickers.push(make_pair(y,x));
	visited[y][x] = true;
	while(!queue_image_stickers.empty()) {
		pair<int, int> stick_site = queue_image_stickers.front();
		y = stick_site.first;
		x = stick_site.second;
		stickOnImage(y, x);
		queue_image_stickers.pop();

		if (y - 1 >= 0 && !visited[y - 1][x]) {
			queue_image_stickers.push(make_pair(y - 1, x));
			visited[y - 1][x] = true;
		}
		if (y + 1 < image_chip_height && !visited[y + 1][x]) {
			queue_image_stickers.push(make_pair(y + 1, x));
			visited[y + 1][x] = true;
		}
		if (x - 1 >= 0 && !visited[y][x - 1]) {
			queue_image_stickers.push(make_pair(y, x - 1));
			visited[y][x - 1] = true;
		}
		if (x + 1 < image_chip_width && !visited[y][x + 1]) {
			queue_image_stickers.push(make_pair(y, x + 1));
			visited[y][x + 1] = true;
		}
	}
}

bool PictureMontage::runMontage(string folder_path, string image_path) {
	if (!readTheImageChips(folder_path))
		return false;
	image = imread(image_path);
	if(image.empty()) {
		cout << "can't read the image!" << endl;
		return false;
	}
	vec_chips_color.assign(number_of_chips, vector< vector<int> >(grid_number * grid_number, vector<int>(3, 0)));
	chips_visited.assign(number_of_chips, 0);
	calcChipsColor();
	calcImageColor();
	stickChips();
	return true;
}
