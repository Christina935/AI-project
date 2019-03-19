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

//将字符串数值转换成int类型的数值 
int string_to_num(string str)
{
	int size = str.size(),num=str[0]-'0';
	for(int i=1;i<size;i++){
		num+=(str[i]-'0')*10;
	}	
	return num;
}

//将文件中的内容读取出来，放入三元顺序表中 
matric GetMatric(const char *filename)
{
	matric smatric;
	ifstream ReadFile;
	string temp;
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail()){
		return smatric;
	}else{
		int circle=1;
		while(getline(ReadFile,temp)){
			if(circle==1)	smatric.num_of_row = string_to_num(temp);//第一行是行数 
			else if(circle==2)	smatric.num_of_column = string_to_num(temp);//第二行是列数 
			else if(circle==3)	smatric.no_zero = string_to_num(temp);//第三行是非零数值的个数 
			else{
				triple tri;
				int size = temp.size();
				string pattern=" ";//将第三行之后的每一行文本以空格为标记进行切割 
				int posr,posc;
				posr = temp.find(pattern,0);
				string r = temp.substr(0,posr);
				tri.row = string_to_num(r); //切割出来的第一个数值是非零数据的行 
				temp.erase(0,posr+1);
				posc = temp.find(pattern,0);
				string c = temp.substr(0,posc);
				tri.column = string_to_num(c);//切割出来的第二个数值是非零数据的列 
				temp.erase(0,posc+1);
				tri.num = string_to_num(temp);//切割出来的第三个数值是非零数据的值 
				smatric.data.push_back(tri);
			}
			circle++;
		}
	}	
	ReadFile.close();
} 

//将三元顺序表中非零数据按照行列的大小排序 
bool comparison(triple A,triple B)
{
	if(A.row!=B.row) return A.row<B.row;
	else return A.column<B.column;
}

//两个三元顺序表相加 
matric AplusB(matric A, matric B)
{
	matric smatric;
	smatric.num_of_row=A.num_of_row; //两个相加的矩阵的行列相同，相加所得的结果的行列也相同 
	smatric.num_of_column=A.num_of_column;
	int sizeA=A.no_zero, sizeB=B.no_zero;
	int visitB[sizeB] , visitA[sizeA];
	memset(visitB,0,sizeof(visitB));
	memset(visitA,0,sizeof(visitA));
	for(int i=0;i<sizeA;i++){
		for(int j=0;j<sizeB;j++){
			triple tri;
			//如果找到了行、列相同的两个非零数据，并且没有相加过，则将他们进行相加 
			if(A.data[i].row==B.data[j].row && A.data[i].column==B.data[j].column&&visitB[j]==0){
				tri.row=A.data[i].row;
				tri.column=A.data[i].column;
				tri.num=A.data[i].num + B.data[j].num;
				smatric.data.push_back(tri);	
				visitB[j]=1;
				visitA[i]=1;//标记 已经相加过 
			}
		}
		if(visitA[i]==0){
			smatric.data.push_back(A.data[i]);
			//A中的这个非零数据在B中没找到相同行列的非零数据，则直接将它放入表中 
		}
	}
	for(int k=0;k<sizeB;k++){
		if(visitB[k]==0){
			smatric.data.push_back(B.data[k]);
		}
	}
	smatric.no_zero = smatric.data.size();
	sort(smatric.data.begin(),smatric.data.end(),comparison);
	return smatric;
}

int main()
{
	matric A , B , C;
	A=GetMatric("F:\\学习资料\\大三上\\人工智能\\实验\\实验1\\A.txt");
	B=GetMatric("F:\\学习资料\\大三上\\人工智能\\实验\\实验1\\B.txt");
	C=AplusB(A,B);
	
//	cout << "row=" << C.num_of_row << endl;
//	cout <<	"column=" << C.num_of_column <<endl;
//	cout <<	"no_zero=" << C.no_zero <<endl;
//	for(int i=0;i<C.no_zero;i++){
//		cout << "row==" << C.data[i].row << " ";
//		cout <<"column==" << C.data[i].column << " ";
//		cout << "num==" << C.data[i].num << endl;
//	}
	
	ofstream in;
	in.open("F:\\学习资料\\大三上\\人工智能\\实验\\实验1\\C.txt",ios::trunc);
	in << C.num_of_row << "\n";
	in << C.num_of_column << "\n";
	in << C.no_zero << "\n";
	for(int i=0;i<C.no_zero;i++){
		in << C.data[i].row << " ";
		in << C.data[i].column << " ";
		in << C.data[i].num << "\n";
	}
	in.close();
	return 0;
}

