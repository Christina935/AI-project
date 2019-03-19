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

struct node {
	int pos;  //��pos������
	int n_children; //�ӽ����� �������м��� 
	string type;  //��¼���ִ������label 
	vector<string> children_type; //������ӽ�㣬���¼�������м��ֲ�ͬ��ȡֵ 
	vector<node*> children; //���ӽ�� 
};

node *root;  //�����
vector<string> dataset; //���ݼ� 
vector<string> label; //���ݼ��еĲ�ֵͬ 
vector<vector<string> > attribute;//�������  
vector<string> vali;
vector<vector<string> > vali_attr;//������� 

/*���ã���һ���ı��и��һ��������*/
vector<string> get_xi(string temp)
{ 
	vector<string> xi;
	string pattern=",";
	temp+=pattern;
	int size=temp.size(),pos;
	for(int i=0;i<size;i++){
		pos=temp.find(pattern,i);
		if(pos<size){
			string s=temp.substr(i,pos-i);
			xi.push_back(s);
			i=pos; // i=pos+pattern.size()-1; 
		}
	}
	//�ҳ����ݼ��еĲ�ͬlabel 
	int size_xi = xi.size();
	if(label.size()==0) label.push_back(xi[size_xi-1]);
	else if(label.size()<2 && label.size()>0){
		vector<string>::iterator it;
		it =find(label.begin(),label.end(),xi[size_xi-1]);
		if(it==label.end()) 
			label.push_back(xi[size_xi-1]);
	}
	return xi;
}   

/*���ã���ȡ�ļ�����trainÿ���ı��и��һ�������֣�����train_attribute�����y����train_label��*/ 
void get_train(const char *filename,int n)
{
	vector<string> data;
	vector<vector<string> > attr;
	ifstream ReadFile;
	string temp;
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()) return;
	else{
		while(getline(ReadFile,temp)){
			vector<string> xi;
			xi=get_xi(temp);
			//һ���ı������һ����label 
			data.push_back(xi[xi.size()-1]);
			xi.pop_back();
			attr.push_back(xi);
		}	
	}
	ReadFile.close();
	int data_size=data.size(),j=0;
	for(int i=0;i<data_size;i++){
		if(n==0){
			dataset.push_back(data[i]);
			attribute.push_back(attr[i]);
		}else{
			if(j<(n-1)){
				dataset.push_back(data[i]);
				attribute.push_back(attr[i]);
				j++;
			}
			else if(j==(n-1)){
				vali.push_back(data[i]);
				vali_attr.push_back(attr[i]);
				j=0;
			}
		}
		
	}
}



/*�ж��Ƿ�����߽�����: ���赱ǰ�������ݼ�ΪD��������ΪA
�� D�е���������ͬһ���C���򽫵�ǰ�����ΪC���Ҷ���
�� AΪ�ռ�����D������������A������������ȡֵ��ͬ����ʱ�޷����֡�
  ����ǰ�����ΪҶ��㣬���ΪD�г���������
�� DΪ�ռ����򽫵�ǰ�����ΪҶ��㣬���Ϊ������г��������� */ 

bool meet_with_bound(vector<string> data, vector<vector<string> > attr)
{
	/* three conditions */
	/* ��*/ 
	int data_size=data.size(),num_label1=0;
	for(int i=0;i<data_size;i++){
		if(data[i]==label[0]) num_label1++;
	} 
	if((num_label1==data_size) || (num_label1==0)){ //֤��ֻ��һ����� 
		return true;
	}
	
	/* ��*/ 
	if(attr.empty()) return true; 
	
	int attr_size=attr[0].size(), row=1,column=0;
	for(int i=1;i<data_size;i++){
		//��attr�е�ÿһ�к͵�һ�е�attr���бȽ� 
		for(int j=0;j<attr_size;j++){
			if(attr[i][j]==attr[0][j]) column++; 
		}
		if(column==attr_size) row++;
	}
	//�����������������������ȡֵ����ͬ����ǰ���ΪҶ�ӽ�� 
	if(row==data_size) return true;
	
	/* ��*/
	if(data.empty()) return true; 
	
	/*����������������*/ 
	return false;
	
}

