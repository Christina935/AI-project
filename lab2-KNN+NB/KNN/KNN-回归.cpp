/* ʹ��KNN����ع�����
 ����֤����ͨ������Kֵ��ѡ��ͬ����ȷ�ʽ�õ�һ��׼ȷ�����ŵ�ģ�Ͳ��� */
 
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
using namespace std;

vector<vector<double> >train_emotion;//���ѵ����������ϵ��,������ anger��disgust��fear��joy��sad��surprise
vector<string> Diff; //��Ų�ͬ�ĵ���
vector<vector<string> > train;   //���trainѵ���� 
vector<vector<double> > one_hot; //one-hot���󣺼�¼�����Ƿ���� 
vector<vector<double> > bridge;  //bridge���󣺼�¼���ʳ��ִ��� 
vector<vector<string> > validation; //�����֤��validation 
vector<vector<double> > vali_emotion;//�����֤��validation������ϵ��
vector<vector<double> > result_emotion;//���ͨ��ģ�͵ó�����֤��validation������ϵ�� 
vector<vector<double> > tf; 
vector<vector<double> > tf_idf;

/*���ã���һ���ַ���str�и��һ�������ʣ���str�У������Կո�ָ�*/  
vector<string> get_word(string temp)
{
	vector<string> v;
	string pattern=" ";//�����Կո�ֿ� 
	temp+=pattern;
	int size=temp.size(),pos;
	for(int i=0;i<size;i++){
		pos=temp.find(pattern,i); //�ӵ�i��λ�ÿ�ʼ��temp��Ѱ�ҿո��λ�� 
		if(pos<size){
			string s=temp.substr(i,pos-i);//�и�����ĵ��� 
			v.push_back(s);
			vector<string>::iterator it;
			it=find(Diff.begin(),Diff.end(),s); //��Diff(���ʼ�����Ѱ��������� 
			if(it==Diff.end()){
				Diff.push_back(s); //���Diff��û�иõ��ʣ��򽫸õ��ʷ���Diff�� 
			} 
			i=pos+pattern.size()-1;//��i�Ƶ��ҵ��Ŀո�֮���λ�� 
		}
	}
	return v;
}

/*���ã���string���͵�����ת��double�� */
double stringToDouble(const string& str)  
{  
    istringstream iss(str);  
    double num;  
    iss >> num;  
    return num;      
}   
 
