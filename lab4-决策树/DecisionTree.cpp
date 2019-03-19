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
	int pos;  //第pos个特征
	int n_children; //子结点个数 即特征有几类 
	string type;  //记录出现次数最多label 
	vector<string> children_type; //如果有子结点，则记录该特征有几种不同的取值 
	vector<node*> children; //孩子结点 
};

node *root;  //根结点
vector<string> dataset; //数据集 
vector<string> label; //数据集中的不同值 
vector<vector<string> > attribute;//存放属性  
vector<string> vali;
vector<vector<string> > vali_attr;//存放属性 

/*作用：将一行文本切割成一个个数字*/
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
	//找出数据集中的不同label 
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

/*作用：读取文件，将train每行文本切割成一个个数字，放入train_attribute，结果y放入train_label中*/ 
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
			//一行文本的最后一个是label 
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



/*判断是否满足边界条件: 假设当前结点的数据集为D，特征集为A
① D中的样本属于同一类别C，则将当前结点标记为C类的叶结点
② A为空集，或D中所有样本在A中所有特征上取值相同，此时无法划分。
  将当前结点标记为叶结点，类别为D中出现最多的类
③ D为空集，则将当前结点标记为叶结点，类别为父结点中出现最多的类 */ 

bool meet_with_bound(vector<string> data, vector<vector<string> > attr)
{
	/* three conditions */
	/* ①*/ 
	int data_size=data.size(),num_label1=0;
	for(int i=0;i<data_size;i++){
		if(data[i]==label[0]) num_label1++;
	} 
	if((num_label1==data_size) || (num_label1==0)){ //证明只有一个类别 
		return true;
	}
	
	/* ②*/ 
	if(attr.empty()) return true; 
	
	int attr_size=attr[0].size(), row=1,column=0;
	for(int i=1;i<data_size;i++){
		//将attr中的每一行和第一行的attr进行比较 
		for(int j=0;j<attr_size;j++){
			if(attr[i][j]==attr[0][j]) column++; 
		}
		if(column==attr_size) row++;
	}
	//如果所有样本的所有特征的取值都相同，则当前结点为叶子结点 
	if(row==data_size) return true;
	
	/* ③*/
	if(data.empty()) return true; 
	
	/*三个条件都不满足*/ 
	return false;
	
}

/***************************** ID3 **********************************************/ 
//计算经验熵 
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

