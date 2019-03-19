#include<iostream>
#include<string.h>
#include<string>
#include<stdio.h>
#include<iomanip>
#include<iostream> 
#include<fstream>
#include<vector>
#include<algorithm> 
#include<cmath>
#include <sstream>    //使用stringstream需要引入这个头文件  
#include <time.h>
#include<stdlib.h>
using namespace std;

vector<vector<double> > train;
vector<vector<double> > test; 
vector<vector<double> > vali;
vector<double> train_label;
vector<double> vali_label; 
vector<double> w;
vector<double> res;

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
	string pattern=",";
	temp+=pattern;
	int size=temp.size(),pos;
	for(int i=0;i<size;i++){
		pos=temp.find(pattern,i);
		if(pos<size){
			string s=temp.substr(i,pos-i);
			double x=stringToDouble(s);
			xi.push_back(x);
			i=pos; // i=pos+pattern.size()-1; 
		}
	}
	return xi;
}   


/*作用：读取train文件，将train的每一行切割成一个个单词，放入train中，结果y放入train_y中*/ 
void get_train(const char *filename,int n)
{
	vector<double> data;  
	vector<vector<double> > attr; 
	ifstream ReadFile;
	string temp;
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()){
		return;
	}
	else{
		while(getline(ReadFile,temp)){
			vector<double> xi;
			xi=get_xi(temp);
			xi.insert(xi.begin(),1); //插入常数项1 
			data.push_back(xi[xi.size()-1]);//最后一项即为label 
			xi.pop_back(); //抛掉最后一项 
			attr.push_back(xi); 
		}	
	}
	ReadFile.close();
	int data_size=data.size(),j=0;
	for(int i=0;i<data_size;i++){
		if(j<n){
			train_label.push_back(data[i]);
			train.push_back(attr[i]);
			j++;
		}
		else if(j==n){
			vali_label.push_back(data[i]);
			vali.push_back(attr[i]);
			j=1;
		}	
	}
}

/*作用：读取test文件，将test每行文本切割成一个个单词，放入vali*/ 
void get_test(const char *filename)
{
	ifstream ReadFile;
	string temp;
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()) return;
	else{
		while(getline(ReadFile,temp)){
			vector<double> xi;
			xi=get_xi(temp);
			xi.insert(xi.begin(),1);
			xi.pop_back();
			test.push_back(xi);
		}	
	}
	ReadFile.close();
}

void renew_w()
{
	double alpha=1;//设定步长（学习率） 
	int train_row = train.size(),train_column=train[0].size();
	int num_of_iterator=0;//迭代次数 
	vector<double> new_w;
	while(num_of_iterator<500){
		int w_size=w.size();
		srand(num_of_iterator);//设定随机数种子
		int i=rand()%train_row;  //随机选择一行来更新w 
		
		for(int k=0;k<w_size;k++){
			/***************计算该维度的梯度***********/
			double Err=0;
			double r=0;
			for(int j=0;j<train_column;j++){
			//计算Err的前部分 	
				r=r+w[j]*train[i][j];
			}
//				double h=1.0/(1+exp(-r));
//				double likelihood=1.0*train_label[i]*log(h)+1.0*(1-train_label[i])*log(1-h);
//				cout << "对数似然likehood=" << likelihood << endl;
			
			Err=1.0*(1.0/(1+exp(-r))-train_label[i])*train[i][k];
			/***************计算该维度的梯度***********/
			new_w.push_back(w[k]-1.0*alpha*Err);	 
		}
		w.clear();
		w.assign(new_w.begin(),new_w.end());
		new_w.clear();
		num_of_iterator++;	
	}
	
}

void predict(vector<vector<double> > v)
{
	res.clear();
	int row=v.size(),column=v[0].size();
	for(int i=0;i<row;i++){
		double r=0;
		for(int j=0;j<column;j++){
			r=r+w[j]*v[i][j];
		}
		double pos=1.0/(1+exp(-r));
		if(pos>=0.5) res.push_back(1);
		else res.push_back(0);
 	}
}

double accurancy()
{
	int row=vali_label.size(),num=0;;
	for(int i=0;i<row;i++){
		if(vali_label[i]==res[i]) num++;
	} 
	double accu=1.0*num/row;
	return accu;
}

int main()
{
	get_train("F:\\学习资料\\大三上\\人工智能\\实验\\lab5\\train.csv",4);
	int train_row=train.size(),train_column=train[0].size();
	int w_size=train[0].size();
	for(int i=0;i<w_size;i++){
		w.push_back(1);
	}	
	renew_w();
	/******************vali**************************/ 
//	predict(vali);
//	double acc=accurancy();
//	cout << "准确率=" << acc << endl; 
	/******************vali**************************/
	/******************test**************************/
	get_test("F:\\学习资料\\大三上\\人工智能\\实验\\lab5\\test.csv");
	predict(test);
	ofstream outFile;
	outFile.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab5\\15352154_laipingping.txt",ios::out);
	int row_of_res=res.size();
	for(int r=0;r<row_of_res;r++){
		outFile << res[r] << "\n";	
	}
	outFile.close();
	return 0;
}
