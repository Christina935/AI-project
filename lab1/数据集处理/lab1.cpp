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
using namespace std;
 
vector<string> result; //存放切割出来的文本 
vector<string> Diff;   //存放不同的单词 
vector<vector<int> > bridge; //记录每个文本中每个单词出现的次数 
vector<vector<int> > one_hot; 
vector<vector<double> > tf;
vector<vector<double> > tf_idf;
//将每行文本切割出来的单词放在一个vector中，在将vector放在word中 
vector<vector<string> > word; 


//读取文件，将每行文本放入result中 
void GetString(const char *filename)
{
	ifstream ReadFile;
	string temp;  //将读取的文本放到temp中去 
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()){
		return ;
	}else{
		//一行一行地读取文件，将每行文本放到temp中 
		while(getline(ReadFile,temp)){ 
			int size = temp.size();
			string pattern="	";
			//以tap符为标记来获得我们需要的内容【第二个tap之后的内容】 
			int pos;
			pos = temp.find(pattern,0);
			pos = temp.find(pattern,pos+1); //找到第二个tap 
			string s = temp.substr(pos+1,size);  //将第三块内容切割出来 
			result.push_back(s); //将切割出来的文本放入result中 
		}
	}	
	ReadFile.close();
} 
 
//获得不同的单词 
void getDiff()
{
	vector<string> v;
	for(int i=0;i<result.size();i++){
		string str = result[i];
		int pos;
		string pattern = " ";//以空格符为标记将文本中切割成单词 
		str+=pattern;
		int size = str.size();
		for(int j=0; j<size; j++){
			pos = str.find(pattern,j); //从位置j开始在str中找到空格的位置 
			if(pos<size){
				string s = str.substr(j,pos-j); // 截取从第j位开始的长度为pos-j的字符串(单词） 
				v.push_back(s);
				vector<string>::iterator it;
				it = find(Diff.begin(),Diff.end(),s); 
				//在存放不同单词的vector Diff中找切割出来的单词 
				if(it==Diff.end()){ //如果在Diff中没有这个单词，则把这个单词加进去 
					Diff.push_back(s);
				}
				j=pos+pattern.size()-1;
			}
		}
		word.push_back(v);
		v.clear();
	}
} 
 
//获得每个文本中每个单词的个数 
void getBridge()
{
	int size = result.size();
	vector<int> v;
	int size_of_diff = Diff.size();
	for(int i=0;i<size;i++){
		for(int j=0; j<size_of_diff;j++){
			vector<string>::iterator it;
			it = find(word[i].begin(),word[i].end(),Diff[j]);//在每行文本的单词中寻找Diff中的单词 
			if(it==word[i].end()){
				v.push_back(0); //找不到记录为0 
			}else{
				bool num=false;
				while(it!=word[i].end()){
					it=word[i].erase(it);
					if(num==false){
						v.push_back(1);//第一次找到，记录为1 
					} 
					else{
						v[j]++;	//不是第一次找到，则把原来记录的数字++ 
					} 
					it = find(word[i].begin(),word[i].end(),Diff[j]);
					num=true;
				}	
			}
		}
		bridge.push_back(v);//将在每行文本中寻找Diff的结果v放入bridge中 
		v.clear();	
	}
} 

//获得one_hot矩阵 
//直接遍历bridge矩阵，大于0则记录为1，等于0则记录为0 
void get_onehot()
{
	int size_of_row=bridge.size();
	int size_of_column=bridge[0].size();
	vector<int> v;
	for(int i=0;i<size_of_row;i++){
		for(int j=0;j<size_of_column;j++){
			if(bridge[i][j]>=1){
				v.push_back(1);
			}
			else if(bridge[i][j]==0) v.push_back(0);
		}
		one_hot.push_back(v);
		v.clear();
	}
}

//获得tf矩阵 
void get_tf()
{
	int size_of_row=bridge.size();
	int size_of_column=bridge[0].size();
	vector<double> v;
	double num;
	for(int i=0;i<size_of_row;i++){
		int Denominator=0;
		for(int k=0;k<size_of_column;k++){
			Denominator+=bridge[i][k];//计算tf公式的分母(文本的单词总数）
			//也可以通过word获得 
		}
		//Denominator = word[i].size(); 
		for(int j=0;j<size_of_column;j++){
			if(bridge[i][j]>0){
				num=1.0*bridge[i][j]/Denominator;
				v.push_back(num);
			} 
			else if(bridge[i][j]==0) v.push_back(0);
		}
		tf.push_back(v);
		v.clear();
	}
}

