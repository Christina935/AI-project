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
 
vector<string> result; //����и�������ı� 
vector<string> Diff;   //��Ų�ͬ�ĵ��� 
vector<vector<int> > bridge; //��¼ÿ���ı���ÿ�����ʳ��ֵĴ��� 
vector<vector<int> > one_hot; 
vector<vector<double> > tf;
vector<vector<double> > tf_idf;
//��ÿ���ı��и�����ĵ��ʷ���һ��vector�У��ڽ�vector����word�� 
vector<vector<string> > word; 


//��ȡ�ļ�����ÿ���ı�����result�� 
void GetString(const char *filename)
{
	ifstream ReadFile;
	string temp;  //����ȡ���ı��ŵ�temp��ȥ 
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()){
		return ;
	}else{
		//һ��һ�еض�ȡ�ļ�����ÿ���ı��ŵ�temp�� 
		while(getline(ReadFile,temp)){ 
			int size = temp.size();
			string pattern="	";
			//��tap��Ϊ��������������Ҫ�����ݡ��ڶ���tap֮������ݡ� 
			int pos;
			pos = temp.find(pattern,0);
			pos = temp.find(pattern,pos+1); //�ҵ��ڶ���tap 
			string s = temp.substr(pos+1,size);  //�������������и���� 
			result.push_back(s); //���и�������ı�����result�� 
		}
	}	
	ReadFile.close();
} 
 
//��ò�ͬ�ĵ��� 
void getDiff()
{
	vector<string> v;
	for(int i=0;i<result.size();i++){
		string str = result[i];
		int pos;
		string pattern = " ";//�Կո��Ϊ��ǽ��ı����и�ɵ��� 
		str+=pattern;
		int size = str.size();
		for(int j=0; j<size; j++){
			pos = str.find(pattern,j); //��λ��j��ʼ��str���ҵ��ո��λ�� 
			if(pos<size){
				string s = str.substr(j,pos-j); // ��ȡ�ӵ�jλ��ʼ�ĳ���Ϊpos-j���ַ���(���ʣ� 
				v.push_back(s);
				vector<string>::iterator it;
				it = find(Diff.begin(),Diff.end(),s); 
				//�ڴ�Ų�ͬ���ʵ�vector Diff�����и�����ĵ��� 
				if(it==Diff.end()){ //�����Diff��û��������ʣ����������ʼӽ�ȥ 
					Diff.push_back(s);
				}
				j=pos+pattern.size()-1;
			}
		}
		word.push_back(v);
		v.clear();
	}
} 
 
//���ÿ���ı���ÿ�����ʵĸ��� 
void getBridge()
{
	int size = result.size();
	vector<int> v;
	int size_of_diff = Diff.size();
	for(int i=0;i<size;i++){
		for(int j=0; j<size_of_diff;j++){
			vector<string>::iterator it;
			it = find(word[i].begin(),word[i].end(),Diff[j]);//��ÿ���ı��ĵ�����Ѱ��Diff�еĵ��� 
			if(it==word[i].end()){
				v.push_back(0); //�Ҳ�����¼Ϊ0 
			}else{
				bool num=false;
				while(it!=word[i].end()){
					it=word[i].erase(it);
					if(num==false){
						v.push_back(1);//��һ���ҵ�����¼Ϊ1 
					} 
					else{
						v[j]++;	//���ǵ�һ���ҵ������ԭ����¼������++ 
					} 
					it = find(word[i].begin(),word[i].end(),Diff[j]);
					num=true;
				}	
			}
		}
		bridge.push_back(v);//����ÿ���ı���Ѱ��Diff�Ľ��v����bridge�� 
		v.clear();	
	}
} 

//���one_hot���� 
//ֱ�ӱ���bridge���󣬴���0���¼Ϊ1������0���¼Ϊ0 
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

//���tf���� 
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
			//Ҳ����ͨ��word��� 
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

//��ȡtf_idf���� 
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
			//ͨ�����㽫one_hot ÿ�е����������֪һ�����ʳ����ڶ���ƪ�ı��� 
		}
		t+=1;
		double res = log2(1.0*D/t);//D�����ı����� ��res��idf������ 
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
	int row;//�������ݵ��� 
	int column;//�������ݵ��� 
	int num;//�������ݵ�ֵ 
}triple; //��Ԫ˳��� 

typedef struct{
	int num_of_row;//ϡ���������� 
	int num_of_column;//ϡ���������� 
	int no_zero;//�������ݵĸ��� 
	vector<triple> data;//��¼�������ݵ��С��к���ֵ 
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
	GetString("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\ʵ��1\\semeval.txt");
	cout << "semeval�������ǣ�"<< result.size() << endl; 
	getDiff();
	int size = Diff.size();
	cout << "semeval������" << size << endl;
	getBridge();
	get_onehot();
	get_tf();
	get_tfidf();
	
	ofstream in;
	in.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\ʵ��1\\onehot.txt",ios::trunc);
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
	in_tf.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\ʵ��1\\tf.txt",ios::trunc);
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
	in_tfidf.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\ʵ��1\\tfidf.txt",ios::trunc);
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
	in_sma.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\ʵ��1\\smatric.txt",ios::trunc);
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