/***************************** ID3 **********************************************/ 
//���㾭���� 
double get_HD(vector<string> data)
{
	int num_label1=0 , num_label2=0;
	int data_size =data.size();
	for(int i=0;i<data_size;i++){
		if(data[i] ==label[0]) num_label1++;
		else if(data[i]==label[1]) num_label2++;
	}
	double label1_en=0,label2_en=0;
	if(num_label1==0 || num_label1==data_size)  label1_en=0;
	else label1_en=(-1)*(1.0*num_label1/data_size)*log(1.0*num_label1/data_size);
	if(num_label2==0 || num_label2==data_size) label2_en=0;
	else label2_en=(-1)*(1.0*num_label2/data_size)*log(1.0*num_label2/data_size);
	double HD=label1_en+label2_en;
	return HD;
} 

//����ÿ�������µ������� 
vector<double> getCondition(vector<string> data, vector<vector<string> > attr)
{
	vector<double> condition_entropy; //��¼ÿ�������µ������� 
	vector<string> diff_attr; //��¼ÿ�������Ĳ�ֵͬ 
	vector<vector<double> > attr_pos; //��¼ÿ�������Ĳ�ֵͬ��λ�� 
	int attr_row = attr.size(),attr_column=attr[0].size();
	for(int j=0;j<attr_column;j++){ //����ÿһ�� 
		for(int i=0;i<attr_row;i++){    //����ÿһ�е�ÿһ�� 
			//�����ҳ��������м��ֲ�ͬ��ֵ�����Ҽ�¼��ֵͬ���ֵ�λ�� 
			if(diff_attr.size()==0){  
				diff_attr.push_back(attr[i][j]);
				if(attr_pos.size()==0){
					vector<double> v;
					v.push_back(i);
					attr_pos.push_back(v);
				}	
			}else{
				vector<string>::iterator it;
				it=find(diff_attr.begin(),diff_attr.end(),attr[i][j]);
				if(it==diff_attr.end()){
					diff_attr.push_back(attr[i][j]);
					int size_d=diff_attr.size();
					if(attr_pos.size()<size_d){
						vector<double> v;
						v.push_back(i);
						attr_pos.push_back(v);
					}else attr_pos[size_d-1].push_back(i);	
				}else{
					int position=distance(diff_attr.begin(),it);
					if(attr_pos.size()<=position){
						vector<double> v;
						v.push_back(i);
						attr_pos.push_back(v); 
					}else attr_pos[position].push_back(i);
				}	
			}
		}
		
		//�����ڸ������µ������� 
		double  HDA=0;
		int num_diffattr=diff_attr.size(),num_attr=attr.size();
		for(int k=0;k<num_diffattr;k++){ //�����Ĳ�ֵͬ 
			int num_pos=attr_pos[k].size();
			int num_label1=0,num_label2=0;
			for(int g=0;g<num_pos;g++){ //����������Ϊ��ֵ��ǰ���³��ֲ�ͬlabel������ 
				 if(data[attr_pos[k][g]]==label[0])
				 	num_label1++;
				 else if(data[attr_pos[k][g]]==label[1])
				 	num_label2++;
			} 
		
			//��������Ϊ��ֵʱ�������� 
			double  part = 1.0*num_pos/num_attr;
			double label1_en=0,label2_en=0;
			if(num_label1==0 || num_label1==num_pos)  label1_en=0;
			else label1_en=(-1)*(1.0*num_label1/num_pos)*log(1.0*num_label1/num_pos);
			if(num_label2==0 || num_label2==num_pos) label2_en=0;
			else label2_en=(-1)*(1.0*num_label2/num_pos)*log(1.0*num_label2/num_pos);
			double cond =1.0*part*(label1_en+label2_en);
			//������Ϊ��ֵʱ�������ؼ��뵽�������������� 
			HDA+=cond;		
		}
		//���������������ؼ�¼���� 
		condition_entropy.push_back(HDA);
		diff_attr.clear();
		attr_pos.clear();	 
	}
	return condition_entropy;
}

//������Ϣ���� 
vector<double> get_Gain(vector<string> data, vector<vector<string> > attr)
{
	int res;
	double max=0;
	double HD=get_HD(data);
//	cout << "  ������=" << HD << endl; 
	vector<double> condition_entropy=getCondition(data,attr);
	vector<double> gain;
	int con_size=condition_entropy.size();
	for(int i=0;i<con_size;i++){
//		cout << "  ������= " << condition_entropy[i] << endl; 
		double diff=HD-condition_entropy[i];
		cout << "  ��Ϣ����= " << diff << endl; 
		gain.push_back(diff);
	}
	return gain;	
}

