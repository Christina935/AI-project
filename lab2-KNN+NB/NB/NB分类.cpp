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

vector<string> train_label; //���ѵ��������б�ǩ 
vector<string> Diff; //��Ų�ͬ�ĵ���
vector<vector<string> > train;   //���ѵ����train�ĵ��� 
vector<vector<string> > validation; //�����֤��validation �� ���Լ�test �ĵ��� 
vector<string> validation_label; //��֤����label 
vector<string> result;//���ͨ��ģ�͵ó�����֤�������
vector<vector<double> >NumOfWords; //��¼ѵ��������֤��ÿ�����ʵĳ��ִ���
vector<double> p_label;//��¼ÿ����б�ǩ��train�г��ֵĸ��ʣ�˳���ǣ�anger��disgust��fear��joy�� sad��surprise
vector<vector<int> > pos;//��¼6����б�ǩ��train�г��ֵ�λ�� 
vector<vector<double> > p_words;//��¼��һ�����ǰ���µ��ʳ��ֵĸ��� 
string emotion[6]={"anger","disgust","fear","joy","sad","surprise"};


/*���ã���һ���ַ���str�и��һ�������ʣ���str�У������Կո�ָ�*/
vector<string> get_word(string temp)
{
	//��һ���ı��и�ɵ��� 
	vector<string> v;
	string pattern=" ";//�����Կո�ֿ� 
	temp+=pattern;
	int size=temp.size(),pos;
	for(int i=0;i<size;i++){
		pos=temp.find(pattern,i);
		if(pos<size){
			string s=temp.substr(i,pos-i);//�и�����ĵ��� 
			v.push_back(s);
			vector<string>::iterator it;
			//��Diff(���ʼ�����Ѱ��������� 
			it=find(Diff.begin(),Diff.end(),s); 
			if(it==Diff.end()){
				//���Diff��û�иõ��ʣ��򽫸õ��ʷ���Diff�� 
				Diff.push_back(s); 
			} 
			i=pos+pattern.size()-1;
		}
	}
	return v;
}

/*���ã�
��ȡtrainning�ļ�����ÿ���ı��ĵ��ʷ���train�У���������train_label��*/ 
void GetTrain(const char *filename)
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
				//�и��label 
				string s1=",";
				int pos,size=temp.size();
				pos=temp.find(s1,0);//�ҵ�����λ�ã�����֮����ı���Ϊlabel 
				string emotion=temp.substr(pos+1,size);
				train_label.push_back(emotion);
				temp.erase(pos,size);//�и��label��ʣ��Ϊwords 
				vector<string> v = get_word(temp);
				train.push_back(v);
				v.clear();
			}
		}
	}
	ReadFile.close();
} 

/*��ȡvalidation�ļ�����ÿ���ı��ĵ��ʷ���validation�У���������label��*/
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
				//�и��label 
				string s1=",";
				int pos,size=temp.size();
				pos=temp.find(s1,0);//�ҵ�����λ�ã�����֮����ı���Ϊlabel 
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

/*���ã���ȡtest�ļ�����ÿ���ı��ĵ��ʷ���test�У������ŵ�test_label��*/ 
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
				pos=temp.find(s1,0);//�ҵ�����λ�� 
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

/*���ã����NumOfWords����
NumOfWords:��¼���ʳ��ֵĴ��� */
void get_numofwords()
{
	int column=Diff.size(),row1=train.size(),row2=validation.size(),row=row1+row2,k;//one_hot��������������� 
	vector<double> v; //��¼NumOfWords�����һ�� 
	for(int i=0;i<row;i++){
		for(int j=0;j<column;j++){
			if(i<row1){ //0~��row1-1����:trainѵ�����Ĳ��� 
				vector<string>::iterator it;
				//��ÿ���ı��ĵ�������Diff�еĵ��� 
				it=find(train[i].begin(),train[i].end(),Diff[j]);
				if(it==train[i].end()){ //�Ҳ�����¼Ϊ0 
					v.push_back(0);   
				}else{     
					bool first=true; //��¼�Ƿ��ǵ�һ���ҵ� 
					//�ڸ����ı����ܹ��ҵ��õ��ʣ���Ѹõ���ɾ����Ȼ�����Ѱ�Ҹõ��ʣ�ֱ���Ҳ���Ϊֹ 
					while(it!=train[i].end()){
						it=train[i].erase(it);
						if(first==true){
							v.push_back(1); //��һ���ҵ�����¼Ϊ1 
						}else{
							v[j]++;//���ǵ�һ���ҵ�����ԭ����¼������++ 
						}
						it=find(train[i].begin(),train[i].end(),Diff[j]);
						first=false;
					} 
				}	
			}else {   //row1-row�У�test���Լ� 
				k=i-row1;
				vector<string>::iterator it;
				//��ÿ���ı��ĵ�������Diff�еĵ��� 
				it=find(validation[k].begin(),validation[k].end(),Diff[j]);
				if(it==validation[k].end()){ //�Ҳ�����¼Ϊ0 
					v.push_back(0);
				}else{  
					v.push_back(1); //�ҵ���¼Ϊ1��validationֻ��֪���õ��ʳ��ֹ�������Ҫ��¼�����˼��� 
					/*bool first=true; //��¼�Ƿ��ǵ�һ���ҵ� 
					while(it!=validation[k].end()){
						it=validation[k].erase(it);
						if(first==true){
							v.push_back(1); //��һ���ҵ�����¼Ϊ1 
						}else{
							v[j]++;//���ǵ�һ���ҵ�����ԭ����¼������++ 
						}
						it=find(validation[k].begin(),validation[k].end(),Diff[j]);
						first=false;
					}*/
				}	
			}	
		}
		//����ÿ���ı���Ѱ��Diff�Ľ������NumOfWords������ 
		NumOfWords.push_back(v);
		v.clear();
	}
}  