//计算每个特征下的条件熵 
vector<double> getCondition(vector<string> data, vector<vector<string> > attr)
{
	vector<double> condition_entropy; //记录每个特征下的条件熵 
	vector<string> diff_attr; //记录每个特征的不同值 
	vector<vector<double> > attr_pos; //记录每个特征的不同值的位置 
	int attr_row = attr.size(),attr_column=attr[0].size();
	for(int j=0;j<attr_column;j++){ //遍历每一列 
		for(int i=0;i<attr_row;i++){    //遍历每一列的每一行 
			//遍历找出该特征有几种不同的值，并且记录不同值出现的位置 
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
		
		//计算在该特征下的条件熵 
		double  HDA=0;
		int num_diffattr=diff_attr.size(),num_attr=attr.size();
		for(int k=0;k<num_diffattr;k++){ //特征的不同值 
			int num_pos=attr_pos[k].size();
			int num_label1=0,num_label2=0;
			for(int g=0;g<num_pos;g++){ //计算在特征为该值的前提下出现不同label的数量 
				 if(data[attr_pos[k][g]]==label[0])
				 	num_label1++;
				 else if(data[attr_pos[k][g]]==label[1])
				 	num_label2++;
			} 
		
			//计算特征为该值时的条件熵 
			double  part = 1.0*num_pos/num_attr;
			double label1_en=0,label2_en=0;
			if(num_label1==0 || num_label1==num_pos)  label1_en=0;
			else label1_en=(-1)*(1.0*num_label1/num_pos)*log(1.0*num_label1/num_pos);
			if(num_label2==0 || num_label2==num_pos) label2_en=0;
			else label2_en=(-1)*(1.0*num_label2/num_pos)*log(1.0*num_label2/num_pos);
			double cond =1.0*part*(label1_en+label2_en);
			//将特征为该值时的条件熵加入到该特征的条件熵 
			HDA+=cond;		
		}
		//将该特征的条件熵记录起来 
		condition_entropy.push_back(HDA);
		diff_attr.clear();
		attr_pos.clear();	 
	}
	return condition_entropy;
}

//计算信息增益 
vector<double> get_Gain(vector<string> data, vector<vector<string> > attr)
{
	int res;
	double max=0;
	double HD=get_HD(data);
//	cout << "  经验熵=" << HD << endl; 
	vector<double> condition_entropy=getCondition(data,attr);
	vector<double> gain;
	int con_size=condition_entropy.size();
	for(int i=0;i<con_size;i++){
//		cout << "  条件熵= " << condition_entropy[i] << endl; 
		double diff=HD-condition_entropy[i];
		cout << "  信息增益= " << diff << endl; 
		gain.push_back(diff);
	}
	return gain;	
}

/***************************** ID3 **********************************************/ 

/******************************************** C4.5 *****************************************************/
//计算每个特征下的条件熵和每个特征的熵 
vector<double> get_cond_split(vector<string> data,vector<vector<string> >attr,vector<double> &splitInfo)
{
	vector<double> condition_entropy; //记录每个特征下的条件熵 
	vector<string> diff_attr; //记录每个特征的不同值 
	vector<vector<double> > attr_pos; //记录每个特征的不同值的位置 
	int attr_row = attr.size(),attr_column=attr[0].size();
	for(int j=0;j<attr_column;j++){ //遍历每一列 
		for(int i=0;i<attr_row;i++){    //遍历每一列的每一行 
			//遍历找出该特征有几种不同的值，并且记录不同值出现的位置 
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
		
		//计算在该特征下的条件熵 
		double  HDA=0;
		int num_diffattr=diff_attr.size(),num_attr=attr.size();
		for(int k=0;k<num_diffattr;k++){ //特征的不同值 
			int num_pos=attr_pos[k].size();
			int num_label1=0,num_label2=0;
			for(int g=0;g<num_pos;g++){ //计算在特征为该值的前提下出现不同label的数量 
				 if(data[attr_pos[k][g]]==label[0])
				 	num_label1++;
				 else if(data[attr_pos[k][g]]==label[1])
				 	num_label2++;
			} 
		
			//计算特征为该值时的条件熵 
			double  part = 1.0*num_pos/num_attr;
			double label1_en=0,label2_en=0;
			if(num_label1==0 || num_label1==num_pos)  label1_en=0;
			else label1_en=(-1)*(1.0*num_label1/num_pos)*log(1.0*num_label1/num_pos);
			if(num_label2==0 || num_label2==num_pos) label2_en=0;
			else label2_en=(-1)*(1.0*num_label2/num_pos)*log(1.0*num_label2/num_pos);
			double cond =1.0*part*(label1_en+label2_en);
			//将特征为该值时的条件熵加入到该特征的条件熵 
			HDA+=cond;		
		}
		//将该特征的条件熵记录起来 
		condition_entropy.push_back(HDA);
		
		//计算该特征的熵
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

//计算信息增益率 
vector<double> get_GainRatio(vector<string> data,vector<vector<string> >attr)
{
	vector<double> gain;
	double HD=get_HD(data);
	vector<double> splitInfo;
	vector<double> condition_entropy=get_cond_split(data,attr,splitInfo);
	//计算信息增益 
	int con_size=condition_entropy.size();
	for(int i=0;i<con_size;i++){
		double diff=HD-condition_entropy[i];
		gain.push_back(diff);
	}
	
	//计算信息增益率 
	vector<double> gainRatio;
	for(int i=0;i<con_size;i++){
//		cout << "  信息增益= " << gain[i] << endl; 
//		cout << "  特征的熵= " << splitInfo[i] << endl; 
		double ratio=1.0*gain[i]/splitInfo[i];
//		cout << "  信息增益率= " << ratio << endl; 
		gainRatio.push_back(ratio);
	}
	
	return gainRatio;
}

/******************************************** C4.5 *****************************************************/

/******************************************** CART *****************************************************/
vector<double> get_gini(vector<string> data,vector<vector<string> > attr)
{
	vector<double> gini; //记录每个特征下的gini系数 
	vector<string> diff_attr; //记录每个特征的不同值 
	vector<vector<double> > attr_pos; //记录每个特征的不同值的位置 
	int attr_row = attr.size(),attr_column=attr[0].size();
	for(int j=0;j<attr_column;j++){ //遍历每一列 
		for(int i=0;i<attr_row;i++){    //遍历每一列的每一行 
			//遍历找出该特征有几种不同的值，并且记录不同值出现的位置 
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
		
		//计算在该特征下的gini系数 
		double  GN=0;
		int num_diffattr=diff_attr.size(),num_attr=attr.size();
		for(int k=0;k<num_diffattr;k++){ //特征的不同值 
			int num_pos=attr_pos[k].size();
			
			int num_label1=0,num_label2=0;
			for(int g=0;g<num_pos;g++){ //计算在特征为该值的前提下出现不同label的数量 
				 if(data[attr_pos[k][g]]==label[0])
				 	num_label1++;
				 else if(data[attr_pos[k][g]]==label[1])
				 	num_label2++;
			} 
			
			//计算特征为该值时的gini系数 
			double  part = 1.0*num_pos/num_attr;
			double  gini_part=1.0*part*(1-(1.0*num_label1/num_pos)*(1.0*num_label1/num_pos)-(1.0*num_label2/num_pos)*(1.0*num_label2/num_pos));
			GN+=gini_part;		
		}
		//将该特征的gini系数记录起来 
//		cout << "  特征的GINI系数= "<< GN << endl; 
		gini.push_back(GN);
		diff_attr.clear();
		attr_pos.clear();	 
	}
	return gini;
}
/******************************************** GART *****************************************************/
/*作用：
根据不同的算法选出决策点*/ 
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
//	vector<string> diff_attr; //记录每个特征的不同值 
	vector<vector<double> > attr_pos; //记录每个特征的不同值的位置 
	int attr_row = attr.size();
	for(int i=0;i<attr_row;i++){    //遍历该特征的每一行 
		//遍历找出该特征有几种不同的值，并且记录不同值出现的位置 
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
    //遍历该特征的不同取值，然后把这个取值对应的行取出来，形成子数据集和子特征集 
	for(int i=0;i<row_pos;i++){
		for(int j=0;j<attr_pos[i].size();j++){
			part.push_back(data[attr_pos[i][j]]);
			vector<string> p;//存放子attr的每一行 
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
		/*如果满足边界条件，则选择类别为dataset中出现最多的类*/
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

	//记录当前结点的type为出现次数最多的type 
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
	//用一个visit来记录特征是否已经选择。
	//因为选择特征时是在切割后的特征中选择，那么选择的特征的位置和原始的位置是不同的，那么就需要根据visit来找到它的原始位置。
	int vi_num=visit.size(),num=-1;
	for(int h=0;h<vi_num;h++){
		if(visit[h]==false && num<attr_chosen) num++;
		if(num==attr_chosen){
			visit[h]=true;
			p->pos=h;	
			break;
		} 
	}
	cout << "  选择第" << p->pos+1 << "个特征"<< endl<<endl; 
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
	//递归建树 
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
		cout << "第"<< (p->pos+1) << "个特征" << endl;
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
	get_train("F:\\学习资料\\大三上\\人工智能\\实验\\lab4\\决策树 周五5-6验收用数据\\YStrain3.csv",0);
//	get_train("F:\\学习资料\\大三上\\人工智能\\实验\\lab4\\lab4_Decision_Tree\\train.csv",3);
	get_test("F:\\学习资料\\大三上\\人工智能\\实验\\lab4\\决策树 周五5-6验收用数据\\YStest3.csv");





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
// 	cout << "准确率=" << acc << endl; 
 	
 	vector<string> test_res=get_res(root,test);
 	ofstream outFile;
 	outFile.open("F:\\学习资料\\大三上\\人工智能\\实验\\lab4\\lab4_Decision_Tree\\data.txt",ios::out);
	int row_of_res=test_res.size();
	for(int r=0;r<row_of_res;r++){
		cout << test_res[r] << endl;
		outFile << test_res[r] << "\n";	
	}
	outFile.close();
 	
 	

	return 0;
}
