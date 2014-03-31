#include "picture_montage_class.h"
using namespace std;
using namespace cv;

int main(int argc, char** argv) {  //paragrams: image chips folder path / image path
	if(argc != 5) {
		cout << "parameters wrong!" << endl;
		return -1;  //paragrams: two paths
	}
	int size_of_chips = atoi(argv[3]);
	int grid_number = atoi(argv[4]);
	PictureMontage montage(size_of_chips, grid_number);
	if (!montage.runMontage(argv[1], argv[2])) {
		cout << "something wrong!" << endl;
		return -1;
	}
	waitKey(0);
	return 0;
}