/***************************** ID3 **********************************************/ 

/******************************************** C4.5 *****************************************************/
//����ÿ�������µ������غ�ÿ���������� 
vector<double> get_cond_split(vector<string> data,vector<vector<string> >attr,vector<double> &splitInfo)
{
	vector<double> condition_entropy; //��¼ÿ�������µ������� 
	vector<string> diff_attr; //��¼ÿ�������Ĳ�ֵͬ 
	vector<vector<double> > attr_pos; //��¼ÿ�������Ĳ�ֵͬ��λ�� 
	int attr_row = attr.size(),attr_column=attr[0].size();
	for(int j=0;j<attr_column;j++){ //����ÿһ�� 
		for(int i=0;i<attr_row;i++){    //����ÿһ�е�ÿһ�� 
			//�����ҳ��������м��ֲ�ͬ��ֵ�����Ҽ�¼��ֵͬ���ֵ�λ�� 
			if(diff_attr.size()==0){  
				diff_attr.push_back(attr[i][j]);
				if(attr_pos.size()==0){
					vector<double> v;
					v.push_back(i);
					attr_pos.push_back(v);
				}	
			}else{
				vector<string>::iterator it;
				it=find(diff_attr.begin(),diff_attr.end(),attr[i][j]);
				if(it==diff_attr.end()){
					diff_attr.push_back(attr[i][j]);
					int size_d=diff_attr.size();
					if(attr_pos.size()<size_d){
						vector<double> v;
						v.push_back(i);
						attr_pos.push_back(v);
					}else attr_pos[size_d-1].push_back(i);	
				}else{
					int position=distance(diff_attr.begin(),it);
					if(attr_pos.size()<=position){
						vector<double> v;
						v.push_back(i);
						attr_pos.push_back(v); 
					}else attr_pos[position].push_back(i);
				}	
			}
		}
		
		//�����ڸ������µ������� 
		double  HDA=0;
		int num_diffattr=diff_attr.size(),num_attr=attr.size();
		for(int k=0;k<num_diffattr;k++){ //�����Ĳ�ֵͬ 
			int num_pos=attr_pos[k].size();
			int num_label1=0,num_label2=0;
			for(int g=0;g<num_pos;g++){ //����������Ϊ��ֵ��ǰ���³��ֲ�ͬlabel������ 
				 if(data[attr_pos[k][g]]==label[0])
				 	num_label1++;
				 else if(data[attr_pos[k][g]]==label[1])
				 	num_label2++;
			} 
		
			//��������Ϊ��ֵʱ�������� 
			double  part = 1.0*num_pos/num_attr;
			double label1_en=0,label2_en=0;
			if(num_label1==0 || num_label1==num_pos)  label1_en=0;
			else label1_en=(-1)*(1.0*num_label1/num_pos)*log(1.0*num_label1/num_pos);
			if(num_label2==0 || num_label2==num_pos) label2_en=0;
			else label2_en=(-1)*(1.0*num_label2/num_pos)*log(1.0*num_label2/num_pos);
			double cond =1.0*part*(label1_en+label2_en);
			//������Ϊ��ֵʱ�������ؼ��뵽�������������� 
			HDA+=cond;		
		}
		//���������������ؼ�¼���� 
		condition_entropy.push_back(HDA);
		
		//�������������
		double split=0,data_size=data.size();
		for(int k=0;k<num_diffattr;k++){
			int num=attr_pos[k].size();
			split+=(-1.0)*(1.0*num/data_size)*log(1.0*num/data_size);
		}
		splitInfo.push_back(split);
		 
		diff_attr.clear();
		attr_pos.clear();	 
	}
	return condition_entropy;
}

