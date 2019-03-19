#include<iostream>
#include<cstring>
#include<string.h>
#include<cmath>
#include<algorithm>
#include<vector>
#include<fstream>
#include<stdlib.h>
#include<time.h>
#include <sstream>    //使用stringstream需要引入这个头文件
using namespace std;

#define innode 12    //输入结点数
#define hidenode 5  //隐藏层结点数
#define hidelayer 1  //隐藏层层数
#define outnode 1    //输出结点数
#define alpha 0.00001    //学习速率

vector<vector<double> >train_x;
vector<double> train_y;
vector<vector<double> > vali_x;
vector<double> vali_y;
vector<vector<double> >test;
vector<double> result;
vector<vector<double> > input_w;
vector<double> hidden_w;
vector<double> MSE;
double train_loss;
vector<double> vali_mse;
double vali_loss;
vector<vector<double> > test20;
vector<double> testres;


/*作用：将string类型的数据转成double型 */
double stringToDouble(const string& str)
{
	istringstream iss(str);
	double num;
	iss >> num;
	return num;
}

/*作用：将一行文本切割成一个个数字*/
vector<double> get_xi(string temp)
{
	vector<double> xi;
	string pattern = ",";
	temp += pattern;
	int size = temp.size(), pos;
	for (int i = 0;i<size;i++) {
		pos = temp.find(pattern, i);
		if (pos<size) {
			string s = temp.substr(i, pos - i);
			double x = stringToDouble(s);
			xi.push_back(x);
			i = pos; // i=pos+pattern.size()-1; 
		}
	}
	return xi;
}

void get_train(const char* filename,int n) {
	ifstream ReadFile;
	string temp;
	vector<vector<double> > x;
	vector<double> y;
	ReadFile.open(filename, ios::in);
	bool first = true;
	if (ReadFile.fail()) {
		return;
	}
	else {
		while (getline(ReadFile, temp)) {
			if (first == true) first = false;
			else {
				vector<double> v;
				v = get_xi(temp);
				v.erase(v.begin(),v.begin()+2);
				v.insert(v.begin(), 1);
				y.push_back(v[v.size() - 1]);
				v.pop_back();
				x.push_back(v);
			}		
		}
	}
	ReadFile.close();

	int x_size = x.size(),j=0;
	for (int i = 0;i < x_size;i++) {
		if (j < n) {
			train_x.push_back(x[i]);
			train_y.push_back(y[i]);
			j++;
		}
		else if (j == n) {
			vali_x.push_back(x[i]);
			vali_y.push_back(y[i]);
			j = 1;
		} 		
	}
	
	for(int i=8120;i<x_size;i++){
		test20.push_back(x[i]);
		testres.push_back(y[i]);
	}
}

void get_test(const char* filename){
	ifstream ReadFile;
	string temp;
	ReadFile.open(filename, ios::in);
	bool first = true;
	if (ReadFile.fail()) {
		return;
	}
	else {
		while (getline(ReadFile, temp)) {
			if (first == true) first = false;
			else {
				vector<double> v;
				v = get_xi(temp);
				v.erase(v.begin(),v.begin()+2);
				v.insert(v.begin(), 1);
				v.pop_back();
				test.push_back(v);
			}		
		}
	}
	ReadFile.close();
}

void normalization(vector<vector<double> > v){
	int row=v.size(),column=v[0].size();
	double max=0,min=1000000;
	for(int i=0;i<column;i++){
		for(int j=0;j<row;j++){
			if(v[j][i]>max) max=v[j][i];
			if(v[j][i]<min) min=v[j][i];
		}
		for(int j=0;j<row;j++){
			v[j][i]= 1.0*(v[j][i]-min)/(max-min);
		}
	}
}


double get_random() {
	double res = (double)rand() / (RAND_MAX+1);
	return res;
}

double sigmoid(double x)
{
	double res = 1.0 / (1.0 + exp(-x));
	return res;
}


//初始化输入层到隐藏层的W 和 隐藏层到输出层的W
void initialize_weight(double weight) {
	for (int i = 0;i < innode+1;i++) {
		vector<double> v;
		for (int j = 0;j < hidenode;j++) {
			v.push_back(get_random());	
		}
		input_w.push_back(v);
	}
	for (int i = 0;i < hidenode;i++) {
		hidden_w.push_back(get_random());
	}
}

