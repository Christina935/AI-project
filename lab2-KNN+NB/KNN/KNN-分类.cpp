// ʹ��KNN�����������
// ����֤����ͨ������Kֵ��ѡ��ͬ����ȷ�ʽ�õ�һ��׼ȷ�����ŵ�ģ�Ͳ��� 
 
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

vector<string> train_label; //���ѵ�����ı�ǩ��������
vector<string> Diff; //��Ų�ͬ�ĵ���
vector<vector<string> > train;   //���ѵ����train�ĵ��� 
vector<vector<double> > one_hot; //one-hot���� 
vector<vector<string> > validation; //�����֤��validation 
vector<string> validation_label; //��֤����label 
vector<string> result_label;//���ͨ��ģ�͵ó�����֤���Ľ����ǩ 
vector<vector<double> >bridge; //��¼ѵ��������֤��ÿ�����ʵĳ��ִ���
vector<vector<double> >tf; //ѵ��������֤����tf����
vector<vector<double> > tf_idf; //ѵ��������֤����tf����

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

/*���ã���ȡtest�ļ�����ÿ���ı��ĵ��ʷ���validation��*/ 
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
			if(i<row1){ //0~��row1-1����:trainѵ�����Ĳ��� 
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
			}else {   //row1-row�У�test���Լ� 
				k=i-row1;
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

/*���ã����tf���� 
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
	int size_of_column=Diff.size();
	for(int j=0;j<size_of_column;j++){
		int t=0;
		for(int i=0;i<size_of_row;i++){
			t+=one_hot[i][j];
			//ͨ�����㽫one_hotѵ�������� ÿ�е�����
			//�����֪һ�����ʳ����ڶ���ƪ�ı��� 
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
	for(int i=row1;i<row;i++){ //������֤��two����ÿһ�� 
		vector<dis_pos> distance;  //��¼��֤���е�һ�к�ѵ������ÿһ�е��ľ��� 
		vector<string> k_label; //��¼ѡȡ��ѵ�����е�K����ӽ����ı���label 
		for(int j=0;j<row1;j++){ //����ѵ����one-hotÿһ�У������֤����һ�к�ѵ����ÿһ�еľ��� 
			double dis=0;
			if(dis_way=="Euclidean"){
				for(int g=0;g<Diff.size();g++){
					dis+=(two[i][g]-two[j][g])*(two[i][g]-two[j][g]);
				}
				dis=sqrt(dis);// ��ŷ�Ͼ��� 	
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
			distance.push_back(res); //��֤����һ�� �� train��ÿһ�еľ��� 
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
		Counter num[diff_size];// ��¼diff_label��ÿ����ǩ��k_label�г��ֵĴ��� 
		for(int m=0;m<diff_size;m++){
			num[m].pos=-1;
			num[m].count=0;
			for(int q=0;q<k;q++){
				if(diff_label[m]==k_label[q]){
					/*��ͬlabelʱ��¼dis��С��λ�� 
					if(num[m].pos!=-1){
						if(distance[num[m].pos].dis>distance[q].dis) num[m].pos=q;
					}else{
						num[m].pos = q;
					}
					num[m].count++;	*/
					if(dis_way=="cos"){
						num[m].pos=m; 
						//Ȩ��
						num[m].count += 1.0*((distance[q].dis)*(distance[q].dis));	
					}else{
						num[m].pos=m; 
						//Ȩ��
						num[m].count += 1.0/((distance[q].dis)*(distance[q].dis));	
					}
				} 
			}
		} 
		
		sort(num,num+diff_size,comparation2); //��Ȩ�شӴ�С���� 
		/*��ʵ�ֵ����ִ�����ͬʱѡ��������label
		int f=0,best_pos=0;
		while(num[f].count!=num[f+1].count && f<diff_size){
			if(num[f].count==num[f+1].count){
				if(distance[num[f+1].pos].dis <distance[num[f].pos].dis) best_pos=f+1;
				f++;
			}else break;
		} 
		result_label.push_back(k_label[num[best_pos].pos]); //ѡ����ִ������ģ���������label */
		result_label.push_back(diff_label[num[0].pos]); //ѡ��Ȩ������label 
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
	GetTrain("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\classification_dataset\\train_set.csv");
//	GetVal("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\classification_dataset\\validation_set.csv");
	GetTest("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\classification_dataset\\test_set.csv");
	get_one_bri();
	get_tf();	
	get_tfidf();

	double accuracy = 0;
	string dis_way[4]={"3-th","CityBlock","cos","Euclidean"};
	get_result(9,dis_way[1],tf);
	
	/*validat ���ѭ�����ԣ���������
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
		cout << "׼ȷ��=" << best_res.accuracy << endl; 
		cout << "k=" << best_res.k << endl; 
		cout << "dis_way=" << best_res.dis_way << endl; 
		best_res.accuracy=0;
		best_res.k=0;
	} */
	
	/*���test�Ľ��*/
	ofstream outFile;
	outFile.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab2\\DATA\\classification_dataset\\15352154_Sample_KNN_classification.csv",ios::out);
	int row_of_res=result_label.size();
	outFile << "textid" << ',' << "label" << "\n";
	for(int r=0;r<row_of_res;r++){
		outFile << (r+1) << ','<< result_label[r] << "\n";	
	}
	outFile.close(); 
	
 	return 0;
 } 
  