//������Ϣ������ 
vector<double> get_GainRatio(vector<string> data,vector<vector<string> >attr)
{
	vector<double> gain;
	double HD=get_HD(data);
	vector<double> splitInfo;
	vector<double> condition_entropy=get_cond_split(data,attr,splitInfo);
	//������Ϣ���� 
	int con_size=condition_entropy.size();
	for(int i=0;i<con_size;i++){
		double diff=HD-condition_entropy[i];
		gain.push_back(diff);
	}
	
	//������Ϣ������ 
	vector<double> gainRatio;
	for(int i=0;i<con_size;i++){
//		cout << "  ��Ϣ����= " << gain[i] << endl; 
//		cout << "  ��������= " << splitInfo[i] << endl; 
		double ratio=1.0*gain[i]/splitInfo[i];
//		cout << "  ��Ϣ������= " << ratio << endl; 
		gainRatio.push_back(ratio);
	}
	
	return gainRatio;
}

/******************************************** C4.5 *****************************************************/

/******************************************** CART *****************************************************/
vector<double> get_gini(vector<string> data,vector<vector<string> > attr)
{
	vector<double> gini; //��¼ÿ�������µ�giniϵ�� 
	vector<string> diff_attr; //��¼ÿ�������Ĳ�ֵͬ 
	vector<vector<double> > attr_pos; //��¼ÿ�������Ĳ�ֵͬ��λ�� 
	int attr_row = attr.size(),attr_column=attr[0].size();
	for(int j=0;j<attr_column;j++){ //����ÿһ�� 
		for(int i=0;i<attr_row;i++){    //����ÿһ�е�ÿһ�� 
			//�����ҳ��������м��ֲ�ͬ��ֵ�����Ҽ�¼��ֵͬ���ֵ�λ�� 
			if(diff_attr.size()==0){  
				diff_attr.push_back(attr[i][j]);
				if(attr_pos.size()==0){
					vector<double> v;
					v.push_back(i);
					attr_pos.push_back(v);
				}	
			}else{
				vector<string>::iterator it;
				it=find(diff_attr.begin(),diff_attr.end(),attr[i][j]);
				if(it==diff_attr.end()){
					diff_attr.push_back(attr[i][j]);
					int size_d=diff_attr.size();
					if(attr_pos.size()<size_d){
						vector<double> v;
						v.push_back(i);
						attr_pos.push_back(v);
					}else attr_pos[size_d-1].push_back(i);	
				}else{
					int position=distance(diff_attr.begin(),it);
					if(attr_pos.size()<=position){
						vector<double> v;
						v.push_back(i);
						attr_pos.push_back(v); 
					}else attr_pos[position].push_back(i);
				}	
			}
		}
		
		//�����ڸ������µ�giniϵ�� 
		double  GN=0;
		int num_diffattr=diff_attr.size(),num_attr=attr.size();
		for(int k=0;k<num_diffattr;k++){ //�����Ĳ�ֵͬ 
			int num_pos=attr_pos[k].size();
			
			int num_label1=0,num_label2=0;
			for(int g=0;g<num_pos;g++){ //����������Ϊ��ֵ��ǰ���³��ֲ�ͬlabel������ 
				 if(data[attr_pos[k][g]]==label[0])
				 	num_label1++;
				 else if(data[attr_pos[k][g]]==label[1])
				 	num_label2++;
			} 
			
			//��������Ϊ��ֵʱ��giniϵ�� 
			double  part = 1.0*num_pos/num_attr;
			double  gini_part=1.0*part*(1-(1.0*num_label1/num_pos)*(1.0*num_label1/num_pos)-(1.0*num_label2/num_pos)*(1.0*num_label2/num_pos));
			GN+=gini_part;		
		}
		//����������giniϵ����¼���� 
//		cout << "  ������GINIϵ��= "<< GN << endl; 
		gini.push_back(GN);
		diff_attr.clear();
		attr_pos.clear();	 
	}
	return gini;
}
/******************************************** GART *****************************************************/
/*���ã�
���ݲ�ͬ���㷨ѡ�����ߵ�*/ 
int choose_attr(vector<string> data, vector<vector<string> > attr,string choose_way)
{
	int attr_choose; 
	if(choose_way=="ID3")
	{
		vector<double> gain=get_Gain(data, attr);
		vector<double>::iterator biggest = max_element(gain.begin(), gain.end());
		attr_choose = distance(gain.begin(),biggest);
		
	}
	else if(choose_way=="C4.5")
	{
		vector<double> gainRatio = get_GainRatio(data,attr);
		vector<double>::iterator biggest = max_element(gainRatio.begin(), gainRatio.end());
		attr_choose = distance(gainRatio.begin(),biggest);
	}
	else if(choose_way=="CART")
	{
		vector<double> gini=get_gini(data,attr);
		vector<double>::iterator smallest = min_element(gini.begin(), gini.end());
		attr_choose = distance(gini.begin(),smallest);
	}
	else
	{
		/* what do you want to do? */
	}
	return attr_choose;
}

