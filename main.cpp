#include"main.hpp"

#include<numeric>
using namespace std;

int main(){

	vector<int> a = {1, 2, 3, 4, 1, 2, 3, 4};

	int sum = 0;
	int mul = 1;
	sum = accumulate(a.begin(), a.end(), 0);
	mul = accumulate(a.begin(), a.end(), mul, multiplies<int>());
	cout << mul << endl;



}

int main2(){
	initGLFWWindow();

	vk_init();
	preRender();

	mainLoop();

	postRender();
	vk_deinit();

	return 0;
}
