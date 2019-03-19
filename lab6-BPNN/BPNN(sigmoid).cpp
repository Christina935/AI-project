#include<iostream>
#include<cstring>
#include<string.h>
#include<cmath>
#include<algorithm>
#include<vector>
#include<fstream>
#include<stdlib.h>
#include<time.h>
#include <sstream>    //ʹ��stringstream��Ҫ�������ͷ�ļ�
using namespace std;

#define innode 12    //��������
#define hidenode 5  //���ز�����
#define hidelayer 1  //���ز����
#define outnode 1    //��������
#define alpha 0.00001    //ѧϰ����

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


/*���ã���string���͵�����ת��double�� */
double stringToDouble(const string& str)
{
	istringstream iss(str);
	double num;
	iss >> num;
	return num;
}

/*���ã���һ���ı��и��һ��������*/
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


//��ʼ������㵽���ز��W �� ���ز㵽������W
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

//��������ǰ�򴫲�
void forwardPropagationEpoc(vector<double> sample,vector<double> & hidden_out,double &output) {
	//���ز������
	vector<double> hidden_in;
	for (int i = 0;i < hidenode;i++) {
		double sum = 0;
		for (int j = 0;j < innode+1;j++) {
			sum += sample[j] * input_w[j][i];
		}
		hidden_in.push_back(sum);
	}
	

	//���ز�����
	hidden_out.clear();
	for (int i = 0;i < hidenode;i++) {
		double res = sigmoid(hidden_in[i]);
		hidden_out.push_back(res);
	}

	//����������
	double output_in=0;
	for (int i = 0;i < hidenode;i++) {
		output_in += 1.0*hidden_out[i] * hidden_w[i];
	}
	
	//��������� 
	output=output_in;

}


//�����������򴫲�,����Ȩֵ
void backPropagationEpoc(double rightout,double output,vector<double> hidden_out,vector<double> sample) {

	double mse = 1.0*(rightout - output)*(rightout - output);
	train_loss+=mse;
	//���ز�->������Ȩֵ���²���
	vector<double> renew_hw;
	renew_hw.clear();
	double hw = 1.0*(rightout- output);
	for (int i = 0;i < hidenode;i++) {
		double hw1 = hw*hidden_out[i];
		renew_hw.push_back(hw1);
	}
	//�����-�����ز��Ȩֵ���²��� 
	for (int i = 0;i < innode+1;i++) {
		for (int j = 0;j < hidenode;j++) {
			double iw = 1.0*hw*hidden_w[j] * hidden_out[j] * (1 - hidden_out[j])*sample[i];
			input_w[i][j] = input_w[i][j] + 1.0*alpha*iw / (innode+1);
		}
	}
//�������ز�->������Ȩֵ 
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
	get_train("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab6\\train.csv",3);
	get_test("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab6\\test.csv");	
	normalization(train_x);
	int iteration = 3000;
	trainning(iteration);
	predict(test);
	
//	predict(test20);
//	//20�����ݼ���Ԥ��ֵ����ʵֵ�ĶԱ�
//	ofstream out1;
//	out1.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab6\\res1.csv",ios::out);
//	int row_of_mse=result.size();
//	for(int r=0;r<row_of_mse;r++){
//		out1 << result[r] << ",";	
//	}
//	out1<<"\n";
//	out1.close();
//	ofstream out2;
//	out2.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab6\\res2.csv",ios::out);
//	int row_of_vali=test20.size();
//	for(int r=0;r<row_of_vali;r++){
//		out2 << testres[r]<<",";
//	}
//	out2.close(); 
	
	
	ofstream outFile;
	outFile.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab6\\data.txt",ios::out);
	int row_of_res=result.size();
	for(int r=0;r<row_of_res;r++){
		outFile << result[r] << "\n";	
	}
	outFile.close();
	
	
/*���train_loss��vali_loss	*/
//	ofstream out1;
//	out1.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab6\\mse.csv",ios::out);
//	int row_of_mse=MSE.size();
//	for(int r=0;r<row_of_mse;r++){
//		out1 << MSE[r] << ",";	
//	}
//	out1<<"\n";
//	out1.close();
//	ofstream out2;
//	out2.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab6\\mse1.csv",ios::out);
//	int row_of_vali=vali_mse.size();
//	for(int r=0;r<row_of_vali;r++){
//		out2 << vali_mse[r]<<",";
//	}
//	out2.close();

	
	
    return 0;
}