//获取tf_idf矩阵 
void get_tfidf()
{
	int D=result.size();
	vector<double> idf;
	int size_of_row=one_hot.size();
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
	int size_of_idf=idf.size(); 
	for(int i=0;i<size_of_row;i++){
		for(int j=0;j<size_of_column;j++){
			 double tfidf =1.0*idf[j]*tf[i][j];//tf_idf = idf*tf 
			 v.push_back(tfidf);
		}
		tf_idf.push_back(v);
		v.clear();
	}
} 

typedef struct{
	int row;//非零数据的行 
	int column;//非零数据的列 
	int num;//非零数据的值 
}triple; //三元顺序表 

typedef struct{
	int num_of_row;//稀疏矩阵的行数 
	int num_of_column;//稀疏矩阵的列数 
	int no_zero;//非零数据的个数 
	vector<triple> data;//记录非零数据的行、列和数值 
}matric;

matric smatric;

void get_smatric()
{
	smatric.num_of_row=result.size();
	smatric.num_of_column=Diff.size();
	smatric.no_zero=0;
	int size_of_row=one_hot.size();
	int size_of_column=one_hot[0].size();
	triple tri;
	for(int i=0;i<size_of_row;i++){
		for(int j=0;j<size_of_column;j++){
			 if(one_hot[i][j]!=0){
			 	tri.row=i;
			 	tri.column=j;
			 	tri.num=one_hot[i][j];
			 	smatric.data.push_back(tri);
			 	smatric.no_zero++;
			 }
		}	
	}	
} 


int main()
{
	GetString("F:\\学习资料\\大三上\\人工智能\\实验\\实验1\\semeval.txt");
	cout << "semeval的行数是："<< result.size() << endl; 
	getDiff();
	int size = Diff.size();
	cout << "semeval的列数" << size << endl;
	getBridge();
	get_onehot();
	get_tf();
	get_tfidf();
	
	ofstream in;
	in.open("F:\\学习资料\\大三上\\人工智能\\实验\\实验1\\onehot.txt",ios::trunc);
	int size_of_row=one_hot.size();
	int size_of_column=one_hot[0].size();
	for(int i=0;i<size_of_row;i++){
		for(int j=0;j<size_of_column;j++){
			in<<one_hot[i][j]<<" ";  
	//		cout << left << setw(10) << one_hot[i][j] << " ";
		}
		in<<"\n";
	//	cout << endl;
	}
	in.close();
	
	ofstream in_tf;
	in_tf.open("F:\\学习资料\\大三上\\人工智能\\实验\\实验1\\tf.txt",ios::trunc);
	for(int i=0;i<size_of_row;i++){
		for(int j=0;j<size_of_column;j++){
			in_tf<<tf[i][j]<<" ";  
	//		cout << left << setw(10) << tf[i][j] << " ";
		}
		in_tf<<"\n";
	//	cout << endl;
	}
	in_tf.close();
	
	ofstream in_tfidf;
	in_tfidf.open("F:\\学习资料\\大三上\\人工智能\\实验\\实验1\\tfidf.txt",ios::trunc);
	for(int i=0;i<size_of_row;i++){
		for(int j=0;j<size_of_column;j++){
			in_tfidf<<tf_idf[i][j]<<" ";  
	//		cout << left << setw(10) << tf_idf[i][j] << " ";
		}
		in_tfidf<<"\n";
	//	cout << endl;
		
	}
//	cout <<"tfidf="<< tf_idf[1][30]<< endl;
	in_tfidf.close();

	get_smatric();
	ofstream in_sma;
	in_sma.open("F:\\学习资料\\大三上\\人工智能\\实验\\实验1\\smatric.txt",ios::trunc);
	in_sma << smatric.num_of_row << "\n" << smatric.num_of_column << "\n" << smatric.no_zero << "\n";
//	cout << matric.num_of_row << endl;
//	cout << matric.num_of_column<< endl;
//	cout << matric.no_zero << endl;
	int size_of_tri = smatric.data.size();
	for(int i=0;i<size_of_tri;i++){
		in_sma << smatric.data[i].row << " " << smatric.data[i].column << " " << smatric.data[i].num << "\n";
//		cout << matric.data[i].row << " " << matric.data[i].column << " " << matric.data[i].num << endl;
	}
	return 0;
	
	
} 
