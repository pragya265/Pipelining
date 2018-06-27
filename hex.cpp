#include<iostream>
#include<sstream>
#include<bitset>
#include<string>
#include<fstream>
#include<algorithm>
#include<math.h>

using namespace std;
int score_board[31][2]={0}, i=0, score_board_time[31] = {}; float stall_id=0, alu=0, total_branches=0, branch_taken=0, load_access=0, store_access=0, cc=0, miss_inst=0, hit_inst=0, miss_data=0, hit_data=0, ex_forwarding = 0, not_taken=0, mem_forwarding = 0; int IF=0, ID=0, EX=0, MEM=0, WB=0, rs1=-1, rs2=-1, rd=-1, a[123659]; int j=0, branchvalid_0=0;

string opcode="", tag = "";

bool comp(int a, int b)
{
	return(a<b);
}

int stringToHex(string s1)
{
stringstream str;
str << s1;
int value;
str >> hex >> value;
return value;
}

char swap(char &a, char &b)
{
    char temp;
    temp=a;
    a=b;
    b=temp;
    return a,b;
}

string rev_data(string hex_bin_data)
{
    int len = hex_bin_data.length();
    for (int i=0; i<len/2; i++)
       swap(hex_bin_data[i], hex_bin_data[len-i-1]);
    return hex_bin_data;
}

int bin_dec(int num)
{   
    int dec_value = 0, base = 1;
     
    int temp = num;
    while (temp)
    {
        int last_digit = temp % 10;
        temp = temp/10;
        dec_value += last_digit*base; 
        base = base*2;
    }
     
    return dec_value;
}
void write_back()
{
    WB = MEM + 1;
    if(opcode!="00010" || opcode != "00011" || opcode!="10111")
    {
        score_board[rd][0] = WB;
    }
}
void memory()
{
    MEM = EX+1;
    if((MEM%32)==11)
    {
        miss_data++;
        MEM = MEM + 15;
    }
    else
    {
	hit_data++;
    }

    if(opcode!="00010" || opcode != "00011" || opcode!="10111")
    {
        score_board[rd][2] = MEM;
    }
    if(opcode=="00011" || opcode=="10111")
    {
	branchvalid_0 = MEM;
    }
    write_back();
}
void execution()
{
    EX = max({EX+1, MEM, ID+1}, comp);
    score_board[rd][1] = EX;
    if(opcode!="00010" || opcode != "00011")
    {
        score_board[rd][1]=EX;
    }
    memory();
}

void score_boarding_load()   /*for load*/
{
    if(tag == "alu" && score_board[rs1][1] == ID)
    {
        ex_forwarding++;
    }
   if((tag == "alu" || tag == "load word") && score_board[rs1][2] == ID)
    {
        mem_forwarding++;
    }

}

void score_boarding_store()
{
    if(tag == "alu" && score_board[rs1][1] == ID)
    {
        ex_forwarding++;
    }
   if((tag == "alu" || tag == "load word") && score_board[rs1][2] == ID)
    {
        mem_forwarding++;
    }
   

    if(tag == "alu" && score_board[rs2][1] == ID)
    {
        ex_forwarding++;
    }
    if((tag == "alu" || tag == "load word") && score_board[rs2][2] == ID)
    {
        mem_forwarding++;
    }
    
    
}
void score_boarding_add()
{
   if(tag == "alu" && score_board[rs1][1] == ID)
    {
        ex_forwarding++;
    }
   if((tag == "alu" || tag == "load word") && score_board[rs1][2] == ID)
    {
        mem_forwarding++;
    }
   if(score_board[rs1][1] > ID)
    {
	stall_id = stall_id + (max(EX, score_board[rs1][1])-ID+1);  //formula given by professor for calculating stalls from ID stage 
	score_board_time[rs1] = score_board[rs1][1];
        
    }
    if(score_board[rs1][2] > ID)
    {
	stall_id = stall_id + (max(EX, score_board[rs1][2])-ID+1);
	score_board_time[rs1] = score_board[rs1][2];
    }

    if(tag == "alu" && score_board[rs2][1] == ID)
    {
        ex_forwarding++;
    }
    if((tag == "alu" || tag == "load word") && score_board[rs2][2] == ID)
    {
        mem_forwarding++;
    }
    if(score_board[rs2][1] > ID) 
    {
	stall_id = stall_id + (max(EX, score_board[rs2][1])-ID-1);  //formula given by professor for calculating stalls from ID stage
        score_board_time[rs2] = score_board[rs2][1];
           
    }  
    if(score_board[rs2][2] > ID)
    {
	stall_id = stall_id + (max(EX, score_board[rs2][2])-ID+1);
	score_board_time[rs2] = score_board[rs2][2];
    }
}

void score_boarding_branch()
{
   if(tag == "alu" && score_board[rs1][1] == ID)
    {
        ex_forwarding++;
    }
   if((tag == "alu" || tag == "load word") && score_board[rs1][2] == ID)
    {
        mem_forwarding++;
    }
   

    if(tag == "alu" && score_board[rs2][1] == ID)
    {
        ex_forwarding++;
    }
    if((tag == "alu" || tag == "load word") && score_board[rs2][2] == ID)
    {
        mem_forwarding++;
    }
   
    
}

void score_boarding() //score boarding for extra instructions
{
    score_board[rd][3] = ID;
}

