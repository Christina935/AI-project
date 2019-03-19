// 使用KNN处理分类问题
// 在验证集上通过调节K值，选择不同距离等方式得到一个准确率最优的模型参数 
 
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

typedef struct{
	double dis;
	int pos;
}dis_pos; 

vector<string> train_label; //存放训练集的标签（情绪）
vector<string> Diff; //存放不同的单词
vector<vector<string> > train;   //存放训练集train的单词 
vector<vector<double> > one_hot; //one-hot矩阵 
vector<vector<string> > validation; //存放验证集validation 
vector<string> validation_label; //验证集的label 
vector<string> result_label;//存放通过模型得出的验证集的结果标签 
vector<vector<double> >bridge; //记录训练集和验证集每个单词的出现次数
vector<vector<double> >tf; //训练集和验证集的tf矩阵
vector<vector<double> > tf_idf; //训练集和验证集的tf矩阵

/*作用：将一个字符串str切割成一个个单词，在str中，单词以空格分割*/
vector<string> get_word(string temp)
{
	//将一行文本切割成单词 
	vector<string> v;
	string pattern=" ";//单词以空格分开 
	temp+=pattern;
	int size=temp.size(),pos;
	for(int i=0;i<size;i++){
		pos=temp.find(pattern,i);
		if(pos<size){
			string s=temp.substr(i,pos-i);//切割出来的单词 
			v.push_back(s);
			vector<string>::iterator it;
			//在Diff(单词集）中寻找这个单词 
			it=find(Diff.begin(),Diff.end(),s); 
			if(it==Diff.end()){
				//如果Diff中没有该单词，则将该单词放入Diff中 
				Diff.push_back(s); 
			} 
			i=pos+pattern.size()-1;
		}
	}
	return v;
}

/*作用：
读取trainning文件，将每行文本的单词放入train中，情绪放在train_label中*/ 
void GetTrain(const char *filename)
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
				//切割出label 
				string s1=",";
				int pos,size=temp.size();
				pos=temp.find(s1,0);//找到，的位置，逗号之后的文本即为label 
				string emotion=temp.substr(pos+1,size);
				train_label.push_back(emotion);
				temp.erase(pos,size);//切割掉label，剩下为words 
				vector<string> v = get_word(temp);
				train.push_back(v);
				v.clear();
			}
		}
	}
	ReadFile.close();
} 

/*读取validation文件，将每行文本的单词放入validation中，情绪放在label中*/
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
				//切割出label 
				string s1=",";
				int pos,size=temp.size();
				pos=temp.find(s1,0);//找到，的位置，逗号之后的文本即为label 
				string emotion=temp.substr(pos+1,size);
				validation_label.push_back(emotion);
				temp.erase(pos,size);
				vector<string> v=get_word(temp);
				validation.push_back(v);
				v.clear();
			}
		}
	}
	ReadFile.close();
} 

