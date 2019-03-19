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

vector<vector<double> >train_emotion;//存放训练集的情绪系数,依次是 anger、disgust、fear、joy、sad、surprise
vector<string> Diff; //存放不同的单词
vector<vector<string> > train;   //存放train训练集 
vector<vector<double> > one_hot; //one-hot矩阵：记录单词是否出现 
vector<vector<double> > bridge;  //bridge矩阵：记录单词出现次数 
vector<vector<string> > validation; //存放验证集validation 或测试集test 
vector<vector<double> > vali_emotion;//存放验证集validation的情绪系数
vector<vector<double> > result_emotion;//存放通过模型得出的验证集validation的情绪系数 
vector<vector<double> > tf; 
vector<vector<double> > tf_idf;

/*作用：将一个字符串str切割成一个个单词，在str中，单词以空格分割*/  
vector<string> get_word(string temp)
{
	vector<string> v;
	string pattern=" ";//单词以空格分开 
	temp+=pattern;
	int size=temp.size(),pos;
	for(int i=0;i<size;i++){
		pos=temp.find(pattern,i); //从第i个位置开始在temp中寻找空格的位置 
		if(pos<size){
			string s=temp.substr(i,pos-i);//切割出来的单词 
			v.push_back(s);
			vector<string>::iterator it;
			it=find(Diff.begin(),Diff.end(),s); //在Diff(单词集）中寻找这个单词 
			if(it==Diff.end()){
				Diff.push_back(s); //如果Diff中没有该单词，则将该单词放入Diff中 
			} 
			i=pos+pattern.size()-1;//将i移到找到的空格之后的位置 
		}
	}
	return v;
}

/*作用：将string类型的数据转成double型 */
double stringToDouble(const string& str)  
{  
    istringstream iss(str);  
    double num;  
    iss >> num;  
    return num;      
}   
 
/*作用：读取trainning文件，将每行文本的单词放入train中，情绪系数放在train_emotion中*/
void GetTrain(const char *filename)
{
	ifstream ReadFile;
	string temp; //将读取的每行文本放到temp中
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()){
		return ;
	}else{
		bool first=true;
		while(getline(ReadFile,temp)){
		//csv文件读取出来每行内容的形式：“str1,str2,str3",即每格的内容以，隔开 
			if(first==true) first=false;//不读取文件中的第一行 
			else{
				string s1=",";
				int pos,size=temp.size();
				pos=temp.find(s1,0);//找到，的位置，第一个逗号之前是文本内容 
				string words=temp.substr(0,pos);
				temp.erase(0,pos+1);//删除words 
				vector<string> v = get_word(words); //将words切割成一个个单词 
				train.push_back(v);
				v.clear();
				//收集情绪系数 anger、disgust、fear、joy、sad、surprise
				int num=6;
				double e;
				vector<double> label;
				while(num--){
					pos=temp.find(s1,0); 
					string em=temp.substr(0,pos);
					temp.erase(0,pos+1); 
					e=stringToDouble(em);
					label.push_back(e);
				}
				train_emotion.push_back(label);
				label.clear();
			}
		}
	}
	ReadFile.close();
} 


/*作用：
读取validation验证集文件，将每行文本的单词放入validation中，情绪系数放到vali_emotion中*/ 
void GetVal(const char *filename)
{
	ifstream ReadFile;
	string temp; //将读取的每行文本放到temp中
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()){
		return ;
	}else{
		bool first=true;
		while(getline(ReadFile,temp)){  //每一行中每一格的内容以，分开 
			if(first==true) first=false;
			else{
				string s1=",";
				int pos,size=temp.size();
				pos=temp.find(s1,0);//找到，的位置，第一个逗号之前是文本内容 
				string words=temp.substr(0,pos);
				temp.erase(0,pos+1);//删除words 
				vector<string> v = get_word(words);
				validation.push_back(v);
				v.clear();
				//收集情绪系数 anger、disgust、fear、joy、sad、surprise
				int num=6;
				double e;
				vector<double> label;
				while(num--){
					pos=temp.find(s1,0); 
					string em=temp.substr(0,pos);
					temp.erase(0,pos+1); 
					e=stringToDouble(em);
					label.push_back(e);
				}
				vali_emotion.push_back(label);
				label.clear();
			}
		}
	}
	ReadFile.close();
} 