vector<vector<string> > divide_data(vector<string> data, int choose_attr,vector<vector<vector<string> > > &subattr,vector<vector<string> >attr,vector<string>& diff_attr)
{
	/* divide dataset `D` according to the attribute `a` */
//	vector<string> diff_attr; //��¼ÿ�������Ĳ�ֵͬ 
	vector<vector<double> > attr_pos; //��¼ÿ�������Ĳ�ֵͬ��λ�� 
	int attr_row = attr.size();
	for(int i=0;i<attr_row;i++){    //������������ÿһ�� 
		//�����ҳ��������м��ֲ�ͬ��ֵ�����Ҽ�¼��ֵͬ���ֵ�λ�� 
		if(diff_attr.size()==0){
			diff_attr.push_back(attr[i][choose_attr]);
			if(attr_pos.size()==0){
				vector<double> v;
				v.push_back(i);
				attr_pos.push_back(v);
			}	
		}else{
			vector<string>::iterator it;
			it=find(diff_attr.begin(),diff_attr.end(),attr[i][choose_attr]);
			if(it==diff_attr.end()){
				diff_attr.push_back(attr[i][choose_attr]);
				int size_d=diff_attr.size();
				if(attr_pos.size()<size_d){
					vector<double> v;
					v.push_back(i);
					attr_pos.push_back(v);
				}else attr_pos[size_d-1].push_back(i);	
			}else{
				int position=distance(diff_attr.begin(),it);
				if(attr_pos.size()<=position){
					vector<double> v;
					v.push_back(i);
					attr_pos.push_back(v); 
				}else attr_pos[position].push_back(i);
			}	
		}
	}
	vector<vector<string> > subsets;
	vector<string> part; 
	vector<vector<string> > part_attr;
	int row_pos=attr_pos.size(),column_attr=attr[0].size();
    //�����������Ĳ�ͬȡֵ��Ȼ������ȡֵ��Ӧ����ȡ�������γ������ݼ����������� 
	for(int i=0;i<row_pos;i++){
		for(int j=0;j<attr_pos[i].size();j++){
			part.push_back(data[attr_pos[i][j]]);
			vector<string> p;//�����attr��ÿһ�� 
			for(int k=0;k<column_attr;k++){
				if(k!=choose_attr) p.push_back(attr[attr_pos[i][j]][k]); 
			}
			part_attr.push_back(p);
			p.clear();
		}
		subsets.push_back(part);
		subattr.push_back(part_attr);
		part_attr.clear();
		part.clear();
	}
	return subsets;
}


