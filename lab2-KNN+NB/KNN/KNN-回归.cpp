/* 使用KNN处理回归问题
 在验证集上通过调节K值，选择不同距离等方式得到一个准确率最优的模型参数 */
 
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
vector<vector<string> > validation; //存放验证集validation 
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
	for(int i=0;i<size_of_row;i++){
		int Denominator=0;
		for(int k=0;k<size_of_column;k++){
			Denominator+=bridge[i][k];//计算tf公式的分母(文本的单词总数）
		}
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

/*作用：获取tf_idf矩阵 
idf = (文本总数)/(出现该单词的文本数+1)
tfidf[i][j] = tf[i][j]*idf[i]  */  
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
} 

typedef struct{
	double dis;
	int pos;
}dis_pos; 

bool comparationsmall(dis_pos d1,dis_pos d2)
{
	return d1.dis<d2.dis;
}

bool comparationbig(dis_pos d1,dis_pos d2)
{
	return d1.dis>d2.dis;
}

void get_result(int k,string dis_way,vector<vector<double> > two)
{
	dis_pos res;
	int row1=train.size(),row2=validation.size(),row=row1+row2;
	for(int i=row1;i<row;i++){ //遍历验证集one-hot每一行 
		vector<dis_pos> distance;  //记录验证集中的一行和训练集的每一行到的距离 
		for(int j=0;j<row1;j++){ //遍历训练集one-hot每一行，算出验证集的一行和训练集每一行的距离 
			double dis=0; 
			if(dis_way=="Euclidean"){  //欧氏距离 
				for(int g=0;g<Diff.size();g++){
					dis+=(two[i][g]-two[j][g])*(two[i][g]-two[j][g]);
				}
				dis=sqrt(dis);	 	
			}else if(dis_way=="CityBlock"){ //曼哈顿距离 
				for(int g=0;g<Diff.size();g++){
					if(two[i][g]>two[j][g])
					dis+=two[i][g]-two[j][g];
					else dis+=two[j][g]-two[i][g];
				}	 
			}else if(dis_way=="3-th"){
				for(int g=0;g<Diff.size();g++){
					double d;
				 	if(two[i][g]>two[j][g]) d=two[i][g]-two[j][g];
				 	else d=two[j][g]-two[i][g];
				 	dis+=pow(d,3);
				}
			 	dis=pow(dis,1.0/3);
			}else if(dis_way=="cos"){ //余弦相似度 
				double ab,a,b;
				for(int g=0;g<Diff.size();g++){
					ab=ab+1.0*two[i][g]*two[j][g];
				 	a=a+1.0*two[i][g]*two[i][g];
				 	b=b+1.0*two[j][g]*two[j][g];
				}
				dis = 1.0*ab/(1.0*sqrt(a)*sqrt(b));	
			}
			res.dis = dis;
			res.pos = j;
			distance.push_back(res); //验证集的一行 与 train的每一行的距离 
		}
		
		/*线性函数归一化*/
		int row_of_dis=distance.size();
		double min=100000,max=0;
		for(int m=0;m< row_of_dis;m++){
			if(distance[m].dis>max) max=distance[m].dis;
			if(distance[m].dis<min) min=distance[m].dis;
		}
		for(int m=0;m<row_of_dis;m++){
			if(max!=min)
			distance[m].dis = 1.0*(distance[m].dis-min)/(max-min) ;
		}
		/*余弦相似度：余弦值为正且值越大，表示两个文本差距越小；为负代表差距大
		 所有余弦相似度按从大到小排序 ，其他距离按从小到大排序 */ 
		if(dis_way=="cos") sort(distance.begin(),distance.end(),comparationbig); 
		else sort(distance.begin(),distance.end(),comparationsmall); 
	//	for(int g=0;g<k;g++) cout <<"pos=" <<  distance[g].pos <<"   dis=" << distance[g].dis<< endl;
		int column_of_emotion=6;
		vector<double> em;
		/*余弦相似度：余弦值为正且值越大，表示两个文本差距越小；为负代表差距大
		 所有余弦相似度越大 则 权重就应该越大，所以用* ；
		 其他距离 距离越小，则权重应该越大，所以用倒数 */ 
		if(dis_way=="cos"){
			for(int c=0;c<column_of_emotion;c++){
				double p=0; 
				for(int r=0;r<k;r++){
					p+=1.0*train_emotion[distance[r].pos][c]*(distance[r].dis);
				}
				em.push_back(p);
			}	
		}else{
			for(int c=0;c<column_of_emotion;c++){
				double p=0; 
				for(int r=0;r<k;r++){
					p+=1.0*train_emotion[distance[r].pos][c]/(distance[r].dis+1);
					//分母为距离+1,防止出现分母为0的情况 
				}
				em.push_back(p);
			}
		}
		result_emotion.push_back(em);
		em.clear();
		distance.clear();
	}
		int row_of_res=result_emotion.size(),column_of_res=6;
		/*整体归一化*/
		for(int r=0;r<row_of_res;r++){
			double all=0; 
			for(int c=0;c<column_of_res;c++){
				all+=result_emotion[r][c];  //计算每个测试样本所有情绪的概率的总和 
			}
			for(int h=0;h<column_of_res;h++){
				/*让每个情绪的概率除以该测试样本情绪的概率总和
				 这样同一测试样本的各个情绪概率总和为1 */
				if(all!=0) result_emotion[r][h]/=all; 
			}
		}	
} 

int main()
{
	GetTrain("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\regression_dataset\\train_set.csv");
//	GetVal("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\regression_dataset\\validation_set.csv");
	GetTest("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\regression_dataset\\test_set.csv");
	get_one_bri();
	get_tf();
	get_tfidf();
	string dis_way[4]={"CityBlock","Euclidean","3-th","cos"};
	int k=11; 
	get_result(k,dis_way[3],tf_idf);
	
	ofstream outFile;
	outFile.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\regression_dataset\\15352154_Sample_KNN_regression.csv",ios::out);
	int row_of_res=result_emotion.size(),column_of_res=6;
//	outFile << "textid" << ',' << "anger" << ',' << "disgust" << ',' << "fear" << ',' << "joy" << ',' << "sad" << ',' << "surprise" << "\n";
	for(int r=0;r<row_of_res;r++){
//		outFile << (r+1) << ',';
		for(int c=0;c<column_of_res;c++){
			outFile << result_emotion[r][c] << ',';	
		}
		outFile << "\n";
	}
	outFile.close();
	result_emotion.clear();	
} 
