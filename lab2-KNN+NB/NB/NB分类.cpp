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

vector<string> train_label; //存放训练集的情感标签 
vector<string> Diff; //存放不同的单词
vector<vector<string> > train;   //存放训练集train的单词 
vector<vector<string> > validation; //存放验证集validation 或 测试集test 的单词 
vector<string> validation_label; //验证集的label 
vector<string> result;//存放通过模型得出的验证集的情感
vector<vector<double> >NumOfWords; //记录训练集和验证集每个单词的出现次数
vector<double> p_label;//记录每个情感标签在train中出现的概率，顺序是：anger、disgust、fear、joy、 sad、surprise
vector<vector<int> > pos;//记录6个情感标签在train中出现的位置 
vector<vector<double> > p_words;//记录在一种情感前提下单词出现的概率 
string emotion[6]={"anger","disgust","fear","joy","sad","surprise"};


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

/*作用：读取test文件，将每行文本的单词放入test中，情绪放到test_label中*/ 
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

/*作用：获得NumOfWords矩阵
NumOfWords:记录单词出现的次数 */
void get_numofwords()
{
	int column=Diff.size(),row1=train.size(),row2=validation.size(),row=row1+row2,k;//one_hot矩阵的行数和列数 
	vector<double> v; //记录NumOfWords矩阵的一行 
	for(int i=0;i<row;i++){
		for(int j=0;j<column;j++){
			if(i<row1){ //0~（row1-1）行:train训练集的部分 
				vector<string>::iterator it;
				//在每行文本的单词中找Diff中的单词 
				it=find(train[i].begin(),train[i].end(),Diff[j]);
				if(it==train[i].end()){ //找不到记录为0 
					v.push_back(0);   
				}else{     
					bool first=true; //记录是否是第一次找到 
					//在该行文本中能够找到该单词，则把该单词删除，然后继续寻找该单词，直到找不到为止 
					while(it!=train[i].end()){
						it=train[i].erase(it);
						if(first==true){
							v.push_back(1); //第一次找到，记录为1 
						}else{
							v[j]++;//不是第一次找到，把原来记录的数字++ 
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
				}else{  
					v.push_back(1); //找到记录为1，validation只需知道该单词出现过，不需要记录出现了几次 
					/*bool first=true; //记录是否是第一次找到 
					while(it!=validation[k].end()){
						it=validation[k].erase(it);
						if(first==true){
							v.push_back(1); //第一次找到，记录为1 
						}else{
							v[j]++;//不是第一次找到，把原来记录的数字++ 
						}
						it=find(validation[k].begin(),validation[k].end(),Diff[j]);
						first=false;
					}*/
				}	
			}	
		}
		//将在每行文本中寻找Diff的结果放入NumOfWords矩阵中 
		NumOfWords.push_back(v);
		v.clear();
	}
}  

/*作用：计算在train中每个label出现的概率，以及出现的位置*/
void get_labelpossibility()
{	
 	//每种情感出现的次数
	double accur_angry=0,accur_disgust=0,accur_fear=0,accur_joy=0,accur_sad=0,accur_surprise=0;  
	//记录每种情感label在train中出现的行数 
	vector<int> v0,v1,v2,v3,v4,v5;
	int size_of_trainlabel=train_label.size();
	for(int i=0;i<size_of_trainlabel;i++){
		if(train_label[i]==emotion[0]){
			accur_angry+=1;
			v0.push_back(i);
		}else if(train_label[i]==emotion[1]){
			accur_disgust+=1;
			v1.push_back(i);
		}else if(train_label[i]==emotion[2]){
			accur_fear+=1;
			v2.push_back(i);
		}else if(train_label[i]==emotion[3]){
			accur_joy+=1;
			v3.push_back(i);
		}else if(train_label[i]==emotion[4]){
			accur_sad+=1;
			v4.push_back(i);
		}else if(train_label[i]==emotion[5]){
			accur_surprise+=1;
			v5.push_back(i);
		}
	}
	/*初始版
	accur_angry=1.0*accur_angry/size_of_trainlabel;
	p_label.push_back(accur_angry);
	accur_disgust=1.0*accur_disgust/size_of_trainlabel;
	p_label.push_back(accur_disgust);
	accur_fear=1.0*accur_fear/size_of_trainlabel;
	p_label.push_back(accur_fear);
	accur_joy=1.0*accur_joy/size_of_trainlabel;
	p_label.push_back(accur_joy);
	accur_sad=1.0*accur_sad/size_of_trainlabel;
	p_label.push_back(accur_sad);
	accur_surprise=1.0*accur_surprise/size_of_trainlabel;
	p_label.push_back(accur_surprise);*/
	
	/*优化*/ 
	accur_angry=1.0*(accur_angry+1)/(size_of_trainlabel+6);
	p_label.push_back(accur_angry);
	accur_disgust=1.0*(accur_disgust+1)/(size_of_trainlabel+6);
	p_label.push_back(accur_disgust);
	accur_fear=1.0*(accur_fear+1)/(size_of_trainlabel+6);
	p_label.push_back(accur_fear);
	accur_joy=1.0*(accur_joy+1)/(size_of_trainlabel+6);
	p_label.push_back(accur_joy);
	accur_sad=1.0*(accur_sad+1)/(size_of_trainlabel+6);
	p_label.push_back(accur_sad);
	accur_surprise=1.0*(accur_surprise+1)/(size_of_trainlabel+6);
	p_label.push_back(accur_surprise);
	pos.push_back(v0);
	pos.push_back(v1);
	pos.push_back(v2);
	pos.push_back(v3);
	pos.push_back(v4);
	pos.push_back(v5);
}

void Bernoulli()
{
	int num_label=6,label_row=0,size_diff=Diff.size();
	double nex=0;
	vector<double> v;
	for(int i=0;i<num_label;i++){
		label_row=pos[i].size();//出现第i个label的行数
		if(label_row!=0){
			for(int m=0;m<size_diff;m++){
			 	nex=0; //记录某个单词在某个情感前提下出现的文本数 
				for(int j=0;j<label_row;j++){
					if(NumOfWords[pos[i][j]][m]!=0){
						nex+=1; 
					}else continue;
				} 
				//	nex=1.0*nex/label_row; //0.10
			    //  nex=1.0*(nex+1)/(label_row+2);//降低很多,0.10~0.05 
			    //	nex=1.0*(nex+1)/(label_row+train.size()); //升高 0.44 
		        //	nex=1.0*(nex+1)/(label_row+1600); //0.4469
		        //	nex=1.0*(nex+0.00005)/(label_row+0.0001);//0.382637
		   	    //	nex=1.0*(nex+0.00001)/(label_row+0.00002);//0.385852
		   	    	double  a=0.00001 ;
			    	nex=1.0*(nex+1*a)/(label_row+2*a); //0.385852
		          //  nex=1.0*(nex+0.0003)/(label_row+0.0006);	 //0.376206	 
				v.push_back(nex);
			}
			p_words.push_back(v);
			v.clear();
		}else{
			p_words.push_back(v);
		} 
		
	}
} 

/*作用：运用多项式模型，计算出 在每种情感的前提下单词出现的概率*/
void Multinomial(){
	int num_label=6,label_row=0,size_diff=Diff.size();
	double nwex=0,all_words=0,norepet_words=0;
	vector<double> v;
	for(int i=0;i<num_label;i++){ //遍历每种情感 
		label_row=pos[i].size();//出现第i个label的行数
		if(label_row!=0){
			all_words=0; //出现该label的文本的所有单词数 
			norepet_words=0; //出现该label的文本的所有不重复单词数 
			for(int k=0;k<label_row;k++){
				for(int g=0;g<size_diff;g++){
					if(NumOfWords[pos[i][k]][g]!=0){
						all_words+=NumOfWords[pos[i][k]][g];
						norepet_words+=1;
					} 	
				}
			}
			for(int m=0;m<size_diff;m++){
			 	nwex=0; //记录Diff中每个单词在该情感label前提下出现的次数 
				for(int j=0;j<label_row;j++){	
					nwex+=NumOfWords[pos[i][j]][m]; 
				} 
			//	 nwex=1.0*nwex/all_words; //原始版 0.106109
			//	nwex=1.0*(nwex+1)/(all_words+size_diff); //0.456592
			// 	nwex=1.0*(nwex+1)/(all_words+norepet_words); //0.048 
				double a=0.00001;
				nwex=1.0*(nwex+1*a)/(all_words+norepet_words*a);
				//a:0.1~0.00001 a越小，准确率越高，当a=0.000001时结果依然和a=0.00001一样，继续变小也是 
				v.push_back(nwex);
			}
			p_words.push_back(v);
			v.clear();
		}else{
			p_words.push_back(v);
		}
	}
}

/*作用：计算每种情感在validation/test每行文本的前提下出现的概率*/ 
void get_result()
{
	int size_label=6,row_of_vali=validation.size(),size_diff=Diff.size();
	int size_train=train.size();
	double possibility=1;
	vector<double> v;
	for(int m=0;m<row_of_vali;m++){ //验证集的每一行 
		for(int i=0;i<size_label;i++){ //六种情感 
			possibility=p_label[i];
			if(pos[i].size()==0) possibility=0;//train中不存在这种情感 
			else{
				for(int j=0;j<size_diff;j++){
					if(NumOfWords[m+size_train][j]!=0){
						possibility=1.0*possibility*p_words[i][j];
					}else{
						continue;
					}
					
				} 
			}
			v.push_back(possibility);
		}
		int pos = (int) ( max_element(v.begin(),v.end())-v.begin());
		result.push_back(emotion[pos]);
		v.clear(); 
	}	
}

double correct()
{
	int count=0,size=validation.size();
	for(int i=0;i<size;i++){
		if(result[i]==validation_label[i]) count++;
	}
	double accuracy = (1.0*count)/size;
	return accuracy;
}

int main()
{
	GetTrain("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\classification_dataset\\train_set.csv");
	GetVal("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\classification_dataset\\validation_set.csv");
//	GetTest("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\classification_dataset\\test_set.csv");
	get_numofwords();
	get_labelpossibility();
	p_words.clear();
//	Bernoulli();
	Multinomial();
	get_result();
	
	/*测试validation的准确率 
	double accuracy=correct();
	cout << "正确率=" << accuracy << endl;*/
	
	/*输出test的结果*/
	ofstream outFile;
	outFile.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab2\\DATA\\classification_dataset\\15352154_Sample_NB_classification.csv",ios::out);
	int row_of_res=result.size();
	outFile << "textid" << ',' << "label" << "\n";
	for(int r=0;r<row_of_res;r++){
		outFile << (r+1) << ',' << result[r] << "\n";	
	}
	outFile.close(); 
	
	return 0;
}

