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
#include <sstream>    //ʹ��stringstream��Ҫ�������ͷ�ļ�  
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


/*���ã���ȡtrain�ļ�����train��ÿһ���и��һ�������ʣ�����train�У����y����train_y��*/ 
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
			xi.insert(xi.begin(),1); //���볣����1 
			data.push_back(xi[xi.size()-1]);//���һ�Ϊlabel 
			xi.pop_back(); //�׵����һ�� 
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

/*���ã���ȡtest�ļ�����testÿ���ı��и��һ�������ʣ�����vali*/ 
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
	double alpha=1;//�趨������ѧϰ�ʣ� 
	int train_row = train.size(),train_column=train[0].size();
	int num_of_iterator=0;//�������� 
	vector<double> new_w;
	while(num_of_iterator<500){
		int w_size=w.size();
		srand(num_of_iterator);//�趨���������
		int i=rand()%train_row;  //���ѡ��һ��������w 
		
		for(int k=0;k<w_size;k++){
			/***************�����ά�ȵ��ݶ�***********/
			double Err=0;
			double r=0;
			for(int j=0;j<train_column;j++){
			//����Err��ǰ���� 	
				r=r+w[j]*train[i][j];
			}
//				double h=1.0/(1+exp(-r));
//				double likelihood=1.0*train_label[i]*log(h)+1.0*(1-train_label[i])*log(1-h);
//				cout << "������Ȼlikehood=" << likelihood << endl;
			
			Err=1.0*(1.0/(1+exp(-r))-train_label[i])*train[i][k];
			/***************�����ά�ȵ��ݶ�***********/
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
	get_train("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab5\\train.csv",4);
	int train_row=train.size(),train_column=train[0].size();
	int w_size=train[0].size();
	for(int i=0;i<w_size;i++){
		w.push_back(1);
	}	
	renew_w();
	/******************vali**************************/ 
//	predict(vali);
//	double acc=accurancy();
//	cout << "׼ȷ��=" << acc << endl; 
	/******************vali**************************/
	/******************test**************************/
	get_test("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab5\\test.csv");
	predict(test);
	ofstream outFile;
	outFile.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab5\\15352154_laipingping.txt",ios::out);
	int row_of_res=res.size();
	for(int r=0;r<row_of_res;r++){
		outFile << res[r] << "\n";	
	}
	outFile.close();
	return 0;
}