/*作用：
读取test测试集文件，将每行文本的单词放入test中，情绪系数放到vali_emotion中*/ 
void GetTest(const char *filename)
{
	ifstream ReadFile;
	string temp; //将读取的每行文本放到temp中
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()){
		return ;
	}else{
		bool first=true;
		while(getline(ReadFile,temp)){  //每一行中每一格的内容以，分开 
			if(first==true) first=false;
			else{
				string s1=",";
				int pos,size=temp.size();
				pos=temp.find(s1,0);//找到，的位置，第一个逗号之前是序号 
				temp.erase(0,pos+1);   //删除序号和第一个逗号 
				string words=temp.substr(0,pos);
				vector<string> v = get_word(words);
				validation.push_back(v);
				v.clear();
			}
		}
	}
	ReadFile.close();
} 
 
 
/*作用：获得one-hot和bridge矩阵
one-hot:记录单词是否出现
bridge：记录单词出现的次数 */  
void get_one_bri()
{
	int column=Diff.size(),row1=train.size(),row2=validation.size(),row=row1+row2,k;//one_hot矩阵的行数和列数 
	vector<double> v; //记录one-hot矩阵的一行 
	vector<double> v1; //记录bridge矩阵的一行 
	for(int i=0;i<row;i++){
		for(int j=0;j<column;j++){
			if(i<row1){                //0~（row1-1）行:train训练集的部分 
				vector<string>::iterator it;
				//在每行文本的单词中找Diff中的单词 
				it=find(train[i].begin(),train[i].end(),Diff[j]);
				if(it==train[i].end()){ //找不到记录为0 
					v.push_back(0);
					v1.push_back(0);    
				}else{     //找到记录为1 
					v.push_back(1);
					bool first=true; //记录是否是第一次找到 
					//在该行文本中能够找到该单词，则把该单词删除，然后继续寻找该单词，直到找不到为止 
					while(it!=train[i].end()){
						it=train[i].erase(it);
						if(first==true){
							v1.push_back(1); //第一次找到，记录为1 
						}else{
							v1[j]++;//不是第一次找到，把原来记录的数字++ 
						}
						it=find(train[i].begin(),train[i].end(),Diff[j]);
						first=false;
					}
				}	
			}else {
				k=i-row1;  //row1-row行：test测试集 
				vector<string>::iterator it;
				//在每行文本的单词中找Diff中的单词 
				it=find(validation[k].begin(),validation[k].end(),Diff[j]);
				if(it==validation[k].end()){ //找不到记录为0 
					v.push_back(0);
					v1.push_back(0);
				}else{     //找到记录为1 
					v.push_back(1);
					bool first=true; //记录是否是第一次找到 
					while(it!=validation[k].end()){
						it=validation[k].erase(it);
						if(first==true){
							v1.push_back(1); //第一次找到，记录为1 
						}else{
							v1[j]++;//不是第一次找到，把原来记录的数字++ 
						}
						it=find(validation[k].begin(),validation[k].end(),Diff[j]);
						first=false;
					}
				}	
			}	
		}
		//将在每行文本中寻找Diff的结果放入one-hot矩阵中 
		one_hot.push_back(v);
		v.clear();
		bridge.push_back(v1);
		v1.clear(); 
	}
} 

