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

//���ַ�����ֵת����int���͵���ֵ 
int string_to_num(string str)
{
	int size = str.size(),num=str[0]-'0';
	for(int i=1;i<size;i++){
		num+=(str[i]-'0')*10;
	}	
	return num;
}

//���ļ��е����ݶ�ȡ������������Ԫ˳����� 
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
			if(circle==1)	smatric.num_of_row = string_to_num(temp);//��һ�������� 
			else if(circle==2)	smatric.num_of_column = string_to_num(temp);//�ڶ��������� 
			else if(circle==3)	smatric.no_zero = string_to_num(temp);//�������Ƿ�����ֵ�ĸ��� 
			else{
				triple tri;
				int size = temp.size();
				string pattern=" ";//��������֮���ÿһ���ı��Կո�Ϊ��ǽ����и� 
				int posr,posc;
				posr = temp.find(pattern,0);
				string r = temp.substr(0,posr);
				tri.row = string_to_num(r); //�и�����ĵ�һ����ֵ�Ƿ������ݵ��� 
				temp.erase(0,posr+1);
				posc = temp.find(pattern,0);
				string c = temp.substr(0,posc);
				tri.column = string_to_num(c);//�и�����ĵڶ�����ֵ�Ƿ������ݵ��� 
				temp.erase(0,posc+1);
				tri.num = string_to_num(temp);//�и�����ĵ�������ֵ�Ƿ������ݵ�ֵ 
				smatric.data.push_back(tri);
			}
			circle++;
		}
	}	
	ReadFile.close();
} 

//����Ԫ˳����з������ݰ������еĴ�С���� 
bool comparison(triple A,triple B)
{
	if(A.row!=B.row) return A.row<B.row;
	else return A.column<B.column;
}

//������Ԫ˳������ 
matric AplusB(matric A, matric B)
{
	matric smatric;
	smatric.num_of_row=A.num_of_row; //������ӵľ����������ͬ��������õĽ��������Ҳ��ͬ 
	smatric.num_of_column=A.num_of_column;
	int sizeA=A.no_zero, sizeB=B.no_zero;
	int visitB[sizeB] , visitA[sizeA];
	memset(visitB,0,sizeof(visitB));
	memset(visitA,0,sizeof(visitA));
	for(int i=0;i<sizeA;i++){
		for(int j=0;j<sizeB;j++){
			triple tri;
			//����ҵ����С�����ͬ�������������ݣ�����û����ӹ��������ǽ������ 
			if(A.data[i].row==B.data[j].row && A.data[i].column==B.data[j].column&&visitB[j]==0){
				tri.row=A.data[i].row;
				tri.column=A.data[i].column;
				tri.num=A.data[i].num + B.data[j].num;
				smatric.data.push_back(tri);	
				visitB[j]=1;
				visitA[i]=1;//��� �Ѿ���ӹ� 
			}
		}
		if(visitA[i]==0){
			smatric.data.push_back(A.data[i]);
			//A�е��������������B��û�ҵ���ͬ���еķ������ݣ���ֱ�ӽ���������� 
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
	A=GetMatric("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\ʵ��1\\A.txt");
	B=GetMatric("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\ʵ��1\\B.txt");
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
	in.open("F:\\ѧϰ����\\������\\�˹�����\\ʵ��\\ʵ��1\\C.txt",ios::trunc);
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