void recursive(node *p,string choose_way,vector<string> data , vector<vector<string> > attr,vector<bool> & visit )
{
	if(meet_with_bound(data,attr))
	{
		/*�������߽���������ѡ�����Ϊdataset�г���������*/
		int data_size=data.size(),num_label1=0,num_label2=0; 
		for(int i=0;i<data_size;i++){
			if(data[i]==label[0]) num_label1++;
			else if(data[i]==label[1]) num_label2++;
		}
		if(num_label1>num_label2){
			p->type=label[0];
		}else{
			p->type=label[1];
		}
		p->n_children=0;
		p->pos=-1;
		return;
	}

	//��¼��ǰ����typeΪ���ִ�������type 
	int data_size=data.size(),num_label1=0,num_label2=0; 
	for(int i=0;i<data_size;i++){
		if(data[i]==label[0]) num_label1++;
		else if(data[i]==label[1]) num_label2++;
	}
	if(num_label1>num_label2){
		p->type=label[0];
	}else{
		p->type=label[1];
	}
	/* choose the best attribute */
	int attr_chosen = choose_attr(data,attr,choose_way);
	//��һ��visit����¼�����Ƿ��Ѿ�ѡ��
	//��Ϊѡ������ʱ�����и���������ѡ����ôѡ���������λ�ú�ԭʼ��λ���ǲ�ͬ�ģ���ô����Ҫ����visit���ҵ�����ԭʼλ�á�
	int vi_num=visit.size(),num=-1;
	for(int h=0;h<vi_num;h++){
		if(visit[h]==false && num<attr_chosen) num++;
		if(num==attr_chosen){
			visit[h]=true;
			p->pos=h;	
			break;
		} 
	}
	cout << "  ѡ���" << p->pos+1 << "������"<< endl<<endl; 
	/* 
	 * divide the current dataset into
	 * several subsets
	 * according to `attr_chosen`
	 */
	vector<vector<vector<string> > > subattr; 
	vector<string> diff_attr;
	vector<vector<string> > subsets = divide_data(data,attr_chosen,subattr,attr,diff_attr);	
	int num_sub=subsets.size();	
	p->n_children=num_sub;
	p->children_type.assign(diff_attr.begin(),diff_attr.end());
	//�ݹ齨�� 
	for(int i=0;i<num_sub;i++){
		node *subnode = new node();
		vector<bool> vis;
		vis.assign(visit.begin(),visit.end());
		recursive(subnode,choose_way,subsets[i] , subattr[i],vis);
		p->children.push_back(subnode);
	} 
	return;
}

void print(node *p,int depth)
{
	for(int i=0;i<depth;i++) cout << "\t";
	if(!p->children_type.empty()){
		cout << "��"<< (p->pos+1) << "������" << endl;
		for(int i=0;i<p->children_type.size();i++){
			for(int d=0;d<depth+1;d++) cout << "\t";
			cout << p->children_type[i] << endl;
			print(p->children[i],depth+2);
			
		}
	}else{
		cout << p->type << endl;
	}
}


//test
vector<vector<string> > test;
void get_test(const char *filename)
{
	ifstream ReadFile;
	string temp;
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()) return;
	else{
		while(getline(ReadFile,temp)){
			vector<string> xi;
			xi=get_xi(temp);
			test.push_back(xi);
		}	
	}
	ReadFile.close();
}


vector<string> get_res(node *p1,vector<vector<string> > tv)
{
	vector<string> res;
	int row_tv=tv.size() , column_tv=tv[0].size();
	node *p=p1;
	for(int i=0;i<row_tv;i++){
		while(p->n_children!=0){
			bool have_path=false; 
			for(int j=0;j<p->n_children;j++){
				if(tv[i][p->pos]==p->children_type[j]){
					p=p->children[j];
					have_path=true;
				} 
			}
			if(have_path==false){
				res.push_back(p->type);
				break;
			} 
		}
		if(p->n_children==0){
			res.push_back(p->type);
		} 
		p=p1;
	}
	return res;
}

double get_accurancy(vector<string> res)
{
	double acc,num=0;
	int row_vali=vali.size();
	for(int i=0;i<row_vali;i++){
		if(res[i]==vali[i]) num++;
	}
	acc=1.0*num/row_vali;
	return acc;
}


int main()
{
	root = new node();
	get_train("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab4\\������ ����5-6����������\\YStrain3.csv",0);
//	get_train("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab4\\lab4_Decision_Tree\\train.csv",3);
	get_test("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab4\\������ ����5-6����������\\YStest3.csv");





	string choose_way[3]={"ID3","C4.5","CART"};
	int attr_num=attribute[0].size();
	vector<bool> visit;
	for(int i=0;i<attr_num;i++){
		visit.push_back(false);
	}
	recursive(root,choose_way[0],dataset , attribute,visit);
	print(root,0);
	
//	
//	vector<string> res=get_res(root,vali_attr);
// 	double acc=	get_accurancy(res);
// 	cout << "׼ȷ��=" << acc << endl; 
 	
 	vector<string> test_res=get_res(root,test);
 	ofstream outFile;
 	outFile.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\lab4\\lab4_Decision_Tree\\data.txt",ios::out);
	int row_of_res=test_res.size();
	for(int r=0;r<row_of_res;r++){
		cout << test_res[r] << endl;
		outFile << test_res[r] << "\n";	
	}
	outFile.close();
 	
 	

	return 0;
}