/*作用：读取test文件，将每行文本的单词放入validation中*/ 
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
				pos=temp.find(s1,0);//找到，的位置 
				temp.erase(0,pos+1);
				pos=temp.find(s1,0);
				temp.erase(pos,size);
				vector<string> v=get_word(temp);
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
			if(i<row1){ //0~（row1-1）行:train训练集的部分 
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
			}else {   //row1-row行：test测试集 
				k=i-row1;
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

/*作用：获得tf矩阵 
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
	int size_of_column=Diff.size();
	for(int j=0;j<size_of_column;j++){
		int t=0;
		for(int i=0;i<size_of_row;i++){
			t+=one_hot[i][j];
			//通过计算将one_hot训练集部分 每列的总数
			//算出可知一个单词出现在多少篇文本中 
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

bool comparationsmall(dis_pos d1,dis_pos d2)
{
	return d1.dis<d2.dis;
}

bool comparationbig(dis_pos d1,dis_pos d2)
{
	return d1.dis>d2.dis;
}

typedef struct{
	int pos;
	double count;
}Counter;

bool comparation2(Counter c1,Counter c2)
{
	return c1.count>c2.count;
}

void get_result(int k,string dis_way,vector<vector<double> > two)
{
	dis_pos res; 
	int row1=train.size(),row2=validation.size(),row=row1+row2;
	for(int i=row1;i<row;i++){ //遍历验证集two矩阵每一行 
		vector<dis_pos> distance;  //记录验证集中的一行和训练集的每一行到的距离 
		vector<string> k_label; //记录选取的训练集中的K个最接近的文本的label 
		for(int j=0;j<row1;j++){ //遍历训练集one-hot每一行，算出验证集的一行和训练集每一行的距离 
			double dis=0;
			if(dis_way=="Euclidean"){
				for(int g=0;g<Diff.size();g++){
					dis+=(two[i][g]-two[j][g])*(two[i][g]-two[j][g]);
				}
				dis=sqrt(dis);// 算欧氏距离 	
			}else if(dis_way=="CityBlock"){
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
			}else if(dis_way=="cos"){
				double ab,a,b;
				for(int g=0;g<Diff.size();g++){
					ab+=1.0*two[i][g]*two[j][g];
				 	a+=1.0*two[i][g]*two[i][g];
				 	b+=1.0*two[j][g]*two[j][g];
				}
				dis = 1.0*ab/(1.0*sqrt(a)*sqrt(b));	
			}
			res.dis=dis;
			res.pos=j;
			distance.push_back(res); //验证集的一行 与 train的每一行的距离 
		}
		if(dis_way=="cos") sort(distance.begin(),distance.end(),comparationbig); 
		else sort(distance.begin(),distance.end(),comparationsmall); 
		for(int m=0;m<k;m++){  
			k_label.push_back(train_label[distance[m].pos]); 
		//	cout <<"ror=" <<distance[m].pos << "  dis=" << distance[m].dis <<"  label=" << train_label[distance[m].pos] <<endl;
		}
		vector<string> diff_label;
		diff_label.push_back("anger");
		diff_label.push_back("disgust");
		diff_label.push_back("fear");
		diff_label.push_back("joy");
		diff_label.push_back("sad");
		diff_label.push_back("surprise");
		int diff_size=6;
		Counter num[diff_size];// 记录diff_label的每个标签在k_label中出现的次数 
		for(int m=0;m<diff_size;m++){
			num[m].pos=-1;
			num[m].count=0;
			for(int q=0;q<k;q++){
				if(diff_label[m]==k_label[q]){
					/*相同label时记录dis最小的位置 
					if(num[m].pos!=-1){
						if(distance[num[m].pos].dis>distance[q].dis) num[m].pos=q;
					}else{
						num[m].pos = q;
					}
					num[m].count++;	*/
					if(dis_way=="cos"){
						num[m].pos=m; 
						//权重
						num[m].count += 1.0*((distance[q].dis)*(distance[q].dis));	
					}else{
						num[m].pos=m; 
						//权重
						num[m].count += 1.0/((distance[q].dis)*(distance[q].dis));	
					}
				} 
			}
		} 
		
		sort(num,num+diff_size,comparation2); //按权重从大到小排列 
		/*①实现当出现次数相同时选择距离近的label
		int f=0,best_pos=0;
		while(num[f].count!=num[f+1].count && f<diff_size){
			if(num[f].count==num[f+1].count){
				if(distance[num[f+1].pos].dis <distance[num[f].pos].dis) best_pos=f+1;
				f++;
			}else break;
		} 
		result_label.push_back(k_label[num[best_pos].pos]); //选择出现次数最多的（即众数）label */
		result_label.push_back(diff_label[num[0].pos]); //选择权重最大的label 
	}			
} 

double correct()
{
	int count=0,size=validation.size();
	for(int i=0;i<size;i++){
		if(result_label[i]==validation_label[i]) count++;
	}
	double accuracy = (1.0*count)/size;
	return accuracy;
}
 
typedef struct{
	double accuracy;
	int k;
	string dis_way;	
}BR; 
 
int main()
{
	GetTrain("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\classification_dataset\\train_set.csv");
//	GetVal("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\classification_dataset\\validation_set.csv");
	GetTest("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\classification_dataset\\test_set.csv");
	get_one_bri();
	get_tf();	
	get_tfidf();

	double accuracy = 0;
	string dis_way[4]={"3-th","CityBlock","cos","Euclidean"};
	get_result(9,dis_way[1],tf);
	
	/*validat 多次循环测试，调整参数
	BR best_res;
	best_res.accuracy=0;
	best_res.k=0;
	int size_of_train=train.size();
	for(int j=0;j<4;j++){
		for(int k=1;k<20;k++){
			result_label.clear();
		//	get_result(k,dis_way[j],one_hot);
			get_result(k,dis_way[j],tf);
		//	get_result(k,dis_way[j],tf_idf);
			accuracy=correct();
			cout<< "accuracy==" << accuracy << endl; 
			if(accuracy>best_res.accuracy){
				best_res.accuracy = accuracy;
				best_res.k=k;
				best_res.dis_way=dis_way[j];
			}
		}
		cout << "准确率=" << best_res.accuracy << endl; 
		cout << "k=" << best_res.k << endl; 
		cout << "dis_way=" << best_res.dis_way << endl; 
		best_res.accuracy=0;
		best_res.k=0;
	} */
	
	/*输出test的结果*/
	ofstream outFile;
	outFile.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\classification_dataset\\15352154_Sample_KNN_classification.csv",ios::out);
	int row_of_res=result_label.size();
	outFile << "textid" << ',' << "label" << "\n";
	for(int r=0;r<row_of_res;r++){
		outFile << (r+1) << ','<< result_label[r] << "\n";	
	}
	outFile.close(); 
	
 	return 0;
 } 
  