/*���ã�������train��ÿ��label���ֵĸ��ʣ��Լ����ֵ�λ��*/
void get_labelpossibility()
{	
 	//ÿ����г��ֵĴ���
	double accur_angry=0,accur_disgust=0,accur_fear=0,accur_joy=0,accur_sad=0,accur_surprise=0;  
	//��¼ÿ�����label��train�г��ֵ����� 
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
	/*��ʼ��
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
	
	/*�Ż�*/ 
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
		label_row=pos[i].size();//���ֵ�i��label������
		if(label_row!=0){
			for(int m=0;m<size_diff;m++){
			 	nex=0; //��¼ĳ��������ĳ�����ǰ���³��ֵ��ı��� 
				for(int j=0;j<label_row;j++){
					if(NumOfWords[pos[i][j]][m]!=0){
						nex+=1; 
					}else continue;
				} 
				//	nex=1.0*nex/label_row; //0.10
			    //  nex=1.0*(nex+1)/(label_row+2);//���ͺܶ�,0.10~0.05 
			    //	nex=1.0*(nex+1)/(label_row+train.size()); //���� 0.44 
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

/*���ã����ö���ʽģ�ͣ������ ��ÿ����е�ǰ���µ��ʳ��ֵĸ���*/
void Multinomial(){
	int num_label=6,label_row=0,size_diff=Diff.size();
	double nwex=0,all_words=0,norepet_words=0;
	vector<double> v;
	for(int i=0;i<num_label;i++){ //����ÿ����� 
		label_row=pos[i].size();//���ֵ�i��label������
		if(label_row!=0){
			all_words=0; //���ָ�label���ı������е����� 
			norepet_words=0; //���ָ�label���ı������в��ظ������� 
			for(int k=0;k<label_row;k++){
				for(int g=0;g<size_diff;g++){
					if(NumOfWords[pos[i][k]][g]!=0){
						all_words+=NumOfWords[pos[i][k]][g];
						norepet_words+=1;
					} 	
				}
			}
			for(int m=0;m<size_diff;m++){
			 	nwex=0; //��¼Diff��ÿ�������ڸ����labelǰ���³��ֵĴ��� 
				for(int j=0;j<label_row;j++){	
					nwex+=NumOfWords[pos[i][j]][m]; 
				} 
			//	 nwex=1.0*nwex/all_words; //ԭʼ�� 0.106109
			//	nwex=1.0*(nwex+1)/(all_words+size_diff); //0.456592
			// 	nwex=1.0*(nwex+1)/(all_words+norepet_words); //0.048 
				double a=0.00001;
				nwex=1.0*(nwex+1*a)/(all_words+norepet_words*a);
				//a:0.1~0.00001 aԽС��׼ȷ��Խ�ߣ���a=0.000001ʱ�����Ȼ��a=0.00001һ����������СҲ�� 
				v.push_back(nwex);
			}
			p_words.push_back(v);
			v.clear();
		}else{
			p_words.push_back(v);
		}
	}
}

/*���ã�����ÿ�������validation/testÿ���ı���ǰ���³��ֵĸ���*/ 
void get_result()
{
	int size_label=6,row_of_vali=validation.size(),size_diff=Diff.size();
	int size_train=train.size();
	double possibility=1;
	vector<double> v;
	for(int m=0;m<row_of_vali;m++){ //��֤����ÿһ�� 
		for(int i=0;i<size_label;i++){ //������� 
			possibility=p_label[i];
			if(pos[i].size()==0) possibility=0;//train�в������������ 
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
	GetTrain("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\classification_dataset\\train_set.csv");
	GetVal("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\classification_dataset\\validation_set.csv");
//	GetTest("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\classification_dataset\\test_set.csv");
	get_numofwords();
	get_labelpossibility();
	p_words.clear();
//	Bernoulli();
	Multinomial();
	get_result();
	
	/*����validation��׼ȷ�� 
	double accuracy=correct();
	cout << "��ȷ��=" << accuracy << endl;*/
	
	/*���test�Ľ��*/
	ofstream outFile;
	outFile.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\classification_dataset\\15352154_Sample_NB_classification.csv",ios::out);
	int row_of_res=result.size();
	outFile << "textid" << ',' << "label" << "\n";
	for(int r=0;r<row_of_res;r++){
		outFile << (r+1) << ',' << result[r] << "\n";	
	}
	outFile.close(); 
	
	return 0;
}