//INSTRUCTION DECODE
void instruction_decode(string data)
{
   
    opcode  = data.substr(2,5);
    ID = max({ID+1, EX, IF+1}, comp);
    if(opcode=="00000" || opcode=="10011" || opcode=="00100")   //load
    {	
        rs1 = bin_dec(stringToHex(rev_data(data.substr(15, 5))));
        rd = bin_dec(stringToHex(rev_data(data.substr(7,5))));
        load_access++;
        score_boarding_load();
	tag = "load word";

    }
    else if(opcode=="10011" || opcode=="00100")   //jalr//opimm//
    {	
        rs1 = bin_dec(stringToHex(rev_data(data.substr(15, 5))));
        rd = bin_dec(stringToHex(rev_data(data.substr(7,5))));
        score_boarding_load();

    }
    else if(opcode=="00010")  //store
    {
        rs1 = bin_dec(stringToHex(rev_data(data.substr(15,5))));
        rs2 = bin_dec(stringToHex(rev_data(data.substr(20,5))));
        store_access++;
        score_boarding_store();
    }
    else if(opcode=="00011" || opcode=="10111")  //branch//system
    {
	
        rs1 = bin_dec(stringToHex(rev_data(data.substr(15,5))));
        rs2 = bin_dec(stringToHex(rev_data(data.substr(20,5))));
        total_branches++;
	if((a[i+1]-a[i])!=4)
	{	
		branch_taken++;
	}	
       else
	not_taken++;
        score_boarding_branch();
    }
    else if(opcode=="10100" || opcode=="10110" || opcode=="01011"|| opcode=="11011")  //auipc//lui//j//jal
    {
        rd = bin_dec(stringToHex(rev_data(data.substr(7,5)))); 
        score_boarding();
    }
    else if(opcode=="00110")  //op//add
    {        
	rs1 = bin_dec(stringToHex(rev_data(data.substr(15,5))));
        rs2 = bin_dec(stringToHex(rev_data(data.substr(20,5))));
        rd = bin_dec(stringToHex(rev_data(data.substr(7,5))));
        alu++;
        score_boarding_add();
	tag = "alu";
    }

    execution();
}

void instruction_fetch(string data)
{
  
    if(opcode=="00011" || opcode=="10111")
	{
		IF = max({IF+1, ID, branchvalid_0 + 1}, comp);
	}
    else
	{
		IF = max(IF+1, ID);
	}
	
    if((IF%128)==55)
    {
        miss_inst++;
        IF = IF+15;
    }
    else
    {
        hit_inst++;
    }
        string hex_bin_data, hex_bin_data_rev;
        stringstream hexa;
        hexa<<hex<<data;
        int n;
        hexa>>n;
        bitset<32> bin_data(n);
        hex_bin_data = bin_data.to_string(); 
        hex_bin_data_rev = rev_data(hex_bin_data);
	instruction_decode(hex_bin_data_rev);
}


//int main(int argc, char** argv)
int main(void)
{  
    	string data, address, rev_address;
    	int b;
	unsigned int dec_addr=0;
	float k;
    	ifstream infile;
    	infile.open("in.txt");
    	ifstream file;
    	file.open("add.txt");
    	while(!file.eof())
    	{
       		file>>address;
		dec_addr = stringToHex(address);
		a[j]=dec_addr;
		j++;
    	}

    	while(!infile.eof())
    	{
        	infile>>data;
		i++;
		k++;
		
 		instruction_fetch(data);

    	}
	float a = (alu/k)*100;	
	float bt = (total_branches/k)*100;
	float l = (load_access/k)*100;
	float s = (store_access/k)*100;

    	cout<<"Number of Data Cache Load Access: "<<load_access<<endl;
   	cout<<"Number of Data Cache Store Access: "<<store_access<<endl;
    	cout<<"Number of Instruction Cache Access: "<<i<<endl;
    	cout<<"Number of Hits for Instruction Cache: "<<hit_inst<<endl;
    	cout<<"Number of Hits for Data Cache: "<<hit_data<<endl;
    	cout<<"Number of "<<endl<<"ALU type:"<<alu<<endl<<"Branch type:"<<total_branches<<endl<<"Load type:"<<load_access<<endl<<"Store type:"<<store_access<<endl;
    	cout<<"Number of taken branches: "<<branch_taken<<endl;
    	cout<<"Number of forwarding from execution stage:"<<ex_forwarding<<endl;
    	cout<<"Number of forwarding from memory stage:"<<mem_forwarding<<endl;
    	cout<<"Number of stall cycles in ID stage:"<<stall_id<<endl;
    	cout<<"Total cycles:"<<WB<<endl;
    	cout<<"Hit ratio of data cache: "<<(hit_data)/(load_access+store_access)<<endl;
    	cout<<"Instruction per Cycle: "<<k/WB<<endl;
	cout<<"Percentage of taken branch over total branches: "<<(branch_taken/total_branches)*100<<endl;
    	cout<<"Instruction Frequency of "<<endl<<"ALU type:"<<a<<endl<<"Branch type:"<<bt<<endl<<"Load type:"<<l<<endl<<"Store type:"<<s<<endl;
    	cout<<"Percentage of stalled cycles: "<<((stall_id)/WB)*100<<endl;
    	infile.close();
	file.close();	
   
}