//单个样本前向传播
void forwardPropagationEpoc(vector<double> sample,vector<double> & hidden_out,double &output) {
	//隐藏层的输入
	vector<double> hidden_in;
	for (int i = 0;i < hidenode;i++) {
		double sum = 0;
		for (int j = 0;j < innode+1;j++) {
			sum += sample[j] * input_w[j][i];
		}
		hidden_in.push_back(sum);
	}
	

	//隐藏层的输出
	hidden_out.clear();
	for (int i = 0;i < hidenode;i++) {
		double res = sigmoid(hidden_in[i]);
		hidden_out.push_back(res);
	}

	//输出层的输入
	double output_in=0;
	for (int i = 0;i < hidenode;i++) {
		output_in += 1.0*hidden_out[i] * hidden_w[i];
	}
	
	//输出层的输出 
	output=output_in;

}


//单个样本后向传播,更新权值
void backPropagationEpoc(double rightout,double output,vector<double> hidden_out,vector<double> sample) {

	double mse = 1.0*(rightout - output)*(rightout - output);
	train_loss+=mse;
	//隐藏层->输出层的权值更新步长
	vector<double> renew_hw;
	renew_hw.clear();
	double hw = 1.0*(rightout- output);
	for (int i = 0;i < hidenode;i++) {
		double hw1 = hw*hidden_out[i];
		renew_hw.push_back(hw1);
	}
	//输入层-》隐藏层的权值更新步长 
	for (int i = 0;i < innode+1;i++) {
		for (int j = 0;j < hidenode;j++) {
			double iw = 1.0*hw*hidden_w[j] * hidden_out[j] * (1 - hidden_out[j])*sample[i];
			input_w[i][j] = input_w[i][j] + 1.0*alpha*iw / (innode+1);
		}
	}
//更新隐藏层->输出层的权值 
	for (int i = 0;i < hidenode;i++) {
		hidden_w[i] = hidden_w[i] + 1.0*alpha*renew_hw[i]/(1.0*hidenode);
	}	
}


void predict(vector<vector<double> > test_x) {
	result.clear();
	int test_size = test_x.size();
	double output=0;
	vector<double> hidden_out;
	for (int i = 0;i < test_size;i++) {
		forwardPropagationEpoc(test_x[i],hidden_out,output);
		result.push_back(output);
		double mse=(vali_y[i]-output)*(vali_y[i]-output);
		vali_loss+=mse;
//		cout<<output<< endl; 
	}
}

void trainning(int iteration)
{
	int train_size = train_x.size();
	initialize_weight(1);
	vector<double> hidden_out;
	double output;
	while (iteration--) {
		train_loss=0;
		for (int i = 0;i < train_size;i++) {
			hidden_out.clear();
			output=0;
			forwardPropagationEpoc(train_x[i],hidden_out,output);
			backPropagationEpoc(train_y[i], output, hidden_out, train_x[i]);	
		}
		
		train_loss = 1.0*train_loss/train_x.size();
		MSE.push_back(train_loss);
//		cout << "train_loss=" << train_loss << endl;
		vali_loss=0;
		predict(vali_x);
		vali_loss=vali_loss/vali_x.size();
		vali_mse.push_back(vali_loss);
//		cout << "vali_loss=" << vali_loss << endl;

	}
}



int main()
{
	get_train("F:\\学习资料\\大三上\\人工智能\\实验\\lab6\\train.csv",3);
	get_test("F:\\学习资料\\大三上\\人工智能\\实验\\lab6\\test.csv");	
	normalization(train_x);
	int iteration = 3000;
	trainning(iteration);
	predict(test);
	
//	predict(test20);
//	//20个数据集的预测值和真实值的对比
//	ofstream out1;
//	out1.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab6\\res1.csv",ios::out);
//	int row_of_mse=result.size();
//	for(int r=0;r<row_of_mse;r++){
//		out1 << result[r] << ",";	
//	}
//	out1<<"\n";
//	out1.close();
//	ofstream out2;
//	out2.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab6\\res2.csv",ios::out);
//	int row_of_vali=test20.size();
//	for(int r=0;r<row_of_vali;r++){
//		out2 << testres[r]<<",";
//	}
//	out2.close(); 
	
	
	ofstream outFile;
	outFile.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab6\\data.txt",ios::out);
	int row_of_res=result.size();
	for(int r=0;r<row_of_res;r++){
		outFile << result[r] << "\n";	
	}
	outFile.close();
	
	
/*输出train_loss和vali_loss	*/
//	ofstream out1;
//	out1.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab6\\mse.csv",ios::out);
//	int row_of_mse=MSE.size();
//	for(int r=0;r<row_of_mse;r++){
//		out1 << MSE[r] << ",";	
//	}
//	out1<<"\n";
//	out1.close();
//	ofstream out2;
//	out2.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab6\\mse1.csv",ios::out);
//	int row_of_vali=vali_mse.size();
//	for(int r=0;r<row_of_vali;r++){
//		out2 << vali_mse[r]<<",";
//	}
//	out2.close();

	
	
    return 0;
}