/*作用：获得train_tf矩阵 
tf = (单词出现次数)/（文本单词总数）*/  
void get_tf()
{
	int size_of_row=bridge.size();
	int size_of_column=bridge[0].size();
	vector<double> v;
	double num;
	double a=0.01;
	for(int i=0;i<size_of_row;i++){
		double Denominator=Diff.size()*a;	
	//	int Denominator=0;
		for(int k=0;k<size_of_column;k++){
			Denominator+=bridge[i][k];//计算tf公式的分母(文本的单词总数）
		}
		for(int j=0;j<size_of_column;j++){
			if(bridge[i][j]>0){
		//	num=1.0*bridge[i][j]/Denominator;
				num=1.0*(bridge[i][j]+1*a)/Denominator;
			v.push_back(num);
			} 
			else if(bridge[i][j]==0){
			/*	v.push_back(0);*/
				num = 1.0*a/Denominator;
				v.push_back(num);
			} 
		}
		tf.push_back(v);
		v.clear();
	}
}

/*作用：获取tf_idf矩阵 
idf = (文本总数)/(出现该单词的文本数+1)
tfidf[i][j] = tf[i][j]*idf[i]    
void get_tfidf()
{
	int D=train.size();
	vector<double> idf;
	int size_of_row=D;
	int size_of_column=one_hot[0].size();
	for(int j=0;j<size_of_column;j++){
		int t=0;
		for(int i=0;i<size_of_row;i++){
			t+=one_hot[i][j];
			//通过计算将one_hot 每列的总数算出可知一个单词出现在多少篇文本中 
		}
		t+=1;
		double res = log2(1.0*D/t);//D——文本总数 ，res是idf的数据 
		idf.push_back(res);
	}
	vector<double> v;
	int row=D+validation.size(); 
	for(int i=0;i<row;i++){
		for(int j=0;j<size_of_column;j++){
			 double tfidf =1.0*idf[j]*tf[i][j];//tf_idf = idf*tf 
			 v.push_back(tfidf);
		}
		tf_idf.push_back(v);
		v.clear();
	}
} */

/*计算validation/test每行文本中每种情感系数*/
void get_result()
{
	int row_vali = validation.size(),row_train=train.size(),size_diff=Diff.size();
	int num_emotion=6;
	vector<int> Diff_pos;//记录validation一行文本出现的单词在Diff的位置 
	vector<double> v; //记录一行文本的情感系数 
	for(int i=0;i<row_vali;i++){ //遍历validation的每一行文本  
		Diff_pos.clear();
		for(int j=0;j<size_diff;j++){ //找出validation一行文本出现的单词在Diff的位置 
			if(one_hot[i+row_train][j]!=0) Diff_pos.push_back(j);  
		}
		int size_pos=Diff_pos.size();
		double row_p=1.0,train_p=0;
		for(int b=0;b<num_emotion;b++){ //遍历六种情感 
			train_p=0;
			for(int m=0;m<row_train;m++){  //遍历train的每一行 
				row_p=1.0;  
				for(int k=0;k<size_pos;k++){
					row_p=1.0*row_p*tf[m][Diff_pos[k]];  
				}
				row_p=1.0*row_p*train_emotion[m][b]; 
				train_p+=row_p;
			}
			v.push_back(train_p); 
		}
		double all_p=0;
		for(int j=0;j<num_emotion;j++){
			all_p+=v[j];
		}
		if(all_p!=0){
			for(int j=0;j<num_emotion;j++){
				v[j]=1.0*v[j]/all_p;
			} 
		}
		result_emotion.push_back(v);
		v.clear();	
	}
}

int main()
{
	GetTrain("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\regression_dataset\\train_set.csv");
//	GetVal("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\regression_dataset\\validation_set.csv");
	GetTest("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\regression_dataset\\test_set.csv");
	get_one_bri();
	get_tf();
	get_result();
	ofstream outFile;
	outFile.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\regression_dataset\\15352154_Sample_NB_regression.csv",ios::out);
	int row_of_res=result_emotion.size(),column_of_res=6;
	outFile << "textid" << ',' << "anger" << ',' << "disgust" << ',' << "fear" << ',' << "joy" << ',' << "sad" << ',' << "surprise" << "\n";
	for(int r=0;r<row_of_res;r++){
		outFile << (r+1) << ',';
		for(int c=0;c<column_of_res;c++){
			outFile << result_emotion[r][c] << ',';	
		}
		outFile << "\n";
	}
	outFile.close();
	return 0;
}