/*���ã���ȡtrainning�ļ�����ÿ���ı��ĵ��ʷ���train�У�����ϵ������train_emotion��*/
void GetTrain(const char *filename)
{
	ifstream ReadFile;
	string temp; //����ȡ��ÿ���ı��ŵ�temp��
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()){
		return ;
	}else{
		bool first=true;
		while(getline(ReadFile,temp)){
		//csv�ļ���ȡ����ÿ�����ݵ���ʽ����str1,str2,str3",��ÿ��������ԣ����� 
			if(first==true) first=false;//����ȡ�ļ��еĵ�һ�� 
			else{
				string s1=",";
				int pos,size=temp.size();
				pos=temp.find(s1,0);//�ҵ�����λ�ã���һ������֮ǰ���ı����� 
				string words=temp.substr(0,pos);
				temp.erase(0,pos+1);//ɾ��words 
				vector<string> v = get_word(words); //��words�и��һ�������� 
				train.push_back(v);
				v.clear();
				//�ռ�����ϵ�� anger��disgust��fear��joy��sad��surprise
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

/*���ã�
��ȡvalidation��֤���ļ�����ÿ���ı��ĵ��ʷ���validation�У�����ϵ���ŵ�vali_emotion��*/ 
void GetVal(const char *filename)
{
	ifstream ReadFile;
	string temp; //����ȡ��ÿ���ı��ŵ�temp��
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()){
		return ;
	}else{
		bool first=true;
		while(getline(ReadFile,temp)){  //ÿһ����ÿһ��������ԣ��ֿ� 
			if(first==true) first=false;
			else{
				string s1=",";
				int pos,size=temp.size();
				pos=temp.find(s1,0);//�ҵ�����λ�ã���һ������֮ǰ���ı����� 
				string words=temp.substr(0,pos);
				temp.erase(0,pos+1);//ɾ��words 
				vector<string> v = get_word(words);
				validation.push_back(v);
				v.clear();
				//�ռ�����ϵ�� anger��disgust��fear��joy��sad��surprise
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
 
/*���ã�
��ȡtest���Լ��ļ�����ÿ���ı��ĵ��ʷ���test�У�����ϵ���ŵ�vali_emotion��*/ 
void GetTest(const char *filename)
{
	ifstream ReadFile;
	string temp; //����ȡ��ÿ���ı��ŵ�temp��
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()){
		return ;
	}else{
		bool first=true;
		while(getline(ReadFile,temp)){  //ÿһ����ÿһ��������ԣ��ֿ� 
			if(first==true) first=false;
			else{
				string s1=",";
				int pos,size=temp.size();
				pos=temp.find(s1,0);//�ҵ�����λ�ã���һ������֮ǰ����� 
				temp.erase(0,pos+1);   //ɾ����ź͵�һ������ 
				string words=temp.substr(0,pos);
				vector<string> v = get_word(words);
				validation.push_back(v);
				v.clear();
			}
		}
	}
	ReadFile.close();
}  
 
/*���ã����one-hot��bridge����
one-hot:��¼�����Ƿ����
bridge����¼���ʳ��ֵĴ��� */  
void get_one_bri()
{
	int column=Diff.size(),row1=train.size(),row2=validation.size(),row=row1+row2,k;//one_hot��������������� 
	vector<double> v; //��¼one-hot�����һ�� 
	vector<double> v1; //��¼bridge�����һ�� 
	for(int i=0;i<row;i++){
		for(int j=0;j<column;j++){
			if(i<row1){                //0~��row1-1����:trainѵ�����Ĳ��� 
				vector<string>::iterator it;
				//��ÿ���ı��ĵ�������Diff�еĵ��� 
				it=find(train[i].begin(),train[i].end(),Diff[j]);
				if(it==train[i].end()){ //�Ҳ�����¼Ϊ0 
					v.push_back(0);
					v1.push_back(0);    
				}else{     //�ҵ���¼Ϊ1 
					v.push_back(1);
					bool first=true; //��¼�Ƿ��ǵ�һ���ҵ� 
					//�ڸ����ı����ܹ��ҵ��õ��ʣ���Ѹõ���ɾ����Ȼ�����Ѱ�Ҹõ��ʣ�ֱ���Ҳ���Ϊֹ 
					while(it!=train[i].end()){
						it=train[i].erase(it);
						if(first==true){
							v1.push_back(1); //��һ���ҵ�����¼Ϊ1 
						}else{
							v1[j]++;//���ǵ�һ���ҵ�����ԭ����¼������++ 
						}
						it=find(train[i].begin(),train[i].end(),Diff[j]);
						first=false;
					}
				}	
			}else {
				k=i-row1;  //row1-row�У�test���Լ� 
				vector<string>::iterator it;
				//��ÿ���ı��ĵ�������Diff�еĵ��� 
				it=find(validation[k].begin(),validation[k].end(),Diff[j]);
				if(it==validation[k].end()){ //�Ҳ�����¼Ϊ0 
					v.push_back(0);
					v1.push_back(0);
				}else{     //�ҵ���¼Ϊ1 
					v.push_back(1);
					bool first=true; //��¼�Ƿ��ǵ�һ���ҵ� 
					while(it!=validation[k].end()){
						it=validation[k].erase(it);
						if(first==true){
							v1.push_back(1); //��һ���ҵ�����¼Ϊ1 
						}else{
							v1[j]++;//���ǵ�һ���ҵ�����ԭ����¼������++ 
						}
						it=find(validation[k].begin(),validation[k].end(),Diff[j]);
						first=false;
					}
				}	
			}	
		}
		//����ÿ���ı���Ѱ��Diff�Ľ������one-hot������ 
		one_hot.push_back(v);
		v.clear();
		bridge.push_back(v1);
		v1.clear(); 
	}
} 

/*���ã����train_tf���� 
tf = (���ʳ��ִ���)/���ı�����������*/  
void get_tf()
{
	int size_of_row=bridge.size();
	int size_of_column=bridge[0].size();
	vector<double> v;
	double num;
	for(int i=0;i<size_of_row;i++){
		int Denominator=0;
		for(int k=0;k<size_of_column;k++){
			Denominator+=bridge[i][k];//����tf��ʽ�ķ�ĸ(�ı��ĵ���������
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

/*���ã���ȡtf_idf���� 
idf = (�ı�����)/(���ָõ��ʵ��ı���+1)
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
			//ͨ�����㽫one_hot ÿ�е����������֪һ�����ʳ����ڶ���ƪ�ı��� 
		}
		t+=1;
		double res = log2(1.0*D/t);//D�����ı����� ��res��idf������ 
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
	for(int i=row1;i<row;i++){ //������֤��one-hotÿһ�� 
		vector<dis_pos> distance;  //��¼��֤���е�һ�к�ѵ������ÿһ�е��ľ��� 
		for(int j=0;j<row1;j++){ //����ѵ����one-hotÿһ�У������֤����һ�к�ѵ����ÿһ�еľ��� 
			double dis=0; 
			if(dis_way=="Euclidean"){  //ŷ�Ͼ��� 
				for(int g=0;g<Diff.size();g++){
					dis+=(two[i][g]-two[j][g])*(two[i][g]-two[j][g]);
				}
				dis=sqrt(dis);	 	
			}else if(dis_way=="CityBlock"){ //�����پ��� 
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
			}else if(dis_way=="cos"){ //�������ƶ� 
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
			distance.push_back(res); //��֤����һ�� �� train��ÿһ�еľ��� 
		}
		
		/*���Ժ�����һ��*/
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
		/*�������ƶȣ�����ֵΪ����ֵԽ�󣬱�ʾ�����ı����ԽС��Ϊ���������
		 �����������ƶȰ��Ӵ�С���� ���������밴��С�������� */ 
		if(dis_way=="cos") sort(distance.begin(),distance.end(),comparationbig); 
		else sort(distance.begin(),distance.end(),comparationsmall); 
	//	for(int g=0;g<k;g++) cout <<"pos=" <<  distance[g].pos <<"   dis=" << distance[g].dis<< endl;
		int column_of_emotion=6;
		vector<double> em;
		/*�������ƶȣ�����ֵΪ����ֵԽ�󣬱�ʾ�����ı����ԽС��Ϊ���������
		 �����������ƶ�Խ�� �� Ȩ�ؾ�Ӧ��Խ��������* ��
		 �������� ����ԽС����Ȩ��Ӧ��Խ�������õ��� */ 
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
					//��ĸΪ����+1,��ֹ���ַ�ĸΪ0����� 
				}
				em.push_back(p);
			}
		}
		result_emotion.push_back(em);
		em.clear();
		distance.clear();
	}
		int row_of_res=result_emotion.size(),column_of_res=6;
		/*�����һ��*/
		for(int r=0;r<row_of_res;r++){
			double all=0; 
			for(int c=0;c<column_of_res;c++){
				all+=result_emotion[r][c];  //����ÿ�������������������ĸ��ʵ��ܺ� 
			}
			for(int h=0;h<column_of_res;h++){
				/*��ÿ�������ĸ��ʳ��Ըò������������ĸ����ܺ�
				 ����ͬһ���������ĸ������������ܺ�Ϊ1 */
				if(all!=0) result_emotion[r][h]/=all; 
			}
		}	
} 

int main()
{
	GetTrain("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\regression_dataset\\train_set.csv");
//	GetVal("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\regression_dataset\\validation_set.csv");
	GetTest("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\regression_dataset\\test_set.csv");
	get_one_bri();
	get_tf();
	get_tfidf();
	string dis_way[4]={"CityBlock","Euclidean","3-th","cos"};
	int k=11; 
	get_result(k,dis_way[3],tf_idf);
	
	ofstream outFile;
	outFile.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\regression_dataset\\15352154_Sample_KNN_regression.csv",ios::out);
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
