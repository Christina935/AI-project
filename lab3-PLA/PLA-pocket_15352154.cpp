/*口袋PLA算法*/ 

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
using namespace std;

vector<vector<double> > train;    /* 存放train的x */ 
vector<double> train_y;  /* 存放train的y */ 
vector<vector<double> > vali;     /* 存放validationx */ 
vector<vector<double> > test; 
vector<double> vali_y;   /* 存放validation或test的y */ 
vector<double> result_y;   /* 存放预测的y */ 
vector<double> w;                 /* 存放权重w */ 
double TP=0,FN=0,TN=0,FP=0;  
double Accuracy=0,Recall=0,Precision=0,F1=0;
vector<double> best_w;
double best_F1=0;

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
void get_train(const char *filename)
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
			train_y.push_back(xi[xi.size()-1]);
			xi.pop_back();
			train.push_back(xi);
		}	
	}
	ReadFile.close();
}

/*作用：读取validation文件，将validation每行文本切割成一个个单词，放入vali，结果y放入vali_y中*/ 
void get_vali(const char *filename)
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
			vali_y.push_back(xi[xi.size()-1]);
			xi.pop_back();
			vali.push_back(xi);
		}	
	}
	ReadFile.close();
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

/*作用：用更新的W来预测train的结果*/
double predict_train(vector<double> w1)
{
	TP=0,FN=0,TN=0,FP=0;
	int train_row=train.size(),train_column=train[0].size();
	for(int row_of_train=0;row_of_train<train_row;row_of_train++){	
		double predict_y=0;
		// predict_y= xi[n]*w[n]  
		for(int column_of_train=0;column_of_train<train_column;column_of_train++){
			predict_y+=1.0*train[row_of_train][column_of_train]*w1[column_of_train];
		}
		if(predict_y<0) predict_y=-1;
		else if (predict_y>0) predict_y=1;
		if(train_y[row_of_train]==1 && predict_y==1) TP++;
		else if(train_y[row_of_train]==1 && predict_y==-1) FN++;
		else if(train_y[row_of_train]==-1 && predict_y==-1) TN++; 
		else if(train_y[row_of_train]==-1 && predict_y==1) FP++;
	}
	double recall=1.0*TP/(TP+FN);
	double precision=1.0*TP/(TP+FP);
	double best_F1 =1.0*(2*precision*recall)/(precision+recall);
	return best_F1;  
}

/*作用：更新w
用w预测train的每一行的结果，如果预测出错，则更新w（w[n+1]=w[n]+x*y)
将更新w的F值和记录的最优的F值比较，大则更新best_w.*/
void renew_w(int num_of_predict)
{
	//此处可能可以优化：不一定每次都取前面几行，可以随机产生num_of_predict行 
	int train_row = train.size(),train_column=train[0].size();
	for(int row_of_train=0;row_of_train<train_row;row_of_train++){	
		if(num_of_predict>0){
			double cal_y=0;
			// 用w预测train的每一行的结果，cal_y= xi[n]*w[n]  
			for(int column_of_train=0;column_of_train<train_column;column_of_train++){
				cal_y+=1.0*train[row_of_train][column_of_train]*w[column_of_train];
			}  
			/*这里可能可以优化，考虑容错性，即能接收结果偏差多少*/ 
			if(cal_y<0) cal_y=-1;
			else if(cal_y>0) cal_y=1;
			//如果预测的结果和真实的结果不同的更新w 
			if(cal_y!=train_y[row_of_train]){
				for(int pos_w=0;pos_w<train_column;pos_w++){
				   w[pos_w]+=1.0*train[row_of_train][pos_w]*train_y[row_of_train];
 			    }
			    num_of_predict--;
			    row_of_train=-1;
			   	/*比较正确性*/
				double w_F1=predict_train(w);
				if(w_F1 > best_F1){			
					best_w.assign(w.begin(),w.end());
					best_F1=w_F1;
				}		    
			} 	
		
		}else{
			break;
		}
	}
}


/*作用：用完全更新好的W来预测vali或test的结果*/
double predict(vector<double> w1)
{
	TP=0,FN=0,TN=0,FP=0;
	int vali_row=vali.size(),vali_column=vali[0].size();
	for(int row_of_vali=0;row_of_vali<vali_row;row_of_vali++){	
		double predict_y=0;
		// predict_y= xi[n]*w[n]  
		for(int column_of_vali=0;column_of_vali<vali_column;column_of_vali++){
			predict_y+=1.0*vali[row_of_vali][column_of_vali]*w1[column_of_vali];
		}
		if(predict_y<0) predict_y=-1;
		else if (predict_y>0) predict_y=1;
	//	result_y.push_back(predict_y);
		if(vali_y[row_of_vali]==1 && predict_y==1) TP++;
		else if(vali_y[row_of_vali]==1 && predict_y==-1) FN++;
		else if(vali_y[row_of_vali]==-1 && predict_y==-1) TN++; 
		else if(vali_y[row_of_vali]==-1 && predict_y==1) FP++;
	}
	double recall=1.0*TP/(TP+FN);
	double precision=1.0*TP/(TP+FP);
	double best_F1 =1.0*(2*precision*recall)/(precision+recall);
	return best_F1;  
}


void standard()
{
	/*
	cout << "TP=" << TP << endl;
	cout << "TN=" << TN << endl;
	cout << "FP=" << FP << endl;
	cout << "FN=" << FN << endl;*/
	Accuracy=1.0*(TP+TN)/(TP+FP+TN+FN);
	Recall=1.0*TP/(TP+FN);
	Precision=1.0*TP/(TP+FP);
	F1=1.0*(2*Precision*Recall)/(Precision+Recall);
	cout << "Accuracy=" << Accuracy << endl;
	cout << "Recall=" << Recall << endl;
	cout << "Precision=" << Precision << endl;
	cout << "F1=" << F1 << endl; 
}

/*作用：用完全更新好的W来预测test的结果*/
void predict_test(vector<double> w1)
{
	int vali_row=vali.size(),vali_column=vali[0].size();
	for(int row_of_vali=0;row_of_vali<vali_row;row_of_vali++){	
		double predict_y=0;
		// predict_y= xi[n]*w[n]  
		for(int column_of_vali=0;column_of_vali<vali_column;column_of_vali++){
			predict_y+=1.0*vali[row_of_vali][column_of_vali]*w1[column_of_vali];
		}
		if(predict_y<0) predict_y=-1;
		else if (predict_y>0) predict_y=1;
		result_y.push_back(predict_y);
	}
}


int main()
{
	get_train("F:\\学习资料\\大三上\\人工智能\\实验\\lab3\\lab3数据\\train.csv");
	get_vali("F:\\学习资料\\大三上\\人工智能\\实验\\lab3\\lab3数据\\val.csv");
	get_test("F:\\学习资料\\大三上\\人工智能\\实验\\lab3\\lab3数据\\test.csv");

	int size_w=train[0].size();
	for(int i=0;i<size_w;i++){
		w.push_back(1);
		best_w.push_back(1); 
	}
	best_F1=predict_train(best_w);
//	cout << "初始best_F1=" << best_F1 << endl;
	int num_of_predict=3000;
	cout << "迭代次数=" << num_of_predict << endl; 
	renew_w(num_of_predict);
	predict(best_w);
	standard();	
	predict_test(best_w);
	ofstream outFile;
	outFile.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab3\\lab3数据\\PLA.csv",ios::out);
	int row_of_res=result_y.size();
	for(int r=0;r<row_of_res;r++){
		outFile << result_y[r] << "\n";	
	}
	outFile.close();
	return 0;
} 
